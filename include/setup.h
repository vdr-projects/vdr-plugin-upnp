/*
 * setup.h
 *
 *  Created on: 21.09.2012
 *      Author: savop
 */

#ifndef SETUP_H_
#define SETUP_H_

#include <vdr/plugin.h>
#include "../include/config.h"
#include "../include/tools.h"

#define STRING_SIZE 1024

class cMenuEditIpItem: public cMenuEditItem {
private:
  char *value;
  int curNum;
  int pos;
  bool step;
protected:
  virtual void Set(void);
public:
  cMenuEditIpItem(const char *Name, char *Value); // Value must be 16 bytes
  ~cMenuEditIpItem();
  virtual eOSState ProcessKey(eKeys Key);
};

/**
 * The VDR setup page
 *
 * This class shows and manages the settings within the VDR setup OSD
 *
 */
class cMenuSetupUPnP : public cMenuSetupPage {
public:
    cMenuSetupUPnP();
    virtual ~cMenuSetupUPnP();

    /**
     * Processes a keystroke
     *
     * This processes a keystroke which is done by the user and updates the
     * menu accordingly
     *
     * It returns the current state of the VDR after pressing a key
     *
     * @return The current state of the VDR
     */
    virtual eOSState ProcessKey(
        eKeys Key       ///< Key, pressed by the user
    );

    static bool SetupParse(const char *Name, const char *Value, upnp::cConfig& config);
protected:
    /**
     * Stores the setup information
     *
     * This stores the setup information in the configuration file
     */
    virtual void Store(void);

    /**
     * Update the menu
     *
     * This updates the menu osd and refreshes the screen.
     */
    void Update(void);

    /**
     * Loads the setup information
     *
     * This loads the setup information from the configuration file
     */
    void Load(void);
private:

    cOsdItem *ctrlGenerate;

    int switchExpertSettings;
    int switchBindAddress;
    int switchLive;
    int interfaceIndex;

    int upnpport;
    int wsport;
    int lvport;

    char webserverRoot[STRING_SIZE];
    char presentationUrl[STRING_SIZE];
    char address[16];
    char interface[16];
    char databaseFile[STRING_SIZE];
    char deviceUUID[STRING_SIZE];

    upnp::cConfig config;

    void GetInterfaces();
    int GetIndexOfInterface(std::string interface) const;

    const char* interfaces[16];
    int ifaceCount;

    upnp::StringVector ifaceVector;
};

#endif /* SETUP_H_ */
