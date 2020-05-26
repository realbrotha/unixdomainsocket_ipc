//#include "UnixDomainSocketInterface.h"

#include "UnixDomainSocketCore.h"

int main(int argc, char *argv[]) {
  UnixDomainSocketCore core_;
  core_.Initialize(true);
}
