//
// Created by realbro on 20. 5. 26..
//

#include "UnixDomainSocketServer.h"
#include "EpollWrapper.h"
#include "SocketWrapper.h"
#include "FileDescriptorTool.h"
#include "MessageDefine.hpp"
#include <unistd.h>
#include <stdio.h>
#include <string.h>

#include <iostream>

namespace {
constexpr char kFILE_NAME[] = "/tmp/test_server";
constexpr int kMAX_EVENT_COUNT = 3;
constexpr int kMAX_READ_SIZE = 1024;
constexpr int kMAX_CONNECTION_COUNT = 1;
}

UnixDomainSocketServer::UnixDomainSocketServer()
    : accept_checker_fd_(-1), epoll_fd_(-1), server_addr_({0,}), stopped_(false) {
  memset(&server_addr_, 0, sizeof(server_addr_));
  server_addr_.sun_family = AF_UNIX;
  strcpy(server_addr_.sun_path, kFILE_NAME);
}

UnixDomainSocketServer::~UnixDomainSocketServer() {

}

bool UnixDomainSocketServer::Initialize(t_ListenerCallbackProc ConnectCallback,
                                        t_ListenerCallbackProc DisconnectCallback,
                                        t_ListenerCallbackProc ReadCallback,
                                        int product_code) {
  if (ConnectCallback)
    connect_callback_proc_ = std::move(ConnectCallback);
  if (DisconnectCallback)
    disconnect_callback_proc_ = std::move(DisconnectCallback);
  if (ReadCallback)
    read_callback_proc_ = std::move(ReadCallback);

  if (0 == access(kFILE_NAME, F_OK)) {
    unlink(kFILE_NAME);
  }

  int socket = -1;
  if (!SocketWrapper::Create(socket)) {
    std::cout << "Socket Create Failed" << std::endl;
    return false;
  }
  accept_checker_fd_ = socket;
  printf ("Accept Check fd : %d\n",accept_checker_fd_ );
  //async_message_callback_ = callback;

  bool result = false;

  do {
    if (!SocketWrapper::Bind(accept_checker_fd_, server_addr_)) {
      std::cout << "Socket Bind Failed :" << errno << std::endl;
      break;
    }
    if (!SocketWrapper::Listen(accept_checker_fd_, 5)) {
      std::cout << "Socket Listen Failed" << std::endl;
      break;
    }
    if (!EpollWrapper::EpollCreate(1, true, epoll_fd_)) {
      std::cout << "Epoll Create Failed" << std::endl;
      break;
    }
    //client_socket_list_.assign(kMAX_CONNECTION_COUNT, 0); // TODO: 커넥션 확장성 고려

    StartEpollThread();

    result = true;
  } while(false);

  return result;
}

void UnixDomainSocketServer::Finalize() {
  stopped_ = true;

  StopEpollThread();
  epoll_thread_.reset();

}
bool UnixDomainSocketServer::StartEpollThread() {
  bool result = false;

  if (epoll_thread_.get() && epoll_thread_->joinable()) {
    printf("already exist\n");
    return true;
  }
  try {
    printf("start thread -1\n");
    epoll_thread_.reset(new std::thread(&UnixDomainSocketServer::EpollHandler, std::ref(*this)));
    result = true;
  } catch (std::exception &ex) {
    printf("thread ex -1\n");
  } catch (...) {
    printf("thread ex -2\n");
  }
  return result;
}
void UnixDomainSocketServer::StopEpollThread() {
  if (epoll_thread_.get()) {
    if (std::this_thread::get_id() == epoll_thread_->get_id()) {
      epoll_thread_->detach();
    } else {
      try {
        epoll_thread_->join();
      } catch (const std::exception &ex) {
        printf("exceoption -1 \n");
      } catch (...) {
        printf("exceoption -2\n");
      }
    }
  }
}
void UnixDomainSocketServer::EpollHandler() {
  std::cout << "~~~~~EpollHandler ~~~~~~~" << std::endl;

  if (!EpollWrapper::EpollControll(epoll_fd_,
                                   accept_checker_fd_,
                                   EPOLLIN | EPOLLOUT | EPOLLERR,
                                   EPOLL_CTL_ADD)) {
    std::cout << "epoll add failed" << std::endl;
    return;
  }
  int errorCount = 0;
  while (!stopped_) {
    epoll_event gettingEvent[kMAX_EVENT_COUNT] = {0, {0}};
    int event_count = epoll_wait(epoll_fd_, gettingEvent, kMAX_EVENT_COUNT, 10 /* ms */);
    if (event_count < 0) {  // epoll error
      if (errno == EINTR) {
        continue;
      }
      if (++errorCount < 3) {
        continue;
      }
      break; // 에러가 많으면 종료
    }
    if (event_count > 0) // 이벤트 떨어짐.  event_count = 0 처리안함.
    {
      for (int i = 0; i < event_count; ++i) {
        std::array<char, kMAX_READ_SIZE> message;

        if (gettingEvent[i].data.fd == accept_checker_fd_) // accept check
        {
          int client_socket = 0;
          if (!SocketWrapper::Accept(client_socket, accept_checker_fd_, server_addr_)) {
            std::cout << "Socket Accept Failed" << std::endl;
            continue;
          } else { //  accept ok
            const int kREADY_SOCKET =0;
            //client_socket_list_[client_socket] = kREADY_SOCKET;

            std::string connect_signal= "connect";
            std::copy(connect_signal.begin(), connect_signal.end(), message.begin());
            message[connect_signal.length()] = '\0';

            if (connect_callback_proc_)
              connect_callback_proc_(message);

            std::cout << "Socket Accept Called" << std::endl;
            if (!EpollWrapper::EpollControll(epoll_fd_,
                                             client_socket,
                                             EPOLLIN | EPOLLOUT | EPOLLERR | EPOLLET,
                                             EPOLL_CTL_ADD)) {
              std::cout << "epoll add failed" << std::endl;
              continue;
            }
          }
        } else { // accept 가 처리된 구들.
          int read_size = read( gettingEvent[i].data.fd, message.data(), message.size());
          if (read_size < 0) {   // read 0
            continue;
          } else if (read_size == 0) { // Disconnect
            ConnectionManager::GetInstance().Remove(gettingEvent[i].data.fd);

            std::string connect_signal= "disconnect";
            std::copy(connect_signal.begin(), connect_signal.end(), message.begin());
            message[connect_signal.length()] = '\0';

            if (disconnect_callback_proc_)
              disconnect_callback_proc_(message);

            EpollWrapper::EpollControll(epoll_fd_,
                                        gettingEvent[i].data.fd,
                                        EPOLLIN | EPOLLOUT | EPOLLERR | EPOLLET,
                                        EPOLL_CTL_DEL);
            continue;
          }
          // Read data ok
          // TODO : 긴 데이터에 대한 처리 할것 , Read가 계속적으로 발생가능 알아서 처리
          message[read_size] = '\0';
          std::vector<char> buffer;
          buffer.assign(message.begin(), message.begin() + read_size);
          IpcJsonMessage msg;
          msg.Deserialize(buffer);

          if (msg.msg_id == 0 &&
              msg.msg_type == 0 &&
              msg.json_string == "hi~there") {
            printf ("hello complete\n");
            ConnectionManager::GetInstance().Add(msg.src, gettingEvent[i].data.fd);
            //client_socket_list_[msg.src] = gettingEvent[i].data.fd;
            continue;
          }

          if (read_callback_proc_)
            read_callback_proc_(message);
        }
      }
    }
  }
  printf("bye bye~");
}

bool UnixDomainSocketServer::SendMessage(std::string &send_string) {
  // TODO : 큰 데이터 처리 끊어서 보낼것
  ssize_t write_result_size = 0;
  if (client_socket_list_.size() > 0 && client_socket_list_[0] > 0) {
    write_result_size = write(client_socket_list_[0], send_string.c_str(), send_string.length());
  }
  return (write_result_size > 0) ? true : false;
}
