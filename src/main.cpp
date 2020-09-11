#include <iostream>
#include <string>

int main(int argc, char *argv[])
{
  char type = 0; // 0 if server, 1 if client
  std::string welcome_message;

  // reads first argument to see if it is server os client
  if (argc < 2) {
    std::cout << "Too few arguments, usage: `./tcp -C` or `./tcp -S <welcome message>`" << std::endl;
  } else if (argc >= 2 && argv[1][0] == '-' && (argv[1][1] == 'C' || argv[1][1] == 'c')) {
    type = 'c';
  } else if (argc >= 3 && argv[1][0] == '-' && (argv[1][1] == 'S' || argv[1][1] == 's')) {
    type = 's';
    welcome_message = argv[2];  // if it is a server stores the welcome message
  }

  if (type == 'c') {
    std::cout << "starting client" << std::endl;
  } else if (type == 's') {
    std::cout << "starting server with message " << welcome_message << std::endl;
  }


  return 0;
}
