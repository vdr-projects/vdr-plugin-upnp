/*
 * pluginManager.h
 *
 *  Created on: 05.09.2012
 *      Author: savop
 */

#ifndef PLUGINMANAGER_H_
#define PLUGINMANAGER_H_

#include "../include/plugin.h"

namespace upnp {

class cPluginManager {
public:
  cUPnPResourceProvider* CreateResourceProviderInstance(const std::string& schema);
private:
  void LoadPlugins();
  void UnloadPlugins();

  typedef std::list<cUPnPResourceProvider> ProviderList;
  typedef std::list<cMediaProfiler> ProfilerList;

  ProviderList providers;
  ProfilerList profilers;
};

}  // namespace upnp

#endif /* PLUGINMANAGER_H_ */
