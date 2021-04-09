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

// connection.cpp

#include "connection.hpp"
#include <cstring>
#include <unistd.h>
#include <fcntl.h>
#include <json/json.h>

namespace Filer
{
  int Conversion::jsonToCSV(std::istream& jsonstring,
			    std::ostream& ofile,
			    std::ostream& err)
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

  int Conversion::jsonToCSV(std::istream& jsonstring,
			    std::ostream& ofile,
			    const std::string& readtime,
			    std::ostream& err)
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
		  << d["iswarmedup"] << ","
		  << readtime << std::endl;
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
}
