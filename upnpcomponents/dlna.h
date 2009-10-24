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

class cRegisteredProfiles : public cList<cRegisteredProfile> {
    friend class cDlna;
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
    static cDlna* getInstance(void);
    virtual ~cDlna();
    //const char* getProtocolInfo(UPnPObjectID OID);
    const char* getDeviceDescription(const char* URLBase);
    void registerProfile(DLNAProfile* Profile, int Op = -1, const char* Ps = NULL, int Ci = -1, unsigned int Flags = 0);
    void registerMainProfiles();
    const char* getSupportedProtocols();
    const char* getProtocolInfo(DLNAProfile *Prof);
    DLNAProfile* getProfileOfChannel(cChannel* Channel);
    DLNAProfile* getProfileOfRecording(cRecording* Recording);
    DLNAProfile* getProfileOfFile(cString File);
private:
    const char* getRegisteredProtocolInfoString(cRegisteredProfile *Profile);
    cDlna();
    void init(void);
    static cDlna* mInstance;
    cRegisteredProfiles* mRegisteredProfiles;
};

#endif	/* _DLNA_H */

