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

#define CRYPTOPP_ENABLE_NAMESPACE_WEAK 1

#include "Client.h"
#include "ServerConnection.h"
#include "Exception.h"
#include "IllegalParameterException.h"
#include "DataItem.h"
#include "CryptoHelper.h"
#include "Key.h"
#include "KeyHint.h"

#include <curl/curl.h>
#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <iomanip>

using namespace std;
using namespace CryptoPP;

namespace elfcloud {

ServerConnection::ServerConnection(Client *pInelfcloudClient): speedLimitSend(0), speedLimitReceive(0) {
	authenticated = false;
    curl_global_init(CURL_GLOBAL_DEFAULT);
    curl = curl_easy_init();
	client=pInelfcloudClient;
	res=CURLE_OK;
}

ServerConnection::~ServerConnection() {
     curl_easy_cleanup(curl);
     curl_global_cleanup();
}

void ServerConnection::setAddress(const string& pInAddress) {
	address = pInAddress;
	address.append("/1.2/json");

	dataItemAPIStoreAddress = pInAddress;
	dataItemAPIStoreAddress.append("/1.2/store");

	dataItemAPIFetchAddress = pInAddress;
	dataItemAPIFetchAddress.append("/1.2/fetch");
}

string ServerConnection::getAddress() {
	return address;
}

void ServerConnection::ensureAuthenticatedState() {

	if (!authenticated) {
		Json::Value jsonAuth = createRequestAuth();
		Json::Value jsonAuthResp = performServerJSONRequest(jsonAuth, true);

		if (!jsonAuthResp.isNull()) {
			authenticated = true;
		} else {
			throw Exception(ECSCI_EXC_INIT_OR_PARAM_FAILURE, "Authentication not accepted by server");
		}
	}
}

Json::Value ServerConnection::createRequestAuth() {

	Json::Value auth(Json::objectValue);
	auth["method"] = Json::Value("auth");
	auth["params"] = Json::Value(Json::objectValue);
	auth["params"]["username"] = Json::Value(getAuthUsername());
	auth["params"]["auth_method"] = Json::Value(getAuthMethod());
	auth["params"]["auth_data"] = Json::Value(getAuthPassword());
	auth["params"]["apikey"] = Json::Value(getAPIKey());

	return auth;
}

string ServerConnection::getAPIKey() {
	return apiKey;
}

string ServerConnection::getAuthPassword() {
	return password;
}

string ServerConnection::getAuthMethod() {
	return authMethod;
}

string ServerConnection::getAuthUsername() {
	return username;
}

// Exception is thrown whenever server returns an error object
Json::Value ServerConnection::performServerJSONRequest(const Json::Value& pInServerRequest, bool pInAuthRequest) {

	string jsonOutput = pInServerRequest.toStyledString();
	if (jsonOutput.empty()) {
		throw IllegalParameterException();
	}

	Json::Value root;
	Json::Reader reader(Json::Features::all());

	byte *requestBody=(byte*) jsonOutput.c_str();

    // performServerCoreRequest will allocate response buffer and hand over ownership to us
	byte *responseBody=0;
	unsigned int responseBodyLength=0;

	map<string, string> requestHeaders;
	map<string, string> responseHeaders;

	int attempts=2;
	while (attempts>0) {

		if (!pInAuthRequest)
			ensureAuthenticatedState();

		if (responseBody) {
            delete[] responseBody;
			responseBody=NULL;
		}

		responseBodyLength=0;
		requestHeaders.clear();
		responseHeaders.clear();

        if (!client->getConf("http.json.output").compare("1")) {
			stringstream ss;
			ss << "JSON-REQUEST" << endl
				<< "=============================================" << endl
                << jsonOutput;
			Client::log(ss.str());
        }

		performServerCoreRequest(requestHeaders,
				requestBody,
				strlen((const char*) requestBody),
				responseHeaders,
				&responseBody,
				&responseBodyLength,
				ELFCLOUD_INTERFACE_JSON);

		attempts--;

		string strBody((const char*) responseBody, responseBodyLength);

        // Response body buffer can be released from the heap, we'll just keep the string copy for JSON parsing and error outputs
        delete[] responseBody;
        responseBody=NULL;

		bool parsingSuccessful = reader.parse(strBody, root);

		if (!parsingSuccessful) {
			stringstream ss;
			ss << "Failed to parse server JSON response: " << reader.getFormattedErrorMessages() << strBody;
			Client::log(ss.str());
            continue;
		}

        if (!client->getConf("http.json.output").compare("1")) {
			stringstream ss;
			ss << "JSON-RESPONSE" << endl
				<< "=============================================" << endl
                << root.toStyledString();
			Client::log(ss.str());
        }

		Json::Value error=root.get("error", Json::Value().null);

		if (!error.isNull()) {
			Json::Value errorId=error.get("code", Json::Value().null);
			int errorNumber=errorId.asInt();

			if (101==errorNumber) { // TODO: Client authorization failure --> ENUM
                stringstream ss;
                ss << "Server error " << errorNumber << ": " << error.get("message", Json::Value("n/a"));
				Client::log(ss.str());
				authenticated=false;
				continue;
			} else {
                stringstream ss;
                ss << "Server error " << errorNumber << ": " << error.get("message", Json::Value("n/a"));
				Client::log(ss.str());
                throw elfcloud::Exception(ECSCI_EXC_BACKEND_EXCEPTION, ss.str());
			}
		} // if error element is present
		else {
			// No error element present
            Json::Value::Members members=root.getMemberNames();
            for (unsigned int i=0; i<members.size(); i++)
                if (!members[i].compare("result")) {
					stringstream ss;
					ss << "Returning result from JSON query";
					Client::log(ss.str(), 9);
                    return root.get("result", Json::Value::null);
				}
            std::cout  << "No error or result element present in the server JSON response: " << strBody << endl;
            throw Exception();
		}

	} // while attempts remaining

	throw Exception(ECSCI_EXC_REQUEST_PROCESSING_FAILED, "Unknown error in SCI request processing, ran out of attempts");
}

void ServerConnection::setAPIKey(const string& pInAPIKey) {

	if (pInAPIKey.empty()) {
		throw IllegalParameterException();
	}
	apiKey = pInAPIKey;
}

void ServerConnection::setAuthMethod(const string& pInAuthMethod) {

	if (pInAuthMethod.compare("password")) {
		throw IllegalParameterException();
	}
	authMethod = pInAuthMethod;
}

void ServerConnection::setAuthUsername(const string& pInUsername) {

	if (pInUsername.empty()) {
		throw IllegalParameterException();
	}
	username = pInUsername;
}

void ServerConnection::setAuthPassword(const string& pInPassword) {

	if (pInPassword.empty()) {
		throw IllegalParameterException();
	}
	password = pInPassword;
}

// Upon successful return (no throw), pOutResponseBody will have a byte[] buffer pointer left
// to be freed by the caller. Length of the buffer will be set to pOutResponseBodyLength. Response
// HTTP headers will be populated into the pOutMapResponseHeaders map.
void ServerConnection::performServerCoreRequest(const map<string, string> &pInMapRequestHeaders,
		const byte *pInRequestBody,
		const unsigned int pInBodyLength,
		map<string, string> &pOutMapResponseHeaders,
		byte **pOutResponseBody,
		unsigned int *pOutResponseBodyLength,
		const elfcloudInterfaceType pInInterfaceType,
        shared_ptr<DataItemFilePassthrough> pInDataItem) {

    (*pOutResponseBody)=NULL;
    (*pOutResponseBodyLength)=0;
    pOutMapResponseHeaders.clear();

	if (!curl) {
		Client::log("ServerConnection/performServerCoreRequest(): CURL library not initialized");
        throw Exception(ECSCI_EXC_INIT_OR_PARAM_FAILURE, "CURL library not initialized");
	}

	httpBuffer httpHeaderBuffer;
	httpHeaderBuffer.bufferSize = 10000;
	httpHeaderBuffer.buffer = new byte[httpHeaderBuffer.bufferSize];
    httpHeaderBuffer.dataitem=pInDataItem;

	httpBuffer httpBodyBuffer;
	httpBodyBuffer.bufferSize = 5000;
	httpBodyBuffer.buffer = new byte[httpBodyBuffer.bufferSize];
    httpBodyBuffer.dataitem=pInDataItem;
    httpBodyBuffer.client=client;

	struct curl_slist *headers=NULL;
	map<string, string>::const_iterator cit=pInMapRequestHeaders.begin();
	while (cit!=pInMapRequestHeaders.end()) {
		string headerRow=(*cit).first;
		headerRow.append(": ");
		headerRow.append((*cit).second);
        if (pInInterfaceType!=ELFCLOUD_INTERFACE_JSON) {
            if (!client->getConf("http.data-api.header.output").compare("1")) {
				stringstream ss;
				ss << "Data Item API request header: " << headerRow;
				Client::log(ss.str(), 5);
			}
        }
		headers = curl_slist_append(headers, headerRow.c_str());
		cit++;
	}

	char tmpBuffer[4096];
	sprintf(tmpBuffer, "Content-Length: %u", pInBodyLength);
	headers = curl_slist_append(headers, tmpBuffer);

	curl_easy_reset(curl);

	switch (pInInterfaceType) {
    	case ELFCLOUD_INTERFACE_JSON: {
    		headers = curl_slist_append(headers, "Content-type: application/json; charset=utf-8");
    		curl_easy_setopt(curl, CURLOPT_URL, address.c_str());
			stringstream ss;
			ss << "Sending JSON request of " << pInBodyLength << " bytes to URL " << address.c_str();
			Client::log(ss.str(), 3);
    		break;
    	}
    	case ELFCLOUD_INTERFACE_STORE: {
    		headers = curl_slist_append(headers, "Content-type: application/octet-stream");
    		curl_easy_setopt(curl, CURLOPT_URL, dataItemAPIStoreAddress.c_str());
			stringstream ss;
			ss << "Sending STORE request of " << pInBodyLength << " bytes to URL " << dataItemAPIStoreAddress.c_str();
			Client::log(ss.str(), 3);
    		break;
    	}
    	case ELFCLOUD_INTERFACE_FETCH: {
    		headers = curl_slist_append(headers, "Content-type: application/octet-stream");
    		curl_easy_setopt(curl, CURLOPT_URL, dataItemAPIFetchAddress.c_str());

			stringstream ss;
			ss << "Sending FETCH request of " << pInBodyLength << " bytes to URL " << dataItemAPIFetchAddress.c_str();
			Client::log(ss.str(), 3);

    		break;
    	}
    	default: {
            curl_slist_free_all(headers);
            deleteBufferChain(&httpHeaderBuffer);
            deleteBufferChain(&httpBodyBuffer);
			Client::log("ServerConnection/performServerCoreRequest(): Bad interface type specified");
    		throw Exception(ECSCI_EXC_INIT_OR_PARAM_FAILURE, "Bad interface type specified");
        }
	}

	curl_easy_setopt(curl, CURLOPT_POSTFIELDS, pInRequestBody);
	curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, pInBodyLength);
	curl_easy_setopt(curl, CURLOPT_COOKIEFILE, "");
	curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 1L);
	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, ServerConnection::write_data);
	curl_easy_setopt(curl, CURLOPT_WRITEDATA, &httpBodyBuffer);
    curl_easy_setopt(curl, CURLOPT_HEADERFUNCTION, ServerConnection::write_header);
	curl_easy_setopt(curl, CURLOPT_WRITEHEADER, &httpHeaderBuffer);
	curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

    if (speedLimitReceive>0) {
        curl_off_t limit=speedLimitReceive;
        curl_easy_setopt(curl, CURLOPT_MAX_RECV_SPEED_LARGE, limit);
    }
    if (speedLimitSend>0) {
        curl_off_t limit=speedLimitSend;
        curl_easy_setopt(curl, CURLOPT_MAX_SEND_SPEED_LARGE, limit);
    }

#ifdef WINDOWS
	curl_easy_setopt(curl, CURLOPT_CAINFO, "curl-ca-bundle.crt");
#endif

    if (client->getConf("http.proxy").compare("not found")) {
        curl_easy_setopt(curl, CURLOPT_PROXY, client->getConf("http.proxy").c_str());
        if (client->getConf("http.proxy.credentials").compare("not found")) {
            curl_easy_setopt(curl, CURLOPT_PROXYUSERPWD, client->getConf("http.proxy.credentials").c_str());
        }
    }

    res = curl_easy_perform(curl);

    if (res!=0) {
		stringstream ss;
		ss << "ServerConnection/performServerCoreRequest(): CURL error code: " << res;
		Client::log(ss.str(), 1);
        curl_slist_free_all(headers);

        deleteBufferChain(&httpHeaderBuffer);
        deleteBufferChain(&httpBodyBuffer);

        throw Exception(ECSCI_EXC_REQUEST_PROCESSING_FAILED, "Error performing HTTP operation (libcurl)");
    } else {
        stringstream ss;
		ss << "ServerConnection/performServerCoreRequest(): CURL OK";
		Client::log(ss.str(), 9);
    }

    if (ELFCLOUD_INTERFACE_FETCH==pInInterfaceType && httpBodyBuffer.cryptoHelper.get() && httpHeaderBuffer.serverResponseHash.size()) {
        std::string calculatedHash=httpBodyBuffer.cryptoHelper->getHashEncryptedDataStream();
        //cout << "3-verifying post-passthrough-fetch hash. Calculated=" << calculatedHash << ", server: " << httpHeaderBuffer.serverResponseHash << endl;

        if (calculatedHash.compare(httpHeaderBuffer.serverResponseHash)) {
            stringstream ss;
            ss << "Passthrough fetch X-ELFCLOUD-HASH mismatch, local: " << calculatedHash << ", remote: " << httpHeaderBuffer.serverResponseHash;
            Client::log(ss.str(), 1);
            throw Exception(ECSCI_EXC_REQUEST_PROCESSING_FAILED, "Data item hash mismatch during passthrough fetch processing");
        }
    }

    // Merge allocated header and body fragments into single buffers. If there are no "next buffers" in the chains, let's not bother,
    // there may be some extra space in the buffers but bytesUsed will indicate the length consumed.
    {
        if (httpHeaderBuffer.nextBuffer) {
            httpBuffer httpMergedHeader=mergeBufferChain(httpHeaderBuffer);
            deleteBufferChain(&httpHeaderBuffer);
            httpHeaderBuffer=httpMergedHeader;
        }

        if (httpBodyBuffer.nextBuffer) {
            httpBuffer httpMergedBody=mergeBufferChain(httpBodyBuffer);
            deleteBufferChain(&httpBodyBuffer);
            httpBodyBuffer=httpMergedBody;
        }
    }

	if (pInInterfaceType!=ELFCLOUD_INTERFACE_JSON) {
        if (!client->getConf("http.data-api.header.output").compare("1")) {
			stringstream ss;
            ss << "ServerConnection/performServerCoreRequest(): DATA ITEM API RESPONSE HEADERS" << endl
                 << "=============================================" << endl
                 << string((const char*) httpHeaderBuffer.buffer, httpHeaderBuffer.bytesUsed);
			Client::log(ss.str(), 6);
        }
    }

	curl_slist_free_all(headers);

	// Parse HTTP headers from the server's response and pass all received headers upstream to the calling function
	std::map<string, string> elfcloudHeaders;
	parseHeader(elfcloudHeaders, pOutMapResponseHeaders, &httpHeaderBuffer);

    (*pOutResponseBody) = httpBodyBuffer.buffer;
    (*pOutResponseBodyLength) = httpBodyBuffer.bytesUsed;

	{
		stringstream ss;
		ss << "ServerConnection/performServerCoreRequest(): Response HTTP body length " << httpBodyBuffer.bytesUsed;
		Client::log(ss.str(), 5);
	}

	deleteBufferChain(&httpHeaderBuffer);
}

size_t ServerConnection::write_header(void *ptr, size_t size, size_t nmemb, void *userData) {

	httpBuffer* httpBuf = (httpBuffer*) userData;

    // When dataitem has been defined, parse and populate metaheader http response header immediately when received.
    // This is required for passthrough dataitem fetch processing, when decryption is done already in curl callback function.
    if (httpBuf->dataitem.get()) {
        char temp[size*nmemb+1];
        memset(temp, 0, size*nmemb+1);
        memcpy(temp, ptr, size*nmemb);

        unsigned int headerLength=strlen(temp);
        if (headerLength>2) {
            for (unsigned int i=1; i<=2; i++)
                if ('\n'==temp[headerLength-i] || '\r'==temp[headerLength-i])
                    temp[headerLength-i]=0;
        }

        const unsigned int xMetaLen=strlen("X-ELFCLOUD-META: ");
        const unsigned int xHashLen=strlen("X-ELFCLOUD-HASH: ");

        if (strlen(temp)>xMetaLen && strncmp("X-ELFCLOUD-META: ", temp, xMetaLen)==0) {
            // X-ELFCLOUD-META: v1:ENC:AES256:KHA:2cb40902eab770edbe7a2e57506bb467:DSC:::
            string headerStr(&temp[xMetaLen]); 
            httpBuf->dataitem->parseMetaDataString(headerStr);
        }

        if (strlen(temp)>xHashLen && strncmp("X-ELFCLOUD-HASH: ", temp, xHashLen)==0) {
            // X-ELFCLOUD-HASH: ffc0b831c421f9ca6eceb1ae0a73434e
            httpBuf->serverResponseHash.assign(&temp[xHashLen]);
        }

    }

	while (httpBuf->nextBuffer!=0) {
		httpBuf = httpBuf->nextBuffer;
	}

	size_t freeBufferMemory = 0;
	freeBufferMemory = httpBuf->bufferSize - httpBuf->bytesUsed;

	if ((size*nmemb) <= freeBufferMemory) {
		memcpy(&httpBuf->buffer[httpBuf->bytesUsed], ptr, size*nmemb);
		httpBuf->bytesUsed += size*nmemb;
	} else {
		memcpy(&httpBuf->buffer[httpBuf->bytesUsed], ptr, freeBufferMemory);
		httpBuf->bytesUsed += freeBufferMemory;

		httpBuf->nextBuffer = new httpBuffer;
		httpBuf->nextBuffer->nextBuffer = NULL;

		httpBuf->nextBuffer->bufferSize = 50000;
		if ((size*nmemb)>(httpBuf->nextBuffer->bufferSize-5000)) {
			httpBuf->nextBuffer->bufferSize += (size*nmemb);
		}

		httpBuf->nextBuffer->bytesUsed = 0;
		httpBuf->nextBuffer->buffer = new byte[httpBuf->nextBuffer->bufferSize];

        if (0==httpBuf->nextBuffer->buffer) {
            // Failed to allocate new chained buffer, remove references to new buffer and return only the amount of bytes cleared
            // that got copied into the original last buffer in the chain.
            delete httpBuf->nextBuffer;
            httpBuf->nextBuffer=NULL;

            // Returns to CURL the number of bytes copied to original buffer, the rest cannot be accepted. Original buffer is now full.
            return freeBufferMemory;
        }

        // All provided data has been copied. The function will return with "all given bytes copied" status.
        memcpy(&httpBuf->nextBuffer->buffer[httpBuf->nextBuffer->bytesUsed],
               (void*) ((unsigned long) ptr+freeBufferMemory), (size*nmemb)-freeBufferMemory);
		httpBuf->nextBuffer->bytesUsed += ((size*nmemb)-freeBufferMemory);
	}

	return size*nmemb;
}

size_t ServerConnection::write_data(void *ptr, size_t size, size_t nmemb, void *userData) {

	httpBuffer* httpBuf = (httpBuffer*) userData;

    if (httpBuf->dataitem.get()) {
        // Passthrough mode, decrypt on the fly and write to output stream
        if (httpBuf->bytesWritten==0 && httpBuf->cryptoHelper==0) {
            // Initialize cryptohelper continuous cipher
            httpBuf->cryptoHelper.reset(new CryptoHelper());

            std::map<std::string, std::string> metaKV=httpBuf->dataitem->getMetaHeaderKVPairs();

            string enc=metaKV["ENC"];
            string kha=metaKV["KHA"];

            if (!enc.size() || !kha.size()) {
                Client::log("Data item is missing encryption information in the meta header, cannot process passthrough fetch write", 1);
                return 0;
            }

            std::ofstream outputStream(httpBuf->dataitem->getFilePath(), std::fstream::out|std::fstream::binary|std::fstream::trunc);
            if (outputStream.fail()) {
                Client::log("Unable to truncate target file, cannot process passthrough fetch write", 1);
                return 0;
            }
            outputStream.close();

            if (enc.compare("NONE")) {
                // Resolve DI key and initialize cryptohelper for stream decryption
                KeyHint kHint(ECSCI_HASHALG_MD5, kha, CryptoHelper::getEncryptionAlgorithm(enc));
                httpBuf->dataitem->setKeyHint(&kHint);

                // Key pointers returned by KeyRing are copies and must be free'd
                shared_ptr<elfcloud::Key> key;

                try {
                    // Throws if the key is not found
                    key.reset(httpBuf->client->getKeyRing()->getCipherKey(kHint));
                    httpBuf->cryptoHelper->decryptDataStreamBegin(key.get());
                } catch (...) {
                    stringstream ss;
                    ss << "Decryption key for hash " << kha << " and mode " << enc << " could not be found, cannot process passthrough fetch write";
                    Client::log(ss.str(), 1);
                    return 0;
                }
            }
        }

        if (httpBuf->cryptoHelper) {
            // If cryptohelper has been initialized, decrypt the chunk in-place before writing it out
            if (false==httpBuf->cryptoHelper->decryptDataStreamContinue((const byte*) ptr, (byte*) ptr, size*nmemb)) {
                Client::log("Decryption failed, cannot process passthrough fetch write", 1);
                return 0;
            }
        }

        std::ofstream outputStream(httpBuf->dataitem->getFilePath(), std::fstream::out|std::fstream::binary|std::fstream::app);
        if (outputStream.fail()) {
            Client::log("Unable to write to target file, cannot process passthrough fetch write", 1);
            return 0;
        }
        outputStream.write((const char*) ptr, size*nmemb);
        if (outputStream.fail()) {
            Client::log("Unable to write to target file, cannot process passthrough fetch write", 1);
            return 0;
        }
        outputStream.close();
        httpBuf->bytesWritten=httpBuf->bytesWritten+size*nmemb;
        return size*nmemb;
    } // if passthrough variables are defined

	while (httpBuf->nextBuffer!=0) {
		httpBuf = httpBuf->nextBuffer;
	}

	size_t freeBufferMemory = 0;
	freeBufferMemory = httpBuf->bufferSize - httpBuf->bytesUsed;

	if ((size*nmemb) <= freeBufferMemory) {
		memcpy(&httpBuf->buffer[httpBuf->bytesUsed], ptr, size*nmemb);
		httpBuf->bytesUsed += size*nmemb;
	} else {
		memcpy(&httpBuf->buffer[httpBuf->bytesUsed], ptr, freeBufferMemory);
		httpBuf->bytesUsed += freeBufferMemory;

		httpBuf->nextBuffer = new httpBuffer;
		httpBuf->nextBuffer->nextBuffer = NULL;

		httpBuf->nextBuffer->bufferSize = 50000;
		if ((size*nmemb)>(httpBuf->nextBuffer->bufferSize-5000)) {
			httpBuf->nextBuffer->bufferSize += (size*nmemb);
		}

		httpBuf->nextBuffer->bytesUsed = 0;
		httpBuf->nextBuffer->buffer = new byte[httpBuf->nextBuffer->bufferSize];

        if (0==httpBuf->nextBuffer->buffer) {
            // Failed to allocate new chained buffer, remove references to new buffer and return only the amount of bytes cleared
            // that got copied into the original last buffer in the chain.
            delete httpBuf->nextBuffer;
            httpBuf->nextBuffer=NULL;

            // Returns to CURL the number of bytes copied to original buffer, the rest cannot be accepted. Original buffer is now full.
            return freeBufferMemory;
        }

        // All provided data has been copied. The function will return with "all given bytes copied" status.
        memcpy(&httpBuf->nextBuffer->buffer[httpBuf->nextBuffer->bytesUsed],
               (void*) ((unsigned long) ptr+freeBufferMemory), (size*nmemb)-freeBufferMemory);
		httpBuf->nextBuffer->bytesUsed += ((size*nmemb)-freeBufferMemory);
	}

	return size*nmemb;
}

void ServerConnection::parseHeader(std::map<string, string>& pOutelfcloudHeaders, std::map<string, string>& pOutAllHeaders, httpBuffer* pInHeaderBuffer) {

	bool endFound=false;
	unsigned int i=0, startPos=0, endPos=0;

	string line;
	while (i<pInHeaderBuffer->bytesUsed && pInHeaderBuffer->buffer[i]!=0) {
		if (pInHeaderBuffer->buffer[i]=='\r' || pInHeaderBuffer->buffer[i]=='\n') {
			endPos=i;
			endFound=true;
		}

		if (endFound) {
			while (i<pInHeaderBuffer->bytesUsed && (pInHeaderBuffer->buffer[i]=='\r' || pInHeaderBuffer->buffer[i]=='\n') && pInHeaderBuffer->buffer[i]!=0) i++;

			line.assign((const char*) (&pInHeaderBuffer->buffer[startPos]), (size_t) (endPos-startPos));

			startPos=i;
			endPos=0;
			endFound=false;

			size_t colon=line.find(": ", 0);
			if (colon!=string::npos) {
				string name, value;
				name.assign(line.substr(0, colon));
				value.assign(line.substr(colon+2));
				pOutAllHeaders.insert(make_pair(name, value));
			}

			if (line.find("X-ELFCLOUD-", 0)!=string::npos) {
				colon=line.find(": ", 0);
				if (colon!=string::npos) {
					string name, value;
					name.assign(line.substr(0, colon));
					value.assign(line.substr(colon+2));

					pOutelfcloudHeaders.insert(make_pair(name, value));
				}
			}

			continue;
		} else {
			i++;
		}
	}
}

// Free linked list of body buffers, everything is free's except the initial
// httpBuffer struct (however, it's contained buffer is freed) which is usually allocated from stack.
void ServerConnection::deleteBufferChain(httpBuffer *httpBodyBuffer) {
	httpBuffer *temp_ptr;
	while (httpBodyBuffer->nextBuffer != 0) {
		temp_ptr = httpBodyBuffer->nextBuffer;
		httpBodyBuffer->nextBuffer = temp_ptr->nextBuffer;
		delete[] temp_ptr->buffer;
		delete temp_ptr;
	}
	delete[] httpBodyBuffer->buffer;
}

httpBuffer ServerConnection::mergeBufferChain(const httpBuffer& pInBuffer) {

	// Calculate total size of the chained buffers
	unsigned int totalSize = pInBuffer.bytesUsed;
	httpBuffer *buffer=pInBuffer.nextBuffer;
	while(buffer!=0) {
		totalSize += buffer->bytesUsed;
		buffer = buffer->nextBuffer;
	}

	// Allocate large buffer, copy each segment.
    httpBuffer newBuffer;
    //memset(&newBuffer, 0, sizeof(httpBuffer));

    try {
        newBuffer.buffer = new byte[totalSize];
        newBuffer.bufferSize=totalSize;

        memcpy(newBuffer.buffer, pInBuffer.buffer, pInBuffer.bytesUsed);
        newBuffer.bytesUsed = pInBuffer.bytesUsed;
        httpBuffer *bufferLoop = pInBuffer.nextBuffer;
        while(bufferLoop != 0) {
            memcpy(newBuffer.buffer+newBuffer.bytesUsed, bufferLoop->buffer, bufferLoop->bytesUsed);
            newBuffer.bytesUsed += bufferLoop->bytesUsed;
            bufferLoop = bufferLoop->nextBuffer;
        }

        assert(newBuffer.bytesUsed == totalSize);
    }
    catch (bad_alloc& e) {
        throw Exception(ECSCI_EXC_MEMORY_ALLOCATION_ERROR, "Unable to allocate response body buffer");
    }

    return newBuffer;
}

} // ns


