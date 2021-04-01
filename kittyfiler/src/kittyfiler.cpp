#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <sstream>
#include <json/json.h>
#include <termios.h>
#include <unistd.h>
#include <sys/types.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <poll.h>
#include <signal.h>

namespace Filer
{
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

  int jsonToCSV(std::istream& jsonstring,
		std::string filename,
		std::ostream& err = std::cerr)
  {
    try
      {
	Json::Value v;
	std::ofstream ofile;
	jsonstring >> v;
	Json::Value& data = v["data"];
	ofile.open(filename.c_str(),
		   std::ofstream::out | std::ofstream::app);
	for (uint i = 0; i < data.size(); i++)
	  {
	    Json::Value& d = data[i];
	    ofile << v["sentmillis"] << ","
		  << d["timemillis"] << ","
		  << d["value"] << ","
		  << d["iswarmedup"] << std::endl;
	  }
	ofile.close();
      }
    catch (Json::Exception& e)
      {
	err << "Failed to parse Json string with "
	    << e.what() << std::endl;
	return -1;
      }
    return 0;
  }

  void Connection::_setDefaultOptions()
  {
    if (_fd)
      {
	_portSettings.c_cflag &= ~CRTSCTS;
	_portSettings.c_cflag |= CREAD | CLOCAL;
	_portSettings.c_iflag &= ~(IXON | IXOFF | IXANY);
	_portSettings.c_iflag &= ~(ICANON | ECHO | ECHOE | ISIG);
	tcsetattr(fd(), TCSANOW, &_portSettings);
      }
    else throw std::runtime_error("Failed to config, port not open");
  }

  Connection::Connection(const char* special,
			 speed_t baud)
    : _special(special)
  {
    init(baud);
  }

  Connection::Connection(Connection& o)
    : _special(o._special), _fd(new int),
      _portSettings(o._portSettings)
  {
    *_fd = o.fd();;
  }

  Connection::~Connection()
  {
    if (_fd) closePort();
    delete _fd;
  }

  void Connection::init(speed_t baud)
  {
    if (!_fd)
      {
	openPort();
      }
    else
      {
	closePort();
	openPort();
      }
    configureBaud(baud);
    _setDefaultOptions();
  }

  // Copy current configuration of port to struct
  void Connection::getPortConfig()
  {
    if (!_fd) tcgetattr(*_fd, &_portSettings);
    else throw std::runtime_error("File not open");
  }

  // Open serial port
  void Connection::openPort()
  {
    if (!_fd) _fd = new int;
    else throw std::runtime_error("File already open");
    *_fd = open(_special, O_RDWR | O_NOCTTY);
    if (*_fd < 0)
      {
	_fd = NULL;
	throw std::runtime_error("Could not connect to port");
      }
  }

  void Connection::closePort()
  {
    if (_fd)
      {
	close(*_fd);
	_fd = NULL;
      }
  }

  inline bool Connection::isOpen()
  {
    if (_fd) return 1;
    else return 0;
  }

  inline int Connection::fd()
  {
    if (_fd) return *_fd;
    else return -1;
  }

  // Set the connection speed
  void Connection::configureBaud(speed_t baud)
  {
    if (_fd)
      {
	if (cfsetispeed(&_portSettings, baud) < 0
	    || cfsetospeed(&_portSettings, baud) < 0)
	  throw std::runtime_error("Could not adjust baud setting");
      }
    else throw std::runtime_error("File not open");
  }

  int Connection::readUntil(std::ostream& buffer, char eor)
  {
    if (_fd)
      {
	char b[1] = {'\0'};
	int counter = 0;
	ssize_t code = 1;

	while (b[0] != eor || code > 0)
	  {
	    code = read(fd(), b, 1);

	    if (code < 0)
	      {
		_lastError = errno;
		std::string e = "Read error: ";
		e += getErrorString();
		throw std::runtime_error(e);
	      }
	    if (code > 0)
	      {
		buffer << b[0];
		counter++;
	      }
	  }
	return counter;
      }
    else throw std::runtime_error("File not open");
  }

  std::string Connection::getErrorString()
  {
    return std::string(strerror(_lastError));
  }
}

/// Class to set properties within signal handlers
namespace Handler
{
  bool _breakS = 0;
  bool _terminateP = 0;
  std::ostream* _outptr;
  void _print(int sig)
  {
    if (_outptr)
      *_outptr << "Received signal "
	       << strsignal(sig) << std::endl;
  }

  void signalTerminate(int sig)
  {
    _breakS = 1;
    _terminateP = 1;
    _print(sig);
  }

  void signalBreak(int sig)
  {
    _breakS = 1;
    _print(sig);
  }

  bool breakS() {return _breakS;};

  bool terminateP() {return _terminateP;};

  void reset()
  {
    _breakS = 0;
    _terminateP = 0;
  }
}

int main(int argc, char** argv)
{
  // Use signal to setup handling of signals
  Handler::_outptr = &std::cerr;
  signal(SIGHUP, Handler::signalTerminate);
  signal(SIGINT, Handler::signalBreak);
  signal(SIGTERM, Handler::signalTerminate);

  // If there are arguments given, try to log data
  if (argc > 1)
    {
      std::string special = argv[1];
      Filer::Connection* c = NULL;
      try
	{
	  c = new Filer::Connection(special.c_str());
	  std::stringstream ss;

	  struct pollfd pfd;
	  pfd.fd = c->fd();
	  pfd.events = POLLIN;

	  long pollStat = 0;
	  while (pollStat >= 0 && !Handler::breakS())
	    {
	      pollStat = poll(&pfd, 1, 60000);
	      if (pollStat > 0)
		{
		  c->readUntil(ss, '\n');
		  std::cout << ss.str() << std::endl;
		  if (argc > 2) Filer::jsonToCSV(ss, argv[2]);
		}
	    }
	}
      catch (std::exception& e)
	{
	  if (c) c->closePort();
	  std::cout << e.what() << std::endl;
	  return -1;
	}

      std::cout << "Exiting" << std::endl;
      return 0;
    }
}
