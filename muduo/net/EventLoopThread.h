// Copyright 2010, Shuo Chen.  All rights reserved.
// http://code.google.com/p/muduo/
//
// Use of this source code is governed by a BSD-style license
// that can be found in the License file.

// Author: Shuo Chen (chenshuo at chenshuo dot com)
//
// This is a public header file, it must only include public header files.

#ifndef MUDUO_NET_EVENTLOOPTHREAD_H
#define MUDUO_NET_EVENTLOOPTHREAD_H

#include "muduo/base/Condition.h"
#include "muduo/base/Mutex.h"
#include "muduo/base/Thread.h"

namespace muduo
{
namespace net
{

class EventLoop;

// 在这个类中创建el和线程，并将两者进行绑定， one loop per thread

class EventLoopThread : noncopyable
{
 public:
  typedef std::function<void(EventLoop*)> ThreadInitCallback;  // 这就是一个上层回调

  EventLoopThread(const ThreadInitCallback& cb = ThreadInitCallback(),
                  const string& name = string());
  ~EventLoopThread();
  EventLoop* startLoop();

 private:
  void threadFunc();  // thread创建的子线程的运行函数

  EventLoop* loop_ GUARDED_BY(mutex_);
  bool exiting_;  // 是否退出循环
  Thread thread_;  // 此loop绑定的thread
  MutexLock mutex_;
  Condition cond_ GUARDED_BY(mutex_);
  ThreadInitCallback callback_;
};

}  // namespace net
}  // namespace muduo

#endif  // MUDUO_NET_EVENTLOOPTHREAD_H

