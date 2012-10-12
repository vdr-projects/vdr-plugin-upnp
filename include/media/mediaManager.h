/*
 * mediaManager.h
 *
 *  Created on: 31.08.2012
 *      Author: savop
 */

#ifndef MEDIAMANAGER_H_
#define MEDIAMANAGER_H_

#include "../../include/plugin.h"
#include "../../include/tools.h"
#include <vdr/thread.h>
#include <list>
#include <string>
#include <stdint.h>
#include <tntdb/connect.h>
#include <tntdb/connection.h>

namespace upnp {

class cResourceStreamer;
class cPluginManager;

class cMediaManager : public cThread {
  friend void cUPnPResourceProvider::OnContainerUpdate(const string& uri, long updateID, const string& target = string());
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

  void OnContainerUpdate(const string& uri, long updateID, const string& target);
  bool UpdateContainerUpdateId(const string& objectID, long updateID);

  bool ScanURI(const string& uri, cUPnPResourceProvider* provider);

  bool RefreshObject(cMetadata& metadata);

  cUPnPResourceProvider* CreateResourceProvider(const std::string& uri);

  uint32_t          systemUpdateID;
  IdList            eventedContainerUpdateIDs;
  StringList        scanTargets;
  string            databaseFile;
  string            pluginDirectory;
  tntdb::Connection connection;

  upnp::cPluginManager* pluginManager;

};

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

}  // namespace upnp


#endif /* MEDIAMANAGER_H_ */
