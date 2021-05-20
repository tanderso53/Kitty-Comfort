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

// kittycomfort.ino

#include <SoftwareSerial.h>

class EchoFun
{
 public:
	virtual void operator () (char c);
};

class btecho
:public EchoFun
{
 public:
	btecho(SoftwareSerial& bt);
	void operator () (char c);
 private:
	SoftwareSerial& _bt;
};

class secho
:public EchoFun
{
 public:
	void operator () (char c);
};

class Command
{
 public:
	Command();
	Command(byte bsize);
	bool checkCommand(Stream& s);
	bool matchCommand(char* match, char* source);
	bool matchCommand(const char* match);
	int readCom(Stream& s, EchoFun& fun);
	bool writeCom(Stream& s, const char* c);
	byte bsize() {return _bsize;};
	bool progMode() {return _progMode;};
	operator bool () {return _readBuffer;};
	void clear();

 private:
	byte _bsize = 8;
	char* _readBuffer = NULL;
	bool _progMode = 0;
};

Command::Command()
{
	_readBuffer = new char[_bsize];
}

Command::Command(byte bsize)
:_bsize(bsize)
{
	_readBuffer = new char[_bsize];
}

int Command::readCom(Stream& s, EchoFun& fun)
{
	if (s.available() && this)
		{
			int ctr = 0;
			clear();
			for (uint8_t i = 0; i < _bsize; i++)
				{
					ctr++;
					char& c = _readBuffer[i];
					c = s.read();
					fun(c);
					if (c == '\n')
						{
							c = '\0';
							break;
						}
				}
			return ctr;
		}
	return -1;
}

bool Command::checkCommand(Stream& s)
{
	if (s.available() && this)
		{
			if (s.readBytesUntil('\n', _readBuffer, _bsize) > 3)
				{
					char progCmd[] = {'p', 'r', 'o'};
					if (matchCommand(progCmd, _readBuffer))
						toggle(_progMode);
					return true;
				}
		}
	return false;
}

bool Command::matchCommand(char* match, char* source)
{
	for (int i = 0; i < 3; i++)
		if (source[i] != match[i])
				return 0;
	return 1;
}

// Globals
const int apin = A0;
const int dpin = 13;
const int btrx = 4;
const int bttx = 6;
bool progMode = 0;
char readBuffer[8];
//char transBuffer[64];
unsigned long readoutDelay = 10000; // in ms
unsigned long transmitDelay = 60000; // in ms
unsigned long lastTransmit = 0;
AmmoniaSensor as(apin, dpin);
SoftwareSerial bt(btrx, bttx);
DataStructure ds(1 + transmitDelay / readoutDelay);
Command cli;

void readData()
{
	if (millis() > as.lastRead() + readoutDelay)
		{
			// If time to read, read the ammonia
			ds.addData(as.readCounts(), millis(), as.isWarmedUp());
		}
}

void outputData(Stream& s)
{
	if (millis() > lastTransmit + transmitDelay)
		{
			// Print Json to output
			s.print(ds.jsonFullString());
			ds.clear();
			lastTransmit = millis();
		}
}

void setup()
{
	as.init();
	Serial.begin(9600);
	Serial.println(as.readCounts());

	// Set up bluetooth
	bt.begin(115200);
}

void loop()
{
	// Check if there is a command in the serial buffer
	if (!cli.checkCommand(Serial)) cli.checkCommand(bt);

	if (cli.progMode())
		{
			secho se();
			Serial.println("RAWR");
			if (cli.readCom(Serial, se()) > 0)
				{
					if (cli.matchCommand("size"))
						{
						}

					if (cli.matchCommand("exit"))
						{
						}

					cli.clear();
				}
		}

	// Read and output data
	else
		{
			readData();
			outputData(bt);
			outputData(Serial);
		}
}
