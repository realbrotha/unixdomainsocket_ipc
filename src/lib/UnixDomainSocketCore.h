//
// Created by realbro on 20. 5. 26..
//

#ifndef TESTIPC_UNIXDOMAINSOCKET_SRC_UNIXDOMAINSOKET_LIB_UNIXDOMAINSOCKETCORE_H_
#define TESTIPC_UNIXDOMAINSOCKET_SRC_UNIXDOMAINSOKET_LIB_UNIXDOMAINSOCKETCORE_H_

#include "UnixDomainSocketFactoryBase.h"

#include <thread>

// TODO : Class 뎁스가 깊어져서 Impl pImpl형태로 만드는게 좋을듯, Method 하나가 추가될때마다 클래스 수정이 많다.

class UnixDomainSocketCore {
 public :
  UnixDomainSocketCore();
  ~UnixDomainSocketCore();

  bool Initialize(t_ListenerCallbackProc ConnectCallback,
                  t_ListenerCallbackProc DisconnectCallback,
                  t_ListenerCallbackProc ReadCallback, bool server_mode = false);
  bool Finalize();

  bool SendMessage(std::string data);
 private:

  std::unique_ptr<UnixDomainSocketFactoryBase> soeket_core_;
};

#endif //TESTIPC_UNIXDOMAINSOCKET_SRC_UNIXDOMAINSOKET_LIB_UNIXDOMAINSOCKETCORE_H_
