/* 
 * File:   metadata.h
 * Author: savop
 *
 * Created on 28. Mai 2009, 21:14
 */

#ifndef _METADATA_H
#define	_METADATA_H

#include <vdr/tools.h>
#include <vdr/channels.h>
#include <vdr/recording.h>
#include "../common.h"
#include "database.h"
#include "object.h"
#include "resources.h"

struct cUPnPResultSet {
    int mNumberReturned;
    int mTotalMatches;
    const char* mResult;
};

struct cSearchCriteria {
    const char* Property;
    bool Descending;
};

class cMediaDatabase : public cThread {
    friend class cUPnPServer;
    friend class cUPnPObjectMediator;
private:
    unsigned int             mSystemUpdateID;
    cUPnPObjectFactory*      mFactory;
    cHash<cUPnPClassObject>* mObjects;
    cSQLiteDatabase*         mDatabase;
    cUPnPObjectID            mLastInsertObjectID;
    cUPnPObjectID getNextObjectID();
    void cacheObject(cUPnPClassObject* Object);
    int prepareDatabase();
    int loadChannels();
    int loadRecordings();
    void updateChannelEPG();
    void updateRecordings();
    bool init();
    void updateSystemID();
    virtual void Action();
public:
    unsigned int getSystemUpdateID();
    const char*  getContainerUpdateIDs();
    cMediaDatabase();
    virtual ~cMediaDatabase();
    cUPnPClassObject* getObjectByID(cUPnPObjectID ID);
    int browse(OUT cUPnPResultSet** Results, IN const char* ID, IN bool BrowseMetadata, IN const char* Filter = "*", IN unsigned int Offset = 0, IN unsigned int Count = 0, IN const char* SortCriteria = "");
    int search(OUT cUPnPResultSet** Results, IN const char* ID, IN const char* Search, IN const char* Filter = "*", IN unsigned int Offset = 0, IN unsigned int Count = 0, IN const char* SortCriteria = "");
};

#endif	/* _METADATA_H */

