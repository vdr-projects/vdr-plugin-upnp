/* 
 * File:   profiles_ac3.h
 * Author: savop
 *
 * Created on 7. Dezember 2009, 13:04
 */

#ifndef _PROFILES_AC3_H
#define	_PROFILES_AC3_H

#include "profile_data.h"

extern DLNAProfile DLNA_PROFILE_AC3;                    ///< DLNA AC3

class cAC3Profiler : public cDLNAProfiler, public cAudioProfiler {
public:
    virtual DLNAProfile* probeDLNAProfile(AVFormatContext* FormatCtx);
    virtual AudioPortionProfile probeAudioProfile(AVFormatContext* FormatCtx);
};

extern cAC3Profiler AC3Profiler;

#endif	/* _PROFILES_AC3_H */

