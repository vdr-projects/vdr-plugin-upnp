/*
 * contentDirectory.cpp
 *
 *  Created on: 27.08.2012
 *      Author: savop
 */

#include "../include/contentDirectory.h"
#include "../include/media/mediaManager.h"
#include "../include/server.h"
#include <vdr/i18n.h>
#include <upnp/upnptools.h>
#include <sstream>

namespace upnp {

#define UPNP_CDS_ACTION_SEARCHCAPABILITIES  "GetSearchCapabilities"
#define UPNP_CDS_ACTION_SORTCAPABILITIES    "GetSortCapabilities"
#define UPNP_CDS_ACTION_SYSTEMUPDATEID      "GetSystemUpdateID"
#define UPNP_CDS_ACTION_BROWSE              "Browse"
#define UPNP_CDS_ACTION_SEARCH              "Search"
#define UPNP_CDS_ACTION_CREATEOBJECT        "CreateObject"
#define UPNP_CDS_ACTION_DESTROYOBJECT       "DestroyObject"
#define UPNP_CDS_ACTION_UPDATEOBJECT        "UpdateObject"
#define UPNP_CDS_ACTION_IMPORTRESOURCE      "ImportResource"
#define UPNP_CDS_ACTION_EXPORTRESOURCE      "ExportResource"
#define UPNP_CDS_ACTION_STOPTRANSFERRES     "StopTransferResource"
#define UPNP_CDS_ACTION_TRANSFERPROGRESS    "GetTransferProgress"
#define UPNP_CDS_ACTION_DELETERESOURCE      "DeleteResource"
#define UPNP_CDS_ACTION_CREATEREFERENCE     "CreateReference"

cContentDirectory::cContentDirectory()
: cUPnPService(
    cUPnPService::Description(
        "urn:schemas-upnp-org:service:ContentDirectory:1",
        "urn:upnp-org:serviceId:ContentDirectory",
        "cds_scpd.xml",
        "cds_control",
        "cds_event"
        ))
{

}

cContentDirectory::~cContentDirectory(){
  Cancel(1);
}

void cContentDirectory::Init(cMediaServer* server, UpnpDevice_Handle deviceHandle){
  cUPnPService::Init(server, deviceHandle);

  Start();
}

int cContentDirectory::Subscribe(Upnp_Subscription_Request* Request){
  IXML_Document* PropertySet = NULL;

  /* The system update ID */
  UpnpAddToPropertySet(&PropertySet, "SystemUpdateID",
      tools::ToString(mMediaServer->GetManager().GetSystemUpdateID()).c_str());
  /* The container update IDs as CSV list */
  UpnpAddToPropertySet(&PropertySet, "ContainerUpdateIDs",
      tools::IdListToCSV(mMediaServer->GetManager().GetContainerUpdateIDs()).c_str());
  /* The transfer IDs, which are not supported, i.e. empty */
  UpnpAddToPropertySet(&PropertySet, "TransferIDs", "");
  // Accept subscription
  int ret = UpnpAcceptSubscriptionExt(this->mDeviceHandle, Request->UDN, Request->ServiceId, PropertySet, Request->Sid);

  if(ret != UPNP_E_SUCCESS){
    esyslog("UPnP\tSubscription failed (Error code: %d)", ret);
  }

  ixmlDocument_free(PropertySet);
  return ret;
}

void cContentDirectory::Action(){
  int Retry = 5;
  LOG(5, "Start Content directory thread");
  while(this->Running()){
    IXML_Document* PropertySet = NULL;

    /* The system update ID */
    UpnpAddToPropertySet(&PropertySet, "SystemUpdateID",
        tools::ToString(mMediaServer->GetManager().GetSystemUpdateID()).c_str());
    /* The container update IDs as CSV list */
    UpnpAddToPropertySet(&PropertySet, "ContainerUpdateIDs",
        tools::IdListToCSV(mMediaServer->GetManager().GetContainerUpdateIDs()).c_str());

    int ret = UpnpNotifyExt(this->mDeviceHandle, this->mMediaServer->GetDeviceUUID().c_str(),
                            this->mServiceDescription.serviceID.c_str(), PropertySet);
    ixmlDocument_free(PropertySet);

    if(ret != UPNP_E_SUCCESS){
      Retry--;
      esyslog("UPnP\tState change notification failed (Error code: %d)",ret);
      esyslog("UPnP\t%d of %d notifications failed", (5-Retry), 5);
    }
    else {
      Retry = 5;
    }
    if (!Retry){
      esyslog("UPnP\tMaximum retries of notifications reached. Stopping...");
      this->Cancel();
    }
    // Sleep 2 seconds
    sleep.Wait(2000);
  }
}

void cContentDirectory::Stop(){
  this->sleep.Signal();
  this->Cancel(2);
}

int cContentDirectory::Execute(Upnp_Action_Request* request){
  if (request == NULL) {
    esyslog("UPnP\tCMS Action Handler - request is null");
    return UPNP_E_BAD_REQUEST;
  }

  if(!strcmp(request->ActionName, UPNP_CDS_ACTION_SEARCHCAPABILITIES))
    return this->GetSearchCapabilities(request);
  if(!strcmp(request->ActionName, UPNP_CDS_ACTION_SORTCAPABILITIES))
    return this->GetSortCapabilities(request);
  if(!strcmp(request->ActionName, UPNP_CDS_ACTION_SYSTEMUPDATEID))
    return this->GetSystemUpdateID(request);
  if(!strcmp(request->ActionName, UPNP_CDS_ACTION_BROWSE))
    return this->Browse(request);
  if(!strcmp(request->ActionName, UPNP_CDS_ACTION_SEARCH))
    return this->Search(request);
  if(!strcmp(request->ActionName, UPNP_CDS_ACTION_CREATEOBJECT))
      return this->CreateObject(request);
  if(!strcmp(request->ActionName, UPNP_CDS_ACTION_DESTROYOBJECT))
      return this->DestroyObject(request);
  if(!strcmp(request->ActionName, UPNP_CDS_ACTION_UPDATEOBJECT))
      return this->UpdateObject(request);
  if(!strcmp(request->ActionName, UPNP_CDS_ACTION_IMPORTRESOURCE))
      return this->ImportResource(request);
  if(!strcmp(request->ActionName, UPNP_CDS_ACTION_EXPORTRESOURCE))
      return this->ExportResource(request);
  if(!strcmp(request->ActionName, UPNP_CDS_ACTION_STOPTRANSFERRES))
      return this->StopTransferResource(request);
  if(!strcmp(request->ActionName, UPNP_CDS_ACTION_TRANSFERPROGRESS))
      return this->GetTransferProgress(request);
  if(!strcmp(request->ActionName, UPNP_CDS_ACTION_DELETERESOURCE))
      return this->DeleteResource(request);
  if(!strcmp(request->ActionName, UPNP_CDS_ACTION_CREATEREFERENCE))
      return this->CreateReference(request);

  return UPNP_E_BAD_REQUEST;
}

int cContentDirectory::CreateReference(Upnp_Action_Request* request){
  SetError(request, UPNP_SOAP_E_ACTION_NOT_IMPLEMENTED);
  return request->ErrCode;
}

int cContentDirectory::DeleteResource(Upnp_Action_Request* request){
  SetError(request, UPNP_SOAP_E_ACTION_NOT_IMPLEMENTED);
  return request->ErrCode;
}

int cContentDirectory::GetTransferProgress(Upnp_Action_Request* request){
  SetError(request, UPNP_SOAP_E_ACTION_NOT_IMPLEMENTED);
  return request->ErrCode;
}

int cContentDirectory::StopTransferResource(Upnp_Action_Request* request){
  SetError(request, UPNP_SOAP_E_ACTION_NOT_IMPLEMENTED);
  return request->ErrCode;
}

int cContentDirectory::ExportResource(Upnp_Action_Request* request){
  SetError(request, UPNP_SOAP_E_ACTION_NOT_IMPLEMENTED);
  return request->ErrCode;
}

int cContentDirectory::ImportResource(Upnp_Action_Request* request){
  SetError(request, UPNP_SOAP_E_ACTION_NOT_IMPLEMENTED);
  return request->ErrCode;
}

int cContentDirectory::UpdateObject(Upnp_Action_Request* request){
  SetError(request, UPNP_SOAP_E_ACTION_NOT_IMPLEMENTED);
  return request->ErrCode;
}

int cContentDirectory::DestroyObject(Upnp_Action_Request* request){
  SetError(request, UPNP_SOAP_E_ACTION_NOT_IMPLEMENTED);
  return request->ErrCode;
}

int cContentDirectory::CreateObject(Upnp_Action_Request* request){
  SetError(request, UPNP_SOAP_E_ACTION_NOT_IMPLEMENTED);
  return request->ErrCode;
}

int cContentDirectory::Search(Upnp_Action_Request* request){
  cMediaManager::SearchRequest searchRequest;

  if(this->ParseStringValue(request->ActionRequest, "ContainerID", searchRequest.objectID)){
    esyslog("UPnP\tInvalid arguments. ObjectID missing or wrong");
    this->SetError(request, UPNP_SOAP_E_INVALID_ARGS);
    return request->ErrCode;
  }

  if(this->ParseStringValue(request->ActionRequest, "BrowseFlag", searchRequest.searchCriteria)){
    esyslog("UPnP\tInvalid arguments. Search criteria missing or wrong");
    this->SetError(request, UPNP_SOAP_E_INVALID_ARGS);
    return request->ErrCode;
  }

  if(this->ParseStringValue(request->ActionRequest, "Filter", searchRequest.filter)){
    esyslog("UPnP\tInvalid arguments. Filter missing or wrong");
    this->SetError(request, UPNP_SOAP_E_INVALID_ARGS);
    return request->ErrCode;
  }

  long startIndex;
  if(this->ParseIntegerValue(request->ActionRequest, "StartingIndex", startIndex)){
    esyslog("UPnP\tInvalid arguments. Starting index missing or wrong");
    this->SetError(request, UPNP_SOAP_E_INVALID_ARGS);
    return request->ErrCode;
  }
  searchRequest.startIndex = startIndex;

  long requestCount;
  if(this->ParseIntegerValue(request->ActionRequest, "RequestedCount", requestCount)){
    esyslog("UPnP\tInvalid arguments. Requested count missing or wrong");
    this->SetError(request, UPNP_SOAP_E_INVALID_ARGS);
    return request->ErrCode;
  }
  searchRequest.requestCount = requestCount;

  if(this->ParseStringValue(request->ActionRequest, "SortCriteria", searchRequest.sortCriteria)){
    esyslog("UPnP\tInvalid arguments. Sort criteria missing or wrong");
    this->SetError(request, UPNP_SOAP_E_INVALID_ARGS);
    return request->ErrCode;
  }

  int ret = mMediaServer->GetManager().Search(searchRequest);
  if(ret!=UPNP_E_SUCCESS){
    esyslog("UPnP\tError while browsing. Code: %d", ret);
    this->SetError(request, ret);
    return request->ErrCode;
  }

  ixml::XmlEscapeSpecialChars(searchRequest.result);

  std::stringstream ss;

  ss << "<u:" << request->ActionName << "Response xmlns:u=\"" << GetServiceDescription().serviceType << "\">"
     << "  <Result>" << searchRequest.result << "</Result>"
     << "  <NumberReturned>" << searchRequest.numberReturned << "</NumberReturned>"
     << "  <TotalMatches>" << searchRequest.totalMatches << "</TotalMatches>"
     << "  <UpdateID>" << searchRequest.updateID << "</UpdateID>"
     << "</u:" << request->ActionName << "Response>";

  request->ActionResult = ixmlParseBuffer(ss.str().c_str());
  request->ErrCode = UPNP_E_SUCCESS;

  return request->ErrCode;
}

int cContentDirectory::Browse(Upnp_Action_Request* request){
  cMediaManager::BrowseRequest browseRequest;

  if(this->ParseStringValue(request->ActionRequest, "ObjectID", browseRequest.objectID)){
    esyslog("UPnP\tInvalid arguments. ObjectID missing or wrong");
    this->SetError(request, UPNP_SOAP_E_INVALID_ARGS);
    return request->ErrCode;
  }

  std::string browseFlag;
  if(this->ParseStringValue(request->ActionRequest, "BrowseFlag", browseFlag)){
    esyslog("UPnP\tInvalid arguments. Browse flag missing or wrong");
    this->SetError(request, UPNP_SOAP_E_INVALID_ARGS);
    return request->ErrCode;
  }
  if((browseRequest.browseMetadata = cMediaManager::ToBrowseFlag(browseFlag)) == cMediaManager::NumBrowseFlags){
    esyslog("UPnP\tInvalid arguments. Browse flag invalid");
    this->SetError(request, UPNP_SOAP_E_INVALID_ARGS);
    return request->ErrCode;
  }

  if(this->ParseStringValue(request->ActionRequest, "Filter", browseRequest.filter)){
    esyslog("UPnP\tInvalid arguments. Filter missing or wrong");
    this->SetError(request, UPNP_SOAP_E_INVALID_ARGS);
    return request->ErrCode;
  }

  long startIndex;
  if(this->ParseIntegerValue(request->ActionRequest, "StartingIndex", startIndex)){
    esyslog("UPnP\tInvalid arguments. Starting index missing or wrong");
    this->SetError(request, UPNP_SOAP_E_INVALID_ARGS);
    return request->ErrCode;
  }
  browseRequest.startIndex = startIndex;

  long requestCount;
  if(this->ParseIntegerValue(request->ActionRequest, "RequestedCount", requestCount)){
    esyslog("UPnP\tInvalid arguments. Requested count missing or wrong");
    this->SetError(request, UPNP_SOAP_E_INVALID_ARGS);
    return request->ErrCode;
  }
  browseRequest.requestCount = requestCount;

  if(this->ParseStringValue(request->ActionRequest, "SortCriteria", browseRequest.sortCriteria)){
    esyslog("UPnP\tInvalid arguments. Sort criteria missing or wrong");
    this->SetError(request, UPNP_SOAP_E_INVALID_ARGS);
    return request->ErrCode;
  }

  int ret = mMediaServer->GetManager().Browse(browseRequest);
  if(ret!=UPNP_E_SUCCESS){
    this->SetError(request, ret);
    esyslog("UPnP\tError while browsing: %s (%d)", request->ErrStr, request->ErrCode);
    return request->ErrCode;
  }

  ixml::XmlEscapeSpecialChars(browseRequest.result);

  std::stringstream ss;

  ss << "<u:" << request->ActionName << "Response xmlns:u=\"" << GetServiceDescription().serviceType << "\">"
     << "  <Result>" << browseRequest.result << "</Result>"
     << "  <NumberReturned>" << browseRequest.numberReturned << "</NumberReturned>"
     << "  <TotalMatches>" << browseRequest.totalMatches << "</TotalMatches>"
     << "  <UpdateID>" << browseRequest.updateID << "</UpdateID>"
     << "</u:" << request->ActionName << "Response>";

  request->ActionResult = ixmlParseBuffer(ss.str().c_str());
  request->ErrCode = UPNP_E_SUCCESS;

  return request->ErrCode;
}

int cContentDirectory::GetSystemUpdateID(Upnp_Action_Request* request){
  std::stringstream ss;

  ss << "<u:" << request->ActionName << "Response xmlns:u=\"" << GetServiceDescription().serviceType << "\">"
     << "  <Id>" << mMediaServer->GetManager().GetSystemUpdateID() << "</Id>"
     << "</u:" << request->ActionName << "Response>";

  request->ActionResult = ixmlParseBuffer(ss.str().c_str());
  request->ErrCode = UPNP_E_SUCCESS;

  return request->ErrCode;
}

int cContentDirectory::GetSortCapabilities(Upnp_Action_Request* request){
  std::stringstream ss;

  ss << "<u:" << request->ActionName << "Response xmlns:u=\"" << GetServiceDescription().serviceType << "\">"
     << "  <SortCaps>" << tools::StringListToCSV(mMediaServer->GetManager().GetSortCapabilities()) << "</SortCaps>"
     << "</u:" << request->ActionName << "Response>";

  request->ActionResult = ixmlParseBuffer(ss.str().c_str());
  request->ErrCode = UPNP_E_SUCCESS;

  return request->ErrCode;
}

int cContentDirectory::GetSearchCapabilities(Upnp_Action_Request* request){
  std::stringstream ss;

  ss << "<u:" << request->ActionName << "Response xmlns:u=\"" << GetServiceDescription().serviceType << "\">"
     << "  <SearchCaps>" << tools::StringListToCSV(mMediaServer->GetManager().GetSearchCapabilities()) << "</SearchCaps>"
     << "</u:" << request->ActionName << "Response>";

  request->ActionResult = ixmlParseBuffer(ss.str().c_str());
  request->ErrCode = UPNP_E_SUCCESS;

  return request->ErrCode;
}

void cContentDirectory::SetError(Upnp_Action_Request* request, int error){
  request->ErrCode = error;
  switch(error){
  case UPNP_CDS_E_BAD_METADATA:
    strn0cpy(request->ErrStr,tr("Bad metadata"),LINE_SIZE);
    break;
  case UPNP_CDS_E_CANT_PROCESS_REQUEST:
    strn0cpy(request->ErrStr,tr("Cannot process the request"),LINE_SIZE);
    break;
  case UPNP_CDS_E_DEST_RESOURCE_ACCESS_DENIED:
    strn0cpy(request->ErrStr,tr("Destination resource access denied"),LINE_SIZE);
    break;
  case UPNP_CDS_E_INVALID_CURRENT_TAG:
    strn0cpy(request->ErrStr,tr("Invalid current tag"),LINE_SIZE);
    break;
  case UPNP_CDS_E_INVALID_NEW_TAG:
    strn0cpy(request->ErrStr,tr("Invalid new tag"),LINE_SIZE);
    break;
  case UPNP_CDS_E_INVALID_SEARCH_CRITERIA:
    strn0cpy(request->ErrStr,tr("Invalid or unsupported search criteria"),LINE_SIZE);
    break;
  case UPNP_CDS_E_INVALID_SORT_CRITERIA:
    strn0cpy(request->ErrStr,tr("Invalid or unsupported sort criteria"),LINE_SIZE);
    break;
  case UPNP_CDS_E_NO_SUCH_CONTAINER:
    strn0cpy(request->ErrStr,tr("No such container"),LINE_SIZE);
    break;
  case UPNP_CDS_E_NO_SUCH_DESTINATION_RESOURCE:
    strn0cpy(request->ErrStr,tr("No such destination resource"),LINE_SIZE);
    break;
  case UPNP_CDS_E_NO_SUCH_FILE_TRANSFER:
    strn0cpy(request->ErrStr,tr("No such file transfer"),LINE_SIZE);
    break;
  case UPNP_CDS_E_NO_SUCH_OBJECT:
    strn0cpy(request->ErrStr,tr("No such objectID"),LINE_SIZE);
    break;
  case UPNP_CDS_E_NO_SUCH_SOURCE_RESOURCE:
    strn0cpy(request->ErrStr,tr("No such source resource"),LINE_SIZE);
    break;
  case UPNP_CDS_E_PARAMETER_MISMATCH:
    strn0cpy(request->ErrStr,tr("Parameter mismatch"),LINE_SIZE);
    break;
  case UPNP_CDS_E_READ_ONLY_TAG:
    strn0cpy(request->ErrStr,tr("Read only tag"),LINE_SIZE);
    break;
  case UPNP_CDS_E_REQUIRED_TAG:
    strn0cpy(request->ErrStr,tr("Required tag"),LINE_SIZE);
    break;
  case UPNP_CDS_E_RESOURCE_ACCESS_DENIED:
    strn0cpy(request->ErrStr,tr("Resource access denied"),LINE_SIZE);
    break;
  case UPNP_CDS_E_RESTRICTED_OBJECT:
    strn0cpy(request->ErrStr,tr("Restricted object"),LINE_SIZE);
    break;
  case UPNP_CDS_E_RESTRICTED_PARENT:
    strn0cpy(request->ErrStr,tr("Restricted parent"),LINE_SIZE);
    break;
  case UPNP_CDS_E_TRANSFER_BUSY:
    strn0cpy(request->ErrStr,tr("Transfer busy"),LINE_SIZE);
    break;
  default:
    cUPnPService::SetError(request, error);
    break;
  }
  return;
}

}  // namespace upnp
