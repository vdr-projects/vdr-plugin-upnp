/*
 * mediaManager.cpp
 *
 *  Created on: 01.09.2012
 *      Author: savop
 */

#include "../include/plugin.h"
#include "../include/media/mediaManager.h"
#include "../include/server.h"
#include "../include/parser.h"
#include "../include/tools.h"
#include <upnp/upnp.h>
#include <sstream>
#include <tntdb/statement.h>
#include <tntdb/result.h>
#include <upnp/ixml.h>

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

cMediaManager::cMediaManager()
: mSystemUpdateID(0)
, mDatabaseFile("metadata.db")
{
}

cMediaManager::~cMediaManager(){
}

uint32_t cMediaManager::GetSystemUpdateID() const {
  return mSystemUpdateID;
}

IdList cMediaManager::GetContainerUpdateIDs(bool unevented){
  IdList list = mEventedContainerUpdateIDs;

  if(!unevented)
    mEventedContainerUpdateIDs.clear();

  return list;
}

void cMediaManager::OnContainerUpdate(string uri, long updateID){
  ++mSystemUpdateID;

  mEventedContainerUpdateIDs[tools::GenerateUUIDFromURL(uri)] = updateID;

  mScanDirectories.push_back(uri);

  // Start scanning for changed files.
  Start();
}

StringList cMediaManager::GetSearchCapabilities() const {
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
  tntdb::Connection conn = mConnection;

  stringstream ss;

  ss << "SELECT DISTINCT `" << property::resource::KEY_PROTOCOL_INFO << "` FROM " << db::Resources;

  tntdb::Statement stmt = conn.prepare(ss.str());

  StringList list;

  for(tntdb::Statement::const_iterator it = stmt.begin(); it != stmt.end(); ++it){
    tntdb::Row row = (*it);

    cout << row.getString(property::resource::KEY_PROTOCOL_INFO) << endl;

    list.push_back(row.getString(property::resource::KEY_PROTOCOL_INFO));
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

  tntdb::Statement select1 = mConnection.prepare(select);
  tntdb::Result result = mConnection.select(count);
  tntdb::Statement select2 = mConnection.prepare(resources.str());
  tntdb::Statement select3 = mConnection.prepare(details.str());

  StringList filterList = cFilterCriteria::parse(request.filter);

  request.numberReturned = 0;
  request.updateID = 0;
  request.totalMatches = result.getRow(0).getInt32("totalMatches");

  IXML_Document* DIDLDoc = NULL;
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

      for(tntdb::Statement::const_iterator it2 = select2.begin(); it2 != select2.end(); ++it2){
          row2 = (*it2);

          string resourceURI = row2.getString(property::resource::KEY_RESOURCE);

          IXML_Element* resource = ixml::IxmlAddFilteredProperty(filterList, DIDLDoc, object, property::resource::KEY_RESOURCE, resourceURI);

          if(resource){
            ixml::IxmlAddFilteredProperty(filterList, DIDLDoc, object, property::resource::KEY_PROTOCOL_INFO, row2.getString(property::resource::KEY_PROTOCOL_INFO));
            ixml::IxmlAddFilteredProperty(filterList, DIDLDoc, object, property::resource::KEY_BITRATE, row2.getString(property::resource::KEY_BITRATE));
            ixml::IxmlAddFilteredProperty(filterList, DIDLDoc, object, property::resource::KEY_BITS_PER_SAMPLE, row2.getString(property::resource::KEY_BITS_PER_SAMPLE));
            ixml::IxmlAddFilteredProperty(filterList, DIDLDoc, object, property::resource::KEY_COLOR_DEPTH, row2.getString(property::resource::KEY_COLOR_DEPTH));
            ixml::IxmlAddFilteredProperty(filterList, DIDLDoc, object, property::resource::KEY_DURATION, row2.getString(property::resource::KEY_DURATION));
            ixml::IxmlAddFilteredProperty(filterList, DIDLDoc, object, property::resource::KEY_NR_AUDIO_CHANNELS, row2.getString(property::resource::KEY_NR_AUDIO_CHANNELS));
            ixml::IxmlAddFilteredProperty(filterList, DIDLDoc, object, property::resource::KEY_RESOLUTION, row2.getString(property::resource::KEY_RESOLUTION));
            ixml::IxmlAddFilteredProperty(filterList, DIDLDoc, object, property::resource::KEY_SAMPLE_FREQUENCY, row2.getString(property::resource::KEY_SAMPLE_FREQUENCY));
            ixml::IxmlAddFilteredProperty(filterList, DIDLDoc, object, property::resource::KEY_SIZE, tools::ToString(row2.getInt64(property::resource::KEY_SIZE)));
          }

      }

      select3.setString("objectID", objectID);

      for(tntdb::Statement::const_iterator it3 = select3.begin(); it3 != select2.end(); ++it3){
        row3 = (*it3);

        ixml::IxmlAddFilteredProperty(filterList, DIDLDoc, object, row3.getString("property"), row3.getString("value"));
      }

      ++request.numberReturned;
    }

    request.result = ixmlDocumenttoString(DIDLDoc);

    cout << request.result << endl;

    ixmlDocument_free(DIDLDoc);
    return UPNP_E_SUCCESS;
  }

  error:
  esyslog("UPnP\tFailed to process the request");
  ixmlDocument_free(DIDLDoc);
  return UPNP_CDS_E_CANT_PROCESS_REQUEST;
}

int cMediaManager::Browse(BrowseRequest& request){
  stringstream metadata, count, where;

  metadata << "SELECT *,(SELECT COUNT(1) FROM " << db::Metadata << " m WHERE "
      << "m.`" << property::object::KEY_PARENTID << "` = "
      << "p.`" << property::object::KEY_OBJECTID << "`) as "
      << "`" << property::object::KEY_CHILD_COUNT << "` FROM " << db::Metadata << " p WHERE ";

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
    metadata << " ORDER BY ";
    upnp::cSortCriteria::SortCriteriaList::iterator it = list.begin();
    metadata << (*it).property << " " << ((*it).sortDescending ? "DESC" : "ASC");
    for(++it; it != list.end(); ++it){
      metadata << ", " << (*it).property << " " << ((*it).sortDescending ? "DESC" : "ASC");
    }
  }

  if(request.requestCount){
    metadata << " LIMIT " << request.startIndex << ", " << request.requestCount;
  }

  int ret = 0;
  if((ret = CreateResponse(request, metadata.str(), count.str())) == UPNP_E_SUCCESS) return ret;

  return (request.totalMatches == 0 && request.numberReturned == 0) ? UPNP_CDS_E_CANT_PROCESS_REQUEST : UPNP_E_SUCCESS;
}

int cMediaManager::Search(SearchRequest& request){
  request.numberReturned = 0;
  request.totalMatches = 0;
  request.updateID = 0;

  stringstream metadata, count;

  // TODO: Finish search method

  int ret = 0;
  if((ret = CreateResponse(request, metadata.str(), count.str())) == UPNP_E_SUCCESS) return ret;

  return (request.totalMatches == 0 && request.numberReturned == 0) ? UPNP_CDS_E_CANT_PROCESS_REQUEST : UPNP_E_SUCCESS;
}

cMediaManager::BrowseFlag cMediaManager::ToBrowseFlag(std::string browseFlag) {
  if      (browseFlag.compare("BrowseMetadata") == 0)
    return CD_BROWSE_METADATA;
  else if (browseFlag.compare("BrowseDirectChildren") == 0)
    return CD_BROWSE_DIRECT_CHILDREN;
  else
    return NumBrowseFlags;
}

bool cMediaManager::Initialise(){
  try {
    stringstream ss;
    ss << "sqlite:" << mDatabaseFile;

    mConnection = tntdb::connect(ss.str());

    dsyslog("UPNP\tPreparing database structure...");

    if(CheckIntegrity()) return true;

    mConnection.beginTransaction();

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
       << "`" << property::object::KEY_SCHEDULED_END     << "` TEXT"
       << "`" << property::object::KEY_OBJECT_UPDATE_ID  << "` INTEGER"
       << ")";

    tntdb::Statement objectTable = mConnection.prepare(ss.str());

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

    tntdb::Statement detailsTable = mConnection.prepare(ss.str());

    detailsTable.execute();

    ss.str(string());

    ss << "CREATE TABLE " << db::Resources
       << "("
       << "  resourceID        INTEGER PRIMARY KEY,"
       << "  `" << property::object::KEY_OBJECTID << "` TEXT "
       << "  REFERENCES metadata (`"<< property::object::KEY_OBJECTID <<"`) ON DELETE CASCADE ON UPDATE CASCADE,"
       << "`" << property::resource::KEY_RESOURCE           << "` TEXT    NOT NULL,"
       << "`" << property::resource::KEY_PROTOCOL_INFO      << "` TEXT    NOT NULL,"
       << "`" << property::resource::KEY_SIZE               << "` INTEGER,"
       << "`" << property::resource::KEY_DURATION           << "` TEXT,"
       << "`" << property::resource::KEY_RESOLUTION         << "` TEXT,"
       << "`" << property::resource::KEY_BITRATE            << "` INTEGER,"
       << "`" << property::resource::KEY_SAMPLE_FREQUENCY   << "` INTEGER,"
       << "`" << property::resource::KEY_BITS_PER_SAMPLE    << "` INTEGER,"
       << "`" << property::resource::KEY_NR_AUDIO_CHANNELS  << "` INTEGER,"
       << "`" << property::resource::KEY_COLOR_DEPTH        << "` INTEGER"
       << ")";

    tntdb::Statement resourcesTable = mConnection.prepare(ss.str());

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

    tntdb::Statement rootContainer = mConnection.prepare(ss.str());

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

    mConnection.commitTransaction();

    return true;

  } catch (const std::exception& e) {
    esyslog("UPnP\tException occurred while initializing database: %s", e.what());

    mConnection.rollbackTransaction();

    return false;
  }

  return false;
}

bool cMediaManager::CheckIntegrity(){

  tntdb::Statement checkTable = mConnection.prepare(
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

  tntdb::Statement checkObject = mConnection.prepare(ss.str());

  if( checkObject.select().size() != 1 ){
    isyslog("UPnP\tRoot item does not exist or more than one root item exist.");
    return false;
  }

  return true;
}

void cMediaManager::SetDatabaseFile(string file){
  if(file.empty()) mDatabaseFile = "metadata.db";
  else mDatabaseFile = file;
}

void cMediaManager::Action(){

}

}  // namespace upnp


