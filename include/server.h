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
#include "../include/webserver.h"
#include "../include/config.h"

using namespace std;

namespace upnp {

class cUPnPService;
class cMediaManager;

class cMediaServer {
public:

  struct Description {
    Description(string, string, string , string, string, string , string, string);
    string friendlyName;
    string manufacturer;
    string manufacturerURL;
    string modelDescription;
    string modelName;
    string modelNumber;
    string modelURL;
    string serialNumber;
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

	bool IsRunning() const;

  static cMediaServer* GetInstance();

  void SetConfiguration(upnp::cConfig newConfig);
  upnp::cConfig GetConfiguration() const;

  const char* GetServerIPAddress() const;
  uint16_t GetServerPort() const;

  int GetAnnounceMaxAge() const { return mAnnounceMaxAge; }
  size_t GetMaxContentLength() const { return mMaxContentLength; }

  const cWebserver& GetWebserver() const { return *mWebserver; }
  cMediaManager& GetManager() const { return *mMediaManager; }

  const string GetDeviceUUID() const { return mCurrentConfiguration.deviceUUID; }

  const Description& GetServerDescription() const { return mServerDescription; }
  const iconList& GetServerIcons() const { return mServerIcons; }
  const serviceMap& GetServices() const { return mServices; }

  static void RegisterService(cUPnPService* service);

private:
  class RuntimeException : public std::exception {
  public:
    virtual const char* what() const throw();
  };

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
	bool                mIsRunning;

	cWebserver*         mWebserver;
	cMediaManager*      mMediaManager;

  static serviceMap   mServices;

};

}


#endif /* SERVER_H_ */
