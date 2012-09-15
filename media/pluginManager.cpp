/*
 * pluginManager.cpp
 *
 *  Created on: 05.09.2012
 *      Author: savop
 */

#include "../include/plugin.h"
#include "../include/tools.h"
#include <string>

using namespace std;

namespace upnp {

bool cMetadata::SetObjectIDByUri(string uri){
  return SetObjectID(tools::GenerateUUIDFromURL(uri));
}

bool cMetadata::SetParentIDByUri(string uri){
  return SetParentID(tools::GenerateUUIDFromURL(uri));
}

bool cMetadata::SetObjectID(string objectID){
  if(objectID.empty() || objectID.compare("0") == 0) return false;

  this->objectID = objectID;

  return true;
};

bool cMetadata::SetParentID(string parentID){
  if(objectID.compare("-1") == 0) return false;

  this->parentID = parentID;

  return true;
};

bool cMetadata::SetTitle(string title){
  this->title = title;

  return true;
};

bool cMetadata::SetUpnpClass(string upnpClass){
  //TODO: Validiere upnpClass.
  this->upnpClass = upnpClass;

  return true;
};

bool cMetadata::SetRestricted(bool restricted){
  this->restricted = restricted;

  return true;
};

bool cMetadata::SetDescription(string description){
  this->description = description;

  return true;
};

bool cMetadata::SetLongDescription(string longDescription){
  this->longDescription = longDescription;

  return true;
};

bool cMetadata::SetDate(string date){
  this->date = date;

  return true;
};

bool cMetadata::SetLanguage(string language){
  this->language = language;

  return true;
};

bool cMetadata::SetChannelNr(int channelNr){
  this->channelNr = channelNr;

  return true;
};

bool cMetadata::SetChannelName(string channelName){
  this->channelName = channelName;

  return true;
};

bool cMetadata::SetScheduledStart(string scheduledStart){
  this->scheduledStart = scheduledStart;

  return true;
};

bool cMetadata::SetScheduledEnd(string scheduledEnd){
  this->scheduledEnd = scheduledEnd;

  return true;
};


bool cMetadata::AddProperty(Property property){
  properties.insert(pair<string, Property>(property.GetKey(), property));
  return true;
}

bool cMetadata::SetProperty(Property property, int index){
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

cMetadata::PropertyRange cMetadata::GetPropertiesByKey(string property) {
  return properties.equal_range(property);
}

cMetadata::PropertyRange cMetadata::GetAllProperties() {
  PropertyRange range(properties.begin(), properties.end());
  return range;
}

cMetadata::Property& cMetadata::GetPropertyByKey(string property) {
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

cMetadata* cUPnPResourceProvider::GetMetadata(string uri){

  cMetadata* metadata = new cMetadata;

  metadata->SetTitle(uri.substr(uri.find_last_of("/")+1));
  metadata->SetUpnpClass("object.container");
  metadata->SetObjectIDByUri(uri);
  metadata->SetParentIDByUri(uri.substr(0,uri.find_last_of("/")));
  metadata->SetRestricted(true);

  return metadata;

}

string cUPnPResourceProvider::GetHTTPUri(string uri){
  return string();
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

}  // namespace uünü


