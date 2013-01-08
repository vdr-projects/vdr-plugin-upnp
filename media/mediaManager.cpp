/*
 * mediaManager.cpp
 *
 *  Created on: 01.09.2012
 *      Author: savop
 */

#include "../include/media/mediaManager.h"
#include "../include/webserver.h"
#include "../include/pluginManager.h"
#include "../include/server.h"
#include "../include/parser.h"
#include <upnp/upnp.h>
#include <sstream>
#include <tntdb/result.h>
#include <upnp/ixml.h>
#include <memory>
#include <tntdb/statement.h>
#include <tntdb/transaction.h>
#include <vdr/plugin.h>

namespace upnp {

static const char* DIDLFragment = "<DIDL-Lite "
                                  "xmlns=\"urn:schemas-upnp-org:metadata-1-0/DIDL-Lite/\" "
                                  "xmlns:dc=\"http://purl.org/dc/elements/1.1/\" "
                                  "xmlns:upnp=\"urn:schemas-upnp-org:metadata-1-0/upnp/\" "
                                  "xmlns:dlna=\"urn:schemas-dlna-org:metadata-1-0/\"></DIDL-Lite>";

namespace db {

static const char* Metadata   = "metadata";
static const char* Resources  = "resources";
static const char* Details    = "details";

}

cResourceStreamer::cResourceStreamer(cMediaManager* manager, cUPnPResourceProvider* provider, cMetadata::Resource* resource)
: provider(provider)
, resource(resource)
, manager(manager)
{
  if(resource)
    tools::StringExplode(resource->GetProtocolInfo(),":",protocolInfo);
}

cResourceStreamer::~cResourceStreamer(){
  if(provider)
    provider->Close();
  delete resource;
  delete provider;
}

std::string cResourceStreamer::GetContentFeatures() const {
  if(resource == NULL) return string();

  return protocolInfo.size() == 4 ? protocolInfo[3] : string();
}

std::string cResourceStreamer::GetContentType() const {
  if(resource == NULL) return string();

  return protocolInfo.size() == 4 ? protocolInfo[2] : string();
}

size_t cResourceStreamer::GetContentLength() const {
  if(resource == NULL) return 0;

  return resource->GetSize();
}

std::string cResourceStreamer::GetTransferMode(const string&) const {
  std::string mime = GetContentType();

  if(mime.empty()) return mime;

  if(mime.find("video",0) == 0 || mime.find("audio",0) == 0) return "Streaming";
  else return "Interactive";
}

bool cResourceStreamer::Seekable() const {
  if(!provider) return false;
  return provider->Seekable();
}

bool cResourceStreamer::Open(){
  if(!provider || !resource) return false;
  return provider->Open(resource->GetResourceUri());
}

size_t cResourceStreamer::Read(char* buf, size_t bufLen){
  if(!provider) return -1;
  return provider->Read(buf, bufLen);
}

bool cResourceStreamer::Seek(size_t offset, int origin){
  if(!provider) return false;
  return provider->Seek(offset, origin);
}

void cResourceStreamer::Close(){
  if(provider) provider->Close();
}


cMediaManager::cMediaManager()
: systemUpdateID(0)
, pluginManager(NULL)
{
  SetDatabaseDir(string());
}

cMediaManager::~cMediaManager(){
  try {
    connection.execute("VACUUM");
  } catch (const std::exception& e) {
    esyslog("UPnP\tFailed to vacuum database '%s': '%s'", databaseFile.c_str(), e.what());
  }

  delete pluginManager;
}

uint32_t cMediaManager::GetSystemUpdateID() const {
  return systemUpdateID;
}

IdList cMediaManager::GetContainerUpdateIDs(bool unevented){
  IdList list = eventedContainerUpdateIDs;

  if(!unevented)
    eventedContainerUpdateIDs.clear();

  return list;
}

void cMediaManager::OnContainerUpdate(const string& uri, long updateID, const string& target){
  systemUpdateID = time(NULL);

  string objectID = tools::GenerateUUIDFromURL(uri);

  eventedContainerUpdateIDs[objectID] = updateID;

  // If we cannot update the containerUpdateID, we do not know, it is very likely, that this container
  // does not exist. Therefore, we cannot scan this directory successfully.
  if(!UpdateContainerUpdateId(objectID, updateID)) return;

  stringstream ss;

  ss << uri;

  if(!target.empty()){
    ss << target;
  }

  scanTargets.push_back(ss.str());

  // Start scanning for changed files.
  Start();
}

bool cMediaManager::UpdateContainerUpdateId(const string& objectID, long int updateID){
  stringstream update;

  update << "UPDATE " << db::Metadata << " SET "
         << " `" << property::object::KEY_OBJECT_UPDATE_ID << "`=" << updateID
         << " WHERE `" << property::object::KEY_OBJECTID << "`"
         << " = :objectID";

  try {
    tntdb::Statement stmt = connection.prepare(update.str());

    stmt.setString("objectID", objectID);

    if(stmt.execute() == 0){
      isyslog("UPnP\tContainer with ID '%s' not found. Cannot update it.", objectID.c_str());
      return false;
    }

    return true;

  } catch (const std::exception& e) {
    esyslog("UPnP\tException occurred while updating container with ID '%s': %s", objectID.c_str(), e.what());

    return false;
  }

  return false;

}

StringList cMediaManager::GetSearchCapabilities() const {
  StringList list;

//  list.push_back(property::object::KEY_TITLE);
//  list.push_back(property::object::KEY_CREATOR);
//  list.push_back(property::object::KEY_DESCRIPTION);
//  list.push_back(property::object::KEY_LONG_DESCRIPTION);
//  list.push_back(property::object::KEY_CLASS);
//  list.push_back(property::object::KEY_DATE);
//  list.push_back(property::object::KEY_LANGUAGE);
//  list.push_back(property::resource::KEY_PROTOCOL_INFO);

  return list;
}

StringList cMediaManager::GetSortCapabilities() const {
  StringList list;

  list.push_back(property::object::KEY_TITLE);
  list.push_back(property::object::KEY_CREATOR);
  list.push_back(property::object::KEY_DESCRIPTION);
  list.push_back(property::object::KEY_LONG_DESCRIPTION);
  list.push_back(property::object::KEY_CLASS);
  list.push_back(property::object::KEY_DATE);
  list.push_back(property::object::KEY_LANGUAGE);
  list.push_back(property::resource::KEY_PROTOCOL_INFO);

  return list;
}

StringList cMediaManager::GetSupportedProtocolInfos() const {
  tntdb::Connection conn = connection;

  stringstream ss;

  ss << "SELECT DISTINCT `" << property::resource::KEY_PROTOCOL_INFO << "` FROM " << db::Resources;

  StringList list;

  try {
    tntdb::Statement stmt = conn.prepare(ss.str());

    for(tntdb::Statement::const_iterator it = stmt.begin(); it != stmt.end(); ++it){
      tntdb::Row row = (*it);
      list.push_back(row.getString(property::resource::KEY_PROTOCOL_INFO));
    }

  } catch (const std::exception& e) {
    esyslog("UPnP\tException occurred while getting protocol infos: %s", e.what());
  }

  return list;
}

int cMediaManager::CreateResponse(MediaRequest& request, const string& select, const string& count){
  stringstream resources, details;

  resources << "SELECT * FROM " << db::Resources << " WHERE "
            << "`" << property::object::KEY_OBJECTID << "` = "
            << ":objectID";

  details   << "SELECT * FROM " << db::Details << " WHERE "
            << "`" << property::object::KEY_OBJECTID << "` = "
            << ":objectID";

  IXML_Document* DIDLDoc = NULL;

  try {
    tntdb::Statement select1 = connection.prepare(select);
    tntdb::Result result = connection.select(count);
    tntdb::Statement select2 = connection.prepare(resources.str());
    tntdb::Statement select3 = connection.prepare(details.str());

    StringList filterList = cFilterCriteria::parse(request.filter);

    request.numberReturned = 0;
    request.updateID = 0;
    request.totalMatches = result.getRow(0).getInt32("totalMatches");

    if(ixmlParseBufferEx(DIDLFragment, &DIDLDoc)==IXML_SUCCESS){

      IXML_Node* root = ixmlNode_getFirstChild((IXML_Node*) DIDLDoc);

      tntdb::Row row, row2, row3;

      for(tntdb::Statement::const_iterator it = select1.begin(); it != select1.end(); ++it){

        row = (*it);

        IXML_Element* object;
        string upnpClass = row.getString(property::object::KEY_CLASS);

        bool isContainer;


        if(upnpClass.find("object.item",0) == 0){
          object = ixmlDocument_createElement(DIDLDoc, "item");
          isContainer = false;
        } else if(upnpClass.find("object.container",0) == 0) {
          object = ixmlDocument_createElement(DIDLDoc, "container");
          isContainer = true;
        } else {
          goto error;
        }
        ixmlNode_appendChild(root, (IXML_Node*)object);

        string objectID = row.getString(property::object::KEY_OBJECTID);

        LOG(5, "Added new %s with objectID '%s' (%s) to result set.", (isContainer) ? "container" : "item", objectID.c_str(), row.getString(property::object::KEY_TITLE).c_str());

        ixml::IxmlAddProperty(DIDLDoc, object, property::object::KEY_OBJECTID, objectID);
        ixml::IxmlAddProperty(DIDLDoc, object, property::object::KEY_PARENTID, row.getString(property::object::KEY_PARENTID));
        ixml::IxmlAddProperty(DIDLDoc, object, property::object::KEY_RESTRICTED, row.getString(property::object::KEY_RESTRICTED));
        ixml::IxmlAddProperty(DIDLDoc, object, property::object::KEY_TITLE, row.getString(property::object::KEY_TITLE).substr(0, MAX_METADATA_LENGTH_S));
        ixml::IxmlAddProperty(DIDLDoc, object, property::object::KEY_CLASS, row.getString(property::object::KEY_CLASS).substr(0, MAX_METADATA_LENGTH_S));

        if(isContainer){
          ixml::IxmlAddFilteredProperty(filterList, DIDLDoc, object, property::object::KEY_CHILD_COUNT, row.getString(property::object::KEY_CHILD_COUNT));
        }
        else {
          ixml::IxmlAddFilteredProperty(filterList, DIDLDoc, object, property::object::KEY_CHANNEL_NR, row.getString(property::object::KEY_CHANNEL_NR));
          ixml::IxmlAddFilteredProperty(filterList, DIDLDoc, object, property::object::KEY_CHANNEL_NAME, row.getString(property::object::KEY_CHANNEL_NAME));
          ixml::IxmlAddFilteredProperty(filterList, DIDLDoc, object, property::object::KEY_SCHEDULED_START, row.getString(property::object::KEY_SCHEDULED_START));
          ixml::IxmlAddFilteredProperty(filterList, DIDLDoc, object, property::object::KEY_SCHEDULED_END, row.getString(property::object::KEY_SCHEDULED_END));
        }

        ixml::IxmlAddFilteredProperty(filterList, DIDLDoc, object, property::object::KEY_CREATOR, row.getString(property::object::KEY_CREATOR));
        ixml::IxmlAddFilteredProperty(filterList, DIDLDoc, object, property::object::KEY_DESCRIPTION, row.getString(property::object::KEY_DESCRIPTION));
        ixml::IxmlAddFilteredProperty(filterList, DIDLDoc, object, property::object::KEY_LONG_DESCRIPTION, row.getString(property::object::KEY_LONG_DESCRIPTION));
        ixml::IxmlAddFilteredProperty(filterList, DIDLDoc, object, property::object::KEY_DATE, row.getString(property::object::KEY_DATE));
        ixml::IxmlAddFilteredProperty(filterList, DIDLDoc, object, property::object::KEY_LANGUAGE, row.getString(property::object::KEY_LANGUAGE));


        select2.setString("objectID", objectID);

        int i=0;
        string resourceFile;
        string resourceURI;
        stringstream tntnet;
        for(tntdb::Statement::const_iterator it2 = select2.begin(); it2 != select2.end(); ++it2){
            row2 = (*it2);

            string resourceFile = row2.getString(property::resource::KEY_RESOURCE);
            boost::shared_ptr<cUPnPResourceProvider> provider(CreateResourceProvider(resourceFile));

            tntnet.str(string());
            if(provider.get()){
              resourceURI = provider->GetHTTPUri(resourceFile, cMediaServer::GetInstance()->GetServerIPAddress(), row2.getString(property::resource::KEY_PROTOCOL_INFO));
              if(resourceURI.empty()){
                tntnet << cMediaServer::GetInstance()->GetWebserver().GetBaseUrl() << "getStream?objectID=" << objectID << "&resourceID=" << i++;
                resourceURI = tntnet.str();
              }
            } else if(resourceFile.find("thumb://",0) == 0 && !resourceFile.substr(8).empty()) {
              tntnet << cMediaServer::GetInstance()->GetWebserver().GetBaseUrl() << "thumbs/" << resourceFile.substr(8);
              resourceURI = tntnet.str();
            }

            IXML_Element* resource = ixml::IxmlAddFilteredProperty(filterList, DIDLDoc, object, property::resource::KEY_RESOURCE, resourceURI);

            if(resource){
              ixml::IxmlAddFilteredProperty(filterList, DIDLDoc, resource, property::resource::KEY_PROTOCOL_INFO, row2.getString(property::resource::KEY_PROTOCOL_INFO));
              ixml::IxmlAddFilteredProperty(filterList, DIDLDoc, resource, property::resource::KEY_BITRATE, row2.getString(property::resource::KEY_BITRATE));
              ixml::IxmlAddFilteredProperty(filterList, DIDLDoc, resource, property::resource::KEY_BITS_PER_SAMPLE, row2.getString(property::resource::KEY_BITS_PER_SAMPLE));
              ixml::IxmlAddFilteredProperty(filterList, DIDLDoc, resource, property::resource::KEY_COLOR_DEPTH, row2.getString(property::resource::KEY_COLOR_DEPTH));
              ixml::IxmlAddFilteredProperty(filterList, DIDLDoc, resource, property::resource::KEY_DURATION, row2.getString(property::resource::KEY_DURATION));
              ixml::IxmlAddFilteredProperty(filterList, DIDLDoc, resource, property::resource::KEY_NR_AUDIO_CHANNELS, row2.getString(property::resource::KEY_NR_AUDIO_CHANNELS));
              ixml::IxmlAddFilteredProperty(filterList, DIDLDoc, resource, property::resource::KEY_RESOLUTION, row2.getString(property::resource::KEY_RESOLUTION));
              ixml::IxmlAddFilteredProperty(filterList, DIDLDoc, resource, property::resource::KEY_SAMPLE_FREQUENCY, row2.getString(property::resource::KEY_SAMPLE_FREQUENCY));
              ixml::IxmlAddFilteredProperty(filterList, DIDLDoc, resource, property::resource::KEY_SIZE, tools::ToString(row2.getInt64(property::resource::KEY_SIZE)));
            }

        }

        select3.setString("objectID", objectID);

        for(tntdb::Statement::const_iterator it3 = select3.begin(); it3 != select2.end(); ++it3){
          row3 = (*it3);

          ixml::IxmlAddFilteredProperty(filterList, DIDLDoc, object, row3.getString("property"), row3.getString("value"));
        }

        ++request.numberReturned;
      }

      request.result = ixmlNodetoString((IXML_Node*)DIDLDoc);

      ixmlDocument_free(DIDLDoc);
      return UPNP_E_SUCCESS;
    }
  } catch (const std::exception& e) {
    esyslog("UPnP\tException occurred while creating response for object '%s': %s",
       request.objectID.c_str(), e.what());
  }

  error:
  esyslog("UPnP\tFailed to process the request");
  ixmlDocument_free(DIDLDoc);
  return UPNP_CDS_E_CANT_PROCESS_REQUEST;
}

int cMediaManager::Browse(BrowseRequest& request){

  LOG(5, "Browse request for ObjectID = '%s', filter: '%s', sort: '%s', from: %d count: %d",
         request.objectID.c_str(),
         request.filter.c_str(),
         request.sortCriteria.c_str(),
         request.startIndex,
         request.requestCount);

  stringstream metadata, count, where;

  metadata << "SELECT *,(SELECT COUNT(1) FROM " << db::Metadata << " m WHERE "
      << "m.`" << property::object::KEY_PARENTID << "` = "
      << "p.`" << property::object::KEY_OBJECTID << "`) as "
      << "`" << property::object::KEY_CHILD_COUNT << "`,"
      << "r.`" << property::resource::KEY_PROTOCOL_INFO << "` FROM " << db::Metadata << " p "
      << "LEFT JOIN " << db::Resources << " r USING (`" << property::object::KEY_OBJECTID << "`) WHERE ";

  count << "SELECT COUNT(1) as totalMatches FROM " << db::Metadata << " WHERE ";

  switch (request.browseMetadata){
  case CD_BROWSE_METADATA:
    where << "`" << property::object::KEY_OBJECTID << "`";

    // Set the offset and count to 0,1 as this is the only allowed option here.
    request.requestCount = 1;
    request.startIndex = 0;
    break;
  case CD_BROWSE_DIRECT_CHILDREN:
    where << "`" << property::object::KEY_PARENTID << "`";

    // Limit the number of results to reduce response time and SOAP message size.
    if(request.requestCount == 0 || request.requestCount > 30)
      request.requestCount = 30;

    break;
  default:
    esyslog("UPnP\tInvalid arguments. Browse flag invalid");
    return UPNP_SOAP_E_INVALID_ARGS;
  }

  where << " = '" << request.objectID << "'";

  metadata << where.str();
  count << where.str();

  cSortCriteria::SortCriteriaList list = cSortCriteria::parse(request.sortCriteria);
  if(!list.empty()){
    metadata << " ORDER BY `";
    upnp::cSortCriteria::SortCriteriaList::iterator it = list.begin();
    metadata << (*it).property << "` " << ((*it).sortDescending ? "DESC" : "ASC");
    for(++it; it != list.end(); ++it){
      metadata << ", `" << (*it).property << "` " << ((*it).sortDescending ? "DESC" : "ASC");
    }
  }

  if(request.requestCount){
    metadata << " LIMIT " << request.startIndex << ", " << request.requestCount;
  }

  int ret = 0;
  if((ret = CreateResponse(request, metadata.str(), count.str())) != UPNP_E_SUCCESS) return ret;

  LOG(5, "Found %d matches, returning %d", request.totalMatches, request.numberReturned);

  if(request.totalMatches == 0 && request.numberReturned == 0){
    dsyslog("Container %s is empty.", request.objectID);
    return UPNP_CDS_E_CANT_PROCESS_REQUEST;
  } else {
    return UPNP_E_SUCCESS;
  }
}

int cMediaManager::Search(SearchRequest& request){
  request.numberReturned = 0;
  request.totalMatches = 0;
  request.updateID = 0;

  stringstream metadata, count;

  // TODO: Finish search method

  int ret = 0;
  if((ret = CreateResponse(request, metadata.str(), count.str())) != UPNP_E_SUCCESS) return ret;

  return (request.totalMatches == 0 && request.numberReturned == 0) ? UPNP_CDS_E_CANT_PROCESS_REQUEST : UPNP_E_SUCCESS;
}

cMediaManager::BrowseFlag cMediaManager::ToBrowseFlag(const std::string& browseFlag) {
  if      (browseFlag.compare("BrowseMetadata") == 0)
    return CD_BROWSE_METADATA;
  else if (browseFlag.compare("BrowseDirectChildren") == 0)
    return CD_BROWSE_DIRECT_CHILDREN;
  else
    return NumBrowseFlags;
}

void cMediaManager::Housekeeping(){
}

bool cMediaManager::Initialise(){

  try {
    stringstream ss;
    ss << "sqlite:" << databaseFile;

    connection = tntdb::connect(ss.str());

    LOG(2, "Preparing database structure...");

    if(!CheckIntegrity()){
      try {

        connection.beginTransaction();

        ss.str(string());

        ss << "CREATE TABLE " << db::Metadata
           << "("
           << "`" << property::object::KEY_OBJECTID          << "` TEXT    PRIMARY KEY,"
           << "`" << property::object::KEY_PARENTID          << "` TEXT    NOT NULL,"
           << "`" << property::object::KEY_TITLE             << "` TEXT    NOT NULL,"
           << "`" << property::object::KEY_CLASS             << "` TEXT    NOT NULL,"
           << "`" << property::object::KEY_RESTRICTED        << "` INTEGER NOT NULL,"
           << "`" << property::object::KEY_CREATOR           << "` TEXT,"
           << "`" << property::object::KEY_DESCRIPTION       << "` TEXT,"
           << "`" << property::object::KEY_LONG_DESCRIPTION  << "` TEXT,"
           << "`" << property::object::KEY_DATE              << "` TEXT,"
           << "`" << property::object::KEY_LANGUAGE          << "` TEXT,"
           << "`" << property::object::KEY_CHANNEL_NR        << "` INTEGER,"
           << "`" << property::object::KEY_CHANNEL_NAME      << "` TEXT,"
           << "`" << property::object::KEY_SCHEDULED_START   << "` TEXT,"
           << "`" << property::object::KEY_SCHEDULED_END     << "` TEXT,"
           << "`" << property::object::KEY_OBJECT_UPDATE_ID  << "` INTEGER"
           << ")";

        tntdb::Statement objectTable = connection.prepare(ss.str());

        objectTable.execute();

        ss.str(string());

        ss << "CREATE TABLE " << db::Details
           << "("
           << "  `propertyID` INTEGER PRIMARY KEY,"
           << "  `" << property::object::KEY_OBJECTID << "` TEXT "
           << "  REFERENCES metadata (`"<< property::object::KEY_OBJECTID <<"`) ON DELETE CASCADE ON UPDATE CASCADE,"
           << "  `property`   TEXT,"
           << "  `value`      TEXT"
           << ")";

        tntdb::Statement detailsTable = connection.prepare(ss.str());

        detailsTable.execute();

        ss.str(string());

        ss << "CREATE TABLE " << db::Resources
           << "("
           << "`" << property::object::KEY_OBJECTID << "` TEXT "
           << "REFERENCES metadata (`"<< property::object::KEY_OBJECTID <<"`) ON DELETE CASCADE ON UPDATE CASCADE,"
           << "`" << property::resource::KEY_RESOURCE           << "` TEXT,"
           << "`" << property::resource::KEY_PROTOCOL_INFO      << "` TEXT    NOT NULL,"
           << "`" << property::resource::KEY_SIZE               << "` INTEGER,"
           << "`" << property::resource::KEY_DURATION           << "` TEXT,"
           << "`" << property::resource::KEY_RESOLUTION         << "` TEXT,"
           << "`" << property::resource::KEY_BITRATE            << "` INTEGER,"
           << "`" << property::resource::KEY_SAMPLE_FREQUENCY   << "` INTEGER,"
           << "`" << property::resource::KEY_BITS_PER_SAMPLE    << "` INTEGER,"
           << "`" << property::resource::KEY_NR_AUDIO_CHANNELS  << "` INTEGER,"
           << "`" << property::resource::KEY_COLOR_DEPTH        << "` INTEGER,"
           << "PRIMARY KEY ("
           << "`" << property::object::KEY_OBJECTID             << "`,"
           << "`" << property::resource::KEY_RESOURCE           << "`"
           << ")"
           << ")";

        tntdb::Statement resourcesTable = connection.prepare(ss.str());

        resourcesTable.execute();

        ss.str(string());

        ss << "INSERT INTO " << db::Metadata << " ("
           << "`" << property::object::KEY_OBJECTID          << "`, "
           << "`" << property::object::KEY_PARENTID          << "`, "
           << "`" << property::object::KEY_TITLE             << "`, "
           << "`" << property::object::KEY_CLASS             << "`, "
           << "`" << property::object::KEY_RESTRICTED        << "`, "
           << "`" << property::object::KEY_CREATOR           << "`, "
           << "`" << property::object::KEY_DESCRIPTION       << "`, "
           << "`" << property::object::KEY_LONG_DESCRIPTION  << "`) "
           << " VALUES (:objectID, :parentID, :title, :class, :restricted, :creator, :description, :longDescription)";

        tntdb::Statement rootContainer = connection.prepare(ss.str());

        const cMediaServer::Description desc = cMediaServer::GetInstance()->GetServerDescription();

        rootContainer.setString("objectID", "0")
                     .setString("parentID", "-1")
                     .setString("title", desc.friendlyName)
                     .setString("creator", desc.manufacturer)
                     .setString("class", "object.container")
                     .setBool("restricted", true)
                     .setString("description", desc.modelName)
                     .setString("longDescription", desc.modelDescription)
                     .execute();

        connection.commitTransaction();

      } catch (const std::exception& e) {
        esyslog("UPnP\tException occurred while initializing database '%s': %s", databaseFile.c_str(), e.what());
        connection.rollbackTransaction();

        return false;
      }

    }

  } catch (const std::exception& e) {
    esyslog("UPnP\tException occurred while connecting to database '%s': %s", databaseFile.c_str(), e.what());

    return false;
  }

  dsyslog("UPnP\tLoading Plugins...");
  pluginManager = new upnp::cPluginManager();

  if(!pluginManager->LoadPlugins()){
    esyslog("UPnP\tError while loading upnp plugins");
    return false;
  } else {
    dsyslog("UPnP\tFound %d plugins", pluginManager->Count());
  }

  dsyslog("UPnP\tScanning directories...");
  // Do an full initial scan on startup.
  upnp::cPluginManager::ProviderList providers = pluginManager->GetProviders();
  for(upnp::cPluginManager::ProviderList::iterator it = providers.begin(); it != providers.end(); ++it){
    scanTargets.push_back((*it)->GetRootContainer());
  }
  Start();

  return true;
}

bool cMediaManager::CheckIntegrity(){

  connection.execute("PRAGMA foreign_keys = ON");
  connection.execute("PRAGMA page_size = 4096");
  connection.execute("PRAGMA cache_size = 16384");
  connection.execute("PRAGMA temp_store = MEMORY");
  connection.execute("PRAGMA synchronous = NORMAL");
  connection.execute("PRAGMA locking_mode = EXCLUSIVE");

  tntdb::Statement checkTable = connection.prepare(
          "SELECT name FROM sqlite_master WHERE type='table' AND name=:table;"
          );

  if( checkTable.setString("table", db::Metadata).select().empty() ){
    isyslog("UPnP\tTable '%s' does not exist", db::Metadata);
    return false;
  }
  if( checkTable.setString("table", db::Details).select().empty() ){
    isyslog("UPnP\tTable '%s' does not exist", db::Details);
    return false;
  }
  if( checkTable.setString("table", db::Resources).select().empty() ){
    isyslog("UPnP\tTable '%s' does not exist", db::Resources);
    return false;
  }

  stringstream ss;

  ss << "SELECT `" << property::object::KEY_OBJECTID << "` FROM " << db::Metadata << " WHERE `"
                  << property::object::KEY_OBJECTID << "` = '0' AND `"
                  << property::object::KEY_PARENTID << "` = '-1';";

  tntdb::Statement checkObject = connection.prepare(ss.str());

  if( checkObject.select().size() != 1 ){
    isyslog("UPnP\tRoot item does not exist or more than one root item exist.");
    return false;
  }

  return true;
}

cResourceStreamer* cMediaManager::GetResourceStreamer(const string& objectID, int resourceID){
  LOG(5, "Try to stream resource[%d] of objectID %s", resourceID, objectID.c_str());

  stringstream resourceSQL;

  resourceSQL << "SELECT * FROM " << db::Resources << " WHERE "
              << "`" << property::object::KEY_OBJECTID << "` = "
              << ":objectID"
              << " ORDER BY ROWID ASC LIMIT " << resourceID << ",1";

  tntdb::Statement select = connection.prepare(resourceSQL.str());

  tntdb::Result result = select.setString("objectID", objectID)
                               .select();

  if(result.size() == 0) return NULL;

  tntdb::Row row = result.getRow(0);

  cMetadata::Resource* resource = new cMetadata::Resource();

  bool ret = true;
  if(!row.isNull(property::resource::KEY_BITRATE))
    ret = resource->SetBitrate(row.getInt32(property::resource::KEY_BITRATE));
  if(!row.isNull(property::resource::KEY_BITS_PER_SAMPLE))
    ret = resource->SetBitsPerSample(row.getInt32(property::resource::KEY_BITS_PER_SAMPLE));
  if(!row.isNull(property::resource::KEY_COLOR_DEPTH))
    ret = resource->SetColorDepth(row.getInt32(property::resource::KEY_COLOR_DEPTH));
  if(!row.isNull(property::resource::KEY_DURATION))
    ret = resource->SetDuration(row.getString(property::resource::KEY_DURATION));
  if(!row.isNull(property::resource::KEY_NR_AUDIO_CHANNELS))
    ret = resource->SetNrAudioChannels(row.getInt32(property::resource::KEY_NR_AUDIO_CHANNELS));
  if(!row.isNull(property::resource::KEY_PROTOCOL_INFO))
    ret = resource->SetProtocolInfo(row.getString(property::resource::KEY_PROTOCOL_INFO));
  if(!row.isNull(property::resource::KEY_RESOLUTION))
    ret = resource->SetResolution(row.getString(property::resource::KEY_RESOLUTION));
  if(!row.isNull(property::resource::KEY_RESOURCE))
    ret = resource->SetResourceUri(row.getString(property::resource::KEY_RESOURCE));
  if(!row.isNull(property::resource::KEY_SAMPLE_FREQUENCY))
    ret = resource->SetSampleFrequency(row.getInt32(property::resource::KEY_SAMPLE_FREQUENCY));
  if(!row.isNull(property::resource::KEY_SIZE))
    ret = resource->SetSize(row.getInt64(property::resource::KEY_SIZE));

  if(!ret) {
    delete resource;
    return NULL;
  }

  cUPnPResourceProvider* provider = CreateResourceProvider(resource->GetResourceUri());

  cResourceStreamer* streamer = new cResourceStreamer(this, provider, resource);

  return streamer;
}

cUPnPResourceProvider* cMediaManager::CreateResourceProvider(const string& uri){
  return pluginManager->CreateProvider(uri.substr(0, uri.find_first_of(':',0)));
}

void cMediaManager::SetDatabaseDir(const string& file){
  if(file.empty())
    databaseFile = cPlugin::ConfigDirectory(PLUGIN_NAME_I18N);
  else databaseFile = file;

  databaseFile += "/metadata.db";
}

void cMediaManager::Action(){
  string uri;
  while(!scanTargets.empty()){
    uri = scanTargets.front();
    boost::shared_ptr<cUPnPResourceProvider> provider(CreateResourceProvider(uri));
    if(!ScanURI(uri, provider.get())){
      //isyslog("UPnP\tAn error occured while scanning '%s'!", uri.c_str());
    }
    scanTargets.pop_front();
  }
}

bool cMediaManager::ScanURI(const string& uri, cUPnPResourceProvider* provider){
  if (provider == NULL) return false;

  cMetadata metadata;

  if(!provider->IsContainer(uri)){
    cPluginManager::ProfilerList profilers = pluginManager->GetProfilers();

    string schema = uri.substr(0, uri.find_first_of(':',0));
    for(cPluginManager::ProfilerList::iterator it = profilers.begin(); it != profilers.end(); ++it){
      if((*it)->CanHandleSchema(schema)){
        if((*it)->GetMetadata(uri, metadata, provider) && RefreshObject(metadata)){
          return true;
        }
      }
    }

    return false;

  } else {
    if(!provider->GetMetadata(uri, metadata)){
      isyslog("UPnP\tUnable to get the metadata of '%s'", uri.c_str());
      return false;
    }

    if(!RefreshObject(metadata)){
      isyslog("UPnP\tUnable to save the metadata of '%s'", uri.c_str());
      return false;
    }

    StringList entries = provider->GetContainerEntries(uri);
    stringstream ss;

    ss << "SELECT `" << property::object::KEY_OBJECTID << "` FROM " << db::Metadata
       << " WHERE `" << property::object::KEY_PARENTID << "` = :parentID";

    StringList intersection;

    try {
      tntdb::Statement objects = connection.prepare(ss.str());
      objects.setString("parentID", tools::GenerateUUIDFromURL(uri));

      StringList::iterator rit;
      string deletableID, entryID;
      for(tntdb::Statement::const_iterator dit = objects.begin(); dit != objects.end(); ++dit){
        tntdb::Row row = (*dit);
        deletableID = row.getString(property::object::KEY_OBJECTID);
        for(rit = entries.begin(); rit != entries.end(); ++rit){
          entryID = tools::GenerateUUIDFromURL(uri + (*rit));
          if(entryID.compare(deletableID) == 0) break;
        }
        if(rit == entries.end())
          intersection.push_back(deletableID);
      }

    } catch (const std::exception& e) {
      esyslog("UPnP\tException occurred while getting objects from '%s' from database '%s': %s",
          tools::GenerateUUIDFromURL(uri).c_str(), databaseFile.c_str(), e.what());

      return false;
    }

    if(intersection.size() > 0){
      ss.str(string());
      ss << "DELETE FROM " << db::Metadata << " WHERE "
         << " `" << property::object::KEY_PARENTID << "`"
         << " = '" << tools::GenerateUUIDFromURL(uri) << "'";

      for(StringList::iterator it = intersection.begin(); it != intersection.end(); ++it){
        ss << " AND"
           << " `" << property::object::KEY_OBJECTID << "`"
           << " == '" << *it << "'";
      }

      try {
        tntdb::Statement objects = connection.prepare(ss.str());
        objects.execute();
      } catch (const std::exception& e) {
        esyslog("UPnP\tException occurred while removing old object in '%s' from database '%s': %s",
            tools::GenerateUUIDFromURL(uri).c_str(), databaseFile.c_str(), e.what());

        return false;
      }
    }

    string entryUri;
    for(StringList::iterator it = entries.begin(); it != entries.end(); ++it){
      entryUri = uri + *it;
      ScanURI(entryUri, provider);
    }
  }

  return true;
}

bool cMediaManager::RefreshObject(cMetadata& metadata){
  stringstream ss;

  string objectID = metadata.GetPropertyByKey(property::object::KEY_OBJECTID).GetString();

  try {

    connection.beginTransaction();

    ss << "INSERT OR IGNORE INTO " << db::Metadata << " ("
       << "`" << property::object::KEY_OBJECTID          << "`,"
       << "`" << property::object::KEY_PARENTID          << "`,"
       << "`" << property::object::KEY_TITLE             << "`,"
       << "`" << property::object::KEY_CLASS             << "`,"
       << "`" << property::object::KEY_RESTRICTED        << "`"
       << ") VALUES ("
       << ":objectID, :parentID, :title, :class, :restricted"
       << "); ";

    tntdb::Statement insert = connection.prepare(ss.str());

    insert.setString("objectID", objectID)
          .setString("parentID", metadata.GetPropertyByKey(property::object::KEY_PARENTID).GetString())
          .setString("title",    metadata.GetPropertyByKey(property::object::KEY_TITLE).GetString())
          .setString("class",    metadata.GetPropertyByKey(property::object::KEY_CLASS).GetString())
          .setBool  ("restricted", metadata.GetPropertyByKey(property::object::KEY_RESTRICTED).GetBoolean())
          .execute();

    ss.str(string());

    ss << "UPDATE " << db::Metadata << " SET "
       << "`" << property::object::KEY_OBJECTID          << "` = :objectID,"
       << "`" << property::object::KEY_PARENTID          << "` = :parentID,"
       << "`" << property::object::KEY_TITLE             << "` = :title,"
       << "`" << property::object::KEY_CLASS             << "` = :class,"
       << "`" << property::object::KEY_RESTRICTED        << "` = :restricted,"
       << "`" << property::object::KEY_CREATOR           << "` = :creator,"
       << "`" << property::object::KEY_DESCRIPTION       << "` = :description,"
       << "`" << property::object::KEY_LONG_DESCRIPTION  << "` = :longDescription,"
       << "`" << property::object::KEY_DATE              << "` = :date,"
       << "`" << property::object::KEY_LANGUAGE          << "` = :language,"
       << "`" << property::object::KEY_CHANNEL_NR        << "` = :channelNr,"
       << "`" << property::object::KEY_CHANNEL_NAME      << "` = :channelName,"
       << "`" << property::object::KEY_SCHEDULED_START   << "` = :start,"
       << "`" << property::object::KEY_SCHEDULED_END     << "` = :end"
       << " WHERE "
       << "`" << property::object::KEY_OBJECTID          << "` = :objectID"
       << ";";

    tntdb::Statement object = connection.prepare(ss.str());

    const cMediaServer::Description desc = cMediaServer::GetInstance()->GetServerDescription();

    object.setString("objectID", objectID)
          .setString("parentID", metadata.GetPropertyByKey(property::object::KEY_PARENTID).GetString())
          .setString("title",    metadata.GetPropertyByKey(property::object::KEY_TITLE).GetString())
          .setString("class",    metadata.GetPropertyByKey(property::object::KEY_CLASS).GetString())
          .setBool  ("restricted", metadata.GetPropertyByKey(property::object::KEY_RESTRICTED).GetBoolean());

    (!metadata.GetPropertyByKey(property::object::KEY_CREATOR).IsEmpty()) ?
    object.setString("creator", metadata.GetPropertyByKey(property::object::KEY_CREATOR).GetString()) :
    object.setNull("creator");

    (!metadata.GetPropertyByKey(property::object::KEY_DESCRIPTION).IsEmpty()) ?
    object.setString("description", metadata.GetPropertyByKey(property::object::KEY_DESCRIPTION).GetString()) :
    object.setNull("description");

    (!metadata.GetPropertyByKey(property::object::KEY_LONG_DESCRIPTION).IsEmpty()) ?
    object.setString("longDescription", metadata.GetPropertyByKey(property::object::KEY_LONG_DESCRIPTION).GetString()) :
    object.setNull("longDescription");

    (!metadata.GetPropertyByKey(property::object::KEY_DATE).IsEmpty()) ?
    object.setString("date", metadata.GetPropertyByKey(property::object::KEY_DATE).GetString()) :
    object.setNull("date");

    (!metadata.GetPropertyByKey(property::object::KEY_LANGUAGE).IsEmpty()) ?
    object.setString("language", metadata.GetPropertyByKey(property::object::KEY_LANGUAGE).GetString()) :
    object.setNull("language");

    (!metadata.GetPropertyByKey(property::object::KEY_CHANNEL_NR).IsEmpty()) ?
    object.setInt("channelNr", metadata.GetPropertyByKey(property::object::KEY_CHANNEL_NR).GetInteger()) :
    object.setNull("channelNr");

    (!metadata.GetPropertyByKey(property::object::KEY_CHANNEL_NAME).IsEmpty()) ?
    object.setString("channelName", metadata.GetPropertyByKey(property::object::KEY_CHANNEL_NAME).GetString()) :
    object.setNull("channelName");

    (!metadata.GetPropertyByKey(property::object::KEY_SCHEDULED_START).IsEmpty()) ?
    object.setString("start", metadata.GetPropertyByKey(property::object::KEY_SCHEDULED_START).GetString()) :
    object.setNull("start");

    (!metadata.GetPropertyByKey(property::object::KEY_SCHEDULED_END).IsEmpty()) ?
    object.setString("end", metadata.GetPropertyByKey(property::object::KEY_SCHEDULED_END).GetString()) :
    object.setNull("end");

    object.execute();

    stringstream resourcestr;

    resourcestr << "INSERT OR IGNORE INTO " << db::Resources << " ("
                << "`" << property::object::KEY_OBJECTID            << "`, "
                << "`" << property::resource::KEY_RESOURCE          << "`,"
                << "`" << property::resource::KEY_PROTOCOL_INFO     << "`"
                << ") VALUES ( "
                << ":objectID, :resource, :protocolInfo"
                << ")";

    tntdb::Statement resourcestmt1 = connection.prepare(resourcestr.str());

    resourcestr.str(string());

    resourcestr << "UPDATE " << db::Resources << " SET "
                << "`" << property::object::KEY_OBJECTID            << "` = :objectID, "
                << "`" << property::resource::KEY_RESOURCE          << "` = :resource,"
                << "`" << property::resource::KEY_PROTOCOL_INFO     << "` = :protocolInfo,"
                << "`" << property::resource::KEY_SIZE              << "` = :size,"
                << "`" << property::resource::KEY_DURATION          << "` = :duration,"
                << "`" << property::resource::KEY_RESOLUTION        << "` = :resolution,"
                << "`" << property::resource::KEY_BITRATE           << "` = :bitrate,"
                << "`" << property::resource::KEY_SAMPLE_FREQUENCY  << "` = :sampleFreq,"
                << "`" << property::resource::KEY_BITS_PER_SAMPLE   << "` = :bpSample,"
                << "`" << property::resource::KEY_NR_AUDIO_CHANNELS << "` = :nrChannels,"
                << "`" << property::resource::KEY_COLOR_DEPTH       << "` = :colorDepth"
                << " WHERE "
                << "`" << property::object::KEY_OBJECTID            << "` = :objectID"
                << " AND "
                << "`" << property::resource::KEY_RESOURCE          << "` = :resource"
                << ";";

    tntdb::Statement resourcestmt2 = connection.prepare(resourcestr.str());

    resourcestr.str(string());

    resourcestr << "DELETE FROM " << db::Resources << " WHERE "
                << "`" << property::object::KEY_OBJECTID << "`"
                << " = :objectID";

    cMetadata::ResourceList resources = metadata.GetResources();

    int t = 1;
    for(cMetadata::ResourceList::iterator it = resources.begin(); it != resources.end(); ++it){
      // This is appended to the delete statement and will delete all resources, which are not in the set.
      // The resources are identified by their resource URI. Therefore: two resources with same URI refer
      // to the same file or stream.
      resourcestr << " AND"
                  << " `" << property::resource::KEY_RESOURCE << "`"
                  << " != :resource" << t;
    }

    tntdb::Statement delresourcestmt = connection.prepare(resourcestr.str());
    delresourcestmt.setString("objectID", objectID);
    t = 1;

    for(cMetadata::ResourceList::iterator it = resources.begin(); it != resources.end(); ++it){

      delresourcestmt.setString(string("resource") + tools::ToString(t), (*it).GetResourceUri());

      resourcestmt1.setString("objectID", objectID)
                   .setString("resource", (*it).GetResourceUri())
                   .setString("protocolInfo", (*it).GetProtocolInfo())
                   .execute();

      resourcestmt2.setString("objectID", objectID)
                   .setString("resource", (*it).GetResourceUri())
                   .setString("protocolInfo", (*it).GetProtocolInfo());

      ((*it).GetSize()) ?
            resourcestmt2.setLong("size",(*it).GetSize()) :
            resourcestmt2.setNull("size");

      (!(*it).GetDuration().empty()) ?
            resourcestmt2.setString("duration",(*it).GetDuration()) :
            resourcestmt2.setNull("duration");

      (!(*it).GetResolution().empty()) ?
            resourcestmt2.setString("resolution",(*it).GetResolution()) :
            resourcestmt2.setNull("resolution");

      ((*it).GetBitrate()) ?
            resourcestmt2.setInt("bitrate",(*it).GetBitrate()) :
            resourcestmt2.setNull("bitrate");

      ((*it).GetSampleFrequency()) ?
            resourcestmt2.setInt("sampleFreq",(*it).GetSampleFrequency()) :
            resourcestmt2.setNull("sampleFreq");

      ((*it).GetBitsPerSample()) ?
            resourcestmt2.setInt("bpSample",(*it).GetBitsPerSample()) :
            resourcestmt2.setNull("bpSample");

      ((*it).GetNrAudioChannels()) ?
            resourcestmt2.setInt("nrChannels",(*it).GetNrAudioChannels()) :
            resourcestmt2.setNull("nrChannels");

      ((*it).GetColorDepth()) ?
            resourcestmt2.setInt("colorDepth",(*it).GetColorDepth()) :
            resourcestmt2.setNull("colorDepth");

      resourcestmt2.execute();
    }

    delresourcestmt.execute();

    stringstream detailstr;

    detailstr << "DELETE FROM " << db::Details << " WHERE "
              << "`" << property::object::KEY_OBJECTID << "`"
              << " = :objectID";

    tntdb::Statement detailstmt = connection.prepare(detailstr.str());

    detailstmt.setString("objectID", objectID)
              .execute();

    detailstr.str(string());

    detailstr << "INSERT INTO " << db::Details << " ("
              << " `" << property::object::KEY_OBJECTID << "`,"
              << " `property`,"
              << " `value`"
              << " ) VALUES ("
              << ":objectID, :property, :value"
              << ")";

    tntdb::Statement detailstmt2 = connection.prepare(detailstr.str());

    cMetadata::PropertyRange properties = metadata.GetAllProperties();
    for(cMetadata::PropertyMap::iterator it = properties.first; it != properties.second; ++it){
      if((*it).first.compare(property::object::KEY_OBJECTID) == 0 ||
         (*it).first.compare(property::object::KEY_PARENTID) == 0 ||
         (*it).first.compare(property::object::KEY_TITLE) == 0 ||
         (*it).first.compare(property::object::KEY_CLASS) == 0 ||
         (*it).first.compare(property::object::KEY_RESTRICTED) == 0 ||
         (*it).first.compare(property::object::KEY_CREATOR) == 0 ||
         (*it).first.compare(property::object::KEY_DESCRIPTION) == 0 ||
         (*it).first.compare(property::object::KEY_LONG_DESCRIPTION) == 0 ||
         (*it).first.compare(property::object::KEY_DATE) == 0 ||
         (*it).first.compare(property::object::KEY_LANGUAGE) == 0 ||
         (*it).first.compare(property::object::KEY_CHANNEL_NR) == 0 ||
         (*it).first.compare(property::object::KEY_CHANNEL_NAME) == 0 ||
         (*it).first.compare(property::object::KEY_SCHEDULED_START) == 0 ||
         (*it).first.compare(property::object::KEY_SCHEDULED_END) == 0) continue;

      detailstmt2.setString("objectID", objectID)
                 .setString("property", (*it).second.GetKey())
                 .setString("value", (*it).second.GetString())
                 .execute();
    }

    connection.commitTransaction();

  } catch (const std::exception& e) {
    esyslog("UPnP\tException occurred while storing object '%s' to database '%s': %s",
        objectID.c_str(), databaseFile.c_str(), e.what());

    connection.rollbackTransaction();

    return false;
  }

  return true;
}

}  // namespace upnp


