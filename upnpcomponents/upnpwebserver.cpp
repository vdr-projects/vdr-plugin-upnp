/* 
 * File:   upnpwebserver.cpp
 * Author: savop
 * 
 * Created on 30. Mai 2009, 18:13
 */

#include <time.h>
#include <vdr/channels.h>
#include <map>
#include <upnp/upnp.h>
#include "upnpwebserver.h"
#include "../server/server.h"
#include "../receiver/livereceiver.h"
#include "../receiver/recplayer.h"
#include "../misc/search.h"

/* COPIED FROM INTEL UPNP TOOLS */
/*******************************************************************************
 *
 * Copyright (c) 2000-2003 Intel Corporation
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * * Redistributions of source code must retain the above copyright notice,
 * this list of conditions and the following disclaimer.
 * * Redistributions in binary form must reproduce the above copyright notice,
 * this list of conditions and the following disclaimer in the documentation
 * and/or other materials provided with the distribution.
 * * Neither name of Intel Corporation nor the names of its contributors
 * may be used to endorse or promote products derived from this software
 * without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL INTEL OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 * OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 ******************************************************************************/
struct File_Info_
{
  /** The length of the file. A length less than 0 indicates the size
   *  is unknown, and data will be sent until 0 bytes are returned from
   *  a read call. */
  off64_t file_length;

  /** The time at which the contents of the file was modified;
   *  The time system is always local (not GMT). */
  time_t last_modified;

  /** If the file is a directory, {\bf is_directory} contains
   * a non-zero value. For a regular file, it should be 0. */
  int is_directory;

  /** If the file or directory is readable, this contains
   * a non-zero value. If unreadable, it should be set to 0. */
  int is_readable;

  /** The content type of the file. This string needs to be allocated
   *  by the caller using {\bf ixmlCloneDOMString}.  When finished
   *  with it, the SDK frees the {\bf DOMString}. */

  DOMString content_type;

};

struct cWebFileHandle {
    cString      Filename;
    off64_t      Size;
    cFileHandle* FileHandle;
};

/****************************************************
 *
 *  The web server
 *
 *  Handles the virtual directories and the
 *  provision of data
 *
 *  Interface between the channels/recordings of the
 *  VDR and the outer world
 *
 ****************************************************/

cUPnPWebServer::cUPnPWebServer(const char* root) : mRootdir(root) {
}

cUPnPWebServer::~cUPnPWebServer(){
    delete [] this->mRootdir;
}

cUPnPWebServer* cUPnPWebServer::mInstance = NULL;

UpnpVirtualDirCallbacks cUPnPWebServer::mVirtualDirCallbacks = {
    cUPnPWebServer::getInfo,
    cUPnPWebServer::open,
    cUPnPWebServer::read,
    cUPnPWebServer::write,
    cUPnPWebServer::seek,
    cUPnPWebServer::close
};

bool cUPnPWebServer::init(){
    MESSAGE("Initialize callbacks for virtual directories.");

    if(UpnpSetWebServerRootDir(this->mRootdir) == UPNP_E_INVALID_ARGUMENT){
        ERROR("The root directory of the webserver is invalid.");
        return false;
    }
    MESSAGE("Setting up callbacks");

    if(UpnpSetVirtualDirCallbacks(&cUPnPWebServer::mVirtualDirCallbacks) == UPNP_E_INVALID_ARGUMENT){
        ERROR("The virtual directory callbacks are invalid.");
        return false;
    }

    if(UpnpIsWebserverEnabled() == FALSE){
        WARNING("The webserver has not been started. For whatever reason...");
        return false;
    }

    MESSAGE("Add virtual directories.");
    if(UpnpAddVirtualDir(UPNP_DIR_SHARES) == UPNP_E_INVALID_ARGUMENT){
        ERROR("The virtual directory %s is invalid.",UPNP_DIR_SHARES);
        return false;
    }
    return true;
}

bool cUPnPWebServer::uninit(){
    MESSAGE("Disabling the internal webserver");
    UpnpEnableWebserver(FALSE);

    MESSAGE("Closing open file transfers");
}

cUPnPWebServer* cUPnPWebServer::getInstance(const char* rootdir){
    if(cUPnPWebServer::mInstance == NULL)
        cUPnPWebServer::mInstance = new cUPnPWebServer(rootdir);

    if(cUPnPWebServer::mInstance){
        return cUPnPWebServer::mInstance;
    }
    else return NULL;
}

int cUPnPWebServer::getInfo(const char* filename, File_Info* info){
    MESSAGE("Getting information of file '%s'", filename);

    propertyMap Properties;
    int Method;
    int Section;

    if(cPathParser::parse(filename, &Section, &Method, &Properties)){
        switch(Section){
            case 0:
                switch(Method){
                    case UPNP_WEB_METHOD_STREAM:
                        {
                            MESSAGE("Stream request");
                            propertyMap::iterator It = Properties.find("resId");
                            unsigned int ResourceID = 0;
                            if(It == Properties.end()){
                                ERROR("No resourceID for stream request");
                                return -1;
                            }
                            else {
                                ResourceID = (unsigned)atoi(It->second);
                                cUPnPResource* Resource = cUPnPResources::getInstance()->getResource(ResourceID);
                                if(!Resource){
                                    ERROR("No such resource with ID (%d)", ResourceID);
                                    return -1;
                                }
                                else {
                                    File_Info_ finfo;

                                    finfo.content_type = ixmlCloneDOMString(Resource->getContentType());
                                    finfo.file_length = Resource->getFileSize();
                                    finfo.is_directory = 0;
                                    finfo.is_readable = 1;
                                    finfo.last_modified = Resource->getLastModification();
                                    memcpy(info, &finfo, sizeof(File_Info_));

                                    MESSAGE("==== File info of Resource #%d ====", Resource->getID());
                                    MESSAGE("Size: %lld", finfo.file_length);
                                    MESSAGE("Dir: %s", finfo.is_directory?"yes":"no");
                                    MESSAGE("Read: %s", finfo.is_readable?"allowed":"not allowed");
                                    MESSAGE("Last modified: %s", ctime(&(finfo.last_modified)));
                                    MESSAGE("Content-type: %s", finfo.content_type);
                                }
                            }
                        }
                        break;
                    case UPNP_WEB_METHOD_BROWSE:
                    //    break;
                    case UPNP_WEB_METHOD_SHOW:
                    //    break;
                    case UPNP_WEB_METHOD_SEARCH:
                    case UPNP_WEB_METHOD_DOWNLOAD:
                    default:
                        ERROR("Unknown or unsupported method ID (%d)", Method);
                        return -1;
                }
                break;
            default:
                ERROR("Unknown or unsupported section ID (%d).", Section);
                return -1;
        }
    }
    else {
        return -1;
    }

    return 0;
}

UpnpWebFileHandle cUPnPWebServer::open(const char* filename, UpnpOpenFileMode mode){
    MESSAGE("File %s was opened for %s.",filename,mode==UPNP_READ ? "reading" : "writing");

    propertyMap Properties;
    int Method;
    int Section;
    cWebFileHandle* WebFileHandle = NULL;

    if(cPathParser::parse(filename, &Section, &Method, &Properties)){
        switch(Section){
            case 0:
                switch(Method){
                    case UPNP_WEB_METHOD_STREAM:
                        {
                            MESSAGE("Stream request");
                            propertyMap::iterator It = Properties.find("resId");
                            unsigned int ResourceID = 0;
                            if(It == Properties.end()){
                                ERROR("No resourceID for stream request");
                                return NULL;
                            }
                            else {
                                ResourceID = (unsigned)atoi(It->second);
                                cUPnPResource* Resource = cUPnPResources::getInstance()->getResource(ResourceID);
                                if(!Resource){
                                    ERROR("No such resource with ID (%d)", ResourceID);
                                    return NULL;
                                }
                                else {
                                    WebFileHandle = new cWebFileHandle;
                                    WebFileHandle->Filename = Resource->getResource();
                                    WebFileHandle->Size = Resource->getFileSize();
                                    switch(Resource->getResourceType()){
                                        case UPNP_RESOURCE_CHANNEL:
                                            {
                                                char* ChannelID = strtok(strdup(Resource->getResource()),":");
                                                int     AudioID = atoi(strtok(NULL,":"));
                                                MESSAGE("Try to create Receiver for Channel %s with Audio ID %d", ChannelID, AudioID);
                                                cChannel* Channel = Channels.GetByChannelID(tChannelID::FromString(ChannelID));
                                                if(!Channel){
                                                    ERROR("No such channel with ID %s", ChannelID);
                                                    return NULL;
                                                }
                                                cLiveReceiver* Receiver = cLiveReceiver::newInstance(Channel,0);
                                                if(!Receiver){
                                                    ERROR("Unable to tune channel. No available tuners?");
                                                    return NULL;
                                                }
                                                WebFileHandle->FileHandle = Receiver;
                                            }
                                            break;
                                        case UPNP_RESOURCE_RECORDING:
                                        //    break;
                                        case UPNP_RESOURCE_FILE:
                                        //    break;
                                        case UPNP_RESOURCE_URL:
                                        default:
                                            return NULL;
                                    }
                                }
                            }
                        }
                        break;
                    case UPNP_WEB_METHOD_BROWSE:
                    //    break;
                    case UPNP_WEB_METHOD_SHOW:
                    //    break;
                    case UPNP_WEB_METHOD_SEARCH:
                    case UPNP_WEB_METHOD_DOWNLOAD:
                    default:
                        ERROR("Unknown or unsupported method ID (%d)", Method);
                        return NULL;
                }
                break;
            default:
                ERROR("Unknown or unsupported section ID (%d).", Section);
                return NULL;
        }
    }
    else {
        return NULL;
    }
    MESSAGE("Open the file handle");
    WebFileHandle->FileHandle->open(mode);
    return (UpnpWebFileHandle)WebFileHandle;
}

int cUPnPWebServer::write(UpnpWebFileHandle fh, char* buf, size_t buflen){
    cWebFileHandle* FileHandle = (cWebFileHandle*)fh;
    MESSAGE("Writing to %s", *FileHandle->Filename);
    return FileHandle->FileHandle->write(buf, buflen);
}

int cUPnPWebServer::read(UpnpWebFileHandle fh, char* buf, size_t buflen){
    cWebFileHandle* FileHandle = (cWebFileHandle*)fh;
    MESSAGE("Reading from %s", *FileHandle->Filename);
    return FileHandle->FileHandle->read(buf, buflen);
}

int cUPnPWebServer::seek(UpnpWebFileHandle fh, off_t offset, int origin){
    cWebFileHandle* FileHandle = (cWebFileHandle*)fh;
    MESSAGE("Seeking on %s", *FileHandle->Filename);
    return FileHandle->FileHandle->seek(offset, origin);
}

int cUPnPWebServer::close(UpnpWebFileHandle fh){
    cWebFileHandle *FileHandle = (cWebFileHandle *)fh;
    MESSAGE("Closing file %s", *FileHandle->Filename);
    FileHandle->FileHandle->close();
    delete FileHandle->FileHandle;
    delete FileHandle;
    return 0;
}