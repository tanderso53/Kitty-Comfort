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

// database.cpp

#include "database.hpp"
#include <pqxx/pqxx>

namespace Filer
{
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
	if (!data.eof()) dv.push_back(svector(1));
	// data.get(s);
      }

    // If an empty element is still on the vector, remove it
    if (dv.back().at(0).empty()) dv.pop_back();

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
	    query += "'";
	    query += *itt;
	    query += "'";
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
