/* 
 * File:   database.h
 * Author: savop
 *
 * Created on 3. September 2009, 22:20
 */

#ifndef _DATABASE_H
#define	_DATABASE_H

#include <sqlite3.h>
#include <vdr/tools.h>
#include "../common.h"

#define SQLITE_CASCADE_DELETES

#define PK_OBJECTS                      TOSTRING(1)
#define PK_RESOURCES                    TOSTRING(2)
#define PK_SEARCHCLASSES                TOSTRING(3)

#define SQLITE_FIRST_CUSTOMID           TOSTRING(100)

#define SQLITE_COLUMN_NAME_LENGTH       64

#define SQLITE_TABLE_RESOURCES          "Resources"
#define SQLITE_TABLE_OBJECTS            "Objects"
#define SQLITE_TABLE_ITEMS              "Items"
#define SQLITE_TABLE_CONTAINERS         "Containers"
#define SQLITE_TABLE_VIDEOITEMS         "VideoItems"
#define SQLITE_TABLE_AUDIOITEMS         "AudioItems"
#define SQLITE_TABLE_IMAGEITEMS         "ImageItems"
#define SQLITE_TABLE_VIDEOBROADCASTS    "VideoBroadcasts"
#define SQLITE_TABLE_AUDIOBROADCASTS    "AudioBroadcasts"
#define SQLITE_TABLE_MOVIES             "Movies"
#define SQLITE_TABLE_PHOTOS             "Photos"
#define SQLITE_TABLE_ALBUMS             "Albums"
#define SQLITE_TABLE_PLAYLISTS          "Playlists"
#define SQLITE_TABLE_SEARCHCLASS        "SearchClass"
#define SQLITE_TABLE_PRIMARY_KEYS       "PrimaryKeys"
#define SQLITE_TABLE_SYSTEM             "System"
#define SQLITE_TABLE_ITEMFINDER         "ItemFinder"

#define SQLITE_TYPE_TEXT                "TEXT"
#define SQLITE_TYPE_INTEGER             "INTEGER"
#define SQLITE_TYPE_BOOL                SQLITE_TYPE_INTEGER
#define SQLITE_TYPE_DATE                SQLITE_TYPE_TEXT
#define SQLITE_TYPE_ULONG               SQLITE_TYPE_INTEGER
#define SQLITE_TYPE_LONG                SQLITE_TYPE_INTEGER
#define SQLITE_TYPE_UINTEGER            SQLITE_TYPE_INTEGER

#define SQLITE_TRANSACTION_BEGIN        "BEGIN IMMEDIATE TRANSACTION "
#define SQLITE_TRANSACTION_END          "COMMIT TRANSACTION"
#define SQLITE_TRANSACTION_TYPE         "ROLLBACK"

#define SQLITE_CONFLICT_CLAUSE          "ON CONFLICT " SQLITE_TRANSACTION_TYPE
#define SQLITE_PRIMARY_KEY              SQLITE_TYPE_INTEGER " PRIMARY KEY"
#define SQLITE_NOT_NULL                 "NOT NULL"
#define SQLITE_UNIQUE                   "UNIQUE"

#define SQLITE_COL_OBJECTID             "ObjectID"
#define SQLITE_COL_PARENTID             "ParentID"
#define SQLITE_COL_TITLE                "Title"
#define SQLITE_COL_CREATOR              "Creator"
#define SQLITE_COL_CLASS                "Class"
#define SQLITE_COL_RESTRICTED           "Restricted"
#define SQLITE_COL_WRITESTATUS          "WriteStatus"
#define SQLITE_COL_REFERENCEID          "RefID"
#define SQLITE_COL_CLASSDERIVED         "IncludeDerived"
#define SQLITE_COL_SEARCHABLE           "Searchable"
#define SQLITE_COL_CONTAINER_UID        "UpdateID"
#define SQLITE_COL_RESOURCEID           "ResourceID"
#define SQLITE_COL_PROTOCOLINFO         "ProtocolInfo"
#define SQLITE_COL_CONTENTTYPE          "ContentType"
#define SQLITE_COL_RESOURCETYPE         "ResourceType"
#define SQLITE_COL_RESOURCE             "Resource"
#define SQLITE_COL_SIZE                 "Size"
#define SQLITE_COL_DURATION             "Duration"
#define SQLITE_COL_BITRATE              "Bitrate"
#define SQLITE_COL_SAMPLEFREQUENCE      "SampleFreq"
#define SQLITE_COL_BITSPERSAMPLE        "BitsPerSample"
#define SQLITE_COL_NOAUDIOCHANNELS      "NoAudioChannels"
#define SQLITE_COL_COLORDEPTH           "ColorDepth"
#define SQLITE_COL_RESOLUTION           "Resolution"
#define SQLITE_COL_GENRE                "Genre"
#define SQLITE_COL_LONGDESCRIPTION      "LongDescription"
#define SQLITE_COL_PRODUCER             "Producer"
#define SQLITE_COL_RATING               "Rating"
#define SQLITE_COL_ACTOR                "Actor"
#define SQLITE_COL_DIRECTOR             "Director"
#define SQLITE_COL_DESCRIPTION          "Description"
#define SQLITE_COL_PUBLISHER            "Publisher"
#define SQLITE_COL_LANGUAGE             "Language"
#define SQLITE_COL_RELATION             "Relation"
#define SQLITE_COL_STORAGEMEDIUM        "StorageMedium"
#define SQLITE_COL_DVDREGIONCODE        "DVDRegionCode"
#define SQLITE_COL_CHANNELNAME          "Channelname"
#define SQLITE_COL_SCHEDULEDSTARTTIME   "ScheduledStartTime"
#define SQLITE_COL_SCHEDULEDENDTIME     "ScheduledEndTime"
#define SQLITE_COL_ICON                 "Icon"
#define SQLITE_COL_REGION               "Region"
#define SQLITE_COL_CHANNELNR            "ChannelNr"
#define SQLITE_COL_RIGHTS               "Rights"
#define SQLITE_COL_RADIOCALLSIGN        "CallSign"
#define SQLITE_COL_RADIOSTATIONID       "StationID"
#define SQLITE_COL_RADIOBAND            "Band"
#define SQLITE_COL_CONTRIBUTOR          "Contributor"
#define SQLITE_COL_DATE                 "Date"
#define SQLITE_COL_ALBUM                "Album"
#define SQLITE_COL_ARTIST               "Artist"
#define SQLITE_COL_DLNA_CONTAINERTYPE   "DLNAContainer"
#define SQLITE_COL_CHILDCOUNT           "ChildCount"
#define SQLITE_COL_ITEMFINDER           "ItemFastID"

#define SQLITE_UPNP_OBJECTID            SQLITE_COL_OBJECTID " " SQLITE_TYPE_INTEGER " " SQLITE_NOT_NULL " " SQLITE_CONFLICT_CLAUSE " "\
                                                                SQLITE_UNIQUE " " SQLITE_CONFLICT_CLAUSE

#define SQLITE_INSERT_TRIGGER(TableA,TableB,Class)      "CREATE TRIGGER IF NOT EXISTS "\
                                                        TableA "_I_" TableB " "\
                                                        "BEFORE INSERT ON "\
                                                        TableB " "\
                                                        "FOR EACH ROW BEGIN "\
                                                        "SELECT CASE "\
                                                        "WHEN ("\
                                                        "((SELECT " SQLITE_COL_OBJECTID " FROM " TableA " "\
                                                        "WHERE " SQLITE_COL_OBJECTID "=NEW." SQLITE_COL_OBJECTID " "\
                                                        ") IS NULL) "\
                                                        "OR "\
                                                        "((SELECT " SQLITE_COL_OBJECTID " FROM " SQLITE_TABLE_OBJECTS " "\
                                                        "WHERE " SQLITE_COL_OBJECTID "=NEW." SQLITE_COL_OBJECTID " "\
                                                        "AND " SQLITE_COL_CLASS " LIKE '" Class "%%') IS NULL) "\
                                                        ") THEN "\
                                                        "RAISE(" SQLITE_TRANSACTION_TYPE ", "\
                                                        "'INSERT on table " TableB " failed due constraint violation "\
                                                        "on foreign key " SQLITE_COL_OBJECTID "'"\
                                                        ") "\
                                                        "END; END;"

#define SQLITE_UPDATE_TRIGGER(TableA,TableB,Class)      "CREATE TRIGGER IF NOT EXISTS "\
                                                        TableA "_U_" TableB " "\
                                                        "BEFORE UPDATE ON "\
                                                        TableB " "\
                                                        "FOR EACH ROW BEGIN "\
                                                        "SELECT CASE "\
                                                        "WHEN ("\
                                                        "((SELECT " SQLITE_COL_OBJECTID " FROM " SQLITE_TABLE_OBJECTS " "\
                                                        "WHERE " SQLITE_COL_OBJECTID "=NEW." SQLITE_COL_OBJECTID " "\
                                                        "AND " SQLITE_COL_CLASS " LIKE '" Class "%%') IS NULL)"\
                                                        ") THEN "\
                                                        "RAISE(" SQLITE_TRANSACTION_TYPE ", "\
                                                        "'UPDATE on table " TableB " failed due constraint violation "\
                                                        "on foreign key " SQLITE_COL_OBJECTID "'"\
                                                        ") "\
                                                        "END; END;"

#define SQLITE_INSERT_REFERENCE_TRIGGER(Table,Column)   "CREATE TRIGGER IF NOT EXISTS "\
                                                        Table "_I_" Table " "\
                                                        "BEFORE INSERT ON " \
                                                        Table " " \
                                                        "FOR EACH ROW BEGIN "\
                                                        "SELECT CASE "\
                                                        "WHEN ( "\
                                                        "((SELECT " SQLITE_COL_OBJECTID " FROM " Table " "\
                                                        "WHERE " SQLITE_COL_OBJECTID " = NEW." Column ") IS NULL) "\
                                                        "AND "\
                                                        "(NEW." Column "!=-1)"\
                                                        ")THEN "\
                                                        "RAISE(" SQLITE_TRANSACTION_TYPE ", 'INSERT on table " Table " "\
                                                        "violates foreign key \"" Column "\"') "\
                                                        "END; END;"

#define SQLITE_UPDATE_REFERENCE_TRIGGER(Table,Column)   "CREATE TRIGGER IF NOT EXISTS "\
                                                        Table "_U_" Table " "\
                                                        "BEFORE INSERT ON " \
                                                        Table " " \
                                                        "FOR EACH ROW BEGIN "\
                                                        "SELECT CASE "\
                                                        "WHEN ( "\
                                                        "((SELECT " SQLITE_COL_OBJECTID " FROM " Table " "\
                                                        "WHERE " SQLITE_COL_OBJECTID " = NEW." Column ") IS NULL) "\
                                                        "AND "\
                                                        "(NEW." Column "!=-1)"\
                                                        ")THEN "\
                                                        "RAISE(" SQLITE_TRANSACTION_TYPE ", 'UPDATE on table " Table " "\
                                                        "violates foreign key \"" Column "\"') "\
                                                        "END; END;"

#define SQLITE_DELETE_REFERENCE_TRIGGER(Table,Column)   "CREATE TRIGGER IF NOT EXISTS "\
                                                        Table "_D_" Table " " \
                                                        "BEFORE DELETE ON " \
                                                        Table " " \
                                                        "FOR EACH ROW BEGIN "\
                                                        "SELECT CASE "\
                                                        "WHEN ("\
                                                        "(SELECT " Column " FROM " Table " "\
                                                        "WHERE " Column " = OLD." SQLITE_COL_OBJECTID ") IS NOT NULL"\
                                                        ")THEN "\
                                                        "RAISE(" SQLITE_TRANSACTION_TYPE ", 'DELETE on table " Table " "\
                                                        "violates foreign key \"" Column "\"') "\
                                                        "END; END;"

#ifdef SQLITE_CASCADE_DELETES
#define SQLITE_DELETE_TRIGGER(TableA,TableB)            "CREATE TRIGGER IF NOT EXISTS "\
                                                        TableA "_D_" TableB " "\
                                                        "BEFORE DELETE ON "\
                                                        TableA " "\
                                                        "FOR EACH ROW BEGIN "\
                                                        "DELETE FROM " TableB " "\
                                                        "WHERE " SQLITE_COL_OBJECTID "=OLD." SQLITE_COL_OBJECTID "; "\
                                                        "END;"

#define SQLITE_DELETE_PARENT_TRIGGER                    "CREATE TRIGGER IF NOT EXISTS "\
                                                        SQLITE_TABLE_OBJECTS "_D_" SQLITE_TABLE_OBJECTS " " \
                                                        "BEFORE DELETE ON " \
                                                        SQLITE_TABLE_OBJECTS " " \
                                                        "FOR EACH ROW BEGIN "\
                                                        "DELETE FROM " SQLITE_TABLE_OBJECTS " "\
                                                        "WHERE " SQLITE_COL_PARENTID "=OLD." SQLITE_COL_OBJECTID "; "\
                                                        "END;"
#else
#define SQLITE_DELETE_TRIGGER(TableA,TableB)            "CREATE TRIGGER IF NOT EXISTS "\
                                                        TableA "_D_" TableB " "\
                                                        "BEFORE DELETE ON "\
                                                        TableA " "\
                                                        "FOR EACH ROW BEGIN "\
                                                        "SELECT CASE "\
                                                        "WHEN ("\
                                                        "(SELECT " SQLITE_COL_OBJECTID " FROM " TableB " "\
                                                        "WHERE " SQLITE_COL_OBJECTID "=OLD." SQLITE_COL_OBJECTID ") IS NOT NULL"\
                                                        ") THEN "\
                                                        "RAISE(" SQLITE_TRANSACTION_TYPE ", "\
                                                        "'DELETE on table " TableA " failed due constraint violation "\
                                                        "on foreign key " SQLITE_COL_OBJECTID "'"\
                                                        ") "\
                                                        "END; END;"

#define SQLITE_DELETE_PARENT_TRIGGER                    SQLITE_DELETE_REFERENCE_TRIGGER(SQLITE_TABLE_OBJECTS, SQLITE_COL_PARENTID)
#endif

/**********************************************\
*                                              *
*  Primary keys                                *
*                                              *
\**********************************************/

#define SQLITE_CREATE_TABLE_PRIMARY_KEYS    "CREATE TABLE IF NOT EXISTS "\
                                            SQLITE_TABLE_PRIMARY_KEYS \
                                            "("\
                                            "KeyID " SQLITE_PRIMARY_KEY " " SQLITE_NOT_NULL ","\
                                            "Key " SQLITE_TYPE_INTEGER " " SQLITE_NOT_NULL\
                                            ");"\
                                            "INSERT OR IGNORE INTO "\
                                            SQLITE_TABLE_PRIMARY_KEYS \
                                            "(KeyID, Key) VALUES ("\
                                            PK_OBJECTS "," SQLITE_FIRST_CUSTOMID\
                                            ");"\
                                            "INSERT OR IGNORE INTO "\
                                            SQLITE_TABLE_PRIMARY_KEYS \
                                            "(KeyID, Key) VALUES ("\
                                            PK_RESOURCES ",0"\
                                            ");"\
                                            "INSERT OR IGNORE INTO "\
                                            SQLITE_TABLE_PRIMARY_KEYS \
                                            "(KeyID, Key) VALUES ("\
                                            PK_SEARCHCLASSES ",0"\
                                            ");"

#define SQLITE_TRIGGER_UPDATE_OBJECTID      "CREATE TRIGGER IF NOT EXISTS "\
                                            SQLITE_TABLE_OBJECTS "_PK_UPDATE "\
                                            "AFTER INSERT ON "\
                                            SQLITE_TABLE_OBJECTS " "\
                                            "BEGIN "\
                                            "UPDATE " SQLITE_TABLE_PRIMARY_KEYS " SET Key=Key+1 WHERE KeyID=" PK_OBJECTS "; "\
                                            "END;"

/**********************************************\
*                                              *
*  System settings                             *
*                                              *
\**********************************************/

#define SQLITE_CREATE_TABLE_SYSTEM          "CREATE TABLE IF NOT EXISTS "\
                                            SQLITE_TABLE_SYSTEM " "\
                                            "("\
                                            "Key " SQLITE_TYPE_TEXT " " SQLITE_NOT_NULL " " SQLITE_UNIQUE ","\
                                            "Value " SQLITE_TYPE_TEXT " "\
                                            ");"

#define SQLITE_TRIGGER_UPDATE_SYSTEM        "CREATE TRIGGER IF NOT EXISTS "\
                                            SQLITE_TABLE_SYSTEM "_VALUE_UPDATE "\
                                            "BEFORE UPDATE "\
                                            "ON " SQLITE_TABLE_SYSTEM " "\
                                            "WHEN ((SELECT Key FROM " SQLITE_TABLE_SYSTEM " WHERE Key=NEW.Key) IS NULL) "\
                                            "BEGIN INSERT INTO " SQLITE_TABLE_SYSTEM " (Key) VALUES (NEW.Key); END;"

/**********************************************\
*                                              *
*  Fast item finder                            *
*                                              *
\**********************************************/

#define SQLITE_CREATE_TABLE_ITEMFINDER      "CREATE TABLE IF NOT EXISTS "\
                                            SQLITE_TABLE_ITEMFINDER " "\
                                            "("\
                                            SQLITE_UPNP_OBJECTID ","\
                                            SQLITE_COL_ITEMFINDER " " SQLITE_TYPE_TEXT " " SQLITE_NOT_NULL " " SQLITE_UNIQUE \
                                            ");"

#define SQLITE_TRIGGER_D_OBJECTS_ITEMFINDER SQLITE_DELETE_TRIGGER(SQLITE_TABLE_OBJECTS,\
                                                                  SQLITE_TABLE_ITEMFINDER)

/**********************************************\
*                                              *
*  Objects                                     *
*                                              *
\**********************************************/

#define SQLITE_CREATE_TABLE_OBJECTS     "CREATE TABLE IF NOT EXISTS "\
                                        SQLITE_TABLE_OBJECTS \
                                        "(" \
                                        SQLITE_COL_OBJECTID " " SQLITE_PRIMARY_KEY " " SQLITE_NOT_NULL " " SQLITE_CONFLICT_CLAUSE "," \
                                        SQLITE_COL_PARENTID " " SQLITE_TYPE_INTEGER " " SQLITE_NOT_NULL " " SQLITE_CONFLICT_CLAUSE "," \
                                        SQLITE_COL_TITLE    " " SQLITE_TYPE_TEXT " " SQLITE_NOT_NULL "," \
                                        SQLITE_COL_CREATOR  " " SQLITE_TYPE_TEXT "," \
                                        SQLITE_COL_CLASS    " " SQLITE_TYPE_TEXT " " SQLITE_NOT_NULL "," \
                                        SQLITE_COL_RESTRICTED " " SQLITE_TYPE_BOOL " " SQLITE_NOT_NULL "," \
                                        SQLITE_COL_WRITESTATUS " " SQLITE_TYPE_INTEGER \
                                        ");"

// Trigger for foreign key ParentID

#define SQLITE_TRIGGER_D_OBJECTS_OBJECTS SQLITE_DELETE_PARENT_TRIGGER

#define SQLITE_TRIGGER_I_OBJECTS_OBJECTS SQLITE_INSERT_REFERENCE_TRIGGER(SQLITE_TABLE_OBJECTS, SQLITE_COL_PARENTID)\
                                         "CREATE TRIGGER IF NOT EXISTS "\
                                         SQLITE_TABLE_OBJECTS "_PI_" SQLITE_TABLE_OBJECTS " "\
                                         "BEFORE INSERT ON "\
                                         SQLITE_TABLE_OBJECTS " " \
                                         "FOR EACH ROW BEGIN "\
                                         "SELECT CASE "\
                                         "WHEN ("\
                                         "((SELECT " SQLITE_COL_PARENTID " FROM " SQLITE_TABLE_OBJECTS " "\
                                         "WHERE " SQLITE_COL_PARENTID "=-1) IS NOT NULL) "\
                                         "AND "\
                                         "(NEW." SQLITE_COL_PARENTID "=-1)"\
                                         ") THEN "\
                                         "RAISE(" SQLITE_TRANSACTION_TYPE ","\
                                         "'INSERT on table " SQLITE_TABLE_OBJECTS " violates constraint. "\
                                         SQLITE_COL_PARENTID " must uniquely be -1') "\
                                         "END; END;"

#define SQLITE_TRIGGER_U_OBJECTS_OBJECTS SQLITE_UPDATE_REFERENCE_TRIGGER(SQLITE_TABLE_OBJECTS, SQLITE_COL_PARENTID)\
                                         "CREATE TRIGGER IF NOT EXISTS "\
                                         SQLITE_TABLE_OBJECTS "_PU_" SQLITE_TABLE_OBJECTS " "\
                                         "BEFORE UPDATE ON "\
                                         SQLITE_TABLE_OBJECTS " " \
                                         "FOR EACH ROW BEGIN "\
                                         "SELECT CASE "\
                                         "WHEN ("\
                                         "((SELECT " SQLITE_COL_PARENTID " FROM " SQLITE_TABLE_OBJECTS " "\
                                         "WHERE " SQLITE_COL_PARENTID "=-1 "\
                                         "AND " SQLITE_COL_OBJECTID "!=NEW." SQLITE_COL_OBJECTID " ) IS NOT NULL) "\
                                         "AND "\
                                         "(NEW." SQLITE_COL_PARENTID "=-1) AND (OLD." SQLITE_COL_PARENTID "!=-1) "\
                                         ") THEN "\
                                         "RAISE(" SQLITE_TRANSACTION_TYPE ","\
                                         "'UPDATE on table " SQLITE_TABLE_OBJECTS " violates constraint. "\
                                         SQLITE_COL_PARENTID " must uniquely be -1') "\
                                         "END; END;"

/**********************************************\
*                                              *
*  Items                                       *
*                                              *
\**********************************************/

#define SQLITE_CREATE_TABLE_ITEMS       "CREATE TABLE IF NOT EXISTS "\
                                        SQLITE_TABLE_ITEMS \
                                        "(" \
                                        SQLITE_UPNP_OBJECTID "," \
                                        SQLITE_COL_REFERENCEID " " SQLITE_TYPE_INTEGER " DEFAULT -1" \
                                        ");"

// Trigger for foreign key ObjectID

#define SQLITE_TRIGGER_D_OBJECT_ITEMS   SQLITE_DELETE_TRIGGER(SQLITE_TABLE_OBJECTS,\
                                                              SQLITE_TABLE_ITEMS)

#define SQLITE_TRIGGER_I_OBJECT_ITEMS   SQLITE_INSERT_TRIGGER(SQLITE_TABLE_OBJECTS,\
                                                              SQLITE_TABLE_ITEMS,\
                                                              UPNP_CLASS_ITEM)

#define SQLITE_TRIGGER_U_OBJECT_ITEMS   SQLITE_UPDATE_TRIGGER(SQLITE_TABLE_OBJECTS,\
                                                              SQLITE_TABLE_ITEMS,\
                                                              UPNP_CLASS_ITEM)

// Trigger for Reference items

#define SQLITE_TRIGGER_I_ITEMS_ITEMS    SQLITE_INSERT_REFERENCE_TRIGGER(SQLITE_TABLE_ITEMS, SQLITE_COL_REFERENCEID)

#define SQLITE_TRIGGER_U_ITEMS_ITEMS    SQLITE_UPDATE_REFERENCE_TRIGGER(SQLITE_TABLE_ITEMS, SQLITE_COL_REFERENCEID)

#define SQLITE_TRIGGER_D_ITEMS_ITEMS    SQLITE_DELETE_REFERENCE_TRIGGER(SQLITE_TABLE_ITEMS, SQLITE_COL_REFERENCEID)

/**********************************************\
*                                              *
*  Containers                                  *
*                                              *
\**********************************************/

#define SQLITE_CREATE_TABLE_CONTAINER   "CREATE TABLE IF NOT EXISTS "\
                                        SQLITE_TABLE_CONTAINERS \
                                        "(" \
                                        SQLITE_UPNP_OBJECTID "," \
                                        SQLITE_COL_SEARCHABLE " " SQLITE_TYPE_INTEGER ","\
                                        SQLITE_COL_CONTAINER_UID " " SQLITE_TYPE_INTEGER " " SQLITE_NOT_NULL ","\
                                        SQLITE_COL_DLNA_CONTAINERTYPE " " SQLITE_TYPE_TEXT \
                                        ");"

#define SQLITE_TRIGGER_D_OBJECT_CONTAINERS  SQLITE_DELETE_TRIGGER(SQLITE_TABLE_OBJECTS,\
                                                                  SQLITE_TABLE_CONTAINERS)

#define SQLITE_TRIGGER_I_OBJECT_CONTAINERS  SQLITE_INSERT_TRIGGER(SQLITE_TABLE_OBJECTS,\
                                                                  SQLITE_TABLE_CONTAINERS,\
                                                                  UPNP_CLASS_CONTAINER)

#define SQLITE_TRIGGER_U_OBJECT_CONTAINERS  SQLITE_UPDATE_TRIGGER(SQLITE_TABLE_OBJECTS,\
                                                                  SQLITE_TABLE_CONTAINERS,\
                                                                  UPNP_CLASS_CONTAINER)

/**********************************************\
*                                              *
*  Video items                                 *
*                                              *
\**********************************************/

#define SQLITE_CREATE_TABLE_VIDEOITEMS  "CREATE TABLE IF NOT EXISTS "\
                                        SQLITE_TABLE_VIDEOITEMS \
                                        "(" \
                                        SQLITE_UPNP_OBJECTID "," \
                                        SQLITE_COL_GENRE " " SQLITE_TYPE_TEXT "," \
                                        SQLITE_COL_LONGDESCRIPTION " " SQLITE_TYPE_TEXT "," \
                                        SQLITE_COL_PRODUCER " " SQLITE_TYPE_TEXT "," \
                                        SQLITE_COL_RATING " " SQLITE_TYPE_TEXT "," \
                                        SQLITE_COL_ACTOR " " SQLITE_TYPE_TEXT "," \
                                        SQLITE_COL_DIRECTOR " " SQLITE_TYPE_TEXT "," \
                                        SQLITE_COL_DESCRIPTION " " SQLITE_TYPE_TEXT "," \
                                        SQLITE_COL_PUBLISHER " " SQLITE_TYPE_TEXT "," \
                                        SQLITE_COL_LANGUAGE " " SQLITE_TYPE_TEXT "," \
                                        SQLITE_COL_RELATION " " SQLITE_TYPE_TEXT \
                                        ");"

#define SQLITE_TRIGGER_D_ITEMS_VIDEOITEMS SQLITE_DELETE_TRIGGER(SQLITE_TABLE_ITEMS, SQLITE_TABLE_VIDEOITEMS)

#define SQLITE_TRIGGER_U_ITEMS_VIDEOITEMS SQLITE_UPDATE_TRIGGER(SQLITE_TABLE_ITEMS, \
                                                                SQLITE_TABLE_VIDEOITEMS, \
                                                                UPNP_CLASS_VIDEO)

#define SQLITE_TRIGGER_I_ITEMS_VIDEOITEMS SQLITE_INSERT_TRIGGER(SQLITE_TABLE_ITEMS, \
                                                                SQLITE_TABLE_VIDEOITEMS, \
                                                                UPNP_CLASS_VIDEO)

/**********************************************\
*                                              *
*  Audio items                                 *
*                                              *
\**********************************************/

#define SQLITE_CREATE_TABLE_AUDIOITEMS  "CREATE TABLE IF NOT EXISTS "\
                                        SQLITE_TABLE_AUDIOITEMS \
                                        "(" \
                                        SQLITE_UPNP_OBJECTID "," \
                                        SQLITE_COL_GENRE " " SQLITE_TYPE_TEXT "," \
                                        SQLITE_COL_LONGDESCRIPTION " " SQLITE_TYPE_TEXT "," \
                                        SQLITE_COL_DESCRIPTION " " SQLITE_TYPE_TEXT "," \
                                        SQLITE_COL_PUBLISHER " " SQLITE_TYPE_TEXT "," \
                                        SQLITE_COL_RELATION " " SQLITE_TYPE_TEXT \
                                        ");"

#define SQLITE_TRIGGER_D_ITEMS_AUDIOITEMS SQLITE_DELETE_TRIGGER(SQLITE_TABLE_ITEMS, SQLITE_TABLE_AUDIOITEMS)

#define SQLITE_TRIGGER_U_ITEMS_AUDIOITEMS SQLITE_UPDATE_TRIGGER(SQLITE_TABLE_ITEMS, \
                                                                SQLITE_TABLE_AUDIOITEMS, \
                                                                UPNP_CLASS_AUDIO)

#define SQLITE_TRIGGER_I_ITEMS_AUDIOITEMS SQLITE_INSERT_TRIGGER(SQLITE_TABLE_ITEMS, \
                                                                SQLITE_TABLE_AUDIOITEMS, \
                                                                UPNP_CLASS_AUDIO)

/**********************************************\
*                                              *
*  Image items                                 *
*                                              *
\**********************************************/

#define SQLITE_CREATE_TABLE_IMAGEITEMS  "CREATE TABLE IF NOT EXISTS "\
                                        SQLITE_TABLE_IMAGEITEMS \
                                        "("\
                                        SQLITE_UPNP_OBJECTID "," \
                                        SQLITE_COL_LONGDESCRIPTION " " SQLITE_TYPE_TEXT "," \
                                        SQLITE_COL_DESCRIPTION " " SQLITE_TYPE_TEXT "," \
                                        SQLITE_COL_PUBLISHER " " SQLITE_TYPE_TEXT "," \
                                        SQLITE_COL_STORAGEMEDIUM " " SQLITE_TYPE_TEXT ","\
                                        SQLITE_COL_RATING " " SQLITE_TYPE_TEXT ","\
                                        SQLITE_COL_DATE " " SQLITE_TYPE_TEXT ","\
                                        SQLITE_COL_RIGHTS " " SQLITE_TYPE_TEXT\
                                        ");"

#define SQLITE_TRIGGER_D_ITEMS_IMAGEITEMS SQLITE_DELETE_TRIGGER(SQLITE_TABLE_ITEMS, SQLITE_TABLE_IMAGEITEMS)

#define SQLITE_TRIGGER_U_ITEMS_IMAGEITEMS SQLITE_UPDATE_TRIGGER(SQLITE_TABLE_ITEMS, \
                                                                SQLITE_TABLE_IMAGEITEMS, \
                                                                UPNP_CLASS_IMAGE)

#define SQLITE_TRIGGER_I_ITEMS_IMAGEITEMS SQLITE_INSERT_TRIGGER(SQLITE_TABLE_ITEMS, \
                                                                SQLITE_TABLE_IMAGEITEMS, \
                                                                UPNP_CLASS_IMAGE)

/**********************************************\
*                                              *
*  Video broadcasts                            *
*                                              *
\**********************************************/

#define SQLITE_CREATE_TABLE_VIDEOBROADCASTS "CREATE TABLE IF NOT EXISTS "\
                                        SQLITE_TABLE_VIDEOBROADCASTS \
                                        "("\
                                        SQLITE_UPNP_OBJECTID "," \
                                        SQLITE_COL_ICON " " SQLITE_TYPE_TEXT ","\
                                        SQLITE_COL_REGION " " SQLITE_TYPE_TEXT ","\
                                        SQLITE_COL_CHANNELNR " " SQLITE_TYPE_TEXT ","\
                                        SQLITE_COL_CHANNELNAME " " SQLITE_TYPE_TEXT " " SQLITE_UNIQUE \
                                        ");"

#define SQLITE_TRIGGER_D_VIDEOITEMS_VIDEOBROADCASTS SQLITE_DELETE_TRIGGER(SQLITE_TABLE_VIDEOITEMS, SQLITE_TABLE_VIDEOBROADCASTS)

#define SQLITE_TRIGGER_U_VIDEOITEMS_VIDEOBROADCASTS SQLITE_UPDATE_TRIGGER(SQLITE_TABLE_VIDEOITEMS,\
                                                                          SQLITE_TABLE_VIDEOBROADCASTS,\
                                                                          UPNP_CLASS_VIDEOBC)

#define SQLITE_TRIGGER_I_VIDEOITEMS_VIDEOBROADCASTS SQLITE_INSERT_TRIGGER(SQLITE_TABLE_VIDEOITEMS,\
                                                                          SQLITE_TABLE_VIDEOBROADCASTS,\
                                                                          UPNP_CLASS_VIDEOBC)

/**********************************************\
*                                              *
*  Audio broadcasts                            *
*                                              *
\**********************************************/

#define SQLITE_CREATE_TABLE_AUDIOBROADCASTS "CREATE TABLE IF NOT EXISTS "\
                                        SQLITE_TABLE_AUDIOBROADCASTS \
                                        "("\
                                        SQLITE_UPNP_OBJECTID "," \
                                        SQLITE_COL_REGION " " SQLITE_TYPE_TEXT ","\
                                        SQLITE_COL_RADIOCALLSIGN " " SQLITE_TYPE_TEXT ","\
                                        SQLITE_COL_RADIOSTATIONID " " SQLITE_TYPE_TEXT ","\
                                        SQLITE_COL_RADIOBAND " " SQLITE_TYPE_TEXT ","\
                                        SQLITE_COL_CHANNELNR " " SQLITE_TYPE_INTEGER \
                                        ");"

#define SQLITE_TRIGGER_D_AUDIOITEMS_AUDIOBROADCASTS SQLITE_DELETE_TRIGGER(SQLITE_TABLE_AUDIOITEMS, SQLITE_TABLE_AUDIOBROADCASTS)

#define SQLITE_TRIGGER_I_AUDIOITEMS_AUDIOBROADCASTS SQLITE_INSERT_TRIGGER(SQLITE_TABLE_AUDIOITEMS,\
                                                                          SQLITE_TABLE_AUDIOBROADCASTS,\
                                                                          UPNP_CLASS_AUDIOBC)

#define SQLITE_TRIGGER_U_AUDIOITEMS_AUDIOBROADCASTS SQLITE_UPDATE_TRIGGER(SQLITE_TABLE_AUDIOITEMS,\
                                                                          SQLITE_TABLE_AUDIOBROADCASTS,\
                                                                          UPNP_CLASS_AUDIOBC)

/**********************************************\
*                                              *
*  Movies                                      *
*                                              *
\**********************************************/

#define SQLITE_CREATE_TABLE_MOVIES      "CREATE TABLE IF NOT EXISTS "\
                                        SQLITE_TABLE_MOVIES \
                                        "("\
                                        SQLITE_UPNP_OBJECTID "," \
                                        SQLITE_COL_STORAGEMEDIUM " " SQLITE_TYPE_TEXT "," \
                                        SQLITE_COL_DVDREGIONCODE " " SQLITE_TYPE_INTEGER "," \
                                        SQLITE_COL_CHANNELNAME " " SQLITE_TYPE_TEXT ","\
                                        SQLITE_COL_SCHEDULEDSTARTTIME " " SQLITE_TYPE_TEXT ","\
                                        SQLITE_COL_SCHEDULEDENDTIME " " SQLITE_TYPE_TEXT\
                                        ");"

#define SQLITE_TRIGGER_D_VIDEOITEMS_MOVIES SQLITE_DELETE_TRIGGER(SQLITE_TABLE_VIDEOITEMS, SQLITE_TABLE_MOVIES)


#define SQLITE_TRIGGER_I_VIDEOITEMS_MOVIES SQLITE_INSERT_TRIGGER(SQLITE_TABLE_VIDEOITEMS,\
                                                                 SQLITE_TABLE_MOVIES,\
                                                                 UPNP_CLASS_MOVIE)

#define SQLITE_TRIGGER_U_VIDEOITEMS_MOVIES SQLITE_UPDATE_TRIGGER(SQLITE_TABLE_VIDEOITEMS,\
                                                                 SQLITE_TABLE_MOVIES,\
                                                                 UPNP_CLASS_MOVIE)

/**********************************************\
*                                              *
*  Photos                                      *
*                                              *
\**********************************************/

#define SQLITE_CREATE_TABLE_PHOTOS      "CREATE TABLE IF NOT EXISTS "\
                                        SQLITE_TABLE_PHOTOS \
                                        "("\
                                        SQLITE_UPNP_OBJECTID "," \
                                        SQLITE_COL_ALBUM " " SQLITE_TYPE_TEXT\
                                        ");"

#define SQLITE_TRIGGER_D_IMAGEITEMS_PHOTOS SQLITE_DELETE_TRIGGER(SQLITE_TABLE_IMAGEITEMS, SQLITE_TABLE_PHOTOS)

#define SQLITE_TRIGGER_I_IMAGEITEMS_PHOTOS SQLITE_INSERT_TRIGGER(SQLITE_TABLE_IMAGEITEMS,\
                                                                 SQLITE_TABLE_PHOTOS,\
                                                                 UPNP_CLASS_PHOTO)

#define SQLITE_TRIGGER_U_IMAGEITEMS_PHOTOS SQLITE_UPDATE_TRIGGER(SQLITE_TABLE_IMAGEITEMS,\
                                                                 SQLITE_TABLE_PHOTOS,\
                                                                 UPNP_CLASS_PHOTO)

/**********************************************\
*                                              *
*  Albums                                      *
*                                              *
\**********************************************/

#define SQLITE_CREATE_TABLE_ALBUMS      "CREATE TABLE IF NOT EXISTS "\
                                        SQLITE_TABLE_ALBUMS \
                                        "("\
                                        SQLITE_UPNP_OBJECTID "," \
                                        SQLITE_COL_STORAGEMEDIUM " " SQLITE_TYPE_TEXT "," \
                                        SQLITE_COL_LONGDESCRIPTION " " SQLITE_TYPE_TEXT "," \
                                        SQLITE_COL_DESCRIPTION " " SQLITE_TYPE_TEXT "," \
                                        SQLITE_COL_PUBLISHER " " SQLITE_TYPE_TEXT "," \
                                        SQLITE_COL_CONTRIBUTOR " " SQLITE_TYPE_TEXT ","\
                                        SQLITE_COL_DATE " " SQLITE_TYPE_TEXT "," \
                                        SQLITE_COL_RELATION " " SQLITE_TYPE_TEXT "," \
                                        SQLITE_COL_RIGHTS " " SQLITE_TYPE_TEXT \
                                        ");"

#define SQLITE_TRIGGER_D_CONTAINERS_ALBUMS SQLITE_DELETE_TRIGGER(SQLITE_TABLE_CONTAINERS, SQLITE_TABLE_ALBUMS)

#define SQLITE_TRIGGER_U_CONTAINERS_ALBUMS SQLITE_UPDATE_TRIGGER(SQLITE_TABLE_CONTAINERS,\
                                                                 SQLITE_TABLE_ALBUMS,\
                                                                 UPNP_CLASS_ALBUM)

#define SQLITE_TRIGGER_I_CONTAINERS_ALBUMS SQLITE_INSERT_TRIGGER(SQLITE_TABLE_CONTAINERS,\
                                                                 SQLITE_TABLE_ALBUMS,\
                                                                 UPNP_CLASS_ALBUM)

/**********************************************\
*                                              *
*  Playlists                                   *
*                                              *
\**********************************************/

#define SQLITE_CREATE_TABLE_PLAYLISTS   "CREATE TABLE IF NOT EXISTS "\
                                        SQLITE_TABLE_PLAYLISTS \
                                        "(" \
                                        SQLITE_UPNP_OBJECTID "," \
                                        SQLITE_COL_ARTIST " " SQLITE_TYPE_TEXT "," \
                                        SQLITE_COL_GENRE " " SQLITE_TYPE_TEXT "," \
                                        SQLITE_COL_LONGDESCRIPTION " " SQLITE_TYPE_TEXT "," \
                                        SQLITE_COL_DESCRIPTION " " SQLITE_TYPE_TEXT ","\
                                        SQLITE_COL_PRODUCER " " SQLITE_TYPE_TEXT "," \
                                        SQLITE_COL_STORAGEMEDIUM " " SQLITE_TYPE_TEXT "," \
                                        SQLITE_COL_CONTRIBUTOR " " SQLITE_TYPE_TEXT "," \
                                        SQLITE_COL_DATE " " SQLITE_TYPE_TEXT ","\
                                        SQLITE_COL_LANGUAGE " " SQLITE_TYPE_TEXT ","\
                                        SQLITE_COL_RIGHTS " " SQLITE_TYPE_TEXT\
                                        ");"

#define SQLITE_TRIGGER_D_CONTAINERS_PLAYLISTS SQLITE_DELETE_TRIGGER(SQLITE_TABLE_CONTAINERS, SQLITE_TABLE_PLAYLISTS)

#define SQLITE_TRIGGER_I_CONTAINERS_PLAYLISTS SQLITE_INSERT_TRIGGER(SQLITE_TABLE_CONTAINERS,\
                                                                    SQLITE_TABLE_PLAYLISTS,\
                                                                    UPNP_CLASS_PLAYLISTCONT)

#define SQLITE_TRIGGER_U_CONTAINERS_PLAYLISTS SQLITE_UPDATE_TRIGGER(SQLITE_TABLE_CONTAINERS,\
                                                                    SQLITE_TABLE_PLAYLISTS,\
                                                                    UPNP_CLASS_PLAYLISTCONT)

/**********************************************\
*                                              *
*  Search classes                              *
*                                              *
\**********************************************/

#define SQLITE_CREATE_TABLE_SEARCHCLASS "CREATE TABLE IF NOT EXISTS "\
                                        SQLITE_TABLE_SEARCHCLASS \
                                        "(" \
                                        SQLITE_COL_OBJECTID " " SQLITE_TYPE_INTEGER " " SQLITE_NOT_NULL "," \
                                        SQLITE_COL_CLASS " " SQLITE_TYPE_TEXT "," \
                                        SQLITE_COL_CLASSDERIVED " " SQLITE_TYPE_BOOL \
                                        ");"

#define SQLITE_TRIGGER_D_CONTAINERS_SEARCHCLASSES "CREATE TRIGGER IF NOT EXISTS " \
                                        SQLITE_TABLE_CONTAINERS "_D_" SQLITE_TABLE_SEARCHCLASS " " \
                                        "BEFORE DELETE ON " \
                                        SQLITE_TABLE_CONTAINERS " " \
                                        "FOR EACH ROW BEGIN "\
                                        "DELETE FROM " SQLITE_TABLE_SEARCHCLASS " "\
                                        "WHERE " SQLITE_COL_OBJECTID "= OLD." SQLITE_COL_OBJECTID "; " \
                                        "END;"

#define SQLITE_TRIGGER_U_CONTAINERS_SEARCHCLASSES "CREATE TRIGGER IF NOT EXISTS " \
                                        SQLITE_TABLE_CONTAINERS "_U_" SQLITE_TABLE_SEARCHCLASS " " \
                                        "BEFORE UPDATE ON " \
                                        SQLITE_TABLE_SEARCHCLASS " " \
                                        "FOR EACH ROW BEGIN "\
                                        "SELECT CASE "\
                                        "WHEN ("\
                                        "(SELECT " SQLITE_COL_OBJECTID " FROM " SQLITE_TABLE_CONTAINERS " "\
                                        "WHERE " SQLITE_COL_OBJECTID "=NEW." SQLITE_COL_OBJECTID ") IS NULL "\
                                        ") THEN "\
                                        "RAISE (" SQLITE_TRANSACTION_TYPE ", 'UPDATE on table " SQLITE_TABLE_SEARCHCLASS " "\
                                        "violates foreign key constraint \"" SQLITE_COL_OBJECTID "\"') " \
                                        "END; END;"

#define SQLITE_TRIGGER_I_CONTAINERS_SEARCHCLASSES "CREATE TRIGGER IF NOT EXISTS " \
                                        SQLITE_TABLE_CONTAINERS "_I_" SQLITE_TABLE_SEARCHCLASS " " \
                                        "BEFORE INSERT ON " \
                                        SQLITE_TABLE_SEARCHCLASS " " \
                                        "FOR EACH ROW BEGIN "\
                                        "SELECT CASE "\
                                        "WHEN ("\
                                        "(SELECT " SQLITE_COL_OBJECTID " FROM " SQLITE_TABLE_CONTAINERS " "\
                                        "WHERE " SQLITE_COL_OBJECTID "=NEW." SQLITE_COL_OBJECTID ") IS NULL "\
                                        ") THEN "\
                                        "RAISE (" SQLITE_TRANSACTION_TYPE ", 'INSERT on table " SQLITE_TABLE_SEARCHCLASS " "\
                                        "violates foreign key constraint \"" SQLITE_COL_OBJECTID "\"') " \
                                        "END; END;"

/**********************************************\
*                                              *
*  Resources                                   *
*                                              *
\**********************************************/

#define SQLITE_CREATE_TABLE_RESOURCES   "CREATE TABLE IF NOT EXISTS "\
                                        SQLITE_TABLE_RESOURCES \
                                        "(" \
                                        SQLITE_COL_RESOURCEID " " SQLITE_PRIMARY_KEY " " SQLITE_NOT_NULL "," \
                                        SQLITE_COL_OBJECTID " " SQLITE_TYPE_INTEGER " " SQLITE_NOT_NULL "," \
                                        SQLITE_COL_PROTOCOLINFO " " SQLITE_TYPE_TEXT " " SQLITE_NOT_NULL "," \
                                        SQLITE_COL_CONTENTTYPE " " SQLITE_TYPE_TEXT " " SQLITE_NOT_NULL "," \
                                        SQLITE_COL_RESOURCETYPE " " SQLITE_TYPE_INTEGER " " SQLITE_NOT_NULL "," \
                                        SQLITE_COL_RESOURCE " " SQLITE_TYPE_TEXT " " SQLITE_NOT_NULL "," \
                                        SQLITE_COL_SIZE " " SQLITE_TYPE_ULONG "," \
                                        SQLITE_COL_DURATION " " SQLITE_TYPE_TEXT "," \
                                        SQLITE_COL_BITRATE " " SQLITE_TYPE_UINTEGER "," \
                                        SQLITE_COL_SAMPLEFREQUENCE " " SQLITE_TYPE_UINTEGER "," \
                                        SQLITE_COL_BITSPERSAMPLE " " SQLITE_TYPE_UINTEGER "," \
                                        SQLITE_COL_NOAUDIOCHANNELS " " SQLITE_TYPE_UINTEGER "," \
                                        SQLITE_COL_COLORDEPTH " " SQLITE_TYPE_UINTEGER "," \
                                        SQLITE_COL_RESOLUTION " " SQLITE_TYPE_TEXT \
                                        ");"

#define SQLITE_TRIGGER_D_OBJECT_RESOURCES "CREATE TRIGGER IF NOT EXISTS " \
                                        SQLITE_TABLE_OBJECTS "_D_" SQLITE_TABLE_RESOURCES " " \
                                        "BEFORE DELETE ON " \
                                        SQLITE_TABLE_OBJECTS " " \
                                        "FOR EACH ROW BEGIN "\
                                        "DELETE FROM " SQLITE_TABLE_RESOURCES " "\
                                        "WHERE " SQLITE_COL_OBJECTID "= OLD." SQLITE_COL_OBJECTID "; " \
                                        "END;"

#define SQLITE_TRIGGER_I_OBJECT_RESOURCES "CREATE TRIGGER IF NOT EXISTS " \
                                        SQLITE_TABLE_OBJECTS "_I_" SQLITE_TABLE_RESOURCES " " \
                                        "BEFORE INSERT ON " \
                                        SQLITE_TABLE_RESOURCES " " \
                                        "FOR EACH ROW BEGIN "\
                                        "SELECT CASE "\
                                        "WHEN ("\
                                        "(SELECT " SQLITE_COL_OBJECTID " FROM " SQLITE_TABLE_OBJECTS " "\
                                        "WHERE " SQLITE_COL_OBJECTID "=NEW." SQLITE_COL_OBJECTID ") IS NULL"\
                                        ") THEN "\
                                        "RAISE (" SQLITE_TRANSACTION_TYPE ", 'INSERT on table " SQLITE_TABLE_RESOURCES " "\
                                        "violates foreign key constraint \"" SQLITE_COL_OBJECTID "\"') " \
                                        "END; END;"

#define SQLITE_TRIGGER_U_OBJECT_RESOURCES "CREATE TRIGGER IF NOT EXISTS " \
                                        SQLITE_TABLE_OBJECTS "_U_" SQLITE_TABLE_RESOURCES " " \
                                        "BEFORE UPDATE ON " \
                                        SQLITE_TABLE_RESOURCES " " \
                                        "FOR EACH ROW BEGIN "\
                                        "SELECT CASE "\
                                        "WHEN ("\
                                        "(SELECT " SQLITE_COL_OBJECTID " FROM " SQLITE_TABLE_OBJECTS " "\
                                        "WHERE " SQLITE_COL_OBJECTID "=NEW." SQLITE_COL_OBJECTID ") IS NULL"\
                                        ") THEN "\
                                        "RAISE (" SQLITE_TRANSACTION_TYPE ", 'INSERT on table " SQLITE_TABLE_RESOURCES " "\
                                        "violates foreign key constraint \"" SQLITE_COL_OBJECTID "\"') " \
                                        "END; END;"

class cSQLiteDatabase;

/**
 * Result row of a SQL SELECT request
 *
 * This is a single row of a {\c SQL SELECT} request.
 *
 * @see cRows
 */
class cRow : public cListObject {
    friend class cSQLiteDatabase;
private:
    int     currentCol;
    int     ColCount;
    char**  Columns;
    char**  Values;
    cRow();
public:
    virtual ~cRow();
    /**
     * Number of columns in this row
     *
     * @return the number of rows
     */
    int  Count(){ return this->ColCount; }
    /**
     * Fetches a Column
     *
     * This will fetch a column of this row and stores the name of the column
     * in the first parameter and the value in the second parameter.
     *
     * @return returns
     * - \bc true, if more columns to come
     * - \bc false, if the column is its last in this row.
     */
    bool fetchColumn(
        cString* Column, /**< The name of the current column */
        cString* Value /**< The value of the current value */
    );
    
    /**
     * Fetches a Column
     *
     * This will fetch a column of this row and stores the name of the column
     * in the first parameter and the value in the second parameter.
     *
     * @return returns
     * - \bc true, if more columns to come
     * - \bc false, if the column is its last in this row.
     */
    bool fetchColumn(
        char** Column, /**< The name of the current column */
        char** Value /**< The value of the current column */
    );
};

/**
 * Result rows of a SQL SELECT request
 *
 * Contains the rows of a SQL SELECT request
 *
 * @see cRow
 */
class cRows : public cList<cRow> {
    friend class cSQLiteDatabase;
private:
    cRow*   mLastRow;
    cRows();
public:
    virtual ~cRows();
    /**
     * Fetches a row from the result
     *
     * This fetches the next row in the resultset by storing the contents of
     * that row in the first parameter.
     *
     * @return returns
     * - \bc true, if more rows to come
     * - \bc false, if the row is its last in this resultset.
     */
    bool fetchRow(
        cRow** Row /**< The Pointer of the row */
    );
};

/**
 * SQLite Database
 *
 * This is a wrapper class for a SQLite3 database connection
 * It supports simple execution functions.
 *
 * On requests with returns any results a instance of \c cRows* will be created.
 */
class cSQLiteDatabase {
    friend class cStatement;
private:
    bool        mAutoCommit;
    bool        mActiveTransaction;
    cRow*       mLastRow;
    cRows*      mRows;
    sqlite3*    mDatabase;
    static cSQLiteDatabase* mInstance;
    cSQLiteDatabase();
    int initialize();
    int initializeTables();
    int initializeTriggers();
    static int getResultRow(void* DB, int NumCols, char** Values, char** ColNames);
    int exec(const char* Statement);
public:
    /**
     * Prints a SQLite escaped text
     *
     * Returns a formated text with special characters to escape SQLite special
     * characters like "'". Additionally to the well known characters of \a printf
     * the following are allowed:
     *
     * - \bc q, like s, escapes single quotes in strings
     * - \bc Q, like q, surrounds the escaped string with additional
     *             single quotes
     * - \bc z, frees the string after reading and coping it
     *
     * @see sprintf()
     * @return the formated string
     */
    static const char* sprintf(
        const char* Format, /**< The format string */
        ... /**< optional properties which will be passed to sprintf */
    );
    virtual ~cSQLiteDatabase();
    /**
     * Returns the instance of the database
     *
     * Returns the instance of the SQLite database. This will create a single
     * instance of none is existing on the very first call. A subsequent call
     * will return the same instance.
     *
     * @return the database instance
     */
    static cSQLiteDatabase* getInstance();
    /**
     * Row count of the last result
     *
     * Returns the row count of the last {\c SQL SELECT} request.
     *
     * @see cRows
     * @return the result row count
     */
    int getResultCount() const { return this->mRows->Count(); }
    /**
     * The last \c INSERT RowID
     *
     * Returns the primary key of the last inserted row.
     * This will only work if there are no successive calls to the database.
     *
     * @return the last insert RowID
     */
    long getLastInsertRowID() const;
    /**
     * Result set of the last request
     *
     * Returns the result rows of the SQL SELECT request.
     * This might be NULL, if the last statement was not a SELECT.
     *
     * @see cRows
     * @return the result rows of the last \c SELECT statement.
     */
    cRows* getResultRows() const { return this->mRows; }
    /**
     * Executes a SQL statement
     *
     * This will execute the statement in the first parameter. If it is followed
     * by any optional parameters it will be formated using the same function as
     * in \c cSQLiteDatabase::sprintf().
     *
     * \sa cSQLiteDatabase::sprintf().
     *
     * @return returns an integer representing
     * - \bc -1, in case of an error
     * - \bc 0, when the statement was executed successfuly
     */
    int execStatement(
        const char* Statement , /**< Statement to be executed */
        ... /**< optional parameters passed to the format string */
    );
    /**
     * Starts a transaction
     *
     * This starts a new transaction and commits or rolls back a previous.
     *
     * @see cSQLiteDatabase::setAutoCommit
     * @see cSQLiteDatabase::commitTransaction
     */
    void startTransaction();
    /**
     * Commits a transaction
     *
     * This function commits the transaction and writes all changes to the
     * database
     *
     * @see cSQLiteDatabase::startTransaction
     */
    void commitTransaction();
    /**
     * Performs a rollback on a transaction
     *
     * This function performs a rollback. No changes will be made to the
     * database
     *
     * @see cSQLiteDatabase::rollbackTransaction
     */
    void rollbackTransaction();
    /**
     * Set the commit behavior
     *
     * This function sets the auto commit behavior on new transactions with
     * \sa cSQLiteDatabase::startTransaction.
     *
     * - \bc true, commits the last transaction before starting a
     *              new one
     * - \bc false, performs a rollback on the old transaction
     *
     */
    void setAutoCommit(
        bool Commit=true /**< Switches the behavior of auto commit */
    ){ this->mAutoCommit = Commit; }
};

#endif	/* _DATABASE_H */