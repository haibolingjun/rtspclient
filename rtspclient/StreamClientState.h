#ifndef __STREAMCLIENTSTATE_
#define __STREAMCLIENTSTATE_

#include "BasicUsageEnvironment.hh"
#include "liveMedia.hh"
#include "utils.h"

// Define a class to hold per-stream state that we maintain throughout each stream's lifetime:

class DLL_LOCAL StreamClientState {
public:
  StreamClientState();
  virtual ~StreamClientState();

public:
  MediaSubsessionIterator* iter;
  MediaSession* session;
  MediaSubsession* subsession;
  TaskToken streamTimerTask;
  double duration;
};

#endif //__STREAMCLIENTSTATE_