/*
 * service.h
 *
 *  Created on: 27.08.2012
 *      Author: savop
 */

#ifndef SERVICE_H_
#define SERVICE_H_

#include <upnp/upnp.h>
#include <string>

namespace upnp {

class cMediaServer;

class cUPnPService {
public:

  struct Description {
    Description(std::string, std::string, std::string, std::string, std::string);
    std::string serviceType;
    std::string serviceID;
    std::string SCPDXML;
    std::string controlDescriptor;
    std::string eventSubscriberDescriptor;
  };

  cUPnPService(Description serviceDescription);

  virtual ~cUPnPService();

  virtual void Init(cMediaServer* server, UpnpDevice_Handle deviceHandle);

  virtual int Subscribe(
    Upnp_Subscription_Request* Request      ///< Information about the subscription
  ) = 0;

  virtual int Execute(
    Upnp_Action_Request* Request            ///< Input and output parameters of an action
  ) = 0;

  const Description& GetServiceDescription() const { return mServiceDescription; };

protected:
  /**
   * Sets an error on an action request
   *
   * This function puts a error message into the action request structure
   * according to its error code
   *
   * @param Request the action request, to set the error for
   * @param Error the error code of which the message should be obtained
   */
  virtual void SetError(
    Upnp_Action_Request* Request,           ///< the action request, to set the error for
    int Error                               ///< the error code of which the message should be obtained
  );
  /**
   * Parses an integer value
   *
   * This tool function parses an integer value from a given \em IXML document. It is searching
   * for the very first occurance of the demanded item.
   *
   * @return Returns
   * - \bc 0, if parsing was successful
   * - \bc <0, if an error occured
   *
   * @param Document the document, which is parsed
   * @param Item the demanded item
   * @param Value the value of the item
   */
  int ParseIntegerValue(
    IXML_Document* Document,             ///< the document, which is parsed
    std::string Item,                    ///< the demanded item
    long& Value                           ///< the value of the item
  );
  /**
   * Parses a string value
   *
   * This tool function parses a string value from a given \em IXML document. It is searching
   * for the very first occurance of the demanded item.
   *
   * @return Returns
   * - \bc 0, if parsing was successful
   * - \bc <0, if an error occured
   *
   * @param Document the document, which is parsed
   * @param Item the demanded item
   * @param Value the value of the item
   */
  int ParseStringValue(
    IXML_Document* Document,             ///< the document, which is parsed
    std::string Item,                    ///< the demanded item
    std::string& Value                   ///< the value of the item
  );

  cMediaServer*     mMediaServer;
  UpnpDevice_Handle mDeviceHandle;            ///< the UPnP device handle of the root device
  Description       mServiceDescription;

};

}  // namespace upnp


#endif /* SERVICE_H_ */
