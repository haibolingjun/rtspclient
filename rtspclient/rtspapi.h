#ifndef __RTSPAPI_
#define __RTSPAPI_

#ifdef __cplusplus
extern "C" {
#endif

#include <stddef.h>
#include "utils.h"

#define _CAPI __attribute__ ((visibility("default")))

//typedef int (*OutBufferFunc)(u_int8_t*,unsigned,int);


_CAPI void* RTSP_InitRTSPClient(char const *progname, char const *rtsp_url,OutBufferFunc callback, int channel_id);
_CAPI int RTSP_StartRTSPClient(void *client_handle);
_CAPI int RTSP_StopRTSPClient(void *client_handle);


#ifdef __cplusplus
}
#endif
#endif //__RTSPAPI_
