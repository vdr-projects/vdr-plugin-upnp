/*
 * plugin.h
 *
 *  Created on: 31.08.2012
 *      Author: savop
 */

#ifndef PLUGIN_H_
#define PLUGIN_H_

#include <string>
#include <map>
#include <list>
#include <stdint.h>

using namespace std;

namespace upnp {

/**
 * Metadata class
 *
 * This class describes the content of a multimedia item like a video or audio file.
 *
 * It contains some general information like the title or a detailed description of
 * the content and one or more resources to the file.
 *
 * Why are multiple resources possible, though I just have that one file? Lets say,
 * you want to display images. It is very useful if there are thumbnail pictures
 * available, which may speed up the display process as the UPnP media renderer may
 * first display those thumbnails rather than the huge image itself. Those images
 * both refer to the same meta information. However, they have different resource
 * information. This may also apply to videos (different resolution, bit rate,
 * whatever...)
 *
 * Every plugin implementor is requested to add as much as available information about
 * a resource, because the media renderer may display them in their user interface,
 * which in turn makes the navigation and identifying the contents much easier.
 *
 * The metadata class itself offers some major properties, which are very common in
 * the VDRs context. However, the implementor may add additional custom information
 * to the list of properties.
 *
 * A Property with the same name but different value may exist multiple times. It is
 * added multiple times to the output. Property attributes are NOT supported. However,
 * there only a handful of attributes, which are likely to be unimportant.
 *
 */
class cMetadata {
public:

  /**
   * Property class
   *
   * This is a basic key-value class, which holds a property (the key)
   * and a value. The value is kept in a string representation. There
   * is a set of getter methods to convert the string in to the corresponding
   * data type. However, there is no type safety given. The implementor
   * must know what the contents of the properties are.
   */
  class Property {
  public:
    Property(){};
    Property(string key, string value);
    Property(string key, long value);
    Property(string key, bool value);

    string GetKey() const;
    string GetString() const;
    long GetInteger() const;
    bool GetBoolean() const;

  private:
    string key;
    string value;
  };

  /**
   * Resource class
   *
   * The resource class contains information about a specific resource.
   *
   * Most of the information are optional except the protocolInfo and the
   * resourceUri.
   *
   * The resourceUri is the locator to the file or stream or whatever. The uri
   * is provided by the resourceProvider plugins and must be understood by the
   * mediaProfiler plugins.
   */
  class Resource {
  public:

    bool SetResourceUri(string resourceUri);
    bool SetProtocolInfo(string protocolInfo);
    bool SetDuration(string duration);
    bool SetResolution(string resolution);
    bool SetBitrate(uint32_t bitrate);
    bool SetSize(uint32_t size);
    bool SetSampleFrequency(uint32_t sampleFrequency);
    bool SetBitsPerSample(uint32_t bitsPerSample);
    bool SetNrAudioChannels(uint32_t nrAudioChannels);
    bool SetColorDepth(uint32_t colorDepth);

    string    GetResourceUri() const { return resourceUri; }
    string    GetProtocolInfo() const { return protocolInfo; }
    string    GetDuration() const { return duration; }
    string    GetResolution() const { return resolution; }
    uint32_t  GetBitrate() const { return bitrate; }
    uint32_t  GetSize() const { return size; }
    uint32_t  GetSampleFrequency() const { return sampleFrequency; }
    uint32_t  GetBitsPerSample() const { return bitsPerSample; }
    uint32_t  GetNrAudioChannels() const { return nrAudioChannels; }
    uint32_t  GetColorDepth() const { return colorDepth; }

  private:
    string    resourceUri;
    string    protocolInfo;
    string    duration;
    string    resolution;
    uint32_t  bitrate;
    uint32_t  size;
    uint32_t  sampleFrequency;
    uint32_t  bitsPerSample;
    uint32_t  nrAudioChannels;
    uint32_t  colorDepth;
  };

  typedef multimap<string, Property> PropertyMap;
  typedef pair<PropertyMap::iterator,PropertyMap::iterator> PropertyRange;
  typedef list<Resource> ResourceList;

  bool AddProperty(Property property);
  bool SetProperty(Property property, int index = 0);

  Property& GetPropertyByKey(string property);
  PropertyRange GetPropertiesByKey(string property);
  PropertyRange GetAllProperties();

  bool SetObjectID(string objectID);
  bool SetObjectIDByUri(string uri);
  string GetObjectID() const { return objectID; }
  bool SetParentID(string parentID);
  bool SetParentIDByUri(string uri);
  string GetParentID() const { return parentID; }
  bool SetTitle(string title);
  string GetTitle() const { return title; }
  bool SetUpnpClass(string upnpClass);
  string GetUpnpClass() const { return upnpClass; }
  bool SetRestricted(bool restricted);
  bool GetRestricted() const { return restricted; }
  bool SetDescription(string description);
  string GetDescription() const { return description; }
  bool SetLongDescription(string longDescription);
  string GetLongDescription() const { return longDescription; }
  bool SetDate(string date);
  string GetDate() const { return date; }
  bool SetLanguage(string language);
  string GetLanguage() const { return language; }
  bool SetChannelNr(int channelNr);
  int GetChannelNr() const { return channelNr; }
  bool SetChannelName(string channelName);
  string GetChannelName() const { return channelName; }
  bool SetScheduledStart(string scheduledStart);
  string GetScheduledStart() const { return scheduledStart; }
  bool SetScheduledEnd(string scheduledEnd);
  string GetScheduledEnd() const { return scheduledEnd; }

private:

  PropertyMap properties;
  ResourceList resources;

  string objectID;
  string parentID;
  string title;
  string upnpClass;
  bool   restricted;
  string description;
  string longDescription;
  string date;
  string language;
  int    channelNr;
  string channelName;
  string scheduledStart;
  string scheduledEnd;

};

#define UPNP_REGISTER_RESOURCE_PROVIDER(cls) extern "C" void *UPnPCreateResourceProvider(void) { return new cls; }

class cUPnPResourceProvider {
public:

  typedef list<string> EntryList;

  virtual ~cUPnPResourceProvider(){};

  /**
   * Returns the schema of this provider
   *
   * Each provider provides a specific uri schema. This
   * is required to distinguish the different kind of file
   * types and resource types.
   *
   * This can be for instance (without ://):
   *
   * Schema   Description
   * vdr://  external http stream
   * rec://   recording
   * file://  any other file, which is not managed by the vdr or the upnp plugin
   *
   */
  virtual string ProvidesSchema() = 0;

  /**
   * Returns the root container of the provider.
   *
   * This function is used to determine the root of the provider.
   * The returned string is prepended to every uri.
   *
   * This results in the following uri:
   *
   * <schema>://<rootContainer>/<path>...
   */
  virtual string GetRootContainer() = 0;

  /**
   * Return a list of the elements in a container.
   *
   * This function returns a list of elements in a container.
   * Each element will be checked by the plugin manager, if it is
   * an item or another container.
   *
   * If an element is a container, the manager will append the
   * element to the URI and creates a new call to this function
   * to recurse through the directory tree.
   *
   * The implementor MUST skip entries, which redirects to the
   * parent container or itself, i.e. containers like "." and "..".
   *
   * Additionally, link loops MUST be avoided as much as possible.
   * The plugin manager will try to detect a loop. However, you
   * should not use links.
   *
   * The given URI is an absolute URI.
   */
  virtual EntryList GetContainerEntries(string uri) = 0;

  /**
   * Checks if the given URI is a container.
   *
   * This function is used by the plugin manager to check if
   * the given absolute URI is a container or not.
   *
   * If it is a container, the plugin manager will try to recurse
   * into that directory and get the elements of it.
   */
  virtual bool IsContainer(string uri) = 0;

  /**
   * Checks if the given URI is a link to another element.
   *
   * The plugin manager uses this function in conjuction with
   * the IsContainer() function to iterate and recurse through
   * the tree. If a link directs to a container (IsContainer()
   * return true), then the plugin manager tries to detect
   * loops.
   *
   * If target is not empty, the target is evaluated and
   * registered as a reference to the original element. Otherwise
   * the element is skipped.
   */
  virtual bool IsLink(string uri, string& target) = 0;

  /**
   * Returns the containerUpdateId of the given container
   *
   * This function called every time a container gets evaluated.
   *
   * This happens at the start of the media server and on every
   * change to that container.
   *
   * This function MUST return an identifier, which enables the
   * plugin manager to distinguish a change of a container.
   *
   * The identifier MUST overcome restarts. If the container was
   * not changes after a restart of the server, the identifier
   * must be equal to the one returned before the server was
   * restarted. If the container was changed, the identifier must
   * be greater than the previous one. The identifier must not
   * decrease in the scope of an unsigned integer, as this may
   * cause clients which were offline for some time to get
   * confused, if the elements differ though the updateId remains
   * the same.
   */
  virtual long GetContainerUpdateId(string uri) = 0;

  /**
   * Returns the meta data of a specific container.
   *
   * This function is used to retrieve meta informatio about a
   * container. It is NOT used to get information about files or
   * resources, as this is done by the media plugins.
   *
   * However, a resource provider may override this function
   * to add more detailed information about a specific container,
   * for instance if it is a special container like a photo album.
   *
   * The implementor is requested to return meta information about
   * the resource provider like title or description when called
   * with the root container uri.
   *
   * The default implementation will fill the required meta data
   * properties with the best matching information, i.e:
   *
   * - Class: object.container
   * - Title: the element name
   * - Restricted: true
   *
   */
  virtual cMetadata* GetMetadata(string uri);

  /**
   * Get the HTTP Uri.
   *
   * This function returns the HTTP Uri if the stream is accessible
   * from an external server via HTTP.
   *
   * The resource MUST be located in the same network than the server.
   *
   * This function returns an empty string by default, which means
   * that the resource provider uses internal access via the file
   * access function Open(), Read(), Close().
   *
   * The implementor of a resource provider MUST either implement
   * this function or the file access methods below.
   */
  virtual string GetHTTPUri(string uri);

  /**
   * Opens a resource
   *
   * If the resource provider uses internal file access it MUST
   * implement the file access methods Open(), i.e. this one, Read()
   * and Close().
   *
   * This opens the given uri and prepares the provider to read
   * from the file. If there is an error while opening the resource,
   * it must return false.
   *
   * The default implementation return false.
   */
  virtual bool Open(string uri);

  /**
   * Reads data from the stream
   *
   * This functions reads up to "buflen" number the number of bytes
   * into the buffer and returns the actual number of bytes read.
   *
   * If there was an error while reading the function MUST return -1.
   * This also applies to when there was no Open() call before reading.
   *
   * If the resource provider reached end of file, it MUST return 0.
   *
   * The default implementation return 0.
   */
  virtual size_t Read(char* buf, size_t bufLen);

  /**
   * Closes the file
   *
   * This function closes the file opened by Open(). It MUST free all
   * acquired memory resources.
   *
   */
  virtual void Close();

protected:

  /**
   * Called whenever a container was changed.
   *
   * This function MUST be called by a resource provider whenever
   * a container was changed during the runtime of the server.
   *
   * The containerUpdateId must equal to the one returned by
   * GetContainerUpdateId() with the same URI. This means,
   * if the provider detected a change of a container, this function
   * is called. Then, the plugin manager might call
   * GetContainerUpdateId() with the URI to retrieve the Id again.
   *
   * The following rules MUST be applied:
   *
   * - A change must be signaled if a file or directory was
   *    - created
   *    - modified
   *    - deleted
   *    - moved
   * - If a file or directory was created, the parent container
   *   URI was changed. Therefore, this URI is signaled with
   *   an appropriate updateID.
   * - If a file was modified, the parent container must be
   *   signaled.
   * - A file is modified, when its contents or metadata have
   *   changed.
   * - If a container was modified, the container itself must be
   *   signaled. The parent container is kept untouched.
   * - If a file or container was deleted, the parent container
   *   must be signaled.
   * - If a file or container is moved, two changes must be
   *   signaled:
   *    - change of the parent container, where the file or container
   *      was moved from
   *    - change of the parent container, where the file or container
   *      was moved to
   */
  void OnContainerUpdate(string uri, long containerUpdateId);

};

#define UPNP_REGISTER_MEDIA_PLUGIN(cls) extern "C" void *UPnPCreateMediaProfiler(void) { return new cls; }

class cMediaProfiler {
public:

  virtual ~cMediaProfiler(){};

  /**
   * Tell the manager if this plugin can handle a specific uri schema
   *
   * This function gets called by the plugin manager multiple times to check
   * if the plugin can handle a specific uri schema.
   *
   * This can be for instance (without ://):
   *
   * Schema   Description
   * vdr://   live-tv stream
   * rec://   recording
   * file://  any other file, which is not managed by the vdr or the upnp plugin
   *
   * The plugin must reject all but the schemas it is capable of.
   *
   * @return \b true if the plugin can handle the uri schema, \b false if not.
   */
  virtual bool CanHandleSchema(string schema) = 0;

  /**
   * Get the metadata from a specific resource.
   *
   * This function is called by the plugin manager when the manager scans for
   * files in a container. The implementor must create and fill a mediaResource
   * object which contains all information about the resource.
   *
   * If the plugin is not able to fill the required information about the resource,
   * it MUST return NULL. The plugin manager will then rotate through all the
   * registered plugins until it finds a suitable one.
   *
   * The required information MUST be set in the cMediaResource::Create function
   * correctly. Otherwise the object creation might fail and a NULL pointer will
   * be returned.
   *
   * Additionally, a plugin implementor is advised to add as many as possible
   * metadata about a resource. Especially, information about the contents might
   * help the user.
   *
   * The URI is a absolute path to the resource.
   *
   * The plugin manager tries to reduce the calls of the resources as much as
   * possible. It will only requests new information, if the resource was
   * changed.
   *
   * A change of a resource is indicated by the resource provider.
   *
   * @param uri the absolute path to the resource.
   * @return the media resource information.
   */
  virtual cMetadata* GetMetadata(string uri) = 0;
protected:
};

}  // namespace upnp

#endif /* PLUGIN_H_ */
