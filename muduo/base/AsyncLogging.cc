// Use of this source code is governed by a BSD-style license
// that can be found in the License file.
//
// Author: Shuo Chen (chenshuo at chenshuo dot com)

#include "muduo/base/AsyncLogging.h"
#include "muduo/base/LogFile.h"
#include "muduo/base/Timestamp.h"

#include <stdio.h>

using namespace muduo;

AsyncLogging::AsyncLogging(const string& basename,
                           off_t rollSize,
                           int flushInterval)
  : flushInterval_(flushInterval),
    running_(false),
    basename_(basename),
    rollSize_(rollSize),
    // 创建muduo::Thread对象，该对象创建子线程时，执行AsyncLogging::threadFunc函数刷盘
    thread_(std::bind(&AsyncLogging::threadFunc, this), "Logging"),
    latch_(1),  // 初始值为1，等待日志后端线程创建好
    mutex_(),
    cond_(mutex_),
    currentBuffer_(new Buffer),
    nextBuffer_(new Buffer),
    buffers_()
{
  currentBuffer_->bzero();
  nextBuffer_->bzero();
  buffers_.reserve(16);
}

// 日志前端写入buffer API.在那里调用？  日志前端生成日志时，会调用写入当前buffer。
void AsyncLogging::append(const char* logline, int len)
{
  muduo::MutexLockGuard lock(mutex_);
  if (currentBuffer_->avail() > len)
  {
    currentBuffer_->append(logline, len);
  }
  else  // 当前buffer无法放入新log
  {
    buffers_.push_back(std::move(currentBuffer_));  // 移动语义（要让当前buffer析构）到写入文件buffers中

    if (nextBuffer_)
    {
      currentBuffer_ = std::move(nextBuffer_);
    }
    else  // 日志前端写入buffer速度太快，写完了当前buffer和备用buffer
    {
      currentBuffer_.reset(new Buffer); // Rarely happens  这种情况可能不止4块缓存
    }
    currentBuffer_->append(logline, len);

    cond_.notify();  // 新加入一块满的可写缓冲块，因此唤醒日志后端写入到文件
  }
}

// 运行在哪？日志后端的实现，写入日志文件刷盘操作。
void AsyncLogging::threadFunc()
{
  assert(running_ == true);

  latch_.countDown();

  LogFile output(basename_, rollSize_, false);

  // 新建两块空闲buffer，以备在临界区交换.分别替换currentBuffer_和nextBuffer_
  BufferPtr newBuffer1(new Buffer);
  BufferPtr newBuffer2(new Buffer);
  newBuffer1->bzero();
  newBuffer2->bzero();

  BufferVector buffersToWrite;  // 待写入buffers。用于替换已满缓冲队列buffers_
  buffersToWrite.reserve(16);

  while (running_)
  {
    assert(newBuffer1 && newBuffer1->length() == 0);
    assert(newBuffer2 && newBuffer2->length() == 0);
    assert(buffersToWrite.empty());

    {
      // 临界区
      muduo::MutexLockGuard lock(mutex_);
      // 没有需要写入的buffer
      if (buffers_.empty())  // unusual usage!
      {
        cond_.waitForSeconds(flushInterval_);
      }
      buffers_.push_back(std::move(currentBuffer_));
      currentBuffer_ = std::move(newBuffer1);
      buffersToWrite.swap(buffers_);
      if (!nextBuffer_)
      {
        nextBuffer_ = std::move(newBuffer2);
      }
    }

    // 将buffersToWrite中的日志写到文件 

    assert(!buffersToWrite.empty());


    // 解决日志堆积问题：超过25块待写，只保留前两个。
    if (buffersToWrite.size() > 25) // 4MB * 25 = 100 MB
    {
      char buf[256];
      snprintf(buf, sizeof buf, "Dropped log messages at %s, %zd larger buffers\n",
               Timestamp::now().toFormattedString().c_str(),
               buffersToWrite.size()-2);
      fputs(buf, stderr);
      output.append(buf, static_cast<int>(strlen(buf)));
      buffersToWrite.erase(buffersToWrite.begin()+2, buffersToWrite.end());
    }

    for (const auto& buffer : buffersToWrite)
    {
      // FIXME: use unbuffered stdio FILE ? or use ::writev ?
      output.append(buffer->data(), buffer->length());  // 添加到LogFile中等待刷盘
    }

    // 待写缓冲buffersToWrite已经刷盘完成，去除多余buffer
    if (buffersToWrite.size() > 2)
    {
      // drop non-bzero-ed buffers, avoid trashing
      buffersToWrite.resize(2);
    }

    // 重新使用待写缓冲buffersToWrite填充两块日志后端buffer1、buiffer2

    if (!newBuffer1)
    {
      assert(!buffersToWrite.empty());
      newBuffer1 = std::move(buffersToWrite.back());
      buffersToWrite.pop_back();
      newBuffer1->reset();
    }

    if (!newBuffer2)
    {
      assert(!buffersToWrite.empty());
      newBuffer2 = std::move(buffersToWrite.back());
      buffersToWrite.pop_back();
      newBuffer2->reset();
    }

    buffersToWrite.clear();
    output.flush();  // LogFile刷盘
  }
  output.flush();
}

