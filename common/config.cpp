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
, useInternalWebserver(false)
, webServerPort(0)
, presentationURL("index.html")
, maxContentLength(KB(20))
, announceMaxAge(1800)
, deviceUUID(tools::GenerateUUIDRandomly())
, serviceURL("services/")
, staticContentURL("http/")
, bindToAddress(true)
, address("0.0.0.0")
, port(0)
, databaseFile("metadata.db")
{
}
