//
// Created by realbro on 20. 5. 26..
//

#include "UnixDomainSocketServer.h"
#include "EpollWrapper.h"
#include "SocketWrapper.h"
#include "FileDescriptorTool.h"

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
  server_addr_.sun_family = AF_UNIX;
  strcpy(server_addr_.sun_path, kFILE_NAME);
}

UnixDomainSocketServer::~UnixDomainSocketServer() {
  Finalize();
}

bool UnixDomainSocketServer::Initialize(t_ListenerCallbackProc ResponseCallback) {
  callback_proc_ = ResponseCallback;

  if (0 == access(kFILE_NAME, F_OK)) {
    unlink(kFILE_NAME);
  }

  int socket = -1;
  if (!SocketWrapper::Create(socket)) {
    return false;
  }
  accept_checker_fd_ = socket;

  if (!SocketWrapper::Bind(accept_checker_fd_, server_addr_)) {
    return false;
  }
  if (!SocketWrapper::Listen(accept_checker_fd_, 5)) {
    return false;
  }
  int epoll_fd = -1;
  if (!EpollWrapper::EpollCreate(1, true, epoll_fd_)) {
    return false;
  }
  epoll_fd_ = epoll_fd;

  client_socket_list_.assign(kMAX_CONNECTION_COUNT, 0); // TODO: 커넥션 확장성 고려

  StartEpollThread();

  return true;
}

void UnixDomainSocketServer::Finalize() {
  stopped_ = true;

  StopEpollThread();
  epoll_thread_.reset();

}
bool UnixDomainSocketServer::StartEpollThread() {
  bool result = false;

  if (epoll_thread_.get() && epoll_thread_->joinable()) {
    return true;
  }
  try {
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
  if (!EpollWrapper::EpollControll(epoll_fd_,
                                   accept_checker_fd_,
                                   EPOLLIN | EPOLLOUT | EPOLLERR,
                                   EPOLL_CTL_ADD)) {
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
        if (gettingEvent[i].data.fd == accept_checker_fd_) // accept check
        {
          int client_socket = 0;
          if (!SocketWrapper::Accept(client_socket, accept_checker_fd_, server_addr_)) {
            continue;
          } else { //  accept ok
            client_socket_list_[0] = client_socket;

            if (!EpollWrapper::EpollControll(epoll_fd_,
                                             client_socket,
                                             EPOLLIN | EPOLLOUT | EPOLLERR | EPOLLET,
                                             EPOLL_CTL_ADD)) {
              continue;
            }
          }
        } else { // accept 가 처리된 구들.
          std::array<char, kMAX_READ_SIZE> message;
          int read_size = read( gettingEvent[i].data.fd, message.data(), message.size());
          if (read_size < 0) {   // read 0
            continue;
          } else if (read_size == 0) { // Disconnect
            EpollWrapper::EpollControll(epoll_fd_,
                                        gettingEvent[i].data.fd,
                                        EPOLLIN | EPOLLOUT | EPOLLERR | EPOLLET,
                                        EPOLL_CTL_DEL);
            continue;
          }
          // Read data ok
          // TODO : 긴 데이터에 대한 처리 할것 , Read가 계속적으로 발생가능 알아서 처리
          if (callback_proc_)
            callback_proc_(message);
        }
      }
    }
  }
}

bool UnixDomainSocketServer::SendMessage(std::string &send_string) {
  // TODO : 큰 데이터 처리 끊어서 보낼것
  ssize_t write_result_size = 0;
  if (client_socket_list_.size() > 0 && client_socket_list_[0] > 0) {
    write_result_size = write(client_socket_list_[0], send_string.c_str(), send_string.length());
  }
  return (write_result_size > 0) ? true : false;
}
