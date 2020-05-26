//
// Created by realbro on 20. 5. 26..
//

#ifndef TESTIPC_UNIXDOMAINSOCKET_SRC_UNIXDOMAINSOKET_LIB_UNIXDOMAINSOCKETSERVER_H_
#define TESTIPC_UNIXDOMAINSOCKET_SRC_UNIXDOMAINSOKET_LIB_UNIXDOMAINSOCKETSERVER_H_

#include "UnixDomainSocketFactoryBase.h"

class UnixDomainSocketServer : public UnixDomainSocketFactoryBase {
 public :
  UnixDomainSocketServer();
  virtual ~UnixDomainSocketServer();

  virtual bool Initialize() final;
  virtual bool Finalize() final;
};

#endif //TESTIPC_UNIXDOMAINSOCKET_SRC_UNIXDOMAINSOKET_LIB_UNIXDOMAINSOCKETSERVER_H_
