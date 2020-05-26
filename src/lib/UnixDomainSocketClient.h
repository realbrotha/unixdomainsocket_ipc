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

  virtual bool Initialize(t_ListenerCallbackProc ResponseCallback) final;
  virtual void Finalize() final;

  virtual bool SendMessage(std::string &send_string);

 private :
  void EpollHandler();
  bool StartEpollThread();
  void StopEpollThread();

  int epoll_fd_;
  int client_socket_fd_;
  struct sockaddr_un server_addr_;
  t_ListenerCallbackProc callback_proc_;

  std::atomic<bool> stopped_;

  std::unique_ptr<std::thread> epoll_thread_;
};

#endif //TESTIPC_UNIXDOMAINSOCKET_SRC_UNIXDOMAINSOKET_LIB_UNIXDOMAINSOCKETCLIENT_H_
