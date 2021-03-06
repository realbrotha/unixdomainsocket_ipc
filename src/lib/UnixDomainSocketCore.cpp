//
// Created by realbro on 20. 5. 26..
//

#include "UnixDomainSocketCore.h"
#include "UnixDomainSocketServer.h"
#include "UnixDomainSocketClient.h"

UnixDomainSocketCore::UnixDomainSocketCore() {

}
UnixDomainSocketCore::~UnixDomainSocketCore() {

}

bool UnixDomainSocketCore::Initialize(t_ListenerCallbackProc ConnectCallback,
                                      t_ListenerCallbackProc DisconnectCallback,
                                      t_ListenerCallbackProc ReadCallback, bool server_mode, int product_code) {
  if (server_mode) {
    printf ("Core::Init server\n\n");
    soeket_core_.reset(new UnixDomainSocketServer());
  } else {
    printf ("Core::Init client\n\n");
    soeket_core_.reset(new UnixDomainSocketClient());
  }
  soeket_core_->Initialize(ConnectCallback, DisconnectCallback, ReadCallback, product_code);
}
bool UnixDomainSocketCore::Finalize() {
  if (soeket_core_.get()) {
    soeket_core_->Finalize();
  }
  soeket_core_.reset();

  return false;
}

bool UnixDomainSocketCore::SendMessage(std::string data) {
  bool result = false;
  if (soeket_core_.get()) {
    soeket_core_->SendMessage(data);
    result = true;
  }
  return result;
}
