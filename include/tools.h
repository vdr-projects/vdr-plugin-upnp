/*
 * tools.h
 *
 *  Created on: 03.08.2012
 *      Author: savop
 */

#ifndef TOOLS_H_
#define TOOLS_H_

#include <vector>
#include <string>
#include <iostream>
#include <upnp/upnp.h>
#include <string.h>
#include <vdr/tools.h>
#include <map>
#include <list>

using namespace std;

#define KB(s)         (s * 1024)
#define MB(s)         (s * 1024 * 1024)

#define ARRAY_SIZE(a)  (sizeof(a)/sizeof(a[0]))

#define CRLF          "\r\n"

#define NL            "\n"

#define att(s)                  strchr(s,'@')!=NULL?strchr(s,'@')+1:NULL
#define prop(s)                 substr(s, 0, strchr(s,'@')-s)

#define MAX_METADATA_LENGTH_L     1024
#define MAX_METADATA_LENGTH_S     256

/**
 * creates a part of a string
 *
 * This creates a substring of a string which begins at the given offset and has the
 * specified lenght.
 *
 * @return the new string
 * @param str the full string
 * @param offset the starting index
 * @param length the length of the new string
 */
char* substr(const char* str, unsigned int offset, unsigned int length);

/****************************************************
 *
 * Known Errors
 *
 ****************************************************/

/* Errors 401-404, 501 are already defined in
 * Intel SDK, however 403 MUST NOT USED.
 */

/****** 600 Common Action Errors ******/

#define UPNP_SOAP_E_ARGUMENT_INVALID            600
#define UPNP_SOAP_E_ARGUMENT_OUT_OF_RANGE       601
#define UPNP_SOAP_E_ACTION_NOT_IMPLEMENTED      602
#define UPNP_SOAP_E_OUT_OF_MEMORY               603
#define UPNP_SOAP_E_HUMAN_INTERVENTION          604
#define UPNP_SOAP_E_STRING_TO_LONG              605
#define UPNP_SOAP_E_NOT_AUTHORIZED              606
#define UPNP_SOAP_E_SIGNATURE_FAILURE           607
#define UPNP_SOAP_E_SIGNATURE_MISSING           608
#define UPNP_SOAP_E_NOT_ENCRYPTED               609
#define UPNP_SOAP_E_INVALID_SEQUENCE            610
#define UPNP_SOAP_E_INVALID_CONTROL_URL         611
#define UPNP_SOAP_E_NO_SUCH_SESSION             612

/****** 700 Action specific Errors ******/

#define UPNP_CDS_E_NO_SUCH_OBJECT               701
#define UPNP_CDS_E_INVALID_CURRENT_TAG          702
#define UPNP_CDS_E_INVALID_NEW_TAG              703
#define UPNP_CDS_E_REQUIRED_TAG                 704
#define UPNP_CDS_E_READ_ONLY_TAG                705
#define UPNP_CDS_E_PARAMETER_MISMATCH           706
#define UPNP_CDS_E_INVALID_SEARCH_CRITERIA      708
#define UPNP_CDS_E_INVALID_SORT_CRITERIA        709
#define UPNP_CDS_E_NO_SUCH_CONTAINER            710
#define UPNP_CDS_E_RESTRICTED_OBJECT            711
#define UPNP_CDS_E_BAD_METADATA                 712
#define UPNP_CDS_E_RESTRICTED_PARENT            713
#define UPNP_CDS_E_NO_SUCH_SOURCE_RESOURCE      714
#define UPNP_CDS_E_RESOURCE_ACCESS_DENIED       715
#define UPNP_CDS_E_TRANSFER_BUSY                716
#define UPNP_CDS_E_NO_SUCH_FILE_TRANSFER        717
#define UPNP_CDS_E_NO_SUCH_DESTINATION_RESOURCE 718
#define UPNP_CDS_E_DEST_RESOURCE_ACCESS_DENIED  719
#define UPNP_CDS_E_CANT_PROCESS_REQUEST         720

#define UPNP_CMS_E_INCOMPATIBLE_PROTOCOL_INFO   701
#define UPNP_CMS_E_INCOMPATIBLE_DIRECTIONS      702
#define UPNP_CMS_E_INSUFFICIENT_RESOURCES       703
#define UPNP_CMS_E_LOCAL_RESTRICTIONS           704
#define UPNP_CMS_E_ACCESS_DENIED                705
#define UPNP_CMS_E_INVALID_CONNECTION_REFERENCE 706
#define UPNP_CMS_E_NOT_IN_NETWORK               707

namespace upnp {

  typedef std::list<std::string> StringList;
  typedef std::vector<std::string> StringVector;
  typedef std::map<std::string, uint32_t> IdList;

namespace tools {
  string GetAddressByInterface(string Interface);
  string GetNetworkInterfaceByIndex(int Index, bool skipLoop);
  StringVector GetNetworkInterfaces(bool skipLoop);

  string ToString(long number);

  string StringListToCSV(StringList list);
  string IdListToCSV(IdList list);

  string GenerateUUIDFromURL(string url);
  string GenerateUUIDRandomly();
}

namespace ixml {
  void XmlEscapeSpecialChars(string& doc);
  /**
   * First occurance of item
   *
   * Finds the first occurance of a specified item in a given \bc IXML document and returns its value.
   * If an error occures, its code is stored in the last parameter \c 'error'.
   *
   * @return error the error code in case of an error
   * @param doc the \c IXML document to be parsed
   * @param item the item which shall be found
   * @param the value of the item
   */
  int IxmlGetFirstDocumentItem( IN IXML_Document * doc, IN const string item, string& value );
  /**
   * Adds a property
   *
   * This adds a UPnP property to an \bc IXML document.
   * The property must have the pattern "namespace:property@attribute".
   *
   * @return returns
   * - \bc NULL, in case of an error
   * - \bc the newly created property node or the node at which the attribute was
   *       appended to
   * @param document the \c IXML document to put the parameter in
   * @param node the specific node where to put the parameter
   * @param upnpproperty the upnp property
   * @param value the value of the upnp property
   */
  IXML_Element* IxmlAddProperty(IN IXML_Document* document, IN IXML_Element* node, IN const string& upnpproperty, IN const string& value );

  IXML_Element* IxmlAddFilteredProperty(IN const StringList& Filter, IN IXML_Document* document, IN IXML_Element* node, IN const string& upnpproperty, IN const string& value );

  IXML_Element* IxmlReplaceProperty(IN IXML_Document* document, IN IXML_Element* node, IN const string& upnpproperty, IN const string& newValue );
}

}

#endif /* TOOLS_H_ */
