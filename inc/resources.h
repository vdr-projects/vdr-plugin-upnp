/* 
 * File:   resources.h
 * Author: savop
 *
 * Created on 30. September 2009, 15:17
 */

#ifndef _RESOURCES_H
#define	_RESOURCES_H

#include "database.h"
#include "object.h"
#include <vdr/channels.h>
#include <vdr/recording.h>

class cUPnPResourceMediator;
class cMediaDatabase;

/**
 * UPnP Resource
 *
 * This contains all details about a resource
 */
class cUPnPResource : public cListObject {
    friend class cUPnPResourceMediator;
    friend class cUPnPResources;
private:
    unsigned int mResourceID;
    int     mResourceType;
    cString mResource;
    cString mDuration;
    cString mResolution;
    cString mProtocolInfo;
    cString mContentType;
    cString mImportURI;
    off64_t mSize;
    unsigned int mBitrate;
    unsigned int mSampleFrequency;
    unsigned int mBitsPerSample;
    unsigned int mNrAudioChannels;
    unsigned int mColorDepth;
    cUPnPResource();
public:
    /**
     * Get resource ID
     *
     * Gets the resource ID
     *
     * @return the resource ID
     */
    unsigned int getID() const { return this->mResourceID; }
    /**
     * Get the resources
     *
     * Returns the resource. This is in most cases the file name or resource locator
     * where to find the resource
     *
     * @return the resource string
     */
    const char* getResource() const { return this->mResource; }
    /**
     * Get the duration
     *
     * Returns a date time string with the duration of the resource
     *
     * @return the duration of the resource
     */
    const char* getDuration() const { return this->mDuration; }
    /**
     * Get the resolution
     *
     * Returns the resolution string with the pattern width x height in pixels
     *
     * @return the resolution of the resource
     */
    const char* getResolution() const { return this->mResolution; }
    /**
     * Get the protocol info
     *
     * This returns the protocol info field of a resource
     *
     * @return the protocol info string
     */
    const char* getProtocolInfo() const { return this->mProtocolInfo; }
    /**
     * Get the content type
     *
     * Returns the mime type of the content of the resource
     *
     * @return the content type of the resource
     */
    const char* getContentType() const { return this->mContentType; }
    /**
     * Get the import URI
     *
     * This returns the import URI where the resource was located before importing
     * it
     *
     * @return the import URI
     */
    const char* getImportURI() const { return this->mImportURI; }
    /**
     * Get the resource type
     *
     * This returns the resource type of the resource.
     *
     * @return the resource type
     */
    int         getResourceType() const { return this->mResourceType; }
    /**
     * Get the file size
     *
     * Returns the file size in bytes of the resource or 0 if its unknown or a
     * stream
     *
     * @return the file size
     */
    off64_t     getFileSize() const { return this->mSize; };
    /**
     * Get the last modification
     *
     * This returns the timestamp of the last modification to the file. If it
     * is a stream, then its the current time.
     *
     * @return the timestamp with the last modification of the resource
     */
    time_t      getLastModification() const;
    /**
     * Get the bitrate
     *
     * This returns the bitrate of the resource in bits per second.
     *
     * @return the bitrate of the resource
     */
    unsigned int getBitrate() const { return this->mBitrate; }
    /**
     * Get the sample frequency
     *
     * Returns the sample frequency in samples per second.
     *
     * @return the sample frequency of the resource
     */
    unsigned int getSampleFrequency() const { return this->mSampleFrequency; }
    /**
     * Get the bits per sample
     *
     * Returns the number of bits per sample.
     *
     * @return the bits per sample of the resource
     */
    unsigned int getBitsPerSample() const { return this->mBitsPerSample; }
    /**
     * Get number of audio channels
     *
     * Returns the number of audio channels of the audio stream in a video
     *
     * @return the number of audio channels
     */
    unsigned int getNrAudioChannels() const { return this->mNrAudioChannels; }
    /**
     * Get the color depth
     *
     * Returns the color depth of the resource in pits per pixel
     *
     * @return the color depth of the resource
     */
    unsigned int getColorDepth() const { return this->mColorDepth; }
};

class cUPnPClassObject;
class cUPnPClassItem;
class cUPnPClassVideoItem;
class cUPnPClassVideoBroadcast;

/**
 * The resource manager
 *
 * This manages the resources in an internal cache. It may create a new resource
 * from a channel, a recording or a custom file.
 */
class cUPnPResources {
private:
    cHash<cUPnPResource>*        mResources;
    static cUPnPResources*       mInstance;
    cUPnPResourceMediator*       mMediator;
    cSQLiteDatabase*             mDatabase;
    cUPnPResources();
public:
    /**
     * Fill object with its resources
     *
     * This will load all the resources from the database, which are associated
     * to the given object
     *
     * @param Object the object, which shall be filled
     * @return returns
     * - \bc 0, if loading was successful
     * - \bc <0, otherwise
     */
    int getResourcesOfObject(cUPnPClassObject* Object);
    /**
     * Loads all resources from database
     *
     * This loads all resources from the database into the internal cache.
     *
     * @return returns
     * - \bc 0, if loading was successful
     * - \bc <0, otherwise
     */
    int loadResources();
    /*! @copydoc cUPnPResourceMediator::getResource */
    cUPnPResource* getResource(unsigned int ResourceID);
    virtual ~cUPnPResources();
    /**
     * Get the instance of the resource manager
     *
     * This returns the instance of the resource manager.
     *
     * @return the instance of the manager
     */
    static cUPnPResources* getInstance();
    /**
     * Create resource from channel
     *
     * This creates a new resource from the given channel. It determines what
     * kind of video stream it is and further details if available. It stores
     * the resource in the database after creating it.
     *
     * @param Object the videoBroadcast item which holds the resource
     * @param Channel the VDR TV channel
     * @return returns
     * - \bc 0, if loading was successful
     * - \bc <0, otherwise
     */
    int createFromChannel(cUPnPClassVideoBroadcast* Object, cChannel* Channel);
    /**
     * Create resource from recording
     *
     * This creates a new resource from the given recording. It determines what
     * kind of video stream it is and further details if available. It stores
     * the resource in the database after creating it.
     *
     * @param Object the videoItem item which holds the resource
     * @param Recording the VDR TV recording
     * @return returns
     * - \bc 0, if loading was successful
     * - \bc <0, otherwise
     */
    int createFromRecording(cUPnPClassVideoItem* Object, cRecording* Recording);
    /**
     * Create resource from file
     *
     * This creates a new resource from the given file. It determines all available
     * information about the resource by analizing the content. It stores
     * the resource in the database after creating it.
     *
     * @param Object the item which holds the resource
     * @param File the file name
     * @return returns
     * - \bc 0, if loading was successful
     * - \bc <0, otherwise
     */
    int createFromFile(cUPnPClassItem* Object, cString File);
};

/**
 * The resource mediator
 *
 * This is another mediator which communicates with the database. It manages the
 * resources in the database
 */
class cUPnPResourceMediator {
    friend class cUPnPResources;
private:
    cSQLiteDatabase* mDatabase;
    cUPnPResourceMediator();
    unsigned int getNextResourceID();
public:
    virtual ~cUPnPResourceMediator();
    /**
     * Get a resource by ID
     *
     * This returns a resource by its resource ID
     *
     * @param ResourceID the resource ID of the demanded resource
     * @return the requested resource
     */
    cUPnPResource* getResource(unsigned int ResourceID);
    /**
     * Saves the resource
     *
     * This updates the information in the database with those in the resource
     * object
     *
     * @param Resource the resource which shall be saved
     * @return returns
     * - \bc 0, if saving was successful
     * - \bc <0, if an error occured
     */
    int saveResource(cUPnPClassObject* Object, cUPnPResource* Resource);
    /**
     * Create new resource
     *
     * This creates a new resource and stores the skeleton in the database. The
     * newly created resource will only contain all required information.
     *
     * @param Object the Object which will hold the resource
     * @param ResourceType the type of the resource
     * @param ResourceFile the file or URL, where the resource can be located
     * @param ContentType the mime type of the content
     * @param ProtocolInfo the protocol information of the resource
     * @return the newly created resource
     */
    cUPnPResource* newResource(cUPnPClassObject* Object, int ResourceType, cString ResourceFile, cString ContentType, cString ProtocolInfo);
};

#endif	/* _RESOURCES_H */

