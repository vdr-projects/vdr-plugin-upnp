/*
 * net.h
 *
 *  Created on: 03.10.2012
 *      Author: savop
 */

#ifndef NET_H_
#define NET_H_

#include <string>
#include "../tools.h"

using namespace std;

namespace upnp {

namespace tools {

string GetAddressByInterface(string Interface);
string GetNetworkInterfaceByIndex(int Index, bool skipLoop);
StringVector GetNetworkInterfaces(bool skipLoop);

}  // namespace tools

}  // namespace upnp



#endif /* NET_H_ */
