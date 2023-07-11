// Use of this source code is governed by a BSD-style license
// that can be found in the License file.
//
// Author: Shuo Chen (chenshuo at chenshuo dot com)

#ifndef MUDUO_BASE_COUNTDOWNLATCH_H
#define MUDUO_BASE_COUNTDOWNLATCH_H

#include "muduo/base/Condition.h"
#include "muduo/base/Mutex.h"

namespace muduo
{

/// 对condition变量再做一层封装，倒数到0才唤醒

class CountDownLatch : noncopyable
{
 public:

  explicit CountDownLatch(int count);

  void wait();

  void countDown();  // count-1

  int getCount() const;

 private:
  mutable MutexLock mutex_;
  Condition condition_ GUARDED_BY(mutex_);
  int count_ GUARDED_BY(mutex_);  // 此值变为0才会解除阻塞线程
};

}  // namespace muduo
#endif  // MUDUO_BASE_COUNTDOWNLATCH_H
