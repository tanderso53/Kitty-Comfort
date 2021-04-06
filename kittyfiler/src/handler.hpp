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
