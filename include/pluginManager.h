/*
 * pluginManager.h
 *
 *  Created on: 05.09.2012
 *      Author: savop
 */

#ifndef PLUGINMANAGER_H_
#define PLUGINMANAGER_H_

#include "../include/plugin.h"
#include <string>
#include <map>
#include <list>
#include <boost/shared_ptr.hpp>

namespace upnp {

class cPluginManager {
public:

  typedef std::list< boost::shared_ptr<cMediaProfiler> > ProfilerList;
  typedef std::list< boost::shared_ptr<cUPnPResourceProvider> > ProviderList;

  cPluginManager();
  virtual ~cPluginManager();

  bool LoadPlugins(const std::string& directory);

  int Count() const;

  const ProfilerList& GetProfilers() const;
  const ProviderList& GetProviders() const;
  cUPnPResourceProvider* CreateProvider(const string& schema);

private:

  typedef cUPnPResourceProvider*(*ResourceProviderFuncPtr)(void);
  typedef cMediaProfiler*(*MediaProfilerFuncPtr)(void);

  class DLL {
  public:
    DLL(const std::string& file);
    virtual ~DLL();

    bool Load();

    bool IsProvider() const { return isProvider; }

    typedef void*(*FuncPtr)(void);

    FuncPtr GetFunc() const { return function; };
  private:
    std::string file;
    void* handle;
    bool isProvider;
    FuncPtr function;
  };

  typedef std::list< boost::shared_ptr<DLL> > DLLList;
  typedef std::map<string, ResourceProviderFuncPtr > ProviderMap;

  DLLList       dlls;
  ProviderMap   providerFactory;
  ProfilerList  profilers;
  ProviderList  providers;

};

}  // namespace upnp

#endif /* PLUGINMANAGER_H_ */
