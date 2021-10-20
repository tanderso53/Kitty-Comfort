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

//#ifdef ARDUINO_APOLLO3_SFE_ARTEMIS_NANO
#define SF_ARTEMIS
//#endif

#ifndef SF_ARTEMIS
#include <SoftwareSerial.h>
#endif
#ifdef SF_ARTEMIS
#include <ArduinoBLE.h>
#endif
#include <SparkFunBME280.h>
#include <SparkFunCCS811.h>
#include <Wire.h>

#define CCS811_ADDR 0x5B
#define INT_PIN 2
#define WAK_PIN 3

#ifdef SF_ARTEMIS

// Main BLE service id
#define KC_CHAR_BLE_SID "19B10000-E8F2-537E-4C6C-D104768A1214"

// List of all the things we will measure
#define KC_CHAR_AMMONIA "19B10000-E8F2-537E-4C6C-D104768A1215"
#define KC_CHAR_TEMP "19B10000-E8F2-537E-4C6C-D104768A1216"
#define KC_CHAR_HUMIDITY "19B10000-E8F2-537E-4C6C-D104768A1217"
#define KC_CHAR_CO2 "19B10000-E8F2-537E-4C6C-D104768A1218"

// BLE Battery Service
BLEService btService(KC_CHAR_BLE_SID);

// BLE Custom UUID for ammonia
BLEDoubleCharacteristic ammoniaChar[] = {
	BLEDoubleCharacteristic(KC_CHAR_AMMONIA,
													BLERead | BLENotify),
	BLEDoubleCharacteristic(KC_CHAR_TEMP,
													BLERead | BLENotify),
	BLEDoubleCharacteristic(KC_CHAR_HUMIDITY,
													BLERead | BLENotify),
	BLEDoubleCharacteristic(KC_CHAR_CO2,
													BLERead | BLENotify)
}; // remote clients will be able to get notifications if this characteristic changes

// Create descriptor for characteristic
BLEDescriptor ammoniaDesc[] = {
	BLEDescriptor("2901", "Ammonia (counts)"),
	BLEDescriptor("2901", "Temp (deg C)"),
	BLEDescriptor("2901", "Humidity (%)"),
	BLEDescriptor("2901", "CO2 (ppm)")
};

template <typename T>
bool updateBT(T _valuebuf, BLEDoubleCharacteristic& _blech)
{
  /* unsigned int btlen = sizeof(value) / sizeof(char); */
  /* if (btlen > 512) */
  /* { */
  /*   btlen = 512; */
  /* } */
  /* byte valuebuf[btlen]; */
  /* for (int i =0; i < btlen; i++) */
  /* { */
  /*   valuebuf[i] = value[i]; */

  /*   if (valuebuf[i] == '\0') */
  /*   { */
  /*     btlen = i + 1; */
  /*     break; */
  /*   } */
  /* } */
  /* Read the current voltage level on the A0 analog input pin.
     This is used here to simulate the charge level of a battery.
  */

	double _newvalue = (double) _valuebuf;
	double _lastvalue = _blech.value();

	if (((long) (_newvalue * 100) != ((long) (_lastvalue * 100)))) {
		_blech.writeValue(_newvalue);  // and update the battery level characteristic
		return true;
	}

	return false;
}

int startBLE()
{
	if (!BLE.begin()) {
		return -1;
  }

  /* Set a local name for the BLE device
     This name will appear in advertising packets
     and can be used by remote devices to identify this BLE device
     The name can be changed but maybe be truncated based on space left in advertisement packet
  */
  BLE.setLocalName("Kitty Comfort");
  BLE.setAdvertisedService(btService); // add the service UUID

	// Loop through all characteristics, adding them to the service
	for (int i = 0; i < sizeof(ammoniaChar)/sizeof(BLEDoubleCharacteristic); ++i) {
		btService.addCharacteristic(ammoniaChar[i]);

		// Assign Descriptor
		ammoniaChar[i].addDescriptor(ammoniaDesc[i]);

		// Set initial value
		ammoniaChar[i].writeValue(0.0);
	}

	BLE.addService(btService); // Add the battery service
  //ammoniaChar.writeValue('0'); // set initial value for this characteristic

  /* Start advertising BLE.  It will start continuously transmitting BLE
     advertising packets and will be visible to remote BLE central devices
     until it receives a new connection */

  // start advertising
  BLE.advertise();
	return 0;
}
#endif

class AmmoniaSensor
{
 private:
	int _apin = A3;
	int _dpin = 13;
	int _slope = 1;
	int _intercept = 0;
	int _mult = 1;
	bool _init = 0;
	unsigned long _lastRead = 0;
	unsigned long _lastCheck = 0;
	unsigned long _warmUpTime = 86400000; // 24 hrs

 protected:
	void updateTimer(unsigned long& timer);

 public:
	AmmoniaSensor();
	AmmoniaSensor(int apin, int dpin);
	AmmoniaSensor(int apin, int dpin, int warmUptTime);
	void init();
	uint16_t readCounts();
	bool checkAlarm();
	void calibrate(int highCount, int highValue, int lowCount,
								 int lowValue, int mult);
	double readValue();
	unsigned long lastRead() {return _lastRead;}
	unsigned long lastCheck() {return _lastCheck;}
	bool isWarmedUp();
};

AmmoniaSensor::AmmoniaSensor()
{
}

AmmoniaSensor::AmmoniaSensor(int apin, int dpin)
:_apin(apin), _dpin(dpin)
{
}

AmmoniaSensor::AmmoniaSensor(int apin, int dpin, int warmUpTime)
:_apin(apin), _dpin(dpin), _warmUpTime(warmUpTime)
{
}

void AmmoniaSensor::init()
{
	pinMode(_dpin, INPUT);
	_init = 1;
}

uint16_t AmmoniaSensor::readCounts()
{
	if (_init)
		{
			updateTimer(_lastRead);
			return analogRead(A3);
		}
	else return -1;
}

void AmmoniaSensor::updateTimer(unsigned long& timer)
{
	timer = millis();
}

bool AmmoniaSensor::checkAlarm()
{
	if (_init)
		{
			updateTimer(_lastCheck);
			return digitalRead(_dpin);
		}
	else return 1;
}

void AmmoniaSensor::calibrate(int highCount, int highValue,
															int lowCount, int lowValue, int mult)
{
	highValue = highValue * mult;
	lowValue = lowValue * mult;
	_slope =  (highValue - lowValue) / (highCount - lowCount);
	_intercept = highValue - _slope * highCount;
	_mult = mult;
}

double AmmoniaSensor::readValue()
{
	if (_init)
		return (readCounts() * _slope - _intercept) / _mult;
	else
		return 0.0;
}

bool AmmoniaSensor::isWarmedUp()
{
	if (millis() > _warmUpTime) return 1;
	else return 0;
}

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

// Globals
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
#ifndef SF_ARTEMIS
SoftwareSerial bt(btrx, bttx);
#endif
CCS811 myCCS811(CCS811_ADDR);
BME280 myBME280;

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

  #ifndef SF_ARTEMIS
	// Set up bluetooth
	bt.begin(115200);
  #endif

	#ifdef SF_ARTEMIS
	// Start BLE module
	startBLE();
	#endif
}

void loop()
{
	// Check if there is a command in the serial buffer
	if (!checkCommand(Serial)); //checkCommand(bt);

	if (progMode)
		{
			Serial.println("RAWR");
      toggle(progMode);
		}
						
	// Read and output data
	else
		{
			if (readData())
				{
					outputJson(Serial);

#ifndef SF_ARTEMIS
					outputJson(bt);
#endif

#ifdef SF_ARTEMIS
          BLEDevice central = BLE.central();

					//String ammbuff("ammonia: " + String(as.readCounts(), DEC)
          //+ ",time: " + String(millis(), DEC));

					updateBT(as.readCounts(), ammoniaChar[0]); // Ammonia
					updateBT(myBME280.readTempC(), ammoniaChar[1]); // Temp deg C
					updateBT(myBME280.readFloatHumidity(), ammoniaChar[2]); // Humidity
					updateBT(myCCS811.getCO2(), ammoniaChar[3]); // CO2

          //while (central.connected())
          //{
          //}
#endif
				}
		}
}
