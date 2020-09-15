#include <iostream>
#include <string>

#include <sys/socket.h>
#include <sys/types.h>
#include <sys/select.h>
#include <netinet/in.h>

#define QUEUE_SIZE 16
#define PORT_NUMBER 32768
typedef int SOCKET;

using namespace std;

int main(int argc, char *argv[])
{
  bool starting_condition = true;
  string welcome_message;

  // reads first argument to see if it is server os client
  if (argc < 2) {
    cout << "Too few arguments, usage: `./server -m \"welcome message\"`" << endl;
  } else if (argc >= 2 && argv[1][0] == '-' && (argv[1][1] == 'H' || argv[1][1] == 'h')) {
    cout << "Help:\nUsage: `./server\"welcome message\"`" << endl;
  } else if (argc >= 3 && argv[1][0] == '-' && (argv[1][1] == 'M' || argv[1][1] == 'm')) {
    welcome_message = argv[2];  // if it is a server stores the welcome message
  }

  if (starting_condition) {
    cout << "starting server with message \"" << welcome_message << "\"" << endl;

    SOCKET sockfd, clifd;
    int port_no = PORT_NUMBER;
    struct sockaddr_in serv_addr;

    sockfd = socket(AF_INET, SOCK_STREAM, 0);  // start socket with ipv4, tcp, default
    if (sockfd == -1){
      cout << "Error creating socket" << endl;
    }; 

    // configuring the server address structure
    serv_addr.sin_family      = AF_INET;                         // byte order for ipv4
    serv_addr.sin_port        = htons((unsigned short)port_no);  // port number converted from unsigned short to network byte order
    serv_addr.sin_addr.s_addr = INADDR_ANY;                      // current hosts ip

    if (bind(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) == -1) {
      cout << "Problem binding socket." << endl;
    } // binds the socket to the address and port number

    



  }

  return 0;
}
