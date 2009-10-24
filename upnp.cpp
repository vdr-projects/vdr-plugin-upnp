/*
 * upnp.c: A plugin for the Video Disk Recorder
 *
 * See the README file for copyright information and how to reach the author.
 *
 * $Id$
 */

#include <stdio.h>
#include "upnp.h"
#include "misc/menusetup.h"
#include "misc/config.h"

cCondWait DatabaseLocker;

static const char *VERSION        = "0.0.1";
static const char *DESCRIPTION    = PLUGIN_DESCRIPTION;

const char* cPluginUpnp::mConfigDirectory = NULL;

cPluginUpnp::cPluginUpnp(void)
{
  // Initialize any member variables here.
  // DON'T DO ANYTHING ELSE THAT MAY HAVE SIDE EFFECTS, REQUIRE GLOBAL
  // VDR OBJECTS TO EXIST OR PRODUCE ANY OUTPUT!
    this->mUpnpServer = new cUPnPServer();
}

cPluginUpnp::~cPluginUpnp()
{
  // Clean up after yourself!
    delete this->mUpnpServer;
}

const char* cPluginUpnp::getConfigDirectory(){
    return cPluginUpnp::mConfigDirectory;
}

const char *cPluginUpnp::Version(void){
    return VERSION;
}

const char *cPluginUpnp::Description(void) {
    return DESCRIPTION;
}

const char *cPluginUpnp::CommandLineHelp(void)
{
    // Return a string that describes all known command line options.
    cString cmdHelp;
    
    cmdHelp = cString::sprintf(
            "  The server can automatically detect both IP address and an\n"
            "  appropriate port, assuming that the first network interface\n"
            "  is connected to the internal network. However, it is possible\n"
            "  to specify alternative settings with the following options:\n\n"
            "  -i <interface>  --int=<interface>   The server network\n"
            "                                      interface\n"
            "                                      e.g: eth0, wlan1 etc.\n"
            "                                      If given option '-a' must\n"
            "                                      be absent.\n"
            "  -a <address>    --address=<address> The server IPv4 address.\n"
            "                                      If given option '-i' must\n"
            "                                      be absent.\n"
            "  -p <port>       --port=<port>       The server port\n"
            "                                      Supported ports:\n"
            "                                            %5d (auto detect)\n"
            "                                      %5d-%5d (user defined)\n",
            0,
            SERVER_MIN_PORT,
            SERVER_MAX_PORT
            );
    return cmdHelp;
}

bool cPluginUpnp::ProcessArgs(int argc, char *argv[])
{
    return cUPnPConfig::get()->processArgs(argc, argv);
}

bool cPluginUpnp::Initialize(void)
{
     // Initialize any background activities the plugin shall perform.
    MESSAGE("######### LETS GET READY TO RUMBLE #########");

    cPluginUpnp::mConfigDirectory = strdup(cPlugin::ConfigDirectory(this->Name()));
    if(!cPluginUpnp::getConfigDirectory()){
        ERROR("Cannot set configuration directory");
        return false;
    }
    MESSAGE("Configuration directory: %s", cPluginUpnp::getConfigDirectory());
    DatabaseLocker.Signal();
    return this->mUpnpServer->init();
}

bool cPluginUpnp::Start(void)
{
    MESSAGE("Call plugin START");
    // Start any background activities the plugin shall perform.
    return this->mUpnpServer->start();
    //return true;
}

void cPluginUpnp::Stop(void)
{
    MESSAGE("Call plugin STOP");
    // Stop any background activities the plugin is performing.
    this->mUpnpServer->stop();
}

cString cPluginUpnp::Active(void)
{
    // Return a message string if shutdown should be postponed
    return this->mUpnpServer->isRunning() ? _("The UPnP server is still running."): NULL;
}

cMenuSetupPage *cPluginUpnp::SetupMenu(void)
{
  // Return a setup menu in case the plugin supports one.
    return new cMenuSetupUPnP();
}

bool cPluginUpnp::SetupParse(const char *Name, const char *Value)
{
    return cUPnPConfig::get()->parseSetup(Name, Value);
}

VDRPLUGINCREATOR(cPluginUpnp); // Don't touch this!

