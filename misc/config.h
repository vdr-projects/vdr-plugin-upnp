/* 
 * File:   config.h
 * Author: savop
 *
 * Created on 15. August 2009, 13:03
 */

#ifndef _CONFIG_H
#define	_CONFIG_H

#include <vdr/tools.h>
#include "../common.h"

class cUPnPConfig {
private:
    static cUPnPConfig* mInstance;    
    cString mParsedArgs;
    cUPnPConfig();
public:
    char* mInterface;
    char* mAddress;
    int   mPort;
    int   mEnable;
    int   mAutoSetup;
public:
    virtual ~cUPnPConfig();
    static cUPnPConfig* get();
    bool  parseSetup(const char* Name, const char* Value);
    bool  processArgs(int argc, char* argv[]);
};

#endif	/* _CONFIG_H */

