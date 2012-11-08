/*
 * atomic.h
 *
 *  Created on: 06.11.2012
 *      Author: savop
 */

#ifndef ATOMIC_H_
#define ATOMIC_H_

#include <vdr/thread.h>

namespace upnp {

namespace tools {

namespace atomic {

class AtomicInteger {
private:
  uint32_t value;
  cMutex mutex;

public:
  AtomicInteger()
  : value(0)
  {}

  inline void increment() {
    mutex.Lock();
    ++value;
    mutex.Unlock();
  }

  inline void decrement() {
    mutex.Lock();
    --value;
    mutex.Unlock();
  }

  operator unsigned int() const {
    return Get();
  }

  unsigned int Get() const {
    return value;
  }

  AtomicInteger& operator++(){
    increment();
    return *this;
  }

  AtomicInteger operator++(int){
    increment();
    return *this;
  }

  AtomicInteger& operator--(){
    decrement();
    return *this;
  }

  AtomicInteger operator--(int){
    decrement();
    return *this;
  }

};

}  // namespace atomic

}  // namespace tools

}  // namespace upnp


#endif /* ATOMIC_H_ */
