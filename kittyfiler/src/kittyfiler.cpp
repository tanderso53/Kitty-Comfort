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
		      time_t tt;
		      struct tm * ttm;
		      size_t tbsize = 30;
		      char readTime[tbsize];
		      time (&tt);
		      ttm = localtime(&tt);
		      strftime(readTime, tbsize, "%m/%d/%Y %T %Z", ttm);
		      std::string stringTime = readTime;
		      ss.seekg(0);
		      if (al.option('p'))
			{
			  app.printOutput(ss);
			  ss.seekg(0);
			}
		      if (al.option('f'))
			{
			  app.fileOutput(ss);
			  ss.seekg(0);
			}
		      if (al.option('b'))
			{
			  app.databaseOutput(ss, stringTime);
			  ss.seekg(0);
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
      else
	{
	  Filer::App::printUsage(std::cout);
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
