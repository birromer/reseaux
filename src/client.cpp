#include <iostream>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <ncurses.h>

#include <sys/socket.h>
#include <sys/types.h>
#include <sys/select.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>

#define QUEUE_SIZE 16
#define DEFAULT_PORT 32768
#define BUFFER_SIZE 256
typedef int SOCKET;

using namespace std;

void parse_args(int argc, char* argv[], bool *help, bool *verbose, int *port, char hostname[]);
void show_help();

int main(int argc, char *argv[])
{
  bool starting_condition = true;
  bool verbose = true;
  bool help = false;
  int port_no = DEFAULT_PORT;
  char hostname[256];

  // reads arguments
  parse_args(argc, argv, &help, &verbose, &port_no, hostname);

  if (argc < 2) { 
    cout << "Too few arguments, at least specify the hostname." << endl;
    show_help();
    return -1;
  }
  else if (help) { 
    show_help();
    return 0;
  }
 
  if (starting_condition) {
    cout << "Starting client for " << hostname << ":" << port_no << endl;

    SOCKET sockfd;
    char buffer[256];
    memset(buffer, '\0', sizeof(char)*BUFFER_SIZE);

    struct sockaddr_in serv_addr;

    sockfd = socket(AF_INET, SOCK_STREAM, 0);  // start socket with ipv4, tcp, default
    if (sockfd == -1) {
      cout << "Error creating socket" << endl;
      return -1;
    } else if (verbose)
      cout << "Success creating socket" << endl;

    // configuring the server address structure
    serv_addr.sin_family      = AF_INET;                         // byte order for ipv4
    serv_addr.sin_port        = htons((unsigned short)port_no);  // port number converted from unsigned short to network byte order
    serv_addr.sin_addr.s_addr = inet_addr("127.0.0.1");                      // current hosts ip

    if (connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) == -1)
      cout << "Error connecting to host" << endl;
    else if(verbose)
      cout << "received connection from " << inet_ntoa(serv_addr.sin_addr) << " at port " << ntohs(serv_addr.sin_port) << endl;
    
    char letra[1];
    system("stty raw");
    do {
      letra[0] = getchar();
      send(sockfd, letra, sizeof(letra), 0); // sends welcome message to new connection
      printf("%c", letra[0]);
    } while (letra[0] != ' ');
    system("stty cooked");

    send(sockfd, buffer, sizeof(buffer), 0); // sends welcome message to new connection

    memset(buffer, '\0', sizeof(char)*BUFFER_SIZE);
    if (recv(sockfd, buffer, sizeof(buffer), 0) == -1) // receives input from connection
      cout << "Error reading message from socket" << endl;
    else if (verbose)
      cout << "Received message: " << buffer << endl;

    send(sockfd, buffer, sizeof(buffer), 0); // sends welcome message to new connection

    shutdown(sockfd, SHUT_RDWR);
    close(sockfd);
  }

  return 0;
}

void parse_args(int argc, char* argv[], bool *help, bool *verbose, int *port, char hostname[]) {
  int i = 0;
  while(i < argc){
    if(!strcmp(argv[i], "-v") || !strcmp(argv[i], "--verbose")) {
      *verbose = true;
    }
    if(!strcmp(argv[i], "-h") || !strcmp(argv[i], "--help")) {
      *help = true;
    }
    else if(!strcmp(argv[i], "-n") || !strcmp(argv[i], "--hostname")){
      i++;
      strcat(argv[i],"\0");
      if(i < argc)
        strcpy(hostname, argv[i]);
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
  cout << "( -n  | --hostname) <STRING>  : Hostname of the server to connect" << endl;
  cout << "( -p  | --port) <N>           : Port to which connect in the informed hostname" << endl;
}