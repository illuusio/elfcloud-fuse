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

#ifndef ELFCLOUD_DATAITEM_H_
#define ELFCLOUD_DATAITEM_H_

#include "Object.h"
#include "Types.h"

#include <map>
#include <set>
#include <stdlib.h>
#include <string.h>
#include <string>
#include <stdint.h>

using namespace std;

namespace Json {
    class Value;
}

namespace elfcloud {

class Key;
class KeyHint;
class CryptoHelper;
class Container;
class Client;

class DataItem: public Object, public Cacheable {

private:
	// Whether dataPtr pointed buffer has been allocated and is owned by this DataItem object instance
	bool ownsBuffer;
	byte *dataPtr;
	uint64_t dataLength;
	std::string dataItemName;
	std::string description;
	std::set<std::string> tagSet;

	// ID of the DataItem. While 0, DI is not in the cloud.
	uint64_t id;

	// ID of the DataItem's container. While 0, DI is not in the cloud.
	// Cached objects will never refer to each other on object level.
	uint64_t parentId;
	std::string lastAccessed;
	std::string lastModified;
	std::string dataItemMD5;

    // keyHint is used to store encryption mode, key hash and hash type info received on fetch. If the key
    // is not known to the client, then decryption cannot happen, but store could be done to e.g. another container
    // without decrypting the data at all, by simply preserving the meta data.
    elfcloud::KeyHint *keyHint;

    // CHA is stored here upon fetch. When DI data is altered, the hash is cleared to trigger re-hashing.
	std::string contentHash;
	std::map<std::string, std::string> metaHeaderKVPairs;

	Client *client;

public:
	DataItem(Client *pInClient);
	virtual ~DataItem();

	void assign(std::shared_ptr<Object> pInObject);

    void setDataItemName(const std::string& pInelfcloudDataItemName);

    std::string getDataItemName() const;

    std::map<std::string, std::string>& getMetaHeaderKVPairs() {
		return metaHeaderKVPairs;
	}

    void setMetaHeaderKVPairs(std::map<std::string, std::string>& pInKVPairs) {
		metaHeaderKVPairs = pInKVPairs;
	}

    std::set<std::string> getTagSet() const {
		return tagSet;
	}

	std::shared_ptr<Container> getContainer();

    void setTagSet(std::set<std::string> pInTagSet) {
		this->tagSet = pInTagSet;
	}

    std::string getDescription() const {
		return description;
	}

    void setDescription(std::string description) {
		this->description = description;
	}

    uint64_t getDataLength() const {
		return dataLength;
	}

    byte* getDataPtr() {
		return dataPtr;
	}

	uint64_t getId();
	void setId(uint64_t pInId);

	uint64_t getParentId();
	void setParentId(uint64_t pInParentId);

    // Copy data to a newly allocated buffer. Does not take ownership of parameter buffer.
	void setDataWithCopy(const byte* pInDataPtr, unsigned int pInDataLength) {
		if (dataPtr && ownsBuffer) {
			free(dataPtr);
			ownsBuffer = false;
		}
		dataPtr = (byte*) (((malloc(pInDataLength))));
		if (dataPtr) {
			ownsBuffer = true;
			memcpy(dataPtr, pInDataPtr, pInDataLength);
			dataLength = pInDataLength;
		}

        // Content hash will need to be calculated
        contentHash.clear();
	}

	// Assumes ownership of the passed memory buffer
	void setDataPtr(byte* pInDataPtr, unsigned int pInDataLength);

    bool setDataFromFile(const std::string& pInFilePath);

	void initWithDictionaryDataItems(Json::Value& pInClusterDict);

    void setLastModified(const std::string& pInLastModified) {
		lastModified = pInLastModified;
	}

	void setMD5Sum(const std::string& pInMD5) {
		dataItemMD5 = pInMD5;
	}

    void setDataLength(const uint64_t& pInSize) {
		dataLength = pInSize;
	}

    elfcloud::KeyHint* getKeyHint();
    void setKeyHint(const elfcloud::KeyHint *pInHint);

	void setLastAccessed(const std::string& pInLastAccessed) {
		lastAccessed = pInLastAccessed;
	}

    std::string getLastAccessed() {
		return lastAccessed;
	}

    std::string getLastModified() {
		return lastModified;
	}

    std::string getDataItemMd5Sum() {
		return dataItemMD5;
	}

	std::string getMetaDatav1String();

	void parseMetaDataString(std::string& pInMetaString, const bool pInStrict=false);

    std::string getContentHash() const {
		return contentHash;
	}

    void setContentHash(std::string pInContentHash) {
		this->contentHash = pInContentHash;
	}

	virtual std::string dataMode() { return "in-memory"; }

};

class DataItemFilePassthrough: public DataItem {

private:
	std::string filePath;

public:
	DataItemFilePassthrough(Client *pInClient): DataItem(pInClient) {
	}
	std::string getFilePath() { return filePath; }
	void setFilePath(const string pInFilePath) { filePath.assign(pInFilePath); }

	virtual std::string dataMode() { return "passthrough"; }
};

}
#endif /* ELFCLOUD_DATAITEM_H_ */

