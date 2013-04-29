/*
 * mediaReceiverRegistrar.cpp
 *
 *  Created on: 28.04.2013
 *      Author: savop
 */

#include "../include/server.h"
#include "../include/mediaReceiverRegistrar.h"
#include <sstream>
#include <time.h>

namespace upnp {

#define X_MS_MRR_ACTION_ISAUTHORIZED    "IsAuthorized"
#define X_MS_MRR_ACTION_ISVALIDATED     "IsValidated"
#define X_MS_MRR_ACTION_REGISTERDEVICE  "RegisterDevice"

cMediaReceiverRegistrar::cMediaReceiverRegistrar()
: cUPnPService(
    cUPnPService::Description(
            "urn:microsoft.com:service:X_MS_MediaReceiverRegistrar:1",
            "urn:microsoft.com:serviceId:X_MS_MediaReceiverRegistrar",
            "x_mrr_scpd.xml",
            "x_mrr_control",
            "x_mrr_event"
            )),
            authorizationGrantedUpdateID((uint32_t)time(NULL)),
            authorizationDeniedUpdateID((uint32_t)time(NULL))
{
}

cMediaReceiverRegistrar::~cMediaReceiverRegistrar(){

}

int cMediaReceiverRegistrar::Subscribe(Upnp_Subscription_Request* request){
  IXML_Document* PropertySet = NULL;

  /* UpdateID for granted authorizations */
  UpnpAddToPropertySet(&PropertySet, "AuthorizationGrantedUpdateID", tools::ToString(authorizationGrantedUpdateID).c_str());
  /* UpdateID for denied authorizations */
  UpnpAddToPropertySet(&PropertySet, "AuthorizationDeniedUpdateID", tools::ToString(authorizationDeniedUpdateID).c_str());
  // Accept subscription
  int ret = UpnpAcceptSubscriptionExt(this->mDeviceHandle, request->UDN, request->ServiceId, PropertySet, request->Sid);

  if(ret != UPNP_E_SUCCESS){
    esyslog("UPnP\tSubscription failed (Error code: %d)", ret);
  }

  ixmlDocument_free(PropertySet);
  return ret;
}

int cMediaReceiverRegistrar::Execute(Upnp_Action_Request* request){
  if (request == NULL) {
    esyslog("UPnP\tCMS Action Handler - request is null");
    return UPNP_E_BAD_REQUEST;
  }

  if(!strcmp(request->ActionName, X_MS_MRR_ACTION_ISAUTHORIZED))
    return this->IsAuthorized(request);
  if(!strcmp(request->ActionName, X_MS_MRR_ACTION_ISVALIDATED))
    return this->IsValidated(request);
  if(!strcmp(request->ActionName, X_MS_MRR_ACTION_REGISTERDEVICE))
    return this->RegisterDevice(request);

  return UPNP_E_BAD_REQUEST;
}

int cMediaReceiverRegistrar::UpdateDeniedUpdateID(){
  authorizationDeniedUpdateID = (uint32_t)time(NULL);

  return TriggerNotificationForUpdateIDs();
}

int cMediaReceiverRegistrar::UpdateGrantedUpdateID(){
  authorizationGrantedUpdateID = (uint32_t)time(NULL);

  return TriggerNotificationForUpdateIDs();
}

int cMediaReceiverRegistrar::TriggerNotificationForUpdateIDs(){
  IXML_Document* PropertySet = NULL;

  /* UpdateID for granted authorizations */
  UpnpAddToPropertySet(&PropertySet, "AuthorizationGrantedUpdateID", tools::ToString(authorizationGrantedUpdateID).c_str());
  /* UpdateID for denied authorizations */
  UpnpAddToPropertySet(&PropertySet, "AuthorizationDeniedUpdateID", tools::ToString(authorizationDeniedUpdateID).c_str());
  // Trigger notification
  int ret = UpnpNotifyExt(this->mDeviceHandle, this->mMediaServer->GetDeviceUUID().c_str(),
                          this->mServiceDescription.serviceID.c_str(), PropertySet);

  if(ret != UPNP_E_SUCCESS){
    esyslog("UPnP\tNotification failed (Error code: %d)", ret);
  }

  ixmlDocument_free(PropertySet);
  return ret;
}

int cMediaReceiverRegistrar::IsAuthorized(Upnp_Action_Request* request){
  std::stringstream ss;

  ss << "<u:" << request->ActionName << "Response xmlns:u=\"" << GetServiceDescription().serviceType << "\">"
     << "  <Result>" << 1 << "</Result>"
     << "</u:" << request->ActionName << "Response>";

  request->ActionResult = ixmlParseBuffer(ss.str().c_str());
  request->ErrCode = UPNP_E_SUCCESS;

  return request->ErrCode;
}

int cMediaReceiverRegistrar::RegisterDevice(Upnp_Action_Request* request){
  SetError(request, UPNP_SOAP_E_ACTION_NOT_IMPLEMENTED);
  return request->ErrCode;
}

int cMediaReceiverRegistrar::IsValidated(Upnp_Action_Request* request){
  SetError(request, UPNP_SOAP_E_ACTION_NOT_IMPLEMENTED);
  return request->ErrCode;
}

}  // namespace upnp
