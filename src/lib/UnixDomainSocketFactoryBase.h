//
// Created by realbro on 20. 5. 26..
//

#ifndef TESTIPC_UNIXDOMAINSOCKET_SRC_UNIXDOMAINSOKET_LIB_UNIXDOMAINSOCKETFACTORYBASE_H_
#define TESTIPC_UNIXDOMAINSOCKET_SRC_UNIXDOMAINSOKET_LIB_UNIXDOMAINSOCKETFACTORYBASE_H_

#define PURE 0

#include <string>
#include <array>

typedef void (*t_ListenerCallbackProc)(std::array<char, 1024> message);

class UnixDomainSocketFactoryBase {
 public :
  UnixDomainSocketFactoryBase() {}
  virtual ~UnixDomainSocketFactoryBase() {}

  virtual bool SendMessage(std::string &send_string) = PURE;
  virtual bool Initialize(t_ListenerCallbackProc ResponseCallback) = PURE;
  virtual void Finalize() = PURE;
};

#endif //TESTIPC_UNIXDOMAINSOCKET_SRC_UNIXDOMAINSOKET_LIB_UNIXDOMAINSOCKETFACTORYBASE_H_
