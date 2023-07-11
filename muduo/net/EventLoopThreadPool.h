// Copyright 2010, Shuo Chen.  All rights reserved.
// http://code.google.com/p/muduo/
//
// Use of this source code is governed by a BSD-style license
// that can be found in the License file.

// Author: Shuo Chen (chenshuo at chenshuo dot com)
//
// This is an internal header file, you should not include this.

#ifndef MUDUO_NET_EVENTLOOPTHREADPOOL_H
#define MUDUO_NET_EVENTLOOPTHREADPOOL_H

#include "muduo/base/noncopyable.h"
#include "muduo/base/Types.h"

#include <functional>
#include <memory>
#include <vector>

namespace muduo
{

namespace net
{

class EventLoop;
class EventLoopThread;

class EventLoopThreadPool : noncopyable
{
 public:
  typedef std::function<void(EventLoop*)> ThreadInitCallback;

  EventLoopThreadPool(EventLoop* baseLoop, const string& nameArg);
  ~EventLoopThreadPool();
  void setThreadNum(int numThreads) { numThreads_ = numThreads; }
  void start(const ThreadInitCallback& cb = ThreadInitCallback());

  // valid after calling start()
  /// round-robin
  EventLoop* getNextLoop();  // 默认以轮询的方式给subloop分配channel；不断获取这个线程池中的loop

  /// with the same hash code, it will always return the same EventLoop
  EventLoop* getLoopForHash(size_t hashCode);  // hash方式分配

  std::vector<EventLoop*> getAllLoops();

  bool started() const
  { return started_; }

  const string& name() const
  { return name_; }

 private:

  EventLoop* baseLoop_;
  string name_;
  bool started_;
  int numThreads_;
  int next_;  // 轮询的下标

  // 保存着这个线程池中的线程及对应loop，不包括主的.    事件线程
  std::vector<std::unique_ptr<EventLoopThread>> threads_;
  std::vector<EventLoop*> loops_;
};

}  // namespace net
}  // namespace muduo

#endif  // MUDUO_NET_EVENTLOOPTHREADPOOL_H
