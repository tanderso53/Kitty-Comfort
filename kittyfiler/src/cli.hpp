#include <iostream>
#include <vector>
#include <map>

#ifndef cli_hpp
#define cli_hpp

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
}

#endif
