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
#include "../include/tools.h"
#include "../include/media/mediaManager.h"
#include "../upnp.h"

using namespace upnp;
using namespace std;

cMediaServer* cMediaServer::GetInstance(){
	static cMediaServer server;
	return &server;
}

cMediaServer::serviceMap cMediaServer::mServices;

cMediaServer::cMediaServer()
: mServerDescription("VDR UPnP/DLNA MS", "Denis Loh", "http://upnp.vdr-developer.org",
                     DESCRIPTION, "VDR UPnP-DLNA MS", VERSION,
                     "http://projects.vdr-developer.org/projects/plg-upnp/files", VERSION)
, mDeviceHandle(0)
, mAnnounceMaxAge(1800)
, mMaxContentLength(KB(20))
, mIsRunning(false)
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

bool cMediaServer::IsRunning() const {
  return mIsRunning;
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
  for(serviceMap::iterator it = cMediaServer::mServices.begin(); it != cMediaServer::mServices.end(); ++it){
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



  mIsRunning = true;

  return IsRunning();
}

bool cMediaServer::Stop(){

  isyslog("UPnP\tStopping UPnP media server");

  int ret = 0;

  UpnpUnRegisterRootDevice(mDeviceHandle);
  if (ret != UPNP_E_SUCCESS) {
    esyslog("UPnP\tError while sending first advertisments - Errorcode: %d", ret);
    return false;
  }

  UpnpFinish();

  if(mWebserver){
    mWebserver->Stop();

    delete mWebserver;
    mWebserver = NULL;
  }

  if(mMediaManager){
    delete mMediaManager;
    mMediaManager = NULL;
  }

  mIsRunning = false;

  return !IsRunning();
}

bool cMediaServer::Initialize(){
  isyslog("UPnP\tInitializing UPnP media server");

  string address = mCurrentConfiguration.bindToAddress
                   ? mCurrentConfiguration.address
                   : tools::GetAddressByInterface(mCurrentConfiguration.interface);

  if(!address.compare("0.0.0.0"))
    address = tools::GetAddressByInterface(tools::GetNetworkInterfaceByIndex(0, true));

  int ret = 0;

  ret = UpnpInit(address.c_str(), mCurrentConfiguration.port);

  if(ret != UPNP_E_SUCCESS && ret != UPNP_E_INIT){
    esyslog("UPnP\tFailed to initialise UPnP media server. Error code: %d", ret);
    return false;
  }

  mWebserver = new cWebserver(GetServerIPAddress());

  if(mCurrentConfiguration.expertSettings){

    if(mCurrentConfiguration.maxContentLength)
      SetMaxContentLength(mCurrentConfiguration.maxContentLength);

    if(mCurrentConfiguration.announceMaxAge)
      SetAnnounceMaxAge(mCurrentConfiguration.announceMaxAge);

    if(!mCurrentConfiguration.webServerRoot.empty())
      mWebserver->SetWebserverRootDir(mCurrentConfiguration.webServerRoot,
                                      mCurrentConfiguration.staticContentURL,
                                      mCurrentConfiguration.presentationURL);

    if(mCurrentConfiguration.webServerPort)
      mWebserver->SetListenerPort(mCurrentConfiguration.webServerPort);
  }

  mMediaManager = new cMediaManager();

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
  return mWebserver->GetBaseUrl() + mCurrentConfiguration.serviceURL + "deviceDescription.xml";
}

void cMediaServer::RegisterService(cUPnPService* service){
  if(service != NULL){
    cout << "Registered service: " << service->GetServiceDescription().serviceType << endl;
    mServices[service->GetServiceDescription().serviceID] = service;
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

    service = cMediaServer::mServices[actionRequest->ServiceID];

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

    service = cMediaServer::mServices[eventRequest->ServiceId];

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

const char* cMediaServer::RuntimeException::what() const throw() {
  return "Runtime error: media server is not running";
}

bool cMediaServer::CheckDeviceUUID(string deviceUUID) const {
  return deviceUUID.find(mCurrentConfiguration.deviceUUID) != string::npos;
}

cMediaServer::Description::Description(
    string fn, string m, string murl,
    string mod, string mon, string mono,
    string mourl, string sno)
: friendlyName(fn)
, manufacturer(m)
, manufacturerURL(murl)
, modelDescription(mod)
, modelName(mon)
, modelNumber(mono)
, modelURL(mourl)
, serialNumber(sno)
{
}

cMediaServer::ServerIcon::ServerIcon(image::cIcon profile, string filename)
: profile(profile)
, filename(filename)
{
}
