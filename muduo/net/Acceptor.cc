// Copyright 2010, Shuo Chen.  All rights reserved.
// http://code.google.com/p/muduo/
//
// Use of this source code is governed by a BSD-style license
// that can be found in the License file.

// Author: Shuo Chen (chenshuo at chenshuo dot com)

#include "muduo/net/Acceptor.h"

#include "muduo/base/Logging.h"
#include "muduo/net/EventLoop.h"
#include "muduo/net/InetAddress.h"
#include "muduo/net/SocketsOps.h"

#include <errno.h>
#include <fcntl.h>
//#include <sys/types.h>
//#include <sys/stat.h>
#include <unistd.h>

using namespace muduo;
using namespace muduo::net;

Acceptor::Acceptor(EventLoop* loop, const InetAddress& listenAddr, bool reuseport)
  : loop_(loop),
    acceptSocket_(sockets::createNonblockingOrDie(listenAddr.family())),  // 创建非阻塞listenfd
    acceptChannel_(loop, acceptSocket_.fd()),
    listenning_(false),
    idleFd_(::open("/dev/null", O_RDONLY | O_CLOEXEC))  // "/dev/null"为黑洞文件，可以无限向此文件写数据，最终都会被丢弃。这个操作也就是先预留一个文件描述符，当accpet失败，打开过多时，抛弃这个那么就有可用的fd了
{
  assert(idleFd_ >= 0);
  acceptSocket_.setReuseAddr(true);
  acceptSocket_.setReusePort(reuseport);
  acceptSocket_.bindAddress(listenAddr);  // bind将listenfd绑定到服务器地址

  // baseloop -> acceptChannel(listenfd) -> ReadCallback
  acceptChannel_.setReadCallback(  // acceptchannel上可读事件的回调处理函数
      std::bind(&Acceptor::handleRead, this));
}

Acceptor::~Acceptor()
{
  acceptChannel_.disableAll();
  acceptChannel_.remove();  // 从epoll的channelmap中移除
  ::close(idleFd_);
}

void Acceptor::listen()
{
  loop_->assertInLoopThread();
  listenning_ = true;
  acceptSocket_.listen();
  acceptChannel_.enableReading();  // listenfd注册到mainloop的poller
}

// listenfd上有读事件（IO事件/新用户连接时）执行此函数
void Acceptor::handleRead()
{
  loop_->assertInLoopThread();
  
  InetAddress peerAddr;  // 获取客户端地址
  //FIXME loop until no more
  int connfd = acceptSocket_.accept(&peerAddr);
  if (connfd >= 0)
  {
    // string hostport = peerAddr.toIpPort();
    // LOG_TRACE << "Accepts of " << hostport;
    if (newConnectionCallback_)
    {
      // 轮询找到dubloop，唤醒并分发当前新客户端的channel。他就是 TcpServer::newConnection
      newConnectionCallback_(connfd, peerAddr);
    }
    else
    {
      // 必须要有处理IO连接的回调
      sockets::close(connfd);
    }
  }
  else
  {
    LOG_SYSERR << "in Acceptor::handleRead";
    // Read the section named "The special problem of
    // accept()ing when you can't" in libev's doc.
    // By Marc Lehmann, author of libev.
    if (errno == EMFILE)  // too many open files  没有可用的fd了，无法分配connfd
    {
      ::close(idleFd_);
      idleFd_ = ::accept(acceptSocket_.fd(), NULL, NULL);
      ::close(idleFd_);
      idleFd_ = ::open("/dev/null", O_RDONLY | O_CLOEXEC);
    }
  }
}

