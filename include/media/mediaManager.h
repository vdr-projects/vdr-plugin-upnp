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
  std::string GetContentFeatures() const;
  size_t GetContentLength() const;
  std::string GetContentType() const;
  std::string GetTransferMode(const std::string& requestedMode ) const;
  std::string GetRange() const;
  std::string GetAvailableSeekRange(const std::string& seekRequest) const;

  bool Open(string uri);
  size_t Read(char* buf, size_t bufLen);
  bool Seek(size_t offset, int origin);
  void Close();
};

class cMediaManager : public cThread {
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

  void SetDatabaseFile(string file);

  bool Initialise();

  uint32_t GetSystemUpdateID() const;
  IdList GetContainerUpdateIDs(bool unevented = false);
  StringList GetSearchCapabilities() const;
  StringList GetSortCapabilities() const;
  StringList GetSupportedProtocolInfos() const;

  int Browse(BrowseRequest& request);
  int Search(SearchRequest& request);

  static BrowseFlag ToBrowseFlag(std::string browseFlag);

  cResourceStreamer* GetResourceStreamer(std::string objectID);

private:

  void Action();
  bool CheckIntegrity();

  int CreateResponse(MediaRequest&, const string&, const string&);

  void OnContainerUpdate(string uri, long updateID);

  uint32_t mSystemUpdateID;
  IdList   mEventedContainerUpdateIDs;
  StringList mScanDirectories;
  string   mDatabaseFile;
  tntdb::Connection mConnection;

};

}  // namespace upnp


#endif /* MEDIAMANAGER_H_ */
