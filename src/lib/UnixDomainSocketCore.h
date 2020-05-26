//
// Created by realbro on 20. 5. 26..
//

#ifndef TESTIPC_UNIXDOMAINSOCKET_SRC_UNIXDOMAINSOKET_LIB_UNIXDOMAINSOCKETCORE_H_
#define TESTIPC_UNIXDOMAINSOCKET_SRC_UNIXDOMAINSOKET_LIB_UNIXDOMAINSOCKETCORE_H_

#include "UnixDomainSocketFactoryBase.h"

#include <thread>

class UnixDomainSocketCore {
 public :
  UnixDomainSocketCore();
  ~UnixDomainSocketCore();

  bool Initialize(bool server_mode = false);
  bool Finalize();
 private:

  std::unique_ptr<UnixDomainSocketFactoryBase> soeket_core_;
};

#endif //TESTIPC_UNIXDOMAINSOCKET_SRC_UNIXDOMAINSOKET_LIB_UNIXDOMAINSOCKETCORE_H_
