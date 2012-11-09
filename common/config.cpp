/*
 * config.cpp
 *
 *  Created on: 05.08.2012
 *      Author: savop
 */


#include "../include/config.h"
#include "../include/tools.h"
#include "../upnp.h"

using namespace upnp;

upnp::cConfig::cConfig()
: enabled(true)
, expertSettings(false)
, webServerPort(0)
, maxRequestTime(300)
, presentationURL("index.html")
, useLive(false)
, livePort(8008)
, maxContentLength(KB(20))
, announceMaxAge(1800)
, deviceUUID(tools::GenerateUUIDRandomly())
, bindToAddress(true)
, address("0.0.0.0")
, port(0)
{
}

void upnp::cConfig::GenerateNewDeviceUUID(){
  deviceUUID = tools::GenerateUUIDRandomly();
}
