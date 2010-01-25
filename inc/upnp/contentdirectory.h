/* 
 * File:   contentdirectory.h
 * Author: savop
 *
 * Created on 21. August 2009, 16:12
 */

#ifndef _CONTENTDIRECTORY_H
#define	_CONTENTDIRECTORY_H

#include <upnp/upnp.h>
#include "service.h"
#include "metadata.h"

/**
 * The content directory service
 *
 * This is the content directory service which handles all incoming requests
 * for contents managed by the media server.
 */
class cContentDirectory : public cUpnpService, public cThread {
public:
    /**
     * Constructor of a Content Directory
     *
     * This creates an instance of a <em>Content Directory Service</em> and provides
     * interfaces for executing actions and subscribing on events.
     */
    cContentDirectory(
        UpnpDevice_Handle DeviceHandle,     ///< The UPnP device handle of the root device
        cMediaDatabase* MediaDatabase       ///< the media database where requests are processed
    );
    virtual ~cContentDirectory();
    /*! @copydoc cUpnpService::subscribe(Upnp_Subscription_Request* Request) */
    virtual int subscribe(Upnp_Subscription_Request* Request);
    /*! @copydoc cUpnpService::execute(Upnp_Action_Request* Request) */
    virtual int execute(Upnp_Action_Request* Request);
    /*! @copydoc cUpnpService::setError(Upnp_Action_Request* Request, int Error) */
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

