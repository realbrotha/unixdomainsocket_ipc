//
// Created by realbro on 20. 5. 26..
//

// TODO : 나중에 동적 라이브러리로 빼면 사용 할 것
#include "UnixDomainSocketCore.h"
#include <thread>

#define EXPORT extern "C" __attribute__ ((visibiliity("default")))
#define NOEXCEPT throw()

EXPORT bool Initialize(t_ListenerCallbackProc ResponseCallback) NOEXCEPT;
EXPORT bool Finalize() NOEXCEPT;

std::shared_ptr<UnixDomainSocketCore> g_socket_core;

EXPORT bool Initialize(t_ListenerCallbackProc ResponseCallback) NOEXCEPT {
  //do something;
  if (g_socket_core.get()) {
    return true;
  }
  try {
    g_socket_core.reset(new UnixDomainSocketCore());
    g_socket_core->Initialize(ResponseCallback);
  } catch (const std::exception &ex) {
    printf ("Exception 1~.");
  } catch ( ... ) {
    printf ("Exception 2~.");
  }
  return true;
}

EXPORT bool Finalize() NOEXCEPT {
  try {
    if (g_socket_core.get()) {
      g_socket_core->Finalize();
      g_socket_core.reset();
    }
  } catch (std::exception &ex) {
    printf ("Ex 1. ");
    g_socket_core.reset();
  } catch (...) {
    printf("Ex 2. ");
    g_socket_core.reset();
  }
  return true;
}