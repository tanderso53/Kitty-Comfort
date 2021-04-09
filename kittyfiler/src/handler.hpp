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

// handler.hpp

#include <iostream>
#include <signal.h>
#include <cstring>

#ifndef handler_hpp
#define handler_hpp

/// Class to set properties within signal handlers
namespace Handler
{
  bool _breakS = 0;
  bool _terminateP = 0;
  std::ostream* _outptr;
  void _print(int sig)
  {
    if (_outptr)
      *_outptr << "Received signal "
	       << strsignal(sig) << std::endl;
  }

  void signalTerminate(int sig)
  {
    _breakS = 1;
    _terminateP = 1;
    _print(sig);
  }

  void signalBreak(int sig)
  {
    _breakS = 1;
    _print(sig);
  }

  bool breakS() {return _breakS;};

  bool terminateP() {return _terminateP;};

  void reset()
  {
    _breakS = 0;
    _terminateP = 0;
  }
}

#endif
