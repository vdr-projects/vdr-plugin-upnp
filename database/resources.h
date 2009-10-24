/* 
 * File:   resources.h
 * Author: savop
 *
 * Created on 30. September 2009, 15:17
 */

#ifndef _RESOURCES_H
#define	_RESOURCES_H

#include "database.h"
#include "object.h"
#include <vdr/channels.h>
#include <vdr/recording.h>

class cUPnPResourceMediator;
class cMediaDatabase;

class cUPnPResources {
private:
    cHash<cUPnPResource>*        mResources;
    static cUPnPResources*       mInstance;
    cUPnPResourceMediator*       mMediator;
    cSQLiteDatabase*             mDatabase;
    cUPnPResources();
public:
    int getResourcesOfObject(cUPnPClassObject* Object);
    int loadResources();
    cUPnPResource* getResource(unsigned int ResourceID);
    virtual ~cUPnPResources();
    static cUPnPResources* getInstance();
    int createFromChannel(cUPnPClassVideoBroadcast* Object, cChannel* Channel);
    int createFromRecording(cUPnPClassVideoItem* Object, cRecording* Recording);
    int createFromFile(cUPnPClassItem* Object, cString File);
};

class cUPnPResourceMediator {
    friend class cUPnPResources;
private:
    cSQLiteDatabase* mDatabase;
    cUPnPResourceMediator();
    unsigned int getNextResourceID();
public:
    virtual ~cUPnPResourceMediator();
    cUPnPResource* getResource(unsigned int ResourceID);
    int saveResource(cUPnPResource* Resource);
    cUPnPResource* newResource(cUPnPClassObject* Object, int ResourceType, cString ResourceFile, cString ContentType, cString ProtocolInfo);
};

#endif	/* _RESOURCES_H */

