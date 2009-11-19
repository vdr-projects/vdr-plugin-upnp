/* 
 * File:   metadata.h
 * Author: savop
 *
 * Created on 28. Mai 2009, 21:14
 */

#ifndef _METADATA_H
#define	_METADATA_H

#include <vdr/tools.h>
#include <vdr/channels.h>
#include <vdr/recording.h>
#include "../common.h"
#include "database.h"
#include "object.h"
#include "resources.h"

/**
 * The result set of a request
 *
 * This contains the results of a previous \e Browse or \e Search request.
 */
struct cUPnPResultSet {
    int mNumberReturned; ///< The number of returned matches
    int mTotalMatches;   ///< The total amount of matches
    const char* mResult; ///< The DIDL-Lite fragment
};

/**
 * The media database
 *
 * This class is the global object manager. It holds every object in a local cache.
 * Only this class is allowed to create new objects.
 *
 * @see cUPnPClassObject
 */
class cMediaDatabase : public cThread {
    friend class cUPnPServer;
    friend class cUPnPObjectMediator;
private:
    unsigned int             mSystemUpdateID;
    cUPnPObjectFactory*      mFactory;
    cHash<cUPnPClassObject>* mObjects;
    cSQLiteDatabase*         mDatabase;
    cUPnPObjectID            mLastInsertObjectID;
    cUPnPObjectID getNextObjectID();
    void cacheObject(cUPnPClassObject* Object);
    int prepareDatabase();
    int loadChannels();
    int loadRecordings();
    void updateChannelEPG();
    void updateRecordings();
    bool init();
    void updateSystemID();
    virtual void Action();
public:
    /**
     * Returns the SystemUpdateID
     *
     * This returns the \e SystemUpdateID. This changes whenever anything changed
     * within the content directory. This value will be sent through the UPnP
     * network every 2 seconds.
     *
     * @return the SystemUpdateID
     */
    unsigned int getSystemUpdateID();
    /**
     * Returns a CSV list with ContainerUpdateIDs
     *
     * This list contains an unordered list of ordered pairs of ContainerID and
     * its ContainerUpdateID. It contains only recent changes which are not yet
     * beeing evented. This means that evented updates will be removed from list.
     *
     * @return CSV list of ContainerUpdateIDs
     */
    const char*  getContainerUpdateIDs();
    /**
     * Constructor
     *
     * This creates an instance of the media database.
     */
    cMediaDatabase();
    virtual ~cMediaDatabase();
    /**
     * Add a Fastfind
     *
     * This creates a \e Fastfind entry. It is a string which can be used to
     * relocate a objectID. Usually this is a file name or another ID with which
     * the related object can be found.
     *
     * @return returns
     * - \bc -1, if the creation was successful
     * - \bc 0, otherwise
     */
    int addFastFind(
        cUPnPClassObject* Object, ///< the object, which should be registered
        const char* FastFind      ///< the string with which the object shall be
                                  ///< relocated
    );
    /**
     * Finds a object by Fastfind
     *
     * This returns the object via the \e Fastfind string. The object must be
     * previosly registered via \c cMediaDatabase::addFastFind().
     *
     * It tries to find the object in the internal object cache. If this fails,
     * the object will be loaded from the database.
     *
     * @see cMediaDatabase::addFastFind
     * @return The object associated with FastFind
     */
    cUPnPClassObject* getObjectByFastFind(
        const char* FastFind ///< the string with which the object shall be
                             ///< relocated
    );
    /**
     * Finds a object by its ObjectID
     *
     * This returns the object via its \e ObjectID.
     *
     * It tries to find the object in the internal object cache. If this fails,
     * the object will be loaded from the database.
     *
     * @return The object associated with FastFind
     */
    cUPnPClassObject* getObjectByID(
        cUPnPObjectID ID ///< The ObjectID of the requested object
    );
    /**
     * Performs a browse on the database
     *
     * This performs a browse request on the database and returns a structure
     * containing the matching count and DIDL-Lite fragement which is sent to
     * the control point.
     *
     * @return returns an integer representing one of the following:
     * - \bc UPNP_CDS_E_INVALID_SORT_CRITERIA, when the sort criteria is malformed
     * - \bc UPNP_CDS_E_CANT_PROCESS_REQUEST, when there is an internal error while
     *                                        processing the request
     * - \bc UPNP_CDS_E_NO_SUCH_OBJECT, when the requested ObjectID does not exist
     * - \bc UPNP_SOAP_E_ACTION_FAILED, when the action failed due any reasons
     * - \bc UPNP_E_SUCCESS, if the request was successful
     */
    int browse(
        OUT cUPnPResultSet** Results,   ///< the result of the request
        IN const char* ID,              ///< the objectID of the request
        IN bool BrowseMetadata,         ///< \b true to browse metadata, \b false otherwise
        IN const char* Filter = "*",    ///< the filter applied to the returned metadata
        IN unsigned int Offset = 0,     ///< the starting offset
        IN unsigned int Count = 0,      ///< maximum count returned
        IN const char* SortCriteria = "" ///< sorts the results before returning them
    );
    /**
     * Performs a search on the database
     *
     * This performs a search request on the database and returns a structure
     * containing the matching count and DIDL-Lite fragement which is sent to
     * the control point.
     * 
     * @note
     * The submitted ID must be a ContainerID. Searches are performed only
     * in this container.
     *
     * @return returns an integer representing one of the following:
     * - \bc UPNP_CDS_E_INVALID_SORT_CRITERIA, when the sort criteria is malformed
     * - \bc UPNP_CDS_E_CANT_PROCESS_REQUEST, when there is an internal error while
     *                                        processing the request
     * - \bc UPNP_CDS_E_NO_SUCH_OBJECT, when the requested ObjectID does not exist
     * - \bc UPNP_SOAP_E_ACTION_FAILED, when the action failed due any reasons
     * - \bc UPNP_E_SUCCESS, if the request was successful
     */
    int search(
        OUT cUPnPResultSet** Results,       ///< the result of the request
        IN const char* ID,                  ///< the ContainerID
        IN const char* Search,              ///< the search string
        IN const char* Filter = "*",        ///< the filter applied to the returned metadata
        IN unsigned int Offset = 0,         ///< the starting offset
        IN unsigned int Count = 0,          ///< maximum count returned
        IN const char* SortCriteria = ""    ///< sorts the results before returning them
    );
};

#endif	/* _METADATA_H */

