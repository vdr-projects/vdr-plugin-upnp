/*
 * requestCounter.h
 *
 *  Created on: 04.11.2012
 *      Author: savop
 */

#ifndef REQUESTCOUNTER_H_
#define REQUESTCOUNTER_H_

struct request_counter_t {
  // Should be std::atomic<int>. Unfortunatelly, it's supported on C++11 only.
  // Hope, it works anyway.
  static int OPEN_REQUESTS;
  request_counter_t(){++OPEN_REQUESTS;}
  ~request_counter_t(){--OPEN_REQUESTS;}
};

#endif /* REQUESTCOUNTER_H_ */
