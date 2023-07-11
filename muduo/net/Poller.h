// Copyright 2010, Shuo Chen.  All rights reserved.
// http://code.google.com/p/muduo/
//
// Use of this source code is governed by a BSD-style license
// that can be found in the License file.

// Author: Shuo Chen (chenshuo at chenshuo dot com)
//
// This is an internal header file, you should not include this.

#ifndef MUDUO_NET_POLLER_H
#define MUDUO_NET_POLLER_H

#include <map>
#include <vector>

#include "muduo/base/Timestamp.h"
#include "muduo/net/EventLoop.h"

namespace muduo
{
namespace net
{

class Channel;

///
/// Base class for IO Multiplexing   IO多路复用基类
///
/// This class doesn't own the Channel objects.
class Poller : noncopyable
{
 public:
  typedef std::vector<Channel*> ChannelList;

  Poller(EventLoop* loop);
  virtual ~Poller();

  /// Polls the I/O events.
  /// Must be called in the loop thread.
  // 进行epollwait监听，并把发生事件的channel列表传出
  virtual Timestamp poll(int timeoutMs, ChannelList* activeChannels) = 0;

  /// Changes the interested I/O events.
  /// Must be called in the loop thread.
  // 被ventLoop::updateChannel调用。epollmod更新fd监听事件
  virtual void updateChannel(Channel* channel) = 0;

  /// Remove the channel, when it destructs.
  /// Must be called in the loop thread.
  virtual void removeChannel(Channel* channel) = 0;

  virtual bool hasChannel(Channel* channel) const;

  // 生成具体的IO多路复用Poller实现：如poll epoll。为什么不在cc文件中实现？基类不可包含派生类头文件
  static Poller* newDefaultPoller(EventLoop* loop);

  void assertInLoopThread() const
  {
    ownerLoop_->assertInLoopThread();
  }

 protected:
 // 根据文件描述符获取fd对应的channel
  typedef std::map<int, Channel*> ChannelMap;
  ChannelMap channels_;  // poller上注册的fd及对应channel，可能它没有感兴趣事件

 private:
  EventLoop* ownerLoop_;  // 所属的el
};

}  // namespace net
}  // namespace muduo

#endif  // MUDUO_NET_POLLER_H
