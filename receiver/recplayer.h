/* 
 * File:   recplayer.h
 * Author: savop
 *
 * Created on 8. Juni 2009, 11:57
 */

#ifndef _RECPLAYER_H
#define	_RECPLAYER_H

#include "../common.h"
#include "filehandle.h"
#include <vdr/recording.h>

class cRecordingPlayer : cFileHandle {
public:
    static cRecordingPlayer *newInstance(cRecording *Recording);
    virtual ~cRecordingPlayer();
    virtual void open(UpnpOpenFileMode mode);
    virtual int read(char* buf, size_t buflen);
    virtual int write(char* buf, size_t buflen);
    virtual int seek(off_t offset, int origin);
    virtual void close();
private:
    void Scan(void);
    cRecordingPlayer(cRecording *Recording);
    FILE* GetFile(int Index = 0);
    FILE* NextFile(void);
    int SeekInFile(int Index, off_t Offset);
    cRecording *mRecording;
    off_t* mOffsets;
    off_t  mOffset;
    off_t  mTotalLenght;
    int mIndex;
    FILE *mFile;
};

#endif	/* _RECPLAYER_H */

