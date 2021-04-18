// ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// +                                                                +
// +                           KITTYFILER                           +
// +                 A program to file cat data into                +
// +                      a Postgresql database                     +
// +                                                                +
// ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

// Copyright 2021 Tyler J. Anderson

// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions
// are met:

// 1. Redistributions of source code must retain the above copyright
// notice, this list of conditions and the following disclaimer.

// 2. Redistributions in binary form must reproduce the above copyright
// notice, this list of conditions and the following disclaimer in the
// documentation and/or other materials provided with the distribution.

// 3. Neither the name of the copyright holder nor the names of its
// contributors may be used to endorse or promote products derived from
// this software without specific prior written permission.

// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
// "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
// LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
// A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
// HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
// SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
// LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
// DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
// THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
// (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

// cli.cpp

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

  Args::Args(const Args& other)
    :_options(other._options), _optsWithArgs(NULL),
     _args(other._args), _optargs(other._optargs)
  {
    if (other._optsWithArgs)
      _optsWithArgs = new ovector(*other._optsWithArgs);
  }

  Args::Args(Args&& other)
    :_options(std::move(other._options)),
     _optsWithArgs(other._optsWithArgs),
     _args(std::move(other._args)),
     _optargs(std::move(other._optargs))
  {
    other._optsWithArgs = NULL;
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

  Usage::Usage()
  {
  }

  void Usage::addApp(const std::string& appname)
  {
    _app = appname;
  }

  void Usage::addDescription(const std::string& desc)
  {
    this->_description = desc;
  }

  void Usage::addUseCase(const cvector& opts, const csvector& optargs,
			 const svector& args)
  {
    _usecases.push_back(std::make_tuple(opts, optargs, args));
  }

  void Usage::addOption(char opt, const std::string& description)
  {
    _arglist.push_back(std::make_pair(opt, description));
  }

  void Usage::print(std::ostream& out)
  {
    out << "Usage:" << std::endl << std::endl;
    for (auto it = _usecases.begin(); it != _usecases.end(); it++)
      {
	out << _app << " ";
	cvector& o = std::get<0>(*it);
	csvector& oa = std::get<1>(*it);
	svector& a = std::get<2>(*it);

	for (auto itt = o.begin(); itt != o.end(); itt++)
	  out << '-' << *itt << " ";

	for (auto itt = oa.begin(); itt != oa.end(); itt++)
	  out << '-' << itt->first << " " << itt->second << " ";

	for (auto itt = a.begin(); itt != a.end(); itt++)
	  out << *itt << " ";

	out << std::endl << std::endl;
      }

    out << _description << std::endl << std::endl;

    for (auto it = _arglist.begin(); it != _arglist.end(); it++)
      {
	size_t win = _termwidth;
	out << '\t' << '-' << it->first << '\t';
	win = win - 2 * _tab;
	for (auto itt = it->second.begin(); itt != it->second.end();
	     itt++)
	  {
	    if (win == 0)
	      {
		out << std::endl << '\t' << '\t';
		win = _termwidth;
	      }
	    out << *itt;
	    win--;
	  }
	out << std::endl << std::endl;
      }
  }
}
