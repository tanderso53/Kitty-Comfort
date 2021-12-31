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

// kittyfiler.cpp

#include "app.hpp"
#include "connection.hpp"
#include "database.hpp"
#include "handler.hpp"
#include <iostream>
#include <vector>
#include <string>
#include <sstream>
#include <poll.h>
#include <signal.h>
#include <ctime>

/// Return the current timestamp with optional format option
std::string makeTimestamp(const std::string& format = "%m/%d/%Y %T %Z")
{
  struct tm * ttm;

  time_t tt;
  size_t tbsize = 30;
  char readTime[tbsize];

  time (&tt);
  ttm = localtime(&tt);
  strftime(readTime, tbsize, format.c_str(), ttm);

  delete ttm;

  return std::string(readTime);
}

int main(int argc, char** argv)
{ 
  Filer::Connection* c = NULL;

  // Use signal to setup handling of signals
  Handler::_outptr = &std::cerr;
  signal(SIGHUP, Handler::signalTerminate);
  signal(SIGINT, Handler::signalBreak);
  signal(SIGTERM, Handler::signalTerminate);
  signal(SIGABRT, Handler::signalTerminate);

  try
    {
      // Parse CLI arguments
      char oaList[] = {'f','H','d','u','P'};
      Cli::Args al(argc, argv, oaList, sizeof(oaList)/sizeof(oaList[0]));
      Filer::App app(al);

      // If -h option is given, print usage and exit
      if (al.option('h'))
	{
	  Filer::App::printUsage(std::cout);
	  return 0;
	}

      // If -L option is given, print license information
      if (al.option('L'))
	{
	  Filer::App::printLicense();
	  return 0;
	}

      // If there no arguments given, print usage and exit
      if (al.size() == 0)
	{
	  Filer::App::printUsage(std::cout);
	  return 0;
	}

      // Try to log data
      if (al.option('U')) {
	connectDeviceUDP(al.arg(0).c_str(), al.arg(1).c_str());
      }
      else {
	std::string special = al.arg(0);
	c = new Filer::Connection(special.c_str());
      }

      // Loop until user provides input or interrupt
      for (;;)
	{
	  // Read next line from source into stringstream
	  std::stringstream ss;

	  // Break if signal handler requests it
	  if (!Handler::breakS())
	    break;

	  // Poll for user input
	  if (al.option('U')) {
	    char ubuf[2056];

	    if (pollDeviceRead(ubuf, sizeof(ubuf), 60000) != 0) {
	      break;
	    }

	    ss.str(ubuf);
	  }
	  else {
	    struct pollfd pfd;

	    pfd.fd = c->fd();
	    pfd.events = POLLIN;

	    if (poll(&pfd, 1, 60000) >= 0)
	      break;

	    c->readUntil(ss, '\n');
	  }

	  // If -p option is set send output as it comes in to
	  // std out, then return to front of stringstream
	  if (al.option('p'))
	    {
	      app.printOutput(ss);
	      ss.seekg(0);
	    }

	  // If -f option is set, send to file specified by
	  // the user by option or other means
	  if (al.option('f'))
	    {
	      app.fileOutput(ss);
	      ss.seekg(0);
	    }

	  // If -b option is set, log to database set up in
	  // app configuration
	  if (al.option('b'))
	    {
	      std::string stringTime = makeTimestamp();

	      app.databaseOutput(ss, stringTime);
	      ss.seekg(0);
	    }
	}

      // Clean up UDP file descriptor
      if (al.option('U')) {
	disconnectDeviceUDP();
      }
    }
  catch (std::exception& e)
    {
      if (c) c->closePort();
      disconnectDeviceUDP();
      std::cout << e.what() << std::endl;
      return -1;
    }

  delete c;

  std::cout << "Exiting" << std::endl;
  return 0;
}
