// ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// +                                                                +
// +                          KITTYCOMFORT                          +
// +               Microcontroller project to maintain              +
// +                     optimal housecat comfort                   +
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

// datastructure.cpp

#include <datastructure.h>

void DataStructure::toggle(bool& reg)
{
	if (reg) reg = 0;
	else reg = 1;
}

String DataStructure::jsonBuildHeader()
{
	String output = "{\"project\": \"kittycomfort\",";
	output += "\"sentmillis\": ";
	output += millis();
	return output;
}

String DataStructure::jsonBuildFooter(String eot = String('\n'))
{
	String output = "\"EOT\": true}";
	output += eot;
	return output;
}


DataStructure::DataStructure(size_t vsize)
:_vsize(vsize), _size(0)
{
	init();
}

DataStructure::DataStructure(const DataStructure& ds)
:_vsize(ds._vsize), _size(ds._size), _values(new double[_vsize]),
	_timemillises(new unsigned long[_vsize]),
	_warmedups(new bool[_vsize])
{
	for (size_t i = 0; i < ds._size; i++)
		{
			_values[i] = ds._values[i];
			_timemillises[i] = ds._timemillises[i];
			_warmedups[i] = ds._warmedups[i];
		}
}

DataStructure::DataStructure(DataStructure&& ds)
:_vsize(ds.vsize()), _size(ds.size()), _values(ds._values),
	_timemillises(ds._timemillises), _warmedups(ds._warmedups)
{
	ds._vsize = 0;
	ds._size = 0;
	ds._values = NULL;
	ds._timemillises = NULL;
	ds._warmedups = NULL;
}

DataStructure::~DataStructure()
{
	delete [] _values;
	delete [] _timemillises;
	delete [] _warmedups;
}

void DataStructure::init()
{
	_values = new double[_vsize];
	_timemillises = new unsigned long[_vsize];
	_warmedups = new bool[_vsize];
}

void DataStructure::clear()
{
	delete [] _values;
	delete [] _timemillises;
	delete [] _warmedups;
  _size = 0;
	init();
}

inline double DataStructure::value(unsigned int element)
{
	if (element < vsize()) return _values[element];
	else return 0;
}

inline unsigned long DataStructure::timemillis(unsigned int element)
{
	if (element < vsize()) return _timemillises[element];
	else return 0;
}

inline bool DataStructure::warmedup(unsigned int element)
{
	if (element < vsize()) return _warmedups[element];
	else return 0;
}

void DataStructure::addData(double value, unsigned long tmillis,
														bool warm)
{
	if (size() < vsize())
		{
			_values[size()] = value;
			_timemillises[size()] = tmillis;
			_warmedups[size()] = warm;
      _size++;
		}
}

String DataStructure::jsonDataString()
{
	String output = "\"data\": [";
	for (size_t i = 0; i < size(); i++)
		{
			output += "{\"value\": ";
			output += value(i);
			output += ",\"timemillis\": ";
			output += timemillis(i);
			output += ",\"iswarmedup\": ";
      if (warmedup(i)) output += "true";
			else output += "false";
			output += "}";
			if (i < size() -1) output += ",";
		}
	output += "]";
	return output;
}

String DataStructure::jsonFullString()
{
	String output = jsonBuildHeader();
	output += ",";
	output += jsonDataString();
	output += ",";
	output += jsonBuildFooter();
	return output;
}
