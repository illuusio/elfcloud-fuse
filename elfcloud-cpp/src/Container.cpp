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

#include "Container.h"
#include "DataItem.h"
#include "CryptoHelper.h"
#include "Exception.h"
#include "ServerConnection.h"
#include "KeyHint.h"

#include <map>
#include <string>

#include <sstream>

#include <fstream>
#include <stdlib.h>

using std::string;
using std::map;
using std::make_pair;

namespace elfcloud {

    Container::Container(Client *pInClient): client(pInClient) {
    	client=pInClient;
    }

    Container::~Container() {
    }

    bool Container::fetchDataItem(shared_ptr<elfcloud::DataItem> pInOutDataItem) {

        if (pInOutDataItem->dataMode()=="passthrough") {
            return fetchDataItemPassthrough(pInOutDataItem);
        }

    	map<string, string> mapRequestHeaders;

    	char strParent[30];
    	sprintf(strParent, "%llu", (long long unsigned int) getContainerId());

    	mapRequestHeaders.insert(make_pair(string("X-ELFCLOUD-PARENT"), strParent));

    	string diKey=CryptoHelper::base64Encode((byte*) pInOutDataItem->getDataItemName().c_str(), pInOutDataItem->getDataItemName().length());

    	mapRequestHeaders.insert(make_pair(string("X-ELFCLOUD-KEY"), diKey));

    	byte *requestBody=0;
    	byte *responseBody=0;
    	unsigned int responseBodyLength=0;
    	map<string, string> responseHeaders;
        cout << "going to core request" << endl;
    	client->getServerConnection()->performServerCoreRequest(mapRequestHeaders, requestBody, 0,
    			responseHeaders, &responseBody, &responseBodyLength, ELFCLOUD_INTERFACE_FETCH);
        cout << "done core request" << endl;

    	// Data pointed to by finalData will be ultimately placed inside the DataItem object
    	byte *finalData=responseBody;
    	bool mustFreeResponseBuffer=false;

    	std::map<string, string>::iterator it=responseHeaders.find("X-ELFCLOUD-RESULT");
    	bool fetchSuccessful=false;
    	if (it!=responseHeaders.end()) {
    		string res((*it).second);
    		if (!res.compare("OK"))
    			fetchSuccessful=true;
    	}

    	// VERIFY RESPONSE HASH FROM X-ELFCLOUD-HASH
    	string localHash=CryptoHelper::getHashMD5AsHexString(responseBody, responseBodyLength);

    	if ((it=responseHeaders.find("X-ELFCLOUD-HASH"))!=responseHeaders.end()) {
    		string serverHash=(*it).second;

    		if (localHash.compare(serverHash)) {
    			// Response hash mismatch
                stringstream ss;
                ss << "Fetch X-ELFCLOUD-HASH mismatch, local: " << localHash << ", remote: " << serverHash;
                Client::log(ss.str(), 1);
                delete[] finalData;
    			return false;
    		}
    	}

    	// PARSE META HEADER
    	// Example: X-ELFCLOUD-META: v1:ENC:AES256:KHA:5216ddcc58e8dade5256075e77f642da:CHA:5216ddcc58e8dade5256075e77f642da::

    	if ((it=responseHeaders.find("X-ELFCLOUD-META"))!=responseHeaders.end()) {
    		pInOutDataItem->parseMetaDataString((*it).second);
    	}
    	map<string, string> metaTokens=pInOutDataItem->getMetaHeaderKVPairs();

    	if (metaTokens.count("ENC")) {
    		string encValue=(*metaTokens.find("ENC")).second;

    		if (encValue.compare("NONE")) {

    			if (metaTokens.count("KHA")) {
    				string kha=(*metaTokens.find("KHA")).second;
                    KeyHint kHint(ECSCI_HASHALG_MD5, kha, CryptoHelper::getEncryptionAlgorithm(encValue));
                    pInOutDataItem->setKeyHint(&kHint);
    			} else {
                    cout << "Warning fetch could not find KHA meta, relying on default key decryption" << endl;
                }

    			finalData=new byte[responseBodyLength];
                memset(finalData, 0, responseBodyLength);
    			mustFreeResponseBuffer=true;

                bool res;

                // Key pointers returned by KeyRing are copies and must be free'd
                auto_ptr<elfcloud::Key> key;

                try {
                    // Pointer is owned by DI
                    KeyHint *keyHint=pInOutDataItem->getKeyHint();

                    if (!keyHint) {
                        key.reset(client->getKeyRing()->getDefaultCipherKey());
                    } else {
                        key.reset(client->getKeyRing()->getCipherKey(*keyHint));
                    }
                    res=CryptoHelper::decryptData(key.get(), responseBody, finalData, responseBodyLength);
                }
                catch (Exception &e) {
                    string errMsg="Failed to resolve content key for decryption during fetch data item operation ("+e.getMsg()+")";
                    throw Exception(ECSCI_EXC_KEYMGMT_KEY_NOT_FOUND, errMsg);
                }

                if (!res) {
                    // Decrypt has failed
                    delete[] finalData;
                    delete[] responseBody;
                    throw Exception(ECSCI_EXC_ENCRYPTION_ERROR, "Decryption failed during fetch operation");
                }

    		} // decryption is needed
    	}

    	if (mustFreeResponseBuffer) {
    		delete[] responseBody;
    	}

    	if (metaTokens.count("CHA")) {
    		string localContentHash=CryptoHelper::getHashMD5AsHexString(finalData, responseBodyLength);
    		string serverContentHash=(*metaTokens.find("CHA")).second;

    		if (localContentHash.compare(serverContentHash)) {
    			cout << "Content hash mismatch: local " << localContentHash << ", remote " << serverContentHash << endl;
                cout << "Data (total length " << responseBodyLength << ") starts with " << CryptoHelper::getHashMD5AsHexString(finalData, 40) << endl;

                delete[] finalData;
    			return false;
    		}
    	}

    	pInOutDataItem->setDataPtr(finalData, responseBodyLength);
    	return fetchSuccessful;
    }

    // Private function called by fetchDataItem() when the provided DI is of passthrough type.
    // This will write the fetched and decrypted data directly to a local file pointed to by the PTDI.
    bool Container::fetchDataItemPassthrough(shared_ptr<elfcloud::DataItem> pInOutDataItem) {
        map<string, string> mapRequestHeaders;

        char strParent[30];
        sprintf(strParent, "%llu", (long long unsigned int) getContainerId());

        mapRequestHeaders.insert(make_pair(string("X-ELFCLOUD-PARENT"), strParent));

        string diKey=CryptoHelper::base64Encode((byte*) pInOutDataItem->getDataItemName().c_str(), pInOutDataItem->getDataItemName().length());

        mapRequestHeaders.insert(make_pair(string("X-ELFCLOUD-KEY"), diKey));

        shared_ptr<DataItemFilePassthrough> passthroughDI=std::dynamic_pointer_cast<DataItemFilePassthrough>(pInOutDataItem);

        bool fetchSuccessful=false;
        byte *bufferToReceive=0;

        byte *requestBody=0;
        unsigned int responseBodyLength=0;
        map<string, string> responseHeaders;
        client->getServerConnection()->performServerCoreRequest(mapRequestHeaders, requestBody, 0,
                responseHeaders, &bufferToReceive, &responseBodyLength, ELFCLOUD_INTERFACE_FETCH, passthroughDI);

        if (bufferToReceive) {
            free(bufferToReceive);
        }

        std::map<string, string>::iterator it=responseHeaders.find("X-ELFCLOUD-RESULT");
        fetchSuccessful=false;
        if (it!=responseHeaders.end()) {
            string res((*it).second);
            if (!res.compare("OK"))
                fetchSuccessful=true;
        }

        return fetchSuccessful;
    }

    // Store data item with key known by the data item, or default to default key
    bool Container::storeDataItem(shared_ptr<elfcloud::DataItem> pInDataItem) {
        try {
			KeyHint *keyHint=pInDataItem->getKeyHint();
            return storeDataItem(pInDataItem, client->getKeyRing()->getCipherKey(*keyHint));
		} catch (elfcloud::Exception &e) {
            return storeDataItem(pInDataItem, client->getKeyRing()->getDefaultCipherKey());
        }
    }

    // Store with default content key
    bool Container::storeDataItemWithDefaultKey(shared_ptr<elfcloud::DataItem> pInDataItem) {
        return storeDataItem(pInDataItem, client->getKeyRing()->getDefaultCipherKey());
    }

bool Container::storeDataItemPassthrough(shared_ptr<elfcloud::DataItem> pInDataItem, const elfcloud::Key *pInContentKey) {
	map<string, string> mapRequestHeaders;

	char strParent[50];
	sprintf(strParent, "%llu", (long long unsigned int) getContainerId());
	mapRequestHeaders.insert(make_pair(string("X-ELFCLOUD-PARENT"), strParent));

	string diKey=CryptoHelper::base64Encode((byte*) pInDataItem->getDataItemName().c_str(), pInDataItem->getDataItemName().length());
	mapRequestHeaders.insert(make_pair(string("X-ELFCLOUD-KEY"), diKey));

    shared_ptr<DataItemFilePassthrough> passthroughDI=std::dynamic_pointer_cast<DataItemFilePassthrough>(pInDataItem);

    std::ifstream inputStream(passthroughDI->getFilePath(), std::fstream::in|std::fstream::binary);

    if (inputStream.fail())
        return false;

    // Defaulting in 20MB segments
    unsigned int segmentSize=20*1024*1024;

    unsigned int tmpSegmentSize=strtoul(client->getConf("http.segment.size.bytes").c_str(), 0, 10);
    if (tmpSegmentSize>10240) {
        segmentSize=tmpSegmentSize;
    }

    byte* bufferToRead=new byte[segmentSize];
    byte* bufferToStore=new byte[segmentSize];

    bool result=false;

    int segmentCount=1;

    try {
        CryptoHelper cH;
        cH.encryptDataStreamBegin(pInContentKey);

        while (!inputStream.eof()) {
            inputStream.read((char*) bufferToRead, segmentSize);

            unsigned int bytesRead=inputStream.gcount();

            if (false==cH.encryptDataStreamContinue(bufferToRead, bufferToStore, bytesRead)) {
                Client::log("Container/storeDataItem(): Encryption failed with the given key", 1);
                throw Exception();
            }

            if (1==segmentCount) {
                mapRequestHeaders.erase("X-ELFCLOUD-STORE-MODE");
                mapRequestHeaders.insert(make_pair<string, string>("X-ELFCLOUD-STORE-MODE", "REPLACE"));
            } else {
                mapRequestHeaders.erase("X-ELFCLOUD-STORE-MODE");
                mapRequestHeaders.insert(make_pair<string, string>("X-ELFCLOUD-STORE-MODE", "APPEND"));
            }

            mapRequestHeaders.erase("X-ELFCLOUD-META");
            mapRequestHeaders.insert(make_pair<string, string>("X-ELFCLOUD-META", pInDataItem->getMetaDatav1String()));
            mapRequestHeaders.erase("X-ELFCLOUD-HASH");
            mapRequestHeaders.insert(make_pair<string, string>("X-ELFCLOUD-HASH", CryptoHelper::getHashMD5AsHexString(bufferToStore, bytesRead)));

            byte *responseBody=0;
            unsigned int responseBodyLength=0;
            map<string, string> responseHeaders;
            client->getServerConnection()->performServerCoreRequest(mapRequestHeaders, bufferToStore, bytesRead,
                    responseHeaders, &responseBody, &responseBodyLength, ELFCLOUD_INTERFACE_STORE);

            if (responseBody) {
        		delete[] responseBody;
        		responseBody=0;
        	}

            std::map<string, string>::iterator it=responseHeaders.find("X-ELFCLOUD-RESULT");
            if (it!=responseHeaders.end()) {
                string res((*it).second);
                stringstream ss;
                ss << "ELFCLOUD SERVER STORE RESULT: " << res;
                Client::log(ss.str(), 5);
                if (res.compare("OK"))
                    throw Exception();
                else
                    segmentCount++;
            }
        }

        result=true;

    } catch (...) {
    }

    delete[] bufferToStore;
    delete[] bufferToRead;
    inputStream.close();
    return result;
}

// Store data item, REPLACE mode, use given key.
// pInDataItem might be modified with server provided latest timestamps. Persistence state?
bool Container::storeDataItem(shared_ptr<elfcloud::DataItem> pInDataItem, const elfcloud::Key *pInContentKey) {

    // If explicit key is given, update DI with the key hint. Otherwise, find the key by the DI's keyhint.
    // This needs to be done here, so that meta header construction below will have the correct key info.
    if (pInContentKey) {
        KeyHint tmp=pInContentKey->getHint();
        pInDataItem->setKeyHint(&tmp);
    } else {
        // Will throw if the key is not found, we don't need to store the key here
        client->getKeyRing()->getCipherKey(*pInDataItem->getKeyHint());
    }

	{
		stringstream ss;
		ss << "Container/storeDataItem(): DataItem=" << pInDataItem->getDataItemName() << ", Container=" << (long long unsigned int) getContainerId() << ", Data mode=" << pInDataItem->dataMode();
        Client::log(ss.str(), 2);
	}

    if (pInDataItem->dataMode()=="passthrough") {
        return storeDataItemPassthrough(pInDataItem, pInContentKey);
    }

	map<string, string> mapRequestHeaders;

	char strParent[50];
	sprintf(strParent, "%llu", (long long unsigned int) getContainerId());

	mapRequestHeaders.insert(make_pair(string("X-ELFCLOUD-PARENT"), strParent));

	string diKey=CryptoHelper::base64Encode((byte*) pInDataItem->getDataItemName().c_str(), pInDataItem->getDataItemName().length());
	mapRequestHeaders.insert(make_pair(string("X-ELFCLOUD-KEY"), diKey));

	bool localBufferAllocated=false;
	byte* bufferToStore=pInDataItem->getDataPtr();

    bufferToStore=new byte[pInDataItem->getDataLength()];
    localBufferAllocated=true;

    if (false==CryptoHelper::encryptData(pInContentKey, pInDataItem->getDataPtr(), bufferToStore, pInDataItem->getDataLength())) {
        delete[] bufferToStore;
		Client::log("Container/storeDataItem(): Encryption failed with the given key", 1);
        throw Exception(ECSCI_EXC_ENCRYPTION_ERROR, "Encryption failed with the given key");
    }

	/***
		switch (pInStoreMode) {
			case ELFCLOUD_STORE_MODE_NEW: {
				headers = curl_slist_append(headers, "X-ELFCLOUD-STORE-MODE: NEW");
				break;
			}
			case ELFCLOUD_STORE_MODE_REPLACE: {
				headers = curl_slist_append(headers, "X-ELFCLOUD-STORE-MODE: REPLACE");
				break;
			}
			case ELFCLOUD_STORE_MODE_APPEND: {
				headers = curl_slist_append(headers, "X-ELFCLOUD-STORE-MODE: APPEND");
				break;
			}
			case ELFCLOUD_STORE_MODE_PATCH: {
				headers = curl_slist_append(headers, "X-ELFCLOUD-STORE-MODE: PATCH");

				sprintf(tmpBuffer, "X-ELFCLOUD-OFFSET: %u", pInPatchOffset);
				headers = curl_slist_append(headers, tmpBuffer);

				break;
			}
	 ***/

	// STORE MODE (AND X-ELFCLOUD-OFFSET)
	mapRequestHeaders.insert(make_pair<string, string>("X-ELFCLOUD-STORE-MODE", "REPLACE"));

	// META HEADER
	// TODO: Consider APPEND and PATCH scenarios and how the content hash is calculated, should probably be omitted from the meta headers in this case
	mapRequestHeaders.insert(make_pair<string, string>("X-ELFCLOUD-META", pInDataItem->getMetaDatav1String()));

	// STORE REQUEST HASH
	mapRequestHeaders.insert(make_pair<string, string>("X-ELFCLOUD-HASH",
			CryptoHelper::getHashMD5AsHexString(bufferToStore, pInDataItem->getDataLength())));

	byte *responseBody=0;
	unsigned int responseBodyLength=0;
	map<string, string> responseHeaders;
	client->getServerConnection()->performServerCoreRequest(mapRequestHeaders, bufferToStore, pInDataItem->getDataLength(),
			responseHeaders, &responseBody, &responseBodyLength, ELFCLOUD_INTERFACE_STORE);

	// Release encrypted data buffer if it exists
	if (localBufferAllocated) {
		delete[] bufferToStore;
	}

	if (responseBody) {
		// performServerCoreRequest provides server's http response buffer (merged from chunks) to upstream caller,
		// release responsibility is with us.
		delete[] responseBody;
		responseBody=0;
	}

	bool storeSuccesful=false;
	std::map<string, string>::iterator it=responseHeaders.find("X-ELFCLOUD-RESULT");
	if (it!=responseHeaders.end()) {
		string res((*it).second);
		stringstream ss;
        ss << "ELFCLOUD SERVER STORE RESULT: " << res;
        Client::log(ss.str(), 5);
		if (!res.compare("OK"))
			storeSuccesful=true;
	}

	return storeSuccesful;
}

bool Container::addClusterToServer(Cluster *pInCluster) {

	if (!pInCluster || getContainerId()==0) {
		return false;
	}

	try {
        // We are holding the shared_ptr so it's safe to use raw ptr for sub-function
		Json::Value jsonAddCluster = createRequestAddCluster(pInCluster);
		Json::Value jsonAddClusterResp = client->getServerConnection()->performServerJSONRequest(jsonAddCluster);
        ((Container *) pInCluster)->initWithDictionary(jsonAddClusterResp);
	} catch (Exception& ex) {
		return false;
	}

	return true;
}

bool Container::addClusterToServer(shared_ptr<Cluster> pInCluster) {
    return addClusterToServer(pInCluster.get());
}

Json::Value Container::createRequestAddCluster(Cluster* pInCluster) {
	Json::Value add_cluster(Json::objectValue);
	add_cluster["method"] = Json::Value("add_cluster");
	add_cluster["params"] = Json::Value(Json::objectValue);
	add_cluster["params"]["parent_id"] = Json::Value((Json::Value::UInt64) getContainerId());
	add_cluster["params"]["name"] = Json::Value(pInCluster->getClusterName());

	return add_cluster;

}
Json::Value Container::createRequestListClusters() {
	Json::Value list_clusters(Json::objectValue);
	list_clusters["method"] = Json::Value("list_clusters");
	list_clusters["params"] = Json::Value(Json::objectValue);
	list_clusters["params"]["parent_id"] = Json::Value((Json::Value::UInt64) getContainerId());

	return list_clusters;
}

Json::Value Container::createRequestListDataItems(){
	Json::Value list_dataitems(Json::objectValue);
	list_dataitems["method"] = Json::Value("list_dataitems");
	list_dataitems["params"] = Json::Value(Json::objectValue);
	list_dataitems["params"]["parent_id"] = Json::Value((Json::Value::UInt64) getContainerId());

	return list_dataitems;
}

Json::Value Container::createRequestListContents() {
	Json::Value req(Json::objectValue);
	req["method"] = Json::Value("list_contents");
	req["params"] = Json::Value(Json::objectValue);
	req["params"]["parent_id"] = Json::Value((Json::Value::UInt64) getContainerId());

	return req;
}

std::map<std::string, std::list<shared_ptr<elfcloud::Object>>*> *Container::listContents() {

	std::list<shared_ptr<Cluster>> *listClusters = new std::list<shared_ptr<Cluster>>();
	std::list<shared_ptr<DataItem>> *listDataitems = new std::list<shared_ptr<DataItem>>();

	std::map<std::string, std::list<shared_ptr<elfcloud::Object>>*> *responseMap = new std::map<std::string, std::list<shared_ptr<elfcloud::Object>>*>();
	responseMap->insert(make_pair("clusters", (std::list<shared_ptr<elfcloud::Object>>*) listClusters));
	responseMap->insert(make_pair("dataitems", (std::list<shared_ptr<elfcloud::Object>>*) listDataitems));

	try {
		ServerConnection *serverConn = client->getServerConnection();

		// Create and perform list_contents request
		Json::Value req = createRequestListContents();
		Json::Value pInRespDict = serverConn->performServerJSONRequest(req);

		// Valid result always contains these two dictionary member arrays, even when empty
		if (!pInRespDict.isMember("clusters")) throw Exception();
		if (!pInRespDict.isMember("dataitems")) throw Exception();

		Json::Value clusters = pInRespDict.get("clusters", Json::Value::null);

		Json::Value::iterator iterClusters = clusters.begin();
		while (iterClusters != clusters.end()) {
			shared_ptr<Cluster> cluster(new Cluster(client));
			Json::Value aCluster = *iterClusters;
			std::dynamic_pointer_cast<Container>(cluster)->initWithDictionary(aCluster);
            cluster=std::dynamic_pointer_cast<Cluster>(client->setCacheContainer(cluster));
			listClusters->push_back(cluster);
			iterClusters++;
		}

		Json::Value dataitems = pInRespDict.get("dataitems", Json::Value::null);

		Json::Value::iterator iterDataitems = dataitems.begin();
		while (iterDataitems != dataitems.end()) {
			shared_ptr<DataItem> dataitem(new DataItem(client));
			Json::Value aDataitem = *iterDataitems;
			dataitem->initWithDictionaryDataItems(aDataitem);
            // cast is not needed dataitem=std::dynamic_pointer_cast<DataItem>(client->setCacheDataItem(dataitem));
            dataitem=client->setCacheDataItem(dataitem);
			listDataitems->push_back(dataitem);
			iterDataitems++;
		}

	}
	catch (Exception &ex) {
		throw Exception(ECSCI_EXC_REQUEST_PROCESSING_FAILED, "Processing API call list_contents failed");
	}

	return responseMap;
}

std::list<std::shared_ptr<Cluster>>* Container::listClusters() {

    std::list<shared_ptr<Cluster>> *response=new std::list<shared_ptr<Cluster>>();

    try {
        ServerConnection *serverConn=client->getServerConnection();

        // Create list_vaults request
        Json::Value req = createRequestListClusters();

        // Call server
        Json::Value resp = serverConn->performServerJSONRequest(req);

        // Process response array
        Json::Value clusters = resp;

        Json::Value::iterator iterClusters = clusters.begin();

        while (iterClusters!=clusters.end()) {
            shared_ptr<Cluster> cluster(new Cluster(client));
            Json::Value aCluster = *iterClusters;
            std::dynamic_pointer_cast<Container>(cluster)->initWithDictionary(aCluster);
            cluster=std::dynamic_pointer_cast<Cluster>(client->setCacheContainer(cluster));
            response->push_back(cluster);
            iterClusters++;
        }
    } catch (Exception &ex) {
        throw Exception(ECSCI_EXC_REQUEST_PROCESSING_FAILED, "Cluster listing failed");
    }

    return response;
}

std::list<shared_ptr<DataItem>>* Container::listDataItems() {

    std::list<shared_ptr<DataItem>> *response=new std::list<shared_ptr<DataItem>>();

    try {
        ServerConnection *serverConn=client->getServerConnection();

        // Create list_vaults request
        Json::Value req = createRequestListDataItems();

        // Call server
        Json::Value resp = serverConn->performServerJSONRequest(req);

        // Process response array
        Json::Value dataItems = resp;

        Json::Value::iterator iterDataItems = dataItems.begin();

        while (iterDataItems!=dataItems.end()) {
            shared_ptr<DataItem> dataItem(new DataItem(client));
            Json::Value aDataItem = *iterDataItems;
            dataItem->initWithDictionaryDataItems(aDataItem);
            dataItem=std::dynamic_pointer_cast<DataItem>(client->setCacheDataItem(dataItem));
            response->push_back(dataItem);
            iterDataItems++;
        }
    } catch (Exception &ex) {
        throw Exception(ECSCI_EXC_REQUEST_PROCESSING_FAILED, "Data item listing failed");
    }

    return response;
}

Json::Value Container::createRequestRemoveDataItem(DataItem& pInDataItem) {
	Json::Value remove_dataItem(Json::objectValue);
	remove_dataItem["method"] = Json::Value("remove_dataitem");
	remove_dataItem["params"] = Json::Value(Json::objectValue);
	remove_dataItem["params"]["name"] = Json::Value(pInDataItem.getDataItemName());
	remove_dataItem["params"]["parent_id"] = Json::Value((Json::Value::UInt64) getContainerId());
	return remove_dataItem;
}

bool Container::removeDataItem(shared_ptr<elfcloud::DataItem> pInDataItem) {
	try {
		Json::Value jsonRequest = createRequestRemoveDataItem(*pInDataItem);
		Json::Value jsonResponse = client->getServerConnection()->performServerJSONRequest(jsonRequest);
	} catch (Exception& ex) {
		return false;
	}
	return true;
}

Json::Value Container::createRequestUpdateDataItem(DataItem& pInDataItem) {

	Json::Value update_dataitem(Json::objectValue);
	update_dataitem["method"] = Json::Value("update_dataitem");
	update_dataitem["params"] = Json::Value(Json::objectValue);
	update_dataitem["params"]["parent_id"] = Json::Value((Json::Value::UInt64) getContainerId());
	update_dataitem["params"]["name"] = Json::Value(pInDataItem.getDataItemName());
	update_dataitem["params"]["meta"] = Json::Value(pInDataItem.getMetaDatav1String());

	return update_dataitem;
}

Json::Value Container::createRequestRenameDataItem(const DataItem& pInDataItem, const std::string& pInNewName) {

	Json::Value req(Json::objectValue);
	req["method"] = Json::Value("rename_dataitem");
	req["params"] = Json::Value(Json::objectValue);
	req["params"]["parent_id"] = Json::Value((Json::Value::UInt64) getContainerId());
	req["params"]["name"] = Json::Value(pInDataItem.getDataItemName());
	req["params"]["new_name"] = Json::Value(pInNewName);

	return req;
}

// Move data item to different container within the same vault. Supports DI rename during move to
// facilitate moving to a container that already had a data item of the same name.
Json::Value Container::createRequestRelocateDataItem(const DataItem& pInDataItem, const Container& pInNewContainer, const std::string pInNewName) {

	Json::Value req(Json::objectValue);
	req["method"] = Json::Value("relocate_dataitem");
	req["params"] = Json::Value(Json::objectValue);
	req["params"]["parent_id"] = Json::Value((Json::Value::UInt64) getContainerId());
    req["params"]["new_parent_id"] = Json::Value((Json::Value::UInt64) pInNewContainer.getContainerId());
	req["params"]["name"] = Json::Value(pInDataItem.getDataItemName());

    std::string newName=pInNewName;
    if (!newName.size()) {
        newName=pInDataItem.getDataItemName();
    }

	req["params"]["new_name"] = Json::Value(newName);

	return req;
}

bool Container::updateDataItem(shared_ptr<DataItem> pInDataItem) {
	try {
		Json::Value jsonUpdateDataItem = createRequestUpdateDataItem(*pInDataItem.get());
		Json::Value jsonUpdateDataItemResp = client->getServerConnection()->performServerJSONRequest(jsonUpdateDataItem);
	} catch (Exception& ex) {
		return false;
	}

	return true;
}

// Raw pointer variant is needed because the function is called from within DataItem class itself
// and the self pointer cannot be wrapped into a shared_ptr.
void Container::renameDataItem(elfcloud::DataItem *pInDataItem, const std::string& pInNewName)
{
    try {
        Json::Value jsonReq = createRequestRenameDataItem(*pInDataItem, pInNewName);
        Json::Value jsonResp = client->getServerConnection()->performServerJSONRequest(jsonReq);
    } catch (Exception& ex) {
        throw Exception(ECSCI_EXC_BACKEND_EXCEPTION, "Rename API call failed");
    }
}

void Container::renameDataItem(shared_ptr<elfcloud::DataItem> pInDataItem, const std::string& pInNewName) {
    renameDataItem(pInDataItem.get(), pInNewName);
}

// static
Json::Value Container::createRequestGetContainerById(const uint64_t pInContainerId) {

	Json::Value request(Json::objectValue);
	request["method"] = Json::Value("get_container_by_id");
	request["params"] = Json::Value(Json::objectValue);

	request["params"]["container_ids"] = Json::Value(Json::arrayValue);
	request["params"]["container_ids"].append(Json::Value((Json::Value::UInt64) pInContainerId));

	return request;
}

// static
DllExport shared_ptr<Container> Container::getContainerById(Client *pInClient,
                                                            uint64_t pInContainerId) {

    shared_ptr<Container> cached=pInClient->getCacheContainer(pInContainerId);

    if (cached) {
        if (cached->getContainerType().compare("vault")==0) {
            return std::dynamic_pointer_cast<Vault>(cached);
        } else if (cached->getContainerType().compare("cluster")==0) {
            return std::dynamic_pointer_cast<Cluster>(cached);
        }
        throw Exception();
    }

	ServerConnection *serverConn = pInClient->getServerConnection();

	Json::Value req = Container::createRequestGetContainerById(pInContainerId);
	Json::Value resp = serverConn->performServerJSONRequest(req);

	if (resp.size() != 1) {
		// Queried with single ID, response array must contain only one dictionary
		throw Exception();
	}

	Json::Value::iterator iter = resp.begin(); // iterate response main level array
	Json::Value aItem = *iter;
	if (!aItem.isObject()) {
		// Response seems invalid, dictionary objects expected in array
		throw Exception();
	}

	if (!aItem.isMember("type")) throw Exception();
	std::string cType = ((aItem.get("type", Json::Value::null).asString()));

	if (!cType.compare("vault")) {
		shared_ptr<Vault> v(new Vault(pInClient));
		std::dynamic_pointer_cast<Container>(v)->initWithDictionary(aItem);
        v=std::dynamic_pointer_cast<Vault>(pInClient->setCacheContainer(v));
		return v;
	}
	else if (!cType.compare("cluster")) {
		shared_ptr<Cluster> c(new Cluster(pInClient));
		std::dynamic_pointer_cast<Container>(c)->initWithDictionary(aItem);
        c=std::dynamic_pointer_cast<Cluster>(pInClient->setCacheContainer(c));
		return c;
	}

	// Unrecognized container type, cannot parse
	throw Exception();
}

}
