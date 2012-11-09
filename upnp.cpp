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
            "                                                file shall be located.\n");
}

bool cPluginUpnp::ProcessArgs(int argc, char *argv[])
{
  static struct option long_options[] = {
    {"db-dir",   required_argument, NULL, 'd'},
    {0, 0, 0, 0}
  };

  // Parse your own setup parameters and store their values.
  upnp::cConfig config = mMediaServer->GetConfiguration();

  int c = 0; int index = -1;
  while((c = getopt_long(argc, argv, "d:",long_options, &index)) != -1){
    switch(c){
    case 'd':
      if(!cMenuSetupUPnP::SetupParse("databaseDir", optarg, config)) return false;
      break;
    default:
      return false;
    }
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
  // Perform any cleanup or other regular tasks.
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
