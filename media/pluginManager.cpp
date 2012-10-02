/*
 * pluginManager.cpp
 *
 *  Created on: 05.09.2012
 *      Author: savop
 */

#include "../include/plugin.h"
#include "../include/tools.h"
#include "../include/pluginManager.h"
#include <string>
#include <dlfcn.h>
#include <dirent.h>

using namespace std;

namespace upnp {

cMetadata::ValidatorMap cMetadata::validators;

class PropertyValidator {
private:
  string key;
public:
  PropertyValidator(string key) : key(key) {
    cMetadata::RegisterPropertyValidator(this);
  }
  virtual ~PropertyValidator(){}
  virtual string GetPropertyKey() const { return key; };
  virtual bool Validate(const cMetadata::Property& property) = 0;
};

void cMetadata::RegisterPropertyValidator(PropertyValidator* validator){
  string key = validator->GetPropertyKey();

  validators[key] = validator;
}

bool cMetadata::SetObjectIDByUri(const string& uri){
  return SetProperty(Property(property::object::KEY_OBJECTID, tools::GenerateUUIDFromURL(uri)));
}

bool cMetadata::SetParentIDByUri(const string& uri){
  return SetProperty(Property(property::object::KEY_PARENTID, tools::GenerateUUIDFromURL(uri)));
}

bool cMetadata::AddProperty(const Property& property){
  string key = property.GetKey();

  // Try to find a validator
  PropertyValidator* validator = validators[key];
  // If there is one and it fails to validate the property, return false.
  // Otherwise ignore it.
  if(validator && !validator->Validate(property)) return false;

  if(properties.find(key) != properties.end()){
    if(key.compare(property::object::KEY_CHANNEL_NAME) == 0 ||
       key.compare(property::object::KEY_CHANNEL_NR) == 0 ||
       key.compare(property::object::KEY_CLASS) == 0 ||
       key.compare(property::object::KEY_CREATOR) == 0 ||
       key.compare(property::object::KEY_DATE) == 0 ||
       key.compare(property::object::KEY_DESCRIPTION) == 0 ||
       key.compare(property::object::KEY_LANGUAGE) == 0 ||
       key.compare(property::object::KEY_LONG_DESCRIPTION) == 0 ||
       key.compare(property::object::KEY_OBJECTID) == 0 ||
       key.compare(property::object::KEY_PARENTID) == 0 ||
       key.compare(property::object::KEY_RESTRICTED) == 0 ||
       key.compare(property::object::KEY_SCHEDULED_END) == 0 ||
       key.compare(property::object::KEY_SCHEDULED_START) == 0 ||
       key.compare(property::object::KEY_TITLE) == 0)
    {
      esyslog("UPnP\tProperty '%s' already exist!", key.c_str());
      return false;
    }
  }

  properties.insert(pair<string, Property>(property.GetKey(), property));
  return true;
}

bool cMetadata::SetProperty(const Property& property, int index){
  // Try to find a validator
  PropertyValidator* validator = validators[property.GetKey()];
  // If there is one and it fails to validate the property, return false.
  // Otherwise ignore it.
  if(validator && !validator->Validate(property)) return false;

  PropertyRange ret = properties.equal_range(property.GetKey());

  // No property with the given name found. Let's add it to the map.
  if(ret.first == ret.second) return AddProperty(property);

  int i = 0;
  for(PropertyMap::iterator it = ret.first; it != ret.second; ++it){
    if(i == index){
      (*it).second = property;
      return true;
    }
    ++i;
  }

  return false;

}

cMetadata::PropertyRange cMetadata::GetPropertiesByKey(const string& property) {
  return properties.equal_range(property);
}

cMetadata::PropertyRange cMetadata::GetAllProperties() {
  PropertyRange range(properties.begin(), properties.end());
  return range;
}

cMetadata::Property& cMetadata::GetPropertyByKey(const string& property) {
  return (*properties.find(property)).second;
}

cMetadata::Property::Property(string key, bool val)
: key(key)
{
  value = val ? "true" : "false";
}

cMetadata::Property::Property(string key, string value)
: key(key)
, value(value)
{
}

cMetadata::Property::Property(string key, long val)
: key(key)
{
  value = tools::ToString(val);
}

string cMetadata::Property::GetString() const {
  return value;
}

bool cMetadata::Property::GetBoolean() const {
  return value.compare("true") == 0 ? true : false;
}

long cMetadata::Property::GetInteger() const {
  return atol(value.c_str());
}

string cMetadata::Property::GetKey() const {
  return key;
}

bool cMetadata::Resource::SetResourceUri(string resourceUri){
  this->resourceUri = resourceUri;
  return true;
}

bool cMetadata::Resource::SetProtocolInfo(string protocolInfo){
  this->protocolInfo = protocolInfo;
  return true;
}

bool cMetadata::Resource::SetDuration(string duration){
  this->duration = duration;
  return true;
}

bool cMetadata::Resource::SetResolution(string resolution){
  this->resolution = resolution;
  return true;
}

bool cMetadata::Resource::SetBitrate(uint32_t bitrate){
  this->bitrate = bitrate;
  return true;
}

bool cMetadata::Resource::SetSize(uint32_t size){
  this->size = size;
  return true;
}

bool cMetadata::Resource::SetSampleFrequency(uint32_t sampleFrequency){
  this->sampleFrequency = sampleFrequency;
  return true;
}

bool cMetadata::Resource::SetBitsPerSample(uint32_t bitsPerSample){
  this->bitsPerSample = bitsPerSample;
  return true;
}

bool cMetadata::Resource::SetNrAudioChannels(uint32_t nrAudioChannels){
  this->nrAudioChannels = nrAudioChannels;
  return true;
}

bool cMetadata::Resource::SetColorDepth(uint32_t colorDepth){
  this->colorDepth = colorDepth;
  return true;
}

class ClassValidator : public PropertyValidator {
public:
  ClassValidator() : PropertyValidator(property::object::KEY_CLASS) {}
  virtual bool Validate(const cMetadata::Property& property){
    string value = property.GetString();

    if(value.find("object.container", 0) == 0 ||
       value.find("object.item", 0) == 0)
    {
      return true;
    }
    else
    {
      return false;
    }
  }
} ClassValidatorInst;

cMetadata cUPnPResourceProvider::GetMetadata(string uri){

  cMetadata metadata;

  metadata.SetObjectIDByUri(uri);
  metadata.SetParentIDByUri(uri.substr(0,uri.find_last_of("/")));
  metadata.SetProperty(cMetadata::Property(property::object::KEY_TITLE, uri.substr(uri.find_last_of("/")+1)));
  metadata.SetProperty(cMetadata::Property(property::object::KEY_CLASS, "object.container"));
  metadata.SetProperty(cMetadata::Property(property::object::KEY_RESTRICTED, true));

  return metadata;

}

string cUPnPResourceProvider::GetHTTPUri(string uri){
  return string();
}

bool cUPnPResourceProvider::Seekable() const {
  return false;
}

bool cUPnPResourceProvider::Open(string uri){
  return false;
}

size_t cUPnPResourceProvider::Read(char* buf, size_t bufLen){
  return -1;
}

bool cUPnPResourceProvider::Seek(size_t offset, int origin){
  return false;
}

void cUPnPResourceProvider::Close(){
}

upnp::cPluginManager::cPluginManager(cMediaManager* manager)
: manager(manager)
{}

cPluginManager::~cPluginManager(){}

#define UPNPPLUGIN_PREFIX "libupnp-"
#define SO_INDICATOR      ".so."

bool upnp::cPluginManager::LoadPlugins(const string& directory){

  DIR* dirHandle;
  struct dirent* dirEntry;

  if((dirHandle = opendir(directory.c_str())) == NULL){
    esyslog("UPnP\tLoading directory '%s' failed. Errno: %d", directory.c_str(), errno);
    return false;
  }

  string filename;
  while ((dirEntry = readdir(dirHandle)) != NULL) {
    filename = dirEntry->d_name;
    if(filename.compare(".") || filename.compare("..")){
      if(filename.find(UPNPPLUGIN_PREFIX,0) == 0 &&
         filename.find(UPNPPLUGIN_VERSION) != string::npos &&
         filename.find(SO_INDICATOR) != string::npos)
      {
        boost::shared_ptr<DLL> dll(new DLL(directory + "/" + filename));
        if(dll->Load())
          dlls.push_back(dll);
      }
    }
  }
  closedir(dirHandle);

  return true;
}

upnp::cPluginManager::DLL::DLL(const string& f)
: file(f)
, handle(NULL)
, provider(NULL)
, profiler(NULL)
{
}

bool upnp::cPluginManager::DLL::Load(){
  if(handle)
    return true;

  handle = dlopen(file.c_str(), RTLD_NOW);

  const char* error = dlerror();
  if(!error){
    provider = (FunctionPtr)dlsym(handle, "UPnPCreateResourceProvider");
    if (!(error = dlerror())){
      isyslog("UPnP\tFound provider in %s", file.c_str());
      return true;
    } else {
      dsyslog("UPnP\tError: %s", error);
    }

    profiler = (FunctionPtr)dlsym(handle, "UPnPCreateMediaProfiler");
    if (!(error = dlerror())){
      isyslog("UPnP\tFound profiler in %s", file.c_str());
      return true;
    } else {
      dsyslog("UPnP\tError: %s", error);
    }
  } else {
    dsyslog("UPnP\tError: %s", error);
  }

  isyslog("UPnP\tInvalid library '%s', no valid symbols found.", file.c_str());

  return false;
}

upnp::cPluginManager::DLL::~DLL()
{
  if(handle)
    dlclose(handle);
}

}  // namespace upnp
