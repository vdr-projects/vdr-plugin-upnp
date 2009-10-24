/* 
 * File:   config.cpp
 * Author: savop
 * 
 * Created on 15. August 2009, 13:03
 */

#include <stdio.h>
#include <vdr/tools.h>
#include <getopt.h>
#include "config.h"
#include "../common.h"

cUPnPConfig::cUPnPConfig(){
    this->mParsedArgs = NULL;
    this->mInterface = NULL;
    this->mAddress = NULL;
    this->mAutoSetup = 0;
    this->mEnable = 0;
    this->mPort = 0;
}

cUPnPConfig::~cUPnPConfig(){}

cUPnPConfig* cUPnPConfig::get(){
    if(cUPnPConfig::mInstance == NULL)
        cUPnPConfig::mInstance = new cUPnPConfig();

    if(cUPnPConfig::mInstance)
        return cUPnPConfig::mInstance;
    else return NULL;
}

cUPnPConfig* cUPnPConfig::mInstance = NULL;

bool cUPnPConfig::processArgs(int argc, char* argv[]){
  // Implement command line argument processing here if applicable.
    static struct option long_options[] = {
        {"int",     required_argument, NULL, 'i'},
        {"address", required_argument, NULL, 'a'},
        {"port",    required_argument, NULL, 'p'},
        {"autodetect", no_argument,    NULL, 'd'}
    };

    int c = 0;

    // Check if anything went wrong by setting 'success' to false
    // As there are multiple tests you may get a faulty behavior
    // if the current value is not considered in latter tests.
    // Asume that: Success = true;
    // success && false = false; --> new value of success is false
    // success && true = true; --> still true.
    // So, in the case of true and only true, success contains the
    // desired value.
    bool success = true;
    bool ifaceExcistent = false;
    bool addExcistent = false;

    while((c = getopt_long(argc, argv, "i:a:p:v",long_options, NULL)) != -1){
        switch(c){
            case 'i': 
                if(addExcistent) { ERROR("Address given but must be absent!"); return false; }
                success = this->parseSetup(SETUP_SERVER_INT, optarg) && success;
                success = this->parseSetup(SETUP_SERVER_ADDRESS, NULL) && success;
                success = this->parseSetup(SETUP_SERVER_AUTO, "0") && success;
                ifaceExcistent = true;
                break;
            case 'a':
                if(ifaceExcistent) { ERROR("Interface given but must be absent!"); return false; }
                success = this->parseSetup(SETUP_SERVER_ADDRESS, optarg) && success;
                success = this->parseSetup(SETUP_SERVER_INT, NULL) && success;
                success = this->parseSetup(SETUP_SERVER_AUTO, "0") && success;
                addExcistent = true;
                break;
            case 'p':
                success = this->parseSetup(SETUP_SERVER_PORT, optarg) && success;
                success = this->parseSetup(SETUP_SERVER_AUTO, "0") && success;
                break;
            case 'd':
                success = this->parseSetup(SETUP_SERVER_AUTO, optarg) && success;
            default:
                return false;
        }
    }

    return success;
}

bool cUPnPConfig::parseSetup(const char *Name, const char *Value)
{
    const char* ptr;
    if(*this->mParsedArgs && (ptr = strstr(this->mParsedArgs,Name))!=NULL){
        MESSAGE("Skipping %s=%s, was overridden in command line.",Name, Value);
        return true;
    }

    MESSAGE("VARIABLE %s has value %s", Name, Value);
    // Parse your own setup parameters and store their values.
    if      (!strcasecmp(Name, SETUP_SERVER_ENABLED))    this->mEnable = atoi(Value);
    else if (!strcasecmp(Name, SETUP_SERVER_AUTO))       this->mAutoSetup = atoi(Value);
    else if (!strcasecmp(Name, SETUP_SERVER_INT))        this->mInterface = strdup0(Value); // (Value) ? strn0cpy(this->mInterface, Value, strlen(this->mInterface)) : NULL;
    else if (!strcasecmp(Name, SETUP_SERVER_ADDRESS))    this->mAddress = strdup0(Value); //(Value) ? strn0cpy(this->mAddress, Value, strlen(this->mAddress)) : NULL;
    else if (!strcasecmp(Name, SETUP_SERVER_PORT))       this->mPort = atoi(Value);
    else return false;

    this->mParsedArgs = cString::sprintf("%s%s",*this->mParsedArgs,Name);

    return true;
}