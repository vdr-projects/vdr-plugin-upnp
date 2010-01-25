/*
 * File:   util.h
 * Author: savop
 *
 * Created on 21. Mai 2009, 21:25
 */

#ifndef _UTIL_H
#define	_UTIL_H

#include <vdr/tools.h>
#include <vdr/plugin.h>
#include <upnp/ixml.h>

#ifdef __cplusplus
extern "C" {
#if 0
}
#endif

#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
/**
 * Compares two strings
 *
 * This struct compares two strings and returns true on equality or false otherwise
 * It is used in conjuction with hashmaps
 */
struct strCmp {
    /**
     * Compares two strings
     * @return returns
     * - \bc true, in case of equality
     * - \bc false, otherwise
     * @param s1 the first string
     * @param s2 the second string
     */
    bool operator()(const char* s1, const char* s2) const { return (strcmp(s1,s2) < 0); }
};
/**
 * Gets the IP address
 *
 * Returns the IP address of a given interface. The interface must be a valid interface
 * identifier like eth0 or wlan1.
 *
 * @return a structure containing the IP address
 * @param Interface to obtain the IP from
 */
const sockaddr_in* getIPFromInterface(const char* Interface);
/**
 * Gets the MAC address
 *
 * Returns a string representation of the MAC address of a given interface.  The interface
 * must be a valid interface identifier like eth0 or wlan1.
 *
 * The pattern of the address is sixth byte hex number separated with ":"
 *
 * @return a string containing the MAC
 * @param Interface to obtain the MAC from
 */
const char* getMACFromInterface(const char* Interface);
/**
 * List with interfaces
 *
 * Returns an array with interfaces found on the system. The number of items
 * in the array is stored in the parameter \c count.
 *
 * @return array list of interfaces
 * @param count number of interfaces in the array
 */
char** getNetworkInterfaces(int *count);
/**
 * First occurance of item
 *
 * Finds the first occurance of a specified item in a given \bc IXML document and returns its value.
 * If an error occures, its code is stored in the last parameter \c 'error'.
 *
 * @return the value of the item
 * @param doc the \c IXML document to be parsed
 * @param item the item which shall be found
 * @param error the error code in case of an error
 */
char* ixmlGetFirstDocumentItem( IN IXML_Document * doc, IN const char *item, int* error );
/**
 * Adds a property
 *
 * This adds a UPnP property to an \bc IXML document.
 * The property must have the pattern "namespace:property@attribute".
 *
 * @return returns
 * - \bc <0, in case of an error
 * - \bc 0, otherwise
 * @param document the \c IXML document to put the parameter in
 * @param node the specific node where to put the parameter
 * @param upnpproperty the upnp property
 * @param value the value of the upnp property
 */
int ixmlAddProperty(IN IXML_Document* document, IN IXML_Element* node, const char* upnpproperty, const char* value );
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

char* duration(off64_t duration, unsigned int timeBase = 1);

#if 0
{
#endif
}
#endif

/**
 * Escapes XML special characters
 *
 * This function escapes XML special characters, which must be transformed before
 * inserting it into another XML document.
 *
 * @return the escaped document
 * @param Data the data to escape
 * @param Buf the pointer where the escaped document shall be stored
 */
const char* escapeXMLCharacters(const char* Data, char** Buf);

/** @private */
class cMenuEditIpItem: public cMenuEditItem {
private:
	char *value;
	int curNum;
	int pos;
	bool step;
protected:
	virtual void Set(void);
public:
	cMenuEditIpItem(const char *Name, char *Value); // Value must be 16 bytes
	~cMenuEditIpItem();
	virtual eOSState ProcessKey(eKeys Key);
};

#endif	/* _UTIL_H */

