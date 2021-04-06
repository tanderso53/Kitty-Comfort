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
#include <pqxx/pqxx>

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

  struct auth
  {
  public:
    std::string database = "postgres";
    std::string host = "localhost";
    std::string user = "postgres";
    std::string port = "5432";
    std::string password = "";
  };

  class Database
  {
  public:
    typedef std::vector<std::string> svector;
    Database();
    explicit Database(auth* a);
    void init(auth* a);
    void clear();
    int append(const std::string& table, std::istream& data);
    bool tableExists(const std::string& table);
    int createTable(std::string table, svector headers,
		    svector types);

  private:
    auth* _auth = NULL;
    std::string _conString();
  };

  int jsonToCSV(std::istream& jsonstring,
		std::ostream& ofile,
		std::ostream& err = std::cerr)
  {
    try
      {
	Json::Value v;
	jsonstring >> v;
	Json::Value& data = v["data"];
	for (uint i = 0; i < data.size(); i++)
	  {
	    Json::Value& d = data[i];
	    ofile << v["sentmillis"] << ","
		  << d["timemillis"] << ","
		  << d["value"] << ","
		  << d["iswarmedup"] << std::endl;
	  }
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

  Database::Database()
  {
  }

  Database::Database(auth* a)
  {
    init(a);
  }

  void Database::init(auth* a)
  {
    if (_auth) clear();
    _auth = a;
  }

  void Database::clear()
  {
    if (_auth) _auth = NULL;
  }

  int Database::append(const std::string& table, std::istream& data)
  {
    if (!tableExists(table))
      {
	std::string e;
	e += "In Database::append: Table ";
	e += table;
	e += " does not exist.";
	throw std::runtime_error(e);
      }
    pqxx::connection c(_conString());
    pqxx::work w(c);
    std::vector<svector> dv;
    dv.push_back(svector(1));

    // Parse the incoming stream
    while (!data.eof())
      {
	char s = '\0';
	char sep = ',';
	svector& sdv = dv.back();
	while (s != '\n' && !data.eof())
	  {
	    if (s == '\0') data.get(s);
	    if (s != sep && s != '\n' && s != '\0')
	      sdv.back().push_back(s);
	    else if (s == sep) sdv.push_back("");
	    data.get(s);
	  }
	for (auto it = sdv.begin(); it != sdv.end(); it++)
	  std::cerr << *it << '-' << it->length() << ",";
	std::cerr << '-' << sdv.size() << std::endl;
	if (!data.eof()) dv.push_back(svector(1));
	// data.get(s);
      }

    // If an empty element is still on the vector, remove it
    for (auto rit = dv.rbegin();
	 rit->at(0).empty() && rit != dv.rend();
	 rit++)
      dv.pop_back();
    svector& checkstuff = dv.back();
    int i = 0;
    for (auto it = checkstuff.begin(); it != checkstuff.end(); it++)
      {
	std::cerr << i << "." << '\t';
	for (auto itt = it->begin(); itt != it->end(); itt++)
	  std::cerr << int(*itt) << ',';
	std::cerr << '-' << it->size() << std::endl;
      }

    for (auto it = dv.begin(); it != dv.end(); it++)
      {
	svector& sdv = *it;
	// Define the insert query
	std::string query;
	query += "INSERT INTO ";
	query += table;
	query += " VALUES (";
	for (auto itt = sdv.begin(); itt != sdv.end(); itt++)
	  {
	    query += *itt;
	    if (itt != sdv.end() -1) query += ',';
	  }
	query += ")";

	// Perform the query
	w.exec(query);
      }
    w.commit();

    // Return number of rows sent
    return dv.size();
  }

  bool Database::tableExists(const std::string& table)
  {
    pqxx::connection c(_conString());
    pqxx::work w(c);
    std::string query;
    query += "SELECT table_name FROM information_schema.tables ";
    query += "WHERE table_name='";
    query += table.substr(table.find_first_of('.')+1);
    query += "'";
    pqxx::result r = w.exec(query);
    w.commit();
    if (!r.empty()) return 1;
    return 0;
  }

  int Database::createTable(std::string table, svector headers,
			    svector types)
  {
    if (tableExists(table)) return -1;
    pqxx::connection c(_conString());
    pqxx::work w(c);

    // Build query
    std::string query = "CREATE TABLE ";
    query += table;
    query += " (";
    for (uint i = 0; i < headers.size(); i++)
      {
	query += headers[i];
	query += " ";
	query += types[i];
	if (i < headers.size() - 1) query += ",";
      }
    query += ")";

    // Execute table creation
    w.exec(query);
    w.commit();

    // Return 0 on success
    return 0;
  }

  std::string Database::_conString()
  {
    std::string cs = "host=";
    cs += _auth->host;
    cs += " dbname=";
    cs += _auth->database;
    cs += " user=";
    cs += _auth->user;
    if (!_auth->password.empty())
      {
	cs += " password=";
	cs += _auth->password;
      }
    return cs;
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

namespace Cli
{
  class Args
  {
  public:
    typedef std::vector<char> ovector;
    typedef std::vector<std::string> avector;
    typedef std::map<char, std::string> omap;

    /// Default constructor constructs an empty object
    Args();

    /// Initialization constructor parses a set of standard arguments
    /// and places them in containers
    Args(int argc, char** argv);

    /// Initialization constructor parses arguments with some args
    /// set as option arguments
    Args(int argc, char** argv, char* list, size_t lsize);

    /// Deconstructor to remove dynamic list of possible opt args
    ~Args();

    /// Initializes or re-initializes the object using a C standard
    /// argument list
    int init(int argc, char** argv);

    /// Initialize or re-initialize passing a set of options to have
    /// arguments
    int init(int argc, char** argv, char* list, size_t lsize);

    /// Check if option _opt_ was set
    bool option(char opt);

    /// Check if arg _a_ was given
    bool arg(std::string a);

    /// Return the nth argument
    std::string arg(size_t n);

    /// Return number of main arguments given
    size_t size() {return _args.size();};

    /// Check if option _opt_ had an attached argument
    std::string optarg(char opt);

    /// Checks if all argument containers are empty
    bool empty();

    /// Empties all argument containers
    void clear();

  private:
    bool _isopt(char* a);
    bool _findOptArgs(char s);
    ovector _options;
    ovector* _optsWithArgs = NULL;
    avector _args;
    omap _optargs;
  };

  Args::Args()
  {
  }

  Args::Args(int argc, char** argv)
  {
    init(argc, argv);
  }

  Args::Args(int argc, char** argv, char* list, size_t lsize)
  {
    init(argc, argv, list, lsize);
  }

  Args::~Args()
  {
    delete _optsWithArgs;
  }

  int Args::init(int argc, char** argv)
  {
    if (!this->empty()) clear();
    if (argc > 1)
      {
	for (int i = 1; i < argc; i++)
	  {
	    char*& curarg = argv[i];

	    // Assign as option if there is a dash
	    if (_isopt(curarg)) _options.push_back(curarg[1]);
	    else _args.push_back(curarg);
	  }

	// Return the number of args parsed on success
	return argc -1;
      }

    // If no args read, return 0
    return 0;
  }

  int Args::init(int argc, char** argv, char* list, size_t lsize)
  {
    // Clear if the object is already initiallized
    if (!this->empty()) clear();
    _optsWithArgs = new ovector;

    // Add the list of specified optargs
    for (uint i = 0; i < lsize; i++)
      _optsWithArgs->push_back(list[i]);

    // If there are any arguments available, parse them
    if (argc > 1)
      {
	for (int i = 1; i < argc; i++)
	  {
	    char*& curarg = argv[i];

	    // Assign as option if there is a dash
	    if (_isopt(curarg))
	      {
		_options.push_back(curarg[1]);

		// If next argument is not an option, record it as a
		// possible option argument
		if (_findOptArgs(_options.back()))
		    {
		      bool localError = 0;

		      if (i + 1 < argc)
			{
			  char*& nextarg = argv[i+1];

			  if (!_isopt(nextarg))
			    {
			      _optargs.emplace(curarg[1], nextarg);
			      // Consume next argument
			      i ++;
			    }
			  else
			    {
			      localError = 1;
			    }
			}
		      else
			{
			  localError = 1;
			}
		      if (localError)
			{
			  // Exception if no argument
			  std::string e;
			  e += "In Cli ::Args::init: ";
			  e += "Option ";
			  e += _options.back();
			  e += " requires argument.";
			  throw std::runtime_error(e);
			}
		    }
	      }
	    // If it neither matches, it's a regular argument
	    else _args.push_back(curarg);
	  }
	// Return the number of args parsed on success
	return argc -1;
      }

    // If no args read, return 0
    return 0;
  }

  bool Args::option(char opt)
  {
    for (auto it = _options.begin(); it != _options.end(); it++)
      if (*it == opt) return 1;
    return 0;
  }

  bool Args::arg(std::string a)
  {
    for (auto it = _args.begin(); it != _args.end(); it++)
      if (it->compare(a) == 0) return 1;
    return 0;
  }

  std::string Args::arg(size_t n)
  {
    if (n < _args.size())
      return _args[n];
    else
      throw std::runtime_error("In Cli::Args::arg: element out of range");
  }

  std::string Args::optarg(char opt)
  {
    if (_optargs.find(opt) != _optargs.end())
      return _optargs.at(opt);
    else
      throw std::runtime_error("In Cli::Args::optarg: no argument given");
  }

  bool Args::empty()
  {
    return !_options.empty() || !_args.empty() || !_optargs.empty();
  }

  void Args::clear()
  {
    _options.clear();
    _args.clear();
    _optargs.clear();
    if (_optsWithArgs) _optsWithArgs = NULL;
  }

  bool Args::_isopt(char* a)
  {
    if (a[0] == '-' && strlen(a) < 3 ) return 1;
    return 0;
  }

  bool Args::_findOptArgs(char s)
  {
    if (_optsWithArgs)
      {
	for (auto it = _optsWithArgs->begin();
	     it != _optsWithArgs->end();
	     it++)
	  if (*it == s) return 1;
      }
    return 0;
  }
}

int main(int argc, char** argv)
{
  int exitStatus = 0;
  // Use signal to setup handling of signals
  Handler::_outptr = &std::cerr;
  signal(SIGHUP, Handler::signalTerminate);
  signal(SIGINT, Handler::signalBreak);
  signal(SIGTERM, Handler::signalTerminate);
  signal(SIGABRT, Handler::signalTerminate);

  try
    {
      // Parse CLI arguments
      char oaList[] = {'f','h','d','u','P'};
      Cli::Args al(argc, argv, oaList, sizeof(oaList)/sizeof(oaList[0]));

      // If there are arguments given, try to log data
      if (al.size() == 1 && !Handler::terminateP())
	{
	  std::string special = al.arg(0);
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
		      if (al.option('p'))
			std::cout << ss.str() << std::endl;
		      if (al.option('f'))
			{
			  std::ofstream ofile;
			  ofile.open(al.optarg('f').c_str(),
				     std::ofstream::out | std::ofstream::app);
			  Filer::jsonToCSV(ss, ofile);
			}
		      if (al.option('b'))
			{
			  std::stringstream sparsed;
			  Filer::auth a;
			  if (al.option('d')) a.database = al.optarg('d');
			  if (al.option('h')) a.host = al.optarg('h');
			  if (al.option('u')) a.user = al.optarg('u');
			  if (al.option('P')) a.password = al.optarg('P');
			  ss.seekg(0);
			  Filer::jsonToCSV(ss, sparsed);
			  Filer::Database db(&a);
			  std::string tablename = "kittyfiler.ammonia";
			  if (!db.tableExists(tablename))
			    db.createTable(tablename,
					   {std::string("sentmillis"),
					    std::string("timemillis"),
					    std::string("value"),
					    std::string("warmedup")},
					   {std::string("integer"),
					    std::string("integer"),
					    std::string("numeric"),
					    std::string("bool")});
			  db.append(tablename, sparsed);
			}
		      std::stringstream tss;
		      ss.swap(tss);
		    }
		}
	    }
	  catch (std::exception& e)
	    {
	      if (c) c->closePort();
	      std::cout << e.what() << std::endl;
	      exitStatus = -1;
	    }
	}
    }
  catch (std::exception& e)
    {
      std::cerr << e.what() << std::endl;
      exitStatus = -1;
    }

  std::cout << "Exiting" << std::endl;
  return exitStatus;
}
