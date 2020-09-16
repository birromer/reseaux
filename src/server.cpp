#include <iostream>
#include <string.h>
#include <unistd.h>
#include <sstream>

#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>

#define QUEUE_SIZE 16
#define DEFAULT_PORT 32768
#define BUFFER_SIZE 256
typedef int SOCKET;

using namespace std;

int process_connection(SOCKET *sockfd, SOCKET *clifd, struct sockaddr *cli_addr, char buffer[]);
void parse_args(int argc, char* argv[], bool *help, bool *verbose, int *port, char message[]);
void show_help();

int main(int argc, char *argv[])
{
  bool starting_condition = true;
  bool verbose = true;
  bool help = false;
  int port_no = DEFAULT_PORT;
  char buffer[256];
  memset(buffer, '\0', sizeof(char)*BUFFER_SIZE);

  // reads arguments
  parse_args(argc, argv, &help, &verbose, &port_no, buffer);
  for (int i = 0; i < BUFFER_SIZE; i++) {
    if (buffer[i] == '\0') {
      buffer[i+1] = '\n';
      break;
    }
  }

  if (argc < 2) {
    cout << "Too few arguments." << endl;
    show_help();
    return -1;
  }
  else if (help) { 
    show_help();
    return 0;
  }
 
  if (starting_condition) {
    cout << "starting server with message \"" << buffer << "\"" << endl;

    SOCKET sockfd, clifd;

    struct sockaddr_in serv_addr, cli_addr;

    sockfd = socket(AF_INET, SOCK_STREAM, 0);  // start socket with ipv4, tcp, default
    if (sockfd == -1) {
      cout << "Error creating socket" << endl;
      return -1;
    }
    else if (verbose)
      cout << "Success creating socket" << endl;

    const int opt = 1;
    setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)); // set option in order not to fail relaunch

    // configuring the server address structure
    serv_addr.sin_family      = AF_INET;                         // byte order for ipv4
    serv_addr.sin_port        = htons((unsigned short)port_no);  // port number converted from unsigned short to network byte order
    serv_addr.sin_addr.s_addr = INADDR_ANY;                      // current hosts ip

    if (bind(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) == -1) {
      cout << "Problem binding socket." << endl; // binds the socket to the address and port number
      return -1;
    }
    else if (verbose)
      cout << "Success binding socket to address " << inet_ntoa(serv_addr.sin_addr) << " at port " <<htons(serv_addr.sin_port) << endl;

    listen(sockfd, QUEUE_SIZE);  // starts queueing at most QUEUE_SIZE connections
    socklen_t socklen = sizeof(cli_addr);

    int out;

    char web_message[256];
    sprintf(web_message, "HTTP/1.1 200 OK\nContent-Type: text/plain\nContent-Length: %d\n\n%s", strlen(buffer), buffer);

    do {
      clifd = accept(sockfd, (struct sockaddr *)&cli_addr, &socklen);  // takes address of first connection in the queue
    
      if (verbose)
        cout << "received connection from " << inet_ntoa(cli_addr.sin_addr) << " at port " << ntohs(cli_addr.sin_port) << endl;
    
      //send(clifd, buffer, sizeof(buffer), 0); // sends welcome message to new connection
      send(clifd, web_message, sizeof(web_message), 0); // sends welcome message to new connection

      out = process_connection(&sockfd, &clifd, (struct sockaddr *)&cli_addr, buffer);

    } while (out >= 0);

    shutdown(clifd, SHUT_RDWR);
    close(clifd);
    shutdown(sockfd, SHUT_RDWR);
    close(sockfd);
  }

  return 0;
}
int process_connection(SOCKET *sockfd, SOCKET *clifd, struct sockaddr *cli_addr, char buffer[]) {
    memset(buffer, '\0', sizeof(char)*BUFFER_SIZE);
    cout << endl << "Message being typed by the client: " << endl;
    do {

      if (recv(*clifd, buffer, sizeof(char)*BUFFER_SIZE, 0) == -1) {// receives input from connection
        cout << "Error reading message from socket" << endl;

      } else if (buffer[0] == '0' || buffer[0] == ' ') {
        cout << "End of message. Client left." << endl;

        if (buffer[0] == ' ') {
          strcpy(buffer, "-1");
          send(*clifd, buffer, sizeof(char)*BUFFER_SIZE, 0); // sends -1 to he client to indicate the end 
          memset(buffer, '\0', sizeof(char)*BUFFER_SIZE);
          return 0;

        } else {
          cout << "* Quit command received. *" << endl;
          return -1;
        }

      } else {
        cout << buffer << endl;
        send(*clifd, buffer, sizeof(char)*BUFFER_SIZE, 0); // sends welcome message to new connection
        memset(buffer, '\0', sizeof(char)*BUFFER_SIZE);
      }

    } while (true);
}

void parse_args(int argc, char* argv[], bool *help, bool *verbose, int *port, char message[]) {
  int i = 0;
  while(i < argc){
    if(!strcmp(argv[i], "-v") || !strcmp(argv[i], "--verbose")) {
      *verbose = true;
    }
    if(!strcmp(argv[i], "-h") || !strcmp(argv[i], "--help")) {
      *help = true;
    }
    else if(!strcmp(argv[i], "-m") || !strcmp(argv[i], "--message")){
      i++;
      strcat(argv[i],"\0");
      if(i < argc)
        strcpy(message, argv[i]);
    }
    else if(!strcmp(argv[i], "-p") || !strcmp(argv[i], "--port")){
      i++;
      if(i < argc)
        *port= atoi(argv[i]);
    }
    i++;
  }
}

void show_help() {
  cout << "Command line options:" << endl << endl;
  cout << "( -h  | --help )              : Displays help message" << endl;
  cout << "( -v  | --verbose )           : Displays more processing information" << endl;
  cout << "( -p  | --port) <N>           : Port to which bind the socket" << endl;
  cout << "( -m  | --message ) <STRING>  : Welcome message to greet new connection" << endl;
}