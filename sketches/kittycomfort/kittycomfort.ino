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

#include "arduino_secrets.h"
#include "ammoniasensor.h"

#include <WiFiNINA.h>
#include <WiFiUdp.h>
#include <SparkFunBME280.h>
#include <SparkFunCCS811.h>
#include <Wire.h>

#define CCS811_ADDR 0x5B
#define INT_PIN 2
#define WAK_PIN 3
#define LISTEN_PORT 2391

// Status Register Definitions
#define REG_STATUS_OK 0x0001
#define REG_STATUS_WIFI 0x0002
#define REG_STATUS_UDP 0x0004

bool matchCommand(char* match, char* source)
{
	for (int i = 0; i < 3; i++)
		if (source[i] != match[i])
				return 0;
	return 1;
}

void toggle(bool& reg)
{
	if (reg) reg = 0;
	else reg = 1;
}

// Globals -- General
uint16_t regStatus = 0;
const int apin = A3;
const int dpin = 13;
const int btrx = 4;
const int bttx = 6;
byte loopStatus = 0;
bool progMode = 0;
char readBuffer[8];
//char transBuffer[64];
unsigned long readoutDelay = 10000; // in ms
unsigned long transmitDelay = 60000; // in ms
unsigned long lastTransmit = 0;
AmmoniaSensor as(apin, dpin);
CCS811 myCCS811(CCS811_ADDR);
BME280 myBME280;

// Globals -- WiFi
char ssid[] = "SECRET_SSID";
char pass[] = "SECRET_PASS";
unsigned long wifiCheckDelay = 10000; // in ms
unsigned long wifiLastCheck = 0; // in ms
WiFiUDP Udp;
//char udpPacketBuffer[256];
//char udpSendBuffer[512];

bool readData()
{
	if (millis() > as.lastRead() + readoutDelay)
		{
			return true;
		}
	return false;
}

bool checkCommand(Stream& s)
{
	if (s.available())
		{
			if (s.readBytesUntil('\n', readBuffer, 8) > 3)
				{
					char progCmd[] = {'p', 'r', 'o'};
					if (matchCommand(progCmd, readBuffer))
						toggle(progMode);
					return true;
				}
		}
	return false;
}

/// Print Status Object
void outputStatus(Stream& s)
{
	s.print("\"status\": {");
	s.print("\"isWarmedUp\": ");
	if (as.isWarmedUp()) s.print("true");
	else s.print("false");
	s.print(", \"CCS811\": \"");
	if (myCCS811.checkForStatusError())
		s.print(myCCS811.getErrorRegister());
	else s.print("ok");
	s.print("\", \"sentmillis\": ");
	s.print(millis());
	s.print("}");
}

/// Print Json object that will contain data arrays
void outputDataHeader(Stream& s)
{
	s.print("\"data\": [");
}

/// Print Closing bracket on the data object
void outputDataFooter(Stream& s)
{
	s.print("]");
}

/// Print out data as Json array of keys
template <class T>
void outputData(const char* name, T data, Stream& s,
								const char* unit)
{
	// Print Json to output
	s.print("{\"name\": \"");
	s.print(name);
	s.print("\", \"value\": ");
	s.print(data);
	s.print(", \"timemillis\": ");
	s.print(millis());
	s.print(", \"unit\": \"");
	s.print(unit);
	s.print("\"}");
}

void outputJson(Stream& s)
{
	// Begin json output
	s.print("{");

	// Output status object
	outputStatus(s);

  // Separator between status and data
  s.print(", ");

	// Output data object
	outputDataHeader(s);
	outputData("ammonia", as.readCounts(), s, "counts");
	s.print(", ");
	outputData("temp", myBME280.readTempC(), s, "degC");
	s.print(", ");
	outputData("pressure", myBME280.readFloatPressure(),
						 s, "pa");
	s.print(", ");
	outputData("altitude",
						 myBME280.readFloatAltitudeMeters(),
						 s, "m");
	s.print(", ");
	outputData("rh", myBME280.readFloatHumidity(),
						 s, "%");
	if (myCCS811.dataAvailable())
		{
			myCCS811.setEnvironmentalData(myBME280.readFloatHumidity(),
																		myBME280.readTempC());
			myCCS811.readAlgorithmResults();
			s.print(", ");
			outputData("co2", myCCS811.getCO2(), s, "ppm");
			s.print(", ");
			outputData("tvoc", myCCS811.getTVOC(), s, "ppb");
		}
	outputDataFooter(s);

	// End json object
	s.print("}");
	s.print('\n');
}

void startUDP()
{
	regStatus = regStatus | REG_STATUS_WIFI;
	Serial.println("WiFi connected");
	Serial.print("IP: ");
	Serial.println(WiFi.localIP());
	Udp.begin(LISTEN_PORT);
}

void setup()
{
	as.init();
	Serial.begin(9600);
	// Serial.println(as.readCounts());

  // Setup WAK and INT pins
  pinMode(WAK_PIN, OUTPUT);
  digitalWrite(WAK_PIN, LOW);
  pinMode(INT_PIN, INPUT);

	// Begin I2C
	Wire.begin();

  // This begins the CCS811 sensor and prints error status of
  // .beginWithStatus()
  CCS811Core::CCS811_Status_e returnCode = myCCS811.beginWithStatus();

  // Initialize BME280
  // For I2C, enable the following and disable the SPI section
  myBME280.settings.commInterface = I2C_MODE;
  myBME280.settings.I2CAddress = 0x77;
  myBME280.settings.runMode = 3; //Normal mode
  myBME280.settings.tStandby = 0;
  myBME280.settings.filter = 4;
  myBME280.settings.tempOverSample = 5;
  myBME280.settings.pressOverSample = 5;
  myBME280.settings.humidOverSample = 5;

  // Calling .begin() causes the settings to be loaded Make sure
	// sensor had enough time to turn on. BME280 requires 2ms to start
	// up.
  delay(10);
  myBME280.begin();

	// WiFi initiallization
  unsigned long stmillis = millis();
	for (unsigned long i = 0; i < stmillis + wifiCheckDelay; i = millis())
	{
		if (WiFi.begin(ssid, pass) == WL_CONNECTED)
		{
			startUDP();
			break;
		}
	}
}

void loop()
{
	// Check if there is a command in the serial buffer
	if (!checkCommand(Serial)); //checkCommand(bt);

	bool rd = readData();

	if (progMode)
		{
			Serial.println("RAWR");
      toggle(progMode);
		}
						
	// Read and output data
	else
		{
			if (rd)
				{
					outputJson(Serial);
				}
		}

	// Manage WiFi interface
	if (WiFi.status() != WL_CONNECTED && (regStatus & REG_STATUS_WIFI) == REG_STATUS_WIFI)
	{
		regStatus == regStatus & (~REG_STATUS_WIFI) & (~REG_STATUS_UDP);
		Serial.println("WiFi Disconnected");
	}

	if ((regStatus & REG_STATUS_WIFI) == REG_STATUS_WIFI)
	{
		int psize = Udp.parsePacket();

		if (psize)
		{
			regStatus = regStatus | REG_STATUS_UDP;
			Udp.flush();
		}

		if ((regStatus & REG_STATUS_UDP) == REG_STATUS_UDP && rd)
		{
			Udp.beginPacket(Udp.remoteIP(), Udp.remotePort());
			outputJson(Udp);
		}
	}
	else if (millis() > wifiCheckDelay + wifiLastCheck)
	{
		if (WiFi.begin(ssid, pass) == WL_CONNECTED)
		{
			startUDP();
		}

		wifiLastCheck = millis();
	}
}
