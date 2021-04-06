#include "cli.hpp"
#include <cstring>

namespace Cli
{
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
