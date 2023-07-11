// Use of this source code is governed by a BSD-style license
// that can be found in the License file.
//
// Author: Shuo Chen (chenshuo at chenshuo dot com)

#ifndef MUDUO_BASE_CURRENTTHREAD_H
#define MUDUO_BASE_CURRENTTHREAD_H

#include "muduo/base/Types.h"

namespace muduo
{
namespace CurrentThread
{
  // internal
  extern __thread int t_cachedTid;  // 线程独享。每个线程都拥有这个变量的副本，互相独立互不干扰
  extern __thread char t_tidString[32];
  extern __thread int t_tidStringLength;
  extern __thread const char* t_threadName;
  void cacheTid();  // 获取tid并缓存tid，缓存是为了防止多次系统调用

  inline int tid()
  {
    if (__builtin_expect(t_cachedTid == 0, 0))
    {
      cacheTid();
    }
    return t_cachedTid;
  }

  inline const char* tidString() // for logging
  {
    return t_tidString;
  }

  inline int tidStringLength() // for logging
  {
    return t_tidStringLength;
  }

  inline const char* name()
  {
    return t_threadName;
  }

  bool isMainThread();

  void sleepUsec(int64_t usec);  // for testing

  string stackTrace(bool demangle);
}  // namespace CurrentThread
}  // namespace muduo

#endif  // MUDUO_BASE_CURRENTTHREAD_H
