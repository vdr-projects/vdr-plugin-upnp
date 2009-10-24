/* 
 * File:   livereceiver.h
 * Author: savop
 *
 * Created on 4. Juni 2009, 13:28
 */

#ifndef _LIVERECEIVER_H
#define	_LIVERECEIVER_H

#include "../common.h"
#include "filehandle.h"
#include <vdr/thread.h>
#include <vdr/receiver.h>

class cLiveReceiver : public cReceiver, public cThread, public cFileHandle {
public:
    static cLiveReceiver* newInstance(cChannel *Channel, int Priority);
    virtual ~cLiveReceiver(void);
    virtual void open(UpnpOpenFileMode mode);
    virtual int read(char* buf, size_t buflen);
    virtual int write(char* buf, size_t buflen);
    virtual int seek(off_t offset, int whence);
    virtual void close();
protected:
    virtual void Receive(uchar *Data, int Length);
    virtual void Activate(bool On);
    virtual void Action(void);
private:
    cLiveReceiver(cChannel *Channel, cDevice *Device);
    cDevice  *mDevice;
    cChannel *mChannel;
    cRingBufferLinear *mLiveBuffer;
    cRingBufferLinear *mOutputBuffer;
    cFrameDetector *mFrameDetector;
    cPatPmtGenerator mPatPmtGenerator;
};

#endif	/* _LIVERECEIVER_H */

