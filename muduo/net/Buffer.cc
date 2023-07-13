// Copyright 2010, Shuo Chen.  All rights reserved.
// http://code.google.com/p/muduo/
//
// Use of this source code is governed by a BSD-style license
// that can be found in the License file.

// Author: Shuo Chen (chenshuo at chenshuo dot com)
//

#include "muduo/net/Buffer.h"

#include "muduo/net/SocketsOps.h"

#include <errno.h>
#include <sys/uio.h>

using namespace muduo;
using namespace muduo::net;

const char Buffer::kCRLF[] = "\r\n";

const size_t Buffer::kCheapPrepend;
const size_t Buffer::kInitialSize;

ssize_t Buffer::readFd(int fd, int* savedErrno)
{
  // saved an ioctl()/FIONREAD call to tell how much to read
  char extrabuf[65536];  // 开辟栈空间，用于从socket读出时，buffer空间不够暂存，待buffer重新分配足够空间后，交还数据给buffer

  struct iovec vec[2];  // 分散读集中写

  const size_t writable = writableBytes();

  // 第一块缓冲区，指向buffer的可写空间
  vec[0].iov_base = begin()+writerIndex_;
  vec[0].iov_len = writable;
  // 第二块缓冲区，指向临时栈数组。长度超过buffer的writable的部分存在这里，后期append到扩容后的buffer中
  vec[1].iov_base = extrabuf;
  vec[1].iov_len = sizeof extrabuf;
  // when there is enough space in this buffer, don't read into extrabuf.
  // when extrabuf is used, we read 128k-1 bytes at most.
  const int iovcnt = (writable < sizeof extrabuf) ? 2 : 1;  // 缓冲区可写不足64K，则选择2

  const ssize_t n = sockets::readv(fd, vec, iovcnt);  // 从内核缓冲区读入buffer
  if (n < 0)
  {
    *savedErrno = errno;  // 保存errno防止被覆盖
  }
  else if (implicit_cast<size_t>(n) <= writable)  // 全部读入buffer
  {
    writerIndex_ += n;
  }
  else  // extrabuf写入了n - writable长度数据
  {
    writerIndex_ = buffer_.size();
    append(extrabuf, n - writable);  // 扩容buffer，并将数据从extrabuffer交还buffer
  }
  // if (n == writable + sizeof extrabuf)
  // {
  //   goto line_30;
  // }
  return n;
}

