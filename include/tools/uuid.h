/*
 * uuid.h
 *
 *  Created on: 03.10.2012
 *      Author: savop
 */

#ifndef UUID_H_
#define UUID_H_

#include <string>

using namespace std;

namespace upnp {

namespace tools {

  string GenerateUUIDFromURL(string url);
  string GenerateUUIDRandomly();

}

}

#endif /* UUID_H_ */
