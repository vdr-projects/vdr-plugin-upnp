/*
 * contentDirectory.h
 *
 *  Created on: 27.08.2012
 *      Author: savop
 */

#ifndef CONTENTDIRECTORY_H_
#define CONTENTDIRECTORY_H_

#include "../include/service.h"
#include "../include/tools.h"
#include <vdr/thread.h>

namespace upnp {

class cContentDirectory : public cUPnPService, public cThread {
public:

  /**
   * Constructor of a Connection manager
   *
   * This creates an instance of a <em>Connection Manager Service</em> and provides
   * interfaces for executing actions and subscribing on events.
   */
  cContentDirectory();
  virtual ~cContentDirectory();
  /*! @copydoc cUpnpService::subscribe(Upnp_Subscription_Request* Request) */
  virtual int Subscribe(Upnp_Subscription_Request* request);
  /*! @copydoc cUpnpService::execute(Upnp_Action_Request* Request) */
  virtual int Execute(Upnp_Action_Request* request);
  /*! @copydoc cUpnpService::setError(Upnp_Action_Request* Request, int Error) */
  virtual void SetError(Upnp_Action_Request* request, int error);

  virtual void Init(cMediaServer* server, UpnpDevice_Handle deviceHandle);
private:
  int GetSearchCapabilities(Upnp_Action_Request* Request);
  int GetSortCapabilities(Upnp_Action_Request* Request);
  int GetSystemUpdateID(Upnp_Action_Request* Request);
  int Browse(Upnp_Action_Request* Request);
  int Search(Upnp_Action_Request* Request);
  int CreateObject(Upnp_Action_Request* Request);
  int DestroyObject(Upnp_Action_Request* Request);
  int UpdateObject(Upnp_Action_Request* Request);
  int DeleteResource(Upnp_Action_Request* Request);
  int CreateReference(Upnp_Action_Request* Request);
  int StopTransferResource(Upnp_Action_Request* Request);
  int GetTransferProgress(Upnp_Action_Request* Request);
  int ExportResource(Upnp_Action_Request* Request);
  int ImportResource(Upnp_Action_Request* Request);

  void Action();

} ContentDirectory;

}  // namespace upnp

#endif /* CONTENTDIRECTORY_H_ */
