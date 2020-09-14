//
// Created by realbro on 20. 9. 14..
//

#ifndef TESTEPOLLPIPE_SRC_LIB_CONNECTIONMANAGER_H_
#define TESTEPOLLPIPE_SRC_LIB_CONNECTIONMANAGER_H_

#include <map>

class ConnectionManager {
 public :
  static ConnectionManager& GetInstance();

  bool Add(int product_code, int socket_fd);
  bool Remove(int socket_fd);

 private :
  ConnectionManager();
  ~ConnectionManager();

  std::map<int, int> client_socket_list_;
};

#endif //TESTEPOLLPIPE_SRC_LIB_CONNECTIONMANAGER_H_
