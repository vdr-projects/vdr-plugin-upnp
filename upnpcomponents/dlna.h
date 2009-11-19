/* 
 * File:   dlna.h
 * Author: savop
 *
 * Created on 18. April 2009, 23:27
 */

#ifndef _DLNA_H
#define	_DLNA_H

#include "../common.h"
#include <vdr/channels.h>
#include <vdr/recording.h>

class cDlna;

/**
 * Registered DLNA profile
 *
 * This class contains information about a certain registered profile
 * like play speeds or flags
 */
class cRegisteredProfile : public cListObject {
    friend class cDlna;
private:
    DLNAProfile* Profile;
    int Operation;
    const char* PlaySpeeds;
    int Conversion;
    int PrimaryFlags;
public:
    cRegisteredProfile(){};
    virtual ~cRegisteredProfile(){};
};

/**
 * Enable DLNA compliant media transfer
 *
 * This class enables media transmission with DLNA conformity. Its compliant with
 * version 1.5 of the DLNA guidelines.
 *
 */
class cDlna {
    friend class cUPnPServer;
public:
    /**
     * Returns the instance of DLNA object
     *
     * This will create a DLNA object instance. It will return the same instance
     * on subsequent calls.
     *
     * @return the DLNA object instance
     */
    static cDlna* getInstance(void);
    virtual ~cDlna();
    //const char* getProtocolInfo(UPnPObjectID OID);
    /**
     * Device description document
     *
     * This will return the device description document with service type
     * definitions as well as some DLNA specific information
     *
     * @return The description document
     */
    const char* getDeviceDescription(
        const char* URLBase         ///< the URLBase to be set in the document
    );
    /**
     * Registeres a DLNA profile
     *
     * Registeres a DLNA profile with specific optional options
     *
     * @see common.h
     */
    void registerProfile(
        DLNAProfile* Profile,       ///< the DLNA profile
        int Op = -1,                ///< operation mode
        const char* Ps = NULL,      ///< play speed (CSV list)
        int Ci = -1,                ///< conversion indication flag
        unsigned int Flags = 0      ///< DLNA flags
    );
    /**
     * Registeres all known DLNA profiles
     *
     * Registeres all well known DLNA profiles with its known options
     */
    void registerMainProfiles();
    /**
     * CSV list of supported protocols
     *
     * Returns a comma separated list with all supported protocols. This
     * means, it returns the list of protocols of the registered profiles.
     *
     * @return CSV list of registered protocols
     */
    const char* getSupportedProtocols();
    /**
     * Protocol info of a specific DLNA profile
     *
     * Returns the protocol info string of a specific DLNA profile with its
     * options and flags.
     *
     * @return the protocol info string of the profile
     */
    const char* getProtocolInfo(
        DLNAProfile *Prof       ///< the Profile of which the protocol info shall be returned
    );
    /**
     * Profile of a channel
     *
     * Returns the DLNA profile of a VDR channel. It checks the video type to determine
     * which profile will match.
     *
     * @return the matching DLNA profile
     */
    DLNAProfile* getProfileOfChannel(
        cChannel* Channel       ///< the channel of which the profile should created from
    );
    /**
     * Profile of a recording
     *
     * Returns the DLNA profile of a VDR recording. It checks the video file to determine
     * which profile will match.
     *
     * @return the matching DLNA profile
     */
    DLNAProfile* getProfileOfRecording(
        cRecording* Recording       ///< the recording of which the profile should be created from
    );
    /**
     * Profile of a file
     *
     * Returns the DLNA profile of a file. It checks the content of the file with \em ffmpeg to
     * determine which profile will match.
     *
     * @return the matching DLNA profile
     */
    DLNAProfile* getProfileOfFile(
        cString File                ///< the file of which the profile should be created from
    );
private:
    const char* getRegisteredProtocolInfoString(cRegisteredProfile *Profile);
    cDlna();
    void init(void);
    static cDlna* mInstance;
    cList<cRegisteredProfile>* mRegisteredProfiles;
};

#endif	/* _DLNA_H */

