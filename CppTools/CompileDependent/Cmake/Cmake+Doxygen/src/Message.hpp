#pragma once
#include <iosfwd>
#include <string>
/**
* \brief Pretty nice class.
* \details This class is used to demonstrate a number of section commands.
* \author John Doe
* \author Jan Doe
* \version 4.1a
* \date 1990-2011
* \pre First initialize the system.
* \bug Not all memory is freed when deleting an object of this class.
* \warning Improper use can crash your application
* \copyright GNU Public License.
*/

class Message {
public:
  /*! \brief Constructor from a string
  * \param[in] m a message
  */
  Message(const std::string &m) : message_(m) {}
  /*! \brief Constructor from a character array
  * \param[in] m a message
  */
  Message(const char * m): message_(std:: string(m)){}
  friend std::ostream &operator<<(std::ostream &os, Message &obj) {
    return obj.printObject(os);
  }
private:
  /*! The message to be forwarded to screen */
  std::string message_;
  /*! \brief Function to forward message to screen
  * \param[in, out] os output stream
  */
  std::ostream &printObject(std::ostream &os);
};