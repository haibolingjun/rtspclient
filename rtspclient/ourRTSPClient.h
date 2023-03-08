#ifndef __OURRTSPCLIENT_
#define __OURRTSPCLIENT_

#include "StreamClientState.h"
#include "utils.h"

// If you're streaming just a single stream (i.e., just from a single URL, once), then you can define and use just a
// single "StreamClientState" structure, as a global variable in your application.  However, because - in this demo
// application - we're showing how to play multiple streams, concurrently, we can't do that.  Instead, we have to have a
// separate "StreamClientState" structure for each "RTSPClient".  To do this, we subclass "RTSPClient", and add a
// "StreamClientState" field to the subclass:

class DLL_LOCAL ourRTSPClient : public RTSPClient {
public:
  static ourRTSPClient *createNew(UsageEnvironment &env, char const *rtspURL, OutBufferFunc callback, int channel_id,
                                  int verbosityLevel = 0, char const *applicationName = NULL,
                                  portNumBits tunnelOverHTTPPortNum = 0);

public:
  void ShutdownHandler(unsigned int err);
  unsigned int videoWidth();
  unsigned int videoHeight();
  unsigned int videoFps();
protected:
  ourRTSPClient(UsageEnvironment &env, char const *rtspURL, OutBufferFunc callback, int channel_id, int verbosityLevel,
                char const *applicationName, portNumBits tunnelOverHTTPPortNum);
  // called only by createNew();
  virtual ~ourRTSPClient();

public:
  StreamClientState scs;
  OutBufferFunc callback_;
  ErrorCallbackFunc shutdown_callback_;
  int channel_id_;
  std::mutex session_lock_;
  unsigned int video_width_;
  unsigned int video_height_;
  unsigned int video_fps_;
};

#endif //__OURRTSPCLIENT_