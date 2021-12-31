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

// app.cpp
#include "app.hpp"
#include "database.hpp"
#include "connection.hpp"
#include <filesystem>
#include <fstream>
#include <sstream>

void Filer::App::printUsage(std::ostream& out)
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
  u.addUseCase({'U', 'p','b'},
	       {std::make_pair('f', "<filename>"),
		std::make_pair('H', "<host>"),
		std::make_pair('d', "<database>"),
		std::make_pair('u', "<user>"),
		std::make_pair('P', "<password>")},
	       {"<address>", "<port>"});
  u.addUseCase({'h','L'}, {}, {});
  u.addOption('U', "Obtain data from specified UDP port");
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

void Filer::App::printLicense(std::ostream& out)
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

Filer::App::App()
{
}

Filer::App::App(const Cli::Args& argList)
{
  init(argList);
}

Filer::App::App(Cli::Args&& argList)
{
  init(argList);
}

Filer::App::App(const App& other)
  :_argList(new Cli::Args(*other._argList))
{
}

Filer::App::App(App&& other)
  :_argList(other._argList)
{
  _argList = NULL;
}

Filer::App::~App()
{
  delete _argList;
}

void Filer::App::init(const Cli::Args& argList)
{
  _argList = new Cli::Args(argList);
}

void Filer::App::init(Cli::Args&& argList)
{
  _argList = new Cli::Args(argList);
}

Cli::Args& Filer::App::argList()
{
  return *_argList;
}

bool Filer::App::isInit()
{
  return _argList;
}

int Filer::App::printOutput(std::istream& instream)
{
  while (instream.good())
    {
      char c;
      instream.get(c);
      std::cout << c;
      instream.peek();
    }

  if (instream.bad())
    {
      std::string err = "In Filer::App::printOutput: ";
      err += "Stream passed to printOutput is mangled";
      throw std::runtime_error(err);
    }

  return 0;
}

int Filer::App::fileOutput(std::istream& instream)
{
  std::ofstream ofile;
  ofile.open(argList().optarg('f').c_str(),
	     std::ofstream::out | std::ofstream::app);
  Filer::Conversion::jsonToCSV(instream, ofile);
  return 0;
}

int Filer::App::databaseOutput(std::istream& instream,
			const std::string& stringTime)
{
  std::stringstream sparsed;
  Filer::auth a;
  if (argList().option('d')) a.database = argList().optarg('d');
  if (argList().option('H')) a.host = argList().optarg('H');
  if (argList().option('u')) a.user = argList().optarg('u');
  if (argList().option('P')) a.password = argList().optarg('P');
  instream.seekg(0);
  Filer::Conversion::jsonToCSV(instream, sparsed, stringTime);
  Filer::Database db(&a);
  std::string tablename = "kittyfiler.sensorreadings";

  Filer::Database::svector headers =
    {std::string("sentmillis"),
     std::string("timemillis"),
     std::string("name"),
     std::string("value"),
     std::string("unit"),
     std::string("warmedup"),
     std::string("readtime")};

  Filer::Database::svector types =
    {std::string("bigint"),
     std::string("bigint"),
     std::string("text"),
     std::string("numeric"),
     std::string("text"),
     std::string("bool"),
     std::string("timestamptz")};

  if (!db.tableExists(tablename))
    db.createTable(tablename, headers, types);

  db.append(tablename, sparsed, headers);

  return 0;
}
