/*
 * File:   metadata.cpp
 * Author: savop
 *
 * Created on 28. Mai 2009, 16:50
 */

#include <upnp/ixml.h>
#include <time.h>
#include <vdr/tools.h>
#include "object.h"
#include "resources.h"
#include "metadata.h"
#include "../common.h"
#include "search.h"
#include <vdr/channels.h>
#include <vdr/epg.h>
#include <upnp/upnp.h>
#include <vdr/device.h>

#define KEY_SYSTEM_UPDATE_ID        "SystemUpdateID"

 /**********************************************\
 *                                              *
 *  Media database                              *
 *                                              *
 \**********************************************/

cMediaDatabase::cMediaDatabase(){
    this->mSystemUpdateID = 0;
    this->mLastInsertObjectID = 0;
    this->mDatabase = cSQLiteDatabase::getInstance();
    this->mObjects = new cHash<cUPnPClassObject>;
    this->mFactory = cUPnPObjectFactory::getInstance();
    this->mFactory->registerMediator(UPNP_CLASS_ITEM, new cUPnPItemMediator(this));
    this->mFactory->registerMediator(UPNP_CLASS_CONTAINER, new cUPnPContainerMediator(this));
    this->mFactory->registerMediator(UPNP_CLASS_VIDEO, new cUPnPVideoItemMediator(this));
    this->mFactory->registerMediator(UPNP_CLASS_VIDEOBC, new cUPnPVideoBroadcastMediator(this));
    this->mFactory->registerMediator(UPNP_CLASS_MOVIE, new cUPnPMovieMediator(this));
}

cMediaDatabase::~cMediaDatabase(){
    delete this->mDatabase;
}

bool cMediaDatabase::init(){
    MESSAGE(VERBOSE_SDK,"Initializing...");
    if(this->prepareDatabase()){
        ERROR("Initializing of database failed.");
        return false;
    }
#ifndef WITHOUT_TV
    if(this->loadChannels()){
        ERROR("Loading channels failed");
        return false;
    }
#endif
#ifndef WITHOUT_RECORDS
    if(this->loadRecordings()){
        ERROR("Loading records failed");
        return false;
    }
#endif
    return true;
}

void cMediaDatabase::updateSystemID(){
    this->mDatabase->execStatement("INSERT OR REPLACE INTO %s (Key,Value) VALUES (%Q,%d)",
                                    SQLITE_TABLE_SYSTEM,
                                    KEY_SYSTEM_UPDATE_ID,
                                    this->getSystemUpdateID()+1);
}

const char* cMediaDatabase::getContainerUpdateIDs(){
    return "";
}

unsigned int cMediaDatabase::getSystemUpdateID(){
    if(this->mDatabase->execStatement("SELECT Value FROM %s WHERE Key=%Q",
                                        SQLITE_TABLE_SYSTEM,
                                        KEY_SYSTEM_UPDATE_ID)){
        ERROR("Error while executing statement");
        return 0;
    }
    cRows* Rows = this->mDatabase->getResultRows();
    cRow* Row;
    cString Column, Value;
    if(!Rows->fetchRow(&Row)){
        ERROR("No rows found");
        return 0;
    }
    while(Row->fetchColumn(&Column, &Value)){
        if(!strcasecmp(Column, "Value")){
            this->mSystemUpdateID = (unsigned int)atoi(Value);
        }
    }
    return this->mSystemUpdateID;
}

cUPnPObjectID cMediaDatabase::getNextObjectID(){
    cString Column, Value;
    if(this->mDatabase->execStatement("SELECT Key FROM %s WHERE KeyID=%Q",
                                      SQLITE_TABLE_PRIMARY_KEYS,
                                      PK_OBJECTS)){
        ERROR("Error while executing statement");
        return 0;
    }
    cRows* Rows = this->mDatabase->getResultRows();
    cRow* Row;
    int ret = 0;
    if(!Rows->fetchRow(&Row)){
        ERROR("No rows found");
        ret = 0;
    }
    else {
        while(Row->fetchColumn(&Column, &Value)){
            if(!strcasecmp(Column, "Key")){
                this->mLastInsertObjectID = atoi(Value);
                ret = this->mLastInsertObjectID;
            }
        }
    }
    delete Rows;
    return ret;
}

int cMediaDatabase::addFastFind(cUPnPClassObject* Object, const char* FastFind){
    if(!Object || !FastFind){
        MESSAGE(VERBOSE_OBJECTS,"Invalid fast find parameters");
        return -1;
    }

    if(this->mDatabase->execStatement("INSERT OR REPLACE INTO %s (%s, %s) VALUES (%Q, %Q)",
            SQLITE_TABLE_ITEMFINDER,
            SQLITE_COL_OBJECTID,
            SQLITE_COL_ITEMFINDER,
            *Object->getID(),
            FastFind)){
        ERROR("Error while executing statement");
        return -1;
    }
    return 0;
}

cUPnPClassObject* cMediaDatabase::getObjectByFastFind(const char* FastFind){
    if(!FastFind) return NULL;
    MESSAGE(VERBOSE_OBJECTS,"Try to find Object with identifier %s", FastFind);
    cString Column, Value;
    if(this->mDatabase->execStatement("SELECT %s FROM %s WHERE %s=%Q",
                                       SQLITE_COL_OBJECTID,
                                       SQLITE_TABLE_ITEMFINDER,
                                       SQLITE_COL_ITEMFINDER,
                                       FastFind)){
        ERROR("Error while executing statement");
        return 0;
    }
    cRows* Rows = this->mDatabase->getResultRows();
    cRow* Row;
    if(!Rows->fetchRow(&Row)){
        ERROR("No rows found");
        return NULL;
    }
    while(Row->fetchColumn(&Column, &Value)){
        if(!strcasecmp(Column, SQLITE_COL_OBJECTID)){
            return this->getObjectByID(atoi(Value));
        }
    }
    delete Rows;
    return NULL;
}

cUPnPClassObject* cMediaDatabase::getObjectByID(cUPnPObjectID ID){
    MESSAGE(VERBOSE_OBJECTS, "Try to find Object with ID '%s'", *ID);
    cUPnPClassObject* Object;
    if((Object = this->mObjects->Get((unsigned int)ID))){
        MESSAGE(VERBOSE_OBJECTS, "Found cached object with ID '%s'", *ID);
    }
    else if((Object = this->mFactory->getObject(ID))){
        //this->cacheObject(Object);
        MESSAGE(VERBOSE_OBJECTS, "Found object with ID '%s' in database", *ID);
    }
    else {
        ERROR("No object with such ID '%s'", *ID);
        return NULL;
    }
    return Object;
}

void cMediaDatabase::cacheObject(cUPnPClassObject* Object){
    if(this->mObjects->Get((unsigned int)Object->getID())==NULL){
        MESSAGE(VERBOSE_OBJECTS ,"Added %s to cache.", *Object->getID());
        this->mObjects->Add(Object, (unsigned int)Object->getID());
    }
}

int cMediaDatabase::prepareDatabase(){
    if(this->getObjectByID(0)==NULL){
        MESSAGE(VERBOSE_SDK, "Creating database structure");
        cUPnPClassContainer* Root = (cUPnPClassContainer*)this->mFactory->createObject(UPNP_CLASS_CONTAINER, _(PLUGIN_SHORT_NAME));
        Root->setID(0);
        if(this->mFactory->saveObject(Root)) return -1;

#ifndef WITHOUT_VIDEO
        cUPnPClassContainer* Video  = (cUPnPClassContainer*)this->mFactory->createObject(UPNP_CLASS_CONTAINER, _("Video"));
        Video->setID(1);
        Root->addObject(Video);
        cClass VideoClass = { UPNP_CLASS_VIDEO, true };
        Video->addSearchClass(VideoClass);
        Video->setSearchable(true);
        if(this->mFactory->saveObject(Video)) return -1;
#endif
#ifndef WITHOUT_AUDIO
        cUPnPClassContainer* Audio  = (cUPnPClassContainer*)this->mFactory->createObject(UPNP_CLASS_CONTAINER, _("Audio"));
        Audio->setID(2);
        Root->addObject(Audio);
        cClass AudioClass = { UPNP_CLASS_AUDIO, true };
        Audio->addSearchClass(AudioClass);
        Audio->setSearchable(true);
        if(this->mFactory->saveObject(Audio)) return -1;
#endif
#ifndef WITHOUT_TV
        cUPnPClassContainer* TV     = (cUPnPClassContainer*)this->mFactory->createObject(UPNP_CLASS_CONTAINER, _("TV"));
        TV->setID(3);
        TV->setContainerType(DLNA_CONTAINER_TUNER);
        TV->setSearchable(true);
        cClass VideoBCClass = { UPNP_CLASS_VIDEOBC, true };
        TV->addSearchClass(VideoBCClass);
        Video->addObject(TV);
        if(this->mFactory->saveObject(TV)) return -1;
#endif
#ifndef WITHOUT_RECORDS
        cUPnPClassContainer* Records = (cUPnPClassContainer*)this->mFactory->createObject(UPNP_CLASS_CONTAINER, _("Records"));
        Records->setID(4);
        Video->addObject(Records);
        Records->addSearchClass(VideoClass);
        Records->setSearchable(true);
        if(this->mFactory->saveObject(Records)) return -1;
#endif
#ifndef WITHOUT_RADIO
        cUPnPClassContainer* Radio  = (cUPnPClassContainer*)this->mFactory->createObject(UPNP_CLASS_CONTAINER, _("Radio"));
        Radio->setID(5);
        Audio->addObject(Radio);
        cClass AudioBCClass = { UPNP_CLASS_AUDIOBC, true };
        Radio->addSearchClass(AudioBCClass);
        Radio->setSearchable(true);
        if(this->mFactory->saveObject(Radio)) return -1;
#endif
#ifndef WITHOUT_CUSTOM_VIDEOS
        cUPnPClassContainer* CustomVideos = (cUPnPClassContainer*)this->mFactory->createObject(UPNP_CLASS_CONTAINER, _("User videos"));
        CustomVideos->setID(6);
        Video->addObject(CustomVideos);
        CustomVideos->addSearchClass(VideoClass);
        CustomVideos->setSearchable(true);
        if(this->mFactory->saveObject(CustomVideos)) return -1;
#endif
    }
    return 0;
}

#ifndef WITHOUT_TV
int cMediaDatabase::loadChannels(){
    MESSAGE(VERBOSE_LIVE_TV ,"Loading channels");
    cUPnPClassContainer* TV = (cUPnPClassContainer*)this->getObjectByID(3);
    if(TV){
        bool noResource = false;
        // TODO: Add to setup
        // if an error occured while loading resources, add the channel anyway
        bool addWithoutResources = false;
        cChannel* Channel = NULL;
        for(int Index = 0; (Channel = Channels.Get(Index)); Index = Channels.GetNextNormal(Index)){
            // Iterating the channels
//        for(Channel = Channels.First(); Channel; Channel = Channels.(Channel)){
            bool inList = false;

            tChannelID ChannelID = Channel->GetChannelID();
            MESSAGE(VERBOSE_LIVE_TV, "Determine if the channel %s is already listed", *ChannelID.ToString());
            cUPnPClassVideoBroadcast* ChannelItem = NULL;

            ChannelItem = (cUPnPClassVideoBroadcast*)this->getObjectByFastFind(ChannelID.ToString());

            inList = (ChannelItem && TV->getObject(ChannelItem->getID())) ? true : false;
            
            if(!inList){
                if(Channel->GroupSep()){
                    MESSAGE(VERBOSE_LIVE_TV, "Skipping group '%s'", Channel->Name());
                    // Skip channel groups
                    // Channel groups may be supported theoretically. However, DLNA states that a tuner needs
                    // a consecutive list of channels. A simple work-around may be a virtual tuner for each group.
                }
                else if(Channel->Vpid()==0){
                    // TODO: add radio support
                    MESSAGE(VERBOSE_LIVE_TV, "Skipping radio '%s'", Channel->Name());
                }
                else {
                    noResource = false;
                    MESSAGE(VERBOSE_LIVE_TV, "Adding channel '%s' ID:%s", Channel->Name(), *ChannelID.ToString());
                    ChannelItem = (cUPnPClassVideoBroadcast*)this->mFactory->createObject(UPNP_CLASS_VIDEOBC, Channel->Name());
                    ChannelItem->setChannelName(Channel->Name());
                    ChannelItem->setChannelNr(Channel->Number());
                    // Set primary language of the stream
                    if(Channel->Alang(0)){
                        ChannelItem->setLanguage(Channel->Alang(0));
                    }
                    if(cUPnPResources::getInstance()->createFromChannel(ChannelItem, Channel)){
                        ERROR("Unable to get resources for this channel");
                        noResource = true;
                    }
                    if(!noResource || addWithoutResources) {
                        TV->addObject(ChannelItem);
                        if(this->mFactory->saveObject(ChannelItem) ||
                           this->addFastFind(ChannelItem, ChannelID.ToString())){
                            this->mFactory->deleteObject(ChannelItem);
                            return -1;
                        }
                        MESSAGE(VERBOSE_LIVE_TV, "Successfuly added channel");
                    }
                    else {
                        // Delete temporarily created object with no resource
                        this->mFactory->deleteObject(ChannelItem);
                    }
                }
            }
            else {
                MESSAGE(VERBOSE_LIVE_TV, "Skipping %s, already in database", Channel->Name());
            }
        }
    }
    return 0;
}

void cMediaDatabase::updateChannelEPG(){
    cUPnPClassContainer* TV = (cUPnPClassContainer*)this->getObjectByID(3);
    if(TV){
        // Iterating channels
        MESSAGE(VERBOSE_EPG_UPDATES, "Getting schedule...");
        cSchedulesLock SchedulesLock;
        const cSchedules *Schedules = cSchedules::Schedules(SchedulesLock);

        cList<cUPnPClassObject>* List = TV->getObjectList();
        MESSAGE(VERBOSE_EPG_UPDATES, "TV folder has %d items", List->Count());
        for(cUPnPClassVideoBroadcast* ChannelItem = (cUPnPClassVideoBroadcast*)List->First();
            ChannelItem;
            ChannelItem = (cUPnPClassVideoBroadcast*)List->Next(ChannelItem)
            ){
            MESSAGE(VERBOSE_EPG_UPDATES, "Find channel by number %d", ChannelItem->getChannelNr());
            cChannel* Channel = Channels.GetByNumber(ChannelItem->getChannelNr());

            if(!Channel){
                continue;
            }
            else {
                MESSAGE(VERBOSE_EPG_UPDATES, "Found channel with ID %s", *Channel->GetChannelID().ToString());

                const cSchedule* Schedule = Schedules->GetSchedule(Channel);
                const cEvent* Event = Schedule?Schedule->GetPresentEvent():NULL;
                if(Event){

                    time_t LastEPGChange = Event->StartTime();
                    time_t LastObjectChange = ChannelItem->modified();

                    MESSAGE(VERBOSE_EPG_UPDATES, "Last event start: %s", ctime(&LastEPGChange));
                    MESSAGE(VERBOSE_EPG_UPDATES, "Last object modification:   %s", ctime(&LastObjectChange));
                    if(LastEPGChange >= LastObjectChange){
                        MESSAGE(VERBOSE_EPG_UPDATES, "Updating details");

                        if(Event){
                            ChannelItem->setTitle(Event->Title()?Event->Title():Channel->Name());
                            ChannelItem->setLongDescription(Event->Description());
                            ChannelItem->setDescription(Event->ShortText());
                        }
                        else {
                            ChannelItem->setTitle(Channel->Name());
                            ChannelItem->setLongDescription(NULL);
                            ChannelItem->setDescription(NULL);
                        }

                        this->mFactory->saveObject(ChannelItem);
                    }
                    else {
                        MESSAGE(VERBOSE_EPG_UPDATES, "Channel did not change");
                    }
                }
                else {
                    MESSAGE(VERBOSE_EPG_UPDATES, "No EPG data");
                    ChannelItem->setTitle(Channel->Name());
                    ChannelItem->setLongDescription(NULL);
                    ChannelItem->setDescription(NULL);
                }
            }
        }
    }
}
#endif
#ifndef WITHOUT_RECORDS
int cMediaDatabase::loadRecordings(){
    MESSAGE(VERBOSE_RECORDS, "Loading recordings");
    cUPnPClassContainer* Records = (cUPnPClassContainer*)this->getObjectByID(4);
    if(Records){
        bool noResource = false;
        // TODO: Add to setup
        // if an error occured while loading resources, add the channel anyway
        bool addWithoutResources = false;
        cRecording* Recording = NULL;
        for(Recording = Recordings.First(); Recording; Recording = Recordings.Next(Recording)){
            // Iterating the records
            bool inList = false;

            MESSAGE(VERBOSE_RECORDS, "Determine if the channel %s is already listed", Recording->FileName());

            cUPnPClassMovie *MovieItem = NULL;

            MovieItem = (cUPnPClassMovie*)this->getObjectByFastFind(Recording->FileName());

            inList = (MovieItem && Records->getObject(MovieItem->getID())) ? true : false;

            if(!inList){
                noResource = false;
                const cRecordingInfo* RecInfo = Recording->Info();

                MESSAGE(VERBOSE_RECORDS, "Adding movie '%s' File name:%s", RecInfo->Title(), Recording->FileName());

                MovieItem = (cUPnPClassMovie*)this->mFactory->createObject(UPNP_CLASS_MOVIE, RecInfo->Title()?RecInfo->Title():Recording->Name());
                MovieItem->setDescription(RecInfo->ShortText());
                MovieItem->setLongDescription(RecInfo->Description());
                MovieItem->setStorageMedium(UPNP_STORAGE_HDD);
                
                if(RecInfo->Components()){
                    // The first component
                    tComponent *Component = RecInfo->Components()->Component(0);
                    if(Component) MovieItem->setLanguage(Component->language);
                }

                if(cUPnPResources::getInstance()->createFromRecording(MovieItem, Recording)){
                    ERROR("Unable to get resources for this channel");
                    noResource = true;
                }
                if(!noResource || addWithoutResources) {
                    Records->addObject(MovieItem);
                    if(this->mFactory->saveObject(MovieItem) ||
                       this->addFastFind(MovieItem, Recording->FileName())){
                        this->mFactory->deleteObject(MovieItem);
                        return -1;
                    }
                    MESSAGE(VERBOSE_RECORDS, "Successfuly added movie");
                }
                else {
                    // Delete temporarily created object with no resource
                    this->mFactory->deleteObject(MovieItem);
                }
            }
            else {
                MESSAGE(VERBOSE_RECORDS, "Skipping %s, already in Database", Recording->FileName());
            }
        }
    }
    return 0;
}
#endif

void cMediaDatabase::Action(){
    time_t LastEPGUpdate = 0;
    while(this->Running()){

        if(cSchedules::Modified() >= LastEPGUpdate){
            MESSAGE(VERBOSE_EPG_UPDATES, "Schedule changed. Updating...");
            updateChannelEPG();
            LastEPGUpdate = cSchedules::Modified();
        }

        cCondWait::SleepMs(60 * 1000); // sleep a minute
    }
}

int cMediaDatabase::browse(
    OUT cUPnPResultSet** Results,
    IN const char* ID,
    IN bool BrowseMetadata,
    IN const char* Filter,
    IN unsigned int Offset,
    IN unsigned int Count,
    IN const char* SortCriteria
){
    *Results = new cUPnPResultSet;
    (*Results)->mNumberReturned = 0;
    (*Results)->mTotalMatches = 0;
    (*Results)->mResult = NULL;

    MESSAGE(VERBOSE_DIDL, "===== Browsing =====");
    MESSAGE(VERBOSE_DIDL, "ID: %s", ID);
    MESSAGE(VERBOSE_DIDL, "Browse %s", BrowseMetadata?"metadata":"children");
    MESSAGE(VERBOSE_DIDL, "Filter: %s", Filter);
    MESSAGE(VERBOSE_DIDL, "Offset: %d", Offset);
    MESSAGE(VERBOSE_DIDL, "Count: %d", Count);
    MESSAGE(VERBOSE_DIDL, "Sort: %s", SortCriteria);

    cUPnPObjectID ObjectID = atoi(ID);

    cStringList* FilterList = cFilterCriteria::parse(Filter);
    cList<cSortCrit>* SortCriterias = cSortCriteria::parse(SortCriteria);

    if(!SortCriterias){
        return UPNP_CDS_E_INVALID_SORT_CRITERIA;
    }

    cUPnPClassObject* Object = this->getObjectByID(ObjectID);
    if(Object){
        IXML_Document* DIDLDoc = NULL;
        if(ixmlParseBufferEx(UPNP_DIDL_SKELETON, &DIDLDoc)==IXML_SUCCESS){

            IXML_Node* Root = ixmlNode_getFirstChild((IXML_Node*) DIDLDoc);
            switch(BrowseMetadata){
                case true:
                    ixmlNode_appendChild(Root, Object->createDIDLFragment(DIDLDoc, FilterList));
                    delete FilterList;
                    (*Results)->mNumberReturned = 1;
                    (*Results)->mTotalMatches = 1;
                    (*Results)->mResult = ixmlDocumenttoString(DIDLDoc);
                    ixmlDocument_free(DIDLDoc);
                    return UPNP_E_SUCCESS;
                case false:
                    if(Object->isContainer()){
                        cUPnPClassContainer* Container = Object->getContainer();
                        (*Results)->mTotalMatches = Container->getChildCount();
                        cUPnPObjects* Children = Container->getObjectList();

                        if(SortCriterias){
                            for(cSortCrit* SortBy = SortCriterias->First(); SortBy ; SortBy = SortCriterias->Next(SortBy)){
                                MESSAGE(VERBOSE_DIDL, "Sorting by %s %s", SortBy->Property, SortBy->SortDescending?"ascending":"descending");
                                Children->SortBy(SortBy->Property, SortBy->SortDescending);
                            }
                        }

                        cUPnPClassObject* Child = Children->First();
                        if(Count==0) Count = Container->getChildCount();
                        while(Offset-- && (Child = Children->Next(Child))){}
                        for(; Count && Child ; Child = Children->Next(Child), Count--){
                            MESSAGE(VERBOSE_DIDL, "Appending %s to didl", Child->getTitle());
                            ixmlNode_appendChild(Root, Child->createDIDLFragment(DIDLDoc, FilterList));
                            (*Results)->mNumberReturned++;
                        }
                        delete FilterList;
                        delete SortCriterias;
                    }
                    else {
                        (*Results)->mNumberReturned = 0;
                        (*Results)->mTotalMatches = 0;
                    }
                    (*Results)->mResult = ixmlDocumenttoString(DIDLDoc);
                    ixmlDocument_free(DIDLDoc);
                    return UPNP_E_SUCCESS;
            }
        }
        else {
            ERROR("Unable to parse DIDL skeleton");
            return UPNP_CDS_E_CANT_PROCESS_REQUEST;
        }
    }
    else {
        ERROR("No such object: %s", ID);
        return UPNP_CDS_E_NO_SUCH_OBJECT; // No such object;
    }
    return UPNP_SOAP_E_ACTION_FAILED;
}