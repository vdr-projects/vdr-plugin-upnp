/* 
 * File:   avdetector.h
 * Author: savop
 *
 * Created on 26. Oktober 2009, 13:02
 */

#ifndef _AVDETECTOR_H
#define	_AVDETECTOR_H

#include "../database/object.h"
#include <vdr/recording.h>
#include <vdr/channels.h>

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
}

/**
 * The audio/video detector
 *
 * This is the audio video detector, which analizes the audio and video stream
 * of a file to gather more information about the resource. This is also
 * required for determination of a suitable DLNA profile.
 */
class cAudioVideoDetector {
private:
    void init();
    void uninit();
    int detectChannelProperties();
    int detectFileProperties();
    int detectRecordingProperties();
    /**
     * Detect video properties
     *
     * This detects video properties of a video stream
     *
     * @return returns
     * - \bc 0, if the detection was successful
     * - \bc <0, otherwise
     */
    int analyseVideo(AVFormatContext* FormatCtx);
    /**
     * Detect audio properties
     *
     * This detects audio properties of a video or audio stream
     *
     * @return returns
     * - \bc 0, if the detection was successful
     * - \bc <0, otherwise
     */
    int analyseAudio(AVFormatContext* FormatCtx);
    int analyseFormat(AVFormatContext* FormatCtx);
    int detectDLNAProfile();
    UPNP_RESOURCE_TYPES mResourceType;
    union {
        const cChannel*     Channel;
        const cRecording*   Recording;
        const char*         Filename;
    }           mResource;
    int         mWidth;
    int         mHeight;
    int         mBitrate;
    int         mBitsPerSample;
    int         mColorDepth;
    off64_t     mDuration;
    off64_t     mSize;
    int         mSampleFrequency;
    int         mNrAudioChannels;
    DLNAProfile* mDLNAProfile;
public:
    cAudioVideoDetector(const char* Filename);
    cAudioVideoDetector(const cChannel* Channel);
    cAudioVideoDetector(const cRecording* Recording);
    virtual ~cAudioVideoDetector();
    /**
     * Detect resource properties of the file
     * 
     * This detects the resource properties of a file. If the returned value
     * is 0, no erros occured while detection and the properties are properly
     * set. Otherwise, in case of an error, the properties may have
     * unpredictable values.
     * 
     * @return returns
     * - \bc 0, if the detection was successful
     * - \bc <0, otherwise
     */
    int detectProperties();
    DLNAProfile* getDLNAProfile() const { return this->mDLNAProfile; }
    const char* getContentType() const { return (this->mDLNAProfile) ? this->mDLNAProfile->mime : NULL; }
    const char* getProtocolInfo() const;
    int         getWidth() const { return this->mWidth; }
    int         getHeight() const { return this->mHeight; }
    int         getBitrate() const { return this->mBitrate; }
    int         getBitsPerSample() const { return this->mBitsPerSample; }
    int         getSampleFrequency() const { return this->mSampleFrequency; }
    int         getNumberOfAudioChannels() const { return this->mNrAudioChannels; }
    int         getColorDepth() const { return this->mColorDepth; }
    off64_t     getFileSize() const { return this->mSize; }
    off64_t     getDuration() const { return this->mDuration; }
};

#endif	/* _AVDETECTOR_H */

