/*
 * setup.cpp
 *
 *  Created on: 21.09.2012
 *      Author: savop
 */

#include "../include/setup.h"
#include "../include/server.h"
#include "../include/tools.h"
#include <vdr/osdbase.h>
#include <vdr/menuitems.h>

cMenuSetupUPnP::cMenuSetupUPnP()
: switchExpertSettings(0)
{
  Load();
  Update();
}

cMenuSetupUPnP::~cMenuSetupUPnP(){
}

void cMenuSetupUPnP::Update(){
  int current = Current();
  Clear();
  // Add OSD menu item for enabling UPnP Server

  Add(new cMenuEditBoolItem(tr("Enable UPnP server"),(int*)&config.enabled,tr("disabled"),tr("enabled")));
  Add(new cMenuEditBoolItem(tr("Use expert settings"),&switchExpertSettings));
  if(switchExpertSettings){

    cOsdItem *wsCat = new cOsdItem("--- Webserver settings -------------------------------------------------------");
    wsCat->SetSelectable(false);
    Add(wsCat);

    Add(new cMenuEditStrItem(tr("Webserver root directory"), webserverRoot, STRING_SIZE));
    Add(new cMenuEditIntItem(tr("Webserver port (0=auto)"), &wsport, 0, 65536));

    Add(new cMenuEditStrItem(tr("Presentation URL"), presentationUrl, STRING_SIZE));

    Add(new cMenuEditBoolItem(tr("Use \"live\" plugin for presentation"),&switchLive));

    if(switchLive){
      Add(new cMenuEditIntItem(tr("Live webserver port (0=auto)"), &lvport, 0, 65536));
    }

    cOsdItem *usCat = new cOsdItem("--- UPnP server settings -----------------------------------------------------");
    usCat->SetSelectable(false);
    Add(usCat);

    Add(ctrlGenerate = new cOsdItem(tr("Generate new device UUID")));

    cMenuEditStrItem* devUUID = new cMenuEditStrItem(tr("Current device UUID"), deviceUUID, STRING_SIZE);
    devUUID->SetSelectable(false);

    Add(devUUID);
    Add(new cMenuEditIntItem(tr("Max. content length of SOAP Messages"), (int*)&config.maxContentLength, KB(10), KB(200)));
    Add(new cMenuEditIntItem(tr("Max. age of UPnP announcements"), &config.announceMaxAge, 0, 3600));

    Add(new cMenuEditBoolItem(tr("Bind UPnP server to "),&switchBindAddress,tr("interface"),tr("address")));
    if(switchBindAddress){
      Add(new cMenuEditIpItem(tr("Set IP address"), address));
    } else {
      GetInterfaces();
      Add(new cMenuEditStraItem(tr("Set network interface"), &interfaceIndex, ifaceCount, interfaces));
    }
    Add(new cMenuEditIntItem(tr("UPnP server port (0=auto)"), &upnpport, 0, 65536));

    cOsdItem *dbCat = new cOsdItem("--- Database settings --------------------------------------------------------");
    dbCat->SetSelectable(false);
    Add(dbCat);

    Add(new cMenuEditStrItem(tr("Path to database file"), databaseDir, STRING_SIZE));
  }

  SetCurrent(Get(current));
  Display();
}

eOSState cMenuSetupUPnP::ProcessKey(enum eKeys key){
  int oldExpertSettings = switchExpertSettings;
  int oldInterfaceBind = switchBindAddress;
  int oldUseLive = switchLive;

  cOsdItem *item = this->Get(this->Current());

  eOSState state = cMenuSetupPage::ProcessKey(key);

  if(item == ctrlGenerate && key == kOk){
    config.GenerateNewDeviceUUID();
    strn0cpy(deviceUUID, config.deviceUUID.c_str(), STRING_SIZE);
    Update();
    state = osContinue;
  }

  if(key != kNone && (oldExpertSettings != switchExpertSettings ||
                      oldInterfaceBind != switchBindAddress ||
                      oldUseLive != switchLive)){
    Update();
  }

  return state;
}

void cMenuSetupUPnP::GetInterfaces() {
  ifaceVector = upnp::tools::GetNetworkInterfaces(true);

  for(ifaceCount = 0; ifaceCount < (int)ifaceVector.size(); ++ifaceCount){
    interfaces[ifaceCount] = ifaceVector[ifaceCount].c_str();
  }

}

int cMenuSetupUPnP::GetIndexOfInterface(std::string interface) const {
  for(int i = 0; i < (int)ifaceVector.size(); ++i){
    if(ifaceVector[i].compare(interface) == 0) return i;
  }

  return 0;
}

void cMenuSetupUPnP::Load(){
  config = upnp::cMediaServer::GetInstance()->GetConfiguration();

  switchExpertSettings = config.expertSettings?1:0;
  switchBindAddress = config.bindToAddress?1:0;
  switchLive = config.useLive;
  interfaceIndex = GetIndexOfInterface(config.interface);
  wsport = config.webServerPort;
  upnpport = config.port;
  lvport = config.livePort;

  strn0cpy(webserverRoot, config.webServerRoot.c_str(), STRING_SIZE);
  strn0cpy(address, config.address.c_str(), 16);
  strn0cpy(interface, config.interface.c_str(), 16);
  strn0cpy(databaseDir, config.databaseDir.c_str(), STRING_SIZE);
  strn0cpy(presentationUrl, config.presentationURL.c_str(), STRING_SIZE);
  strn0cpy(deviceUUID, config.deviceUUID.c_str(), STRING_SIZE);
}

void cMenuSetupUPnP::Store(){

  config.expertSettings = switchExpertSettings?true:false;
  config.webServerRoot = webserverRoot;
  config.address = address;
  config.interface = interface;
  config.databaseDir = databaseDir;
  config.presentationURL = presentationUrl;

  config.bindToAddress = switchBindAddress?true:false;
  config.webServerPort = wsport;
  config.port = upnpport;

  config.useLive = switchLive?true:false;
  config.livePort = lvport;

  upnp::cMediaServer::GetInstance()->SetConfiguration(config);

  SetupStore("enabled", config.enabled);
  SetupStore("expertSettings", config.expertSettings);
  SetupStore("webServerRoot", config.webServerRoot.c_str());
  SetupStore("webServerPort", config.webServerPort);
  SetupStore("presentationURL", config.presentationURL.c_str());
  SetupStore("useLive", config.useLive);
  SetupStore("livePort", config.livePort);
  SetupStore("maxContentLength", config.maxContentLength);
  SetupStore("announceMaxAge", config.announceMaxAge);
  SetupStore("deviceUUID", config.deviceUUID.c_str());
  SetupStore("bindToAddress", config.bindToAddress);
  SetupStore("address", config.address.c_str());
  SetupStore("interface", config.interface.c_str());
  SetupStore("port", config.port);
  SetupStore("databaseDir", config.databaseDir.c_str());
}

std::string cMenuSetupUPnP::parsedArgs;

bool cMenuSetupUPnP::SetupParse(const char *name, const char *value, upnp::cConfig& config)
{

  if(parsedArgs.find(name) != std::string::npos){
    dsyslog("UPnP\tSkipping %s=%s, was overridden in command line.", name, value);
    return true;
  }

  if      (strcasecmp(name, "enabled")==0)                config.enabled = atoi(value)?true:false;
  else if (strcasecmp(name, "expertSettings")==0)         config.expertSettings = atoi(value)?true:false;
  else if (strcasecmp(name, "webServerRoot")==0)          config.webServerRoot = value;
  else if (strcasecmp(name, "webServerPort")==0)          config.webServerPort = (uint16_t)atoi(value);
  else if (strcasecmp(name, "presentationURL")==0)        config.presentationURL = value;
  else if (strcasecmp(name, "useLive")==0)                config.useLive = atoi(value)?true:false;
  else if (strcasecmp(name, "livePort")==0)               config.livePort = atoi(value);
  else if (strcasecmp(name, "maxContentLength")==0)       config.maxContentLength = atol(value);
  else if (strcasecmp(name, "announceMaxAge")==0)         config.announceMaxAge = atoi(value);
  else if (strcasecmp(name, "deviceUUID")==0)             config.deviceUUID = value;
  else if (strcasecmp(name, "bindToAddress")==0)          config.bindToAddress = atoi(value)?true:false;
  else if (strcasecmp(name, "address")==0)                config.address = value;
  else if (strcasecmp(name, "interface")==0)              config.interface = value;
  else if (strcasecmp(name, "port")==0)                   config.port = (uint16_t)atoi(value);
  else if (strcasecmp(name, "databaseDir")==0)            config.databaseDir = value;
  else return false;

  parsedArgs += name;

  return true;
}

cMenuEditIpItem::cMenuEditIpItem(const char *Name, char *Value):cMenuEditItem(Name) {
  value = Value;
  curNum = -1;
  pos = -1;
  step = false;
  Set();
}

cMenuEditIpItem::~cMenuEditIpItem() {
}

void cMenuEditIpItem::Set(void) {
  char buf[1000];
  if (pos >= 0) {
    in_addr_t addr = inet_addr(value);
    if ((int)addr == -1)
      addr = 0;
    int p = 0;
    for (int i = 0; i < 4; ++i) {
      p += snprintf(buf + p, sizeof(buf) - p, pos == i ? "[%d]" : "%d",
          pos == i ? curNum : (addr >> (i * 8)) & 0xff);
      if (i < 3)
        buf[p++] = '.';
    }
    SetValue(buf);
  } else
    SetValue(value);
}

eOSState cMenuEditIpItem::ProcessKey(eKeys Key) {
  in_addr addr;
  addr.s_addr = inet_addr(value);
  if ((int)addr.s_addr == -1)
    addr.s_addr = 0;

  switch (Key) {
  case kUp:
    if (pos >= 0) {
      if (curNum < 255) ++curNum;
    } else
      return cMenuEditItem::ProcessKey(Key);
    break;

  case kDown:
    if (pos >= 0) {
      if (curNum > 0) --curNum;
    } else
      return cMenuEditItem::ProcessKey(Key);
    break;

  case kOk:
    if (pos >= 0) {
      addr.s_addr = inet_addr(value);
      if ((int)addr.s_addr == -1)
        addr.s_addr = 0;
      addr.s_addr &= ~(0xff << (pos * 8));
      addr.s_addr |= curNum << (pos * 8);
      strcpy(value, inet_ntoa(addr));
    } else
      return cMenuEditItem::ProcessKey(Key);
    curNum = -1;
    pos = -1;
    break;

  case kRight:
    if (pos >= 0) {
      addr.s_addr = inet_addr(value);
      if ((int)addr.s_addr == -1)
        addr.s_addr = 0;
      addr.s_addr &= ~(0xff << (pos * 8));
      addr.s_addr |= curNum << (pos * 8);
      strcpy(value, inet_ntoa(addr));
    }

    if (pos == -1 || pos == 3)
      pos = 0;
    else
      ++pos;

    curNum = (addr.s_addr >> (pos * 8)) & 0xff;
    step = true;
    break;

  case kLeft:
    if (pos >= 0) {
      addr.s_addr = inet_addr(value);
      if ((int)addr.s_addr == -1)
        addr.s_addr = 0;
      addr.s_addr &= ~(0xff << (pos * 8));
      addr.s_addr |= curNum << (pos * 8);
      strcpy(value, inet_ntoa(addr));
    }

    if (pos <= 0)
      pos = 3;
    else
      --pos;

    curNum = (addr.s_addr >> (pos * 8)) & 0xff;
    step = true;
    break;

      case k0 ... k9: /* Netbeans reports error with this line (.. is okay but wrong) */
                if (pos == -1)
      pos = 0;

    if (curNum == -1 || step) {
      curNum = Key - k0;
      step = false;
    } else
      curNum = curNum * 10 + (Key - k0);

    if ((curNum * 10 > 255) || (curNum == 0)) {
      in_addr addr;
      addr.s_addr = inet_addr(value);
      if ((int)addr.s_addr == -1)
        addr.s_addr = 0;
      addr.s_addr &= ~(0xff << (pos * 8));
      addr.s_addr |= curNum << (pos * 8);
      strcpy(value, inet_ntoa(addr));
      if (++pos == 4)
        pos = 0;
      curNum = (addr.s_addr >> (pos * 8)) & 0xff;
      step = true;
    }
    break;

  default:
    return cMenuEditItem::ProcessKey(Key);
  }

  Set();
  return osContinue;
}
