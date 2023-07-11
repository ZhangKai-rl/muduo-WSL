// Copyright 2010, Shuo Chen.  All rights reserved.
// http://code.google.com/p/muduo/
//
// Use of this source code is governed by a BSD-style license
// that can be found in the License file.

// Author: Shuo Chen (chenshuo at chenshuo dot com)
//
// This is an internal header file, you should not include this.

#ifndef MUDUO_NET_CHANNEL_H
#define MUDUO_NET_CHANNEL_H

#include "muduo/base/noncopyable.h"
#include "muduo/base/Timestamp.h"

#include <functional>
#include <memory>

namespace muduo
{
namespace net
{

class EventLoop;

///
/// A selectable I/O channel.
///
/// This class doesn't own the file descriptor.
/// The file descriptor could be a socket,
/// an eventfd, a timerfd, or a signalfd

// 对大量的文件描述符进行管理的类。将fd可能的读写错误操作的cb函数绑定到channel类.***一个fd绑定一个channel***
// poller不直接与channel进行交互，而是通过eventloop间接交互
class Channel : noncopyable
{
 public:
  typedef std::function<void()> EventCallback;
  typedef std::function<void(Timestamp)> ReadEventCallback;

  Channel(EventLoop* loop, int fd);
  ~Channel();

  void handleEvent(Timestamp receiveTime);  // 处理此fd上发生的事件
  // 设置回调函数。在TcpConnection类构造时调用
  void setReadCallback(ReadEventCallback cb)
  { readCallback_ = std::move(cb); }  // 将左值转成右值；节省资源
  void setWriteCallback(EventCallback cb)
  { writeCallback_ = std::move(cb); }
  void setCloseCallback(EventCallback cb)
  { closeCallback_ = std::move(cb); }
  void setErrorCallback(EventCallback cb)
  { errorCallback_ = std::move(cb); }

  /// Tie this channel to the owner object managed by shared_ptr,
  /// prevent the owner object being destroyed in handleEvent.
  void tie(const std::shared_ptr<void>&);  // 防止channel被手动remove掉，channel还在执行以上回调，因为el里面有removechannel

  int fd() const { return fd_; }
  int events() const { return events_; }
  void set_revents(int revt) { revents_ = revt; } // used by pollers   此channel中发生了什么事件，有poller给与
  // int revents() const { return revents_; }
  bool isNoneEvent() const { return events_ == kNoneEvent; }
  
  // 设置fd相应的事件状态，类似epoll ctl add delte
  void enableReading() { events_ |= kReadEvent; update(); }
  void disableReading() { events_ &= ~kReadEvent; update(); }
  void enableWriting() { events_ |= kWriteEvent; update(); }
  void disableWriting() { events_ &= ~kWriteEvent; update(); }
  void disableAll() { events_ = kNoneEvent; update(); }

  // 返回fd当前状态，是否关注此类事件
  bool isWriting() const { return events_ & kWriteEvent; }
  bool isReading() const { return events_ & kReadEvent; }

  // for Poller ？
  int index() { return index_; }
  void set_index(int idx) { index_ = idx; }

  // for debug
  string reventsToString() const;
  string eventsToString() const;

  void doNotLogHup() { logHup_ = false; }

  EventLoop* ownerLoop() { return loop_; }
  void remove();

 private:
  static string eventsToString(int fd, int ev);
  // update和remove作用是调用epoll_ctl将此channel加入epollfd。通过channel所属的el，调用poller相应方法，注册fd的events事件。
  void update();
  void handleEventWithGuard(Timestamp receiveTime);  // ？

  static const int kNoneEvent;
  static const int kReadEvent;
  static const int kWriteEvent;

  EventLoop* loop_;  // 此channel属于哪个el
  const int  fd_;  // 此channel绑定的fd
  int        events_;  // 注册fd感兴趣的事件到epoll
  int        revents_; // epollwait传出参数，此channel/fd实际发生的事件
  int        index_; // used by Poller.  具体作用？三种kNew、kAdded、kDeleted
  bool       logHup_;  // 日志相关

  std::weak_ptr<void> tie_; // void表示接受任意类型。跨线程，用于延长生命周期TCPConnection
  bool tied_;
  
  bool eventHandling_;
  bool addedToLoop_;
  // 四种rvents事件回调， 由tcpconnection进行设置，此fd上发生的事件对应的处理函数
  // 因为channel能过知道fd发生的具体时间revents，所以它负责具体事件的回调操作
  ReadEventCallback readCallback_;
  EventCallback writeCallback_;
  EventCallback closeCallback_;
  EventCallback errorCallback_;
};

}  // namespace net
}  // namespace muduo

#endif  // MUDUO_NET_CHANNEL_H
