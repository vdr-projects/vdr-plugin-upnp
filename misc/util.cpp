/*
 * File:   util.cpp
 * Author: savop, andreas
 *
 * Created on 21. Mai 2009, 21:25
 *
 * Extracted from streamdev-server plugin common.c
 *  $Id: common.c,v 1.6 2008/03/31 10:34:26 schmirl Exp $
 */
#include "util.h"
#include "../common.h"
#include <string.h>
#include <string>
#include <sys/ioctl.h>
#include <net/if.h>
#include <upnp/ixml.h>
#include <arpa/inet.h>
#include <iosfwd>
#include <time.h>

#define DURATION_MAX_STRING_LENGTH      13              // DLNA: 1-5 DIGIT hours :
                                                        //         2 DIGIT minutes :
                                                        //         2 DIGIT seconds .
                                                        //         3 DIGIT fraction

char* duration(off64_t duration, unsigned int timeBase){
    if (timeBase == 0) timeBase = 1;

    int seconds, minutes, hours, fraction;

    seconds     = duration / timeBase;
    fraction    = duration % timeBase;
    minutes     = seconds / 60;
    seconds    %= 60;
    hours       = minutes / 60;
    minutes    %= 60;

    char* output = new char[DURATION_MAX_STRING_LENGTH];

    if(!snprintf(
            output,
            DURATION_MAX_STRING_LENGTH,
            UPNP_DURATION_FORMAT_STRING,
            hours, minutes, seconds)
      ){
        delete output;
        return NULL;
    }
    else {
        if(timeBase > 1 && 
           !snprintf(
                output,
                DURATION_MAX_STRING_LENGTH,
                "%s" UPNP_DURATION_FRAME_FORMAT,
                output, fraction)
          ){
            delete output;
            return NULL;
        }
        else return output;
    }
}

char* substr(const char* str, unsigned int offset, unsigned int length){
    if(offset > strlen(str)) return NULL;
    if(length > strlen(str+offset)) length = strlen(str+offset);
    char* substring = (char*)malloc(sizeof(substring)*length+1);
    strncpy(substring, str+offset, length);
    substring[length] = '\0';
    return substring;
}

const char* getMACFromInterface(const char* Interface) {
    int fd;
    struct ifreq ifr;

    fd = socket(AF_INET, SOCK_DGRAM, 0);

    ifr.ifr_addr.sa_family = AF_INET;
    strncpy(ifr.ifr_name, Interface, IFNAMSIZ-1);

    ioctl(fd, SIOCGIFHWADDR, &ifr);

    close(fd);

    char *ret = new char[18];

    sprintf(ret, "%.2x:%.2x:%.2x:%.2x:%.2x:%.2x",
         (unsigned char)ifr.ifr_hwaddr.sa_data[0],
         (unsigned char)ifr.ifr_hwaddr.sa_data[1],
         (unsigned char)ifr.ifr_hwaddr.sa_data[2],
         (unsigned char)ifr.ifr_hwaddr.sa_data[3],
         (unsigned char)ifr.ifr_hwaddr.sa_data[4],
         (unsigned char)ifr.ifr_hwaddr.sa_data[5]);

    return ret;
}

char** getNetworkInterfaces(int *count){
    int fd;
    struct ifconf ifc;
    struct ifreq ifr[10];
    int  nifaces, i;
    char** ifaces;
    *count = 0;

    memset(&ifc,0,sizeof(ifc));
    ifc.ifc_buf = (char*) (ifr);
    ifc.ifc_len = sizeof(ifr);
    
    fd = socket(AF_INET, SOCK_DGRAM, 0);
    int ret = ioctl(fd, SIOCGIFCONF, &ifc);
    close(fd);
    if(ret==0){
        nifaces = ifc.ifc_len/sizeof(struct ifreq);
        ifaces = new char* [nifaces+1];
        for(i = 0; i < nifaces; i++){
            ifaces[i] = new char[IFNAMSIZ];
            ifaces[i] = strdup(ifr[i].ifr_name);
        }
        ifaces[i] = NULL;
        *count = nifaces;
        return ifaces;
    }
    else {
        return NULL;
    }
}

const sockaddr_in* getIPFromInterface(const char* Interface){
    if(Interface==NULL) return NULL;
    int fd;
    struct ifreq ifr;
    fd = socket(AF_INET, SOCK_DGRAM, 0);
    /* I want to get an IPv4 IP address */
    ifr.ifr_addr.sa_family = AF_INET;
    /* I want IP address attached to "eth0" */
    strncpy(ifr.ifr_name, Interface, IFNAMSIZ-1);
    int ret = ioctl(fd, SIOCGIFADDR, &ifr);
    close(fd);
    const sockaddr_in* IpAddress = new sockaddr_in;
    if(ret==0){
        IpAddress = (sockaddr_in *)&ifr.ifr_addr;
        return IpAddress;
    }
    else {
        delete IpAddress;
        return NULL;
    }
}

cMenuEditIpItem::cMenuEditIpItem(const char *Name, char *Value):cMenuEditItem(Name) {
	value = Value;
	curNum = -1;
	pos = -1;
	step = false;
	Set();
}

cMenuEditIpItem::~cMenuEditIpItem() {
}

void cMenuEditIpItem::Set(void) {
	char buf[1000];
	if (pos >= 0) {
		in_addr_t addr = inet_addr(value);
		if ((int)addr == -1)
			addr = 0;
		int p = 0;
		for (int i = 0; i < 4; ++i) {
			p += snprintf(buf + p, sizeof(buf) - p, pos == i ? "[%d]" : "%d",
					pos == i ? curNum : (addr >> (i * 8)) & 0xff);
			if (i < 3)
				buf[p++] = '.';
		}
		SetValue(buf);
	} else
		SetValue(value);
}

eOSState cMenuEditIpItem::ProcessKey(eKeys Key) {
	in_addr addr;
	addr.s_addr = inet_addr(value);
	if ((int)addr.s_addr == -1)
		addr.s_addr = 0;

	switch (Key) {
	case kUp:
		if (pos >= 0) {
			if (curNum < 255) ++curNum;
		} else
			return cMenuEditItem::ProcessKey(Key);
		break;

	case kDown:
		if (pos >= 0) {
			if (curNum > 0) --curNum;
		} else
			return cMenuEditItem::ProcessKey(Key);
		break;

	case kOk:
		if (pos >= 0) {
			addr.s_addr = inet_addr(value);
			if ((int)addr.s_addr == -1)
				addr.s_addr = 0;
			addr.s_addr &= ~(0xff << (pos * 8));
			addr.s_addr |= curNum << (pos * 8);
			strcpy(value, inet_ntoa(addr));
		} else
			return cMenuEditItem::ProcessKey(Key);
		curNum = -1;
		pos = -1;
		break;

	case kRight:
		if (pos >= 0) {
			addr.s_addr = inet_addr(value);
			if ((int)addr.s_addr == -1)
				addr.s_addr = 0;
			addr.s_addr &= ~(0xff << (pos * 8));
			addr.s_addr |= curNum << (pos * 8);
			strcpy(value, inet_ntoa(addr));
		}

		if (pos == -1 || pos == 3)
			pos = 0;
		else
			++pos;

		curNum = (addr.s_addr >> (pos * 8)) & 0xff;
		step = true;
		break;

	case kLeft:
		if (pos >= 0) {
			addr.s_addr = inet_addr(value);
			if ((int)addr.s_addr == -1)
				addr.s_addr = 0;
			addr.s_addr &= ~(0xff << (pos * 8));
			addr.s_addr |= curNum << (pos * 8);
			strcpy(value, inet_ntoa(addr));
		}

		if (pos <= 0)
			pos = 3;
		else
			--pos;

		curNum = (addr.s_addr >> (pos * 8)) & 0xff;
		step = true;
		break;

	    case k0 ... k9: /* Netbeans reports error with this line (.. is okay but wrong) */
                if (pos == -1)
			pos = 0;

		if (curNum == -1 || step) {
			curNum = Key - k0;
			step = false;
		} else
			curNum = curNum * 10 + (Key - k0);

		if ((curNum * 10 > 255) || (curNum == 0)) {
			in_addr addr;
			addr.s_addr = inet_addr(value);
			if ((int)addr.s_addr == -1)
				addr.s_addr = 0;
			addr.s_addr &= ~(0xff << (pos * 8));
			addr.s_addr |= curNum << (pos * 8);
			strcpy(value, inet_ntoa(addr));
			if (++pos == 4)
				pos = 0;
			curNum = (addr.s_addr >> (pos * 8)) & 0xff;
			step = true;
		}
		break;

	default:
		return cMenuEditItem::ProcessKey(Key);
	}

	Set();
	return osContinue;
}

const char* escapeXMLCharacters(const char* Data, char** Buf){
    if(Data==NULL){
        ERROR("Escape XML: No data to escape");
        return NULL;
    }
    std::string NewData = "";
    int Char = 0;
    for(unsigned int i = 0; i < strlen(Data); i++){
        Char = Data[i];
        switch(Char){
            case L'€': NewData += "&euro;"; break;
            case L'"': NewData += "&quot;"; break;
            case L'&': NewData += "&amp;"; break;
            case L'<': NewData += "&lt;"; break;
            case L'>': NewData += "&gt;"; break;
            case L'¡': NewData += "&iexcl;"; break;
            case L'¢': NewData += "&cent;"; break;
            case L'£': NewData += "&pound;"; break;
            case L'¤': NewData += "&curren;"; break;
            case L'¥': NewData += "&yen;"; break;
            case L'¦': NewData += "&brvbar;"; break;
            case L'§': NewData += "&sect;"; break;
            case L'¨': NewData += "&uml;"; break;
            case L'©': NewData += "&copy;"; break;
            case L'ª': NewData += "&ordf;"; break;
            case L'¬': NewData += "&not;"; break;
            case L'­': NewData += "&shy;"; break;
            case L'®': NewData += "&reg;"; break;
            case L'¯': NewData += "&macr;"; break;
            case L'°': NewData += "&deg;"; break;
            case L'±': NewData += "&plusmn;"; break;
            case L'²': NewData += "&sup2;"; break;
            case L'³': NewData += "&sup3;"; break;
            case L'´': NewData += "&acute;"; break;
            case L'µ': NewData += "&micro;"; break;
            case L'¶': NewData += "&para;"; break;
            case L'·': NewData += "&middot;"; break;
            case L'¸': NewData += "&cedil;"; break;
            case L'¹': NewData += "&sup1;"; break;
            case L'º': NewData += "&ordm;"; break;
            case L'»': NewData += "&raquo;"; break;
            case L'«': NewData += "&laquo;"; break;
            case L'¼': NewData += "&frac14;"; break;
            case L'½': NewData += "&frac12;"; break;
            case L'¾': NewData += "&frac34;"; break;
            case L'¿': NewData += "&iquest;"; break;
            case L'À': NewData += "&Agrave;"; break;
            case L'Á': NewData += "&Aacute;"; break;
            case L'Â': NewData += "&Acirc;"; break;
            case L'Ã': NewData += "&Atilde;"; break;
            case L'Ä': NewData += "&Auml;"; break;
            case L'Å': NewData += "&Aring;"; break;
            case L'Æ': NewData += "&AElig;"; break;
            case L'Ç': NewData += "&Ccedil;"; break;
            case L'È': NewData += "&Egrave;"; break;
            case L'É': NewData += "&Eacute;"; break;
            case L'Ê': NewData += "&Ecirc;"; break;
            case L'Ë': NewData += "&Euml;"; break;
            case L'Ì': NewData += "&Igrave;"; break;
            case L'Í': NewData += "&Iacute;"; break;
            case L'Î': NewData += "&Icirc;"; break;
            case L'Ï': NewData += "&Iuml;"; break;
            case L'Ð': NewData += "&ETH;"; break;
            case L'Ñ': NewData += "&Ntilde;"; break;
            case L'Ò': NewData += "&Ograve;"; break;
            case L'Ó': NewData += "&Oacute;"; break;
            case L'Ô': NewData += "&Ocirc;"; break;
            case L'Õ': NewData += "&Otilde;"; break;
            case L'Ö': NewData += "&Ouml;"; break;
            case L'×': NewData += "&times;"; break;
            case L'Ø': NewData += "&Oslash;"; break;
            case L'Ù': NewData += "&Ugrave;"; break;
            case L'Ú': NewData += "&Uacute;"; break;
            case L'Û': NewData += "&Ucirc;"; break;
            case L'Ü': NewData += "&Uuml;"; break;
            case L'Ý': NewData += "&Yacute;"; break;
            case L'Þ': NewData += "&THORN;"; break;
            case L'ß': NewData += "&szlig;"; break;
            case L'à': NewData += "&agrave;"; break;
            case L'á': NewData += "&aacute;"; break;
            case L'â': NewData += "&acirc;"; break;
            case L'ã': NewData += "&atilde;"; break;
            case L'ä': NewData += "&auml;"; break;
            case L'å': NewData += "&aring;"; break;
            case L'æ': NewData += "&aelig;"; break;
            case L'ç': NewData += "&ccedil;"; break;
            case L'è': NewData += "&egrave;"; break;
            case L'é': NewData += "&eacute;"; break;
            case L'ê': NewData += "&ecirc;"; break;
            case L'ë': NewData += "&euml;"; break;
            case L'ì': NewData += "&igrave;"; break;
            case L'í': NewData += "&iacute;"; break;
            case L'î': NewData += "&icirc;"; break;
            case L'ï': NewData += "&iuml;"; break;
            case L'ð': NewData += "&eth;"; break;
            case L'ñ': NewData += "&ntilde;"; break;
            case L'ò': NewData += "&ograve;"; break;
            case L'ó': NewData += "&oacute;"; break;
            case L'ô': NewData += "&ocirc;"; break;
            case L'õ': NewData += "&otilde;"; break;
            case L'ö': NewData += "&ouml;"; break;
            case L'÷': NewData += "&divide;"; break;
            case L'ø': NewData += "&oslash;"; break;
            case L'ù': NewData += "&ugrave;"; break;
            case L'ú': NewData += "&uacute;"; break;
            case L'û': NewData += "&ucirc;"; break;
            case L'ü': NewData += "&uuml;"; break;
            case L'ý': NewData += "&yacute;"; break;
            case L'þ': NewData += "&thorn;"; break;
            default: NewData += Data[i]; break;
        }
    }
    *Buf = strdup(NewData.c_str());
    return (*Buf);
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
char* ixmlGetFirstDocumentItem( IN IXML_Document * doc, IN const char *item, int* error ) {
    IXML_NodeList *nodeList = NULL;
    IXML_Node *textNode = NULL;
    IXML_Node *tmpNode = NULL;

    char *ret = NULL;
    *error = 0;
    
    nodeList = ixmlDocument_getElementsByTagName( doc, ( char * )item );
    
    if( nodeList != NULL ) {
        if( ( tmpNode = ixmlNodeList_item( nodeList, 0 ) ) ) {
            
            textNode = ixmlNode_getFirstChild( tmpNode );

            if(textNode != NULL){
                ret = strdup( ixmlNode_getNodeValue( textNode ) );
            }
        }
    } else {
        *error = -1;
    }
    
    if( nodeList != NULL) {
        ixmlNodeList_free( nodeList );
    }
        
   
    return ret;
}

int ixmlAddProperty(IXML_Document* document, IXML_Element* node, const char* upnpproperty, const char* value){
    if(!node) return -1;
    IXML_Element* PropertyNode = NULL;
    
    const char* attribute = att(upnpproperty);
    const char* property = prop(upnpproperty);
    if(attribute){
        if(strcasecmp(property,"")){
            ixmlElement_setAttribute(node, attribute, value);
        }
        else {
            IXML_NodeList* NodeList = ixmlElement_getElementsByTagName(node, property);
            if(NodeList!=NULL){
                IXML_Node* Node = ixmlNodeList_item(NodeList, 0);
                PropertyNode = (IXML_Element*) ixmlNode_getFirstChild(Node);
                if(PropertyNode){
                    ixmlElement_setAttribute(PropertyNode, attribute, value);
                }
                else {
                    ixmlNodeList_free(NodeList);
                    return -1;
                }
            }
            else {
                return -1;
            }
        }
    }
    else {
        PropertyNode = ixmlDocument_createElement(document, property);
        IXML_Node* PropertyText = ixmlDocument_createTextNode(document, value);
        ixmlNode_appendChild((IXML_Node*) PropertyNode, PropertyText);
        ixmlNode_appendChild((IXML_Node*) node, (IXML_Node*) PropertyNode);
    }
    return 0;
}
