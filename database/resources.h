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
    int saveResource(cUPnPResource* Resource);
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

