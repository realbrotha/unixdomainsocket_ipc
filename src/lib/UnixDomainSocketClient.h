//
// Created by realbro on 20. 5. 26..
//

#ifndef TESTIPC_UNIXDOMAINSOCKET_SRC_UNIXDOMAINSOKET_LIB_UNIXDOMAINSOCKETCLIENT_H_
#define TESTIPC_UNIXDOMAINSOCKET_SRC_UNIXDOMAINSOKET_LIB_UNIXDOMAINSOCKETCLIENT_H_

#include "UnixDomainSocketFactoryBase.h"

#include <sys/socket.h>
#include <sys/un.h>

#include <atomic>
#include <thread>
namespace {
constexpr char kFILE_NAME[] = "/tmp/test_server";
constexpr int kMAX_EVENT_COUNT = 1;
constexpr int kMAX_READ_SIZE = 1024;
}

class UnixDomainSocketClient : public UnixDomainSocketFactoryBase {
 public :
  UnixDomainSocketClient();
  virtual ~UnixDomainSocketClient();

  virtual bool Initialize(t_ListenerCallbackProc ConnectCallback,
                          t_ListenerCallbackProc DisconnectCallback,
                          t_ListenerCallbackProc ReadCallback,
                          int product_code) final;
  virtual void Finalize() final;

  virtual bool SendMessage(std::string &send_string);
  virtual bool SendMessage(int product_code, std::string &send_string);

 private :
  bool SendServerHello();
  int product_code_;
  int seq_id_;

  void EpollHandler();
  bool StartEpollThread();
  void StopEpollThread();

  int epoll_fd_;
  int server_socket_fd_;
  struct sockaddr_un server_addr_;

  t_ListenerCallbackProc connect_callback_proc_;
  t_ListenerCallbackProc disconnect_callback_proc_;
  t_ListenerCallbackProc read_callback_proc_;

  std::atomic<bool> stopped_;

  std::unique_ptr<std::thread> epoll_thread_;
};

#endif //TESTIPC_UNIXDOMAINSOCKET_SRC_UNIXDOMAINSOKET_LIB_UNIXDOMAINSOCKETCLIENT_H_
