#include <iostream>
#include <string.h>
#include <unistd.h>
#include <sstream>

#include <sys/socket.h>
#include <netdb.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>

#define QUEUE_SIZE 16
#define DEFAULT_PORT 32768
#define BUFFER_SIZE 256
typedef int SOCKET;

using namespace std;

bool test_http(char buffer[]);
int process_connection(SOCKET *sockfd, SOCKET *clifd, struct sockaddr *cli_addr, char buffer[]);
void gen_personal_welcome(char welcome_message[], char personalized_message[], char hostname[]);
void parse_args(int argc, char* argv[], bool *help, bool *verbose, int *port, char message[]);
void show_help();

int main(int argc, char *argv[])
{
  bool starting_condition = true;
  bool verbose = true;
  bool help = false;
  int port_no = DEFAULT_PORT;
  char welcome_message[BUFFER_SIZE];
  char custom_message[BUFFER_SIZE];
  char web_message[2048];
  char buffer[BUFFER_SIZE];

  memset(buffer, '\0', sizeof(char)*BUFFER_SIZE);          // initializes both buffers to null char
  memset(welcome_message, '\0', sizeof(char)*BUFFER_SIZE);

  // reads arguments
  parse_args(argc, argv, &help, &verbose, &port_no, welcome_message);

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
    cout << "starting server with message \"" << welcome_message << "\"" << endl;

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
    char hostname[BUFFER_SIZE];
    char service[BUFFER_SIZE];

    do {
      clifd = accept(sockfd, (struct sockaddr *)&cli_addr, &socklen);  // takes address of first connection in the queue
      getnameinfo((const sockaddr*)&cli_addr, sizeof(cli_addr), hostname, sizeof(hostname), service, sizeof(service), 0);
      gen_personal_welcome(welcome_message, custom_message, hostname);
    
      if (verbose)
        cout << "Received connection from " << inet_ntoa(cli_addr.sin_addr) << " at port " << ntohs(cli_addr.sin_port) << endl;

      if (recv(clifd, buffer, sizeof(char)*BUFFER_SIZE, 0) == -1) {// receives input from connection
        cout << "Error reading message from socket" << endl;
        return -1;

      } else if (test_http(buffer)) {
        cout << "Connecting to web client." << endl;
        sprintf(web_message, "HTTP/1.1 200 OK\nContent-Type: text/html\nContent-Length: %d\n\n<!DOCTYPE html><html><head><title>%s</title></head><body>%s</body></html>", (int)strlen(welcome_message)*2+69, welcome_message, welcome_message);
        send(clifd, custom_message, sizeof(custom_message), 0); // sends welcome message the web page 
        cout << "Disconnecting from web client." << endl;
        out = 1;

      } else {
        send(clifd, custom_message, sizeof(custom_message), 0); // sends welcome message to new connection
        out = process_connection(&sockfd, &clifd, (struct sockaddr *)&cli_addr, buffer);
      }


    } while (out >= 0);

    cout << "Shutting down and closing server." << endl;
    shutdown(clifd, SHUT_RDWR);
    close(clifd);
    shutdown(sockfd, SHUT_RDWR);
    close(sockfd);
  }

  return 0;
}

bool test_http(char buffer[]) {
  int i;
  for (i = 0; i < strlen(buffer); i++) {
    if (buffer[i] == '1' && buffer[i-1] == '.' && buffer[i-2] == '1' && buffer[i-3])
      return true;
  }
  return false;
}

int process_connection(SOCKET *sockfd, SOCKET *clifd, struct sockaddr *cli_addr, char buffer[]) {
    memset(buffer, '\0', sizeof(char)*BUFFER_SIZE);
    cout << endl << "Message being typed by the client: " << endl;
    do {

      // receives input from connection
      if (recv(*clifd, buffer, sizeof(char)*BUFFER_SIZE, 0) == -1) { // error if received value is -1
        cout << "Error reading message from socket" << endl;
        return -1;

      } else if (buffer[0] == '0' || buffer[0] == ' ') { // '0' and ' ' are the escape cases
        cout << "End of message. Client left." << endl;

        if (buffer[0] == ' ') {  // escape case for client to leave connection
          memset(buffer, '\0', sizeof(char)*BUFFER_SIZE);  
          strcpy(buffer, "-1");
          send(*clifd, buffer, sizeof(char)*BUFFER_SIZE, 0); // sends -1 to he client to indicate  that it can disconnect
          return 0;  // return 0 for server to know that it should continue accepting connections  

        } else {
          cout << "* Quit command received. *" << endl;
          strcpy(buffer, "-1");
          send(*clifd, buffer, sizeof(char)*BUFFER_SIZE, 0); // sends -1 to he client to indicate the end 
          return -1;  // return -1 for server to know that it should close
        }

      } else {                  // if it is none of the escape cases
        cout << buffer << endl; // just informs of the received character and sends it back
        send(*clifd, buffer, sizeof(char)*BUFFER_SIZE, 0); // sends received char back to client 
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

void gen_personal_welcome(char welcome_message[], char personalized_message[], char hostname[]) {
  memset(personalized_message, '\0', sizeof(char)*BUFFER_SIZE);
  strcpy(personalized_message, welcome_message);
  strcat(personalized_message, "\nYour ip is ");
  strcat(personalized_message, hostname);
}