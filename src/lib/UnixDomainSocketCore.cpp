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

bool UnixDomainSocketCore::Initialize(bool server_mode) {
  if (server_mode) {
    printf ("Core::Init server");
    soeket_core_.reset(new UnixDomainSocketServer());
  } else {
    printf ("Core::Init client");
    soeket_core_.reset(new UnixDomainSocketClient());
  }
  soeket_core_->Initialize();
}
bool UnixDomainSocketCore::Finalize() {
  if (soeket_core_.get()) {
    soeket_core_->Finalize();
  }
  soeket_core_.reset();

  return false;
}