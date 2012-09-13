/*
 * connectionManager.cpp
 *
 *  Created on: 27.08.2012
 *      Author: savop
 */

#include "../include/connectionManager.h"
#include "../include/tools.h"
#include "../include/server.h"
#include "../include/media/mediaManager.h"
#include <vdr/i18n.h>
#include <upnp/upnptools.h>
#include <string>
#include <sstream>

namespace upnp {

#define UPNP_CMS_ACTION_GETPROTOCOLINFO "GetProtocolInfo"
#define UPNP_CMS_ACTION_GETCURRENTCONNECTIONIDS "GetCurrentConnectionIDs"
#define UPNP_CMS_ACTION_GETCURRENTCONNECTIONINFO "GetCurrentConnectionInfo"
#define UPNP_CMS_ACTION_PREPAREFORCONNECTION "PrepareForConnection"
#define UPNP_CMS_ACTION_CONNECTIONCOMPLETE "ConnectionComplete"

cConnectionManager::cConnectionManager()
: cUPnPService(
    cUPnPService::Description(
        "urn:schemas-upnp-org:service:ConnectionManager:1",
        "urn:upnp-org:serviceId:ConnectionManager",
        "cms_scpd.xml",
        "cms_control",
        "cms_event"
        ))
{

  cVirtualConnection* connection = cVirtualConnection::GenerateVirtualConnection(std::string(), std::string(),
                                                                                -1, cVirtualConnection::VC_OUTPUT);
  mVirtualConnections[connection->GetConnectionID()] = connection;
}

cConnectionManager::~cConnectionManager(){
  DeleteConnections();
}

int cConnectionManager::Subscribe(Upnp_Subscription_Request* request){
  IXML_Document* PropertySet = NULL;

  std::string protocolInfo = tools::StringListToCSV(mMediaServer->GetManager().GetSupportedProtocolInfos());

  /* The protocol infos which this server supports */
  UpnpAddToPropertySet(&PropertySet, "SourceProtocolInfo", protocolInfo.c_str());
  /* Not set, this field is only used by Media Renderers */
  UpnpAddToPropertySet(&PropertySet, "SinkProtocolInfo", "");
  /* The current connection IDs of all virtual connections */
  UpnpAddToPropertySet(&PropertySet, "CurrentConnectionIDs", GetConnectionIDsCVS().c_str());
  // Accept subscription
  int ret = UpnpAcceptSubscriptionExt(this->mDeviceHandle, request->UDN, request->ServiceId, PropertySet, request->Sid);

  if(ret != UPNP_E_SUCCESS){
    esyslog("UPnP\tSubscription failed (Error code: %d)", ret);
  }

  ixmlDocument_free(PropertySet);
  return ret;
}

int cConnectionManager::Execute(Upnp_Action_Request* request){
  if (request == NULL) {
    esyslog("UPnP\tCMS Action Handler - request is null");
    return UPNP_E_BAD_REQUEST;
  }

  if(!strcmp(request->ActionName, UPNP_CMS_ACTION_GETPROTOCOLINFO))
    return this->GetProtocolInfo(request);
  if(!strcmp(request->ActionName, UPNP_CMS_ACTION_GETCURRENTCONNECTIONIDS))
    return this->GetCurrentConnectionIDs(request);
  if(!strcmp(request->ActionName, UPNP_CMS_ACTION_GETCURRENTCONNECTIONINFO))
    return this->GetCurrentConnectionInfo(request);
  if(!strcmp(request->ActionName, UPNP_CMS_ACTION_PREPAREFORCONNECTION))
    return this->PrepareForConnection(request);
  if(!strcmp(request->ActionName, UPNP_CMS_ACTION_CONNECTIONCOMPLETE))
    return this->ConnectionComplete(request);

  return UPNP_E_BAD_REQUEST;
}

int cConnectionManager::GetCurrentConnectionIDs(Upnp_Action_Request* request){
  std::string IDs = this->GetConnectionIDsCVS();
  if(IDs.empty()){
    SetError(request, UPNP_E_INTERNAL_ERROR);
    return request->ErrCode;
  }

  std::stringstream ss;

  ss << "<u:" << request->ActionName << "Response xmlns:u=\"" << GetServiceDescription().serviceType << "\">"
     << "  <ConnectionIDs>" << IDs << "</ConnectionIDs>"
     << "</u:" << request->ActionName << "Response>";

  request->ActionResult = ixmlParseBuffer(ss.str().c_str());
  request->ErrCode = UPNP_E_SUCCESS;
  return request->ErrCode;
}

int cConnectionManager::GetCurrentConnectionInfo(Upnp_Action_Request* request){

  long id;
  if(ParseIntegerValue(request->ActionRequest, "ConnectionID", id) != UPNP_E_SUCCESS){
    esyslog("UPnP\tInvalid arguments. ConnectionID missing or wrong");
    SetError(request, UPNP_SOAP_E_INVALID_ARGS);
    return request->ErrCode;
  }
  int32_t connectionID = id;

  cVirtualConnection* connection = mVirtualConnections[connectionID];

  if(connection == NULL){
    esyslog("UPnP\tNo valid connection found with given ID=%d!", connectionID);
    SetError(request, UPNP_CMS_E_INVALID_CONNECTION_REFERENCE);
    return request->ErrCode;
  }

  std::stringstream ss;
  ss << "<u:" << request->ActionName << "Response xmlns:u=\"" << GetServiceDescription().serviceType << "\">"
     << "  <ProtocolInfo>" << connection->GetRemoteProtocolInfo() << "</ProtocolInfo>"
     << "  <PeerConnectionManager>" << connection->GetPeerConnectionManager() << "</PeerConnectionManager>"
     << "  <PeerConnectionID>" << connection->GetPeerConnectionID() << "</PeerConnectionID>"
     << "  <Direction>" << connection->GetDirectionString() << "</Direction>"
     << "  <RcsID>" << connection->GetRcsID() << "</RcsID>"
     << "  <AVTransportID>" << connection->GetAVTransportID() << "</AVTransportID>"
     << "  <Status>" << connection->GetStatusString() << "</Status>"
     << "</u:" << request->ActionName << "Response>";

  request->ActionResult = ixmlParseBuffer(ss.str().c_str());
  request->ErrCode = UPNP_E_SUCCESS;
  return request->ErrCode;
}

int cConnectionManager::GetProtocolInfo(Upnp_Action_Request* request){
  std::stringstream ss;

  std::string protocolInfo = tools::StringListToCSV(mMediaServer->GetManager().GetSupportedProtocolInfos());

  ss << "<u:" << request->ActionName << "Response xmlns:u=\"" << GetServiceDescription().serviceType << "\">"
     << "  <Source>" << protocolInfo.c_str() << "</Source>"
     << "  <Sink></Sink>"
     << "</u:" << request->ActionName << "Response>";

  request->ActionResult = ixmlParseBuffer(ss.str().c_str());
  request->ErrCode = UPNP_E_SUCCESS;
  return request->ErrCode;
}

int cConnectionManager::ConnectionComplete(Upnp_Action_Request* request){
  SetError(request, UPNP_SOAP_E_ACTION_NOT_IMPLEMENTED);
  return request->ErrCode;
}

int cConnectionManager::PrepareForConnection(Upnp_Action_Request* request){
  SetError(request, UPNP_SOAP_E_ACTION_NOT_IMPLEMENTED);
  return request->ErrCode;
}

const std::string cConnectionManager::GetConnectionIDsCVS(){
  std::stringstream ss;

  ConnectionList::iterator it = mVirtualConnections.begin();
  ss << (*it).second->GetConnectionID();
  for(++it; it != mVirtualConnections.end(); ++it){
    ss << "," << (*it).second->GetConnectionID();
  }

  return ss.str();
}

void cConnectionManager::DeleteConnections(){
  for(ConnectionList::iterator it = mVirtualConnections.begin(); it != mVirtualConnections.end();++it){
    cVirtualConnection::DestroyVirtualConnection((*it).second);
  }
  mVirtualConnections.clear();
}

bool cConnectionManager::DeleteConnection(int32_t connectionID){
  if(connectionID == 0){
    esyslog("UPnP\tCannot delete default connection with connectionID = 0");
    return false;
  }

  cVirtualConnection::DestroyVirtualConnection(mVirtualConnections[connectionID]);
  mVirtualConnections.erase(connectionID);

  return OnConnectionChange();
}

bool cConnectionManager::CreateConnection(const std::string & remoteProtocolInfo,
    const std::string & peerConnectionManager,
    int32_t peerConnectionID,
    const std::string & direction)
{
  cVirtualConnection* connection = cVirtualConnection::GenerateVirtualConnection(
      remoteProtocolInfo, peerConnectionManager,
      peerConnectionID, direction);

  mVirtualConnections[connection->GetConnectionID()] = connection;

  return OnConnectionChange();
}

bool cConnectionManager::OnConnectionChange(){
  IXML_Document* PropertySet = NULL;
  UpnpAddToPropertySet(&PropertySet, "CurrentConnectionIDs", GetConnectionIDsCVS().c_str());
  int ret = UpnpNotifyExt(mDeviceHandle, mMediaServer->GetDeviceUUID().c_str(), mServiceDescription.serviceID.c_str(), PropertySet);
  ixmlDocument_free(PropertySet);

  if(ret != UPNP_E_SUCCESS){
      esyslog("UPnP\tState change notification failed (Error code: %d)",ret);
      return false;
  }

  return true;
}

void cConnectionManager::SetError(Upnp_Action_Request* request, int error){
  request->ErrCode = error;
  switch(error){
  case UPNP_CMS_E_INCOMPATIBLE_PROTOCOL_INFO:
    strn0cpy(request->ErrStr,tr("Incompatible protocol info"),LINE_SIZE);
    break;
  case UPNP_CMS_E_INCOMPATIBLE_DIRECTIONS:
    strn0cpy(request->ErrStr,tr("Incompatible directions"),LINE_SIZE);
    break;
  case UPNP_CMS_E_INSUFFICIENT_RESOURCES:
    strn0cpy(request->ErrStr,tr("Insufficient network resources"),LINE_SIZE);
    break;
  case UPNP_CMS_E_LOCAL_RESTRICTIONS:
    strn0cpy(request->ErrStr,tr("Local restrictions"),LINE_SIZE);
    break;
  case UPNP_CMS_E_ACCESS_DENIED:
    strn0cpy(request->ErrStr,tr("Access denied"),LINE_SIZE);
    break;
  case UPNP_CMS_E_INVALID_CONNECTION_REFERENCE:
    strn0cpy(request->ErrStr,tr("Invalid connection reference"),LINE_SIZE);
    break;
  case UPNP_CMS_E_NOT_IN_NETWORK:
    strn0cpy(request->ErrStr,tr("Not in network"),LINE_SIZE);
    break;
  default:
    cUPnPService::SetError(request, error);
    break;
  }
}

}  // namespace upnp
