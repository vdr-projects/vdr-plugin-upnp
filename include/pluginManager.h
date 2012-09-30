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

class cMediaManager;

class cPluginManager {
public:
  typedef void*(*FunctionPtr)(void);

  cPluginManager(cMediaManager* manager);
  virtual ~cPluginManager();

  bool LoadPlugins(const std::string& directory);
private:
  class DLL {
  public:
    DLL(const std::string& file);
    virtual ~DLL();

    bool Load();
    FunctionPtr GetProvider() const { return provider; };
    FunctionPtr GetProfiler() const { return profiler; };
  private:
    std::string file;
    void* handle;
    FunctionPtr provider;
    FunctionPtr profiler;
  };

  typedef std::list< boost::shared_ptr<DLL> > DLLList;

  DLLList dlls;
  cMediaManager* manager;

};

}  // namespace upnp

#endif /* PLUGINMANAGER_H_ */
