// Copyright 2010, Shuo Chen.  All rights reserved.
// http://code.google.com/p/muduo/
//
// Use of this source code is governed by a BSD-style license
// that can be found in the License file.

// Author: Shuo Chen (chenshuo at chenshuo dot com)
//
// This is an internal header file, you should not include this.

#ifndef MUDUO_NET_POLLER_EPOLLPOLLER_H
#define MUDUO_NET_POLLER_EPOLLPOLLER_H

#include "muduo/net/Poller.h"

#include <vector>

struct epoll_event;

namespace muduo
{
namespace net
{

///
/// IO Multiplexing with epoll(4).
///
class EPollPoller : public Poller
{
 public:
  EPollPoller(EventLoop* loop);
  ~EPollPoller() override;
  
  /**
   * @brief 底层调用epoll-wait
   * 
   * @param timeoutMs 
   * @param[out] activeChannels 返回监听到的发生事件的fd，以及发生的具体事件类型
   * @return Timestamp 
   */
  // epoll_wait
  Timestamp poll(int timeoutMs, ChannelList* activeChannels) override;
  // 对应epoll_ctl
  void updateChannel(Channel* channel) override;
  void removeChannel(Channel* channel) override;

 private:
 // epollevent数组长度
  static const int kInitEventListSize = 16;

  static const char* operationToString(int op);  // 用于debug

  // 使用numevents填写活跃的连接的channel
  void fillActiveChannels(int numEvents,
                          ChannelList* activeChannels) const;
  void update(int operation, Channel* channel);  // 更新epollfd中监听此channel的事件

  typedef std::vector<struct epoll_event> EventList;
  int epollfd_;  // 封装的epollfd
  EventList events_; // epollwait传出参数，接受发生的事件信息。 可能会动态扩容
};

}  // namespace net
}  // namespace muduo
#endif  // MUDUO_NET_POLLER_EPOLLPOLLER_H
