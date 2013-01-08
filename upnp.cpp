/*
 * upnp.c: A plugin for the Video Disk Recorder
 *
 * See the README file for copyright information and how to reach the author.
 *
 * $Id$
 */

#include <sstream>
#include <getopt.h>
#include "upnp.h"
#include "include/setup.h"
#include "include/media/requestCounter.h"

using namespace std;
using namespace upnp;

tools::atomic::AtomicInteger request_counter_t::OPEN_REQUESTS;

cPluginUpnp::cPluginUpnp(void)
{
  mMediaServer = cMediaServer::GetInstance();
}

cPluginUpnp::~cPluginUpnp()
{
  // Clean up after yourself!
}

const char *cPluginUpnp::CommandLineHelp(void)
{
  return tr("  The UPnP/DLNA server is designed to detect everything automatically.\n"
            "  Therefore the user is not required to change most of the settings.\n"
            "  \n"
            "  -d <DB directory>    --db-dir=<DB directory>  Specifies the directory\n"
            "                                                where the the database\n"
            "                                                file shall be located.\n"
            "  -a <IP address>      --address=<IP address>   Binds the server to the\n"
            "                                                specific IP address.\n"
            "  -i <Interface>       --interface=<Interface>  Binds the server to the\n"
            "                                                specific network\n"
            "                                                interface.\n"
            "  -p <Port>            --port=<Port>            Sets the port where the\n"
            "                                                UPnP server shall listen\n"
            "                                                on.");
}

bool cPluginUpnp::ProcessArgs(int argc, char *argv[])
{
  static struct option long_options[] = {
    {"db-dir",   required_argument, NULL, 'd'},
    {"port",     required_argument, NULL, 'p'},
    {"address",  required_argument, NULL, 'a'},
    {"interface",required_argument, NULL, 'i'},
    {0, 0, 0, 0}
  };

  // Parse your own setup parameters and store their values.
  upnp::cConfig config = mMediaServer->GetConfiguration();

  int c = 0; int index = -1;
  bool expert = false;
  while((c = getopt_long(argc, argv, "d:p:a:i:",long_options, &index)) != -1){
    switch(c){
    case 'd':
      if(!cMenuSetupUPnP::SetupParse("databaseDir", optarg, config)) return false;
      break;
    case 'p':
      if(!cMenuSetupUPnP::SetupParse("port", optarg, config)) return false;
      expert = true;
      break;
    case 'a':
      if(!cMenuSetupUPnP::SetupParse("address", optarg, config) ||
         !cMenuSetupUPnP::SetupParse("bindToAddress", "1", config)) return false;
      expert = true;
      break;
    case 'i':
      if(!cMenuSetupUPnP::SetupParse("interface", optarg, config) ||
         !cMenuSetupUPnP::SetupParse("bindToAddress", "0", config)) return false;
      expert = true;
      break;
    default:
      return false;
    }
  }

  if(expert){
    if(!cMenuSetupUPnP::SetupParse("expertSettings", optarg, config)) return false;
  }

  mMediaServer->SetConfiguration(config);

  return true;
}

bool cPluginUpnp::Initialize(void)
{
  // Initialize any background activities the plugin shall perform.
  return mMediaServer->Initialize();
}

bool cPluginUpnp::Start(void)
{
  // Start any background activities the plugin shall perform.
  return mMediaServer->Start();
}

void cPluginUpnp::Stop(void)
{
  // Stop any background activities the plugin is performing.
  mMediaServer->Stop();
}

cString cPluginUpnp::Active(void)
{
  if(mMediaServer->GetConfiguration().maxRequestTime > 0 && request_counter_t::OPEN_REQUESTS > 0){
    return cString::sprintf(tr("There are %d requests active."), request_counter_t::OPEN_REQUESTS.Get());
  }
  return NULL;
}

void cPluginUpnp::Housekeeping(void)
{
  if(mMediaServer)
    mMediaServer->Housekeeping();
}

void cPluginUpnp::MainThreadHook(void)
{
  // Perform actions in the context of the main program thread.
  // WARNING: Use with great care - see PLUGINS.html!
}

cMenuSetupPage *cPluginUpnp::SetupMenu(void)
{
  // Return a setup menu in case the plugin supports one.
  return new cMenuSetupUPnP();
}

bool cPluginUpnp::SetupParse(const char *Name, const char *Value)
{
  // Parse your own setup parameters and store their values.
  upnp::cConfig config = mMediaServer->GetConfiguration();

  if(!cMenuSetupUPnP::SetupParse(Name, Value, config)) return false;

  mMediaServer->SetConfiguration(config);

  return true;
}

VDRPLUGINCREATOR(cPluginUpnp); // Don't touch this!
