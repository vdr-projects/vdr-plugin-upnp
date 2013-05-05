/*
 * server.h
 *
 *  Created on: 31.07.2012
 *      Author: savop
 */

#ifndef SERVER_H_
#define SERVER_H_

#include <string>
#include <map>
#include <list>
#include <upnp/upnp.h>
#include <tntdb/connect.h>
#include <tntdb/connection.h>
#include "../include/config.h"

using namespace std;

namespace upnp {

class cMediaManager;
class cWebserver;
class cUPnPService;

class cMediaServer {
public:

  struct Description {
    Description(string, string, string , string, string, string , string, string, string);
    string friendlyName;
    string manufacturer;
    string manufacturerURL;
    string modelDescription;
    string modelName;
    string modelNumber;
    string modelURL;
    string serialNumber;
    string descriptionFile;
  };

  struct ServerIcon {
    ServerIcon(image::cIcon, string);
    image::cIcon  profile;
    string filename;
  };

  typedef map<string, cUPnPService*> serviceMap;
  typedef list<ServerIcon> iconList;

	virtual ~cMediaServer();

	bool Initialize();
	bool Start();
	bool Stop();

  static cMediaServer* GetInstance();

  void SetConfiguration(upnp::cConfig newConfig);
  upnp::cConfig GetConfiguration() const;
  const string& GetConfigDirectory() const { return mConfigDirectory; };

  const char* GetServerIPAddress() const;
  uint16_t GetServerPort() const;

  int GetAnnounceMaxAge() const { return mAnnounceMaxAge; }
  size_t GetMaxContentLength() const { return mMaxContentLength; }

  const cWebserver& GetWebserver() const { return *mWebserver; }
  cMediaManager& GetManager() const { return *mMediaManager; }
  tntdb::Connection& GetDatabase() const { return mConnection; }

  const string GetDeviceUUID() const { return string("uuid:") + mCurrentConfiguration.deviceUUID; }

  const Description& GetServerDescription() const { return mServerDescription; }
  const iconList& GetServerIcons() const { return mServerIcons; }
  static serviceMap& GetServices();

  static void RegisterService(cUPnPService* service);

  void Housekeeping();

  string GetErrorMessage(int error) const;
  string GetErrorHelp(int error) const;

private:
	cMediaServer();

	string GetDeviceDescriptionUrl() const;
	void SetAnnounceMaxAge(int announceMaxAge);
	void SetMaxContentLength(size_t maxContentLength);
	bool CheckDeviceUUID(string deviceUUID) const;

	static int ActionCallback(Upnp_EventType eventtype, void *event, void *cookie);

	Description         mServerDescription;
	iconList            mServerIcons;
	upnp::cConfig       mCurrentConfiguration;
  tntdb::Connection   mConnection;
	string              mConfigDirectory;
	UpnpDevice_Handle   mDeviceHandle;
	int                 mAnnounceMaxAge;
	size_t              mMaxContentLength;

	cWebserver*         mWebserver;
	cMediaManager*      mMediaManager;

};

}


#endif /* SERVER_H_ */
