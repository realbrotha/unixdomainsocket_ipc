
#include <iostream>

#include "UnixDomainSocketCore.h"
void CallbackPtr(std::array<char, 1024> message) {
  printf ("recv [%s]\n",message.data());
}

int main(int argc, char *argv[]) {
  UnixDomainSocketCore core_;
  core_.Initialize(CallbackPtr, false);

  while (1)
  {
    char buff[1024] = { 0,};
    scanf("%s",buff);
    std::string send(buff);
    if (send == "exit") {
      core_.Finalize();
      break;
    }
    core_.SendMessage(send);
  }
}