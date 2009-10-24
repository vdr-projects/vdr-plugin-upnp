/* 
 * File:   upnpwebserver.h
 * Author: savop
 *
 * Created on 30. Mai 2009, 18:13
 */

#ifndef _UPNPWEBSERVER_H
#define	_UPNPWEBSERVER_H

#include "../common.h"
#include <upnp/upnp.h>

class cUPnPWebServer {
    friend class cUPnPServer;
private:
    static cUPnPWebServer *mInstance;
    static UpnpVirtualDirCallbacks mVirtualDirCallbacks;
    const char* mRootdir;
    cUPnPWebServer(const char* root = "/");
protected:
    bool enable(bool enable);
    static void free();
public:
    bool init();
    static cUPnPWebServer* getInstance(const char* rootdir = "/");
    virtual ~cUPnPWebServer();
//};

    /****************************************************
     *
     *  The callback functions for the webserver
     *
     ****************************************************/
    /**
     * Retrieve file information
     *
     * Returns file related information for an virtual directory file
     *
     * @return 0 on success, -1 otherwise
     * @param filename The filename of which the information is gathered
     * @param info     The File_Info structure with the data
     */
    static int getInfo(const char* filename, struct File_Info* info);
    /**
     * Opens a virtual directory file
     *
     * Opens a file in a virtual directory with the specified mode.
     *
     * Possible modes are:
     * - UPNP_READ :    Opens the file for reading
     * - UPNP_WRITE:    Opens the file for writing
     *
     * It returns a file handle to the opened file, NULL otherwise
     *
     * @return FileHandle to the opened file, NULL otherwise
     * @param filename The file to open
     * @param mode UPNP_WRITE for writing, UPNP_READ for reading.
     */
    static UpnpWebFileHandle open(const char* filename, UpnpOpenFileMode mode);
    /**
     * Reads from the opened file
     *
     * Reads <code>buflen</code> bytes from the file and stores the content
     * to the buffer
     *
     * Returns 0 no more bytes read (EOF)
     *         >0 bytes read from file
     *
     * @return number of bytes read, 0 on EOF
     * @param fh the file handle of the opened file
     * @param buf the buffer to write the bytes to
     * @param buflen the maximum count of bytes to read
     *
     */
    static int read(UpnpWebFileHandle fh, char* buf, size_t buflen);
    /**
     * Writes to the opened file
     *
     * Writes <code>buflen</code> bytes from the buffer and stores the content
     * in the file
     *
     * Returns >0 bytes wrote to file, maybe less the buflen in case of write
     * errors
     *
     * @return number of bytes read, 0 on EOF
     * @param fh the file handle of the opened file
     * @param buf the buffer to read the bytes from
     * @param buflen the maximum count of bytes to write
     *
     */
    static int write(UpnpWebFileHandle fh, char* buf, size_t buflen);
    /**
     * Seek in the file
     *
     * Seeks in the opened file and sets the file pointer to the specified offset
     *
     * Returns 0 on success, non-zero value otherwise
     *
     * @return 0 on success, non-zero value otherwise
     * @param fh the file handle of the opened file
     * @param offset a negative oder positive value which moves the pointer
     *        forward or backward
     * @param origin SEEK_CUR, SEEK_END or SEEK_SET
     *
     */
    static int seek(UpnpWebFileHandle fh, off_t offset, int origin);
    /**
     * Closes the file
     *
     * closes the opened file
     *
     * Returns 0 on success, non-zero value otherwise
     *
     * @return 0 on success, non-zero value otherwise
     * @param fh the file handle of the opened file
     *
     */
    static int close(UpnpWebFileHandle fh);
};

#endif	/* _UPNPWEBSERVER_H */

