/* 
 * File:   avdetector.h
 * Author: savop
 *
 * Created on 26. Oktober 2009, 13:02
 */

#ifndef _AVDETECTOR_H
#define	_AVDETECTOR_H

#include "../database/object.h"

/**
 * The audio/video detector
 *
 * This is the audio video detector, which analizes the audio and video stream
 * of a file to gather more information about the resource. This is also
 * required for determination of a suitable DLNA profile.
 */
class cAudioVideoDetector {
public:
    cAudioVideoDetector(){};
    virtual ~cAudioVideoDetector(){};
    /**
     * Detect video properties
     *
     * This detects video properties of a video stream and stores them in the
     * Resource object.
     *
     * @param Resource the resource, where to save the data
     * @param Filename the file, which shall be read
     * @return returns
     * - \bc 0, if the detection was successful
     * - \bc <0, otherwise
     */
    int detectVideoProperties(cUPnPResource* Resource, const char* Filename);
private:
};

#endif	/* _AVDETECTOR_H */

