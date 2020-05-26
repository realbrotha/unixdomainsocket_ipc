//
// Created by realbro on 20. 5. 26..
//

#ifndef TESTIPC_UNIXDOMAINSOCKET_SRC_UNIXDOMAINSOKET_LIB_UNIXDOMAINSOCKETFACTORYBASE_H_
#define TESTIPC_UNIXDOMAINSOCKET_SRC_UNIXDOMAINSOKET_LIB_UNIXDOMAINSOCKETFACTORYBASE_H_

#define PURE 0

class UnixDomainSocketFactoryBase {
 public :
  UnixDomainSocketFactoryBase() {}
  virtual ~UnixDomainSocketFactoryBase() {}

  virtual bool Initialize() = PURE;
  virtual bool Finalize() = PURE;
};

#endif //TESTIPC_UNIXDOMAINSOCKET_SRC_UNIXDOMAINSOKET_LIB_UNIXDOMAINSOCKETFACTORYBASE_H_
