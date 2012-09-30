/*
 * mediaManager.h
 *
 *  Created on: 31.08.2012
 *      Author: savop
 */

#ifndef MEDIAMANAGER_H_
#define MEDIAMANAGER_H_

#include <vdr/thread.h>
#include <list>
#include <string>
#include <stdint.h>
#include <tntdb/connection.h>
#include <tntdb/connect.h>
#include "../../include/plugin.h"
#include "../../include/tools.h"

namespace upnp {

class cMediaManager;

class cResourceStreamer {
  friend class cMediaManager;
private:
  cUPnPResourceProvider* provider;
  cMetadata::Resource* resource;

  cMediaManager* manager;

  StringVector protocolInfo;

  cResourceStreamer(cMediaManager* manager, cUPnPResourceProvider* provider, cMetadata::Resource* resource);
public:
  virtual ~cResourceStreamer();
  std::string GetContentFeatures() const;
  size_t GetContentLength() const;
  std::string GetContentType() const;
  std::string GetTransferMode(const std::string& requestedMode ) const;
  bool Seekable() const;

  bool Open();
  size_t Read(char* buf, size_t bufLen);
  bool Seek(size_t offset, int origin);
  void Close();
};

class cMediaManager : public cThread {
  friend class upnp::cPluginManager;
private:

  struct MediaRequest {
    std::string objectID;
    std::string filter;
    uint32_t    startIndex;
    uint32_t    requestCount;
    std::string sortCriteria;
    std::string result;
    uint32_t    numberReturned;
    uint32_t    totalMatches;
    uint32_t    updateID;
  };

public:

  enum BrowseFlag {
    CD_BROWSE_METADATA,
    CD_BROWSE_DIRECT_CHILDREN,
    NumBrowseFlags
  };

  struct BrowseRequest : public MediaRequest {
    BrowseFlag  browseMetadata;
  };

  struct SearchRequest : public MediaRequest {
    std::string searchCriteria;
  };

  cMediaManager();
  virtual ~cMediaManager();

  void SetPluginDirectory(const string& directory);
  void SetDatabaseFile(const string& file);

  bool Initialise();

  uint32_t GetSystemUpdateID() const;
  IdList GetContainerUpdateIDs(bool unevented = false);
  StringList GetSearchCapabilities() const;
  StringList GetSortCapabilities() const;
  StringList GetSupportedProtocolInfos() const;

  int Browse(BrowseRequest& request);
  int Search(SearchRequest& request);

  static BrowseFlag ToBrowseFlag(const std::string& browseFlag);

  cResourceStreamer* GetResourceStreamer(const std::string& objectID, int resourceID = 0);

private:

  void Action();
  bool CheckIntegrity();

  int CreateResponse(MediaRequest&, const string&, const string&);

  void OnContainerUpdate(string uri, long updateID);

  cUPnPResourceProvider* CreateResourceProvider(const std::string& uri);

  void AddProviderFunctor(upnp::cPluginManager::FunctionPtr providerFunctor);
  void AddProfiler(cMediaProfiler* profiler);

  uint32_t          systemUpdateID;
  IdList            eventedContainerUpdateIDs;
  StringList        scanDirectories;
  string            databaseFile;
  string            pluginDirectory;
  tntdb::Connection connection;

  upnp::cPluginManager* pluginManager;

  typedef std::map<std::string, upnp::cPluginManager::FunctionPtr> ProviderMap;
  typedef std::list<boost::shared_ptr<cMediaProfiler>> ProfilerList;

  ProviderMap   providers;
  ProfilerList  profilers;

};

}  // namespace upnp


#endif /* MEDIAMANAGER_H_ */
