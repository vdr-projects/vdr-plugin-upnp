/*
 * tools.cpp
 *
 *  Created on: 05.08.2012
 *      Author: savop
 */

#include "../include/tools.h"
#include <sstream>
#include <net/if.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/ioctl.h>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_io.hpp>

using namespace std;

char* substr(const char* str, unsigned int offset, unsigned int length){
  if(offset > strlen(str)) return NULL;
  if(length > strlen(str+offset)) length = strlen(str+offset);
  char* substring = (char*)malloc(sizeof(substring)*length+1);
  strncpy(substring, str+offset, length);
  substring[length] = '\0';
  return substring;
}

namespace upnp {

namespace tools {

string GetAddressByInterface(string Interface){
  string address;

  if(!Interface.empty()){
    int fd;
    struct ifreq ifr;
    fd = socket(AF_INET, SOCK_DGRAM, 0);
    /* I want to get an IPv4 IP address */
    ifr.ifr_addr.sa_family = AF_INET;
    /* I want IP address attached to "eth0" */
    strncpy(ifr.ifr_name, Interface.c_str(), IFNAMSIZ-1);
    int ret = ioctl(fd, SIOCGIFADDR, &ifr);
    close(fd);
    if(ret==0){
      sockaddr_in* inAddr = (sockaddr_in*)&ifr.ifr_addr;
      address = inet_ntoa(inAddr->sin_addr);
    }
  }

  return address;
}
string GetNetworkInterfaceByIndex(int Index, bool skipLoop){
  return GetNetworkInterfaces(skipLoop)[Index];
}

vector<string> GetNetworkInterfaces(bool skipLoop){
  vector<string> interfaces;

  int fd;
  struct ifconf ifc;
  struct ifreq ifr[10];
  int nifaces, i;

  memset(&ifc,0,sizeof(ifc));
  ifc.ifc_buf = (char*) (ifr);
  ifc.ifc_len = sizeof(ifr);

  fd = socket(AF_INET, SOCK_DGRAM, 0);
  int ret = ioctl(fd, SIOCGIFCONF, &ifc);
  close(fd);
  if(ret==0){
    nifaces = ifc.ifc_len/sizeof(struct ifreq);
    for(i = 0; i < nifaces; i++){
      if(skipLoop && strcmp("lo", ifr[i].ifr_name)==0)
        continue;
      else
        interfaces.push_back(ifr[i].ifr_name);
    }
  }

  return interfaces;
}

string ToString(long number){
  stringstream ss;
  ss << number;
  return ss.str();
}

string StringListToCSV(StringList list){
  stringstream ss;

  if(list.empty()) return string();

  StringList::iterator it = list.begin();

  ss << (*it);
  for(++it; it != list.end(); ++it){
    ss << "," << (*it);
  }

  return ss.str();
}

string IdListToCSV(IdList list){
  stringstream ss;

  if(list.empty()) return string();

  IdList::iterator it = list.begin();

  ss << (*it).first << "," << (*it).second;
  for(++it; it != list.end(); ++it){
    ss << "," << (*it).first << "," << (*it).second;
  }

  return ss.str();
}

string GenerateUUIDFromURL(string url){
  boost::uuids::string_generator gen;
  boost::uuids::uuid urlNamespace = gen(L"6ba7b811-9dad-11d1-80b4-00c04fd430c8");
  stringstream uuid;

  uuid << boost::uuids::name_generator(urlNamespace)(url);

  return uuid.str();
}

string GenerateUUIDRandomly(){
  stringstream uuid;

  uuid << boost::uuids::random_generator()();

  return uuid.str();
}

}  // namespace tools

namespace ixml {

void XmlEscapeSpecialChars(string& doc){
    std::string buffer;

    buffer.reserve(doc.size()*1.1);
    for(unsigned int i = 0; i < doc.size(); i++){
        switch((long)doc[i]){
            case L'€': buffer.append("&euro;"); break;
            case L'"': buffer.append("&quot;"); break;
            case L'&': buffer.append("&amp;"); break;
            case L'<': buffer.append("&lt;"); break;
            case L'>': buffer.append("&gt;"); break;
            case L'¡': buffer.append("&iexcl;"); break;
            case L'¢': buffer.append("&cent;"); break;
            case L'£': buffer.append("&pound;"); break;
            case L'¤': buffer.append("&curren;"); break;
            case L'¥': buffer.append("&yen;"); break;
            case L'¦': buffer.append("&brvbar;"); break;
            case L'§': buffer.append("&sect;"); break;
            case L'¨': buffer.append("&uml;"); break;
            case L'©': buffer.append("&copy;"); break;
            case L'ª': buffer.append("&ordf;"); break;
            case L'¬': buffer.append("&not;"); break;
            case L'­': buffer.append("&shy;"); break;
            case L'®': buffer.append("&reg;"); break;
            case L'¯': buffer.append("&macr;"); break;
            case L'°': buffer.append("&deg;"); break;
            case L'±': buffer.append("&plusmn;"); break;
            case L'²': buffer.append("&sup2;"); break;
            case L'³': buffer.append("&sup3;"); break;
            case L'´': buffer.append("&acute;"); break;
            case L'µ': buffer.append("&micro;"); break;
            case L'¶': buffer.append("&para;"); break;
            case L'·': buffer.append("&middot;"); break;
            case L'¸': buffer.append("&cedil;"); break;
            case L'¹': buffer.append("&sup1;"); break;
            case L'º': buffer.append("&ordm;"); break;
            case L'»': buffer.append("&raquo;"); break;
            case L'«': buffer.append("&laquo;"); break;
            case L'¼': buffer.append("&frac14;"); break;
            case L'½': buffer.append("&frac12;"); break;
            case L'¾': buffer.append("&frac34;"); break;
            case L'¿': buffer.append("&iquest;"); break;
            case L'À': buffer.append("&Agrave;"); break;
            case L'Á': buffer.append("&Aacute;"); break;
            case L'Â': buffer.append("&Acirc;"); break;
            case L'Ã': buffer.append("&Atilde;"); break;
            case L'Ä': buffer.append("&Auml;"); break;
            case L'Å': buffer.append("&Aring;"); break;
            case L'Æ': buffer.append("&AElig;"); break;
            case L'Ç': buffer.append("&Ccedil;"); break;
            case L'È': buffer.append("&Egrave;"); break;
            case L'É': buffer.append("&Eacute;"); break;
            case L'Ê': buffer.append("&Ecirc;"); break;
            case L'Ë': buffer.append("&Euml;"); break;
            case L'Ì': buffer.append("&Igrave;"); break;
            case L'Í': buffer.append("&Iacute;"); break;
            case L'Î': buffer.append("&Icirc;"); break;
            case L'Ï': buffer.append("&Iuml;"); break;
            case L'Ð': buffer.append("&ETH;"); break;
            case L'Ñ': buffer.append("&Ntilde;"); break;
            case L'Ò': buffer.append("&Ograve;"); break;
            case L'Ó': buffer.append("&Oacute;"); break;
            case L'Ô': buffer.append("&Ocirc;"); break;
            case L'Õ': buffer.append("&Otilde;"); break;
            case L'Ö': buffer.append("&Ouml;"); break;
            case L'×': buffer.append("&times;"); break;
            case L'Ø': buffer.append("&Oslash;"); break;
            case L'Ù': buffer.append("&Ugrave;"); break;
            case L'Ú': buffer.append("&Uacute;"); break;
            case L'Û': buffer.append("&Ucirc;"); break;
            case L'Ü': buffer.append("&Uuml;"); break;
            case L'Ý': buffer.append("&Yacute;"); break;
            case L'Þ': buffer.append("&THORN;"); break;
            case L'ß': buffer.append("&szlig;"); break;
            case L'à': buffer.append("&agrave;"); break;
            case L'á': buffer.append("&aacute;"); break;
            case L'â': buffer.append("&acirc;"); break;
            case L'ã': buffer.append("&atilde;"); break;
            case L'ä': buffer.append("&auml;"); break;
            case L'å': buffer.append("&aring;"); break;
            case L'æ': buffer.append("&aelig;"); break;
            case L'ç': buffer.append("&ccedil;"); break;
            case L'è': buffer.append("&egrave;"); break;
            case L'é': buffer.append("&eacute;"); break;
            case L'ê': buffer.append("&ecirc;"); break;
            case L'ë': buffer.append("&euml;"); break;
            case L'ì': buffer.append("&igrave;"); break;
            case L'í': buffer.append("&iacute;"); break;
            case L'î': buffer.append("&icirc;"); break;
            case L'ï': buffer.append("&iuml;"); break;
            case L'ð': buffer.append("&eth;"); break;
            case L'ñ': buffer.append("&ntilde;"); break;
            case L'ò': buffer.append("&ograve;"); break;
            case L'ó': buffer.append("&oacute;"); break;
            case L'ô': buffer.append("&ocirc;"); break;
            case L'õ': buffer.append("&otilde;"); break;
            case L'ö': buffer.append("&ouml;"); break;
            case L'÷': buffer.append("&divide;"); break;
            case L'ø': buffer.append("&oslash;"); break;
            case L'ù': buffer.append("&ugrave;"); break;
            case L'ú': buffer.append("&uacute;"); break;
            case L'û': buffer.append("&ucirc;"); break;
            case L'ü': buffer.append("&uuml;"); break;
            case L'ý': buffer.append("&yacute;"); break;
            case L'þ': buffer.append("&thorn;"); break;
            default: buffer.append(1, doc[i]); break;
        }
    }

    doc.swap(buffer);
}

//Function copied from Intel SDK
///////////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2000-2003 Intel Corporation
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
// * Redistributions of source code must retain the above copyright notice,
// this list of conditions and the following disclaimer.
// * Redistributions in binary form must reproduce the above copyright notice,
// this list of conditions and the following disclaimer in the documentation
// and/or other materials provided with the distribution.
// * Neither name of Intel Corporation nor the names of its contributors
// may be used to endorse or promote products derived from this software
// without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
// "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
// LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
// A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL INTEL OR
// CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
// EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
// PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
// PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
// OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
// NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
// SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//
///////////////////////////////////////////////////////////////////////////
/********************************************************************************
 * SampleUtil_GetFirstDocumentItem
 *
 * Description:
 *       Given a document node, this routine searches for the first element
 *       named by the input string item, and returns its value as a string.
 *       String must be freed by caller using free.
 * Parameters:
 *   doc -- The DOM document from which to extract the value
 *   item -- The item to search for
 *
 *
 ********************************************************************************/

int IxmlGetFirstDocumentItem( IN IXML_Document * doc, IN std::string item, std::string& value ) {
  IXML_NodeList *nodeList = NULL;
  IXML_Node *textNode = NULL;
  IXML_Node *tmpNode = NULL;

  int error = 0;

  nodeList = ixmlDocument_getElementsByTagName( doc, item.c_str() );

  if( nodeList != NULL ) {
    if( ( tmpNode = ixmlNodeList_item( nodeList, 0 ) ) ) {

      textNode = ixmlNode_getFirstChild( tmpNode );

      if(textNode != NULL){
        value = ixmlNode_getNodeValue( textNode );
      }
    }
  } else {
    error = -1;
  }

  if( nodeList != NULL) {
    ixmlNodeList_free( nodeList );
  }


  return error;
}

IXML_Element* IxmlAddProperty(IXML_Document* document, IXML_Element* node, const string& upnpproperty, const string& value){
  if(!node) return NULL;
  IXML_Element* PropertyNode = NULL;

  string tvalue = value.substr(0,MAX_METADATA_LENGTH_L);

  string::size_type pos = upnpproperty.find('@');
  string attribute = pos!=string::npos ? upnpproperty.substr(pos+1) : string();
  string property = pos!=string::npos ? upnpproperty.substr(0, pos) : upnpproperty;

  if(!attribute.empty()){
    if(property.empty()){
      if(ixmlElement_setAttribute(node, attribute.c_str(), tvalue.c_str())!=IXML_SUCCESS){
        return NULL;
      }
    }
    else {
      IXML_NodeList* NodeList = ixmlElement_getElementsByTagName(node, property.c_str());
      if(NodeList!=NULL){
        PropertyNode = (IXML_Element*) ixmlNodeList_item(NodeList, 0);
        if(PropertyNode){
          if(ixmlElement_setAttribute(PropertyNode, attribute.c_str(), tvalue.c_str())!=IXML_SUCCESS){
            return NULL;
          }
        }
        else {
          ixmlNodeList_free(NodeList);
          return NULL;
        }
      }
      else {
        return NULL;
      }
    }
  }
  else {
    PropertyNode = ixmlDocument_createElement(document, property.c_str());
    IXML_Node* PropertyText = ixmlDocument_createTextNode(document, tvalue.c_str());
    ixmlNode_appendChild((IXML_Node*) PropertyNode, PropertyText);
    ixmlNode_appendChild((IXML_Node*) node, (IXML_Node*) PropertyNode);
  }
  return PropertyNode;
}

IXML_Element* IxmlAddFilteredProperty(const StringList& Filter, IXML_Document* document, IXML_Element* node, const string& upnpproperty, const string& value){
  // leave out empty values.
  if(value.empty() || !value.compare("0")){
    return NULL;
  }

  if((*Filter.begin()).compare("*") == 0) return IxmlAddProperty(document, node, upnpproperty, value);

  for(StringList::const_iterator it = Filter.begin(); it != Filter.end(); ++it){
    if((*it).compare(upnpproperty) == 0) return IxmlAddProperty(document, node, upnpproperty, value);
  }

  return NULL;
}

IXML_Element* IxmlReplaceProperty(IXML_Document* document, IXML_Element* node, const string& upnpproperty, const string& newValue){
  if(!node) return NULL;
  IXML_Element* PropertyNode = NULL;

  string tvalue = newValue.substr(0, MAX_METADATA_LENGTH_L);

  string::size_type pos = upnpproperty.find('@');
  string attribute = pos!=string::npos ? upnpproperty.substr(pos+1) : string();
  string property = pos!=string::npos ? upnpproperty.substr(0, pos) : upnpproperty;

  if(!attribute.empty()){
    if(property.empty()){
      if(tvalue.empty()){
        if(ixmlElement_setAttribute(node, attribute.c_str(), tvalue.c_str())!=IXML_SUCCESS){
          return NULL;
        }
      }
      else {
        ixmlElement_removeAttribute(node, attribute.c_str());
      }
    }
    else {
      IXML_NodeList* NodeList = ixmlElement_getElementsByTagName(node, property.c_str());
      if(NodeList!=NULL){
        PropertyNode = (IXML_Element*) ixmlNodeList_item(NodeList, 0);
        if(PropertyNode){
          if(!tvalue.empty()){
            if(ixmlElement_setAttribute(PropertyNode, attribute.c_str(), tvalue.c_str())!=IXML_SUCCESS){
              return NULL;
            }
          }
          else {
            ixmlElement_removeAttribute(PropertyNode, attribute.c_str());
          }
        }
        else {
          ixmlNodeList_free(NodeList);
          return NULL;
        }
      }
      else {
        return NULL;
      }
    }
  }
  else {
    IXML_NodeList* NodeList = ixmlElement_getElementsByTagName(node, property.c_str());
    if(NodeList!=NULL){
      PropertyNode = (IXML_Element*) ixmlNodeList_item(NodeList, 0);
      IXML_Node* PropertyText = ixmlNode_getFirstChild((IXML_Node*) PropertyNode);

      if(ixmlNode_removeChild((IXML_Node*) PropertyNode, PropertyText, NULL)!=IXML_SUCCESS){
        return NULL;
      }
      if(!tvalue.empty()){
        PropertyText = ixmlDocument_createTextNode(document, tvalue.c_str());
      }
      ixmlNode_appendChild((IXML_Node*) PropertyNode, PropertyText);
    }
    else {
      ixmlNodeList_free(NodeList);
      return NULL;
    }
  }
  return PropertyNode;
}

}  // namespace ixml

}  // namespace upnp
