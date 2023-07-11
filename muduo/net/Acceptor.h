// Copyright 2010, Shuo Chen.  All rights reserved.
// http://code.google.com/p/muduo/
//
// Use of this source code is governed by a BSD-style license
// that can be found in the License file.

// Author: Shuo Chen (chenshuo at chenshuo dot com)
//
// This is an internal header file, you should not include this.

#ifndef MUDUO_NET_ACCEPTOR_H
#define MUDUO_NET_ACCEPTOR_H

#include <functional>

#include "muduo/net/Channel.h"
#include "muduo/net/Socket.h"

namespace muduo
{
namespace net
{

class EventLoop;
class InetAddress;

///
/// Acceptor of incoming TCP connections.
/// 运行于主线程上的mainloop上，负责接受新用户的连接并分发给subreactor
/// 只对应listenfd，acceptchannel
///
class Acceptor : noncopyable
{
 public:
 // 当新连接到达时调用的回调函数----一种上层回调
  typedef std::function<void (int sockfd, const InetAddress&)> NewConnectionCallback;
  // using NewConnectionCallback = std::function<void (int sockfd, const InetAddress&)>;

  Acceptor(EventLoop* loop, const InetAddress& listenAddr, bool reuseport);
  ~Acceptor();

  void setNewConnectionCallback(const NewConnectionCallback& cb)  // 在TcpServer构造函数中调用设置为TcpServer::newConnection
  { newConnectionCallback_ = cb; }

  bool listenning() const { return listenning_; }
  void listen();

 private:

  // 注册到accept channel的readCallback_
  void handleRead();  // 是个回调函数，主listenfd上的连接事件处理函数，即进行accept

  EventLoop* loop_;  // mainloop：**acceptor指向的就是用户定义的哪个baseloop，也叫mainloop**，即主reactor运行的线程绑定的el

  Socket acceptSocket_;  // listenfd
  Channel acceptChannel_;  // listenfd =》 channel

  NewConnectionCallback newConnectionCallback_;  // 此acceptor监听到读事件（连接事件）创建连接后调用的回调函数。这个TcpServer::newConnection函数的功能是公平的选择一个subEventLoop，并把已经接受的连接分发给这个subEventLoop。

  bool listenning_;
  int idleFd_;  // accept失败，fd数量不够时时用到
};

}  // namespace net
}  // namespace muduo

#endif  // MUDUO_NET_ACCEPTOR_H
