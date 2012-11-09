/*
 * webserver.h
 *
 *  Created on: 06.08.2012
 *      Author: savop
 */

#ifndef WEBSERVER_H_
#define WEBSERVER_H_

#include <tnt/tntnet.h>
#include <stdint.h>
#include <string>
#include <vdr/thread.h>

namespace upnp {

  class cMediaServer;

  class cWebserver {
  public:
    cWebserver(std::string address);
    virtual ~cWebserver();

    void SetMaxRequestTime(unsigned int seconds);
    void SetWebserverRootDir(std::string rootDirectory);
    void SetPresentationUrl(std::string presentationUrl);

    void SetListenerPort(uint16_t port);

    bool Initialise();

    bool Start();
    void Stop();

    const std::string GetBaseUrl() const;
    const std::string GetServiceUrl() const;
    const std::string GetControlUrl() const;
    const std::string GetStaticContentUrl() const;
    const std::string GetPresentationUrl() const;

    const std::string GetThumbnailDir() const;

    std::string GetListenerAddress() const { return mListenerAddress; }
    uint16_t GetListenerPort() const { return mListenerPort; }

  private:
    tnt::Tntnet mApplication;
    std::string mWebserverRootDir;
    std::string mListenerAddress;
    uint16_t    mListenerPort;

    std::string mPresentationUrl;
    std::string mStaticContentUrl;
    std::string mServiceUrl;

    class cWSThread : public cThread {
    public:
      cWSThread(cWebserver& webServer);
      void Stop();
      virtual void Action(void);
    private:
      cWebserver& mWebserver;
    } mWebserverThread;
  };

};

#endif /* WEBSERVER_H_ */
