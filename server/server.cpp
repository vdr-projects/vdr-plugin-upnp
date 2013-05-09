/*
 * server.cpp
 *
 *  Created on: 31.07.2012
 *      Author: savop
 */

#include <vdr/tools.h>
#include <string>
#include <sstream>
#include <boost/algorithm/string.hpp>
#include <unistd.h>
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
                     tr(DESCRIPTION), "VDR UPnP-DLNA MS", VERSION,
                     "http://projects.vdr-developer.org/projects/plg-upnp/files", VERSION,
                     "deviceDescription.xml")
, mDeviceHandle(0)
, mAnnounceMaxAge(1800)
, mMaxContentLength(KB(20))
, mWebserver(NULL)
, mMediaManager(NULL)
{
  char tmp[255];

  if (gethostname(tmp, sizeof(tmp)) == 0) {
    string name = tmp;
    boost::to_upper(name);
    mServerDescription.friendlyName = name + " - " + mServerDescription.friendlyName;
  }

  mServerIcons.push_back(ServerIcon(image::DLNA_ICON_PNG_SM_24A, "images/upnpIconSm.png"));
  mServerIcons.push_back(ServerIcon(image::DLNA_ICON_PNG_LRG_24A, "images/upnpIconLrg.png"));
  mServerIcons.push_back(ServerIcon(image::DLNA_ICON_JPEG_SM_24, "images/upnpIconSm.jpeg"));
  mServerIcons.push_back(ServerIcon(image::DLNA_ICON_JPEG_LRG_24, "images/upnpIconLrg.jpeg"));
}

cMediaServer::~cMediaServer(){
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

  try {
    mConnection.execute("VACUUM");
  } catch (const std::exception& e) {
    esyslog("UPnP\tFailed to vacuum database: '%s'", e.what());
  }
}

bool cMediaServer::Start(){

  // If the plugin is not enabled, do not start it.
  if(!mCurrentConfiguration.enabled) return true;

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
    esyslog("UPnP\tFailed to register the device. Error code: %s Help: %s",
        GetErrorMessage(ret).c_str(), GetErrorHelp(ret).c_str());
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
    esyslog("UPnP\tFailed to register the device. Error code: %s Help: %s",
        GetErrorMessage(ret).c_str(), GetErrorHelp(ret).c_str());
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
    esyslog("UPnP\tError while sending first advertisments. Error code: %s Help: %s",
        GetErrorMessage(ret).c_str(), GetErrorHelp(ret).c_str());
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
    esyslog("UPnP\tError while unregistering device. Error code: %s Help: %s",
        GetErrorMessage(ret).c_str(), GetErrorHelp(ret).c_str());
    return false;
  }

  isyslog("UPnP\tStopping web server...");
  if(mWebserver){
    mWebserver->Stop();
  }

  return true;
}

void cMediaServer::Housekeeping(){
  if(mMediaManager){
    mMediaManager->Housekeeping();
  }
}

bool cMediaServer::Initialize(){
  string address;
  uint16_t port = 0;

  mConfigDirectory = cPlugin::ConfigDirectory(PLUGIN_NAME_I18N);

  if(mCurrentConfiguration.expertSettings){
    address = mCurrentConfiguration.bindToAddress
              ? mCurrentConfiguration.address
              : tools::GetAddressByInterface(mCurrentConfiguration.interface);

    if(address.empty() || address.compare("0.0.0.0") == 0){
      address = tools::GetAddressByInterface(tools::GetNetworkInterfaceByIndex(0, true));
    }

    port = mCurrentConfiguration.port;
  } else {
    address = tools::GetAddressByInterface(tools::GetNetworkInterfaceByIndex(0, true));
    port = 0;
  }

  int ret = 0;

  LOG(1, "Initializing UPnP media server on %s:%d", address.empty()?"0":address.c_str(), port);

  ret = UpnpInit(address.empty()?"127.0.0.1":address.c_str(), mCurrentConfiguration.port);

  if(ret != UPNP_E_SUCCESS && ret != UPNP_E_INIT){
    esyslog("UPnP\tFailed to initialise UPnP media server. Error code: %s Help: %s",
        GetErrorMessage(ret).c_str(), GetErrorHelp(ret).c_str());
    return false;
  }

  isyslog("UPnP\tInitialized UPnP media server on %s:%d", UpnpGetServerIpAddress(), UpnpGetServerPort());

  mWebserver = new cWebserver(GetServerIPAddress());

  stringstream ss;

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
      uint16_t port = mCurrentConfiguration.livePort ? mCurrentConfiguration.livePort : 8008;

      ss << "http://" << GetServerIPAddress() << ":" << port << "/";

      mWebserver->SetPresentationUrl(ss.str());
    }

    if(mCurrentConfiguration.webServerPort)
      mWebserver->SetListenerPort(mCurrentConfiguration.webServerPort);

    if(mCurrentConfiguration.maxRequestTime)
      mWebserver->SetMaxRequestTime(mCurrentConfiguration.maxRequestTime);

  }

  ss.str(string());
  ss << "sqlite:" << mCurrentConfiguration.databaseDir << "/metadata.db";
  try {
    mConnection = tntdb::connect(ss.str());

    mConnection.execute("PRAGMA foreign_keys = ON");
    mConnection.execute("PRAGMA page_size = 4096");
    mConnection.execute("PRAGMA cache_size = 16384");
    mConnection.execute("PRAGMA temp_store = MEMORY");
    mConnection.execute("PRAGMA synchronous = NORMAL");
    mConnection.execute("PRAGMA locking_mode = EXCLUSIVE");

    isyslog("UPnP\tSuccessfully connected to %s.", ss.str().c_str());
  } catch (const std::exception& e) {
    esyslog("UPnP\tException occurred while connecting to database '%s': %s", ss.str().c_str(), e.what());
    return false;
  }

  mMediaManager = new cMediaManager();

  ret = UpnpSetMaxContentLength(GetMaxContentLength());

  if(ret != UPNP_E_SUCCESS){
    esyslog("UPnP\tFailed to set max. content length of SOAP messages. Error code: %s Help: %s",
        GetErrorMessage(ret).c_str(), GetErrorHelp(ret).c_str());
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
    LOG(1, "Registered service: %s", service->GetServiceDescription().serviceType.c_str());
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

    LOG(3, "Action request: %s", actionRequest->ActionName);

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

    LOG(3, "Subscription request from: %s", eventRequest->ServiceId);

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

string cMediaServer::GetErrorHelp(int error) const {
  switch (error){
  case UPNP_E_INVALID_DESC:
    return "Invalid device description. Most likely, the web server has an issue start listening on a specific interface or port.";
  default:
    return string();
  }
}

string cMediaServer::GetErrorMessage(int error) const {
  switch (error){
  case UPNP_E_SUCCESS:
    return "Success";
  case UPNP_E_INVALID_HANDLE:
    return "Invalid UPnP handle.";
  case UPNP_E_INVALID_PARAM:
    return "Invalid parameter.";
  case UPNP_E_OUTOF_HANDLE:
    return "Out of UPnP handles.";
  case UPNP_E_OUTOF_CONTEXT:
    return "Out of context";
  case UPNP_E_OUTOF_MEMORY:
    return "Out of memory";
  case UPNP_E_INIT:
    return "Initialization error";
  case UPNP_E_BUFFER_TOO_SMALL:
    return "Buffer too small";
  case UPNP_E_INVALID_DESC:
    return "Invalid device description";
  case UPNP_E_INVALID_URL:
    return "Invalid URL";
  case UPNP_E_INVALID_SID:
    return "Invalid service ID";
  case UPNP_E_INVALID_SERVICE:
    return "Invalid service";
  case UPNP_E_INVALID_DEVICE:
    return "Invalid device";
  case UPNP_E_BAD_RESPONSE:
    return "Bad response";
  case UPNP_E_BAD_REQUEST:
    return "Bad request";
  case UPNP_E_INVALID_ACTION:
    return "Invalid action";
  case UPNP_E_FINISH:
    return "Library finished already";
  case UPNP_E_INIT_FAILED:
    return "Initialization failed";
  case UPNP_E_URL_TOO_BIG:
    return "URL too big";
  case UPNP_E_BAD_HTTPMSG:
    return "Bad HTTP message";
  case UPNP_E_ALREADY_REGISTERED:
    return "Already registered";
  case UPNP_E_NETWORK_ERROR:
    return "Network error";
  case UPNP_E_SOCKET_WRITE:
    return "Socket write error";
  case UPNP_E_SOCKET_READ:
    return "Socket read error";
  case UPNP_E_SOCKET_BIND:
    return "Socket bind error";
  case UPNP_E_SOCKET_CONNECT:
    return "Socket connect error";
  case UPNP_E_OUTOF_SOCKET:
    return "Out of sockets";
  case UPNP_E_LISTEN:
    return "Socket listen error";
  case UPNP_E_TIMEDOUT:
    return "Socket timeout";
  case UPNP_E_SOCKET_ERROR:
    return "General socket error";
  case UPNP_E_FILE_WRITE_ERROR:
    return "File write error";
  case UPNP_E_CANCELED:
    return "Canceled";
  case UPNP_E_EVENT_PROTOCOL:
    return "Event protocol";
  case UPNP_E_SUBSCRIBE_UNACCEPTED:
    return "Subscription rejected";
  case UPNP_E_UNSUBSCRIBE_UNACCEPTED:
    return "Unsubscription rejected";
  case UPNP_E_NOTIFY_UNACCEPTED:
    return "Notification rejected";
  case UPNP_E_INVALID_ARGUMENT:
    return "Invalid argument";
  case UPNP_E_FILE_NOT_FOUND:
    return "File not found";
  case UPNP_E_FILE_READ_ERROR:
    return "File read error";
  case UPNP_E_EXT_NOT_XML:
    return "Not an \".xml\" extension";
  case UPNP_E_NO_WEB_SERVER:
    return "No web server";
  case UPNP_E_OUTOF_BOUNDS:
    return "Out of bounds";
  case UPNP_E_NOT_FOUND:
    return "Not found";
  case UPNP_E_INTERNAL_ERROR:
    return "Internal error";
  default:
    return "Unknown error code. Please see the rest of the logs.";
  }
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
