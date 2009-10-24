/* 
 * File:   upnpservice.h
 * Author: savop
 *
 * Created on 21. August 2009, 18:38
 */

#ifndef _UPNPSERVICE_H
#define	_UPNPSERVICE_H

#include <upnp/upnp.h>

class cUpnpService {
public:
    cUpnpService(UpnpDevice_Handle DeviceHandle);
    virtual ~cUpnpService(){};
    virtual int subscribe(Upnp_Subscription_Request* Request) = 0;
    virtual int execute(Upnp_Action_Request* Request) = 0;
protected:
    virtual void setError(Upnp_Action_Request* Request, int Error);
    int parseIntegerValue(IN IXML_Document* Document, IN const char* Item, OUT int* Value);
    int parseStringValue(IN IXML_Document* Document, IN const char* Item, OUT char** Value);
    UpnpDevice_Handle mDeviceHandle;
};

#endif	/* _UPNPSERVICE_H */

