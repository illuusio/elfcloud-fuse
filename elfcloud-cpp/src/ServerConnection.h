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

#ifndef ELFCLOUD_SERVERCONNECTION_H_
#define ELFCLOUD_SERVERCONNECTION_H_

#include "Object.h"
//#include "Client.h"

//#ifdef ELFCLOUD_LIB
#include <json/json.h>
#include <json/value.h>
#include <curl/curl.h>

#include "ext_cryptopp.h"
//#include <cryptopp/md5.h>
//#endif

#include <map>
#include <string>

using namespace std;

typedef enum elfcloudInterfaceType {
	ELFCLOUD_INTERFACE_JSON = 1,
	ELFCLOUD_INTERFACE_STORE,
	ELFCLOUD_INTERFACE_FETCH
} elfcloudInterfaceType;

typedef enum DataItemStoreMode {
	ELFCLOUD_STORE_MODE_NEW = 1,
	ELFCLOUD_STORE_MODE_REPLACE,
	ELFCLOUD_STORE_MODE_APPEND,
	ELFCLOUD_STORE_MODE_PATCH
} DataItemStoreMode;

namespace elfcloud {
    class Client;
    class Container;
    class DataItemFilePassthrough;
}

typedef struct httpBuffer {
		unsigned int bytesUsed;
		unsigned char* buffer;
		unsigned int bufferSize;
		httpBuffer* nextBuffer;

        // Passthrough fetch handlers
        std::shared_ptr<elfcloud::CryptoHelper> cryptoHelper;
        std::shared_ptr<elfcloud::DataItemFilePassthrough> dataitem;
        unsigned int bytesWritten;
        elfcloud::Client *client;
        std::string serverResponseHash;

        // ELFCLOUD response headers when used as header buffer
        std::map<std::string, std::string> headers;

        httpBuffer() {
            bytesUsed=0;
            buffer=0;
            bufferSize=0;
            nextBuffer=0;
            bytesWritten=0;
            client=0;
        }

} httpBuffer;

namespace elfcloud {


class ServerConnection: public elfcloud::Object {
public:
	ServerConnection(Client *pInelfcloudClient);
	virtual ~ServerConnection();

#ifdef ELFCLOUD_LIB
    Json::Value performServerJSONRequest(const Json::Value& pInServerRequest, bool pInAuthRequest = false);
#endif
    void setAddress(const string& pInAddress);

	void performStoreRequestWithFile(
			const DataItemStoreMode pInStoreMode,
			const string& pInKey,
			const unsigned int pInPatchOffset,
			Container& pInParent,
			const string& pInDataPtr,
			const unsigned int pInDataSize,
			Client* client);

	void performStoreRequestWithBuffer(
			const DataItemStoreMode pInStoreMode,
			const string& pInKey,
			const unsigned int pInPatchOffset,
			Container& pInParent,
			unsigned char* pInDataPtr,
			const unsigned int pInDataSize);

	void performFetchRequest(
			const string& pInKey,
			Container& pInParent,
			unsigned char **pOutDataBuffer,
			unsigned int *pOutDataLength,
			Client* client);

	void performServerCoreRequest(const map<string, string> &pInMapRequestHeaders,
			const byte *pInRequestBody,
			const unsigned int pInBodyLength,
			map<string, string> &pOutMapResponseHeaders,
			byte **pOutResponseBody,
			unsigned int *pOutResponseBodyLength,
			const elfcloudInterfaceType pInInterfaceType,
            shared_ptr<DataItemFilePassthrough> pInDataItem=0);

	void setAPIKey(const string& pInAPIKey);
	void setAuthUsername(const string& pInUsername);
	void setAuthPassword(const string& pInPassword);
	void setAuthMethod(const string& pInAuthMethod);

    void setSpeedLimitReceive(unsigned int pInLimit) {
        speedLimitReceive=pInLimit;
    }

    void setSpeedLimitSend(unsigned int pInLimit) {
        speedLimitSend=pInLimit;
    }

	static size_t write_data(void *ptr, size_t size, size_t nmemb, void *userData);
    static size_t write_header(void *ptr, size_t size, size_t nmemb, void *userData);

private:
	bool authenticated;
#ifdef ELFCLOUD_LIB
    CURL *curl;
	CURLcode res;
#endif

	Client *client;

	// elfcloud.org server JSON interface URL
	string address;

	// elfcloud.org server interface URLs for Data Item API Store and Fetch operations
	string dataItemAPIStoreAddress;
	string dataItemAPIFetchAddress;

	string getAPIKey();
	string getAuthMethod();
	string getAuthUsername();
	string getAuthPassword();
	string getAddress();

	static void parseHeader(std::map<string, string>& pOutelfcloudHeaders, std::map<string, string>& pOutAllHeaders, httpBuffer* pInHeaderBuffer);

	string apiKey;
	string authMethod;
	string username;
	string password;

    // Bytes/second, enforced by libcurl. Value of 0 means unlimited speed.
    unsigned int speedLimitSend;

    // Bytes/second, enforced by libcurl. Value of 0 means unlimited speed.
    unsigned int speedLimitReceive;

#ifdef ELFCLOUD_LIB
	Json::Value createRequestAuth();
#endif

	void deleteBufferChain(httpBuffer *httpBodyBuffer);

	// Combine given buffer chain into a single node chain. Returns a new httpBuffer structs that has no further nodes.
	// Both old and new buffers must be released by the caller.
	httpBuffer mergeBufferChain(const httpBuffer& pInBuffer);

	void ensureAuthenticatedState();
};
}

#endif

