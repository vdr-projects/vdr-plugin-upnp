/*
 * upnp.c: A plugin for the Video Disk Recorder
 *
 * See the README file for copyright information and how to reach the author.
 *
 * $Id$
 */

#include <iostream>
#include "upnp.h"
#include "include/connection.h"

using namespace std;
using namespace upnp;

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
  // Return a string that describes all known command line options.
  return NULL;
}

bool cPluginUpnp::ProcessArgs(int argc, char *argv[])
{
  // Implement command line argument processing here if applicable.
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

void cPluginUpnp::Housekeeping(void)
{
  // Perform any cleanup or other regular tasks.
}

void cPluginUpnp::MainThreadHook(void)
{
  // Perform actions in the context of the main program thread.
  // WARNING: Use with great care - see PLUGINS.html!
}

cString cPluginUpnp::Active(void)
{
  // Return a message string if shutdown should be postponed
  if(mMediaServer->IsRunning()){
    isyslog("UPnP\tPlugin is still active. Shutdown is postponed.");
    return tr("The UPnP server is still running.");
  }

  return NULL;
}

cMenuSetupPage *cPluginUpnp::SetupMenu(void)
{
  // Return a setup menu in case the plugin supports one.
  return NULL;
}

bool cPluginUpnp::SetupParse(const char *Name, const char *Value)
{
  // Parse your own setup parameters and store their values.
  return false;
}

VDRPLUGINCREATOR(cPluginUpnp); // Don't touch this!
