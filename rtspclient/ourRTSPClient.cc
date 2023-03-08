#include "ourRTSPClient.h"

// Implementation of "ourRTSPClient":

ourRTSPClient *ourRTSPClient::createNew(UsageEnvironment &env, char const *rtspURL, OutBufferFunc callback,
                                        int channel_id, int verbosityLevel, char const *applicationName,
                                        portNumBits tunnelOverHTTPPortNum) {
  return new ourRTSPClient(env, rtspURL, callback, channel_id, verbosityLevel, applicationName, tunnelOverHTTPPortNum);
}

ourRTSPClient::ourRTSPClient(UsageEnvironment &env, char const *rtspURL, OutBufferFunc callback, int channel_id,
                             int verbosityLevel, char const *applicationName, portNumBits tunnelOverHTTPPortNum)
    : RTSPClient(env, rtspURL, verbosityLevel, applicationName, tunnelOverHTTPPortNum, -1) {
  callback_ = callback;
  channel_id_ = channel_id;
  shutdown_callback_ = NULL;
  video_width_ = 0;
  video_height_ = 0;
  video_fps_ = 0;
}

ourRTSPClient::~ourRTSPClient() {
  int a=0;
  a++;
}

unsigned int ourRTSPClient::videoWidth(){
  std::unique_lock<std::mutex> my_lock(session_lock_);
  return video_width_;
}
unsigned int ourRTSPClient::videoHeight(){
  std::unique_lock<std::mutex> my_lock(session_lock_);
  return video_height_;
}
unsigned int ourRTSPClient::videoFps(){
  std::unique_lock<std::mutex> my_lock(session_lock_);
  return video_fps_;
}

void ourRTSPClient::ShutdownHandler(unsigned int err){
  if(shutdown_callback_){
    shutdown_callback_(err);
  }
}