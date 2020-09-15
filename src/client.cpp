#include <iostream>
#include <string>

int main(int argc, char *argv[])
{
  bool starting_condition = true;
  // reads first argument to see if it is server os client
  if (argc < 2) {
    //std::cout << "Too few arguments, usage: `./client`" << std::endl;
  } else if (argc >= 2 && argv[1][0] == '-' && (argv[1][1] == 'H' || argv[1][1] == 'h')) {
    std::cout << "Help:\nUsage: `./client`" << std::endl;
  }

  if (starting_condition) {
    std::cout << "starting client" << std::endl;
  }


  return 0;
}
