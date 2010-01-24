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
#include "server.h"

class cUPnPServer;

/**
 * The UPnP/DLNA plugin
 *
 * This is a UPnP/DLNA media server plugin. It supports live-TV and recordings
 * of the VDR as well as custom video files.
 */
class cPluginUpnp : public cPlugin {
private:
    // Add any member variables or functions you may need here.
    cUPnPServer* mUpnpServer;
    static const char*  mConfigDirectory;
public:
    cPluginUpnp(void);
    virtual ~cPluginUpnp();
    /**
     * Get the version of the plugin
     *
     * Returns the version string of the plugin
     *
     * @return a string representation of the plugin version
     */
    virtual const char *Version(void);
    /**
     * Get the description
     *
     * This returns a brief description of the plugin and what it does.
     *
     * @return the description of the plugin
     */
    virtual const char *Description(void);
    /**
     * Get the command line help
     *
     * This returns the command line help output, which comes, when the user
     * types \c --help into the command line.
     *
     * @return the command line help
     */
    virtual const char *CommandLineHelp(void);
    /*! @copydoc cUPnPConfig::processArgs */
    virtual bool ProcessArgs(int argc, char *argv[]);
    /**
     * Initializes the plugin
     *
     * This initializes any background activities of the plugin.
     *
     * @return returns
     * - \bc true, if initializing was successful
     * - \bc false, otherwise
     */
    virtual bool Initialize(void);
    /**
     * Starts the plugin
     *
     * This starts the plugin. It starts additional threads, which are required
     * by the plugin.
     *
     * @return returns
     * - \bc true, if starting was successful
     * - \bc false, otherwise
     */
    virtual bool Start(void);
    /**
     * Stops the plugin
     *
     * This stops the plugin and all its components
     */
    virtual void Stop(void);
    /**
     * Message if still active
     *
     * This returns a message if the plugin is still active when a user attempts
     * to shut down the VDR.
     *
     * @return the message shown on the screen.
     */
    virtual cString Active(void);
    /**
     * Setup menu
     *
     * This creates a new instance of the setup menu, which is shown to the user
     * when he enters the VDR plugin setup menu
     *
     * @return the menu of the plugin
     */
    virtual cMenuSetupPage *SetupMenu(void);
    /*! @copydoc cUPnPConfig::parseSetup */
    virtual bool SetupParse(const char *Name, const char *Value);
    /**
     * Get the configuration directory
     *
     * This returns the directory, where configuration files are stored.
     *
     * @return the directory of the configuration files.
     */
    static const char* getConfigDirectory();
};

extern cCondWait DatabaseLocker;        ///< Locks the database to be loaded only if
                                        ///< the configuration file directory is set

#endif	/* _UPNP_H */

