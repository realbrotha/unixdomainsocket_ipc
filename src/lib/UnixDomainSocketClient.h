//
// Created by realbro on 20. 5. 26..
//

#ifndef TESTIPC_UNIXDOMAINSOCKET_SRC_UNIXDOMAINSOKET_LIB_UNIXDOMAINSOCKETCLIENT_H_
#define TESTIPC_UNIXDOMAINSOCKET_SRC_UNIXDOMAINSOKET_LIB_UNIXDOMAINSOCKETCLIENT_H_

#include "UnixDomainSocketFactoryBase.h"

class UnixDomainSocketClient : public UnixDomainSocketFactoryBase {
 public :
  UnixDomainSocketClient();
  virtual ~UnixDomainSocketClient();

  virtual bool Initialize() final;
  virtual bool Finalize() final;
};

#endif //TESTIPC_UNIXDOMAINSOCKET_SRC_UNIXDOMAINSOKET_LIB_UNIXDOMAINSOCKETCLIENT_H_
