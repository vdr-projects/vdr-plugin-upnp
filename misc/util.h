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
struct strCmp { bool operator()(const char* s1, const char* s2) const { return (strcmp(s1,s2) < 0); }};
const sockaddr_in* getIPFromInterface(const char* Interface);
const char* getMACFromInterface(const char* Interface);
char** getNetworkInterfaces(int *count);
char* ixmlGetFirstDocumentItem( IN IXML_Document * doc, IN const char *item, int* error );
int ixmlAddProperty(IN IXML_Document* document, IN IXML_Element* node, const char* upnpproperty, const char* value );
char* substr(const char* str, unsigned int offset, unsigned int length);
}
#endif

const char* escapeSQLite(const char* Data, char** Buf);
const char* escapeXMLCharacters(const char* Data, char** Buf);

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

