//#include "UnixDomainSocketInterface.h"

#include "UnixDomainSocketCore.h"
#include <unistd.h>
#include <array>
#include <functional>

void ReadbackPtr(std::array<char, 1024> message) {
  printf ("read recv [%s]\n",message.data());
}
void ConnectCallbackPtr(std::array<char, 1024> message) {
  printf ("connect recv [%s]\n",message.data());
}
void DisConnectCallbackPtr(std::array<char, 1024> message) {
  printf ("disconnect recv [%s]\n",message.data());
}
#include "MessageDefine.hpp"

int main(int argc, char *argv[]) {
  printf ("11111\n");
  IpcJsonMessage msg;
  msg.msg_type =11;
  msg.msg_id =22;
  msg.src =33;
  msg.dst =44;
  msg.synchronous = true;
  msg.json_string = "this is test!this is test!this is test!\0";

  std::vector<char> data;
  msg.Serialize(data);

  printf ("result : %d %d %d %d %d [%s]\n", msg.msg_type, msg.msg_id, msg.src, msg.dst,  msg.synchronous, msg.json_string.c_str());
  IpcJsonMessage msg2;
  msg2.Deserialize(data);
  printf ("result : %d %d %d %d %d [%s]\n", msg2.msg_type, msg2.msg_id, msg2.src, msg2.dst,  msg2.synchronous, msg2.json_string.c_str());
  printf ("finish");

  return 1;

  /*
  UnixDomainSocketCore core_;
  core_.Initialize(ConnectCallbackPtr, DisConnectCallbackPtr, ReadbackPtr, true);

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
   */
}
