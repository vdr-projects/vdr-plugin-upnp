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

class cMediaManager : public cThread {
private:

  struct MediaRequest {
    int64_t     objectID;
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

private:

  void Action();
  bool CheckIntegrity();

  void OnContainerUpdate(string containerID, long updateID);

  uint32_t mSystemUpdateID;
  IdList   mEventedContainerUpdateIDs;
  string   mDatabaseFile;
  tntdb::Connection mConnection;

};

}  // namespace upnp


#endif /* MEDIAMANAGER_H_ */
