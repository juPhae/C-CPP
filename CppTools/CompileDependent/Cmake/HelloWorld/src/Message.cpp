#include "Message.hpp"
#include <iostream>
#include <string>
#include "dbg.hpp"
std::ostream &Message::printObject(std::ostream &os) {
  os << "This is my very nice message: " << std::endl;
  os << message_;
  std::cout<<" " << std::endl;
  dbg(message_);
  return os;
}



