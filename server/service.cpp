/*
 * service.cpp
 *
 *  Created on: 27.08.2012
 *      Author: savop
 */


#include "../include/service.h"
#include "../include/server.h"
#include "../include/tools.h"
#include <vdr/i18n.h>

namespace upnp {

cUPnPService::cUPnPService(Description serviceDescription)
: mMediaServer(NULL)
, mDeviceHandle(0)
, mServiceDescription(serviceDescription)
{
  cMediaServer::RegisterService(this);
}

cUPnPService::~cUPnPService(){}

void cUPnPService::Init(cMediaServer* server, UpnpDevice_Handle deviceHandle){
  mMediaServer = server;
  mDeviceHandle = deviceHandle;
}

void cUPnPService::Stop(){
  return;
}

void cUPnPService::SetError(Upnp_Action_Request* request, int error){
  request->ErrCode = error;
  switch(error){
    case UPNP_SOAP_E_INVALID_ACTION:
        strn0cpy(request->ErrStr,tr("Invalid action"),LINE_SIZE);
        break;
    case UPNP_SOAP_E_INVALID_ARGS:
        strn0cpy(request->ErrStr,tr("Invalid args"),LINE_SIZE);
        break;
    case UPNP_SOAP_E_INVALID_VAR:
        strn0cpy(request->ErrStr,tr("Invalid var"),LINE_SIZE);
        break;
    case UPNP_SOAP_E_ACTION_FAILED:
        strn0cpy(request->ErrStr,tr("Action failed"),LINE_SIZE);
        break;
    case UPNP_SOAP_E_ARGUMENT_INVALID:
        strn0cpy(request->ErrStr,tr("Argument value invalid"),LINE_SIZE);
        break;
    case UPNP_SOAP_E_ARGUMENT_OUT_OF_RANGE:
        strn0cpy(request->ErrStr,tr("Argument value out of range"),LINE_SIZE);
        break;
    case UPNP_SOAP_E_ACTION_NOT_IMPLEMENTED:
        strn0cpy(request->ErrStr,tr("Optional action not implemented"),LINE_SIZE);
        break;
    case UPNP_SOAP_E_OUT_OF_MEMORY:
        strn0cpy(request->ErrStr,tr("Out of memory"),LINE_SIZE);
        break;
    case UPNP_SOAP_E_HUMAN_INTERVENTION:
        strn0cpy(request->ErrStr,tr("Human intervention required"),LINE_SIZE);
        break;
    case UPNP_SOAP_E_STRING_TO_LONG:
        strn0cpy(request->ErrStr,tr("String argument to long"),LINE_SIZE);
        break;
    case UPNP_SOAP_E_NOT_AUTHORIZED:
        strn0cpy(request->ErrStr,tr("Action not authorized"),LINE_SIZE);
        break;
    case UPNP_SOAP_E_SIGNATURE_FAILURE:
        strn0cpy(request->ErrStr,tr("Signature failure"),LINE_SIZE);
        break;
    case UPNP_SOAP_E_SIGNATURE_MISSING:
        strn0cpy(request->ErrStr,tr("Signature missing"),LINE_SIZE);
        break;
    case UPNP_SOAP_E_NOT_ENCRYPTED:
        strn0cpy(request->ErrStr,tr("Not encrypted"),LINE_SIZE);
        break;
    case UPNP_SOAP_E_INVALID_SEQUENCE:
        strn0cpy(request->ErrStr,tr("Invalid sequence"),LINE_SIZE);
        break;
    case UPNP_SOAP_E_INVALID_CONTROL_URL:
        strn0cpy(request->ErrStr,tr("Invalid control URL"),LINE_SIZE);
        break;
    case UPNP_SOAP_E_NO_SUCH_SESSION:
        strn0cpy(request->ErrStr,tr("No such session"),LINE_SIZE);
        break;
    case UPNP_SOAP_E_OUT_OF_SYNC:
    default:
        strn0cpy(request->ErrStr,tr("Unknown error code. Contact the device manufacturer"),LINE_SIZE);
        break;
  }
}

int cUPnPService::ParseIntegerValue(IN IXML_Document* Document, IN std::string Item, OUT long& Value){
  std::string Val;
  int error = ixml::IxmlGetFirstDocumentItem(Document, Item, Val);

  if(error) return error;
  else {
    Value = atol(Val.c_str());
    return 0;
  }
}

int cUPnPService::ParseStringValue(IN IXML_Document* Document, IN std::string Item, OUT std::string& Value){
  return ixml::IxmlGetFirstDocumentItem(Document, Item, Value);
}

cUPnPService::Description::Description(string type, string id, string scpd, string control, string event)
: serviceType(type)
, serviceID(id)
, SCPDXML(scpd)
, controlDescriptor(control)
, eventSubscriberDescriptor(event)
{
}

}  // namespace upnp


