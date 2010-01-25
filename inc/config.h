/* 
 * File:   config.h
 * Author: savop
 *
 * Created on 15. August 2009, 13:03
 */

#ifndef _CONFIG_H
#define	_CONFIG_H

#include <vdr/tools.h>
#include "../common.h"

/**
 * The configuration settings
 *
 * This holds the configurations for the server. It holds information about the
 * network settings as well as some status flags.
 */
class cUPnPConfig {
private:
    static cUPnPConfig* mInstance;    
    cString mParsedArgs;
    cUPnPConfig();
public:
    static int verbosity;                               ///< the verbosity of the plugin, the higher the more messages
                                                        ///< are printed.
    char* mInterface;                                   ///< the network interface, which the server is bound to
    char* mAddress;                                     ///< the IP address which is used by the server
    int   mPort;                                        ///< the port which the server is listening on
    int   mEnable;                                      ///< indicates, if the server is enabled or not
    int   mAutoSetup;                                   ///< indicates, if the settings are automatically detected
    char* mDatabaseFolder;                              ///< the directory where the metadata.db is located
    char* mHTTPFolder;                                  ///< the directory where the HTTP data is located
public:
    virtual ~cUPnPConfig();
    /**
     * Get the configuration
     *
     * This returns the instance of the current configuration settings.
     *
     * @return the configuration object
     */
    static cUPnPConfig* get();
    /**
     * Parse setup variable
     *
     * This parses the setup variable with the according value. The value is a
     * string representation and must be converted into the according data type.
     *
     * @return returns
     * - \bc true, if parsing was successful
     * - \bc false, otherwise
     * @param Name the name of the variable
     * @param Value the according value of the variable
     */
    bool  parseSetup(const char* Name, const char* Value);
    /**
     * Processes the commandline arguments
     *
     * This processes the commandline arguments which the user specified at the
     * start of the plugin.
     *
     * @return returns
     * - \bc true, if processing was successful
     * - \bc false, otherwise
     * @param argc the number of arguments in the list
     * @param argv the arguments as a char array
     */
    bool  processArgs(int argc, char* argv[]);
};

#endif	/* _CONFIG_H */

