/* 
 * File:   resources.cpp
 * Author: savop
 * 
 * Created on 30. September 2009, 15:17
 */

#include <string.h>
#include <vdr/channels.h>
#include "../upnpcomponents/dlna.h"
#include <vdr/tools.h>
#include "resources.h"
#include "../misc/avdetector.h"

cUPnPResources* cUPnPResources::mInstance = NULL;

cUPnPResources::cUPnPResources(){
    this->mResources = new cHash<cUPnPResource>;
    this->mMediator = new cUPnPResourceMediator;
    this->mDatabase = cSQLiteDatabase::getInstance();
}

cUPnPResources::~cUPnPResources(){
    delete this->mResources;
    delete this->mMediator;
}

cUPnPResources* cUPnPResources::getInstance(){
    if(!cUPnPResources::mInstance)
        cUPnPResources::mInstance = new cUPnPResources();
    if(cUPnPResources::mInstance) return cUPnPResources::mInstance;
    else return NULL;
}

int cUPnPResources::loadResources(){
    cString Statement = cString::sprintf("SELECT %s FROM %s",
                                        SQLITE_COL_RESOURCEID,
                                        SQLITE_TABLE_RESOURCES
                                        );
    if(this->mDatabase->execStatement(Statement)){
        ERROR("Error while executing statement");
        return -1;
    }
    cRows* Rows = this->mDatabase->getResultRows(); cRow* Row;
    cString Column = NULL, Value = NULL;
    while(Rows->fetchRow(&Row)){
        while(Row->fetchColumn(&Column, &Value)){
            if(!strcasecmp(Column, SQLITE_COL_RESOURCEID)){
                unsigned int ResourceID = (unsigned int)atoi(Value);
                this->getResource(ResourceID);
            }
        }
    }
    return 0;
}

int cUPnPResources::getResourcesOfObject(cUPnPClassObject* Object){
    cString Statement = cString::sprintf("SELECT %s FROM %s WHERE %s='%s'",
                                        SQLITE_COL_RESOURCEID,
                                        SQLITE_TABLE_RESOURCES,
                                        SQLITE_COL_OBJECTID,
                                        *Object->getID()
                                        );
    if(this->mDatabase->execStatement(Statement)){
        ERROR("Error while executing statement");
        return -1;
    }
    cRows* Rows = this->mDatabase->getResultRows(); cRow* Row;
    cString Column = NULL, Value = NULL;
    while(Rows->fetchRow(&Row)){
        while(Row->fetchColumn(&Column, &Value)){
            if(!strcasecmp(Column, SQLITE_COL_RESOURCEID)){
                unsigned int ResourceID = (unsigned int)atoi(Value);
                Object->addResource(this->getResource(ResourceID));
            }
        }
    }
    return 0;
}

cUPnPResource* cUPnPResources::getResource(unsigned int ResourceID){
    cUPnPResource* Resource;
    if((Resource = this->mResources->Get(ResourceID))){
        MESSAGE("Found cached resource");
        return Resource;
    }
    else if((Resource = this->mMediator->getResource(ResourceID))){
        MESSAGE("Found resource in database");
        this->mResources->Add(Resource, ResourceID);
        return Resource;
    }
    else {
        ERROR("No such resource with ID '%d'", ResourceID);
        return NULL;
    }
}

int cUPnPResources::createFromRecording(cUPnPClassVideoItem* Object, cRecording* Recording){
    cString VideoFile = cString::sprintf(VDR_RECORDFILE_PATTERN_TS, Recording->FileName(), 1);

    cAudioVideoDetector* Detector = new cAudioVideoDetector();

    cString ContentType = "video/mpeg";
    cString ProtocolInfo = "http-get:*:video/mpeg:*";

    cUPnPResource* Resource  = this->mMediator->newResource(Object, UPNP_RESOURCE_RECORDING, Recording->FileName(), ContentType, ProtocolInfo);

    if(Detector->detectVideoProperties(Resource, VideoFile)){
        ERROR("Error while detecting video properties");
        return -1;
    }

    delete Detector;
    MESSAGE("To be done");
    return -1;
}

int cUPnPResources::createFromFile(cUPnPClassItem* , cString ){
    MESSAGE("To be done");
    return -1;
}

int cUPnPResources::createFromChannel(cUPnPClassVideoBroadcast* Object, cChannel* Channel){
    if(!Object || !Channel){
        ERROR("Invalid input arguments");
        return -1;
    }

    DLNAProfile* Profile = cDlna::getInstance()->getProfileOfChannel(Channel);

    if(!Profile){
        ERROR("No profile found for Channel %s", *Channel->GetChannelID().ToString());
        return -1;
    }

    const char* ProtocolInfo = cDlna::getInstance()->getProtocolInfo(Profile);

    MESSAGE("Protocol info: %s", ProtocolInfo);

    // Adapted from streamdev
    int index = 0;
    for(int i=0; Channel->Apid(i)!=0; i++, index++){
        MESSAGE("Analog channel %d", i);
        cString ResourceFile     = cString::sprintf("%s:%d", *Channel->GetChannelID().ToString(), index);
        cUPnPResource* Resource  = this->mMediator->newResource(Object, UPNP_RESOURCE_CHANNEL,ResourceFile, Profile->mime, ProtocolInfo);
        Resource->mBitrate       = 0;
        Resource->mBitsPerSample = 0;
        Resource->mColorDepth    = 0;
        Resource->mDuration      = NULL;
        Resource->mImportURI     = NULL;
        Resource->mResolution    = NULL;
        Resource->mSampleFrequency = 0;
        Resource->mSize          = 0;
        Resource->mNrAudioChannels = 0;
        Object->addResource(Resource);
        this->mMediator->saveResource(Resource);
        this->mResources->Add(Resource, Resource->getID());
    }
    for(int i=0; Channel->Dpid(i)!=0; i++, index++){
        MESSAGE("Digital channel %d", i);
        cString ResourceFile     = cString::sprintf("%s:%d", *Channel->GetChannelID().ToString(), index);
        cUPnPResource* Resource  = this->mMediator->newResource(Object, UPNP_RESOURCE_CHANNEL,ResourceFile, Profile->mime, ProtocolInfo);
        Resource->mBitrate       = 0;
        Resource->mBitsPerSample = 0;
        Resource->mColorDepth    = 0;
        Resource->mDuration      = NULL;
        Resource->mImportURI     = NULL;
        Resource->mResolution    = NULL;
        Resource->mSampleFrequency = 0;
        Resource->mSize          = 0;
        Object->addResource(Resource);
        this->mMediator->saveResource(Resource);
        this->mResources->Add(Resource, Resource->getID());
    }

    return 0;
}

cUPnPResourceMediator::cUPnPResourceMediator(){
    this->mDatabase = cSQLiteDatabase::getInstance();
}

cUPnPResourceMediator::~cUPnPResourceMediator(){}

cUPnPResource* cUPnPResourceMediator::getResource(unsigned int ResourceID){
    cUPnPResource* Resource = new cUPnPResource;
    Resource->mResourceID = ResourceID;
    cString Statement = cString::sprintf("SELECT * FROM %s WHERE %s=%d",
                                        SQLITE_TABLE_RESOURCES,
                                        SQLITE_COL_RESOURCEID,
                                        ResourceID
                                        );
    if(this->mDatabase->execStatement(Statement)){
        ERROR("Error while executing statement");
        return NULL;
    }
    cRows* Rows = this->mDatabase->getResultRows(); cRow* Row;
    if(!Rows->fetchRow(&Row)){
        ERROR("No such resource found");
        return NULL;
    }
    cString Column = NULL, Value = NULL;
    while(Row->fetchColumn(&Column, &Value)){
        if(!strcasecmp(SQLITE_COL_OBJECTID, Column)){
            Resource->mObjectID = atoi(Value);
        }
        else if(!strcasecmp(SQLITE_COL_PROTOCOLINFO, Column)){
            Resource->mProtocolInfo = Value;
        }
        else if(!strcasecmp(SQLITE_COL_RESOURCE, Column)){
            Resource->mResource = Value;
        }
        else if(!strcasecmp(SQLITE_COL_SIZE, Column)){
            Resource->mSize = atol(Value);
        }
        else if(!strcasecmp(SQLITE_COL_DURATION, Column)){
            Resource->mDuration = Value;
        }
        else if(!strcasecmp(SQLITE_COL_BITRATE, Column)){
            Resource->mBitrate = atoi(Value);
        }
        else if(!strcasecmp(SQLITE_COL_SAMPLEFREQUENCE, Column)){
            Resource->mSampleFrequency = atoi(Value);
        }
        else if(!strcasecmp(SQLITE_COL_BITSPERSAMPLE, Column)){
            Resource->mBitsPerSample = atoi(Value);
        }
        else if(!strcasecmp(SQLITE_COL_NOAUDIOCHANNELS, Column)){
            Resource->mNrAudioChannels = atoi(Value);
        }
        else if(!strcasecmp(SQLITE_COL_COLORDEPTH, Column)){
            Resource->mColorDepth = atoi(Value);
        }
        else if(!strcasecmp(SQLITE_COL_RESOLUTION, Column)){
            Resource->mResolution = Value;
        }
        else if(!strcasecmp(SQLITE_COL_CONTENTTYPE, Column)){
            Resource->mContentType = Value;
        }
        else if(!strcasecmp(SQLITE_COL_RESOURCETYPE, Column)){
            Resource->mResourceType = atoi(Value);
        }
    }
    return Resource;
}

int cUPnPResourceMediator::saveResource(cUPnPResource* Resource){
    cString Format = "UPDATE %s SET %s WHERE %s=%d";
    cString Sets   = cString::sprintf("%s='%s',"
                                      "%s='%s',"
                                      "%s='%s',"
                                      "%s=%ld,"
                                      "%s='%s',"
                                      "%s=%d,"
                                      "%s=%d,"
                                      "%s=%d,"
                                      "%s=%d,"
                                      "%s=%d,"
                                      "%s='%s',"
                                      "%s='%s',"
                                      "%s=%d",
                                      SQLITE_COL_OBJECTID, *Resource->mObjectID?*Resource->mObjectID:"NULL",
                                      SQLITE_COL_PROTOCOLINFO, *Resource->mProtocolInfo?*Resource->mProtocolInfo:"NULL",
                                      SQLITE_COL_RESOURCE, *Resource->mResource?*Resource->mResource:"NULL",
                                      SQLITE_COL_SIZE, Resource->mSize,
                                      SQLITE_COL_DURATION, *Resource->mDuration?*Resource->mDuration:"NULL",
                                      SQLITE_COL_BITRATE, Resource->mBitrate,
                                      SQLITE_COL_SAMPLEFREQUENCE, Resource->mSampleFrequency,
                                      SQLITE_COL_BITSPERSAMPLE, Resource->mBitsPerSample,
                                      SQLITE_COL_NOAUDIOCHANNELS, Resource->mNrAudioChannels,
                                      SQLITE_COL_COLORDEPTH, Resource->mColorDepth,
                                      SQLITE_COL_RESOLUTION, *Resource->mResolution?*Resource->mResolution:"NULL",
                                      SQLITE_COL_CONTENTTYPE, *Resource->mContentType,
                                      SQLITE_COL_RESOURCETYPE, Resource->mResourceType
                                     );

    cString Statement = cString::sprintf(Format, SQLITE_TABLE_RESOURCES, *Sets, SQLITE_COL_RESOURCEID, Resource->mResourceID);

    if(this->mDatabase->execStatement(Statement)){
        ERROR("Error while executing statement");
        return -1;
    }

    return 0;
}

cUPnPResource* cUPnPResourceMediator::newResource(cUPnPClassObject* Object, int ResourceType, cString ResourceFile, cString ContentType, cString ProtocolInfo){
    cUPnPResource* Resource = new cUPnPResource;
    cString Statement = cString::sprintf("INSERT INTO %s (%s,%s,%s,%s,%s) VALUES ('%s','%s','%s','%s','%d')",
                                         SQLITE_TABLE_RESOURCES,
                                         SQLITE_COL_OBJECTID,
                                         SQLITE_COL_RESOURCE,
                                         SQLITE_COL_PROTOCOLINFO,
                                         SQLITE_COL_CONTENTTYPE,
                                         SQLITE_COL_RESOURCETYPE,
                                         *Object->getID(),
                                         *ResourceFile,
                                         *ProtocolInfo,
                                         *ContentType,
                                         ResourceType
                                        );
    if(this->mDatabase->execStatement(Statement)){
        ERROR("Error while executing statement");
        return NULL;
    }
    Resource->mResourceID = (unsigned int)this->mDatabase->getLastInsertRowID();
    Resource->mObjectID = Object->getID();
    Resource->mResource = ResourceFile;
    Resource->mProtocolInfo = ProtocolInfo;
    Resource->mContentType = ContentType;
    Resource->mResourceType = ResourceType;

    return Resource;
}