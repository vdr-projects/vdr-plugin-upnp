/*
 * server.cpp
 *
 *  Created on: 31.07.2012
 *      Author: savop
 */

#include <vdr/tools.h>
#include <string>
#include <sstream>
#include "../include/server.h"
#include "../include/service.h"
#include "../include/webserver.h"
#include "../include/tools.h"
#include "../include/media/mediaManager.h"
#include "../upnp.h"

using namespace upnp;
using namespace std;

cMediaServer* cMediaServer::GetInstance(){
	static cMediaServer server;
	return &server;
}

cMediaServer::cMediaServer()
: mServerDescription("VDR UPnP/DLNA MS", "Denis Loh", "http://upnp.vdr-developer.org",
                     DESCRIPTION, "VDR UPnP-DLNA MS", VERSION,
                     "http://projects.vdr-developer.org/projects/plg-upnp/files", VERSION,
                     "deviceDescription.xml")
, mDeviceHandle(0)
, mAnnounceMaxAge(1800)
, mMaxContentLength(KB(20))
, mWebserver(NULL)
, mMediaManager(NULL)
{
  mServerIcons.push_back(ServerIcon(image::DLNA_ICON_PNG_SM_24A, "images/upnpIconSm.png"));
  mServerIcons.push_back(ServerIcon(image::DLNA_ICON_PNG_LRG_24A, "images/upnpIconLrg.png"));
  mServerIcons.push_back(ServerIcon(image::DLNA_ICON_JPEG_SM_24, "images/upnpIconSm.jpeg"));
  mServerIcons.push_back(ServerIcon(image::DLNA_ICON_JPEG_LRG_24, "images/upnpIconLrg.jpeg"));
}

cMediaServer::~cMediaServer(){
  delete mWebserver;
  delete mMediaManager;
}

bool cMediaServer::Start(){

  isyslog("UPnP\tStarting UPnP media server");

  int ret;

  // Disable internal webserver, we don't need it.
  UpnpEnableWebserver(false);

  if(!mWebserver->Start()){
    esyslog("UPnP\tFailed to start the web server");
    return false;
  }

  isyslog("UPnP\tRegistering UPnP media server");

  string description = GetDeviceDescriptionUrl();

  ret = UpnpRegisterRootDevice2(UPNPREG_URL_DESC,
                                description.c_str(),
                                description.size(),
                                0,
                                &cMediaServer::ActionCallback,
                                this,
                                &mDeviceHandle);

  if(ret != UPNP_E_SUCCESS){
    esyslog("UPnP\tFailed to register the device. Error code: %d", ret);
    return false;
  }

  ret = UpnpUnRegisterRootDevice(mDeviceHandle);
  if (ret != UPNP_E_SUCCESS) {
    esyslog("UPnP\tUnregistering old devices failed");
    return false;
  }

  ret = UpnpRegisterRootDevice2(UPNPREG_URL_DESC,
                                description.c_str(),
                                description.size(),
                                0,
                                &cMediaServer::ActionCallback,
                                this,
                                &mDeviceHandle);

  if(ret != UPNP_E_SUCCESS){
    esyslog("UPnP\tFailed to register the device. Error code: %d", ret);
    return false;
  }

  isyslog("UPnP\tInitialising services...");
  serviceMap&  services = cMediaServer::GetServices();
  for(serviceMap::iterator it = services.begin(); it != services.end(); ++it){
    isyslog("UPnP\t...%s", (*it).second->GetServiceDescription().serviceType.c_str());
    (*it).second->Init(this, mDeviceHandle);
  }

  //send first advertisments
  isyslog("UPnP\tSend first advertisements to publish start in network");
  ret = UpnpSendAdvertisement(mDeviceHandle, GetAnnounceMaxAge());
  if (ret != UPNP_E_SUCCESS) {
    esyslog("UPnP\tError while sending first advertisments - Errorcode: %d", ret);
    return false;
  }

  return true;
}

bool cMediaServer::Stop(){

  int ret = 0;

  isyslog("UPnP\tStopping services...");
  serviceMap&  services = cMediaServer::GetServices();
  for(serviceMap::iterator it = services.begin(); it != services.end(); ++it){
    isyslog("UPnP\t...%s", (*it).second->GetServiceDescription().serviceType.c_str());
    (*it).second->Stop();
  }

  isyslog("UPnP\tStopping UPnP media server");
  UpnpUnRegisterRootDevice(mDeviceHandle);
  if (ret != UPNP_E_SUCCESS) {
    esyslog("UPnP\tError while sending first advertisments - Errorcode: %d", ret);
    return false;
  }

  UpnpFinish();

  isyslog("UPnP\tStopping web server...");
  if(mWebserver){
    mWebserver->Stop();

    delete mWebserver;
    mWebserver = NULL;
  }

  if(mMediaManager){
    delete mMediaManager;
    mMediaManager = NULL;
  }

  return true;
}

bool cMediaServer::Initialize(){
  string address;
  uint16_t port = 0;

  if(mCurrentConfiguration.expertSettings){
    address = mCurrentConfiguration.bindToAddress
              ? mCurrentConfiguration.address
              : tools::GetAddressByInterface(mCurrentConfiguration.interface);

    if(address.empty() || !address.compare("0.0.0.0")){
      address = tools::GetAddressByInterface(tools::GetNetworkInterfaceByIndex(0, true));
    }

    port = mCurrentConfiguration.port;
  } else {
    address = tools::GetAddressByInterface(tools::GetNetworkInterfaceByIndex(0, true));
    port = 0;
  }

  int ret = 0;

  isyslog("UPnP\tInitializing UPnP media server on %s:%d", address.c_str(), port);

  ret = UpnpInit(address.c_str(), mCurrentConfiguration.port);

  if(ret != UPNP_E_SUCCESS && ret != UPNP_E_INIT){
    esyslog("UPnP\tFailed to initialise UPnP media server. Error code: %d", ret);
    return false;
  }

  mWebserver = new cWebserver(GetServerIPAddress());
  mMediaManager = new cMediaManager();

  if(mCurrentConfiguration.expertSettings){

    if(mCurrentConfiguration.maxContentLength)
      SetMaxContentLength(mCurrentConfiguration.maxContentLength);

    if(mCurrentConfiguration.announceMaxAge)
      SetAnnounceMaxAge(mCurrentConfiguration.announceMaxAge);

    if(!mCurrentConfiguration.webServerRoot.empty())
      mWebserver->SetWebserverRootDir(mCurrentConfiguration.webServerRoot);

    if(!mCurrentConfiguration.useLive){
      if(!mCurrentConfiguration.presentationURL.empty())
        mWebserver->SetPresentationUrl(mCurrentConfiguration.presentationURL);
    } else {
      stringstream ss;

      uint16_t port = mCurrentConfiguration.livePort ? mCurrentConfiguration.livePort : 8008;

      ss << "http://" << GetServerIPAddress() << ":" << port << "/";

      mWebserver->SetPresentationUrl(ss.str());
    }

    if(mCurrentConfiguration.webServerPort)
      mWebserver->SetListenerPort(mCurrentConfiguration.webServerPort);

    if(!mCurrentConfiguration.databaseDir.empty())
      mMediaManager->SetDatabaseDir(mCurrentConfiguration.databaseDir);
  }

  ret = UpnpSetMaxContentLength(GetMaxContentLength());

  if(ret != UPNP_E_SUCCESS){
    esyslog("UPnP\tFailed to set max. content length of SOAP messages. Error code: %d", ret);
    return false;
  }

  isyslog("UPnP\tInitialising webserver");
  if(!mWebserver->Initialise()){
    esyslog("UPnP\tFailed to initialise the web server.");
    return false;
  }

  isyslog("UPnP\tInitialising media manager");
  if(!mMediaManager->Initialise()){
    esyslog("UPnP\tFailed to initialise the media manager.");
    return false;
  }

  return true;
}

void cMediaServer::SetAnnounceMaxAge(int announceMaxAge){
  mAnnounceMaxAge = (announceMaxAge != 0) ? announceMaxAge : 1800;
}

void cMediaServer::SetMaxContentLength(size_t maxContentLength){
  mMaxContentLength = (maxContentLength != 0) ? maxContentLength : KB(20);
}

void cMediaServer::SetConfiguration(upnp::cConfig newConfig){
  mCurrentConfiguration = newConfig;
}

upnp::cConfig cMediaServer::GetConfiguration() const {
  return mCurrentConfiguration;
}

const char* cMediaServer::GetServerIPAddress() const {
  return UpnpGetServerIpAddress();
}

uint16_t cMediaServer::GetServerPort() const {
  return UpnpGetServerPort();
}

string cMediaServer::GetDeviceDescriptionUrl() const {
  return mWebserver->GetServiceUrl() + mServerDescription.descriptionFile;
}

cMediaServer::serviceMap& cMediaServer::GetServices(){
  static serviceMap services;
  return services;
}

void cMediaServer::RegisterService(cUPnPService* service){
  if(service != NULL){
    dsyslog("UPnP\tRegistered service: %s", service->GetServiceDescription().serviceType.c_str());
    GetServices()[service->GetServiceDescription().serviceID] = service;
  }
}

int cMediaServer::ActionCallback(Upnp_EventType eventtype, void *event, void *cookie){
  Upnp_Subscription_Request* eventRequest = NULL;
  Upnp_Action_Request*       actionRequest = NULL;

  cMediaServer* mediaServer = (cMediaServer*)cookie;

  //check committed event variable
  if (event == NULL) {
      esyslog("UPnP\tUPnP Callback - NULL request");
      return UPNP_E_BAD_REQUEST;
  }

  cUPnPService* service;

  switch(eventtype){
  case UPNP_CONTROL_ACTION_REQUEST:
    actionRequest = (Upnp_Action_Request*) event;

    dsyslog("UPnP\tAction request: %s", actionRequest->ActionName);

    if(!mediaServer->CheckDeviceUUID(actionRequest->DevUDN)){
      esyslog("UPnP\tUPnP Callback - action request not for this device");
      return UPNP_E_BAD_REQUEST;
    }

    service = cMediaServer::GetServices()[actionRequest->ServiceID];

    if(service == NULL){
      esyslog("UPnP\tCallback - unsupported service called for control");
      return UPNP_E_BAD_REQUEST;
    }

    return service->Execute(actionRequest);

  case UPNP_EVENT_SUBSCRIPTION_REQUEST:
    eventRequest = (Upnp_Subscription_Request*) event;

    dsyslog("UPnP\tSubscription request from: %s", eventRequest->ServiceId);

    if(!mediaServer->CheckDeviceUUID(eventRequest->UDN)){
      esyslog("UPnP\tUPnP Callback - event request not for this device");
      return UPNP_E_BAD_REQUEST;
    }

    service = cMediaServer::GetServices()[eventRequest->ServiceId];

    if(service == NULL){
      esyslog("UPnP\tCallback - unsupported service called for eventing");
      return UPNP_E_BAD_REQUEST;
    }

    return service->Subscribe(eventRequest);

  default:
    esyslog("UPnP\tUPnP Action Callback - Unsupported Event");
    return UPNP_E_BAD_REQUEST;
  }

}

bool cMediaServer::CheckDeviceUUID(string deviceUUID) const {
  return deviceUUID.find(mCurrentConfiguration.deviceUUID) != string::npos;
}

cMediaServer::Description::Description(
    string fn, string m, string murl,
    string mod, string mon, string mono,
    string mourl, string sno, string desc)
: friendlyName(fn)
, manufacturer(m)
, manufacturerURL(murl)
, modelDescription(mod)
, modelName(mon)
, modelNumber(mono)
, modelURL(mourl)
, serialNumber(sno)
, descriptionFile(desc)
{
}

cMediaServer::ServerIcon::ServerIcon(image::cIcon profile, string filename)
: profile(profile)
, filename(filename)
{
}
