// Copyright 2010, Shuo Chen.  All rights reserved.
// http://code.google.com/p/muduo/
//
// Use of this source code is governed by a BSD-style license
// that can be found in the License file.

// Author: Shuo Chen (chenshuo at chenshuo dot com)
//
// This is a public header file, it must only include public header files.

// reactor反应堆

#ifndef MUDUO_NET_EVENTLOOP_H
#define MUDUO_NET_EVENTLOOP_H

#include <atomic>
#include <functional>
#include <vector>

#include <boost/any.hpp>

#include "muduo/base/Mutex.h"
#include "muduo/base/CurrentThread.h"
#include "muduo/base/Timestamp.h"
#include "muduo/net/Callbacks.h"
#include "muduo/net/TimerId.h"

namespace muduo
{
namespace net
{

class Channel;
class Poller;
class TimerQueue;

///
/// Reactor, at most one per thread. （mainreactor）  reactor，事件循环
///
/// This is an interface class, so don't expose too much details.
class EventLoop : noncopyable
{
 public:
  typedef std::function<void()> Functor;

  EventLoop();
  ~EventLoop();  // force out-line dtor, for std::unique_ptr members.

  ///
  /// Loops forever.
  ///
  /// Must be called in the same thread as creation of the object.
  ///
  void loop();  // 关键方法

  /// Quits loop.
  ///
  /// This is not 100% thread safe, if you call through a raw pointer,
  /// better to call through shared_ptr<EventLoop> for 100% safety.
  void quit();

  ///
  /// Time when poll returns, usually means data arrival.
  ///
  Timestamp pollReturnTime() const { return pollReturnTime_; }

  int64_t iteration() const { return iteration_; }

  /// Runs callback immediately in the loop thread.
  /// It wakes up the loop, and run the cb.
  /// If in the same loop thread, cb is run within the function.
  /// Safe to call from other threads.
  void runInLoop(Functor cb);
  /// Queues callback in the loop thread.
  /// Runs after finish pooling.
  /// Safe to call from other threads.
  void queueInLoop(Functor cb);  // 把上层注册的回调函数放入队列中，唤醒loop所在的线程执行cb

  size_t queueSize() const;

  // timers

  // 这三个是在某个时间执行回调。定时器相关TimeQueue
  ///
  /// Runs callback at 'time'.
  /// Safe to call from other threads.
  ///
  TimerId runAt(Timestamp time, TimerCallback cb);
  ///
  /// Runs callback after @c delay seconds.
  /// Safe to call from other threads.
  ///
  TimerId runAfter(double delay, TimerCallback cb);
  ///
  /// Runs callback every @c interval seconds.
  /// Safe to call from other threads.
  ///
  TimerId runEvery(double interval, TimerCallback cb);
  ///
  /// Cancels the timer.
  /// Safe to call from other threads.
  ///
  void cancel(TimerId timerId);

  // internal usage  ****通过eventfd唤醒loop所在的线程*****。 主reactor使用wakeup唤醒子reactor处理读写.会获得一个fd
  void wakeup();
  
  // channel与poller交互的中间函数
  void updateChannel(Channel* channel);
  void removeChannel(Channel* channel);
  bool hasChannel(Channel* channel);

  // pid_t threadId() const { return threadId_; }
  void assertInLoopThread()
  {
    if (!isInLoopThread())
    {
      abortNotInLoopThread();
    }
  }
  // 判断el对象是否在自己的线程里
  bool isInLoopThread() const { return threadId_ == CurrentThread::tid(); }
  // bool callingPendingFunctors() const { return callingPendingFunctors_; }
  bool eventHandling() const { return eventHandling_; }

  void setContext(const boost::any& context)
  { context_ = context; }

  const boost::any& getContext() const
  { return context_; }

  boost::any* getMutableContext()
  { return &context_; }

  static EventLoop* getEventLoopOfCurrentThread();

 private:
  void abortNotInLoopThread();
  void handleRead();  // waked up  wakeup fd绑定的回调。在构造函数中进行了绑定
  void doPendingFunctors();  // 执行上层回调

  void printActiveChannels() const; // DEBUG

  typedef std::vector<Channel*> ChannelList;

  bool looping_; /* atomic */

  std::atomic<bool> quit_;
  bool eventHandling_; /* atomic */
  bool callingPendingFunctors_; /* atomic */  // 标识当前loop是否有需要执行的回调操作
  
  int64_t iteration_;  // 计数这是这个loop的第几次循环？
  const pid_t threadId_;  // 记录此el是哪个线程创建的
  Timestamp pollReturnTime_;  // poller返回发生事件的channel的时间点
  std::unique_ptr<Poller> poller_;  // 一个loop拥有一个唯一的Poller，属于一个线程，监听多个channel/fd
  std::unique_ptr<TimerQueue> timerQueue_;  // 是什么？

  int wakeupFd_;  // ！！！这是一个eventfd，每个eventloop有一个唯一的（也就是每个线程有唯一wakeupfd）。wakeupFd由建立此eventloop的线程监听，因此通过往wakeup中写，可以唤醒此eventloop对应的线程。
  // unlike in TimerQueue, which is an internal class,
  // we don't expose Channel to client.
  std::unique_ptr<Channel> wakeupChannel_;  // 封装wakeFd

  boost::any context_;

  // scratch variables
  ChannelList activeChannels_;  // 所有有事件发生的channel，loop中每次循环都会clear
  Channel* currentActiveChannel_;  // 此el当前处理的活跃channel

  mutable MutexLock mutex_;  // 保护pendingFunctors_
  std::vector<Functor> pendingFunctors_ GUARDED_BY(mutex_);  // 存储loop需要执行的所有回调操作。
};

}  // namespace net
}  // namespace muduo

#endif  // MUDUO_NET_EVENTLOOP_H
