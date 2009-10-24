/* 
 * File:   contentdirectory.h
 * Author: savop
 *
 * Created on 21. August 2009, 16:12
 */

#ifndef _CONTENTDIRECTORY_H
#define	_CONTENTDIRECTORY_H

#include <upnp/upnp.h>
#include "upnpservice.h"
#include "../database/metadata.h"

class cContentDirectory : public cUpnpService, public cThread {
public:
    cContentDirectory(UpnpDevice_Handle DeviceHandle, cMediaDatabase* MediaDatabase);
    virtual ~cContentDirectory();
    virtual int subscribe(Upnp_Subscription_Request* Request);
    virtual int execute(Upnp_Action_Request* Request);
    virtual void setError(Upnp_Action_Request* Request, int Error);
private:
    cMediaDatabase*             mMediaDatabase;
    void Action();
    int getSearchCapabilities(Upnp_Action_Request* Request);
    int getSortCapabilities(Upnp_Action_Request* Request);
    int getSystemUpdateID(Upnp_Action_Request* Request);
    int browse(Upnp_Action_Request* Request);
//    int search(Upnp_Action_Request* Request);
//    int createObject(Upnp_Action_Request* Request);
//    int destroyObject(Upnp_Action_Request* Request);
//    int updateObject(Upnp_Action_Request* Request);
//    int deleteResource(Upnp_Action_Request* Request);
//    int createReference(Upnp_Action_Request* Request);
};

#endif	/* _CONTENTDIRECTORY_H */

