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
    if(this->mDatabase->execStatement("SELECT %s FROM %s",SQLITE_COL_RESOURCEID,SQLITE_TABLE_RESOURCES)){
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
    if(this->mDatabase->execStatement("SELECT %s FROM %s WHERE %s=%Q",
                                        SQLITE_COL_RESOURCEID,
                                        SQLITE_TABLE_RESOURCES,
                                        SQLITE_COL_OBJECTID,
                                        *Object->getID())){
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

    // TODO: add DLNA-Support to detector

    cString ContentType = "video/mpeg";
    cString ProtocolInfo = "http-get:*:video/mpeg:*";

    cUPnPResource* Resource  = this->mMediator->newResource(Object, UPNP_RESOURCE_RECORDING, Recording->FileName(), ContentType, ProtocolInfo);

    if(Detector->detectVideoProperties(Resource, VideoFile)){
        ERROR("Error while detecting video properties");
        return -1;
    }

    delete Detector;
    MESSAGE("To be continued, may it work with DLNA?! Guess, not!");
    return 0;
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
    if(this->mDatabase->execStatement("SELECT * FROM %s WHERE %s=%d",
                                        SQLITE_TABLE_RESOURCES,
                                        SQLITE_COL_RESOURCEID,
                                        ResourceID)){
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
            Resource->mObjectID = *Value?atoi(Value):-1;
        }
        else if(!strcasecmp(SQLITE_COL_PROTOCOLINFO, Column)){
            Resource->mProtocolInfo = Value;
        }
        else if(!strcasecmp(SQLITE_COL_RESOURCE, Column)){
            Resource->mResource = Value;
        }
        else if(!strcasecmp(SQLITE_COL_SIZE, Column)){
            Resource->mSize = *Value?atol(Value):0;
        }
        else if(!strcasecmp(SQLITE_COL_DURATION, Column)){
            Resource->mDuration = Value;
        }
        else if(!strcasecmp(SQLITE_COL_BITRATE, Column)){
            Resource->mBitrate = *Value?atoi(Value):0;
        }
        else if(!strcasecmp(SQLITE_COL_SAMPLEFREQUENCE, Column)){
            Resource->mSampleFrequency = *Value?atoi(Value):0;
        }
        else if(!strcasecmp(SQLITE_COL_BITSPERSAMPLE, Column)){
            Resource->mBitsPerSample = *Value?atoi(Value):0;
        }
        else if(!strcasecmp(SQLITE_COL_NOAUDIOCHANNELS, Column)){
            Resource->mNrAudioChannels = *Value?atoi(Value):0;
        }
        else if(!strcasecmp(SQLITE_COL_COLORDEPTH, Column)){
            Resource->mColorDepth = *Value?atoi(Value):0;
        }
        else if(!strcasecmp(SQLITE_COL_RESOLUTION, Column)){
            Resource->mResolution = Value;
        }
        else if(!strcasecmp(SQLITE_COL_CONTENTTYPE, Column)){
            Resource->mContentType = Value;
        }
        else if(!strcasecmp(SQLITE_COL_RESOURCETYPE, Column)){
            Resource->mResourceType = *Value?atoi(Value):0;
        }
    }
    return Resource;
}

int cUPnPResourceMediator::saveResource(cUPnPResource* Resource){

    cString Format = "UPDATE %s SET %s=%Q,"
                                   "%s=%Q,"
                                   "%s=%Q,"
                                   "%s=%ld,"
                                   "%s=%Q,"
                                   "%s=%d,"
                                   "%s=%d,"
                                   "%s=%d,"
                                   "%s=%d,"
                                   "%s=%d,"
                                   "%s=%Q,"
                                   "%s=%Q,"
                                   "%s=%d"
                                   " WHERE %s=%d";

    if(this->mDatabase->execStatement(Format,
                                         SQLITE_TABLE_RESOURCES,
                                         SQLITE_COL_OBJECTID, *Resource->mObjectID,
                                         SQLITE_COL_PROTOCOLINFO, *Resource->mProtocolInfo,
                                         SQLITE_COL_RESOURCE, *Resource->mResource,
                                         SQLITE_COL_SIZE, Resource->mSize,
                                         SQLITE_COL_DURATION, *Resource->mDuration,
                                         SQLITE_COL_BITRATE, Resource->mBitrate,
                                         SQLITE_COL_SAMPLEFREQUENCE, Resource->mSampleFrequency,
                                         SQLITE_COL_BITSPERSAMPLE, Resource->mBitsPerSample,
                                         SQLITE_COL_NOAUDIOCHANNELS, Resource->mNrAudioChannels,
                                         SQLITE_COL_COLORDEPTH, Resource->mColorDepth,
                                         SQLITE_COL_RESOLUTION, *Resource->mResolution,
                                         SQLITE_COL_CONTENTTYPE, *Resource->mContentType,
                                         SQLITE_COL_RESOURCETYPE, Resource->mResourceType,
                                         SQLITE_COL_RESOURCEID, Resource->mResourceID)){
        ERROR("Error while executing statement");
        return -1;
    }

    return 0;
}

cUPnPResource* cUPnPResourceMediator::newResource(cUPnPClassObject* Object, int ResourceType, cString ResourceFile, cString ContentType, cString ProtocolInfo){
    cUPnPResource* Resource = new cUPnPResource;
    
    if(this->mDatabase->execStatement("INSERT INTO %s (%s,%s,%s,%s,%s) VALUES (%Q,%Q,%Q,%Q,%d)",
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
                                         ResourceType)){
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