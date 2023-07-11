// Copyright 2010, Shuo Chen.  All rights reserved.
// http://code.google.com/p/muduo/
//
// Use of this source code is governed by a BSD-style license
// that can be found in the License file.

// Author: Shuo Chen (chenshuo at chenshuo dot com)

#include "muduo/net/EventLoop.h"

#include "muduo/base/Logging.h"
#include "muduo/base/Mutex.h"
#include "muduo/net/Channel.h"
#include "muduo/net/Poller.h"
#include "muduo/net/SocketsOps.h"
#include "muduo/net/TimerQueue.h"

#include <algorithm>

#include <signal.h>
#include <sys/eventfd.h>
#include <unistd.h>

using namespace muduo;
using namespace muduo::net;

namespace
{
__thread EventLoop* t_loopInThisThread = 0;  // 当前线程绑定的eventloop对象。防止一个线程创建多个eventloop

const int kPollTimeMs = 10000;

// 创建wakeupfd，用来notify唤醒subReactor处理新来的channel
int createEventfd()
{
  int evtfd = ::eventfd(0, EFD_NONBLOCK | EFD_CLOEXEC);
  if (evtfd < 0)
  {
    LOG_SYSERR << "Failed in eventfd";
    abort();
  }
  return evtfd;
}

#pragma GCC diagnostic ignored "-Wold-style-cast"
class IgnoreSigPipe
{
 public:
  IgnoreSigPipe()
  {
    ::signal(SIGPIPE, SIG_IGN);
    // LOG_TRACE << "Ignore SIGPIPE";
  }
};
#pragma GCC diagnostic error "-Wold-style-cast"

IgnoreSigPipe initObj;
}  // namespace

EventLoop* EventLoop::getEventLoopOfCurrentThread()
{
  return t_loopInThisThread;
}

EventLoop::EventLoop()
  : looping_(false),
    quit_(false),
    eventHandling_(false),
    callingPendingFunctors_(false),
    iteration_(0),
    threadId_(CurrentThread::tid()),
    poller_(Poller::newDefaultPoller(this)),
    timerQueue_(new TimerQueue(this)),
    wakeupFd_(createEventfd()),  // 非常关键！@！！；使用eventfd来初始化wakefd
    wakeupChannel_(new Channel(this, wakeupFd_)),  // 每个subreacto监听属于自己的wakeupChannel
    currentActiveChannel_(NULL)
{
  LOG_DEBUG << "EventLoop created " << this << " in thread " << threadId_;

  if (t_loopInThisThread)  // 这个线程已经绑定了某个eventloop对象
  {
    LOG_FATAL << "Another EventLoop " << t_loopInThisThread
              << " exists in this thread " << threadId_;
  }
  else
  {
    t_loopInThisThread = this;
  }

  // 注册下层wakeup channel（即本el，本线程的）回调函数
  wakeupChannel_->setReadCallback(
      std::bind(&EventLoop::handleRead, this));  // 绑定回调
  // we are always reading the wakeupfd
  // 在此函数中将wakeupfd加入poller监听
  wakeupChannel_->enableReading();
}

EventLoop::~EventLoop()
{
  LOG_DEBUG << "EventLoop " << this << " of thread " << threadId_
            << " destructs in thread " << CurrentThread::tid();
  wakeupChannel_->disableAll();
  wakeupChannel_->remove();
  ::close(wakeupFd_);
  t_loopInThisThread = NULL;
}

void EventLoop::loop()
{
  assert(!looping_);
  assertInLoopThread();

  looping_ = true;
  quit_ = false;  // FIXME: what if someone calls quit() before loop() ?
  LOG_TRACE << "EventLoop " << this << " start looping";

  while (!quit_)
  {
    activeChannels_.clear();
    // 调用epollwait,  reactor会阻塞在io多路复用这里
    // 主要监听三类fd：listenfd、connfd、wakeupfd
    pollReturnTime_ = poller_->poll(kPollTimeMs, &activeChannels_);

    ++iteration_;

    if (Logger::logLevel() <= Logger::TRACE)
    {
      printActiveChannels();
    }

    // TODO sort channel by priority
    eventHandling_ = true;
    for (Channel* channel : activeChannels_)
    {
      currentActiveChannel_ = channel;
      // 对于读事件有两种，一种来自IO线程，一种来自tcp连接。来自IO线程的主要时进行唤醒IO线程进行 doPendingFunctors()
      currentActiveChannel_->handleEvent(pollReturnTime_);  // 处理channel发生的事件
    }
    currentActiveChannel_ = NULL;
    eventHandling_ = false;

    // 执行当前eventloop事件循环需要处理的回调操作。
    // mainloop事先注册一个回调cb（需要subloop来执行） wakeup subloop后，执行下面的方法，执行之前mainloop注册的cb
    // 所以只能mainloop添加回调？只会是mainloop轮询给了个新的channel？
    doPendingFunctors();
  }

  LOG_TRACE << "EventLoop " << this << " stop looping";
  looping_ = false;
}

// 推出事件循环 1.loop在自己的线程中quit 2. 在非loop线程中，调用loop quit。其他线程中调用quit（比如subloop（worker）中，调用mainloop（IO）的quit）
void EventLoop::quit()
{
  quit_ = true;
  // There is a chance that loop() just executes while(!quit_) and exits,
  // then EventLoop destructs, then we are accessing an invalid object.
  // Can be fixed using mutex_ in both places.
  if (!isInLoopThread())
  {
    wakeup();
  }
}

void EventLoop::runInLoop(Functor cb)
{
  if (isInLoopThread())  // 此时thread_ == CurrentThread::tid() 
  {
    cb();  // 直接执行回调
  }
  else  // 在非当前el线程中执行cb，需要唤醒el所在线程的cb
  {
    queueInLoop(std::move(cb));
  }
}

// 把cb放入回调函数队列，唤醒el所在线程执行cb队列
void EventLoop::queueInLoop(Functor cb)  // 目前不是所属的线程，在该el所属的线程中执行cb
{
  {
  MutexLockGuard lock(mutex_);
  pendingFunctors_.push_back(std::move(cb));
  }

  // 是其他线程在放入回调  或者  放cb时在执行cb。
  // callingPendingFunctors_当前loop在执行回调，但是其他loop又给这个loop加入了新的回调。
  if (!isInLoopThread() || callingPendingFunctors_)
  {
    wakeup();
  }
}

size_t EventLoop::queueSize() const
{
  MutexLockGuard lock(mutex_);
  return pendingFunctors_.size();
}

TimerId EventLoop::runAt(Timestamp time, TimerCallback cb)
{
  return timerQueue_->addTimer(std::move(cb), time, 0.0);
}

TimerId EventLoop::runAfter(double delay, TimerCallback cb)
{
  Timestamp time(addTime(Timestamp::now(), delay));
  return runAt(time, std::move(cb));
}

TimerId EventLoop::runEvery(double interval, TimerCallback cb)
{
  Timestamp time(addTime(Timestamp::now(), interval));
  return timerQueue_->addTimer(std::move(cb), time, interval);
}

void EventLoop::cancel(TimerId timerId)
{
  return timerQueue_->cancel(timerId);
}

// 监听此channel fd
void EventLoop::updateChannel(Channel* channel)
{
  assert(channel->ownerLoop() == this);
  assertInLoopThread();
  poller_->updateChannel(channel);
}

void EventLoop::removeChannel(Channel* channel)
{
  assert(channel->ownerLoop() == this);
  assertInLoopThread();
  if (eventHandling_)
  {
    assert(currentActiveChannel_ == channel ||
        std::find(activeChannels_.begin(), activeChannels_.end(), channel) == activeChannels_.end());
  }
  poller_->removeChannel(channel);
}

bool EventLoop::hasChannel(Channel* channel)
{
  assert(channel->ownerLoop() == this);
  assertInLoopThread();
  return poller_->hasChannel(channel);
}

void EventLoop::abortNotInLoopThread()
{
  LOG_FATAL << "EventLoop::abortNotInLoopThread - EventLoop " << this
            << " was created in threadId_ = " << threadId_
            << ", current thread id = " <<  CurrentThread::tid();
}

// 唤醒el所在线程，通过向wakeupfd中写一个数据，wakeupchannel触发读事件，该线程就会被唤醒。
void EventLoop::wakeup()
{
  uint64_t one = 1;
  ssize_t n = sockets::write(wakeupFd_, &one, sizeof one);
  if (n != sizeof one)
  {
    LOG_ERROR << "EventLoop::wakeup() writes " << n << " bytes instead of 8";
  }
}

// wakeup channel上读事件的回调函数，
void EventLoop::handleRead()
{
  uint64_t one = 1;
  ssize_t n = sockets::read(wakeupFd_, &one, sizeof one);
  if (n != sizeof one)
  {
    LOG_ERROR << "EventLoop::handleRead() reads " << n << " bytes instead of 8";
  }
}

void EventLoop::doPendingFunctors()
{
  std::vector<Functor> functors;

  callingPendingFunctors_ = true;

  {  // 减少枷锁的临界区范围   技巧性很强。  防止与queueinloop竞态
  MutexLockGuard lock(mutex_);
  functors.swap(pendingFunctors_);
  }

  for (const Functor& functor : functors)
  {
    functor();  // 执行当前loop需要执行的上层回调操作
  }
  
  callingPendingFunctors_ = false;
}

void EventLoop::printActiveChannels() const
{
  for (const Channel* channel : activeChannels_)
  {
    LOG_TRACE << "{" << channel->reventsToString() << "} ";
  }
}

