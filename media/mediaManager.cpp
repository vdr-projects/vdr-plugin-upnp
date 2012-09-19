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
#include <upnp/upnp.h>
#include <sstream>
#include <tntdb/statement.h>
#include <tntdb/result.h>

namespace upnp {

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
  tntdb::Statement stmt = conn.prepare(
      "SELECT DISTINCT :protocolInfo FROM :resourceTable;"
      );

  stmt.setString("protocolInfo", property::resource::KEY_PROTOCOL_INFO)
      .setString("resourceTable", "resources");

  StringList list;

  for(tntdb::Statement::const_iterator it = stmt.begin(); it != stmt.end(); ++it){
    tntdb::Row row = (*it);
    list.push_back(row.getString(property::resource::KEY_PROTOCOL_INFO));
  }

  return list;
}

int cMediaManager::Browse(BrowseRequest& request){
  request.numberReturned = 0;
  request.totalMatches = 0;
  request.updateID = 0;

  stringstream sql;

  sql << "SELECT * FROM metadata LEFT JOIN resources USING "
      << "(`" << property::object::KEY_OBJECTID << "`)"
      << " WHERE ";

  switch (request.browseMetadata){
  case CD_BROWSE_METADATA:
    sql << "`" << property::object::KEY_OBJECTID << "`";
    break;
  case CD_BROWSE_DIRECT_CHILDREN:
    sql << "`" << property::object::KEY_PARENTID << "`";
    break;
  default:
    esyslog("UPnP\tInvalid arguments. Browse flag invalid");
    return UPNP_SOAP_E_INVALID_ARGS;
  }

  sql << " = '" << request.objectID << "'";

  cSortCriteria::SortCriteriaList list = cSortCriteria::parse(request.sortCriteria);
  if(!list.empty()){
    sql << " ORDER BY ";
    upnp::cSortCriteria::SortCriteriaList::iterator it = list.begin();
    sql << (*it).property << " " << ((*it).sortDescending ? "DESC" : "ASC");
    for(++it; it != list.end(); ++it){
      sql << ", " << (*it).property << " " << ((*it).sortDescending ? "DESC" : "ASC");
    }
  }

  if(request.requestCount){
    sql << " LIMIT " << request.startIndex << ", " << request.requestCount;
  }

  cout << sql.str() << endl;

  tntdb::Statement select = mConnection.prepare(sql.str());

  for(tntdb::Statement::const_iterator it = select.begin(); it != select.end(); ++it){
    tntdb::Row row = (*it);
    cout << row.getString(property::object::KEY_TITLE) << endl;
  }

  return UPNP_E_SUCCESS;
}

int cMediaManager::Search(SearchRequest& request){
  request.numberReturned = 0;
  request.totalMatches = 0;
  request.updateID = 0;

  return UPNP_E_SUCCESS;
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

    ss << "CREATE TABLE metadata"
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
       << ")";

    tntdb::Statement objectTable = mConnection.prepare(ss.str());

    objectTable.execute();

    ss.str(string());

    ss << "CREATE TABLE details"
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

    ss << "CREATE TABLE resources"
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

    ss << "INSERT INTO metadata ("
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

  if( checkTable.setString("table", "metadata").select().empty() ){
    isyslog("UPnP\tTable metadata does not exist");
    return false;
  }
  if( checkTable.setString("table", "details").select().empty() ){
    isyslog("UPnP\tTable details does not exist");
    return false;
  }
  if( checkTable.setString("table", "resources").select().empty() ){
    isyslog("UPnP\tTable resources does not exist");
    return false;
  }

  stringstream ss;

  ss << "SELECT `" << property::object::KEY_OBJECTID << "` FROM metadata WHERE `"
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


