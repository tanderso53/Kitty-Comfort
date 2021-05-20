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

// ammoniasensor.h

#ifndef AMMONIASENSOR_H
#define AMMONIASENSOR_H

#include <Arduino.h>

namespace Ammonia
{
	
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
}

#endif
