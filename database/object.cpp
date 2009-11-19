/* 
 * File:   object.cpp
 * Author: savop
 * 
 * Created on 11. September 2009, 20:39
 */

#include <string.h>
#include <stdio.h>
#include <upnp/upnptools.h>
#include <vdr/recording.h>
#include <vector>
#include "database.h"
#include <vdr/tools.h>
#include <upnp/ixml.h>
#include "metadata.h"
#include "object.h"
#include "../common.h"
#include "resources.h"

cUPnPResource::cUPnPResource(){
    this->mBitrate = 0;
    this->mBitsPerSample = 0;
    this->mColorDepth = 0;
    this->mDuration = NULL;
    this->mImportURI = NULL;
    this->mNrAudioChannels = 0;
    this->mObjectID = 0;
    this->mProtocolInfo = NULL;
    this->mResolution = NULL;
    this->mResource = NULL;
    this->mResourceID = 0;
    this->mSampleFrequency = 0;
    this->mSize = 0;
    this->mContentType = NULL;
}

off64_t cUPnPResource::getFileSize() const {
    return (this->mSize) ? this->mSize : (off64_t)-1;
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

static int CompareUPnPObjects(const void *a, const void *b){
  const cUPnPClassObject *la = *(const cUPnPClassObject **)a;
  const cUPnPClassObject *lb = *(const cUPnPClassObject **)b;
  return la->Compare(*lb);
}

cUPnPObjects::cUPnPObjects(){}

cUPnPObjects::~cUPnPObjects(){}

void cUPnPObjects::SortBy(const char* Property, bool Descending){
    int n = Count();
    cUPnPClassObject *a[n];
    cUPnPClassObject *object = (cUPnPClassObject *)objects;
    int i = 0;
    while (object && i < n) {
        object->setSortCriteria(Property, Descending);
        a[i++] = object;
        object = (cUPnPClassObject *)object->Next();
    }
    qsort(a, n, sizeof(cUPnPClassObject *), CompareUPnPObjects);
    objects = lastObject = NULL;
    for (i = 0; i < n; i++) {
        a[i]->Unlink();
        count--;
        Add(a[i]);
    }
}

 /**********************************************\
 *                                              *
 *  UPnP Objects                                *
 *                                              *
 \**********************************************/

 /**********************************************\
 *                                              *
 *  Object                                      *
 *                                              *
 \**********************************************/

cUPnPClassObject::cUPnPClassObject(){
    this->mID = -1;
    this->mLastID = -1;
    this->mResources = new cList<cUPnPResource>;
    this->mResourcesID = new cHash<cUPnPResource>;
    this->mParent = NULL;
    this->mClass = NULL;
    this->mCreator = NULL;
    this->mTitle = NULL;
    this->mWriteStatus = WS_UNKNOWN;
    this->mRestricted = true;
    this->mDIDLFragment = NULL;
    this->mSortCriteria = NULL;
    this->mLastModified = NULL;
}

cUPnPClassObject::~cUPnPClassObject(){
    if(this->mParent) this->mParent->getContainer()->removeObject(this);
    this->mResources->Clear();
    this->mResourcesID->Clear();
    delete this->mResources;
    delete this->mResourcesID;
    free(this->mDIDLFragment);
}

int cUPnPClassObject::Compare(const cListObject& ListObject) const {
    char* Value1 = NULL; char* Value2 = NULL; int ret = 0;
    cUPnPClassObject* Object = (cUPnPClassObject*)&ListObject;
    if(Object->getProperty(this->mSortCriteria, &Value1) &&
       this->getProperty(this->mSortCriteria, &Value2)){
        ret = strcmp(Value1, Value2);
        if(this->mSortDescending) ret *= -1;
    }
    return ret;
}

void cUPnPClassObject::setSortCriteria(const char* Property, bool Descending){
    this->mSortCriteria = Property;
    this->mSortDescending = Descending;
}

void cUPnPClassObject::clearSortCriteria(){
    this->mSortCriteria = NULL;
    this->mSortDescending = false;
}

int cUPnPClassObject::setID(cUPnPObjectID ID){
    MESSAGE(VERBOSE_MODIFICATIONS, "Set ID from %s to %s", *this->getID(),*ID);
    if((int)ID < 0){
        ERROR("Invalid object ID '%s'",*ID);
        return -1;
    }
    this->mLastID = (this->mID==-1) ? ID : this->mID;
    this->mID = ID;
    return 0;
}

int cUPnPClassObject::setParent(cUPnPClassContainer* Parent){
    if(Parent==NULL){
        MESSAGE(VERBOSE_MODIFICATIONS, "Object '%s' elected as root object", *this->getID());
    }
    // unregister from old parent
    if(this->mParent && Parent != this->mParent){
        this->mParent->getContainer()->removeObject(this);
    }
    this->mParent = Parent;
    return 0;
}

int cUPnPClassObject::setClass(const char* Class){
    if(     !strcasecmp(Class, UPNP_CLASS_ALBUM) ||
            !strcasecmp(Class, UPNP_CLASS_AUDIO) ||
            !strcasecmp(Class, UPNP_CLASS_AUDIOBC) ||
            !strcasecmp(Class, UPNP_CLASS_AUDIOBOOK) ||
            !strcasecmp(Class, UPNP_CLASS_CONTAINER) ||
            !strcasecmp(Class, UPNP_CLASS_GENRE) ||
            !strcasecmp(Class, UPNP_CLASS_IMAGE) ||
            !strcasecmp(Class, UPNP_CLASS_ITEM) ||
            !strcasecmp(Class, UPNP_CLASS_MOVIE) ||
            !strcasecmp(Class, UPNP_CLASS_MOVIEGENRE) ||
            !strcasecmp(Class, UPNP_CLASS_MUSICALBUM) ||
            !strcasecmp(Class, UPNP_CLASS_MUSICARTIST) ||
            !strcasecmp(Class, UPNP_CLASS_MUSICGENRE) ||
            !strcasecmp(Class, UPNP_CLASS_MUSICTRACK) ||
            !strcasecmp(Class, UPNP_CLASS_MUSICVIDCLIP) ||
            !strcasecmp(Class, UPNP_CLASS_OBJECT) ||
            !strcasecmp(Class, UPNP_CLASS_PERSON) ||
            !strcasecmp(Class, UPNP_CLASS_PHOTO) ||
            !strcasecmp(Class, UPNP_CLASS_PHOTOALBUM) ||
            !strcasecmp(Class, UPNP_CLASS_PLAYLIST) ||
            !strcasecmp(Class, UPNP_CLASS_PLAYLISTCONT) ||
            !strcasecmp(Class, UPNP_CLASS_STORAGEFOLD) ||
            !strcasecmp(Class, UPNP_CLASS_STORAGESYS) ||
            !strcasecmp(Class, UPNP_CLASS_STORAGEVOL) ||
            !strcasecmp(Class, UPNP_CLASS_TEXT) ||
            !strcasecmp(Class, UPNP_CLASS_VIDEO) ||
            !strcasecmp(Class, UPNP_CLASS_VIDEOBC)
            ){
        this->mClass = strdup0(Class);
        return 0;
    }
    else {
        ERROR("Invalid or unsupported class '%s'", Class);
        return -1;
    }
}

int cUPnPClassObject::setTitle(const char* Title){
    if(Title==NULL){
        ERROR("Title is empty but required");
        return -1;
    }
    this->mTitle = strdup0(Title);
    return 0;
}

int cUPnPClassObject::setCreator(const char* Creator){
    this->mCreator = strdup0(Creator);
    return 0;
}

int cUPnPClassObject::setRestricted(bool Restricted){
    this->mRestricted = Restricted;
    return 0;
}

int cUPnPClassObject::setWriteStatus(int WriteStatus){
    if(     WriteStatus == WS_MIXED ||
            WriteStatus == WS_NOT_WRITABLE ||
            WriteStatus == WS_PROTECTED ||
            WriteStatus == WS_UNKNOWN ||
            WriteStatus == WS_WRITABLE){
        this->mWriteStatus = WriteStatus;
        return 0;
    }
    else {
        ERROR("Invalid write status '%d'", WriteStatus);
        return -1;
    }
}

bool cUPnPClassObject::getProperty(const char* Property, char** Value) const {
    cString Val;
    if(!strcasecmp(Property, SQLITE_COL_OBJECTID) || !strcasecmp(Property, UPNP_PROP_OBJECTID)){
        Val = *this->getID();
    }
    else if(!strcasecmp(Property, SQLITE_COL_PARENTID) || !strcasecmp(Property, UPNP_PROP_PARENTID)){
        Val = *this->getParentID();
    }
    else if(!strcasecmp(Property, SQLITE_COL_CLASS) || !strcasecmp(Property, UPNP_PROP_CLASS)){
        Val = this->getClass();
    }
    else if(!strcasecmp(Property, SQLITE_COL_TITLE) || !strcasecmp(Property, UPNP_PROP_TITLE)){
        Val = this->getTitle();
    }
    else if(!strcasecmp(Property, SQLITE_COL_CREATOR) || !strcasecmp(Property, UPNP_PROP_CREATOR)){
        Val = this->getCreator();
    }
    else if(!strcasecmp(Property, SQLITE_COL_RESTRICTED) || !strcasecmp(Property, UPNP_PROP_RESTRICTED)){
        Val = this->isRestricted()?"1":"0";
    }
    else if(!strcasecmp(Property, SQLITE_COL_WRITESTATUS) || !strcasecmp(Property, UPNP_PROP_WRITESTATUS)){
        Val = itoa(this->getWriteStatus());
    }
    else {
        ERROR("Invalid property '%s'", Property);
        return false;
    }
    *Value = strdup0(*Val);
    return true;
}

cStringList* cUPnPClassObject::getPropertyList(){
    cStringList* Properties = new cStringList;
    Properties->Append(strdup(UPNP_PROP_CREATOR));
    Properties->Append(strdup(UPNP_PROP_WRITESTATUS));
    return Properties;
}

bool cUPnPClassObject::setProperty(const char* Property, const char* Value){
    int ret;
    if(!strcasecmp(Property, SQLITE_COL_OBJECTID) || !strcasecmp(Property, UPNP_PROP_OBJECTID)){
        ERROR("Not allowed to set object ID by hand");
        return false;
    }
    else if(!strcasecmp(Property, SQLITE_COL_PARENTID) || !strcasecmp(Property, UPNP_PROP_PARENTID)){
        ERROR("Not allowed to set parent ID by hand");
        return false;
    }
    else if(!strcasecmp(Property, SQLITE_COL_CLASS) || !strcasecmp(Property, UPNP_PROP_CLASS)){
        ERROR("Not allowed to set class by hand");
        return false;
    }
    else if(!strcasecmp(Property, SQLITE_COL_TITLE) || !strcasecmp(Property, UPNP_PROP_TITLE)){
        ret = this->setTitle(Value);
    }
    else if(!strcasecmp(Property, SQLITE_COL_CREATOR) || !strcasecmp(Property, UPNP_PROP_CREATOR)){
        ret = this->setCreator(Value);
    }
    else if(!strcasecmp(Property, SQLITE_COL_RESTRICTED) || !strcasecmp(Property, UPNP_PROP_RESTRICTED)){
        ret = this->setRestricted(atoi(Value)==1?true:false);
    }
    else if(!strcasecmp(Property, SQLITE_COL_WRITESTATUS) || !strcasecmp(Property, UPNP_PROP_WRITESTATUS)){
        ret= this->setWriteStatus(atoi(Value));
    }
    else {
        ERROR("Invalid property '%s'", Property);
        return false;
    }
    return ret<0?false:true;
}

int cUPnPClassObject::addResource(cUPnPResource* Resource){
    MESSAGE(VERBOSE_MODIFICATIONS, "Adding resource #%d", Resource->getID());
    if(!Resource){
        ERROR("No resource");
        return -1;
    }
    this->mResources->Add(Resource);
    this->mResourcesID->Add(Resource, Resource->getID());
    return 0;
}

int cUPnPClassObject::removeResource(cUPnPResource* Resource){
    if(!Resource){
        ERROR("No resource");
        return -1;
    }
    this->mResourcesID->Del(Resource, Resource->getID());
    this->mResources->Del(Resource);
    return 0;
}

 /**********************************************\
 *                                              *
 *  Item                                        *
 *                                              *
 \**********************************************/

cUPnPClassItem::cUPnPClassItem(){
    this->setClass(UPNP_CLASS_ITEM);
    this->mReference = NULL;
}

int cUPnPClassItem::setReference(cUPnPClassItem* Reference){
    this->mReference = Reference;
    return 0;
}

cStringList* cUPnPClassItem::getPropertyList(){
    cStringList* Properties = cUPnPClassObject::getPropertyList();
    Properties->Append(strdup(UPNP_PROP_REFERENCEID));
    return Properties;
}

bool cUPnPClassItem::getProperty(const char* Property, char** Value) const {

    if(!strcasecmp(Property, SQLITE_COL_REFERENCEID) || !strcasecmp(Property, UPNP_PROP_REFERENCEID)){
        *Value = strdup0(*this->getReferenceID());
    }
    else return cUPnPClassObject::getProperty(Property, Value);
    return true;
}

bool cUPnPClassItem::setProperty(const char* Property, const char* Value){
    return cUPnPClassObject::setProperty(Property, Value);
}

IXML_Node* cUPnPClassItem::createDIDLFragment(IXML_Document* Document, cStringList* Filter){
    this->mDIDLFragment = Document;

    MESSAGE(VERBOSE_DIDL, "==(%s)= %s =====", *this->getID(), this->getTitle());
    MESSAGE(VERBOSE_DIDL, "ParentID: %s", *this->getParentID());
    MESSAGE(VERBOSE_DIDL, "Restricted: %s", this->isRestricted()?"1":"0");
    MESSAGE(VERBOSE_DIDL, "Class: %s", this->getClass());

    IXML_Node* Didl = ixmlNode_getFirstChild((IXML_Node*) this->mDIDLFragment);

    IXML_Element* eItem = ixmlDocument_createElement(this->mDIDLFragment, "item");
    ixmlElement_setAttribute(eItem, att(UPNP_PROP_OBJECTID), *this->getID());
    ixmlElement_setAttribute(eItem, att(UPNP_PROP_PARENTID), *this->getParentID());
    ixmlElement_setAttribute(eItem, att(UPNP_PROP_RESTRICTED), this->isRestricted()?"1":"0");

    ixmlNode_appendChild(Didl, (IXML_Node*) eItem);

    IXML_Element* eTitle = ixmlDocument_createElement(this->mDIDLFragment, UPNP_PROP_TITLE);
    IXML_Node* Title = ixmlDocument_createTextNode(this->mDIDLFragment, this->getTitle());

    IXML_Element* eClass = ixmlDocument_createElement(this->mDIDLFragment, UPNP_PROP_CLASS);
    IXML_Node* Class = ixmlDocument_createTextNode(this->mDIDLFragment, this->getClass());
    
    ixmlNode_appendChild((IXML_Node*) eTitle, Title);
    ixmlNode_appendChild((IXML_Node*) eClass, Class);
    ixmlNode_appendChild((IXML_Node*) eItem, (IXML_Node*) eTitle);
    ixmlNode_appendChild((IXML_Node*) eItem, (IXML_Node*) eClass);

//    if(Filter==NULL || Filter->Find(UPNP_PROP_CREATOR)) ixmlAddProperty(this->mDIDLFragment, eItem, UPNP_PROP_CREATOR, this->getCreator());
//    if(Filter==NULL || Filter->Find(UPNP_PROP_WRITESTATUS)) ixmlAddProperty(this->mDIDLFragment, eItem, UPNP_PROP_WRITESTATUS, itoa(this->getWriteStatus()));
//    if(Filter==NULL || Filter->Find(UPNP_PROP_REFERENCEID)) ixmlAddProperty(this->mDIDLFragment, eItem, UPNP_PROP_REFERENCEID, *this->getReferenceID());

    for(cUPnPResource* Resource = this->getResources()->First(); Resource; Resource = this->getResources()->Next(Resource)){
        MESSAGE(VERBOSE_DIDL, "Resource: %s", Resource->getResource());
        MESSAGE(VERBOSE_DIDL, "Protocolinfo: %s", Resource->getProtocolInfo());

        cString URLBase = cString::sprintf("http://%s:%d", UpnpGetServerIpAddress(), UpnpGetServerPort());
        cString ResourceURL = cString::sprintf("%s%s/get?resId=%d", *URLBase, UPNP_DIR_SHARES, Resource->getID());

        MESSAGE(VERBOSE_DIDL, "Resource-URI: %s", *ResourceURL);

        IXML_Element* eRes = ixmlDocument_createElement(this->mDIDLFragment, UPNP_PROP_RESOURCE);
        IXML_Node*    Res  = ixmlDocument_createTextNode(this->mDIDLFragment, *ResourceURL);
        ixmlNode_appendChild((IXML_Node*) eRes, Res);

        if(Resource->getBitrate()) ixmlElement_setAttribute(eRes, att(UPNP_PROP_BITRATE), itoa(Resource->getBitrate()));
        if(Resource->getBitsPerSample()) ixmlElement_setAttribute(eRes, att(UPNP_PROP_BITSPERSAMPLE), itoa(Resource->getBitsPerSample()));
        if(Resource->getColorDepth()) ixmlElement_setAttribute(eRes, att(UPNP_PROP_COLORDEPTH), itoa(Resource->getColorDepth()));
        if(Resource->getDuration()) ixmlElement_setAttribute(eRes, att(UPNP_PROP_DURATION), Resource->getDuration());
        if(Resource->getProtocolInfo()) ixmlElement_setAttribute(eRes, att(UPNP_PROP_PROTOCOLINFO), Resource->getProtocolInfo());

        ixmlNode_appendChild((IXML_Node*) eItem, (IXML_Node*) eRes);
    }

    return (IXML_Node*)eItem;
}

 /**********************************************\
 *                                              *
 *  Container                                   *
 *                                              *
 \**********************************************/

cUPnPClassContainer::cUPnPClassContainer(){
    this->setClass(UPNP_CLASS_CONTAINER);
    this->mChildren   = new cUPnPObjects;
    this->mChildrenID = new cHash<cUPnPClassObject>;
    this->mContainerType = NULL;
    this->mUpdateID = 0;
    this->mSearchable = false;
}

cUPnPClassContainer::~cUPnPClassContainer(){
    delete this->mChildren;
    delete this->mChildrenID;
}

IXML_Node* cUPnPClassContainer::createDIDLFragment(IXML_Document* Document, cStringList* Filter){
    this->mDIDLFragment = Document;

    MESSAGE(VERBOSE_DIDL, "===(%s)= %s =====", *this->getID(), this->getTitle());
    MESSAGE(VERBOSE_DIDL, "ParentID: %s", *this->getParentID());
    MESSAGE(VERBOSE_DIDL, "Restricted: %s", this->isRestricted()?"1":"0");
    MESSAGE(VERBOSE_DIDL, "Class: %s", this->getClass());

    IXML_Node* Didl = ixmlNode_getFirstChild((IXML_Node*) this->mDIDLFragment);
    IXML_Element* eItem = ixmlDocument_createElement(this->mDIDLFragment, "container");
    ixmlElement_setAttribute(eItem, att(UPNP_PROP_OBJECTID), *this->getID());
    ixmlElement_setAttribute(eItem, att(UPNP_PROP_PARENTID), *this->getParentID());
    ixmlElement_setAttribute(eItem, att(UPNP_PROP_RESTRICTED), this->isRestricted()?"1":"0");
    ixmlNode_appendChild(Didl, (IXML_Node*) eItem);

    IXML_Element* eTitle = ixmlDocument_createElement(this->mDIDLFragment, UPNP_PROP_TITLE);
    IXML_Node* Title = ixmlDocument_createTextNode(this->mDIDLFragment, this->getTitle());

    IXML_Element* eClass = ixmlDocument_createElement(this->mDIDLFragment, UPNP_PROP_CLASS);
    IXML_Node* Class = ixmlDocument_createTextNode(this->mDIDLFragment, this->getClass());

    ixmlNode_appendChild((IXML_Node*) eTitle, Title);
    ixmlNode_appendChild((IXML_Node*) eClass, Class);
    ixmlNode_appendChild((IXML_Node*) eItem, (IXML_Node*) eTitle);
    ixmlNode_appendChild((IXML_Node*) eItem, (IXML_Node*) eClass);

    return (IXML_Node*)eItem;
}

int cUPnPClassContainer::setUpdateID(unsigned int UID){
    this->mUpdateID = UID;
    return 0;
}

cStringList* cUPnPClassContainer::getPropertyList(){
    cStringList* Properties = cUPnPClassObject::getPropertyList();
    Properties->Append(strdup(UPNP_PROP_DLNA_CONTAINERTYPE));
    Properties->Append(strdup(UPNP_PROP_SEARCHABLE));
    return Properties;
}

bool cUPnPClassContainer::setProperty(const char* Property, const char* Value){
    int ret;
    if(!strcasecmp(Property, SQLITE_COL_DLNA_CONTAINERTYPE) || !strcasecmp(Property, UPNP_PROP_DLNA_CONTAINERTYPE)){
        ret = this->setContainerType(Value);
    }
    else if(!strcasecmp(Property, SQLITE_COL_SEARCHABLE) || !strcasecmp(Property, UPNP_PROP_SEARCHABLE)){
        ret = this->setSearchable(Value);
    }
    else if(!strcasecmp(Property, SQLITE_COL_CONTAINER_UID)){
        ret = this->setUpdateID((unsigned int)atoi(Value));
    }
    else return cUPnPClassObject::setProperty(Property, Value);
    return ret<0?false:true;
}

bool cUPnPClassContainer::getProperty(const char* Property, char** Value) const {
    cString Val;
    if(!strcasecmp(Property, SQLITE_COL_DLNA_CONTAINERTYPE) || !strcasecmp(Property, UPNP_PROP_DLNA_CONTAINERTYPE)){
        Val = this->getContainerType();
    }
    else if(!strcasecmp(Property, SQLITE_COL_SEARCHABLE) || !strcasecmp(Property, UPNP_PROP_SEARCHABLE)){
        Val = this->isSearchable()?"1":"0";
    }
    else if(!strcasecmp(Property, SQLITE_COL_CONTAINER_UID)){
        Val = cString::sprintf("%d", this->getUpdateID());
    }
    else return cUPnPClassObject::getProperty(Property, Value);
    *Value = strdup0(*Val);
    return true;
}

void cUPnPClassContainer::addObject(cUPnPClassObject* Object){
    MESSAGE(VERBOSE_MODIFICATIONS, "Adding object (ID:%s) to container (ID:%s)", *Object->getID(), *this->getID());
    Object->setParent(this);
    this->mChildren->Add(Object);
    this->mChildrenID->Add(Object, (unsigned int)Object->getID());
}

void cUPnPClassContainer::removeObject(cUPnPClassObject* Object){
    this->mChildrenID->Del(Object, (unsigned int)Object->getID());
    this->mChildren->Del(Object, false);
    Object->mParent = NULL;
    MESSAGE(VERBOSE_MODIFICATIONS, "Removed object (ID:%s) from container (ID:%s)", *Object->getID(), *this->getID());
}

cUPnPClassObject* cUPnPClassContainer::getObject(cUPnPObjectID ID) const {
    MESSAGE(VERBOSE_METADATA, "Getting object (ID:%s)", *ID);
    if((int)ID < 0){
        ERROR("Invalid object ID");
        return NULL;
    }
    return this->mChildrenID->Get((unsigned int)ID);
}

int cUPnPClassContainer::setContainerType(const char* Type){
    if(Type==NULL){
        this->mContainerType = Type;
    }
    else if(!strcasecmp(Type, DLNA_CONTAINER_TUNER)){
        this->mContainerType = Type;
    }
    else {
        ERROR("Invalid container type '%s'",Type);
        return -1;
    }
    return 0;
}

int cUPnPClassContainer::addSearchClass(cClass SearchClass){
    this->mSearchClasses.push_back(SearchClass);
    return 0;
}

int cUPnPClassContainer::delSearchClass(cClass SearchClass){
    tClassVector::iterator it = this->mSearchClasses.begin();
    cClass Class;
    for(unsigned int i=0; i<this->mSearchClasses.size(); i++){
        Class = this->mSearchClasses[i];
        if(Class == SearchClass){
            this->mSearchClasses.erase(it+i);
            return 0;
        }
    }
    return -1;
}

int cUPnPClassContainer::addCreateClass(cClass CreateClass){
    this->mCreateClasses.push_back(CreateClass);
    return 0;
}

int cUPnPClassContainer::delCreateClass(cClass CreateClass){
    tClassVector::iterator it = this->mCreateClasses.begin();
    cClass Class;
    for(unsigned int i=0; i<this->mCreateClasses.size(); i++){
        Class = this->mCreateClasses[i];
        if(Class == CreateClass){
            this->mCreateClasses.erase(it+i);
            return 0;
        }
    }
    return -1;
}

int cUPnPClassContainer::setSearchClasses(std::vector<cClass> SearchClasses){
    this->mSearchClasses = SearchClasses;
    return 0;
}

int cUPnPClassContainer::setCreateClasses(std::vector<cClass> CreateClasses){
    this->mCreateClasses = CreateClasses;
    return 0;
}

int cUPnPClassContainer::setSearchable(bool Searchable){
    this->mSearchable = Searchable;
    return 0;
}

bool cUPnPClassContainer::isUpdated(){
    static unsigned int lastUpdateID = this->getUpdateID();
    if(lastUpdateID != this->getUpdateID()){
        lastUpdateID = this->getUpdateID();
        return true;
    }
    else return false;
}

 /**********************************************\
 *                                              *
 *  Video item                                  *
 *                                              *
 \**********************************************/

cUPnPClassVideoItem::cUPnPClassVideoItem(){
    this->setClass(UPNP_CLASS_VIDEO);
    this->mGenre = NULL;
    this->mLongDescription = NULL;
    this->mProducers = NULL;
    this->mRating = NULL;
    this->mActors = NULL;
    this->mDirectors = NULL;
    this->mDescription = NULL;
    this->mPublishers = NULL;
    this->mLanguage = NULL;
    this->mRelations = NULL;
}

cUPnPClassVideoItem::~cUPnPClassVideoItem(){
}

//cString cUPnPClassVideoItem::createDIDLFragment(cStringList* Filter){
//    return NULL;
//}

cStringList* cUPnPClassVideoItem::getPropertyList(){
    cStringList* Properties = cUPnPClassItem::getPropertyList();
    Properties->Append(strdup(UPNP_PROP_LONGDESCRIPTION));
    Properties->Append(strdup(UPNP_PROP_PRODUCER));
    Properties->Append(strdup(UPNP_PROP_GENRE));
    Properties->Append(strdup(UPNP_PROP_RATING));
    Properties->Append(strdup(UPNP_PROP_ACTOR));
    Properties->Append(strdup(UPNP_PROP_DIRECTOR));
    Properties->Append(strdup(UPNP_PROP_DESCRIPTION));
    Properties->Append(strdup(UPNP_PROP_PUBLISHER));
    Properties->Append(strdup(UPNP_PROP_LANGUAGE));
    Properties->Append(strdup(UPNP_PROP_RELATION));
    return Properties;
}

bool cUPnPClassVideoItem::getProperty(const char* Property, char** Value) const {
    cString Val;
    if(!strcasecmp(Property,SQLITE_COL_GENRE) || !strcasecmp(Property,UPNP_PROP_GENRE)){
        Val = this->getGenre();
    }
    else if(!strcasecmp(Property,SQLITE_COL_LONGDESCRIPTION) || !strcasecmp(Property,UPNP_PROP_LONGDESCRIPTION)){
        Val = this->getLongDescription();
    }
    else if(!strcasecmp(Property,SQLITE_COL_PRODUCER) || !strcasecmp(Property,UPNP_PROP_PRODUCER)){
        Val = this->getProducers();
    }
    else if(!strcasecmp(Property,SQLITE_COL_RATING) || !strcasecmp(Property,UPNP_PROP_RATING)){
        Val = this->getRating();
    }
    else if(!strcasecmp(Property,SQLITE_COL_ACTOR) || !strcasecmp(Property,UPNP_PROP_ACTOR)){
        Val = this->getActors();
    }
    else if(!strcasecmp(Property,SQLITE_COL_DIRECTOR) || !strcasecmp(Property,UPNP_PROP_DIRECTOR)){
        Val = this->getDirectors();
    }
    else if(!strcasecmp(Property,SQLITE_COL_DESCRIPTION) || !strcasecmp(Property,UPNP_PROP_DESCRIPTION)){
        Val = this->getDescription();
    }
    else if(!strcasecmp(Property,SQLITE_COL_PUBLISHER) || !strcasecmp(Property,UPNP_PROP_PUBLISHER)){
        Val = this->getPublishers();
    }
    else if(!strcasecmp(Property,SQLITE_COL_LANGUAGE) || !strcasecmp(Property,UPNP_PROP_LANGUAGE)){
        Val = this->getLanguage();
    }
    else if(!strcasecmp(Property,SQLITE_COL_RELATION) || !strcasecmp(Property,UPNP_PROP_RELATION)){
        Val = this->getRelations();
    }
    else return cUPnPClassItem::getProperty(Property, Value);
    *Value = strdup0(*Val);
    return true;
}

bool cUPnPClassVideoItem::setProperty(const char* Property, const char* Value){
    bool ret;
    if(!strcasecmp(Property,SQLITE_COL_GENRE) || !strcasecmp(Property,UPNP_PROP_GENRE)){
        ret = this->setGenre(Value);
    }
    else if(!strcasecmp(Property,SQLITE_COL_LONGDESCRIPTION) || !strcasecmp(Property,UPNP_PROP_LONGDESCRIPTION)){
        ret = this->setLongDescription(Value);
    }
    else if(!strcasecmp(Property,SQLITE_COL_PRODUCER) || !strcasecmp(Property,UPNP_PROP_PRODUCER)){
        ret = this->setProducers(Value);
    }
    else if(!strcasecmp(Property,SQLITE_COL_RATING) || !strcasecmp(Property,UPNP_PROP_RATING)){
        ret = this->setRating(Value);
    }
    else if(!strcasecmp(Property,SQLITE_COL_ACTOR) || !strcasecmp(Property,UPNP_PROP_ACTOR)){
        ret = this->setActors(Value);
    }
    else if(!strcasecmp(Property,SQLITE_COL_DIRECTOR) || !strcasecmp(Property,UPNP_PROP_DIRECTOR)){
        ret = this->setDirectors(Value);
    }
    else if(!strcasecmp(Property,SQLITE_COL_DESCRIPTION) || !strcasecmp(Property,UPNP_PROP_DESCRIPTION)){
        ret = this->setDescription(Value);
    }
    else if(!strcasecmp(Property,SQLITE_COL_PUBLISHER) || !strcasecmp(Property,UPNP_PROP_PUBLISHER)){
        ret = this->setPublishers(Value);
    }
    else if(!strcasecmp(Property,SQLITE_COL_LANGUAGE) || !strcasecmp(Property,UPNP_PROP_LANGUAGE)){
        ret = this->setLanguage(Value);
    }
    else if(!strcasecmp(Property,SQLITE_COL_RELATION) || !strcasecmp(Property,UPNP_PROP_RELATION)){
        ret = this->setRelations(Value);
    }
    else return cUPnPClassItem::setProperty(Property, Value);
    return ret<0?false:true;
}

int cUPnPClassVideoItem::setActors(const char* Actors){
    this->mActors = Actors;
    return 0;
}

int cUPnPClassVideoItem::setGenre(const char* Genre){
    this->mGenre = Genre;
    return 0;
}

int cUPnPClassVideoItem::setDescription(const char* Description){
    this->mDescription = Description;
    return 0;
}

int cUPnPClassVideoItem::setLongDescription(const char* LongDescription){
    this->mLongDescription = LongDescription;
    return 0;
}

int cUPnPClassVideoItem::setProducers(const char* Producers){
    this->mProducers = Producers;
    return 0;
}

int cUPnPClassVideoItem::setRating(const char* Rating){
    this->mRating = Rating;
    return 0;
}

int cUPnPClassVideoItem::setDirectors(const char* Directors){
    this->mDirectors = Directors;
    return 0;
}

int cUPnPClassVideoItem::setPublishers(const char* Publishers){
    this->mPublishers = Publishers;
    return 0;
}

int cUPnPClassVideoItem::setLanguage(const char* Language){
    this->mLanguage = Language;
    return 0;
}

int cUPnPClassVideoItem::setRelations(const char* Relations){
    this->mRelations = Relations;
    return 0;
}

 /**********************************************\
 *                                              *
 *  Video Broadcast item                        *
 *                                              *
 \**********************************************/

cUPnPClassVideoBroadcast::cUPnPClassVideoBroadcast(){
    this->setClass(UPNP_CLASS_VIDEOBC);
    this->mIcon = NULL;
    this->mRegion = NULL;
    this->mChannelNr = 0;
}

cUPnPClassVideoBroadcast::~cUPnPClassVideoBroadcast(){
}

//cString cUPnPClassVideoBroadcast::createDIDLFragment(cStringList* Filter){
//    return NULL;
//}

cStringList* cUPnPClassVideoBroadcast::getPropertyList(){
    cStringList* Properties = cUPnPClassVideoItem::getPropertyList();
    Properties->Append(strdup(UPNP_PROP_CHANNELNAME));
    Properties->Append(strdup(UPNP_PROP_CHANNELNR));
    Properties->Append(strdup(UPNP_PROP_ICON));
    Properties->Append(strdup(UPNP_PROP_REGION));
    return Properties;
}

bool cUPnPClassVideoBroadcast::setProperty(const char* Property, const char* Value){
    bool ret;
    if(!strcasecmp(Property, SQLITE_COL_CHANNELNAME) || !strcasecmp(Property, UPNP_PROP_CHANNELNAME)){
        ret = this->setChannelName(Value);
    }
    else if(!strcasecmp(Property, SQLITE_COL_CHANNELNR) || !strcasecmp(Property, UPNP_PROP_CHANNELNR)){
        ret = this->setChannelNr(atoi(Value));
    }
    else if(!strcasecmp(Property, SQLITE_COL_ICON) || !strcasecmp(Property, UPNP_PROP_ICON)){
        ret = this->setIcon(Value);
    }
    else if(!strcasecmp(Property, SQLITE_COL_REGION) || !strcasecmp(Property, UPNP_PROP_REGION)){
        ret = this->setRegion(Value);
    }
    else return cUPnPClassVideoItem::setProperty(Property, Value);
    return ret<0?false:true;
}

bool cUPnPClassVideoBroadcast::getProperty(const char* Property, char** Value) const {
    cString Val;
    if(!strcasecmp(Property, SQLITE_COL_CHANNELNAME) || !strcasecmp(Property, UPNP_PROP_CHANNELNAME)){
        Val = this->getChannelName();
    }
    else if(!strcasecmp(Property, SQLITE_COL_CHANNELNR) || !strcasecmp(Property, UPNP_PROP_CHANNELNR)){
        Val = itoa(this->getChannelNr());
    }
    else if(!strcasecmp(Property, SQLITE_COL_ICON) || !strcasecmp(Property, UPNP_PROP_ICON)){
        Val = this->getIcon();
    }
    else if(!strcasecmp(Property, SQLITE_COL_REGION) || !strcasecmp(Property, UPNP_PROP_REGION)){
        Val = this->getRegion();
    }
    else return cUPnPClassVideoItem::getProperty(Property, Value);
    *Value = strdup0(*Val);
    return true;
}

int cUPnPClassVideoBroadcast::setChannelName(const char* ChannelName){
    this->mChannelName = ChannelName;
    return 0;
}

int cUPnPClassVideoBroadcast::setChannelNr(int ChannelNr){
    this->mChannelNr = ChannelNr;
    return 0;
}

int cUPnPClassVideoBroadcast::setIcon(const char* IconURI){
    this->mIcon = IconURI;
    return 0;
}

int cUPnPClassVideoBroadcast::setRegion(const char* Region){
    this->mRegion = Region;
    return 0;
}

/**********************************************\
*                                              *
*  Movie item                                  *
*                                              *
\**********************************************/

cUPnPClassMovie::cUPnPClassMovie(){
     this->mDVDRegionCode = 2; // Europe
     this->mStorageMedium = UPNP_STORAGE_UNKNOWN;
}

cUPnPClassMovie::~cUPnPClassMovie(){}

//cString cUPnPClassMovie::createDIDLFragment(cStringList* Filter){
//    return NULL;
//}

cStringList* cUPnPClassMovie::getPropertyList(){
    cStringList* Properties = cUPnPClassVideoItem::getPropertyList();
    Properties->Append(strdup(UPNP_PROP_DVDREGIONCODE));
    Properties->Append(strdup(UPNP_PROP_STORAGEMEDIUM));
    return Properties;
}

bool cUPnPClassMovie::setProperty(const char* Property, const char* Value){
    bool ret;
    if(!strcasecmp(Property, SQLITE_COL_DVDREGIONCODE) || !strcasecmp(Property, UPNP_PROP_DVDREGIONCODE)){
        ret = this->setDVDRegionCode(atoi(Value));
    }
    else if(!strcasecmp(Property, SQLITE_COL_STORAGEMEDIUM) || !strcasecmp(Property, UPNP_PROP_STORAGEMEDIUM)){
        ret = this->setStorageMedium(Value);
    }
    else return cUPnPClassVideoItem::setProperty(Property, Value);
    return ret<0?false:true;
}

bool cUPnPClassMovie::getProperty(const char* Property, char** Value) const {
    cString Val;
    if(!strcasecmp(Property, SQLITE_COL_DVDREGIONCODE) || !strcasecmp(Property, UPNP_PROP_DVDREGIONCODE)){
        Val = itoa(this->getDVDRegionCode());
    }
    else if(!strcasecmp(Property, SQLITE_COL_STORAGEMEDIUM) || !strcasecmp(Property, UPNP_PROP_STORAGEMEDIUM)){
        Val = this->getStorageMedium();
    }
    else return cUPnPClassVideoItem::getProperty(Property, Value);
    *Value = strdup0(*Val);
    return true;
}

int cUPnPClassMovie::setDVDRegionCode(int RegionCode){
//    http://en.wikipedia.org/wiki/DVD_region_code
//    0 	Informal term meaning "worldwide". Region 0 is not an official setting; discs that bear the region 0 symbol either have no flag set or have region 1–6 flags set.
//    1 	Canada, United States; U.S. territories; Bermuda
//    2 	Western Europe; incl. United Kingdom, Ireland, and Central Europe; Eastern Europe, Western Asia; including Iran, Israel, Egypt; Japan, South Africa, Swaziland, Lesotho; French overseas territories
//    3 	Southeast Asia; South Korea; Taiwan; Hong Kong; Macau
//    4 	Mexico, Central and South America; Caribbean; Australia; New Zealand; Oceania;
//    5 	Ukraine, Belarus, Russia, Continent of Africa, excluding Egypt, South Africa, Swaziland, and Lesotho; Central and South Asia, Mongolia, North Korea.
//    6 	People's Republic of China
//    7 	Reserved for future use (found in use on protected screener copies of MPAA-related DVDs and "media copies" of pre-releases in Asia)
//    8 	International venues such as aircraft, cruise ships, etc.[1]
//    ALL (9)	Region ALL discs have all 8 flags set, allowing the disc to be played in any locale on any player.
    if(0 <= RegionCode && RegionCode <= 9){
        this->mDVDRegionCode = RegionCode;
        return 0;
    }
    else {
        ERROR("Invalid DVD region code: %d", RegionCode);
        return -1;
    }
}

int cUPnPClassMovie::setStorageMedium(const char* StorageMedium){
    if(!StorageMedium) this->mStorageMedium = UPNP_STORAGE_UNKNOWN;
    else if(
            strcasecmp(StorageMedium,UPNP_STORAGE_CD_DA) &&
            strcasecmp(StorageMedium,UPNP_STORAGE_CD_R) &&
            strcasecmp(StorageMedium,UPNP_STORAGE_CD_ROM) &&
            strcasecmp(StorageMedium,UPNP_STORAGE_CD_RW) &&
            strcasecmp(StorageMedium,UPNP_STORAGE_DAT) &&
            strcasecmp(StorageMedium,UPNP_STORAGE_DV) &&
            strcasecmp(StorageMedium,UPNP_STORAGE_DVD_AUDIO) &&
            strcasecmp(StorageMedium,UPNP_STORAGE_DVD_RAM) &&
            strcasecmp(StorageMedium,UPNP_STORAGE_DVD_ROM) &&
            strcasecmp(StorageMedium,UPNP_STORAGE_DVD_RW_MINUS) &&
            strcasecmp(StorageMedium,UPNP_STORAGE_DVD_RW_PLUS) &&
            strcasecmp(StorageMedium,UPNP_STORAGE_DVD_R_MINUS) &&
            strcasecmp(StorageMedium,UPNP_STORAGE_DVD_VIDEO) &&
            strcasecmp(StorageMedium,UPNP_STORAGE_D_VHS) &&
            strcasecmp(StorageMedium,UPNP_STORAGE_HDD) &&
            strcasecmp(StorageMedium,UPNP_STORAGE_HI8) &&
            strcasecmp(StorageMedium,UPNP_STORAGE_LD) &&
            strcasecmp(StorageMedium,UPNP_STORAGE_MD_AUDIO) &&
            strcasecmp(StorageMedium,UPNP_STORAGE_MD_PICTURE) &&
            strcasecmp(StorageMedium,UPNP_STORAGE_MICRO_MV) &&
            strcasecmp(StorageMedium,UPNP_STORAGE_MINI_DV) &&
            strcasecmp(StorageMedium,UPNP_STORAGE_NETWORK) &&
            strcasecmp(StorageMedium,UPNP_STORAGE_SACD) &&
            strcasecmp(StorageMedium,UPNP_STORAGE_S_VHS) &&
            strcasecmp(StorageMedium,UPNP_STORAGE_UNKNOWN) &&
            strcasecmp(StorageMedium,UPNP_STORAGE_VHS) &&
            strcasecmp(StorageMedium,UPNP_STORAGE_VHSC) &&
            strcasecmp(StorageMedium,UPNP_STORAGE_VIDEO8) &&
            strcasecmp(StorageMedium,UPNP_STORAGE_VIDEO_CD) &&
            strcasecmp(StorageMedium,UPNP_STORAGE_W_VHS)
    ){
        ERROR("Invalid storage type: %s", StorageMedium);
        return -1;
    }
    else {
        this->mStorageMedium = StorageMedium;
    }
    return 0;
}

/**********************************************\
*                                              *
*  Mediator factory                            *
*                                              *
\**********************************************/

cUPnPObjectFactory* cUPnPObjectFactory::mInstance = NULL;

cUPnPObjectFactory* cUPnPObjectFactory::getInstance(){
    if(!cUPnPObjectFactory::mInstance)
        cUPnPObjectFactory::mInstance = new cUPnPObjectFactory();

    if(cUPnPObjectFactory::mInstance) return cUPnPObjectFactory::mInstance;
    else return NULL;
}

cUPnPObjectFactory::cUPnPObjectFactory(){
    this->mDatabase = cSQLiteDatabase::getInstance();
}

void cUPnPObjectFactory::registerMediator(const char* UPnPClass, cMediatorInterface* Mediator){
    if(UPnPClass == NULL){
        ERROR("Class is undefined");
        return;
    }
    if(Mediator == NULL){
        ERROR("Mediator is undefined");
        return;
    }
    MESSAGE(VERBOSE_SDK, "Registering mediator for class '%s'", UPnPClass);
    this->mMediators[UPnPClass] = Mediator;
    MESSAGE(VERBOSE_SDK, "Now %d mediators registered", this->mMediators.size());
    return;
}

void cUPnPObjectFactory::unregisterMediator(const char* UPnPClass, bool freeMediator){
    if(UPnPClass == NULL){
        ERROR("Class is undefined");
        return;
    }
    tMediatorMap::iterator MediatorIterator = this->mMediators.find(UPnPClass);
    if(MediatorIterator==this->mMediators.end()){
        ERROR("No such mediator found for class '%s'", UPnPClass);
        return;
    }
    MESSAGE(VERBOSE_SDK, "Unregistering mediator for class '%s'", UPnPClass);
    this->mMediators.erase(MediatorIterator);
    if(freeMediator) delete MediatorIterator->second;
    MESSAGE(VERBOSE_SDK, "Now %d mediators registered", this->mMediators.size());
    return;
}

cMediatorInterface* cUPnPObjectFactory::findMediatorByID(cUPnPObjectID ID){
    cString Format = "SELECT %s FROM %s WHERE %s=%Q";
    cString Column = NULL, Value = NULL, Class = NULL;
    cRows* Rows; cRow* Row;
    if(this->mDatabase->execStatement(Format, SQLITE_COL_CLASS, SQLITE_TABLE_OBJECTS, SQLITE_COL_OBJECTID, *ID)){
        ERROR("Error while executing statement");
        return NULL;
    }
    Rows = this->mDatabase->getResultRows();
    if(!Rows->fetchRow(&Row)){
        ERROR("No such object with ID '%s'",*ID);
        return NULL;
    }
    while(Row->fetchColumn(&Column, &Value)){
        if(!strcasecmp(Column, SQLITE_COL_CLASS)){
            Class = strdup0(*Value);
        }
    }
    return this->findMediatorByClass(Class);
}

cMediatorInterface* cUPnPObjectFactory::findMediatorByClass(const char* Class){
    if(!Class){ ERROR("No class specified"); return NULL; }
    MESSAGE(VERBOSE_SQL, "Searching for mediator '%s' in %d mediators", Class, this->mMediators.size());
    tMediatorMap::iterator MediatorIterator = this->mMediators.find(Class);
    if(MediatorIterator==this->mMediators.end()){
        ERROR("No matching mediator for class '%s'",Class);
        return NULL;
    }
    else {
        return MediatorIterator->second;
    }
}

cUPnPClassObject* cUPnPObjectFactory::getObject(cUPnPObjectID ID){
    cMediatorInterface* Mediator = this->findMediatorByID(ID);
    if(Mediator) return Mediator->getObject(ID);
    else {
        return NULL;
    }
}

cUPnPClassObject* cUPnPObjectFactory::createObject(const char* UPnPClass, const char* Title, bool Restricted){
    cMediatorInterface* Mediator = this->findMediatorByClass(UPnPClass);
    return Mediator->createObject(Title, Restricted);
}

int cUPnPObjectFactory::deleteObject(cUPnPClassObject* Object){
    cMediatorInterface* Mediator = this->findMediatorByClass(Object->getClass());
    return Mediator->deleteObject(Object);
}

int cUPnPObjectFactory::clearObject(cUPnPClassObject* Object){
    cMediatorInterface* Mediator = this->findMediatorByClass(Object->getClass());
    return Mediator->clearObject(Object);
}

int cUPnPObjectFactory::saveObject(cUPnPClassObject* Object){
    cMediatorInterface* Mediator = this->findMediatorByClass(Object->getClass());
    return Mediator->saveObject(Object);
}

 /**********************************************\
 *                                              *
 *  Mediators                                   *
 *                                              *
 \**********************************************/

 /**********************************************\
 *                                              *
 *  Object mediator                             *
 *                                              *
 \**********************************************/

cUPnPObjectMediator::cUPnPObjectMediator(cMediaDatabase* MediaDatabase) :
    mMediaDatabase(MediaDatabase){
    this->mDatabase = cSQLiteDatabase::getInstance();
}

cUPnPObjectMediator::~cUPnPObjectMediator(){
    delete this->mDatabase;
    delete this->mMediaDatabase;
}

int cUPnPObjectMediator::saveObject(cUPnPClassObject* Object){
    bool succesful = true;
    
    this->mDatabase->startTransaction();
    if(Object->getID() == -1) succesful = false;
    else if(this->objectToDatabase(Object)) succesful = false;
    else succesful = true;

    if(succesful){
        this->mDatabase->commitTransaction();
        Object->setModified();
        this->mMediaDatabase->cacheObject(Object);
        this->mMediaDatabase->updateSystemID();
        return 0;
    }
    else {
        this->mDatabase->rollbackTransaction();
        return -1;
    }
    return -1;
}

int cUPnPObjectMediator::deleteObject(cUPnPClassObject* Object){
    cString Format = "DELETE FROM %s WHERE %s=%Q";
    if(this->mDatabase->execStatement(Format, SQLITE_TABLE_OBJECTS, SQLITE_COL_OBJECTID, *Object->getID())){
        ERROR("Error while executing statement");
        return -1;
    }
    #ifdef SQLITE_CASCADE_DELETES
    this->clearObject(Object);
    #endif
    delete Object; Object = NULL;
    return 0;
}

int cUPnPObjectMediator::clearObject(cUPnPClassObject* Object){
    cUPnPClassContainer* Container = Object->getContainer();
    if(Container){
        cList<cUPnPClassObject>* List = Container->getObjectList();
        for(cUPnPClassObject* Child = List->First(); Child; Child = List->Next(Child)){
            if(this->deleteObject(Child)) return -1;
        }
    }
    return 0;
}

int cUPnPObjectMediator::initializeObject(cUPnPClassObject* Object, const char* Class, const char* Title, bool Restricted){
    cUPnPObjectID ObjectID = this->mMediaDatabase->getNextObjectID();
    if(Object->setID(ObjectID)){
        ERROR("Error while setting ID");
        return -1;
    }
    cUPnPClassObject* Root = this->mMediaDatabase->getObjectByID(0);
    if(Root){
        Root->getContainer()->addObject(Object);
    }
    else {
        Object->setParent(NULL);
    }
    if(Object->setClass(Class)){
        ERROR("Error while setting class");
        return -1;
    }
    if(Object->setTitle(Title)){
        ERROR("Error while setting title");
        return -1;
    }
    if(Object->setRestricted(Restricted)){
        ERROR("Error while setting restriction");
        return -1;
    }
    if(this->mDatabase->execStatement("INSERT INTO %s (%s, %s, %s, %s, %s) VALUES (%s, %s, %Q, %Q, %d)",
                                         SQLITE_TABLE_OBJECTS,
                                         SQLITE_COL_OBJECTID,
                                         SQLITE_COL_PARENTID,
                                         SQLITE_COL_CLASS,
                                         SQLITE_COL_TITLE,
                                         SQLITE_COL_RESTRICTED,
                                         *Object->getID(),
                                         *Object->getParentID(),
                                         Object->getClass(),
                                         Object->getTitle(),
                                         Object->isRestricted()?1:0)){
        ERROR("Error while executing statement");
        return -1;
    }
    return 0;
}

cUPnPClassObject* cUPnPObjectMediator::getObject(cUPnPObjectID){ WARNING("Getting instance of class 'Object' forbidden"); return NULL; }

cUPnPClassObject* cUPnPObjectMediator::createObject(const char*, bool){ WARNING("Getting instance of class 'Object' forbidden"); return NULL; }

int cUPnPObjectMediator::objectToDatabase(cUPnPClassObject* Object){
    MESSAGE(VERBOSE_MODIFICATIONS, "Updating object #%s", *Object->getID());
    cString Format = "UPDATE %s SET %s WHERE %s='%s'";
    //cString Format = "INSERT OR REPLACE INTO %s (%s) VALUES (%s);";
    cString Set=NULL;
    //cString Columns=NULL, Values=NULL;
    char *Value=NULL;
    cString Properties[] = {
        SQLITE_COL_OBJECTID,
        SQLITE_COL_PARENTID,
        SQLITE_COL_CLASS,
        SQLITE_COL_TITLE,
        SQLITE_COL_RESTRICTED,
        SQLITE_COL_CREATOR,
        SQLITE_COL_WRITESTATUS,
        NULL
    };
    for(cString* Property = Properties; *(*Property)!=NULL; Property++){
        if(!Object->getProperty(*Property, &Value)){
            ERROR("No such property '%s' in object with ID '%s'",*(*Property),*Object->getID());
            return -1;
        }
        Set = cSQLiteDatabase::sprintf("%s%s%s=%Q", *Set?*Set:"", *Set?",":"", *(*Property), Value);
    }
    if(this->mDatabase->execStatement(Format, SQLITE_TABLE_OBJECTS, *Set, SQLITE_COL_OBJECTID, *Object->mLastID)){
        ERROR("Error while executing statement");
        return -1;
    }
    // The update was successful --> the current ID is now also the LastID
    Object->mLastID = Object->mID;
    return 0;
}

int cUPnPObjectMediator::databaseToObject(cUPnPClassObject* Object, cUPnPObjectID ID){
    cString Column = NULL, Value = NULL;
    cRows* Rows; cRow* Row;
    if(this->mDatabase->execStatement("SELECT * FROM %s WHERE %s=%Q",
                                        SQLITE_TABLE_OBJECTS,
                                        SQLITE_COL_OBJECTID,
                                        *ID)){
        ERROR("Error while executing statement");
        return -1;
    }
    Rows = this->mDatabase->getResultRows();
    if(!Rows->fetchRow(&Row)){
        ERROR("No such object with ID '%s'",*ID);
        return -1;
    }
    while(Row->fetchColumn(&Column, &Value)){
        if(!strcasecmp(Column, SQLITE_COL_OBJECTID)){
            if(!*Value || Object->setID(atoi(Value))){
                ERROR("Error while setting object ID");
                return -1;
            }
            this->mMediaDatabase->cacheObject(Object);
        }
        else if(!strcasecmp(Column, SQLITE_COL_PARENTID)){
            if(*Value){
                cUPnPObjectID RefID = atoi(Value);
                cUPnPClassContainer* ParentObject;
                if(RefID == -1){
                    ParentObject = NULL;
                }
                else {
                    ParentObject = (cUPnPClassContainer*)this->mMediaDatabase->getObjectByID(RefID);
                    if(!ParentObject){
                        ERROR("No such parent with ID '%s' found.",*RefID);
                        return -1;
                    }
                }
                Object->setParent(ParentObject);
            }
            else {
                ERROR("Invalid parent ID");
                return -1;
            }
        }
        else if(!strcasecmp(Column, SQLITE_COL_CLASS)){
            if(!*Value || Object->setClass(Value)){
                ERROR("Error while setting class");
                return -1;
            }
        }
        else if(!strcasecmp(Column, SQLITE_COL_TITLE)){
            if(!*Value || Object->setTitle(Value)){
                ERROR("Error while setting title");
                return -1;
            }
        }
        else if(!strcasecmp(Column, SQLITE_COL_RESTRICTED)){
            if(!*Value || Object->setRestricted(atoi(Value)==1?true:false)){
                ERROR("Error while setting restriction");
                return -1;
            }
        }
        else if(!strcasecmp(Column, SQLITE_COL_CREATOR)){
            if(Object->setCreator(Value)){
                ERROR("Error while setting creator");
                return -1;
            }
        }
        else if(!strcasecmp(Column, SQLITE_COL_WRITESTATUS)){
            if(*Value && Object->setWriteStatus(atoi(Value))){
                ERROR("Error while setting write status");
                return -1;
            }
        }
    }
    cUPnPResources::getInstance()->getResourcesOfObject(Object);
    return 0;
}

 /**********************************************\
 *                                              *
 *  Item mediator                               *
 *                                              *
 \**********************************************/

cUPnPItemMediator::cUPnPItemMediator(cMediaDatabase* MediaDatabase) :
    cUPnPObjectMediator(MediaDatabase){}

int cUPnPItemMediator::objectToDatabase(cUPnPClassObject* Object){
    if(cUPnPObjectMediator::objectToDatabase(Object)) return -1;
    cString Format = "INSERT OR REPLACE INTO %s (%s) VALUES (%s);";
    cString Columns=NULL, Values=NULL;
    char *Value=NULL;
    cString Properties[] = {
        SQLITE_COL_OBJECTID,
        SQLITE_COL_REFERENCEID,
        NULL
    };
    for(cString* Property = Properties; *(*Property); Property++){
        Columns = cSQLiteDatabase::sprintf("%s%s%s", *Columns?*Columns:"", *Columns?",":"", *(*Property));
        if(!Object->getProperty(*Property, &Value)){
            ERROR("No such property '%s' in object with ID '%s'",*(*Property),*Object->getID());
            return -1;
        }
        Values = cSQLiteDatabase::sprintf("%s%s%Q", *Values?*Values:"", *Values?",":"", Value);
        
    }
    if(this->mDatabase->execStatement(Format, SQLITE_TABLE_ITEMS, *Columns, *Values)){
        ERROR("Error while executing statement");
        return -1;
    }
    return 0;
}

int cUPnPItemMediator::databaseToObject(cUPnPClassObject* Object, cUPnPObjectID ID){
    if(cUPnPObjectMediator::databaseToObject(Object,ID)){
        ERROR("Error while loading object");
        return -1;
    }
    cUPnPClassItem* Item = (cUPnPClassItem*) Object;
    cString Column = NULL, Value = NULL;
    cRows* Rows; cRow* Row;
    if(this->mDatabase->execStatement("SELECT * FROM %s WHERE %s=%Q",
                                       SQLITE_TABLE_ITEMS,
                                       SQLITE_COL_OBJECTID,
                                       *ID)){
        ERROR("Error while executing statement");
        return -1;
    }
    Rows = this->mDatabase->getResultRows();
    if(!Rows->fetchRow(&Row)){
        MESSAGE(VERBOSE_SQL, "No item properties found");
        return 0;
    }
    while(Row->fetchColumn(&Column, &Value)){
        if(!strcasecmp(Column, SQLITE_COL_REFERENCEID)){
            cUPnPObjectID RefID = atoi(Value);
            cUPnPClassItem* RefObject;
            if(RefID == -1){
                RefObject = NULL;
            }
            else {
                RefObject = (cUPnPClassItem*)this->mMediaDatabase->getObjectByID(RefID);
                if(!RefObject){
                    ERROR("No such reference item with ID '%s' found.",*RefID);
                    return -1;
                }
            }
            Item->setReference(RefObject);
        }
    }
    return 0;
}

cUPnPClassItem* cUPnPItemMediator::getObject(cUPnPObjectID ID){
    MESSAGE(VERBOSE_METADATA, "Getting Item with ID '%s'",*ID);
    cUPnPClassItem* Object = new cUPnPClassItem;
    if(this->databaseToObject(Object, ID)) return NULL;
    return Object;
}

cUPnPClassItem* cUPnPItemMediator::createObject(const char* Title, bool Restricted){
    MESSAGE(VERBOSE_MODIFICATIONS, "Creating Item '%s'",Title);
    cUPnPClassItem* Object = new cUPnPClassItem;
    if(this->initializeObject(Object, UPNP_CLASS_ITEM, Title, Restricted)) return NULL;
    return Object;
}

 /**********************************************\
 *                                              *
 *  Container mediator                          *
 *                                              *
 \**********************************************/

cUPnPContainerMediator::cUPnPContainerMediator(cMediaDatabase* MediaDatabase) :
    cUPnPObjectMediator(MediaDatabase){}

int cUPnPContainerMediator::objectToDatabase(cUPnPClassObject* Object){
    if(cUPnPObjectMediator::objectToDatabase(Object)) return -1;
    cUPnPClassContainer* Container = (cUPnPClassContainer*)Object;
    cString Format = "INSERT OR REPLACE INTO %s (%s) VALUES (%s);";
    cString Columns=NULL, Values=NULL;
    char *Value=NULL;
    cString Properties[] = {
        SQLITE_COL_OBJECTID,
        SQLITE_COL_DLNA_CONTAINERTYPE,
        SQLITE_COL_SEARCHABLE,
        SQLITE_COL_CONTAINER_UID,
        NULL
    };
    for(cString* Property = Properties; *(*Property); Property++){
        Columns = cSQLiteDatabase::sprintf("%s%s%s", *Columns?*Columns:"", *Columns?",":"", *(*Property));
        if(!Container->getProperty(*Property, &Value)){
            ERROR("No such property '%s' in object with ID '%s'",*(*Property),*Container->getID());
            return -1;
        }
        Values = cSQLiteDatabase::sprintf("%s%s%Q", *Values?*Values:"", *Values?",":"", Value);
    }
    if(this->mDatabase->execStatement(Format, SQLITE_TABLE_CONTAINERS, *Columns, *Values)){
        ERROR("Error while executing statement");
        return -1;
    }
    for(unsigned int i=0; i<Container->getSearchClasses()->size(); i++){
        cClass Class = Container->getSearchClasses()->at(i);
        Columns = cSQLiteDatabase::sprintf("%s,%s,%s", SQLITE_COL_OBJECTID, SQLITE_COL_CLASS, SQLITE_COL_CLASSDERIVED);
        Values = cSQLiteDatabase::sprintf("%Q,%Q,%d", *Container->getID(), *Class.ID, Class.includeDerived?1:0);
        if(this->mDatabase->execStatement(Format, SQLITE_TABLE_SEARCHCLASS, *Columns, *Values)){
            ERROR("Error while executing statement");
            return -1;
        }
    }
    // Create classes not necessary at the moment
    return 0;
}

int cUPnPContainerMediator::databaseToObject(cUPnPClassObject* Object, cUPnPObjectID ID){
    if(cUPnPObjectMediator::databaseToObject(Object,ID)){
        ERROR("Error while loading object");
        return -1;
    }
    cUPnPClassContainer* Container = (cUPnPClassContainer*)Object;
    cString Format = "SELECT * FROM %s WHERE %s=%s";
    cString Column = NULL, Value = NULL;
    cRows* Rows; cRow* Row;
    if(this->mDatabase->execStatement(Format, SQLITE_TABLE_CONTAINERS, SQLITE_COL_OBJECTID, *ID)){
        ERROR("Error while executing statement");
        return -1;
    }
    Rows = this->mDatabase->getResultRows();
    if(!Rows->fetchRow(&Row)){
        MESSAGE(VERBOSE_SQL, "No item properties found");
        return 0;
    }
    while(Row->fetchColumn(&Column, &Value)){
        if(!strcasecmp(Column, SQLITE_COL_DLNA_CONTAINERTYPE)){
            if(Container->setContainerType(Value)){
                ERROR("Error while setting container type");
                return -1;
            }
        }
        if(!strcasecmp(Column, SQLITE_COL_CONTAINER_UID)){
            if(Container->setUpdateID((unsigned int)atoi(Value))){
                ERROR("Error while setting update ID");
                return -1;
            }
        }
        if(!strcasecmp(Column, SQLITE_COL_SEARCHABLE)){
            if(Container->setSearchable(atoi(Value)==1?true:false)){
                ERROR("Error while setting searchable");
                return -1;
            }
        }
    }
    if(this->mDatabase->execStatement("SELECT %s FROM %s WHERE %s=%s", SQLITE_COL_OBJECTID,
                                                                  SQLITE_TABLE_OBJECTS,
                                                                  SQLITE_COL_PARENTID,
                                                                  *ID)){
        ERROR("Error while executing statement");
        return -1;
    }
    Rows = this->mDatabase->getResultRows();
    while(Rows->fetchRow(&Row)){
        while(Row->fetchColumn(&Column, &Value)){
            if(!strcasecmp(Column, SQLITE_COL_OBJECTID)){
                Container->addObject(this->mMediaDatabase->getObjectByID(atoi(Value)));
            }
        }
    }
    if(this->mDatabase->execStatement(Format, SQLITE_TABLE_SEARCHCLASS, SQLITE_COL_OBJECTID, *ID)){
        ERROR("Error while executing statement");
        return -1;
    }
    std::vector<cClass> SearchClasses;
    Rows = this->mDatabase->getResultRows();
    while(Rows->fetchRow(&Row)){
        cClass Class;
        while(Row->fetchColumn(&Column, &Value)){
            if(!strcasecmp(Column, SQLITE_COL_CLASS)){
                Class.ID = strdup0(*Value);
            }
            else if(!strcasecmp(Column, SQLITE_COL_CLASSDERIVED)){
                Class.includeDerived = atoi(Value)==1?true:false;
            }
        }
        SearchClasses.push_back(Class);
    }
    if(Container->setSearchClasses(SearchClasses)){
        ERROR("Error while setting search classes");
        return -1;
    }
    return 0;
}

cUPnPClassContainer* cUPnPContainerMediator::createObject(const char* Title, bool Restricted){
    MESSAGE(VERBOSE_MODIFICATIONS, "Creating Container '%s'",Title);
    cUPnPClassContainer* Object = new cUPnPClassContainer;
    if(this->initializeObject(Object, UPNP_CLASS_CONTAINER, Title, Restricted)) return NULL;
    return Object;
}

cUPnPClassContainer* cUPnPContainerMediator::getObject(cUPnPObjectID ID){
    MESSAGE(VERBOSE_METADATA, "Getting Container with ID '%s'",*ID);
    cUPnPClassContainer* Object = new cUPnPClassContainer;
    if(this->databaseToObject(Object, ID)) return NULL;
    return Object;
}

 /**********************************************\
 *                                              *
 *  Video item mediator                         *
 *                                              *
 \**********************************************/

cUPnPVideoItemMediator::cUPnPVideoItemMediator(cMediaDatabase* MediaDatabase) :
    cUPnPItemMediator(MediaDatabase){}

cUPnPClassVideoItem* cUPnPVideoItemMediator::createObject(const char* Title, bool Restricted){
    MESSAGE(VERBOSE_MODIFICATIONS, "Creating Video item '%s'",Title);
    cUPnPClassVideoItem* Object = new cUPnPClassVideoItem;
    if(this->initializeObject(Object, UPNP_CLASS_VIDEO, Title, Restricted)) return NULL;
    return Object;
}

cUPnPClassVideoItem* cUPnPVideoItemMediator::getObject(cUPnPObjectID ID){
    MESSAGE(VERBOSE_METADATA, "Getting Video item with ID '%s'",*ID);
    cUPnPClassVideoItem* Object = new cUPnPClassVideoItem;
    if(this->databaseToObject(Object, ID)) return NULL;
    return Object;
}

int cUPnPVideoItemMediator::objectToDatabase(cUPnPClassObject* Object){
    if(cUPnPItemMediator::objectToDatabase(Object)) return -1;
    cUPnPClassVideoItem* VideoItem = (cUPnPClassVideoItem*)Object;
    cString Format = "INSERT OR REPLACE INTO %s (%s) VALUES (%s);";
    cString Columns=NULL, Values=NULL;
    char *Value=NULL;
    cString Properties[] = {
        SQLITE_COL_OBJECTID,
        SQLITE_COL_GENRE,
        SQLITE_COL_LONGDESCRIPTION,
        SQLITE_COL_PRODUCER,
        SQLITE_COL_RATING,
        SQLITE_COL_ACTOR,
        SQLITE_COL_DIRECTOR,
        SQLITE_COL_DESCRIPTION,
        SQLITE_COL_PUBLISHER,
        SQLITE_COL_LANGUAGE,
        SQLITE_COL_RELATION,
        NULL
    };
    for(cString* Property = Properties; *(*Property); Property++){
        Columns = cSQLiteDatabase::sprintf("%s%s%s", *Columns?*Columns:"", *Columns?",":"", *(*Property));
        if(!VideoItem->getProperty(*Property, &Value)){
            ERROR("No such property '%s' in object with ID '%s'",*(*Property),* VideoItem->getID());
            return -1;
        }
        Values = cSQLiteDatabase::sprintf("%s%s%Q", *Values?*Values:"", *Values?",":"", Value);
    }
    if(this->mDatabase->execStatement(Format, SQLITE_TABLE_VIDEOITEMS, *Columns, *Values)){
        ERROR("Error while executing statement");
        return -1;
    }
    return 0;
}

int cUPnPVideoItemMediator::databaseToObject(cUPnPClassObject* Object, cUPnPObjectID ID){
    if(cUPnPItemMediator::databaseToObject(Object,ID)){
        ERROR("Error while loading object");
        return -1;
    }
    cUPnPClassVideoItem* VideoItem = (cUPnPClassVideoItem*)Object;
    cString Format = "SELECT * FROM %s WHERE %s=%s";
    cString Column = NULL, Value = NULL;
    cRows* Rows; cRow* Row;
    if(this->mDatabase->execStatement(Format, SQLITE_TABLE_VIDEOITEMS, SQLITE_COL_OBJECTID, *ID)){
        ERROR("Error while executing statement");
        return -1;
    }
    Rows = this->mDatabase->getResultRows();
    if(!Rows->fetchRow(&Row)){
        MESSAGE(VERBOSE_SQL, "No item properties found");
        return 0;
    }
    while(Row->fetchColumn(&Column, &Value)){
        if(!strcasecmp(Column, SQLITE_COL_GENRE)){
            if(VideoItem->setGenre(Value)){
                ERROR("Error while setting genre");
                return -1;
            }
        }
        else if(!strcasecmp(Column, SQLITE_COL_LONGDESCRIPTION)){
            if(VideoItem->setLongDescription(Value)){
                ERROR("Error while setting long description");
                return -1;
            }
        }
        else if(!strcasecmp(Column, SQLITE_COL_PRODUCER)){
            if(VideoItem->setProducers(Value)){
                ERROR("Error while setting producers");
                return -1;
            }
        }
        else if(!strcasecmp(Column, SQLITE_COL_RATING)){
            if(VideoItem->setRating(Value)){
                ERROR("Error while setting rating");
                return -1;
            }
        }
        else if(!strcasecmp(Column, SQLITE_COL_ACTOR)){
            if(VideoItem->setActors(Value)){
                ERROR("Error while setting actors");
                return -1;
            }
        }
        else if(!strcasecmp(Column, SQLITE_COL_DIRECTOR)){
            if(VideoItem->setDirectors(Value)){
                ERROR("Error while setting directors");
                return -1;
            }
        }
        else if(!strcasecmp(Column, SQLITE_COL_DESCRIPTION)){
            if(VideoItem->setDescription(Value)){
                ERROR("Error while setting description");
                return -1;
            }
        }
        else if(!strcasecmp(Column, SQLITE_COL_PUBLISHER)){
            if(VideoItem->setPublishers(Value)){
                ERROR("Error while setting publishers");
                return -1;
            }
        }
        else if(!strcasecmp(Column, SQLITE_COL_LANGUAGE)){
            if(VideoItem->setLanguage(Value)){
                ERROR("Error while setting language");
                return -1;
            }
        }
        else if(!strcasecmp(Column, SQLITE_COL_RELATION)){
            if(VideoItem->setRelations(Value)){
                ERROR("Error while setting relations");
                return -1;
            }
        }
    }
    return 0;
}

 /**********************************************\
 *                                              *
 *  Video broadcast item mediator               *
 *                                              *
 \**********************************************/

cUPnPVideoBroadcastMediator::cUPnPVideoBroadcastMediator(cMediaDatabase* MediaDatabase) :
    cUPnPVideoItemMediator(MediaDatabase){}

cUPnPClassVideoBroadcast* cUPnPVideoBroadcastMediator::createObject(const char* Title, bool Restricted){
    MESSAGE(VERBOSE_MODIFICATIONS, "Creating Video broadcast '%s'",Title);
    cUPnPClassVideoBroadcast* Object = new cUPnPClassVideoBroadcast;
    if(this->initializeObject(Object, UPNP_CLASS_VIDEOBC, Title, Restricted)) return NULL;
    return Object;
}

cUPnPClassVideoBroadcast* cUPnPVideoBroadcastMediator::getObject(cUPnPObjectID ID){
    MESSAGE(VERBOSE_METADATA, "Getting Video broadcast with ID '%s'",*ID);
    cUPnPClassVideoBroadcast* Object = new cUPnPClassVideoBroadcast;
    if(this->databaseToObject(Object, ID)) return NULL;
    return Object;
}

int cUPnPVideoBroadcastMediator::objectToDatabase(cUPnPClassObject* Object){
    if(cUPnPVideoItemMediator::objectToDatabase(Object)) return -1;
    cUPnPClassVideoBroadcast* VideoBroadcast = (cUPnPClassVideoBroadcast*)Object;
    cString Format = "INSERT OR REPLACE INTO %s (%s) VALUES (%s);";
    cString Columns=NULL, Values=NULL;
    char *Value=NULL;
    cString Properties[] = {
        SQLITE_COL_OBJECTID,
        SQLITE_COL_ICON,
        SQLITE_COL_REGION,
        SQLITE_COL_CHANNELNAME,
        SQLITE_COL_CHANNELNR,
        NULL
    };
    for(cString* Property = Properties; *(*Property); Property++){
        Columns = cSQLiteDatabase::sprintf("%s%s%s", *Columns?*Columns:"", *Columns?",":"", *(*Property));
        if(!VideoBroadcast->getProperty(*Property, &Value)){
            ERROR("No such property '%s' in object with ID '%s'",*(*Property),* VideoBroadcast->getID());
            return -1;
        }
        Values = cSQLiteDatabase::sprintf("%s%s%Q", *Values?*Values:"", *Values?",":"", Value);
    }
    if(this->mDatabase->execStatement(Format, SQLITE_TABLE_VIDEOBROADCASTS, *Columns, *Values)){
        ERROR("Error while executing statement");
        return -1;
    }
    return 0;
}

int cUPnPVideoBroadcastMediator::databaseToObject(cUPnPClassObject* Object, cUPnPObjectID ID){
    if(cUPnPVideoItemMediator::databaseToObject(Object,ID)){
        ERROR("Error while loading object");
        return -1;
    }
    cUPnPClassVideoBroadcast* VideoBroadcast = (cUPnPClassVideoBroadcast*)Object;
    cString Format = "SELECT * FROM %s WHERE %s=%s";
    cString Column = NULL, Value = NULL;
    cRows* Rows; cRow* Row;
    if(this->mDatabase->execStatement(Format, SQLITE_TABLE_VIDEOBROADCASTS, SQLITE_COL_OBJECTID, *ID)){
        ERROR("Error while executing statement");
        return -1;
    }
    Rows = this->mDatabase->getResultRows();
    if(!Rows->fetchRow(&Row)){
        MESSAGE(VERBOSE_SQL, "No item properties found");
        return 0;
    }
    while(Row->fetchColumn(&Column, &Value)){
        if(!strcasecmp(Column, SQLITE_COL_ICON)){
            if(VideoBroadcast->setIcon(Value)){
                ERROR("Error while setting icon");
                return -1;
            }
        }
        else if(!strcasecmp(Column, SQLITE_COL_REGION)){
            if(VideoBroadcast->setRegion(Value)){
                ERROR("Error while setting region");
                return -1;
            }
        }
        else if(!strcasecmp(Column, SQLITE_COL_CHANNELNR)){
            if(VideoBroadcast->setChannelNr(atoi(Value))){
                ERROR("Error while setting channel number");
                return -1;
            }
        }
        else if(!strcasecmp(Column, SQLITE_COL_CHANNELNAME)){
            if(VideoBroadcast->setChannelName(Value)){
                ERROR("Error while setting channel name");
                return -1;
            }
        }
    }
    return 0;
}

/**********************************************\
*                                              *
*  Movie item mediator                         *
*                                              *
\**********************************************/

cUPnPMovieMediator::cUPnPMovieMediator(cMediaDatabase* MediaDatabase) :
    cUPnPVideoItemMediator(MediaDatabase){}

cUPnPClassMovie* cUPnPMovieMediator::createObject(const char* Title, bool Restricted){
    MESSAGE(VERBOSE_MODIFICATIONS, "Creating movie '%s'",Title);
    cUPnPClassMovie* Object = new cUPnPClassMovie;
    if(this->initializeObject(Object, UPNP_CLASS_MOVIE, Title, Restricted)) return NULL;
    return Object;
}

cUPnPClassMovie* cUPnPMovieMediator::getObject(cUPnPObjectID ID){
    MESSAGE(VERBOSE_METADATA, "Getting movie with ID '%s'",*ID);
    cUPnPClassMovie* Object = new cUPnPClassMovie;
    if(this->databaseToObject(Object, ID)) return NULL;
    return Object;
}

int cUPnPMovieMediator::objectToDatabase(cUPnPClassObject* Object){
    if(cUPnPVideoItemMediator::objectToDatabase(Object)) return -1;
    cUPnPClassMovie* Movie = (cUPnPClassMovie*)Object;
    cString Format = "INSERT OR REPLACE INTO %s (%s) VALUES (%s);";
    cString Columns=NULL, Values=NULL;
    char *Value=NULL;
    cString Properties[] = {
        SQLITE_COL_OBJECTID,
        SQLITE_COL_DVDREGIONCODE,
        SQLITE_COL_STORAGEMEDIUM,
        NULL
    };
    for(cString* Property = Properties; *(*Property); Property++){
        Columns = cSQLiteDatabase::sprintf("%s%s%s", *Columns?*Columns:"", *Columns?",":"", *(*Property));
        if(!Movie->getProperty(*Property, &Value)){
            ERROR("No such property '%s' in object with ID '%s'",*(*Property),* Movie->getID());
            return -1;
        }
        Values = cSQLiteDatabase::sprintf("%s%s%Q", *Values?*Values:"", *Values?",":"", Value);
    }
    if(this->mDatabase->execStatement(Format, SQLITE_TABLE_MOVIES, *Columns, *Values)){
        ERROR("Error while executing statement");
        return -1;
    }
    return 0;
}

int cUPnPMovieMediator::databaseToObject(cUPnPClassObject* Object, cUPnPObjectID ID){
    if(cUPnPVideoItemMediator::databaseToObject(Object,ID)){
        ERROR("Error while loading object");
        return -1;
    }
    cUPnPClassMovie* Movie = (cUPnPClassMovie*)Object;
    cString Format = "SELECT * FROM %s WHERE %s=%s";
    cString Column = NULL, Value = NULL;
    cRows* Rows; cRow* Row;
    if(this->mDatabase->execStatement(Format, SQLITE_TABLE_MOVIES, SQLITE_COL_OBJECTID, *ID)){
        ERROR("Error while executing statement");
        return -1;
    }
    Rows = this->mDatabase->getResultRows();
    if(!Rows->fetchRow(&Row)){
        MESSAGE(VERBOSE_SQL, "No item properties found");
        return 0;
    }
    while(Row->fetchColumn(&Column, &Value)){
        if(!strcasecmp(Column, SQLITE_COL_DVDREGIONCODE)){
            if(Movie->setDVDRegionCode(atoi(Value))){
                ERROR("Error while setting icon");
                return -1;
            }
        }
        else if(!strcasecmp(Column, SQLITE_COL_STORAGEMEDIUM)){
            if(Movie->setStorageMedium(Value)){
                ERROR("Error while setting region");
                return -1;
            }
        }
    }
    return 0;
}