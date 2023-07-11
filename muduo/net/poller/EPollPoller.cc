// Copyright 2010, Shuo Chen.  All rights reserved.
// http://code.google.com/p/muduo/
//
// Use of this source code is governed by a BSD-style license
// that can be found in the License file.

// Author: Shuo Chen (chenshuo at chenshuo dot com)

#include "muduo/net/poller/EPollPoller.h"

#include "muduo/base/Logging.h"
#include "muduo/net/Channel.h"

#include <assert.h>
#include <errno.h>
#include <poll.h>
#include <sys/epoll.h>
#include <unistd.h>

using namespace muduo;
using namespace muduo::net;

// On Linux, the constants of poll(2) and epoll(4)
// are expected to be the same.
static_assert(EPOLLIN == POLLIN,        "epoll uses same flag values as poll");
static_assert(EPOLLPRI == POLLPRI,      "epoll uses same flag values as poll");
static_assert(EPOLLOUT == POLLOUT,      "epoll uses same flag values as poll");
static_assert(EPOLLRDHUP == POLLRDHUP,  "epoll uses same flag values as poll");
static_assert(EPOLLERR == POLLERR,      "epoll uses same flag values as poll");
static_assert(EPOLLHUP == POLLHUP,      "epoll uses same flag values as poll");

namespace
{  // channel索引在epoll中的三个取值，初始化为-1
const int kNew = -1;  // fd尚未添加进poller
const int kAdded = 1;  // epoll_ctl_add
const int kDeleted = 2;  // epoll_ctl_del
}

EPollPoller::EPollPoller(EventLoop* loop)
  : Poller(loop),
    epollfd_(::epoll_create1(EPOLL_CLOEXEC)),  // *******epoll_create1*********
    events_(kInitEventListSize)
{
  if (epollfd_ < 0)
  {
    LOG_SYSFATAL << "EPollPoller::EPollPoller";
  }
}

EPollPoller::~EPollPoller()
{
  ::close(epollfd_);
}

Timestamp EPollPoller::poll(int timeoutMs, ChannelList* activeChannels)
{
  LOG_TRACE << "fd total count " << channels_.size();

  int numEvents = ::epoll_wait(epollfd_,
                               &*events_.begin(),
                               static_cast<int>(events_.size()),  // 类型安全强转
                               timeoutMs);
  int savedErrno = errno;  // 防止多线程设置errno，使用局部变量保存。
  Timestamp now(Timestamp::now());
  if (numEvents > 0)
  {
    LOG_TRACE << numEvents << " events happened";
    fillActiveChannels(numEvents, activeChannels);
    if (implicit_cast<size_t>(numEvents) == events_.size())  // 需要扩容
    {
      events_.resize(events_.size()*2);
    }
  }
  else if (numEvents == 0)
  {
    LOG_TRACE << "timeout and nothing happened";
  }
  else
  {
    // error happens, log uncommon ones
    if (savedErrno != EINTR) // 中断
    {
      errno = savedErrno;  // 用于日志系统输出errno
      LOG_SYSERR << "EPollPoller::poll()";
    }
  }
  return now;
}

void EPollPoller::fillActiveChannels(int numEvents,
                                     ChannelList* activeChannels) const
{
  assert(implicit_cast<size_t>(numEvents) <= events_.size());
  for (int i = 0; i < numEvents; ++i)
  {
    // 获取有事件发生的fd的channel指针********
    Channel* channel = static_cast<Channel*>(events_[i].data.ptr);  // 根据poller：：update获取channel
#ifndef NDEBUG
    int fd = channel->fd();
    ChannelMap::const_iterator it = channels_.find(fd);
    assert(it != channels_.end());
    assert(it->second == channel);
#endif
    channel->set_revents(events_[i].events);  // 写到channel实际发生的事件中
    activeChannels->push_back(channel);
  }
}

void EPollPoller::updateChannel(Channel* channel)
{
  Poller::assertInLoopThread();
  const int index = channel->index();
  LOG_TRACE << "fd = " << channel->fd()
    << " events = " << channel->events() << " index = " << index;
  if (index == kNew || index == kDeleted)
  {
    // a new one, add with EPOLL_CTL_ADD
    int fd = channel->fd();
    if (index == kNew)
    {
      assert(channels_.find(fd) == channels_.end());
      channels_[fd] = channel;
    }
    else // index == kDeleted
    {
      assert(channels_.find(fd) != channels_.end());
      assert(channels_[fd] == channel);
    }

    channel->set_index(kAdded);
    update(EPOLL_CTL_ADD, channel);
  }
  else  // index == kAdded。已经添加，更新epoll事件
  {
    // update existing one with EPOLL_CTL_MOD/DEL
    int fd = channel->fd();
    (void)fd;
    assert(channels_.find(fd) != channels_.end());
    assert(channels_[fd] == channel);
    assert(index == kAdded);
    if (channel->isNoneEvent())  // 此时epoll不再监听，但是后续可能仍会监听，因此不从channel_中erase
    {
      update(EPOLL_CTL_DEL, channel);
      channel->set_index(kDeleted);
    }
    else
    {
      update(EPOLL_CTL_MOD, channel);
    }
  }
}

// 不再监听此channel
void EPollPoller::removeChannel(Channel* channel)
{
  Poller::assertInLoopThread();
  int fd = channel->fd();
  LOG_TRACE << "fd = " << fd;
  assert(channels_.find(fd) != channels_.end());
  assert(channels_[fd] == channel);
  assert(channel->isNoneEvent());
  int index = channel->index();
  assert(index == kAdded || index == kDeleted);

  size_t n = channels_.erase(fd);  // 

  (void)n;
  assert(n == 1);

  if (index == kAdded)
  {
    update(EPOLL_CTL_DEL, channel);
  }
  channel->set_index(kNew);
}

void EPollPoller::update(int operation, Channel* channel)
{
  // 使用临时epoll_event获取该channel更新的epoll_event
  struct epoll_event event;
  memZero(&event, sizeof event);

  event.events = channel->events();  // 更新感兴趣事件
  event.data.ptr = channel;  // data为union，这里不使用fd，而使用*ptr，携带更丰富的信息。

  int fd = channel->fd();
  LOG_TRACE << "epoll_ctl op = " << operationToString(operation)
    << " fd = " << fd << " event = { " << channel->eventsToString() << " }";
  // epoll_ctl操作
  if (::epoll_ctl(epollfd_, operation, fd, &event) < 0)
  {
    if (operation == EPOLL_CTL_DEL)  // 没删除成功，那么我不用这个fd就是了
    {
      LOG_SYSERR << "epoll_ctl op =" << operationToString(operation) << " fd =" << fd;
    }
    else
    {
      LOG_SYSFATAL << "epoll_ctl op =" << operationToString(operation) << " fd =" << fd;
    }
  }
}

const char* EPollPoller::operationToString(int op)  // 用于debug
{
  switch (op)
  {
    case EPOLL_CTL_ADD:
      return "ADD";
    case EPOLL_CTL_DEL:
      return "DEL";
    case EPOLL_CTL_MOD:
      return "MOD";
    default:
      assert(false && "ERROR op");
      return "Unknown Operation";
  }
}
