/* 
 * File:   container.h
 * Author: savop
 *
 * Created on 8. Januar 2010, 10:45
 */

#ifndef _CONTAINER_H
#define	_CONTAINER_H

#include "profile_data.h"

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
}

class cContainerDetector {
public:
    static VideoContainerProfile detect(AVFormatContext* Ctx);
private:
    /**
     * MPEG1
     */
    static VideoContainerProfile detectMPEG1Container(AVFormatContext* Ctx);
    /**
     * MPEG2-PS
     * MPEG2-TS
     * MPEG2-TS-DLNA
     * MPEG2-TS-DLNA-T
     */
    static VideoContainerProfile detectMPEG2Container(AVFormatContext* Ctx);
    /**
     * 3GPP
     * MP4
     */
    static VideoContainerProfile detectMP4Container(AVFormatContext* Ctx);
#ifdef WITH_WINDOWS_MEDIA
    /**
     * ASF
     */
    static VideoContainerProfile detectWMFContainer(AVFormatContext* Ctx);
#endif
};

#endif	/* _CONTAINER_H */

