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
    : epoll_fd_(-1), server_socket_fd_(-1), stopped_(false), server_addr_({0,}) {
  server_addr_.sun_family = AF_UNIX;
  strcpy(server_addr_.sun_path, kFILE_NAME);
}

UnixDomainSocketClient::~UnixDomainSocketClient() {
  Finalize();
}

bool UnixDomainSocketClient::Initialize(t_ListenerCallbackProc ConnectCallback,
                                        t_ListenerCallbackProc DisconnectCallback,
                                        t_ListenerCallbackProc ReadCallback,
                                        int product_code) {
  if (product_code <= 0) {
    printf ("product_code");
    return false;
  }
  product_code_ = product_code;

  if (ConnectCallback)
    connect_callback_proc_ = std::move(ConnectCallback);
  if (DisconnectCallback)
    disconnect_callback_proc_ = std::move(DisconnectCallback);
  if (ReadCallback)
    read_callback_proc_ = std::move(ReadCallback);

  int socket_fd = -1;
  if (!SocketWrapper::Create(socket_fd)) {
    return false;
  }
  server_socket_fd_ = socket_fd;
  FileDescriptorTool::SetCloseOnExec(server_socket_fd_, true);
  FileDescriptorTool::SetNonBlock(server_socket_fd_, true);

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
    if (server_socket_fd_ < 0) { // socket을 생성하고 connection 시킴,

      if (SocketWrapper::Create(server_socket_fd_)) {      // 클라이언트 입장에서 이짓을 하는게 맞는가 싶다. 접속중에 Server가 사라지면 대기를 위함.
        FileDescriptorTool::SetCloseOnExec(server_socket_fd_, true);
        FileDescriptorTool::SetNonBlock(server_socket_fd_, true);
      } else {
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
        continue;
      }
    }

    if (!isConnected_) { // socket을 생성하고 connection 시킴
      if (SocketWrapper::Connect(server_socket_fd_, server_addr_)) { // socket connect ok
        if (SendServerHello()) {  // 서버에게 식별자를 던진다.
          isConnected_ = true;

          std::array<char, kMAX_READ_SIZE> message;
          std::string connect_signal = "connect";
          std::copy(connect_signal.begin(), connect_signal.end(), message.begin());
          message[connect_signal.length()] = '\0';

          if (connect_callback_proc_)
            connect_callback_proc_(message);
        }
      } else { // socket failed
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
        continue;
      }
    }
    if (!isEpollAdded_) {
      if (EpollWrapper::EpollControll(epoll_fd_,
                                      server_socket_fd_,
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
        if (gettingEvent[i].data.fd == server_socket_fd_) // 구지 체크 할 필요 없음
        {
          std::array<char, kMAX_READ_SIZE> message;

          int read_size = read(server_socket_fd_, message.data(), message.size());
          if (read_size < 0) { // continue
            continue;
          }
          if (read_size == 0) { // Error or Disconnect Signal
            restart_flag = true;

            std::string end_signal= "disconnect";
            std::copy(end_signal.begin(), end_signal.end(), message.begin());
            message[end_signal.length()] = '\0';

            if (disconnect_callback_proc_) // disconnect signal
              disconnect_callback_proc_(message);
            continue;
          }
          // read ok
          // TODO : 긴 데이터에 대한 처리 할것 , Read가 계속적으로 발생가능 알아서 처리
          message[read_size] = '\0';
          if (read_callback_proc_)
            read_callback_proc_(message);
        }
      }
    }
    if (event_count < 0 || restart_flag)
    {
      // epoll 상에서 문제가 발생할 경우 혹은 쓰레드 재기동이 필요한경우. 조건문이 좀 이상함 바로 위 조건문이랑 맞출것
      EpollWrapper::EpollControll(epoll_fd_,
                                  server_socket_fd_,
                                  EPOLLIN | EPOLLOUT | EPOLLERR | EPOLLET,
                                  EPOLL_CTL_DEL);
      close(server_socket_fd_);

      server_socket_fd_ = -1;
      isEpollAdded_ = false;
      isConnected_ = false;
      restart_flag = false;
      continue;
    }
  }
}
#include "MessageDefine.hpp"

bool UnixDomainSocketClient::SendServerHello() {
  printf ("Send Hi HELOO\n");
  IpcJsonMessage msg;
  msg.msg_type = 0x00;
  msg.msg_id = 0;
  msg.src = product_code_;
  msg.dst = 0x00;
  msg.synchronous = true;
  msg.json_string = "hi~there";

  std::vector<char> serialize;
  msg.Serialize(serialize);

  ssize_t write_result_size = 0;
  if (server_socket_fd_ > 0) {
    write_result_size = write(server_socket_fd_, &serialize[0], serialize.size());
  }
  printf ("send size [%d], send size [%d]", msg.json_string.length(), write_result_size);
  return (write_result_size > 0) ? true : false;
}

bool UnixDomainSocketClient::SendMessage(int product_code, std::string &send_string) {
  // TODO : 큰 데이터 처리 끊어서 보낼것
  ssize_t write_result_size = 0;
  if (server_socket_fd_ > 0) {
    write_result_size = write(server_socket_fd_, send_string.c_str(), send_string.length());
  }
  return (write_result_size > 0) ? true : false;
}

bool UnixDomainSocketClient::SendMessage(std::string &send_string) {
  // TODO : 큰 데이터 처리 끊어서 보낼것
  IpcJsonMessage msg;
  msg.msg_type = 1;
  msg.msg_id = seq_id_++;
  msg.src = product_code_;
  msg.dst = 0x00;
  msg.synchronous = true;
  msg.json_string = send_string;

  printf ("send data :%d\n", msg.msg_type);
  printf ("send data :%d\n", msg.msg_id);
  printf ("send data :%d\n", msg.src);
  printf ("send data :%d\n", msg.dst);
  printf ("send data :%d\n", msg.synchronous);
  std::vector<char> serialize;
  msg.Serialize(serialize);

  printf ("serialize size :%d\n",serialize.size());
  ssize_t write_result_size = 0;
  if (server_socket_fd_ > 0) {
    write_result_size = write(server_socket_fd_, &serialize[0], serialize.size());
  }
  return (write_result_size > 0) ? true : false;
}
