/*
 * connectionManager.h
 *
 *  Created on: 27.08.2012
 *      Author: savop
 */

#ifndef CONNECTIONMANAGER_H_
#define CONNECTIONMANAGER_H_

#include "../include/service.h"
#include "../include/connection.h"
#include "../include/tools.h"
#include <string>
#include <list>
#include <map>

namespace upnp {

/**
 * The connection manager service
 *
 * This is the connection manager service which handles all incoming connection,
 * creates and destroys connections to clients.
 */
class cConnectionManager : public cUPnPService {
public:

  typedef std::map<int32_t, cVirtualConnection*> ConnectionList;
  /**
   * Constructor of a Connection manager
   *
   * This creates an instance of a <em>Connection Manager Service</em> and provides
   * interfaces for executing actions and subscribing on events.
   */
  cConnectionManager();
  virtual ~cConnectionManager();
  /*! @copydoc cUpnpService::subscribe(Upnp_Subscription_Request* Request) */
  virtual int Subscribe(Upnp_Subscription_Request* request);
  /*! @copydoc cUpnpService::execute(Upnp_Action_Request* Request) */
  virtual int Execute(Upnp_Action_Request* request);
  /*! @copydoc cUpnpService::setError(Upnp_Action_Request* Request, int Error) */
  virtual void SetError(Upnp_Action_Request* request, int error);
private:

  int GetProtocolInfo(Upnp_Action_Request* request);
  int GetCurrentConnectionIDs(Upnp_Action_Request* request);
  int GetCurrentConnectionInfo(Upnp_Action_Request* request);
  int PrepareForConnection(Upnp_Action_Request* request);
  int ConnectionComplete(Upnp_Action_Request* request);
  const std::string GetConnectionIDsCVS();

  void DeleteConnections();
  bool DeleteConnection(int32_t connectionID);
  bool CreateConnection(
      const std::string & remoteProtocolInfo,
      const std::string & peerConnectionManager,
      int32_t peerConnectionID,
      const std::string & direction
      );

  bool OnConnectionChange();

  ConnectionList mVirtualConnections;
  StringList mSupportedProtocols;
} ConnectionManager;

}  // namespace upnp

#endif /* CONNECTIONMANAGER_H_ */
