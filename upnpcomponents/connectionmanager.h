/* 
 * File:   connectionmanager.h
 * Author: savop
 *
 * Created on 21. August 2009, 18:35
 */

#ifndef _CONNECTIONMANAGER_H
#define	_CONNECTIONMANAGER_H

#include "upnpservice.h"

enum eConnectionStatus {
    OK,
    CONTENT_FORMAT_MISMATCH,
    INSUFFICIENT_BANDWIDTH,
    UNRELIABLE_CHANNEL,
    UNKNOWN
};

enum eDirection {
    OUTPUT,
    INPUT
};

class cVirtualConnection : public cListObject {
    friend class cConnectionManager;
private:
    cString mRemoteProtocolInfo;
    cString mRemoteConnectionManager;
    eDirection  mDirection;
    int         mRemoteConnectionID;
    int         mConnectionID;
    int         mAVTransportID;
    const int   mRcsID;
    eConnectionStatus mStatus;
    cVirtualConnection();
    static const char* getStatusString(eConnectionStatus Status);
    static const char* getDirectionString(eDirection Direction);
    static int getDirection(const char* Direction);
    static int getConnectionStatus(const char* ConnectionStatus);
};

class cConnectionManager : public cUpnpService {
public:
    cConnectionManager(UpnpDevice_Handle DeviceHandle);
    virtual ~cConnectionManager();
    virtual int execute(Upnp_Action_Request* Request);
    virtual int subscribe(Upnp_Subscription_Request* Request);
    bool setProtocolInfo(const char* ProtocolInfo);
private:
    virtual void setError(Upnp_Action_Request* Request, int Error);
    int getProtocolInfo(Upnp_Action_Request* Request);
    int getCurrentConnectionIDs(Upnp_Action_Request* Request);
    int getCurrentConnectionInfo(Upnp_Action_Request* Request);
    int prepareForConnection(Upnp_Action_Request* Request);
    int connectionComplete(Upnp_Action_Request* Request);
    cVirtualConnection* createVirtualConnection(const char* RemoteProtocolInfo = NULL, const char* RemoteConnectionManager = NULL, int RemoteConnectionID = -1, eDirection Direction = OUTPUT);
    bool destroyVirtualConnection(int ConnectionID);
    const char* getConnectionIDsCVS();
    cVirtualConnection* mDefaultConnection;
    cList<cVirtualConnection>* mVirtualConnections;
    cString mSupportedProtocols;
};

#endif	/* _CONNECTIONMANAGER_H */

