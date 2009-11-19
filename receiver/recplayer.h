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

/**
 * The recording player
 *
 * This class provides the ability to play VDR records. The difference between
 * usual files and VDR recording files is, that recordings are possibly splitted
 * into multiple files. The class will scan those files and tries to dynamically
 * navigate in them like it would do, if it is a single file.
 *
 */
class cRecordingPlayer : cFileHandle {
public:
    /**
     * Get a new instance of a recording player
     *
     * This returns a new instance of a recording player which plays the
     * specified VDR recording.
     *
     * @param Recording the recording to play
     * @return the new instance of the recording player
     */
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

