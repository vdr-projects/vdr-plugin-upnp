/*
 * upnp.h
 *
 *  Created on: 31.07.2012
 *      Author: savop
 */

#ifndef UPNP_H_
#define UPNP_H_

#include <vdr/plugin.h>
#include "include/server.h"

using namespace upnp;

static const char *VERSION        = "1.0.0";
static const char *DESCRIPTION    = trNOOP("UPnP/DLNA compliant Media Server functionality for VDR");

class cPluginUpnp : public cPlugin {
private:
  // Add any member variables or functions you may need here.
  cMediaServer* mMediaServer;
public:
  cPluginUpnp(void);
  virtual ~cPluginUpnp();
  virtual const char *Version(void) { return VERSION; }
  virtual const char *Description(void) { return tr(DESCRIPTION); }
  virtual const char *CommandLineHelp(void);
  virtual bool ProcessArgs(int argc, char *argv[]);
  virtual bool Initialize(void);
  virtual bool Start(void);
  virtual void Stop(void);
  virtual cString Active(void);
  virtual void Housekeeping(void);
  virtual void MainThreadHook(void);
  virtual cMenuSetupPage *SetupMenu(void);
  virtual bool SetupParse(const char *Name, const char *Value);
};


#endif /* UPNP_H_ */
