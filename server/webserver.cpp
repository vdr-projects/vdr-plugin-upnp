/*
 * webserver.cpp
 *
 *  Created on: 06.08.2012
 *      Author: savop
 */

#include "../include/webserver.h"
#include "../upnp.h"
#include <sstream>
#include <tnt/job.h>

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
  mApplication.shutdown();
  mWebserverThread.Stop();
}

bool cWebserver::Start(){
  return mWebserverThread.Start();
}

void cWebserver::Stop(){
  try {
    mApplication.shutdown();
  } catch (const std::exception& e){
    esyslog("UPnP\tError while stopping web server: %s", e.what());
  }
}

bool cWebserver::Initialise(){

  try {
    // Map static contents
    stringstream ss1, ss2;

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

    mApplication.mapUrl("^/getStream", "resourceStreamer");

    isyslog("UPnP\tUsing %s for static content delivery.", mWebserverRootDir.c_str());

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

void cWebserver::SetWebserverRootDir(std::string rootDirectory){
  if(mWebserverThread.Active()) return;

  if(rootDirectory.empty())
    mWebserverRootDir = std::string(cPluginUpnp::ConfigDirectory(PLUGIN_NAME_I18N)) + "/httpdocs/";
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

const std::string cWebserver::GetBaseUrl() const {
  stringstream s;
  s << "http://" << mListenerAddress << ":" << mListenerPort << "/";

  return s.str();
}

const std::string cWebserver::GetServiceUrl() const {
  return GetBaseUrl() + mServiceUrl;
}

const std::string cWebserver::GetControlUrl() const {
  stringstream s;
  s << "http://" << UpnpGetServerIpAddress() << ":" << UpnpGetServerPort() << "/" << "services/";
  return s.str();
}

const std::string cWebserver::GetPresentationUrl() const {
  return (mPresentationUrl.find("http://",0) == 0) ? mPresentationUrl : (GetBaseUrl() + mPresentationUrl);
}

const std::string cWebserver::GetStaticContentUrl() const {
  return GetBaseUrl() + mStaticContentUrl;
}

cWebserver::cWSThread::cWSThread(cWebserver& webServer)
: mWebserver(webServer)
{
}

void cWebserver::cWSThread::Action(){
  try {
    mWebserver.mApplication.run();
  } catch (const std::exception& e){
    esyslog("UPnP\tError while starting web server: %s", e.what());
  }
}

void cWebserver::cWSThread::Stop(){
  Cancel(5);
}

}  // namespace upnp


