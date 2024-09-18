#include "EchoAssignment.hpp"

#include <cstdio>
#include <cstdlib>
#include <cstring>

#include <arpa/inet.h>

// !IMPORTANT: allowed system calls.
// !DO NOT USE OTHER NETWORK SYSCALLS (send, recv, select, poll, epoll, fork
// etc.)
//  * socket
//  * bind
//  * listen
//  * accept
//  * read
//  * write
//  * close
//  * getsockname
//  * getpeername
// See below for their usage.
// https://github.com/ANLAB-KAIST/KENSv3/wiki/Misc:-External-Resources#linux-manuals

int EchoAssignment::serverMain(const char *bind_ip, int port,
                               const char *server_hello) {
  // Your server code
  // !IMPORTANT: do not use global variables and do not define/use functions
  // !IMPORTANT: for all system calls, when an error happens, your program must
  // return. e.g., if an read() call return -1, return -1 for serverMain.

  //create socket
  int server_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

  //bind
  struct sockaddr_in addr;
  socklen_t len = sizeof(addr);
  memset(&addr, 0, len);

  addr.sin_family = AF_INET;
  inet_pton(AF_INET, bind_ip, &addr.sin_addr);
  addr.sin_port = htons(port);

  if(bind(server_socket, (struct sockaddr *)&addr, len)<0) return -1;

  //listen
  if(listen(server_socket, 1024)<0) return -1;

  while(true){
    //accept
    struct sockaddr_in client_addr;
    socklen_t client_len = sizeof(client_addr);
    memset(&client_addr, 0, client_len);
    int client_fd = accept(server_socket, (struct sockaddr *)&client_addr, &client_len);
    if(client_fd<0){
      close(server_socket);
      return -1;
    }

    //read
    char buffer[1024] = {0};
    int valread = read(client_fd, buffer, 1024);
    if(valread<0){
      close(client_fd);
      close(server_socket);
      return -1;
    }
    buffer[valread-1] = '\0';

    //submit answer
    struct sockaddr_in client_info;
    socklen_t client_info_len = sizeof(client_info);
    if (getpeername(client_fd, (struct sockaddr *)&client_info, &client_info_len) < 0) {
      close(client_fd);
      close(server_socket);
      return -1;
    }

    char client_ip[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &client_info.sin_addr, client_ip, INET_ADDRSTRLEN);
    submitAnswer(client_ip, buffer);

    //write
    std::string response;

    if (strcmp(buffer, "hello") == 0) {
      response = std::string(server_hello);
    } else if(strcmp(buffer, "whoami") == 0){
      response = std::string(client_ip);
    } else if(strcmp(buffer, "whoru") == 0){
      struct sockaddr_in server_info;
      socklen_t server_info_len = sizeof(server_info);
      getsockname(client_fd, (struct sockaddr *)&server_info, &server_info_len);

      char server_ip[INET_ADDRSTRLEN];
      inet_ntop(AF_INET, &server_info.sin_addr, server_ip, INET_ADDRSTRLEN);
      response = std::string(server_ip);
    } else{
      response = std::string(buffer);
    }
    response += "\n";
    
    int valwrite = write(client_fd, response.c_str(), response.size());
    if(valwrite<0){
      close(client_fd);
      close(server_socket);
      return -1;
    }

    //close socket
    close(client_fd);
  }


  close(server_socket);
  return 0;
}

int EchoAssignment::clientMain(const char *server_ip, int port,
                               const char *command) {
  // Your client code
  // !IMPORTANT: do not use global variables and do not define/use functions
  // !IMPORTANT: for all system calls, when an error happens, your program must
  // return. e.g., if an read() call return -1, return -1 for clientMain.

  //create socket
  int client_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

  //connect
  struct sockaddr_in addr;
  socklen_t len = sizeof(addr);
  memset(&addr, 0, len);

  addr.sin_family = AF_INET;
  inet_pton(AF_INET, server_ip, &addr.sin_addr);
  addr.sin_port = htons(port);
  if(connect(client_socket, (struct sockaddr *)&addr, len)<0){
    close(client_socket);
    return -1;
  }

  //write
  std::string request = std::string(command) + "\n";
  int valwrite = write(client_socket, request.c_str(), request.size());
  if(valwrite<0){
    close(client_socket);
    return -1;
  }

  //read
  char buffer[1024] = {0};
  int valread = read(client_socket, buffer, 1024);
  if(valread<0){
    close(client_socket);
    return -1;
  }
  buffer[valread-1] = '\0';

  //submit answer
  submitAnswer(server_ip, buffer);

  //close socket
  close(client_socket);

  return 0;
}

static void print_usage(const char *program) {
  printf("Usage: %s <mode> <ip-address> <port-number> <command/server-hello>\n"
         "Modes:\n  c: client\n  s: server\n"
         "Client commands:\n"
         "  hello : server returns <server-hello>\n"
         "  whoami: server returns <client-ip>\n"
         "  whoru : server returns <server-ip>\n"
         "  others: server echos\n"
         "Note: each command is terminated by newline character (\\n)\n"
         "Examples:\n"
         "  server: %s s 0.0.0.0 9000 hello-client\n"
         "  client: %s c 127.0.0.1 9000 whoami\n",
         program, program, program);
}

int EchoAssignment::Main(int argc, char *argv[]) {

  if (argc == 0)
    return 1;

  if (argc != 5) {
    print_usage(argv[0]);
    return 1;
  }

  int port = atoi(argv[3]);
  if (port == 0) {
    printf("Wrong port number\n");
    print_usage(argv[0]);
  }

  switch (*argv[1]) {
  case 'c':
    return clientMain(argv[2], port, argv[4]);
  case 's':
    return serverMain(argv[2], port, argv[4]);
  default:
    print_usage(argv[0]);
    return 1;
  }
}
