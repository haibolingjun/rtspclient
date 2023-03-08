#include <assert.h>
#include <fcntl.h>
#include <pthread.h>
#include <signal.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <string>
#include <time.h>
#include <unistd.h>

#include "RTSPSession.h"

static void sigterm_handler(int sig) { fprintf(stderr, "signal %d\n", sig); }

class SegFaultException : public std::exception {
public:
  const char *what() const _GLIBCXX_TXN_SAFE_DYN _GLIBCXX_NOTHROW override { return "segmentation fault"; }

  ~SegFaultException() override = default;
};

void throw_segmentation_fault_exception(int) { throw SegFaultException(); }

static int GetRTSPFrame(u_int8_t *framebuff, unsigned int framesize,int cur_chn) {
  printf("channel %d: get %d\n",cur_chn,framesize);
  return 0;
}

int main(int argc, char *argv[]) {

  signal(SIGINT, sigterm_handler);
  signal(SIGSEGV, throw_segmentation_fault_exception);


  std::string rtspPath("rtsp://");//set rtsp address
  const char *rtsppath = (rtspPath.c_str());

  RTSPSession *pRtsp = new RTSPSession;
  if (pRtsp->InitRTSPClient("test", rtsppath,GetRTSPFrame, 0)) {
    delete pRtsp;
    pRtsp = NULL;
    return -1;
  }

  pRtsp->StartRTSPClient();

  usleep(200000 * 1000);
  pRtsp->StopRTSPClient();
  delete pRtsp;
  pRtsp = NULL;
  return 0;
}
