/*
 * mediaReceiverRegistrar.h
 *
 *  Created on: 28.04.2013
 *      Author: savop
 */

#ifndef MEDIARECEIVERREGISTRAR_H_
#define MEDIARECEIVERREGISTRAR_H_

#include "../include/service.h"
#include "../include/tools.h"

namespace upnp {

class cMediaReceiverRegistrar : public cUPnPService {

public:

  cMediaReceiverRegistrar();
  virtual ~cMediaReceiverRegistrar();

  virtual int Subscribe(
    Upnp_Subscription_Request* Request      ///< Information about the subscription
  );

  virtual int Execute(
    Upnp_Action_Request* Request            ///< Input and output parameters of an action
  );

private:

  int IsAuthorized(Upnp_Action_Request* Request);
  int RegisterDevice(Upnp_Action_Request* Request);
  int IsValidated(Upnp_Action_Request* Request);

  uint32_t authorizationGrantedUpdateID;
  uint32_t authorizationDeniedUpdateID;

  int UpdateGrantedUpdateID();
  int UpdateDeniedUpdateID();
  int TriggerNotificationForUpdateIDs();

} MediaReceiverRegistrar;

}  // namespace upnp



#endif /* MEDIARECEIVERREGISTRAR_H_ */
