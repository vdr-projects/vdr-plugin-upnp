/* 
 * File:   avdetector.h
 * Author: savop
 *
 * Created on 26. Oktober 2009, 13:02
 */

#ifndef _AVDETECTOR_H
#define	_AVDETECTOR_H

#include "../database/object.h"

class cAudioVideoDetector {
public:
    cAudioVideoDetector(){};
    virtual ~cAudioVideoDetector(){};
    int detectVideoProperties(cUPnPResource* Resource, const char* Filename);
private:
};

#endif	/* _AVDETECTOR_H */

