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

// 2. Redistributions in binary form must reproduce the above
// copyright notice, this list of conditions and the following
// disclaimer in the documentation and/or other materials provided
// with the distribution.

// 3. Neither the name of the copyright holder nor the names of its
// contributors may be used to endorse or promote products derived
// from this software without specific prior written permission.

// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
// "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
// LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
// FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
// COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT,
// INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
// (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
// SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
// HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
// STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
// ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED
// OF THE POSSIBILITY OF SUCH DAMAGE.

// app.hpp
#ifndef APP_HPP
#define APP_HPP

#include "cli.hpp"
#include <iostream>

namespace Filer
{
  /// Class to execute common filer program options
  class App
  {
  public:
    /// Print usage information to stream
    static void printUsage(std::ostream& out);

    /// Print license information to stream
    static void printLicense(std::ostream& out = std::cout);

    /// Default constructor with members un-initialized
    App();

    /// Initialization constructor where arglist will be copied
    explicit App(const Cli::Args& argList);

    /// Initialization constructor where arglist is moved
    explicit App(Cli::Args&& argList);

    /// Copy constructor
    App(const App& other);

    /// Move constructor
    App(App&& other);

    /// Destructor to take down applist
    ~App();

    /// Print Json output string to stdout
    int printOutput(std::istream& instream);

    /// Print CSV format to file specified in arglist
    int fileOutput(std::istream& instream);

    /// Parse json string then upload to database
    int databaseOutput(std::istream& instream,
		       const std::string& stringTime);

    /// Get reference to arglist
    Cli::Args& argList();

    /// Initialize object by copying arglist
    void init(const Cli::Args& argList);

    /// Initialize object by moving arglist
    void init(Cli::Args&& argList);

    /// Check if object is initialized
    bool isInit();

  private:
    Cli::Args* _argList = NULL;
  };
}

#endif
