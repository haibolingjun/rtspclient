#include "RTSPSession.h"
#include "ourRTSPClient.h"
#include "rtsptool.h"
#include "utils.h"

RTSPSession::RTSPSession() {
  rtsp_client_ = NULL;
  eventLoop_watch_variable_ = 0;
  thread_id_ = 0;
  status_ = 0;
  debug_level_ = 0;
  channel_id_ = -1;
  scheduler_ = NULL;
  env_ = NULL;
}

RTSPSession::~RTSPSession() { StopRTSPClient(); }

int RTSPSession::InitRTSPClient(char const *progname, char const *rtsp_url, OutBufferFunc callback, int channel_id) {
  prog_name_ = progname;
  rtsp_url_ = rtsp_url;
  channel_id_ = channel_id;
  eventLoop_watch_variable_ = 0;

  scheduler_ = BasicTaskScheduler::createNew();
  env_ = BasicUsageEnvironment::createNew(*scheduler_);
  rtsp_client_ = ourRTSPClient::createNew(*env_, rtsp_url, callback, channel_id, debug_level_, progname);

  if (rtsp_client_ == NULL) {
    *env_ << "Failed to create a RTSP client for URL \"" << rtsp_url << "\": " << env_->getResultMsg() << "\n";
    return -1;
  }
  (dynamic_cast<ourRTSPClient *>(rtsp_client_))->shutdown_callback_ =
      std::bind(&RTSPSession::ShutdownHandler, this, std::placeholders::_1);
  rtsp_client_->sendDescribeCommand(continueAfterDESCRIBE);
  return 0;
}

int RTSPSession::StartRTSPClient() {
  if (!rtsp_client_) {
    perror("no rtsp client handle, please call init.");
    return -1;
  }

  eventLoop_watch_variable_ = 0;
  int ret = pthread_create(&thread_id_, NULL, ThreadFun, this);
  if (ret) {
    perror("pthread_create error");
    return -1;
  }
  return 0;
}

int RTSPSession::StopRTSPClient() {
  eventLoop_watch_variable_ = 1;
  if (thread_id_) {
    pthread_join(thread_id_, NULL);
    thread_id_ = 0;
  }

  // clear
  if (rtsp_client_) {
    shutdownStream(rtsp_client_);
    rtsp_client_ = NULL;
  }
  if (env_) {
    env_->reclaim();
    env_ = NULL;
  }
  if (scheduler_) {
    delete scheduler_;
    scheduler_ = NULL;
  }

  status_ = 3;
  return 0;
}
void RTSPSession::ShutdownHandler(unsigned int err) {
  rtsp_client_ = NULL;//already deleted
  if (err == 0)
    return;
  eventLoop_watch_variable_ = 1;
  perror("rtsp thread is error,thead will end.\n");
}

void *RTSPSession::ThreadFun(void *param) {
  RTSPSession *this_session = (RTSPSession *)param;
  this_session->RTSPRun();
  return NULL;
}
void RTSPSession::RTSPRun() {
  status_ = 1;
  env_->taskScheduler().doEventLoop(&eventLoop_watch_variable_); // loop
  // loop end,finished
  status_ = 2;
}

unsigned int RTSPSession::VideoWidth() {
  if (rtsp_client_) {
    ourRTSPClient *p = dynamic_cast<ourRTSPClient *>(rtsp_client_);
    if (p)
      return p->videoWidth();
  }

  return 0;
}
unsigned int RTSPSession::VideoHeight() {
  if (rtsp_client_) {
    ourRTSPClient *p = dynamic_cast<ourRTSPClient *>(rtsp_client_);
    if (p)
      return p->videoHeight();
  }
  return 0;
}
unsigned int RTSPSession::VideoFps() {
  if (rtsp_client_) {
    ourRTSPClient *p = dynamic_cast<ourRTSPClient *>(rtsp_client_);
    if (p)
      return p->videoFps();
  }
  return 0;
}