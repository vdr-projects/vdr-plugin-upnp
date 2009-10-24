/* 
 * File:   filehandle.h
 * Author: savop
 *
 * Created on 15. Oktober 2009, 10:49
 */

#ifndef _FILEHANDLE_H
#define	_FILEHANDLE_H

#include <upnp/upnp.h>
#include "../common.h"

class cFileHandle {
public:
    virtual void open(UpnpOpenFileMode mode) = 0;
    virtual int read(char* buf, size_t buflen) = 0;
    virtual int write(char* buf, size_t buflen) = 0;
    virtual int seek(off_t offset, int whence) = 0;
    virtual void close() = 0;
    virtual ~cFileHandle(){};
private:
};

#endif	/* _FILEHANDLE_H */

