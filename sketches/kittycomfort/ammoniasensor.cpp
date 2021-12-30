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

// ammoniasensor.cpp

#include "ammoniasensor.h"

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
