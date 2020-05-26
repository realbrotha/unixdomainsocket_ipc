//
// Created by realbro on 20. 5. 26..
//

#include "UnixDomainSocketClient.h"
#include "SocketWrapper.h"
#include "EpollWrapper.h"
#include "FileDescriptorTool.h"

#include <unistd.h>

#include <chrono>
#include <iostream>

UnixDomainSocketClient::UnixDomainSocketClient()
    : epoll_fd_(-1), client_socket_fd_(-1), stopped_(false), server_addr_({0,}) {
  server_addr_.sun_family = AF_UNIX;
  strcpy(server_addr_.sun_path, kFILE_NAME);
}

UnixDomainSocketClient::~UnixDomainSocketClient() {
  Finalize();
}

bool UnixDomainSocketClient::Initialize(t_ListenerCallbackProc ResponseCallback) {
  callback_proc_ = ResponseCallback;

  int socket_fd = -1;
  if (!SocketWrapper::Create(socket_fd)) {
    return false;
  }
  client_socket_fd_ = socket_fd;
  FileDescriptorTool::SetCloseOnExec(client_socket_fd_, true);
  FileDescriptorTool::SetNonBlock(client_socket_fd_, true);

  int epoll_fd = -1;
  if (!EpollWrapper::EpollCreate(1, true, epoll_fd)) {
    return false;
  }
  epoll_fd_ = epoll_fd;
  FileDescriptorTool::SetCloseOnExec(epoll_fd_, true);
  FileDescriptorTool::SetNonBlock(epoll_fd_, true);

  StartEpollThread();
  return true;
}
void UnixDomainSocketClient::Finalize() {
  stopped_ = true;

  StopEpollThread();
  epoll_thread_.reset();

}
bool UnixDomainSocketClient::StartEpollThread() {
  bool result = false;
  if (epoll_thread_.get() && epoll_thread_->joinable()) {
    return true;
  }
  try {
    epoll_thread_.reset(new std::thread(&UnixDomainSocketClient::EpollHandler, std::ref(*this)));
    result = true;
  } catch (std::exception &ex) {
    printf("thread ex -1\n");
  } catch (...) {
    printf("thread ex -2\n");
  }
  return result;
}
void UnixDomainSocketClient::StopEpollThread() {
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
void UnixDomainSocketClient::EpollHandler() {
  bool restart_flag = false;
  bool isEpollAdded_ = false;
  bool isConnected_ = false;

  while (!stopped_) {
    if (client_socket_fd_ < 0) { // socket을 생성하고 connection 시킴,
      if (SocketWrapper::Create(client_socket_fd_)) {      // 클라이언트 입장에서 이짓을 하는게 맞는가 싶다. 접속중에 Server가 사라지면 대기를 위함.
        FileDescriptorTool::SetCloseOnExec(client_socket_fd_, true);
        FileDescriptorTool::SetNonBlock(client_socket_fd_, true);
      } else {
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
        continue;
      }
    }

    if (!isConnected_) { // socket을 생성하고 connection 시킴
      if (SocketWrapper::Connect(client_socket_fd_, server_addr_)) { // socket connect ok
        isConnected_ = true;
      } else { // socket failed
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
        continue;
      }
    }
    if (!isEpollAdded_) {
      if (EpollWrapper::EpollControll(epoll_fd_,
                                      client_socket_fd_,
                                      EPOLLIN | EPOLLOUT | EPOLLERR | EPOLLET,
                                      EPOLL_CTL_ADD)) { // Epoll add ok
        isEpollAdded_ = true;
      } else { // epoll failed
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
        continue;
      }
    }
    // 정상적으로 처리 될경우
    epoll_event gettingEvent[kMAX_EVENT_COUNT] = {0, {0,}};
    int event_count = epoll_wait(epoll_fd_, gettingEvent, kMAX_EVENT_COUNT, 10 /* ms */);

    if (event_count > 0)
    { // 이벤트 발생, event_count = 0 처리안함.
      for (int i = 0; i < event_count; ++i)
      {
        if (gettingEvent[i].data.fd == client_socket_fd_) // 구지 체크 할 필요 없음
        {
          std::array<char, kMAX_READ_SIZE> message;

          int read_size = read(client_socket_fd_, message.data(), message.size());
          if (read_size < 0) { // continue
            continue;
          }
          if (read_size == 0) { // Error or Disconnect Signal
            restart_flag = true;

            std::string end_signal= "disconnect";
            std::copy(end_signal.begin(), end_signal.end(), message.begin());
            message[end_signal.length()] = '\0';
            if (callback_proc_) // disconnect signal
              callback_proc_(message);
            continue;
          }
          // read ok
          // TODO : 긴 데이터에 대한 처리 할것 , Read가 계속적으로 발생가능 알아서 처리
          if (callback_proc_)
            callback_proc_(message);
        }
      }
    }
    if (event_count < 0 || restart_flag)
    {
      // epoll 상에서 문제가 발생할 경우 혹은 쓰레드 재기동이 필요한경우. 조건문이 좀 이상함 바로 위 조건문이랑 맞출것
      EpollWrapper::EpollControll(epoll_fd_,
                                  client_socket_fd_,
                                  EPOLLIN | EPOLLOUT | EPOLLERR | EPOLLET,
                                  EPOLL_CTL_DEL);
      close(client_socket_fd_);

      client_socket_fd_ = -1;
      isEpollAdded_ = false;
      isConnected_ = false;
      restart_flag = false;
      continue;
    }
  }
  std::cout << "Client EpollHandler Thread!!!! END OF Thread" << std::endl;
  pthread_exit(NULL);
}

bool UnixDomainSocketClient::SendMessage(std::string &send_string) {
  // TODO : 큰 데이터 처리 끊어서 보낼것
  ssize_t write_result_size = 0;
  if (client_socket_fd_ > 0) {
    write_result_size = write(client_socket_fd_, send_string.c_str(), send_string.length());
  }
  return (write_result_size > 0) ? true : false;
}
