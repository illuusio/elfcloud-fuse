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

#include <fstream>
#include <sstream>
#include <iostream>
#include <iomanip>
#include "Client.h"
#include "Vault.h"
#include "Cluster.h"
#include "ServerConnection.h"
#include "CryptoHelper.h"
#include "Key.h"
#include "KeyRing.h"
#include "Config.h"
#include <stdio.h>

#ifdef WINDOWS
#include <tchar.h>
#endif
#include <time.h>

#include <mutex>

using namespace CryptoPP;
using namespace std;

namespace elfcloud {

    //#pragma unmanaged
    mutex g_log_mutex; 

    typedef struct _Client_impl_data {
        CURL *curl;
        CURLcode res;
    } _Client_impl_data;

    unsigned int Client::logLevel = 0;
    std::string Client::logPath = "./ec-cpplib.log";

	DllExport Client::Client()
    {
	    serverConn = NULL;
    	serverConn = new ServerConnection(this);

        //_impl_data = new _Client_impl_data();

        // Default API key maps to elfcloud.fi standard clients allowed to use
        // fi.elfcloud.datastore and fi.elfcloud.backup type vaults.
        setAPIKey("atk8vzrhnc2by4f");
        setAddress("https://api.elfcloud.fi");
        mConf.clear();

		log("Client instantiated");

        keyRing=new KeyRing();
    }

    Client::~Client()
    {
        if (serverConn) {
            delete serverConn;
            serverConn=NULL;
        }

		if (keyRing) {
            delete keyRing;
        }

        mCacheContainer.clear();
        mCacheDataItem.clear();
    }

    Vault *Client::addVault(const string pInName, const string pInType)
    {
        Vault *tmp=new Vault(this, pInName, pInType);
        return tmp;
    }

    void Client::setAPIKey(const string& pInAPIKey)
    {
    	if (pInAPIKey.empty()) {
    		throw IllegalParameterException();
    	}

        apiKey = pInAPIKey;
    	serverConn->setAPIKey(pInAPIKey);
    }

    void Client::setAuthMethod(const string& pInAuthMethod)
    {
    	if (pInAuthMethod.compare("password")) {
    		throw IllegalParameterException();
    	}

        authMethod = pInAuthMethod;
    	serverConn->setAuthMethod(authMethod);
    }

    void Client::setAuthUsername(const string& pInUsername)
    {
    	if (pInUsername.empty()) {
    		throw IllegalParameterException();
    	}

    	username = pInUsername;
    	serverConn->setAuthUsername(username);
    }

    void Client::setAuthDataPassword(const string& pInPassword)
    {
    	if (pInPassword.empty()) {
    		throw IllegalParameterException();
    	}

    	password = pInPassword;
    	serverConn->setAuthPassword(password);
    }

    void Client::setAddress(const string& pInAddress) {

    	if (pInAddress.empty()) {
    		throw IllegalParameterException();
    	}

    	serverConn->setAddress(pInAddress);
    }

    ServerConnection* Client::getServerConnection() {
    	return serverConn;
    }

    string Client::getAuthMethod() {
    	return authMethod;
    }

    string Client::getAuthUsername() {
    	return username;
    }

    string Client::getAddress() {
    	return address;
    }

    string Client::getAPIKey(){
    	return apiKey;
    }

    elfcloud::KeyRing *Client::getKeyRing() {
        return keyRing;
    }

	DllExport void Client::setPasswordAuthenticationCredentials(const std::string pInUsername, const std::string pInPassword) {
        setAuthMethod("password");
        setAuthUsername(pInUsername);
        setAuthDataPassword(pInPassword);
    }

	DllExport void Client::readUserConfig(const std::string &pInPath) {
		Config *c=new Config();
        c->readUserConfig(pInPath);

        c->importKeysToKeyRing(getKeyRing());

        delete c;
    }

    void Client::log(const std::string &pInLog) {
        Client::log(pInLog, 1);
    }

	void Client::log(const std::string &pInLog, unsigned int pInEntryLevel) {
		// Level 0 = Disabled, will not log anything
		if (!logLevel || pInEntryLevel>logLevel)
			return;

		std::lock_guard<std::mutex> lock(g_log_mutex);

		time_t rawtime;
		struct tm * timeinfo;
		time (&rawtime);
		timeinfo = localtime (&rawtime);

		char tstampbuf[100];
		strftime(tstampbuf, 100, "%Y-%m-%d %H:%M:%S ",timeinfo);

		ofstream myfile;
		myfile.open(logPath.c_str(), std::ios::out | std::ios::ate | std::ios::app);
		myfile << tstampbuf << pInLog << std::endl;
		myfile.close();
	}

    void Client::setConf(std::string pInKey, std::string pInValue)
    {
        mConf[pInKey]=pInValue;

        std::string prefixSpeedLimit("http.limit.");
        if (serverConn && !pInKey.compare(0, prefixSpeedLimit.size(), prefixSpeedLimit)) {
            if (!pInKey.compare("http.limit.receive.Bps"))
                serverConn->setSpeedLimitReceive(atoi(pInValue.c_str()));
            if (!pInKey.compare("http.limit.send.Bps"))
                serverConn->setSpeedLimitSend(atoi(pInValue.c_str()));
        }
    }

    shared_ptr<Container> Client::getCacheContainer(uint64_t pInContainerId) {
        return mCacheContainer[pInContainerId];
    }

    shared_ptr<Container> Client::setCacheContainer(shared_ptr<Container> pInContainer) {

        shared_ptr<Container> current=mCacheContainer[pInContainer->getContainerId()];

        if (current) {
            // Update cached object with current data without replacing cached object,
            // existing pointers remain valid and the objects get updated contents.
            current->assign(pInContainer);
            stringstream ss;
            ss << "set(): Container cache update id " << pInContainer->getContainerId() << ", cache size= " << mCacheContainer.size();
            Client::log(ss.str(), 9);
            return current;
        } else {
            // New cache entry, store the object
            mCacheContainer[pInContainer->getContainerId()]=pInContainer;
            stringstream ss;
            ss << "set(): Container cache insert id " << pInContainer->getContainerId() << ", cache size= " << mCacheContainer.size();
            Client::log(ss.str(), 9);
            return pInContainer;
        }
    }

    shared_ptr<DataItem> Client::getCacheDataItem(uint64_t pInDataItemId) {
        return mCacheDataItem[pInDataItemId];
    }

    shared_ptr<DataItem> Client::setCacheDataItem(shared_ptr<DataItem> pInDataItem) {

        shared_ptr<DataItem> current=mCacheDataItem[pInDataItem->getId()];

        if (current) {
            // Update cached object with current data without replacing cached object,
            // existing pointers remain valid and the objects get updated contents.
            current->assign(pInDataItem);
            stringstream ss;
            ss << "set(): DataItem cache update id " << pInDataItem->getId() << ", cache size= " << mCacheDataItem.size();
            Client::log(ss.str(), 9);
            return current;
        } else {
            // New cache entry, store the object
            mCacheDataItem[pInDataItem->getId()]=pInDataItem;
            stringstream ss;
            ss << "set(): DataItem cache insert id " << pInDataItem->getId() << ", cache size= " << mCacheDataItem.size();
            Client::log(ss.str(), 9);
            return pInDataItem;
        }
    }

    void Client::clearCache() {
        clearCacheDataItem();
        clearCacheContainer();
    }

    void Client::clearCacheContainer() {
        mCacheContainer.clear();
    }

    void Client::clearCacheDataItem() {
        mCacheDataItem.clear();
    }

} // ns elfcloud

