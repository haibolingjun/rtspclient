// Copyright 2023 

#include <assert.h>
#include <chrono>
#include <fcntl.h>
#include <list>
#include <map>
#include <memory>
#include <mutex>
#include <pthread.h>
#include <signal.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <string>
#include <time.h>
#include <unistd.h>
#include <vector>

#include "RTSPSession.h"
#include "rtspapi.h"
#include "rtsptool.h"
#include "utils.h"

void __attribute__((constructor)) my_init() { printf("init rtspso so\n"); }
void __attribute__((destructor)) my_exit() { printf("exit rtspso so\n"); }

////////////////////////////////////////////////////////////////////////////////////////

void *RTSP_InitRTSPClient(char const *progname, char const *rtsp_url, OutBufferFunc callback, int channel_id) {
  RTSPSession *rtsp_session = new RTSPSession;
  if (rtsp_session->InitRTSPClient(progname, rtsp_url, callback, channel_id)) {
    delete rtsp_session;
    rtsp_session = NULL;
    return nullptr;
  }
  return rtsp_session;
} 

int RTSP_StartRTSPClient(void *client_handle) {
  if (!client_handle) {
    return -1;
  }
  
  try {
    RTSPSession *rtsp_session = static_cast<RTSPSession *>(client_handle);
    rtsp_session->StartRTSPClient();
  } catch (...) {
    perror("start rtsp client has exception");
    return -1;
  }
  return 0;
}

int RTSP_StopRTSPClient(void *client_handle) {
  if (!client_handle) {
    return -1;
  }

  try {
    RTSPSession *rtsp_session = static_cast<RTSPSession *>(client_handle);
    rtsp_session->StopRTSPClient();
  } catch (...) {
    perror("stop rtsp client has exception");
    return -1;
  }
  return 0;
}
