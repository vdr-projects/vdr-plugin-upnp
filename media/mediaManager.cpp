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

  list.push_back("dc:title");
  list.push_back("dc:creator");
  list.push_back("dc:description");
  list.push_back("upnp:longDescription");
  list.push_back("res@protocolInfo");
  list.push_back("upnp:class");
  list.push_back("dc:date");
  list.push_back("dc:language");

  return list;
}

StringList cMediaManager::GetSortCapabilities() const {
  StringList list;

  list.push_back("dc:title");
  list.push_back("dc:creator");
  list.push_back("dc:description");
  list.push_back("upnp:longDescription");
  list.push_back("res@protocolInfo");
  list.push_back("upnp:class");
  list.push_back("dc:date");
  list.push_back("dc:language");

  return list;
}

StringList cMediaManager::GetSupportedProtocolInfos() const {
  tntdb::Connection conn = mConnection;
  tntdb::Statement stmt = conn.prepare(
      "SELECT DISTINCT protocolInfo FROM resources;"
      );

  StringList list;

  for(tntdb::Statement::const_iterator it = stmt.begin(); it != stmt.end(); ++it){
    tntdb::Row row = (*it);
    list.push_back(row.getString("protocolInfo"));
  }

  return list;
}

int cMediaManager::Browse(BrowseRequest& request){
  request.numberReturned = 0;
  request.totalMatches = 0;
  request.updateID = 0;

  switch (request.browseMetadata){
  case CD_BROWSE_METADATA:



  case CD_BROWSE_DIRECT_CHILDREN:



  default:
    esyslog("UPnP\tInvalid arguments. Browse flag invalid");
    return UPNP_SOAP_E_INVALID_ARGS;
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

    tntdb::Statement objectTable = mConnection.prepare(
        "CREATE TABLE metadata"
        "("
        "  objectID         TEXT    PRIMARY KEY,"
        "  parentID         TEXT    NOT NULL,"
        "  title            TEXT    NOT NULL,"
        "  class            TEXT    NOT NULL,"
        "  restricted       INTEGER NOT NULL,"
        "  creator          TEXT,"
        "  description      TEXT,"
        "  longDescription  TEXT,"
        "  date             TEXT,"
        "  language         TEXT,"
        "  channelNr        INTEGER,"
        "  channelName      TEXT,"
        "  scheduledStart   TEXT,"
        "  scheduledEnd     TEXT"
        ")");

    objectTable.execute();

    tntdb::Statement detailsTable = mConnection.prepare(
        "CREATE TABLE details"
        "("
        "  propertyID INTEGER PRIMARY KEY,"
        "  objectID   TEXT    REFERENCES metadata (objectID) ON DELETE CASCADE ON UPDATE CASCADE,"
        "  property   TEXT,"
        "  value      TEXT"
        ")");

    detailsTable.execute();

    tntdb::Statement resourcesTable = mConnection.prepare(
        "CREATE TABLE resources"
        "("
        "  resourceID       INTEGER PRIMARY KEY,"
        "  objectID         TEXT    REFERENCES metadata (objectID) ON DELETE CASCADE ON UPDATE CASCADE,"
        "  resourceUri      TEXT    NOT NULL,"
        "  protocolInfo     TEXT    NOT NULL,"
        "  size             INTEGER,"
        "  duration         TEXT,"
        "  resolution       TEXT,"
        "  bitrate          INTEGER,"
        "  sampleFreq       INTEGER,"
        "  bitsPerSample    INTEGER,"
        "  nrAudioChannels  INTEGER,"
        "  colorDepth       INTEGER"
        ")");

    resourcesTable.execute();

    tntdb::Statement rootContainer = mConnection.prepare(
        "INSERT INTO metadata (objectID,  parentID,  title,  class,  restricted,  description)"
        "              VALUES (:objectID, :parentID, :title, :class, :restricted, :description)"
        );

    rootContainer.setString("objectID", "0")
                 .setString("parentID", "-1")
                 .setString("title", cMediaServer::GetInstance()->GetServerDescription().friendlyName)
                 .setString("class", "object.container")
                 .setBool("restricted", true)
                 .setString("description", cMediaServer::GetInstance()->GetServerDescription().modelDescription)
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

  if( checkTable.setString("table", "metadata").select().empty() ) return false;
  if( checkTable.setString("table", "details").select().empty() ) return false;
  if( checkTable.setString("table", "resources").select().empty() ) return false;

  tntdb::Statement checkObject = mConnection.prepare(
          "SELECT objectID FROM metadata WHERE objectID='0' AND parentID='-1';"
          );

  if( checkObject.select().size() != 1 ) return false;

  return true;
}

void cMediaManager::SetDatabaseFile(string file){
  if(file.empty()) mDatabaseFile = "metadata.db";
  else mDatabaseFile = file;
}

void cMediaManager::Action(){

}

}  // namespace upnp


