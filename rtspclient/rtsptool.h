#ifndef __RTSPTOOL_
#define __RTSPTOOL_

#include "BasicUsageEnvironment.hh"
#include "liveMedia.hh"
#include "utils.h"

// Forward function definitions:

// RTSP 'response handlers':
DLL_LOCAL void continueAfterDESCRIBE(RTSPClient* rtspClient, int resultCode, char* resultString);
DLL_LOCAL void continueAfterSETUP(RTSPClient* rtspClient, int resultCode, char* resultString);
DLL_LOCAL void continueAfterPLAY(RTSPClient* rtspClient, int resultCode, char* resultString);

// Other event handler functions:
DLL_LOCAL void subsessionAfterPlaying(void* clientData); // called when a stream's subsession (e.g., audio or video substream) ends
DLL_LOCAL void subsessionByeHandler(void* clientData); // called when a RTCP "BYE" is received for a subsession
DLL_LOCAL void streamTimerHandler(void* clientData);
  // called at the end of a stream's expected duration (if the stream has not already signaled its end using a RTCP "BYE")


// Used to iterate through each stream's 'subsessions', setting up each one:
DLL_LOCAL void setupNextSubsession(RTSPClient* rtspClient);

// Used to shut down and close a stream (including its "RTSPClient" object):
DLL_LOCAL void shutdownStream(RTSPClient* rtspClient,unsigned int error_no = 0);

// A function that outputs a string that identifies each stream (for debugging output).  Modify this if you wish:
DLL_LOCAL UsageEnvironment& operator<<(UsageEnvironment& env, const RTSPClient& rtspClient); 

// A function that outputs a string that identifies each subsession (for debugging output).  Modify this if you wish:
DLL_LOCAL UsageEnvironment& operator<<(UsageEnvironment& env, const MediaSubsession& subsession);

DLL_LOCAL void usage(UsageEnvironment& env, char const* progName);


#endif //__RTSPTOOL_