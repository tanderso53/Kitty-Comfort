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

#include "connection.hpp"
#include "database.hpp"
#include "handler.hpp"
#include "cli.hpp"
#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <sstream>
#include <poll.h>
#include <signal.h>
#include <ctime>
#include <filesystem>

void printUsage(std::ostream& out)
{
  // Build usage form
  Cli::Usage u;

  u.addApp("kittyfiler");
  std::string desc;
  desc = "Command line program to read ammonia data from serial port";
  desc += "and log to stdout, a file, or a database.";
  u.addDescription(desc);
  u.addUseCase({'p'},
	       {std::make_pair('f',"<filename>")},
	       {"<special>"});
  u.addUseCase({'p','b'},
	       {std::make_pair('f', "<filename>"),
		std::make_pair('H', "<host>"),
		std::make_pair('d', "<database>"),
		std::make_pair('u', "<user>"),
		std::make_pair('P', "<password>")},
	       {"<special>"});
  u.addUseCase({'h','L'}, {}, {});
  u.addOption('p', "print raw json to stdout");
  u.addOption('b', "send data to database. Requires connection options");
  u.addOption('f', "write data as CSV to file <filename>. must be absolute");
  u.addOption('H', "Hostname for database, only useful with -b");
  u.addOption('d', "Database name for database, only useful with -b");
  u.addOption('u', "User name for database, only useful with -b");
  u.addOption('P', "Password for database, only useful with -b");
  u.addOption('h', "Print this help message, then exit");
  u.addOption('L', "Print licensing information, then exit");

  // Print out the usage to given output
  u.print(out);
}

void printLicense(std::ostream& out = std::cout)
{
  std::ifstream lfile;
  std::filesystem::path p = "../../LICENSE";
  lfile.open(std::filesystem::canonical(p));

  while (!lfile.eof())
    {
      char license[128];
      lfile.getline(license, 128);
      out << license << std::endl;
    }

  out << std::endl;
  lfile.close();
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
      char oaList[] = {'f','H','d','u','P'};
      Cli::Args al(argc, argv, oaList, sizeof(oaList)/sizeof(oaList[0]));

      // If -h option is given, print usage and exit
      if (al.option('h'))
	{
	  printUsage(std::cout);
	  return 0;
	}

      // If -L option is given, print license information
      if (al.option('L'))
	{
	  printLicense();
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
		      if (al.option('p'))
			std::cout << ss.str() << std::endl;
		      if (al.option('f'))
			{
			  std::ofstream ofile;
			  ofile.open(al.optarg('f').c_str(),
				     std::ofstream::out | std::ofstream::app);
			  Filer::Conversion::jsonToCSV(ss, ofile);
			}
		      if (al.option('b'))
			{
			  std::stringstream sparsed;
			  Filer::auth a;
			  if (al.option('d')) a.database = al.optarg('d');
			  if (al.option('H')) a.host = al.optarg('H');
			  if (al.option('u')) a.user = al.optarg('u');
			  if (al.option('P')) a.password = al.optarg('P');
			  ss.seekg(0);
			  Filer::Conversion::jsonToCSV(ss, sparsed, stringTime);
			  Filer::Database db(&a);
			  std::string tablename = "kittyfiler.ammonia";

			  Filer::Database::svector headers =
			    {std::string("sentmillis"),
			     std::string("timemillis"),
			     std::string("value"),
			     std::string("warmedup"),
			     std::string("readtime")};
			  Filer::Database::svector types =
			    {std::string("bigint"),
			     std::string("bigint"),
			     std::string("numeric"),
			     std::string("bool"),
			     std::string("timestamptz")};
			  if (!db.tableExists(tablename))
			    db.createTable(tablename, headers, types);
			  db.append(tablename, sparsed, headers);
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
	  printUsage(std::cout);
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
