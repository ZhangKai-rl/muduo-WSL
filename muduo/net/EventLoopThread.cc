// Copyright 2010, Shuo Chen.  All rights reserved.
// http://code.google.com/p/muduo/
//
// Use of this source code is governed by a BSD-style license
// that can be found in the License file.

// Author: Shuo Chen (chenshuo at chenshuo dot com)

#include "muduo/net/EventLoopThread.h"

#include "muduo/net/EventLoop.h"

using namespace muduo;
using namespace muduo::net;

EventLoopThread::EventLoopThread(const ThreadInitCallback& cb,
                                 const string& name)
  : loop_(NULL),
    exiting_(false),
    thread_(std::bind(&EventLoopThread::threadFunc, this), name),  // 在这里创建线程对象，并没有创建子线程。这里bind 类成员函数，第一个参数为this指针。
    mutex_(),
    cond_(mutex_),
    callback_(cb)
{
}

EventLoopThread::~EventLoopThread()
{
  exiting_ = true;
  if (loop_ != NULL) // not 100% race-free, eg. threadFunc could be running callback_.
  {
    // still a tiny chance to call destructed object, if threadFunc exits just now.
    // but when EventLoopThread destructs, usually programming is exiting anyway.
    loop_->quit();
    thread_.join();
  }
}

EventLoop* EventLoopThread::startLoop()
{
  assert(!thread_.started());
  thread_.start();  // 进行子线程的创建，然后子线程执行：void EventLoopThread::threadFunc()函数。

  EventLoop* loop = NULL;
  {
    MutexLockGuard lock(mutex_);
    while (loop_ == NULL)
    {
      cond_.wait();  // 下面函数notify后，解除阻塞
    }
    loop = loop_;
  }

  return loop;
}

// 这个方法，是在新创建的子线程里运行的
void EventLoopThread::threadFunc()
{
  EventLoop loop;  // el创建的位置。与上面创建的子线程一一对应。  体现one loop per thread

  if (callback_)  // 运行用户线程初始化业务逻辑回调
  {
    callback_(&loop);
  }

  {
    MutexLockGuard lock(mutex_);
    loop_ = &loop;
    cond_.notify();
  }

  loop.loop();  // Eventloop loop => Pollr.poll
  //assert(exiting_);
  MutexLockGuard lock(mutex_);
  loop_ = NULL;
}

