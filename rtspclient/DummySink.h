#ifndef __DUMMYSINK_
#define __DUMMYSINK_

#include "BasicUsageEnvironment.hh"
#include "liveMedia.hh"
#include "utils.h"

// Define a data sink (a subclass of "MediaSink") to receive the data for each subsession (i.e., each audio or video
// 'substream'). In practice, this might be a class (or a chain of classes) that decodes and then renders the incoming
// audio or video. Or it might be a "FileSink", for outputting the received data into a file (as is done by the
// "openRTSP" application). In this example code, however, we define a simple 'dummy' sink that receives incoming data,
// but does nothing with it.



class DLL_LOCAL DummySink : public MediaSink {
public:
  static DummySink *createNew(UsageEnvironment &env,
                              MediaSubsession &subsession,  // identifies the kind of data that's being received
                              char const *streamId = NULL); // identifies the stream itself (optional)
  void SetCallback(OutBufferFunc callback,int channel_id);
private:
  DummySink(UsageEnvironment &env, MediaSubsession &subsession, char const *streamId);
  // called only by "createNew()"
  virtual ~DummySink();

  static void afterGettingFrame(void *clientData, unsigned frameSize, unsigned numTruncatedBytes,
                                struct timeval presentationTime, unsigned durationInMicroseconds);
  void afterGettingFrame(unsigned frameSize, unsigned numTruncatedBytes, struct timeval presentationTime,
                         unsigned durationInMicroseconds);

private:
  // redefined virtual functions:
  virtual Boolean continuePlaying();

private: 
  OutBufferFunc callback_; //输出回调
  int channel_id_;
  u_int8_t *fReceiveBuffer;
  MediaSubsession &fSubsession;
  char *fStreamId;
};

#endif //__DUMMYSINK_
