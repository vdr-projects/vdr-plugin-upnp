/* 
 * File:   profiles_mp3.h
 * Author: savop
 *
 * Created on 7. Dezember 2009, 13:08
 */

#ifndef _PROFILES_MP3_H
#define	_PROFILES_MP3_H

#include "profile_data.h"

extern DLNAProfile DLNA_PROFILE_MP3;                    ///< DLNA MP3
extern DLNAProfile DLNA_PROFILE_MP3X;                   ///< MP3x

class cMPEGAudioProfiler : public cDLNAProfiler, public cAudioProfiler {
public:
    virtual AudioPortionProfile probeAudioProfile(AVFormatContext* FormatCtx);
    virtual DLNAProfile* probeDLNAProfile(AVFormatContext* FormatCtx);
};

extern cMPEGAudioProfiler MPEGAudioProfiler;

#endif	/* _PROFILES_MP3_H */

