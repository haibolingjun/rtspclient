#ifndef __UTILS_
#define __UTILS_

#include <chrono>
#include <fcntl.h>
#include <memory>
#include <mutex>
#include <functional>

#define DLL_PUBLIC __attribute__ ((visibility("default")))
#define DLL_LOCAL __attribute__ ((visibility("hidden")))

//typedef int (*OutBufferFunc)(u_int8_t*,unsigned,int);
typedef std::function<int(u_int8_t*,unsigned,int)> OutBufferFunc;
typedef std::function<void(unsigned int)> ErrorCallbackFunc;

//typedef std::unique_lock<std::mutex> AutoLock;


class AutoTimeLock {
public:
  AutoTimeLock(std::timed_mutex &tm, int t) : t_m(tm) {}
  bool Lock() {
    std::chrono::milliseconds timeout(time_out);
    return t_m.try_lock_for(timeout);
  }
  void LockLong() { t_m.lock(); }
  void Unlock() { t_m.unlock(); }
  ~AutoTimeLock() { t_m.unlock(); }

private:
  std::timed_mutex &t_m;
  int time_out;
};

#endif //__UTILS_