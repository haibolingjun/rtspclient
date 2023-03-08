#include "rtsptool.h"
#include "DummySink.h"
#include "ourRTSPClient.h"

// A function that outputs a string that identifies each stream (for debugging output).  Modify this if you wish:
UsageEnvironment &operator<<(UsageEnvironment &env, const RTSPClient &rtspClient) {
  return env << "[URL:\"" << rtspClient.url() << "\"]: ";
}

// A function that outputs a string that identifies each subsession (for debugging output).  Modify this if you wish:
UsageEnvironment &operator<<(UsageEnvironment &env, const MediaSubsession &subsession) {
  return env << subsession.mediumName() << "/" << subsession.codecName();
}

void usage(UsageEnvironment &env, char const *progName) {
  env << "Usage: " << progName << " <rtsp-url-1> ... <rtsp-url-N>\n";
  env << "\t(where each <rtsp-url-i> is a \"rtsp://\" URL)\n";
}

// Implementation of the RTSP 'response handlers':

// 调用流:
// sendDescribeCommand(continueAfterDESCRIBE)
// sendSetupCommand(continueAfterSETUP)
// sendPlayCommand(continueAfterPLAY)

void continueAfterDESCRIBE(RTSPClient *rtspClient, int resultCode, char *resultString) {
  do {
    UsageEnvironment &env = rtspClient->envir();                 // alias
    StreamClientState &scs = ((ourRTSPClient *)rtspClient)->scs; // alias

    if (resultCode != 0) {
      env << *rtspClient << "Failed to get a SDP description: " << resultString << "\n";
      delete[] resultString;
      break;
    }

    char *const sdpDescription = resultString;
    env << *rtspClient << "Got a SDP description:\n" << sdpDescription << "\n";

    // Create a media session object from this SDP description:
    scs.session = MediaSession::createNew(env, sdpDescription);
    delete[] sdpDescription; // because we don't need it anymore
    if (scs.session == NULL) {
      env << *rtspClient << "Failed to create a MediaSession object from the SDP description: " << env.getResultMsg()
          << "\n";
      break;
    } else if (!scs.session->hasSubsessions()) {
      env << *rtspClient << "This session has no media subsessions (i.e., no \"m=\" lines)\n";
      break;
    }

    // Then, create and set up our data source objects for the session.  We do this by iterating over the session's
    // 'subsessions', calling "MediaSubsession::initiate()", and then sending a RTSP "SETUP" command, on each one. (Each
    // 'subsession' will have its own data source.)
    scs.iter = new MediaSubsessionIterator(*scs.session);

    // get parameter by MediaSubsession
    //  MediaSubsession *sub_session = scs.iter->next();
    //  scs.iter->reset();
    //  if(sub_session){
    //    const char *mname = sub_session->mediumName();
    //    const char *cname = sub_session->codecName();
    //    const char *pname = sub_session->protocolName();
    //    unsigned short w = sub_session->videoWidth();
    //    unsigned short h = sub_session->videoHeight();
    //    unsigned int fps = sub_session->videoFPS();
    //  }

    setupNextSubsession(rtspClient);
    return;
  } while (0);

  // An unrecoverable error occurred with this stream.
  shutdownStream(rtspClient,1);
}

// By default, we request that the server stream its data using RTP/UDP.
// If, instead, you want to request that the server stream via RTP-over-TCP, change the following to True:
#define REQUEST_STREAMING_OVER_TCP False

void setupNextSubsession(RTSPClient *rtspClient) {
  UsageEnvironment &env = rtspClient->envir();                 // alias
  StreamClientState &scs = ((ourRTSPClient *)rtspClient)->scs; // alias

  scs.subsession = scs.iter->next();
  if (scs.subsession != NULL) {
    if (!scs.subsession->initiate()) {
      env << *rtspClient << "Failed to initiate the \"" << *scs.subsession << "\" subsession: " << env.getResultMsg()
          << "\n";
      setupNextSubsession(rtspClient); // give up on this subsession; go to the next one
    } else {
      env << *rtspClient << "Initiated the \"" << *scs.subsession << "\" subsession (";
      if (scs.subsession->rtcpIsMuxed()) {
        env << "client port " << scs.subsession->clientPortNum();
      } else {
        env << "client ports " << scs.subsession->clientPortNum() << "-" << scs.subsession->clientPortNum() + 1;
      }
      env << ")\n";

      if (true) {
        ourRTSPClient *ourRtspClient = (ourRTSPClient *)rtspClient;
        std::unique_lock<std::mutex> my_lock(ourRtspClient->session_lock_);
        ourRtspClient->video_width_ = scs.subsession->videoWidth();
        ourRtspClient->video_height_ = scs.subsession->videoHeight();
        ourRtspClient->video_fps_ = scs.subsession->videoFPS();
      }
      // Continue setting up this subsession, by sending a RTSP "SETUP" command:
      rtspClient->sendSetupCommand(*scs.subsession, continueAfterSETUP, False, REQUEST_STREAMING_OVER_TCP);
    }
    return;
  }

  // We've finished setting up all of the subsessions.  Now, send a RTSP "PLAY" command to start the streaming:
  if (scs.session->absStartTime() != NULL) {
    // Special case: The stream is indexed by 'absolute' time, so send an appropriate "PLAY" command:
    rtspClient->sendPlayCommand(*scs.session, continueAfterPLAY, scs.session->absStartTime(),
                                scs.session->absEndTime());
  } else {
    scs.duration = scs.session->playEndTime() - scs.session->playStartTime();
    rtspClient->sendPlayCommand(*scs.session, continueAfterPLAY);
  }
}

void continueAfterSETUP(RTSPClient *rtspClient, int resultCode, char *resultString) {
  do {
    ourRTSPClient *ourRtspClient = (ourRTSPClient *)rtspClient;
    UsageEnvironment &env = rtspClient->envir(); // alias
    StreamClientState &scs = ourRtspClient->scs; // alias

    if (resultCode != 0) {
      env << *rtspClient << "Failed to set up the \"" << *scs.subsession << "\" subsession: " << resultString << "\n";
      break;
    }

    env << *rtspClient << "Set up the \"" << *scs.subsession << "\" subsession (";
    if (scs.subsession->rtcpIsMuxed()) {
      env << "client port " << scs.subsession->clientPortNum();
    } else {
      env << "client ports " << scs.subsession->clientPortNum() << "-" << scs.subsession->clientPortNum() + 1;
    }
    env << ")\n";

    // Having successfully setup the subsession, create a data sink for it, and call "startPlaying()" on it.
    // (This will prepare the data sink to receive data; the actual flow of data from the client won't start happening
    // until later, after we've sent a RTSP "PLAY" command.)
    DummySink *pdummysink = DummySink::createNew(env, *scs.subsession, rtspClient->url());
    // perhaps use your own custom "MediaSink" subclass instead
    if (pdummysink == NULL) {
      env << *rtspClient << "Failed to create a data sink for the \"" << *scs.subsession
          << "\" subsession: " << env.getResultMsg() << "\n";
      break;
    }
    pdummysink->SetCallback(ourRtspClient->callback_, ourRtspClient->channel_id_);
    scs.subsession->sink = pdummysink;

    env << *rtspClient << "Created a data sink for the \"" << *scs.subsession << "\" subsession\n";
    // a hack to let subsession handler functions get the "RTSPClient" from the subsession
    scs.subsession->miscPtr = rtspClient;
    scs.subsession->sink->startPlaying(*(scs.subsession->readSource()), subsessionAfterPlaying, scs.subsession);
    // Also set a handler to be called if a RTCP "BYE" arrives for this subsession:
    if (scs.subsession->rtcpInstance() != NULL) {
      scs.subsession->rtcpInstance()->setByeHandler(subsessionByeHandler, scs.subsession);
    }
  } while (0);
  delete[] resultString;

  // Set up the next subsession, if any:
  setupNextSubsession(rtspClient);
}

void continueAfterPLAY(RTSPClient *rtspClient, int resultCode, char *resultString) {
  Boolean success = False;

  // delete[] resultString; //
  // return;
  // 此函数可不做其他处理
  do {
    UsageEnvironment &env = rtspClient->envir();                 // alias
    StreamClientState &scs = ((ourRTSPClient *)rtspClient)->scs; // alias

    if (resultCode != 0) {
      env << *rtspClient << "Failed to start playing session: " << resultString << "\n";
      break;
    }

    // Set a timer to be handled at the end of the stream's expected duration (if the stream does not already signal its
    // end using a RTCP "BYE").  This is optional.  If, instead, you want to keep the stream active - e.g., so you can
    // later 'seek' back within it and do another RTSP "PLAY" - then you can omit this code. (Alternatively, if you
    // don't want to receive the entire stream, you could set this timer for some shorter value.)
    if (scs.duration > 0) {
      // number of seconds extra to delay, after the stream's expected duration.  (This is optional.)
      unsigned const delaySlop = 2;
      scs.duration += delaySlop;
      unsigned uSecsToDelay = (unsigned)(scs.duration * 1000000);
      scs.streamTimerTask =
          env.taskScheduler().scheduleDelayedTask(uSecsToDelay, (TaskFunc *)streamTimerHandler, rtspClient);
    }

    env << *rtspClient << "Started playing session";
    if (scs.duration > 0) {
      env << " (for up to " << scs.duration << " seconds)";
    }
    env << "...\n";

    success = True;
  } while (0);
  delete[] resultString;

  if (!success) {
    // An unrecoverable error occurred with this stream.
    shutdownStream(rtspClient,2);
  }
}

// Implementation of the other event handlers:

void subsessionAfterPlaying(void *clientData) {
  MediaSubsession *subsession = (MediaSubsession *)clientData;
  RTSPClient *rtspClient = (RTSPClient *)(subsession->miscPtr);

  // Begin by closing this subsession's stream:
  Medium::close(subsession->sink);
  subsession->sink = NULL;

  // Next, check whether *all* subsessions' streams have now been closed:
  MediaSession &session = subsession->parentSession();
  MediaSubsessionIterator iter(session);
  while ((subsession = iter.next()) != NULL) {
    if (subsession->sink != NULL)
      return; // this subsession is still active
  }

  // All subsessions' streams have now been closed, so shutdown the client:
  shutdownStream(rtspClient);
}

void subsessionByeHandler(void *clientData) {
  MediaSubsession *subsession = (MediaSubsession *)clientData;
  RTSPClient *rtspClient = (RTSPClient *)subsession->miscPtr;
  UsageEnvironment &env = rtspClient->envir(); // alias

  env << *rtspClient << "Received RTCP \"BYE\" on \"" << *subsession << "\" subsession\n";

  // Now act as if the subsession had closed:
  subsessionAfterPlaying(subsession);
}

void streamTimerHandler(void *clientData) {
  ourRTSPClient *rtspClient = (ourRTSPClient *)clientData;
  StreamClientState &scs = rtspClient->scs; // alias

  scs.streamTimerTask = NULL;

  // Shut down the stream:
  shutdownStream(rtspClient);
}

void shutdownStream(RTSPClient *rtspClient,unsigned int error_no) {
  UsageEnvironment &env = rtspClient->envir();                 // alias
  StreamClientState &scs = ((ourRTSPClient *)rtspClient)->scs; // alias

  // First, check whether any subsessions have still to be closed:
  if (scs.session != NULL) {
    Boolean someSubsessionsWereActive = False;
    MediaSubsessionIterator iter(*scs.session);
    MediaSubsession *subsession;

    while ((subsession = iter.next()) != NULL) {
      if (subsession->sink != NULL) {
        Medium::close(subsession->sink);
        subsession->sink = NULL;

        if (subsession->rtcpInstance() != NULL) {
          // in case the server sends a RTCP "BYE" while handling "TEARDOWN"
          subsession->rtcpInstance()->setByeHandler(NULL, NULL);
        }

        someSubsessionsWereActive = True;
      }
    }

    if (someSubsessionsWereActive) {
      // Send a RTSP "TEARDOWN" command, to tell the server to shutdown the stream.
      // Don't bother handling the response to the "TEARDOWN".
      rtspClient->sendTeardownCommand(*scs.session, NULL);
    }
  }

  ((ourRTSPClient *)rtspClient)->ShutdownHandler(error_no);//notify user

  env << *rtspClient << "Closing the stream.\n";
  Medium::close(rtspClient);
  // Note that this will also cause this stream's "StreamClientState" structure to get reclaimed.
}
