//
// Created by realbro on 20. 5. 26..
//

#ifndef TESTIPC_UNIXDOMAINSOCKET_SRC_UNIXDOMAINSOKET_LIB_UNIXDOMAINSOCKETSERVER_H_
#define TESTIPC_UNIXDOMAINSOCKET_SRC_UNIXDOMAINSOKET_LIB_UNIXDOMAINSOCKETSERVER_H_

#include "UnixDomainSocketFactoryBase.h"

#include <sys/socket.h>
#include <sys/un.h>

#include <thread>
#include <atomic>
#include <vector>
#include <map>

class UnixDomainSocketServer : public UnixDomainSocketFactoryBase {
 public :
  UnixDomainSocketServer();
  virtual ~UnixDomainSocketServer();

  virtual bool Initialize(t_ListenerCallbackProc ConnectCallback,
                          t_ListenerCallbackProc DisconnectCallback,
                          t_ListenerCallbackProc ReadCallback) final;
  virtual void Finalize() final;

  virtual bool SendMessage(std::string &send_string);

 private :
  bool StartEpollThread();
  void StopEpollThread();
  void EpollHandler();

  int epoll_fd_;
  int accept_checker_fd_;
  struct sockaddr_un server_addr_;

  t_ListenerCallbackProc connect_callback_proc_;
  t_ListenerCallbackProc disconnect_callback_proc_;
  t_ListenerCallbackProc read_callback_proc_;

  std::atomic<bool> stopped_;
  std::map<int, int> client_socket_list_;
  std::unique_ptr<std::thread> epoll_thread_;
};

#endif //TESTIPC_UNIXDOMAINSOCKET_SRC_UNIXDOMAINSOKET_LIB_UNIXDOMAINSOCKETSERVER_H_
