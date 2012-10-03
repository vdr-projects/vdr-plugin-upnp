/*
 * ixml.h
 *
 *  Created on: 03.10.2012
 *      Author: savop
 */

#ifndef IXML_H_
#define IXML_H_

#include <string>
#include <upnp/upnp.h>
#include "../tools.h"

using namespace std;

namespace upnp {

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


#endif /* IXML_H_ */
