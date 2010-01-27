/* 
 * File:   object.h
 * Author: savop
 *
 * Created on 11. September 2009, 20:39
 */

#ifndef _OBJECT_H
#define	_OBJECT_H

#include "database.h"
#include "../common.h"
#include "util.h"
#include <string.h>
#include <vdr/tools.h>
#include <map>
#include <vector>
#include <upnp/ixml.h>

/**
 * UPnP Object ID
 *
 * This is a UPnP Object ID representation.
 */
struct cUPnPObjectID {
    int _ID;                    ///< The UPnP Object ID
    /**
     * Constructor
     *
     * Creates invalid ID
     */
    cUPnPObjectID():_ID(-1){}
    /**
     * Constructor
     *
     * Creates from long integer
     */
    cUPnPObjectID(
        long ID         ///< new ID
    ){ _ID = (int)ID; }
    /**
     * Constructor
     *
     * Creates from integer
     */
    cUPnPObjectID(
        int ID          ///< new ID
    ){ _ID = ID; }
    /** Set the object ID */
    cUPnPObjectID &operator=(
        long ID         ///< new ID
    ){ _ID = ID; return *this; }
    /** @overload cUPnPObjectID &operator=(long ID) */
    cUPnPObjectID &operator=(
        int ID          ///< new ID
    ){ _ID = ID; return *this; }
    /** @overload cUPnPObjectID &operator=(long ID) */
    cUPnPObjectID &operator=(
        const cUPnPObjectID& ID     ///< new ID
    ){ if(this != &ID){ _ID = ID._ID; } return *this; }
    /** Pre increment the ID */
    cUPnPObjectID &operator++(){ _ID++; return *this; }
    /** Post increment the ID */
    cUPnPObjectID operator++(int){ cUPnPObjectID old = *this; _ID++; return old; }
    /** Post decrement the ID */
    cUPnPObjectID operator--(int){ cUPnPObjectID old = *this; _ID--; return old; }
    /** Pre decrement the ID */
    cUPnPObjectID &operator--(){ _ID--; return *this; }
    /** Not equal */
    bool operator!=(
        long ID         ///< compare with this ID
    ){ return _ID != ID; }
    /** Equal */
    bool operator==(
        long ID         ///< compare with this ID
    ){ return _ID == ID; }
    /** @overload bool operator!=(long ID) */
    bool operator!=(
        int ID          ///< compare with this ID
    ){ return _ID != ID; }
    /** @overload bool operator==(long ID) */
    bool operator==(
        int ID          ///< compare with this ID
    ){ return _ID == ID; }
    /** @overload bool operator!=(long ID) */
    bool operator!=(
        const cUPnPObjectID& ID ///< compare with this ID
    ){ return *this == ID; }
    /** @overload bool operator==(long ID) */
    bool operator==(
        const cUPnPObjectID& ID ///< compare with this ID
    ){ return *this == ID; }
    /** Casts to unsigned int */
    operator unsigned int(){ return (unsigned int)_ID; }
    /** Casts to int */
    operator int(){ return _ID; }
    /** Casts to long */
    operator long(){ return (long)_ID; }
    /** Casts to string */
    const char* operator*(){ char* buf; return asprintf(&buf,"%d",_ID)?buf:NULL; }
};

/**
 * Structure of a UPnP Class
 *
 * This represents a UPnP Class
 */
struct cClass {
    cString ID;             ///< The upnp class ID
    bool includeDerived;    ///< flag, to indicate if derived classes are allowed
    /**
     * Compares two classes
     *
     * @param cmp the other class to compare with
     */
    bool operator==(const cClass &cmp){ return (!strcasecmp(cmp.ID,ID) && includeDerived==cmp.includeDerived); }
    /*! @copydoc operator==(const cClass &cmp) */
    bool operator!=(const cClass &cmp){ return !(*this==cmp); }
};

class cUPnPClassObject;
class cUPnPObjectMediator;
class cUPnPContainerMediator;
class cUPnPClassContainer;
class cUPnPResource;

/**
 * List of UPnP Objects
 *
 * This is a cList of UPnP Objects
 * The list can be sorted by using a specific property
 */
class cUPnPObjects : public cList<cUPnPClassObject> {
public:
    cUPnPObjects();
    virtual ~cUPnPObjects();
    /**
     * Sorts the list
     *
     * This sorts the list by a specific property and a certain direction
     */
    void SortBy(
        const char* Property,       ///< the property used for sorting
        bool Descending = false     ///< the direction of the sort
    );
};

/**
 * The UPnP class Object
 *
 * This is a UPnP class Object representation with all its properties.
 */
class cUPnPClassObject : public cListObject {
    friend class cMediaDatabase;
    friend class cUPnPObjectMediator;
    friend class cUPnPClassContainer;
private:
    cUPnPObjectID           mLastID;
    bool                    mDeleted;                   // is this Objected marked as deleted, NOT used yet.
protected:
    time_t                  mLastModified;              ///< The last modification of this property
    cUPnPObjectID           mID;                        ///< The object ID
    cUPnPClassObject*       mParent;                    ///< The parent object
    cString                 mClass;                     ///< Class (Who am I?)
    cString                 mTitle;                     ///< Object title
    cString                 mCreator;                   ///< Creator of this object
    bool                    mRestricted;                ///< Ability of changing metadata?
    int                     mWriteStatus;               ///< Ability of writing resources?
    cList<cUPnPResource>*   mResources;                 ///< The resources of this object
    cHash<cUPnPResource>*   mResourcesID;               ///< The resources of this object as hashmap
    IXML_Document*          mDIDLFragment;              ///< The DIDL fragment of the object
    cString                 mSortCriteria;              ///< The sort criteria to sort with
    bool                    mSortDescending;            ///< The direction of the sort
    cUPnPClassObject();
    /**
     * Set the Object ID
     *
     * This is only allowed by mediators and the media database. Manually editing
     * the object ID may result in unpredictable behavior.
     *
     * @param ID the ObjectID of this object
     * @return returns
     * - \bc 0, if setting was successful
     * - \bc <0, otherwise
     */
    int setID(cUPnPObjectID ID);
    /**
     * Set the Parent Object
     *
     * This is only allowed by mediators and the media database. Manually editing
     * the parent may result in unpredictable behavior.
     *
     * @param Parent the parent of this object
     * @return returns
     * - \bc 0, if setting was successful
     * - \bc <0, otherwise
     */
    int setParent(cUPnPClassContainer* Parent);
    /**
     * Set the object class
     *
     * This is only allowed by mediators and the media database. Manually editing
     * the object class may result in unpredictable behavior.
     *
     * @param Class the class of this object
     * @return returns
     * - \bc 0, if setting was successful
     * - \bc <0, otherwise
     */
    int setClass(const char* Class);
    /**
     * Set the modification time
     *
     * This sets the last modification time to the current timestamp. This is
     * used to indicate when the object was updated the last time.
     */
    void setModified(void){ this->mLastModified = time(NULL); }
public:
    /**
     * Last modified
     *
     * Returns when the object was modified the last time.
     *
     * @return last modification timestamp
     */
    time_t  modified() const { return this->mLastModified; }
    virtual ~cUPnPClassObject();
    /**
     * Compares a object
     *
     * This compares a given object with this object
     * It uses the SortCriteria to compare them.
     *
     * @return returns
     * - \bc >0, if the object comes after this one
     * - \bc 0, if the objects have the same property
     * - \bc <0, if the object comes before this one
     * @param ListObject the object to compare with
     */
    virtual int Compare(const cListObject& ListObject) const;
    /**
     * Get the properties of the object
     *
     * This returns a property list with all the properties which can be obtained
     * or set with \c getProperty or \c setProperty.
     *
     * @return a stringlist with the properties
     */
    virtual cStringList* getPropertyList();
    /**
     * Gets a property
     *
     * Returns the value of a specified property. The value is converted into a
     * string.
     *
     * @return returns
     * - \bc true, if the property exists
     * - \bc false, otherwise
     * @param Property the property which should be returned
     * @param Value the value of that property
     */
    virtual bool getProperty(const char* Property, char** Value) const ;
    /**
     * Sets a property
     *
     * Sets the value of a specified property. The value is converted from string
     * into the propper data type
     *
     * @return returns
     * - \bc true, if the property exists
     * - \bc false, otherwise
     * @param Property the property which should be set
     * @param Value the value of that property
     */
    virtual bool setProperty(const char* Property, const char* Value);
    /**
     * Converts to container
     *
     * This will convert the object into a container if it is one. If not, it
     * returns \bc NULL.
     *
     * @return returns
     * - \bc NULL, if it is not a container
     * - a container representation of this object
     */
    virtual cUPnPClassContainer* getContainer(){ return NULL; }
    /**
     * Create the DIDL fragment
     *
     * This creates the DIDL-Lite fragment of the object. The DIDL is written to the
     * specified \em IXML document. The details of the output can be controlled via
     * the filter stringlist
     *
     * @return the DIDL fragment of the object
     * @param Document the IXML document where to write the contents
     * @param Filter the string list with the filter criteria
     */
    virtual IXML_Node* createDIDLFragment(IXML_Document* Document, cStringList* Filter) = 0;
    /**
     * Is this a container?
     *
     * Returns if this object is a container or not
     *
     * @return returns
     * - \bc true, if it is a container
     * - \bc false, otherwise
     */
    bool isContainer(){ return this->getContainer()==NULL?false:true; }
    /**
     * Set the sort criteria
     *
     * This sets a certain criteria which the object can be compared with.
     *
     * @param Property the property to sort after
     * @param Descending sort the objects in descending order
     */
    void setSortCriteria(const char* Property, bool Descending = false);
    /**
     * Clears the sort criteria
     *
     * Clears the property of the sort criteria and sets the descending flag to
     * false.
     */
    void clearSortCriteria();
    /******* Setter *******/
    /**
     * Set the title
     *
     * This sets the title of the object. It is a required metadata information.
     * It must not be \bc NULL or an empty string.
     *
     * @return returns
     * - \bc 0, if setting was successful
     * - \bc <0, otherwise
     * @param Title the title of the object
     */
    int setTitle(const char* Title);
    /**
     * Set the creator
     *
     * The creator of an object is primarily the creator or owner of the object
     *
     * @return returns
     * - \bc 0, if setting was successful
     * - \bc <0, otherwise
     * @param Creator the creator of the object
     */
    int setCreator(const char* Creator);
    /**
     * Set the restriction
     *
     * This sets the restriction flag. If the object is restricted, no modifications
     * to its metadata by the user are allowed.
     *
     * @return returns
     * - \bc 0, if setting was successful
     * - \bc <0, otherwise
     * @param Restricted \bc true, to disallow modification, \bc false to allow it
     */
    int setRestricted(bool Restricted);
    /**
     * Set the write status
     *
     * This sets the write status of a resource. With this indicator, you can set
     * the modifiabilty of resources by a control point.
     *
     * @return returns
     * - \bc 0, if setting was successful
     * - \bc <0, otherwise
     * @param Status the write status
     */
    int setWriteStatus(int Status);
    /**
     * Set the resources
     *
     * This sets the list of resources of an object. The list usally contain a
     * single resource. However, multiple resources a also very common.
     *
     * @return returns
     * - \bc 0, if setting was successful
     * - \bc <0, otherwise
     * @param Resources the resource list of this object
     */
    int setResources(cList<cUPnPResource>* Resources);
    /**
     * Add resource to list
     *
     * This adds the specified resource to the resource list of the object
     *
     * @return returns
     * - \bc 0, if setting was successful
     * - \bc <0, otherwise
     * @param Resource the resource to be added
     */
    int addResource(cUPnPResource* Resource);
    /**
     * Remove resource from list
     *
     * This removes the specified resource from the resource list of the object
     *
     * @return returns
     * - \bc 0, if setting was successful
     * - \bc <0, otherwise
     * @param Resource the resource to be removed
     */
    int removeResource(cUPnPResource* Resource);
    /******* Getter *******/
    /**
     * Get the object ID
     *
     * This returns the object ID of the object.
     *
     * @return the object ID
     */
    cUPnPObjectID getID() const { return this->mID; }
    /**
     * Get the parent ID
     *
     * This returns the ID of the parent container object, associated with this object.
     * It is \bc -1, if the object is the root object.
     *
     * @return the parent ID
     */
    cUPnPObjectID getParentID() const { return this->mParent?this->mParent->getID():cUPnPObjectID(-1); }
    /**
     * Get the parent object
     *
     * This returns the parent container object, associated with this object. It is
     * \bc NULL, if the object is the root object.
     *
     * @return the parent object
     */
    cUPnPClassContainer* getParent() const { return (cUPnPClassContainer*)this->mParent; }
    /**
     * Get the title
     *
     * This returns the title of the object. This may be the title of an item or
     * the folder name in case of a container.
     *
     * @return the title of the object
     */
    const char* getTitle() const { return this->mTitle; }
    /**
     * Get the object class
     *
     * This returns the object class of the object. The classes are defined by
     * the UPnP Working Committee. However, custom classes which are derived from
     * a standardized class are also possible.
     *
     * @return the class of the object
     */
    const char* getClass() const { return this->mClass; }
    /**
     * Get the creator
     *
     * This returns the creator of the object. Usually, this is the primary
     * content creator or the owner of the object
     *
     * @return the creator of the object
     */
    const char* getCreator() const { return this->mCreator; }
    /**
     * Is the resource restricted?
     *
     * Returns \bc true, if the object is restricted or \bc false, otherwise.
     * When the object is restricted, then modifications to the metadata of the
     * object are disallowed.
     *
     * @return returns
     * - \bc true, if the object is restricted
     * - \bc false, otherwise
     */
    bool        isRestricted() const { return this->mRestricted; }
    /**
     * Get write status
     *
     * This returns the write status of the object. It gives information, if the
     * resource is modifiable.
     *
     * @return the write status
     */
    int getWriteStatus() const { return this->mWriteStatus; }
    /**
     * Get a resource by its ID
     *
     * Returns the resource with the specified resource ID.
     *
     * @return the resource by ID
     * @param ResourceID the resource ID of the demanded resource
     */
    cUPnPResource* getResource(unsigned int ResourceID) const { return this->mResourcesID->Get(ResourceID); }
    /**
     * Get the resources
     *
     * This returns a list with resources associated with this object.
     *
     * @return the resources of this object
     */
    cList<cUPnPResource>* getResources() const { return this->mResources; }
};

/**
 * The UPnP class Item
 *
 * This is a UPnP class Item representation with all its properties.
 */
class cUPnPClassItem : public cUPnPClassObject {
    friend class cMediaDatabase;
    friend class cUPnPObjectMediator;
    friend class cUPnPItemMediator;
protected:
//    cUPnPObjectID             mReferenceID;
    cUPnPClassItem*           mReference;                                       ///< The reference item
    /**
     * Constructor of an item
     *
     * This creates a new instance of an item
     */
    cUPnPClassItem();
public:
    virtual ~cUPnPClassItem(){};
    virtual cStringList* getPropertyList();
    virtual IXML_Node* createDIDLFragment(IXML_Document* Document, cStringList* Filter);
    virtual bool setProperty(const char* Property, const char* Value);
    virtual bool getProperty(const char* Property, char** Value) const;
    /******** Setter ********/
    /**
     * Set a reference item
     *
     * This sets a reference item. Its comparable with symlinks in *nix systems
     * @return returns
     * - \bc 0, if setting was successful
     * - \bc <0, otherwise
     * @param Reference the reference item
     */
    int setReference(cUPnPClassItem* Reference);
    /******** Getter ********/
    /**
     * Get the referenced item
     *
     * This returns the referenced item of this item
     *
     * @return the referenced item
     */
    cUPnPClassItem* getReference() const { return this->mReference; }
    /**
     * Get the reference ID
     *
     * This returns the object ID of the referenced item or \b -1, if
     * no reference exists.
     *
     * @return the reference ID
     */
    cUPnPObjectID   getReferenceID() const { return this->mReference?this->mReference->getID():cUPnPObjectID(-1); }
};

typedef std::vector<cClass> tClassVector;

/**
 * The UPnP class Container
 *
 * This is a UPnP class Container representation with all its properties.
 */
class cUPnPClassContainer : public cUPnPClassObject {
    friend class cMediaDatabase;
    friend class cUPnPObjectMediator;
    friend class cUPnPContainerMediator;
protected:
    cString                     mContainerType;                                 ///< DLNA container type
    tClassVector                mSearchClasses;                                 ///< Classes which are searchable
    tClassVector                mCreateClasses;                                 ///< Classes which are creatable
    bool                        mSearchable;                                    ///< Is the Container searchable?
    unsigned int                mUpdateID;                                      ///< The containerUpdateID
    cUPnPObjects*               mChildren;                                      ///< List of children
    cHash<cUPnPClassObject>*    mChildrenID;                                    ///< List of children as hash map
    /**
     * Update the container
     *
     * This performs an update, which acutally increases the containerUpdateID.
     */
    void update();
    /**
     * Sets the containerUpdateID
     *
     * This method should only be used when the containerUpdateID is loaded from
     * the database.
     *
     * @return returns
     * - \bc 0, if setting was successful
     * - \bc <0, otherwise
     * @param UID the containerUpdateID
     */
    int setUpdateID(unsigned int UID);
    /**
     * Constructor of a container
     *
     * This creates a new instance of a container
     */
    cUPnPClassContainer();
public:
    virtual ~cUPnPClassContainer();
    virtual cStringList* getPropertyList();
    virtual IXML_Node* createDIDLFragment(IXML_Document* Document, cStringList* Filter);
    virtual bool setProperty(const char* Property, const char* Value);
    virtual bool getProperty(const char* Property, char** Value) const;
    virtual cUPnPClassContainer* getContainer(){ return this; }
    /**
     * Add a child
     *
     * This adds the specified child to this container. The parent container of the
     * child will be set to this container.
     *
     * @param Object the child to be added
     */
    void addObject(cUPnPClassObject* Object);
    /**
     * Remove a child
     *
     * This removes the specified child from the list of children. The child will
     * also loose its parent container, so that there is no link between left.
     *
     * @param Object the child to be removed
     */
    void removeObject(cUPnPClassObject* Object);
    /**
     * Get a child by ID
     *
     * Returns the child, which is specified by the \c ObjectID.
     *
     * @return the child with the specified ID
     * @param ID the \c ObjectID of the child
     */
    cUPnPClassObject* getObject(cUPnPObjectID ID) const;
    /**
     * Get the list of children
     *
     * This returns a list of the children of the container.
     *
     * @return the list of children
     */
    cUPnPObjects* getObjectList() const { return this->mChildren; }
    /**
     * Add a search class
     *
     * This adds a search class to the search classes vector
     *
     * @return returns
     * - \bc 0, if adding was successful
     * - \bc <0, otherwise
     * @param SearchClass the new class to be added
     */
    int addSearchClass(cClass SearchClass);
    /**
     * Remove a search class
     *
     * This removes a search class from the search classes vector
     *
     * @return returns
     * - \bc 0, if deleting was successful
     * - \bc <0, otherwise
     * @param SearchClass the class to be deleted
     */
    int delSearchClass(cClass SearchClass);
    /**
     * Add a create class
     *
     * This adds a create class to the create classes vector
     *
     * @return returns
     * - \bc 0, if adding was successful
     * - \bc <0, otherwise
     * @param CreateClass the new class to be added
     */
    int addCreateClass(cClass CreateClass);
    /**
     * Remove a create class
     *
     * This removes a create class from the create classes vector
     *
     * @return returns
     * - \bc 0, if deleting was successful
     * - \bc <0, otherwise
     * @param CreateClass the class to be deleted
     */
    int delCreateClass(cClass CreateClass);
    /******** Setter ********/
    /**
     * Set the DLNA container type
     *
     * This sets the DLNA container type. It must be a valid container type value.
     *
     * @return returns
     * - \bc 0, if setting was successful
     * - \bc <0, otherwise
     * @param Type the DLNA container type
     */
    int setContainerType(const char* Type);
    /**
     * Sets the search classes
     *
     * This sets the search classes, which allows the user to search only for
     * these classes in the current container and its children. If the vector
     * is empty the search can return any match. If the additional flag \bc
     * derived is set, then also any derived classes are matched.
     *
     * @return returns
     * - \bc 0, if setting was successful
     * - \bc <0, otherwise
     * @param SearchClasses a vector container the allowed search classes
     */
    int setSearchClasses(std::vector<cClass> SearchClasses);
    /**
     * Sets the create classes
     *
     * This sets the create classes, which allows the user to create new objects
     * in this container, if \em restricted is \bc false.
     *
     * @return returns
     * - \bc 0, if setting was successful
     * - \bc <0, otherwise
     * @param CreateClasses a vector containing the create classes
     */
    int setCreateClasses(std::vector<cClass> CreateClasses);
    /**
     * Sets the searchable flag
     *
     * This sets the searchable flag, which allows or disallows search on this
     * container.
     *
     * @return returns
     * - \bc 0, if setting was successful
     * - \bc <0, otherwise
     * @param Searchable \bc true, to enable or \bc false, to disable searching
     */
    int setSearchable(bool Searchable);
    /******** Getter ********/
    /**
     * Get the DLNA container type
     *
     * This returns the DLNA container type. Currently there are only these possible
     * values beside \bc NULL:
     * - \bc TUNER_1_0
     *
     * @return the DLNA container type
     */
    const char*   getContainerType() const { return this->mContainerType; }
    /**
     * Get the search classes
     *
     * This returns a vector container all possible search classes. This are classes,
     * which can be used for searching in this container.
     *
     * @return a vector with all search classes
     */
    const tClassVector* getSearchClasses() const { return &(this->mSearchClasses); }
    /**
     * Get the create classes
     *
     * This returns a vector containing all possible create classes. This are classes,
     * which can be created in this container. For instance a TV container can only create
     * items of the class VideoBroadcast. The vector is empty when creation of new items
     * by the user is not allowed.
     *
     * @return a vector with create classes
     */
    const tClassVector* getCreateClasses() const { return &(this->mCreateClasses); }
    /**
     * Is this container searchable
     *
     * This returns \bc true, if the container can be search via \em Search or
     * \bc false, otherwise.
     *
     * @return returns
     * - \bc true, if the container is searchable
     * - \bc false, otherwise
     */
    bool          isSearchable() const { return this->mSearchable; }
    /**
     * Get the number of children
     *
     * This returns the total number of children of this container
     *
     * @return the number of childen
     */
    unsigned int getChildCount() const { return this->mChildren->Count(); }
    /**
     * Get the containerUpdateID
     *
     * This returns the containerUpdateID
     *
     * @return the containerUpdateID of this container
     */
    unsigned int getUpdateID() const { return this->mUpdateID; }
    /**
     * Has the container been updated?
     *
     * This returns \bc true, if the container was recently updated or
     * \bc false, otherwise
     *
     * @return returns
     * - \bc true, if the container was updated
     * - \bc false, otherwise
     */
    bool         isUpdated();
};

/**
 * The UPnP class VideoItem
 *
 * This is a UPnP class VideoItem representation with all its properties.
 */
class cUPnPClassVideoItem : public cUPnPClassItem {
    friend class cMediaDatabase;
    friend class cUPnPObjectMediator;
    friend class cUPnPVideoItemMediator;
protected:
    cString             mGenre;                                                 ///< Genre of the video
    cString             mDescription;                                           ///< Description
    cString             mLongDescription;                                       ///< a longer description
    cString             mPublishers;                                            ///< CSV of Publishers
    cString             mLanguage;                                              ///< RFC 1766 Language code
    cString             mRelations;                                             ///< Relation to other contents
    cString             mProducers;                                             ///< CSV of Producers
    cString             mRating;                                                ///< Rating (for parential control)
    cString             mActors;                                                ///< CSV of Actors
    cString             mDirectors;                                             ///< CSV of Directors
    /**
     * Constructor of a video item
     *
     * This creates a new instance of a video item
     */
    cUPnPClassVideoItem();
public:
    virtual ~cUPnPClassVideoItem();
    virtual IXML_Node* createDIDLFragment(IXML_Document* Document, cStringList* Filter);
    virtual cStringList* getPropertyList();
    virtual bool setProperty(const char* Property, const char* Value);
    virtual bool getProperty(const char* Property, char** Value) const;
    /******** Setter ********/
    /**
     * Set a long description
     *
     * A long description may hold information about the content or the story
     * of a video
     *
     * @return returns
     * - \bc 0, if setting was successful
     * - \bc <0, otherwise
     * @param LongDescription the content or story of a video
     */
    int setLongDescription(const char* LongDescription);
    /**
     * Set a description
     *
     * A description may hold short information about the content or the story
     * of a video. Unlike a long description, this contains just a very short
     * brief like a subtitle or the episode title.
     *
     * @return returns
     * - \bc 0, if setting was successful
     * - \bc <0, otherwise
     * @param Description the description of a video
     */
    int setDescription(const char* Description);
    /**
     * Set the publishers
     *
     * This is a CSV list of publishers, who distributes the video.
     *
     * @return returns
     * - \bc 0, if setting was successful
     * - \bc <0, otherwise
     * @param Publishers a CSV list of publishers
     */
    int setPublishers(const char* Publishers);
    /**
     * Set a genre
     *
     * This is a CSV list of genre of a video. This may be something like
     * "Western" or "SciFi". Actually, there is no standardized rule for
     * a genre name, which results in an ambiguous definition of certain
     * genre, like Thriller and Horror.
     *
     * @return returns
     * - \bc 0, if setting was successful
     * - \bc <0, otherwise
     * @param Genre a CSV list of genre
     */
    int setGenre(const char* Genre);
    /**
     * Set the language
     *
     * This sets the language of a video. It is defined by RFC 1766.
     * A valid language definition is \em "de-DE" or \em "en-US".
     *
     * @see http://www.ietf.org/rfc/rfc1766.txt
     * @return returns
     * - \bc 0, if setting was successful
     * - \bc <0, otherwise
     * @param Language the language (RFC 1766)
     */
    int setLanguage(const char* Language);
    /**
     * Sets a relation URL
     *
     * This sets a CSV list of relation URLs, where to find additional
     * information about the movie. The URLs may not contain commas and they
     * must be properly escaped as in RFC 2396
     *
     * @see http://www.ietf.org/rfc/rfc2396.txt
     * @return returns
     * - \bc 0, if setting was successful
     * - \bc <0, otherwise
     * @param Relations a CSV list with relations
     */
    int setRelations(const char* Relations);
    /**
     * Sets the directors
     *
     * This sets a CSV list of directors.
     *
     * @return returns
     * - \bc 0, if setting was successful
     * - \bc <0, otherwise
     * @param Directors a CSV list of directors
     */
    int setDirectors(const char* Directors);
    /**
     * Sets the actors
     *
     * This sets a CSV list of actors in a video. This usually contain the main actors.
     * However, also other actors appearing in the video can be mentioned here.
     *
     * @return returns
     * - \bc 0, if setting was successful
     * - \bc <0, otherwise
     * @param Actors a CSV list of actors
     */
    int setActors(const char* Actors);
    /**
     * Sets the producers
     *
     * This sets a CSV list of producers of a video. These are the people who are
     * involed in the production of a video
     *
     * @return returns
     * - \bc 0, if setting was successful
     * - \bc <0, otherwise
     * @param Producers a CSV list of producers
     */
    int setProducers(const char* Producers);
    /**
     * Sets the rating
     *
     * This is a rating, which can be used for parential control issues.
     *
     * @see http://en.wikipedia.org/wiki/Motion_picture_rating_system
     * @return returns
     * - \bc 0, if setting was successful
     * - \bc <0, otherwise
     * @param Rating the rating of a video
     */
    int setRating(const char* Rating);
    /******** Getter ********/
    /**
     * Get the genres
     *
     * This returns a CSV list of genre
     *
     * @return the genre of a video
     */
    const char* getGenre() const { return this->mGenre; }
    /**
     * Get the long description
     *
     * This returns the long description of a video
     *
     * @return the long description of a video
     */
    const char* getLongDescription() const { return this->mLongDescription; }
    /**
     * Get the description
     *
     * This returns the description of a video
     *
     * @return the description of a video
     */
    const char* getDescription() const { return this->mDescription; }
    /**
     * Get the publishers
     *
     * This returns a CSV list of publishers of the video
     *
     * @return a CSV list of publishers
     */
    const char* getPublishers() const { return this->mPublishers; }
    /**
     * Get the language
     *
     * This returns the language of the video
     *
     * @return the language
     */
    const char* getLanguage() const { return this->mLanguage; }
    /**
     * Get the relations
     *
     * This returns a CSV list of relation URLs.
     *
     * @return a CSV list of relation URLs
     */
    const char* getRelations() const { return this->mRelations; }
    /**
     * Get the actors
     *
     * This returns a CSV list of actors in the video
     *
     * @return a CSV list of actors
     */
    const char* getActors() const { return this->mActors; }
    /**
     * Get the producers
     *
     * This returns a CSV list of producers of a video
     *
     * @return a CSV list of producers
     */
    const char* getProducers() const { return this->mProducers; }
    /**
     * Get the directors
     *
     * This returns a CSV list of directors
     *
     * @return a CSV list of directors
     */
    const char* getDirectors() const { return this->mDirectors; }
    /**
     * Get the rating
     *
     * This returns the rating used for parental control.
     *
     * @return the rating of a video
     */
    const char* getRating() const { return this->mRating; }
};

/**
 * The UPnP class Movie
 *
 * This is a UPnP class Movie representation with all its properties.
 */
class cUPnPClassMovie : public cUPnPClassVideoItem {
    friend class cMediaDatabase;
    friend class cUPnPObjectMediator;
    friend class cUPnPMovieMediator;
protected:
    int          mDVDRegionCode;                ///< The Region code of the movie (0 - 8)
    cString      mStorageMedium;                ///< The storage medium where the movie is stored
    /**
     * Constructor of a movie
     *
     * This creates a new instance of a movie
     */
    cUPnPClassMovie();
public:
    virtual ~cUPnPClassMovie();
    //virtual cString createDIDLFragment(cStringList* Filter);
    virtual cStringList* getPropertyList();
    virtual bool setProperty(const char* Property, const char* Value);
    virtual bool getProperty(const char* Property, char** Value) const;
    /******** Setter ********/
    /**
     * Sets the DVD region code
     * 
     * For more information on this, see http://en.wikipedia.org/wiki/DVD_region_code
     * 
     * The integer representation for \em ALL is 9.
     *
     * @see http://en.wikipedia.org/wiki/DVD_region_code
     * @return returns
     * - \bc 0, if setting was successful
     * - \bc <0, otherwise
     * @param RegionCode the region code of this movie
     */
    int setDVDRegionCode(int RegionCode);
    /**
     * Sets the storage medium
     *
     * This will set the storage medium, where the movie resides. Valid media
     * are defined in \link common.h \endlink
     *
     * @see common.h
     * @return returns
     * - \bc 0, if setting was successful
     * - \bc <0, otherwise
     * @param StorageMedium the medium where the movie is located
     */
    int setStorageMedium(const char* StorageMedium);
    /******** Getter ********/
    /**
     * Get the DVD region code
     *
     * This returns the DVD region code. For more information,
     * see http://en.wikipedia.org/wiki/DVD_region_code
     *
     * The integer representation for \em ALL is 9.
     *
     * @see http://en.wikipedia.org/wiki/DVD_region_code
     * @return the DVD region code
     */
    int getDVDRegionCode() const { return this->mDVDRegionCode; }
    /**
     * Get the storage medium
     *
     * This returns the storage medium, where the movie resides.
     *
     * @return the storage medium
     */
    const char* getStorageMedium() const { return this->mStorageMedium; }
};

/**
 * The UPnP class VideoBroadcast
 *
 * This is a UPnP class VideoBroadcast representation with all its properties.
 */
class cUPnPClassVideoBroadcast : public cUPnPClassVideoItem {
    friend class cMediaDatabase;
    friend class cUPnPObjectMediator;
    friend class cUPnPVideoBroadcastMediator;
protected:
    cString             mIcon;                  ///< The channel icon of the channel
    cString             mRegion;                ///< The region where the channel can be received
    int                 mChannelNr;             ///< The channel number
    cString             mChannelName;           ///< The channel name or provider name
    /**
     * Constructor of a video broadcast
     *
     * This creates a new instance of a video broadcast
     */
    cUPnPClassVideoBroadcast();
public:
    virtual ~cUPnPClassVideoBroadcast();
    virtual IXML_Node* createDIDLFragment(IXML_Document* Document, cStringList* Filter);
    virtual cStringList* getPropertyList();
    virtual bool setProperty(const char* Property, const char* Value);
    virtual bool getProperty(const char* Property, char** Value) const;
    /******** Setter ********/
    /**
     * Set the channel icon
     *
     * This sets the channel icon of this channel. The resource must be a valid
     * URI which can be obtained via the internal webserver
     *
     * @return returns
     * - \bc 0, if setting was successful
     * - \bc <0, otherwise
     * @param IconURI the URI to the icon file
     */
    int setIcon(const char* IconURI);
    /**
     * Set the channel region
     *
     * This sets the region of a channel, where it can be received
     *
     * @return returns
     * - \bc 0, if setting was successful
     * - \bc <0, otherwise
     * @param Region the location where the channel can be received
     */
    int setRegion(const char* Region);
    /**
     * Set channel number
     *
     * This sets the channel number, so that it can be used for directly navigation
     * or channel up and down navigation respectively.
     *
     * @return returns
     * - \bc 0, if setting was successful
     * - \bc <0, otherwise
     * @param ChannelNr the channel number
     */
    int setChannelNr(int ChannelNr);
    /**
     * Set the channel name
     *
     * This sets the channel name or the provider of the channel.
     *
     * @return returns
     * - \bc 0, if setting was successful
     * - \bc <0, otherwise
     * @param ChannelName the channel name
     */
    int setChannelName(const char* ChannelName);
    /******** Getter ********/
    /**
     * Get the channel icon
     *
     * This returns the channel icon of the channel.
     *
     * @return the channel icon
     */
    const char* getIcon() const { return this->mIcon; }
    /**
     * Get the region
     *
     * This returns the region, where the channel can be received
     *
     * @return the channel region
     */
    const char* getRegion() const { return this->mRegion; }
    /**
     * Get the channel number
     *
     * This returns the channel number
     *
     * @return the channel number
     */
    int getChannelNr() const { return this->mChannelNr; }
    /**
     * Get the channel name
     *
     * This returns the channel name or provider name respectively
     *
     * @return the channel name
     */
    const char* getChannelName() const { return this->mChannelName; }
};

/**
 * Mediator interface
 *
 * This is an interface for mediators used to communicate with the database.
 * A mediator is applied to get, create, save or delete an UPnP object.
 */
class cMediatorInterface {
public:
    virtual ~cMediatorInterface(){};
    /**
     * Creates an object
     *
     * This creates a new UPnP object with the specific title and the restriction.
     *
     * @return the newly created object
     * @param Title the title of that object
     * @param Restricted the restriction of the object
     */
    virtual cUPnPClassObject* createObject(const char* Title, bool Restricted) = 0;
    /**
     * Get an object
     *
     * Retrieves a UPnP object from the database and stores its information in the
     * object. The object is obtained via its object ID.
     *
     * @return the object, found in the database
     * @param ID the object ID
     */
    virtual cUPnPClassObject* getObject(cUPnPObjectID ID) = 0;
    /**
     * Saves the object
     *
     * This saves the object in the database by updating the values in the database
     * with those in the object.
     *
     * @return returns
     * - \bc <0, in case of an error
     * - \bc 0, otherwise
     * @param Object the object to be saved
     */
    virtual int saveObject(cUPnPClassObject* Object) = 0;
    /**
     * Deletes the object
     *
     * This deletes the object in the database by removing all its children and then
     * deleting the contents from the database
     *
     * @return returns
     * - \bc <0, in case of an error
     * - \bc 0, otherwise
     * @param Object the object to be deleted
     */
    virtual int deleteObject(cUPnPClassObject* Object) = 0;
    /**
     * Clears the object
     *
     * This clears the object, i.e. all its children will be removed and deleted
     * from the database
     *
     * @return returns
     * - \bc <0, in case of an error
     * - \bc 0, otherwise
     * @param Object the object to be cleared
     */
    virtual int clearObject(cUPnPClassObject* Object) = 0;
};

typedef std::map<const char*, cMediatorInterface*, strCmp> tMediatorMap;

/**
 * The object factory
 *
 * This factory can create, delete, clear or save UPnP objects. It uses mediators
 * to communicate with the persistance database to load or persist the objects.
 *
 * If a new type of object shall be stored in the database an according mediator
 * is needed, which knows the internal database structure. It must implement the
 * cMediatorInterface class to work with this factory.
 */
class cUPnPObjectFactory {
private:
    static cUPnPObjectFactory*            mInstance;
    cSQLiteDatabase*                      mDatabase;
    tMediatorMap                          mMediators;
    cMediatorInterface* findMediatorByID(cUPnPObjectID ID);
    cMediatorInterface* findMediatorByClass(const char* Class);
    cUPnPObjectFactory();
public:
    /**
     * Return the instance of the factory
     *
     * This returns the instance of the factory. When the media database is
     * initialized successfully, it usally has all known mediators already
     * registered.
     *
     * @return the instance of the factory
     */
    static cUPnPObjectFactory* getInstance();
    /**
     * Register a mediator
     *
     * This registers a new mediator by the associated class. The mediator
     * must implement the cMediatorInterface class to be used with this
     * factory.
     *
     * @param UPnPClass the class of which the mediator is associated to
     * @param Mediator the mediator itself
     */
    void registerMediator(const char* UPnPClass, cMediatorInterface* Mediator);
    /**
     * Unregisters a mediator
     *
     * This unregisters a mediator if it is not needed anylonger. If the optional
     * parameter \c freeMediator is set, the object instance will be free'd after
     * removing it from the list.
     *
     * @param UPnPClass the class of the associated mediator
     * @param freeMediator flag to indicate if the mediator shall be free'd after removing
     */
    void unregisterMediator(const char* UPnPClass, bool freeMediator=true);
    /**
     * @copydoc cMediatorInterface::createObject(const char* Title, bool Restricted)
     *
     * @param UPnPClass the class of the new object
     */
    cUPnPClassObject* createObject(const char* UPnPClass, const char* Title, bool Restricted=true);
    /*! @copydoc cMediatorInterface::getObject(cUPnPObjectID ID) */
    cUPnPClassObject* getObject(cUPnPObjectID ID);
    /*! @copydoc cMediatorInterface::saveObject(cUPnPClassObject* Object) */
    int saveObject(cUPnPClassObject* Object);
    /*! @copydoc cMediatorInterface::deleteObject(cUPnPClassObject* Object) */
    int deleteObject(cUPnPClassObject* Object);
    /*! @copydoc cMediatorInterface::clearObject(cUPnPClassObject* Object) */
    int clearObject(cUPnPClassObject* Object);
};

class cMediaDatabase;

/**
 * Object Mediator
 *
 * This is the interface between the objects and the database. It is possible to
 * create new objects, stores objects in the database as well as removing them from
 * it.
 */
class cUPnPObjectMediator : public cMediatorInterface {
protected:
    cSQLiteDatabase*        mDatabase;                  ///< the SQLite 3 database wrapper
    cMediaDatabase*         mMediaDatabase;             ///< the media database
    /**
     * Constructor of object mediator
     *
     * This constructs a new object mediator. This is actually not allowed because
     * it is prohibited to create instances of the UPnP class Object
     */
    cUPnPObjectMediator(
        cMediaDatabase* MediaDatabase                   ///< the media database
    );
    /**
     * Initializes an object
     *
     * This initializes an object, which means, that it will be created in the database with
     * the required details.
     *
     * @return returns
     * - \bc <0, in case of an error
     * - \bc 0, otherwise
     */
    virtual int initializeObject(
        cUPnPClassObject* Object,                       ///< the object to be initialized
        const char* Class,                              ///< the class of the object
        const char* Title,                              ///< the title of the object
        bool Restricted                                 ///< restriction of the object
    );
    /**
     * Store the object in the database
     *
     * This stores the information of an object in the database
     *
     * @return returns
     * - \bc <0, in case of an error
     * - \bc 0, otherwise
     * @param Object the object to be saved
     */
    virtual int objectToDatabase(cUPnPClassObject* Object);
    /**
     * Loads an object from database
     *
     * This loads an object from the database
     *
     * @return returns
     * - \bc <0, in case of an error
     * - \bc 0, otherwise
     * @param Object the object to be loaded
     * @param ID the object ID of that object
     */
    virtual int databaseToObject(cUPnPClassObject* Object, cUPnPObjectID ID);
public:
    virtual ~cUPnPObjectMediator();
    /*! @copydoc cMediatorInterface::createObject(const char* Title, bool Restricted) */
    virtual cUPnPClassObject* createObject(const char* Title, bool Restricted);
    /*! @copydoc cMediatorInterface::getObject(cUPnPObjectID ID) */
    virtual cUPnPClassObject* getObject(cUPnPObjectID ID);
    /*! @copydoc cMediatorInterface::saveObject(cUPnPClassObject* Object) */
    virtual int saveObject(cUPnPClassObject* Object);
    /*! @copydoc cMediatorInterface::deleteObject(cUPnPClassObject* Object) */
    virtual int deleteObject(cUPnPClassObject* Object);
    /*! @copydoc cMediatorInterface::clearObject(cUPnPClassObject* Object) */
    virtual int clearObject(cUPnPClassObject* Object);
};

/**
 * Item Mediator
 *
 * This is the interface between the objects and the database. It is possible to
 * create new objects, stores objects in the database as well as removing them from
 * it.
 */
class cUPnPItemMediator : public cUPnPObjectMediator {
protected:
    /*! @copydoc cUPnPObjectMediator::objectToDatabase(cUPnPClassObject* Object) */
    virtual int objectToDatabase(cUPnPClassObject* Object);
    /*! @copydoc cUPnPObjectMediator::databaseToObject(cUPnPClassObject* Object, cUPnPObjectID ID) */
    virtual int databaseToObject(cUPnPClassObject* Object, cUPnPObjectID ID);
public:
    /**
     * Constructor of item mediator
     *
     * This creates a new item mediator with which it is possible to create new
     * instances of Item objects.
     *
     * @param MediaDatabase the media database
     */
    cUPnPItemMediator(cMediaDatabase* MediaDatabase);
    virtual ~cUPnPItemMediator(){};
    /*! @copydoc cUPnPObjectMediator::createObject(const char* Title, bool Restricted) */
    virtual cUPnPClassItem* createObject(const char* Title, bool Restricted);
    /*! @copydoc cUPnPObjectMediator::getObject(cUPnPObjectID ID) */
    virtual cUPnPClassItem* getObject(cUPnPObjectID ID);
};

/**
 * VideoItem Mediator
 *
 * This is the interface between the objects and the database. It is possible to
 * create new objects, stores objects in the database as well as removing them from
 * it.
 */
class cUPnPVideoItemMediator : public cUPnPItemMediator {
protected:
    virtual int objectToDatabase(cUPnPClassObject* Object);
    virtual int databaseToObject(cUPnPClassObject* Object, cUPnPObjectID ID);
public:
    /**
     * Constructor of videoitem mediator
     *
     * This creates a new videoitem mediator with which it is possible to create new
     * instances of VideoItem objects.
     *
     * @param MediaDatabase the media database
     */
    cUPnPVideoItemMediator(cMediaDatabase* MediaDatabase);
    virtual ~cUPnPVideoItemMediator(){};
    virtual cUPnPClassVideoItem* createObject(const char* Title, bool Restricted);
    virtual cUPnPClassVideoItem* getObject(cUPnPObjectID ID);
};

/**
 * VideoBroadcast Mediator
 *
 * This is the interface between the objects and the database. It is possible to
 * create new objects, stores objects in the database as well as removing them from
 * it.
 */
class cUPnPVideoBroadcastMediator : public cUPnPVideoItemMediator {
protected:
    virtual int objectToDatabase(cUPnPClassObject* Object);
    virtual int databaseToObject(cUPnPClassObject* Object, cUPnPObjectID ID);
public:
    /**
     * Constructor of video broadcast mediator
     *
     * This creates a new video broadcast mediator with which it is possible to create new
     * instances of VideoBroadcast objects.
     *
     * @param MediaDatabase the media database
     */
    cUPnPVideoBroadcastMediator(cMediaDatabase* MediaDatabase);
    virtual ~cUPnPVideoBroadcastMediator(){};
    virtual cUPnPClassVideoBroadcast* createObject(const char* Title, bool Restricted);
    virtual cUPnPClassVideoBroadcast* getObject(cUPnPObjectID ID);
};

/**
 * Movie Mediator
 *
 * This is the interface between the objects and the database. It is possible to
 * create new objects, stores objects in the database as well as removing them from
 * it.
 */
class cUPnPMovieMediator : public cUPnPVideoItemMediator {
protected:
    virtual int objectToDatabase(cUPnPClassObject* Object);
    virtual int databaseToObject(cUPnPClassObject* Object, cUPnPObjectID ID);
public:
    /**
     * Constructor of movie mediator
     *
     * This creates a new movie mediator with which it is possible to create new
     * instances of Movie objects.
     *
     * @param MediaDatabase the media database
     */
    cUPnPMovieMediator(cMediaDatabase* MediaDatabase);
    virtual ~cUPnPMovieMediator(){};
    virtual cUPnPClassMovie* createObject(const char* Title, bool Restricted);
    virtual cUPnPClassMovie* getObject(cUPnPObjectID ID);
};

/**
 * Container Mediator
 *
 * This is the interface between the objects and the database. It is possible to
 * create new objects, stores objects in the database as well as removing them from
 * it.
 */
class cUPnPContainerMediator : public cUPnPObjectMediator {
protected:
    virtual int objectToDatabase(cUPnPClassObject* Object);
    virtual int databaseToObject(cUPnPClassObject* Object, cUPnPObjectID ID);
public:
    /**
     * Constructor of container mediator
     *
     * This creates a new container mediator with which it is possible to create new
     * instances of Container objects.
     *
     * @param MediaDatabase the media database
     */
    cUPnPContainerMediator(cMediaDatabase* MediaDatabase);
    virtual ~cUPnPContainerMediator(){};
    virtual cUPnPClassContainer* createObject(const char* Title, bool Restricted);
    virtual cUPnPClassContainer* getObject(cUPnPObjectID ID);
};

#endif	/* _OBJECT_H */

