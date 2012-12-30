/*
 * webserver.cpp
 *
 *  Created on: 06.08.2012
 *      Author: savop
 */

#include "../include/webserver.h"
#include "../include/tools.h"
#include "../upnp.h"
#include <signal.h>
#include <sstream>
#include <tnt/job.h>
#include <tnt/configurator.h>

namespace upnp {

cWebserver::cWebserver(std::string address)
: mListenerAddress(address)
, mListenerPort(7649)
, mStaticContentUrl("http/")
, mServiceUrl("services/")
, mWebserverThread(*this)
{
  SetWebserverRootDir(string());
  SetPresentationUrl(string());
}

cWebserver::~cWebserver(){
  Stop();
}

bool cWebserver::Start(){
  return mWebserverThread.Start();
}

void cWebserver::Stop(){
  mWebserverThread.Stop();
}

bool cWebserver::Initialise(){

  try {
    // Map static contents
    stringstream ss1, ss2;

    ::signal(SIGPIPE, SIG_IGN);

    mApplication.listen(mListenerAddress.c_str(), mListenerPort);

    mApplication.mapUrl("^/$", "index");

    mApplication.mapUrl("^/index.html", "index");

    ss1.clear(); ss1.str(string());
    ss1 << "^/" << mServiceUrl << "([^/.]+).xml$";

    mApplication.mapUrl(ss1.str(), "$1");

    // Map static contents
    ss1.clear(); ss1.str(string());
    ss1 << "^/" << mStaticContentUrl << "(.*)";

    ss2.clear(); ss2.str(string());
    ss2 << mWebserverRootDir << "$1";

    mApplication.mapUrl(ss1.str(), ss2.str(), "static@tntnet");

    // Map static contents
    ss1.clear(); ss1.str(string());
    ss1 << "^/thumbs/([^.]+.jpg)$";

    ss2.clear(); ss2.str(string());
    ss2 << mWebserverRootDir << "/images/thumbs/$1";

    mApplication.mapUrl(ss1.str(), ss2.str(), "static@tntnet");

    mApplication.mapUrl("^/getStream", "resourceStreamer");

    isyslog("UPnP\tUsing %s for static content delivery.", mWebserverRootDir.c_str());

    // DLNA requires KeepAlive timeout of 60s.
    SetKeepAliveTimeout(60000);

  } catch (const std::exception& e){
    esyslog("UPnP\tError while initialising web server: %s", e.what());
    return false;
  }

  return true;
}

void cWebserver::SetListenerPort(uint16_t port){
  if(mWebserverThread.Active()) return;

  mListenerPort = port ? port : 7649;
}

void cWebserver::SetMaxRequestTime(unsigned int seconds){
  tnt::Configurator config(mApplication);
  config.setMaxRequestTime(seconds);
}

void cWebserver::SetKeepAliveTimeout(unsigned int milliseconds){
  tnt::Configurator config(mApplication);
  config.setKeepAliveTimeout(milliseconds);
}

void cWebserver::SetWebserverRootDir(std::string rootDirectory){
  if(mWebserverThread.Active()) return;

  if(rootDirectory.empty())
#if APIVERSNUM > 10729
    mWebserverRootDir = std::string(cPluginUpnp::ResourceDirectory(PLUGIN_NAME_I18N)) + "/httpdocs/";
#else
    mWebserverRootDir = std::string(cPluginUpnp::ConfigDirectory(PLUGIN_NAME_I18N)) + "/httpdocs/";
#endif
  else
    mWebserverRootDir = rootDirectory;
}

void cWebserver::SetPresentationUrl(std::string presentationUrl){
  if(mWebserverThread.Active()) return;

  if(presentationUrl.empty())
    mPresentationUrl = "index.html";
  else
    mPresentationUrl = presentationUrl;
}

std::string cWebserver::GetBaseUrl() const {
  stringstream s;
  s << "http://" << mListenerAddress << ":" << mListenerPort << "/";

  return s.str();
}

std::string cWebserver::GetServiceUrl() const {
  return GetBaseUrl() + mServiceUrl;
}

std::string cWebserver::GetControlUrl() const {
  stringstream s;
  s << "http://" << UpnpGetServerIpAddress() << ":" << UpnpGetServerPort() << "/services/";
  return s.str();
}

std::string cWebserver::GetPresentationUrl() const {
  return (mPresentationUrl.find("http://",0) == 0) ? mPresentationUrl : (GetBaseUrl() + mPresentationUrl);
}

std::string cWebserver::GetStaticContentUrl() const {
  return GetBaseUrl() + mStaticContentUrl;
}

std::string cWebserver::GetThumbnailDir() const {
  stringstream s;
  s << mWebserverRootDir << "images/thumbs/";

  return s.str();
}

cWebserver::cWSThread::cWSThread(cWebserver& webServer)
: mWebserver(webServer)
{
}

cWebserver::cWSThread::~cWSThread(){
  Stop();
}

void cWebserver::cWSThread::Action(){
  try {
    if(Running()){
      mWebserver.mApplication.run();
      LOG(3, "Started web server thread.");
    }
  } catch (const std::exception& e){
    esyslog("UPnP\tError while starting web server: %s", e.what());
  }
}

void cWebserver::cWSThread::Stop(){
  try {
    tnt::Tntnet::shutdown();
  } catch (const std::exception& e){
    esyslog("UPnP\tError while stopping web server: %s", e.what());
  }
  Cancel(5);
}

}  // namespace upnp


