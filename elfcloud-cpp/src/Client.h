/*** 
 * elfcloud.fi C++ Client
 * ===========================================================================
 * $Id$
 * 
 * LICENSE
 * ===========================================================================
 * Copyright 2010-2013 elfCLOUD /
 * elfcloud.fi - SCIS Secure Cloud Infrastructure Services
 * 
 *    Licensed under the Apache License, Version 2.0 (the "License");
 *    you may not use this file except in compliance with the License.
 *    You may obtain a copy of the License at
 *  
 *        http://www.apache.org/licenses/LICENSE-2.0
 *  
 *    Unless required by applicable law or agreed to in writing, software
 *    distributed under the License is distributed on an "AS IS" BASIS,
 *    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *    See the License for the specific language governing permissions and
 *    limitations under the License.
 *
 ***/

#ifndef ELFCLOUD_CLIENT_H_
#define ELFCLOUD_CLIENT_H_

#include <string>
#include <list>
#include <map>
#include <stdlib.h>
#include <memory>

#include "Object.h"

namespace elfcloud {

class Key;
class KeyRing;
class Vault;
class CryptoHelper;
class ServerConnection;
class Container;
class DataItem;

class Client: public elfcloud::Object {
private:
    std::string apiKey;
    std::string authMethod;
    std::string username;
    std::string password;
    std::string address;

    elfcloud::ServerConnection *serverConn;

    elfcloud::CryptoHelper *cryptoHelper;
    elfcloud::KeyRing *keyRing;

    std::map<std::string, std::string> mConf;
    void initializeServerConnection();

    // Controls library logging, 0 = disabled, higher values produces more log output (9=highest debug)
    static unsigned int logLevel;

    static std::string logPath;

    std::map<uint64_t, std::shared_ptr<Container>> mCacheContainer;
    std::map<uint64_t, std::shared_ptr<DataItem>> mCacheDataItem;

public:
    DllExport Client();
    ~Client();

    static void log(const std::string &pInLog);
    static void log(const std::string &pInLog, unsigned int pInEntryLevel);

    // Level of logging, 0 = Disabled (default), 1 = Minimal, 9 = Maximum
    // Effective immediately during library use.
    static void setLogLevel(unsigned int pInLogLevel) {
        logLevel=pInLogLevel;
    }

    // Defaults to "./ec-cpplib.log", set to log file base name "elfcloud.log" or full path "/tmp/ec.log".
    // Effective immediately during library use.
    static void setLogFilename(const std::string pInPath) {
        logPath=pInPath;
    }

    // CONFIGURATION OPTIONS MANAGEMENT

    std::string getConf(std::string pInKey) {
        std::map<std::string, std::string>::iterator i=mConf.find(pInKey);
        if (i==mConf.end()) return "not found";
        return (*i).second;
    }

    elfcloud::KeyRing *getKeyRing();

    std::shared_ptr<Container> getCacheContainer(uint64_t);
    std::shared_ptr<Container> setCacheContainer(std::shared_ptr<Container>);

    std::shared_ptr<DataItem> getCacheDataItem(uint64_t pInDataItemId);
    std::shared_ptr<DataItem> setCacheDataItem(std::shared_ptr<DataItem> pInDataItem);

    void clearCache();
    void clearCacheContainer();
    void clearCacheDataItem();

    DllExport void readUserConfig(const std::string &pInPath);

    // http.proxy in hostname:port format, can be prefixed as said in libcurl docs:
    //   "the proxy string may be specified with a protocol:// prefix to specify
    //    alternative proxy protocols. Use socks4://, socks4a://, socks5:// or
    //    socks5h:// (the last one to enable socks5 and asking the proxy to do
    //    the resolving"
    //
    // http.proxy.credentials in username:password format
    //
    // http.json.output with value of "1" will print JSON API reqs and responses
    //
    // http.data-api.header.output with value of "1" will print Data Item API
    //   headers for requests and responses.
    //
    // http.limit.receive.Bps = Bytes/s speed limit reading from cloud
    // http.limit.send.Bps    = Bytes/s speed limit pushing to cloud
    //
    // http.segment.size.bytes = segment size to cloud store (in bytes)
    void setConf(std::string pInKey, std::string pInValue);

    elfcloud::Vault *addVault(const std::string pInName, const std::string pInType);

	void setAPIKey(const std::string& pInAPIKey);
	void setAuthUsername(const std::string& pInUsername);
	void setAuthDataPassword(const std::string& pInPassword);
	void setAddress(const std::string& pInAddress);
	void setAuthMethod(const std::string& pInAuthMethod);

	std::string getAPIKey();
	std::string getAuthMethod();
	std::string getAuthUsername();
	std::string getAddress();

	DllExport void setPasswordAuthenticationCredentials(const std::string pInUsername, const std::string pInPassword);

	elfcloud::ServerConnection *getServerConnection();

private:
    struct _Client_impl_data *_impl_data;

public:

};

}

#endif

