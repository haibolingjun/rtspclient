#ifndef __RTSPSESSION_
#define __RTSPSESSION_

#include <string>
#include <pthread.h>
#include "BasicUsageEnvironment.hh"
#include "liveMedia.hh"
#include "utils.h"


class RTSPSession {
public:
  RTSPSession();
  virtual ~RTSPSession();
public:
  int InitRTSPClient(char const *progname, char const *rtsp_url,OutBufferFunc callback, int channel_id);
  int StartRTSPClient();
  int StopRTSPClient();
  void ShutdownHandler(unsigned int err);

  unsigned int VideoWidth();
  unsigned int VideoHeight();
  unsigned int VideoFps();

  static void *ThreadFun(void *param); 
  
private:
  //int OpenURL(UsageEnvironment &env, char const *progname, char const *rtsp_url, int debug_level);
  void RTSPRun();

public:
  RTSPClient *rtsp_client_;
  char eventLoop_watch_variable_;
  int status_;
  int channel_id_;
private:
  //std::mutex session_lock_;
  pthread_t thread_id_;
  std::string rtsp_url_;
  std::string prog_name_;
  int debug_level_;
  TaskScheduler *scheduler_;
  UsageEnvironment *env_;

};

#endif //__RTSPSESSION_