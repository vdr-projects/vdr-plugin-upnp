/* 
 * File:   object.h
 * Author: savop
 *
 * Created on 11. September 2009, 20:39
 */

#ifndef _OBJECT_H
#define	_OBJECT_H

#include "database.h"
#include "../common.h"
#include "../misc/util.h"
#include <string.h>
#include <vdr/tools.h>
#include <map>
#include <vector>
#include <upnp/ixml.h>

struct cUPnPObjectID {
    int _ID;
    cUPnPObjectID():_ID(-1){}
    cUPnPObjectID(long ID){ _ID = (int)ID; }
    cUPnPObjectID(int ID){ _ID = ID; }
    cUPnPObjectID &operator=(long ID){ _ID = ID; return *this; }
    cUPnPObjectID &operator=(int ID){ _ID = ID; return *this; }
    cUPnPObjectID &operator=(const cUPnPObjectID& ID){ if(this != &ID){ _ID = ID._ID; } return *this; }
    cUPnPObjectID &operator++(){ _ID++; return *this; }
    cUPnPObjectID operator++(int){ cUPnPObjectID old = *this; _ID++; return old; }
    cUPnPObjectID operator--(int){ cUPnPObjectID old = *this; _ID--; return old; }
    cUPnPObjectID &operator--(){ _ID--; return *this; }
    bool operator!=(long ID){ return _ID != ID; }
    bool operator==(long ID){ return _ID == ID; }
    bool operator!=(int ID){ return _ID != ID; }
    bool operator==(int ID){ return _ID == ID; }
    bool operator!=(const cUPnPObjectID& ID){ return *this == ID; }
    bool operator==(const cUPnPObjectID& ID){ return *this == ID; }
    operator unsigned int(){ return (unsigned int)_ID; }
    operator int(){ return _ID; }
    operator long(){ return (long)_ID; }
    const char* operator*(){ char* buf; return asprintf(&buf,"%d",_ID)?buf:NULL; }
};

struct cClass {
    cString ID;
    bool includeDerived;
    bool operator==(const cClass &cmp){ return (!strcasecmp(cmp.ID,ID) && includeDerived==cmp.includeDerived); }
    bool operator!=(const cClass &cmp){ return !(*this==cmp); }
};

class cUPnPResource : public cListObject {
    friend class cUPnPResourceMediator;
    friend class cUPnPResources;
private:
    unsigned int mResourceID;
    cUPnPObjectID mObjectID;
    int     mResourceType;
    cString mResource;
    cString mDuration;
    cString mResolution;
    cString mProtocolInfo;
    cString mContentType;
    cString mImportURI;
    unsigned long mSize;
    unsigned int mBitrate;
    unsigned int mSampleFrequency;
    unsigned int mBitsPerSample;
    unsigned int mNrAudioChannels;
    unsigned int mColorDepth;
    cUPnPResource();
public:
    unsigned int getID() const { return this->mResourceID; }
    const char* getResource() const { return this->mResource; }
    const char* getDuration() const { return this->mDuration; }
    const char* getResolution() const { return this->mResolution; }
    const char* getProtocolInfo() const { return this->mProtocolInfo; }
    const char* getContentType() const { return this->mContentType; }
    const char* getImportURI() const { return this->mImportURI; }
    int         getResourceType() const { return this->mResourceType; }
    unsigned long getSize() const { return this->mSize; }
    off64_t     getFileSize() const;
    time_t      getLastModification() const;
    unsigned int getBitrate() const { return this->mBitrate; }
    unsigned int getSampleFrequency() const { return this->mSampleFrequency; }
    unsigned int getBitsPerSample() const { return this->mBitsPerSample; }
    unsigned int getNrAudioChannels() const { return this->mNrAudioChannels; }
    unsigned int getColorDepth() const { return this->mColorDepth; }
};

class cUPnPClassObject;
class cUPnPObjectMediator;
class cUPnPContainerMediator;
class cUPnPClassContainer;

class cUPnPObjects : public cList<cUPnPClassObject> {
public:
    cUPnPObjects();
    virtual ~cUPnPObjects();
    void SortBy(const char* Property, bool Descending = false);
};

class cUPnPClassObject : public cListObject {
    friend class cMediaDatabase;
    friend class cUPnPObjectMediator;
    friend class cUPnPClassContainer;
private:
    cUPnPObjectID           mLastID;
    bool                    mDeleted;                                           // is this Objected marked as deleted
protected:
    time_t                  mLastModified;
    cUPnPObjectID           mID;                                                // The object ID
    cUPnPClassObject*       mParent;
    cString                 mClass;                                             // Class (Who am I?)
    cString                 mTitle;                                             // Object title
    cString                 mCreator;                                           // Creator of this object
    bool                    mRestricted;                                        // Ability of changing metadata?
    int                     mWriteStatus;                                       // Ability of writing resources?
    cList<cUPnPResource>*   mResources;                                         // The resources of this object
    cHash<cUPnPResource>*   mResourcesID;
    IXML_Document*          mDIDLFragment;
    cString                 mSortCriteria;
    bool                    mSortDescending;
    cUPnPClassObject();
    int setID(cUPnPObjectID ID);
    int setParent(cUPnPClassContainer* Parent);
    int setClass(const char* Class);
    void setModified(void){ this->mLastModified = time(NULL); }
public:
    time_t  modified() const { return this->mLastModified; }
    virtual ~cUPnPClassObject();
    virtual int Compare(const cListObject& ListObject) const;
    virtual cStringList* getPropertyList();
    virtual bool getProperty(const char* Property, char** Value) const ;
    virtual bool setProperty(const char* Property, const char* Value);
    virtual cUPnPClassContainer* getContainer(){ return NULL; }
    virtual IXML_Node* createDIDLFragment(IXML_Document* Document, cStringList* Filter) = 0;
    bool isContainer(){ return this->getContainer()==NULL?false:true; }
    void setSortCriteria(const char* Property, bool Descending = false);
    void clearSortCriteria();
    /******* Setter *******/
    int setTitle(const char* Title);
    int setCreator(const char* Creator);
    int setRestricted(bool Restricted);
    int setWriteStatus(int Status);
    int setResources(cList<cUPnPResource>* Resources);
    int addResource(cUPnPResource* Resource);
    int removeResource(cUPnPResource* Resource);
    /******* Getter *******/
    cUPnPObjectID getID() const { return this->mID; }
    cUPnPObjectID getParentID() const { return this->mParent?this->mParent->getID():cUPnPObjectID(-1); }
    cUPnPClassContainer* getParent() const { return (cUPnPClassContainer*)this->mParent; }
    const char* getTitle() const { return this->mTitle; }
    const char* getClass() const { return this->mClass; }
    const char* getCreator() const { return this->mCreator; }
    bool        isRestricted() const { return this->mRestricted; }
    int getWriteStatus() const { return this->mWriteStatus; }
    cUPnPResource* getResource(unsigned int ResourceID) const { return this->mResourcesID->Get(ResourceID); }
    cList<cUPnPResource>* getResources() const { return this->mResources; }
};

class cUPnPClassItem : public cUPnPClassObject {
    friend class cMediaDatabase;
    friend class cUPnPObjectMediator;
    friend class cUPnPItemMediator;
protected:
//    cUPnPObjectID             mReferenceID;
    cUPnPClassItem*           mReference;
    cUPnPClassItem();
public:
    virtual ~cUPnPClassItem(){};
    virtual cStringList* getPropertyList();
    virtual IXML_Node* createDIDLFragment(IXML_Document* Document, cStringList* Filter);
    virtual bool setProperty(const char* Property, const char* Value);
    virtual bool getProperty(const char* Property, char** Value) const;
    /******** Setter ********/
    int setReference(cUPnPClassItem* Reference);
    /******** Getter ********/
    cUPnPClassItem* getReference() const { return this->mReference; }
    cUPnPObjectID   getReferenceID() const { return this->mReference?this->mReference->getID():cUPnPObjectID(-1); }
};

typedef std::vector<cClass> tClassVector;

class cUPnPClassContainer : public cUPnPClassObject {
    friend class cMediaDatabase;
    friend class cUPnPObjectMediator;
    friend class cUPnPContainerMediator;
protected:
    cString                     mContainerType;
    tClassVector                mSearchClasses;
    tClassVector                mCreateClasses;
    bool                        mSearchable;
    unsigned int                mUpdateID;
    cUPnPObjects*               mChildren;
    cHash<cUPnPClassObject>*    mChildrenID;
    void update();
    int setUpdateID(unsigned int UID);
    cUPnPClassContainer();
public:
    virtual ~cUPnPClassContainer();
    virtual cStringList* getPropertyList();
    virtual IXML_Node* createDIDLFragment(IXML_Document* Document, cStringList* Filter);
    virtual bool setProperty(const char* Property, const char* Value);
    virtual bool getProperty(const char* Property, char** Value) const;
    virtual cUPnPClassContainer* getContainer(){ return this; }
    void addObject(cUPnPClassObject* Object);
    void removeObject(cUPnPClassObject* Object);
    cUPnPClassObject* getObject(cUPnPObjectID ID) const;
    cUPnPObjects* getObjectList() const { return this->mChildren; }
    int addSearchClass(cClass SearchClass);
    int delSearchClass(cClass SearchClass);
    int addCreateClass(cClass CreateClass);
    int delCreateClass(cClass CreateClass);
    /******** Setter ********/
    int setContainerType(const char* Type);
    int setSearchClasses(std::vector<cClass> SearchClasses);
    int setCreateClasses(std::vector<cClass> CreateClasses);
    int setSearchable(bool Searchable);
    /******** Getter ********/
    const char*   getContainerType() const { return this->mContainerType; }
    const std::vector<cClass>* getSearchClasses() const { return &(this->mSearchClasses); }
    const std::vector<cClass>* getCreateClasses() const { return &(this->mCreateClasses); }
    bool          isSearchable() const { return this->mSearchable; }
    unsigned int getChildCount() const { return this->mChildren->Count(); }
    unsigned int getUpdateID() const { return this->mUpdateID; }
    bool         isUpdated();
};

class cUPnPClassVideoItem : public cUPnPClassItem {
    friend class cMediaDatabase;
    friend class cUPnPObjectMediator;
    friend class cUPnPVideoItemMediator;
protected:
    cString             mGenre;                                                 // Genre
    cString             mDescription;                                           // Description
    cString             mLongDescription;                                       // a longer description
    cString             mPublishers;                                            // CSV of Publishers
    cString             mLanguage;                                              // RFC 1766 Language code
    cString             mRelations;                                             // Relation to other contents
    cString             mProducers;                                             // CSV of Producers
    cString             mRating;                                                // Rating (for parential control)
    cString             mActors;                                                // CSV of Actors
    cString             mDirectors;                                             // CSV of Directors
    cUPnPClassVideoItem();
public:
    virtual ~cUPnPClassVideoItem();
    //virtual cString createDIDLFragment(cStringList* Filter);
    virtual cStringList* getPropertyList();
    virtual bool setProperty(const char* Property, const char* Value);
    virtual bool getProperty(const char* Property, char** Value) const;
    /******** Setter ********/
    int setLongDescription(const char* LongDescription);
    int setDescription(const char* Description);
    int setPublishers(const char* Publishers);
    int setGenre(const char* Genre);
    int setLanguage(const char* Language);
    int setRelations(const char* Relations);
    int setDirectors(const char* Directors);
    int setActors(const char* Actors);
    int setProducers(const char* Producers);
    int setRating(const char* Rating);
    /******** Getter ********/
    const char* getGenre() const { return this->mGenre; }
    const char* getLongDescription() const { return this->mLongDescription; }
    const char* getDescription() const { return this->mDescription; }
    const char* getPublishers() const { return this->mPublishers; }
    const char* getLanguage() const { return this->mLanguage; }
    const char* getRelations() const { return this->mRelations; }
    const char* getActors() const { return this->mActors; }
    const char* getProducers() const { return this->mProducers; }
    const char* getDirectors() const { return this->mDirectors; }
    const char* getRating() const { return this->mRating; }
};

class cUPnPClassMovie : public cUPnPClassVideoItem {
    friend class cMediaDatabase;
    friend class cUPnPObjectMediator;
    friend class cUPnPMovieMediator;
protected:
    int          mDVDRegionCode;
    cString      mStorageMedium;
    cUPnPClassMovie();
public:
    virtual ~cUPnPClassMovie();
    //virtual cString createDIDLFragment(cStringList* Filter);
    virtual cStringList* getPropertyList();
    virtual bool setProperty(const char* Property, const char* Value);
    virtual bool getProperty(const char* Property, char** Value) const;
    /******** Setter ********/
    int setDVDRegionCode(int RegionCode);
    int setStorageMedium(const char* StorageMedium);
    /******** Getter ********/
    int getDVDRegionCode() const { return this->mDVDRegionCode; }
    const char* getStorageMedium() const { return this->mStorageMedium; }
};

class cUPnPClassVideoBroadcast : public cUPnPClassVideoItem {
    friend class cMediaDatabase;
    friend class cUPnPObjectMediator;
    friend class cUPnPVideoBroadcastMediator;
protected:
    cString             mIcon;
    cString             mRegion;
    int                 mChannelNr;
    cString             mChannelName;
    cUPnPClassVideoBroadcast();
public:
    virtual ~cUPnPClassVideoBroadcast();
    //virtual cString createDIDLFragment(cStringList* Filter);
    virtual cStringList* getPropertyList();
    virtual bool setProperty(const char* Property, const char* Value);
    virtual bool getProperty(const char* Property, char** Value) const;
    /******** Setter ********/
    int setIcon(const char* IconURI);
    int setRegion(const char* Region);
    int setChannelNr(int ChannelNr);
    int setChannelName(const char* ChannelName);
    /******** Getter ********/
    const char* getIcon() const { return this->mIcon; }
    const char* getRegion() const { return this->mRegion; }
    int getChannelNr() const { return this->mChannelNr; }
    const char* getChannelName() const { return this->mChannelName; }
};

class cMediatorInterface {
public:
    virtual ~cMediatorInterface(){};
    virtual cUPnPClassObject* createObject(const char* Title, bool Restricted) = 0;
    virtual cUPnPClassObject* getObject(cUPnPObjectID ID) = 0;
    virtual int saveObject(cUPnPClassObject* Object) = 0;
    virtual int deleteObject(cUPnPClassObject* Object) = 0;
    virtual int clearObject(cUPnPClassObject* Object) = 0;
};

typedef std::map<const char*, cMediatorInterface*, strCmp> tMediatorMap;

class cUPnPObjectFactory {
private:
    static cUPnPObjectFactory*            mInstance;
    cSQLiteDatabase*                      mDatabase;
    tMediatorMap                          mMediators;
    cMediatorInterface* findMediatorByID(cUPnPObjectID ID);
    cMediatorInterface* findMediatorByClass(const char* Class);
    cUPnPObjectFactory();
public:
    static cUPnPObjectFactory* getInstance();
    void registerMediator(const char* UPnPClass, cMediatorInterface* Mediator);
    void unregisterMediator(const char* UPnPClass, bool freeMediator=true);
    cUPnPClassObject* createObject(const char* UPnPClass, const char* Title, bool Restricted=true);
    cUPnPClassObject* getObject(cUPnPObjectID ID);
    int saveObject(cUPnPClassObject* Object);
    int deleteObject(cUPnPClassObject* Object);
    int clearObject(cUPnPClassObject* Object);
};

class cMediaDatabase;

class cUPnPObjectMediator : public cMediatorInterface {
protected:
    cSQLiteDatabase*        mDatabase;
    cMediaDatabase*         mMediaDatabase;
    cUPnPObjectMediator(cMediaDatabase* MediaDatabase);
    virtual int initializeObject(cUPnPClassObject* Object, const char* Class, const char* Title, bool Restricted);
    virtual int objectToDatabase(cUPnPClassObject* Object);
    virtual int databaseToObject(cUPnPClassObject* Object, cUPnPObjectID ID);
public:
    virtual ~cUPnPObjectMediator();
    virtual cUPnPClassObject* createObject(const char* Title, bool Restricted);
    virtual cUPnPClassObject* getObject(cUPnPObjectID);
    virtual int saveObject(cUPnPClassObject* Object);
    virtual int deleteObject(cUPnPClassObject* Object);
    virtual int clearObject(cUPnPClassObject* Object);
};

class cUPnPItemMediator : public cUPnPObjectMediator {
protected:
    virtual int objectToDatabase(cUPnPClassObject* Object);
    virtual int databaseToObject(cUPnPClassObject* Object, cUPnPObjectID ID);
public:
    cUPnPItemMediator(cMediaDatabase* MediaDatabase);
    virtual ~cUPnPItemMediator(){};
    virtual cUPnPClassItem* createObject(const char* Title, bool Restricted);
    virtual cUPnPClassItem* getObject(cUPnPObjectID ID);
};

class cUPnPVideoItemMediator : public cUPnPItemMediator {
protected:
    virtual int objectToDatabase(cUPnPClassObject* Object);
    virtual int databaseToObject(cUPnPClassObject* Object, cUPnPObjectID ID);
public:
    cUPnPVideoItemMediator(cMediaDatabase* MediaDatabase);
    virtual ~cUPnPVideoItemMediator(){};
    virtual cUPnPClassVideoItem* createObject(const char* Title, bool Restricted);
    virtual cUPnPClassVideoItem* getObject(cUPnPObjectID ID);
};

class cUPnPVideoBroadcastMediator : public cUPnPVideoItemMediator {
protected:
    virtual int objectToDatabase(cUPnPClassObject* Object);
    virtual int databaseToObject(cUPnPClassObject* Object, cUPnPObjectID ID);
public:
    cUPnPVideoBroadcastMediator(cMediaDatabase* MediaDatabase);
    virtual ~cUPnPVideoBroadcastMediator(){};
    virtual cUPnPClassVideoBroadcast* createObject(const char* Title, bool Restricted);
    virtual cUPnPClassVideoBroadcast* getObject(cUPnPObjectID ID);
};

class cUPnPContainerMediator : public cUPnPObjectMediator {
protected:
    virtual int objectToDatabase(cUPnPClassObject* Object);
    virtual int databaseToObject(cUPnPClassObject* Object, cUPnPObjectID ID);
public:
    cUPnPContainerMediator(cMediaDatabase* MediaDatabase);
    virtual ~cUPnPContainerMediator(){};
    virtual cUPnPClassContainer* createObject(const char* Title, bool Restricted);
    virtual cUPnPClassContainer* getObject(cUPnPObjectID ID);
};

#endif	/* _OBJECT_H */

