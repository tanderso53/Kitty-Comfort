// Microcontroller project to maintain
// optimal housecat comfort

#include <SoftwareSerial.h>
class AmmoniaSensor
{
 private:
	int _apin = A0;
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
			return analogRead(_apin);
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

String jsonBuildHeader()
{
	String output = "{\"project\": \"kittycomfort\",";
	output += "\"sentmillis\": ";
	output += millis();
	return output;
}

String jsonBuildFooter(String eot = String('\n'))
{
	String output = "\"EOT\": true}";
	output += eot;
	return output;
}

// Place data stored in memory here for asyncronous upload
class DataStructure
{
 private:
	size_t _vsize;
	size_t _size;
	double* _values;
	unsigned long* _timemillises;
	bool* _warmedups;

 public:
	DataStructure(size_t vsize);
	DataStructure(const DataStructure& ds);
	DataStructure(DataStructure&& ds);
	~DataStructure();
	void init();
	void clear();
	size_t vsize() {return _vsize;}
	size_t size() {return _size;}
	double value(unsigned int element);
	unsigned long timemillis(unsigned int element);
	bool warmedup(unsigned int element);
	void addData(double value, unsigned long tmillis, bool warm);
	String jsonDataString();
	String jsonFullString();
};

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
	for (int i = 0; i < ds._size; i++)
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
	for (int i = 0; i < size(); i++)
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
	if (Serial.available())
		{
			if (Serial.readBytesUntil('\n', readBuffer, 8) > 3)
				{
					char progCmd[] = {'p', 'r', 'o'};
					if (matchCommand(progCmd, readBuffer))
						toggle(progMode);
				}
		}

	if (progMode)
		{
			Serial.println("RAWR");
      toggle(progMode);
		}
						
	// Read and output data
	else
		{
			if (millis() > as.lastRead() + readoutDelay)
				{
					// If time to read, read the ammonia
					ds.addData(as.readCounts(), millis(), as.isWarmedUp());
				}

			if (millis() > lastTransmit + transmitDelay)
				{
					// Print Json to output
					if (Serial) Serial.println(ds.jsonFullString());
					if (bt) bt.print(ds.jsonFullString());
					ds.clear();
          lastTransmit = millis();
				}
		}
}
