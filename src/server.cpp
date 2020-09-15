#include <iostream>
#include <string>

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

//    int sockfd = socket(domain, SOCK_STREAM, 0);

  }

  return 0;
}
