#include <iostream>
#include <termios.h>
#include <json/json.h>

namespace Filer
{
  int jsonToCSV(std::istream& jsonstring,
		std::ostream& ofile,
		std::ostream& err = std::cerr);

  class Connection
  {
  private:
    const char* _special;
    int* _fd;
    struct termios _portSettings;
    void _setDefaultOptions();
    int _lastError = 0;

  protected:
    void getPortConfig();

  public:
    explicit Connection(const char* special,
			speed_t baud = B9600);
    Connection(Connection& o);
    ~Connection();
    void init(speed_t baud = B9600);
    void openPort();
    void closePort();
    bool isOpen();
    int fd();
    void configureBaud(speed_t baud = B9600);
    // Read port to ostream until the given null-terminated characters
    // Returns 0 if EOF is reached before characters
    int readUntil(std::ostream& buffer, char eor);
    std::string getErrorString();
  };
}
