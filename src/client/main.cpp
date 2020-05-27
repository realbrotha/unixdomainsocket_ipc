
#include <iostream>

#include "UnixDomainSocketCore.h"
void ReadbackPtr(std::array<char, 1024> message) {
  printf ("read recv [%s]\n",message.data());
}
void ConnectCallbackPtr(std::array<char, 1024> message) {
  printf ("connect recv [%s]\n",message.data());
}
void DisConnectCallbackPtr(std::array<char, 1024> message) {
  printf ("disconnect recv [%s]\n",message.data());
}

int main(int argc, char *argv[]) {
  UnixDomainSocketCore core_;

  core_.Initialize(ConnectCallbackPtr, DisConnectCallbackPtr, ReadbackPtr, false);

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