/* 
 * File:   resources.cpp
 * Author: savop
 * 
 * Created on 30. September 2009, 15:17
 */

#include <string.h>
#include <vdr/channels.h>
#include "upnp/dlna.h"
#include <vdr/tools.h>
#include "resources.h"
#include "avdetector.h"

cUPnPResource::cUPnPResource(){
    this->mBitrate = 0;
    this->mBitsPerSample = 0;
    this->mColorDepth = 0;
    this->mDuration = NULL;
    this->mImportURI = NULL;
    this->mNrAudioChannels = 0;
    this->mProtocolInfo = NULL;
    this->mResolution = NULL;
    this->mResource = NULL;
    this->mResourceID = 0;
    this->mSampleFrequency = 0;
    this->mSize = 0;
    this->mContentType = NULL;
}

time_t cUPnPResource::getLastModification() const {
    time_t Time;
    const cRecording* Recording;
    const cEvent* Event;
    switch(this->mResourceType){
        case UPNP_RESOURCE_CHANNEL:
        case UPNP_RESOURCE_URL:
            Time = time(NULL);
            break;
        case UPNP_RESOURCE_RECORDING:
            Recording = Recordings.GetByName(this->mResource);
            Event = (Recording)?Recording->Info()->GetEvent():NULL;
            Time = (Event)?Event->EndTime():time(NULL);
            break;
        case UPNP_RESOURCE_FILE:
            //break;
        default:
            ERROR("Invalid resource type. This resource might be broken");
            Time = -1;
    }
    return Time;
}

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
        MESSAGE(VERBOSE_METADATA, "Found cached resource");
        return Resource;
    }
    else if((Resource = this->mMediator->getResource(ResourceID))){
        MESSAGE(VERBOSE_METADATA, "Found resource in database");
        this->mResources->Add(Resource, ResourceID);
        return Resource;
    }
    else {
        ERROR("No such resource with ID '%d'", ResourceID);
        return NULL;
    }
}

int cUPnPResources::createFromRecording(cUPnPClassVideoItem* Object, cRecording* Recording){
    if(!Object || !Recording){
        ERROR("Invalid input arguments");
        return -1;
    }

    cAudioVideoDetector* Detector = new cAudioVideoDetector(Recording);

    if(Detector->detectProperties()){
        ERROR("Error while detecting video properties");
        delete Detector;
        return -1;
    }

    if(!Detector->getDLNAProfile()){
        ERROR("No DLNA profile found for Recording %s", Recording->Name());
        delete Detector;
        return -1;
    }

    const char* ProtocolInfo = cDlna::getInstance()->getProtocolInfo(Detector->getDLNAProfile());

    MESSAGE(VERBOSE_METADATA, "Protocol info: %s", ProtocolInfo);
    
    cString ResourceFile     = Recording->FileName();
    cUPnPResource* Resource  = this->mMediator->newResource(Object, UPNP_RESOURCE_RECORDING,ResourceFile, Detector->getDLNAProfile()->mime, ProtocolInfo);
    Resource->mBitrate       = Detector->getBitrate();
    Resource->mBitsPerSample = Detector->getBitsPerSample();
    Resource->mDuration      = duration(Detector->getDuration(), AVDETECTOR_TIME_BASE);
    Resource->mResolution    = (Detector->getWidth() && Detector->getHeight()) ? *cString::sprintf("%dx%d",Detector->getWidth(), Detector->getHeight()) : NULL;
    Resource->mSampleFrequency = Detector->getSampleFrequency();
    Resource->mSize          = Detector->getFileSize();
    Resource->mNrAudioChannels = Detector->getNumberOfAudioChannels();
    Resource->mImportURI     = NULL;
    Resource->mColorDepth    = 0;
    Object->addResource(Resource);
    this->mMediator->saveResource(Object, Resource);
    this->mResources->Add(Resource, Resource->getID());

    delete Detector;
    return 0;
}

int cUPnPResources::createFromFile(cUPnPClassItem* , cString ){
    MESSAGE(VERBOSE_SDK, "To be done");
    return -1;
}

int cUPnPResources::createFromChannel(cUPnPClassVideoBroadcast* Object, cChannel* Channel){
    if(!Object || !Channel){
        ERROR("Invalid input arguments");
        return -1;
    }

    cAudioVideoDetector* Detector = new cAudioVideoDetector(Channel);

    if(Detector->detectProperties()){
        ERROR("Cannot detect channel properties");
        delete Detector;
        return -1;
    }

    if(!Detector->getDLNAProfile()){
        ERROR("No DLNA profile found for Channel %s", *Channel->GetChannelID().ToString());
        delete Detector;
        return -1;
    }

    const char* ProtocolInfo = cDlna::getInstance()->getProtocolInfo(Detector->getDLNAProfile());

    MESSAGE(VERBOSE_METADATA, "Protocol info: %s", ProtocolInfo);

    // Index which may be used to indicate different resources with same channel ID
    // For instance a different DVB card
    // Not used yet.
    int index = 0;
    
    cString ResourceFile     = cString::sprintf("%s:%d", *Channel->GetChannelID().ToString(), index);
    cUPnPResource* Resource  = this->mMediator->newResource(Object, UPNP_RESOURCE_CHANNEL,ResourceFile, Detector->getDLNAProfile()->mime, ProtocolInfo);
    Resource->mBitrate       = Detector->getBitrate();
    Resource->mBitsPerSample = Detector->getBitsPerSample();
    Resource->mDuration      = duration(Detector->getDuration());
    Resource->mResolution    = (Detector->getWidth() && Detector->getHeight()) ? *cString::sprintf("%dx%d",Detector->getWidth(), Detector->getHeight()) : NULL;
    Resource->mSampleFrequency = Detector->getSampleFrequency();
    Resource->mSize          = Detector->getFileSize();
    Resource->mNrAudioChannels = Detector->getNumberOfAudioChannels();
    Resource->mImportURI     = NULL;
    Resource->mColorDepth    = 0;
    Object->addResource(Resource);
    this->mMediator->saveResource(Object, Resource);
    this->mResources->Add(Resource, Resource->getID());

    delete Detector;
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
        if(!strcasecmp(SQLITE_COL_PROTOCOLINFO, Column)){
            Resource->mProtocolInfo = Value;
        }
        else if(!strcasecmp(SQLITE_COL_RESOURCE, Column)){
            Resource->mResource = Value;
        }
        else if(!strcasecmp(SQLITE_COL_SIZE, Column)){
            Resource->mSize = (off64_t)(*Value?atol(Value):0);
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

int cUPnPResourceMediator::saveResource(cUPnPClassObject* Object, cUPnPResource* Resource){

    cString Format = "UPDATE %s SET %s=%Q,"
                                   "%s=%Q,"
                                   "%s=%Q,"
                                   "%s=%lld,"
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
                                         SQLITE_COL_OBJECTID, *Object->getID(),
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
    Resource->mResource = ResourceFile;
    Resource->mProtocolInfo = ProtocolInfo;
    Resource->mContentType = ContentType;
    Resource->mResourceType = ResourceType;
    
    return Resource;
}