//
// Created by realbro on 20. 5. 26..
//

#ifndef TESTIPC_UNIXDOMAINSOCKET_SRC_UNIXDOMAINSOKET_LIB_UNIXDOMAINSOCKETFACTORYBASE_H_
#define TESTIPC_UNIXDOMAINSOCKET_SRC_UNIXDOMAINSOKET_LIB_UNIXDOMAINSOCKETFACTORYBASE_H_

#define PURE 0

#include <string>
#include <array>
#include <functional>

//typedef void (*t_ListenerCallbackProc)(std::array<char, 1024> message);
typedef std::function<void(std::array<char, 1024> message)> t_ListenerCallbackProc;

class UnixDomainSocketFactoryBase {
 public :
  UnixDomainSocketFactoryBase() {}
  virtual ~UnixDomainSocketFactoryBase() {}

  virtual bool SendMessage(std::string &send_string) = PURE;
  virtual bool Initialize(t_ListenerCallbackProc ConnectCallback,
                          t_ListenerCallbackProc DisconnectCallback,
                          t_ListenerCallbackProc ReadCallback) = PURE;
  virtual void Finalize() = PURE;
};

#endif //TESTIPC_UNIXDOMAINSOCKET_SRC_UNIXDOMAINSOKET_LIB_UNIXDOMAINSOCKETFACTORYBASE_H_
