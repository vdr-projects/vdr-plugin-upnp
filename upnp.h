/* 
 * File:   upnp.h
 * Author: savop
 *
 * Created on 17. April 2009, 20:53
 */

#ifndef _UPNP_H
#define	_UPNP_H

#include <vdr/thread.h>
#include <vdr/plugin.h>
#include "common.h"
#include "server/server.h"

class cUPnPServer;

class cPluginUpnp : public cPlugin {
private:
    // Add any member variables or functions you may need here.
    cUPnPServer* mUpnpServer;
    static const char*  mConfigDirectory;
public:
    cPluginUpnp(void);
    virtual ~cPluginUpnp();
    virtual const char *Version(void);
    virtual const char *Description(void);
    virtual const char *CommandLineHelp(void);
    virtual bool ProcessArgs(int argc, char *argv[]);
    virtual bool Initialize(void);
    virtual bool Start(void);
    virtual void Stop(void);
    virtual cString Active(void);
    virtual cMenuSetupPage *SetupMenu(void);
    virtual bool SetupParse(const char *Name, const char *Value);
    static const char* getConfigDirectory();
};

extern cCondWait DatabaseLocker;

#endif	/* _UPNP_H */

