/*
 * server.h
 *
 *  Created on: 31.07.2012
 *      Author: savop
 */

#ifndef SERVER_H_
#define SERVER_H_

#include <string>
#include <upnp/upnp.h>
#include "../include/config.h"
#include "../include/service.h"
#include "../include/webserver.h"

using namespace std;

namespace upnp {

class cMediaManager;

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

  const char* GetServerIPAddress() const;
  uint16_t GetServerPort() const;

  int GetAnnounceMaxAge() const { return mAnnounceMaxAge; }
  size_t GetMaxContentLength() const { return mMaxContentLength; }

  const cWebserver& GetWebserver() const { return *mWebserver; }
  cMediaManager& GetManager() const { return *mMediaManager; }

  const string GetDeviceUUID() const { return string("uuid:") + mCurrentConfiguration.deviceUUID; }

  const Description& GetServerDescription() const { return mServerDescription; }
  const iconList& GetServerIcons() const { return mServerIcons; }
  const serviceMap& GetServices() const { return mServices; }

  static void RegisterService(cUPnPService* service);

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
	UpnpDevice_Handle   mDeviceHandle;
	int                 mAnnounceMaxAge;
	size_t              mMaxContentLength;

	cWebserver*         mWebserver;
	cMediaManager*      mMediaManager;

  static serviceMap   mServices;

};

}


#endif /* SERVER_H_ */
