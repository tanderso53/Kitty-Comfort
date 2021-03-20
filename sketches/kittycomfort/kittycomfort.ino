// Microcontroller project to maintain
// optimal housecat comfort

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
	unsigned long _warmUpTime = 200000;

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

// Globals
int apin = A0;
int dpin = 13;
unsigned long readoutDelay = 500; // in ms
AmmoniaSensor as(apin, dpin);

void setup()
{
	as.init();
	Serial.begin(9600);
	Serial.println(as.readCounts());
}

void loop()
{
	if (millis() > as.lastRead() + readoutDelay)
		{
			Serial.println(as.readCounts());
		}
}
