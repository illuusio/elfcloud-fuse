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

#include "DataItem.h"
#include "CryptoHelper.h"

#include <iostream>
#include <fstream>
#include <sstream>

#include <json/value.h>

namespace elfcloud {

    DataItem::DataItem(Client *pInClient):client(pInClient) {
    	dataPtr=0;
    	ownsBuffer=false;
    	dataLength=0;
    	keyHint=0;
        parentId=0;
        id=0;
    }

    DataItem::~DataItem() {
    	if (dataPtr && ownsBuffer) {
    		free(dataPtr);
    		dataPtr=0;
    		ownsBuffer=false;
    	}

        if (keyHint) {
            delete keyHint;
        }
    }

    void DataItem::assign(std::shared_ptr<Object> pInObject) {
        // Convert base class pointer to specific type shared_ptr and copy all values.
        // assign() is used when updating existing cached object copy with new object data, through updates
        // the other pointer holders can receive immediate updates (considering concurrency) without any need
        // for updating their pointers or objects.
        shared_ptr<DataItem> d=std::dynamic_pointer_cast<DataItem>(pInObject);
        ownsBuffer=d->ownsBuffer;
        dataPtr=d->dataPtr; // logic!
        dataLength=d->dataLength;
        dataItemName=d->dataItemName;
        description.assign(d->description);
        tagSet=d->tagSet;

        id=d->id;
        parentId=d->parentId;
        lastAccessed.assign(d->lastAccessed);
        lastModified.assign(d->lastModified);
        dataItemMD5.assign(d->dataItemMD5);

        keyHint=d->keyHint; // check the logic here, if d is deleted, should we clone the hint

        contentHash.assign(d->contentHash);
        metaHeaderKVPairs=d->metaHeaderKVPairs;

    }

    // Assumes ownership of the passed memory buffer
    void DataItem::setDataPtr(byte* pInDataPtr, unsigned int pInDataLength) {
    	if (dataPtr && ownsBuffer) {
    		{
    			stringstream ss;
    			ss << "DataItem/setDataPtr(): Freeing data buffer, " << dataLength << " bytes.";
    			Client::log(ss.str(), 7);
    		}
    		free(dataPtr);
    		dataPtr = 0;
    		ownsBuffer = false;
    	}
    	ownsBuffer = true;
    	dataPtr = pInDataPtr;
    	dataLength = pInDataLength;

    	{
    		stringstream ss;
    		ss << "DataItem/setDataPtr(): Assumed ownership of " << dataLength << " bytes long buffer.";
    		Client::log(ss.str(), 6);
    	}

        // Content hash will need to be calculated
        contentHash.clear();
    }

    shared_ptr<Container> DataItem::getContainer() {
        shared_ptr<Container> c;

        if (!parentId || !client) {
            return c;
        }

        c=Container::getContainerById(client, parentId);
        return c;
    }

    void DataItem::setDataItemName(const string& pInDataItemName) {
        if (pInDataItemName.empty()) {
            throw IllegalParameterException();
        }

        if (pInDataItemName.compare(getDataItemName())) {
            // Name is different than current name
            if (getId()) {
                shared_ptr<Container> c = getContainer(); 
                if (!c) {
                    throw IllegalParameterException(ECSCI_EXC_INIT_OR_PARAM_FAILURE, "DataItem not aware of current container");
                }

                // Throws if server operation fails
                c->renameDataItem(this, pInDataItemName); 
            }

            // Update local DI name
            dataItemName.assign(pInDataItemName);
        }
    }

    string DataItem::getDataItemName() const {
        return dataItemName;
    }

    bool DataItem::setDataFromFile(const string& pInFilePath) {
    		FILE *fileHandle;

			stringstream ss;
			ss << "DataItem/setDataFromFile(): " << pInFilePath;
			Client::log(ss.str());

    		fileHandle = fopen(pInFilePath.c_str(), "rb");
    		if (fileHandle == NULL) {
				Client::log("DataItem/setDataFromFile(): Unable to open");
    			return false;
    		}

    		fseek(fileHandle, 0L, SEEK_END);
    		unsigned int fileLength=ftell(fileHandle);
    		fclose(fileHandle);

			{
				stringstream ss;
				ss << "DataItem/setDataFromFile(): Going to read " << fileLength << " bytes";
				Client::log(ss.str());
			}

    		byte *dataBuffer=NULL;
			
			try {
				dataBuffer=(byte*) malloc(fileLength);
			} catch (...) {
				dataBuffer=0;
			}

			if (!dataBuffer) {
				// Switched back to malloc, it doesn't throw really
				stringstream ss;
				ss << "DataItem/setDataFromFile(): Memory alloc failed for " << fileLength << " bytes";
				Client::log(ss.str());
				return false;
			} else {
				{
					stringstream ss;
					ss << "DataItem/setDataFromFile(): Allocated " << fileLength << " bytes long buffer.";
					Client::log(ss.str());
				}
			}

    		ifstream fin(pInFilePath.c_str(), ios_base::in | ios_base::binary);
    		if (fin.good()) {
    			fin.read((char*) dataBuffer, fileLength);
    			fin.close();
    		} else {
				Client::log("DataItem/setDataFromFile(): ifstream bad");
				free(dataBuffer);
				return false;
			}

    		setDataPtr(dataBuffer, fileLength);
			Client::log("DataItem/setDataFromFile(): Read ok");
			return true;
    }

    // Returns pointer to KeyHint object owned by the DataItem object
    elfcloud::KeyHint* DataItem::getKeyHint() {
        if (!keyHint) {
            throw elfcloud::Exception(ECSCI_EXC_INIT_OR_PARAM_FAILURE, "KeyHint object does not exist");
        }

        return keyHint;
    }

    void DataItem::setKeyHint(const elfcloud::KeyHint *pInHint) {
        if (keyHint) {
            delete keyHint;
        }
        keyHint=new KeyHint(*pInHint);
    }

	uint64_t DataItem::getId() {
		return this->id;
	}

	void DataItem::setId(uint64_t pInId) {
		this->id = pInId;
	}

	uint64_t DataItem::getParentId() {
		return this->parentId;
	}

	void DataItem::setParentId(uint64_t pInParentId) {
		this->parentId = pInParentId;
	}

    void DataItem::initWithDictionaryDataItems(Json::Value& pInDataItemDict) {

    	if (!pInDataItemDict.isObject()) throw Exception();

    	if (!pInDataItemDict.isMember("parent_id")) throw Exception();
    	setParentId((pInDataItemDict.get("parent_id", Json::Value::null).asUInt64()));

        // When ID is not set, does not trigger rename API call
        if (!pInDataItemDict.isMember("name")) throw Exception();
        setDataItemName(pInDataItemDict.get("name", Json::Value::null).asString());

    	if (!pInDataItemDict.isMember("dataitem_id")) throw Exception();
    	setId((pInDataItemDict.get("dataitem_id", Json::Value::null).asUInt64()));

    	if (!pInDataItemDict.isMember("modified_date")) throw Exception();
    	setLastModified((pInDataItemDict.get("modified_date", Json::Value::null).asString()));

    	if (!pInDataItemDict.isMember("md5sum")) throw Exception();
    	setMD5Sum((pInDataItemDict.get("md5sum", Json::Value::null).asString()));

    	if (!pInDataItemDict.isMember("last_accessed_date")) throw Exception();
    	setLastAccessed(pInDataItemDict.get("last_accessed_date", Json::Value::null).asString());

    	if (!pInDataItemDict.isMember("meta")) throw Exception();
    	string tmp=pInDataItemDict.get("meta", Json::Value::null).asString();

    	try {
    		parseMetaDataString(tmp);
    	} catch (Exception &ex) {
            stringstream ss;
            ss << "Discarding data item meta data, parsing failed for string " << tmp << endl;
            Client::log(ss.str(), 1);
    		metaHeaderKVPairs.clear();
    	}

    	if (!pInDataItemDict.isMember("size")) throw Exception();
    	setDataLength(pInDataItemDict.get("size", Json::Value::null).asUInt64());

    	// TODO: Support for dataitem modify-locks
    	// TODO: Support for dataitem embedded_data

    	return;
    }

    void DataItem::parseMetaDataString(string &pInMetaString, const bool pInStrict) {
        unsigned int metaBufSize=40960;

        if ((pInMetaString.size()-1)>metaBufSize) {
            stringstream ss;
            ss << "Excessive input length while meta parsing: " << pInMetaString << endl;
            Client::log(ss.str(), 1);
        }

    	char metaBuf[metaBufSize];
    	memset(metaBuf, 0, metaBufSize);
    	memcpy(metaBuf, pInMetaString.c_str(), pInMetaString.size());
    	int metaLength=strlen(metaBuf);

    	metaHeaderKVPairs.clear();

    	int i=0, elementStartPos=0;
    	string metaKey, metaValue;
    	bool escaping=false, foundTerminationMarker=false;

    	if (metaLength<4 || metaBuf[i]!='v' || metaBuf[i+1]!='1' || metaBuf[i+2]!=':') {
    		throw Exception();
    	}

    	i=elementStartPos=3;

    	while (metaBuf[i]!=0) {

#if 0 // description is being removed from the HTTP META header to better support extended charsets.
            unsigned char a=metaBuf[i];
    		if (!((a>='a' && a<='z') || (a>='A' && a<='Z') || (a>='0' && a<='9') || (a==':' || a=='\\' || a=='ä' || a=='ö' || a=='å' || a=='Ä' || a=='Ö' || a=='Å' || a=='_' || a=='-' || a==' ' || a=='.' || a=='?' || a=='!' || a=='"'))) {
    			cout << "Invalid character" << endl;
    			throw Exception();
    		}
#endif

    		if (escaping) {
    			i++;
    			escaping=false;
    			continue;
    		}

    		if (!escaping && metaBuf[i]=='\\') {
    			escaping=true;
    			i++;
    			continue;
    		}

    		if (metaBuf[i]==':') {
    			metaBuf[i]=0;

    			string tmp(&metaBuf[elementStartPos]);
    			elementStartPos=i+1;
    			if (metaKey.empty()) {
    				if (tmp.empty()) {
    					// Termination marker found
    					foundTerminationMarker=true;
    					i++;
    					break;
    				}
    				//cout << "KEY: " << tmp << endl;
    				metaKey.assign(tmp);
    			} else {
    				metaValue.assign(tmp);
    				//cout << "VALUE: " << tmp << endl;

    				if (metaHeaderKVPairs.find(metaKey)!=metaHeaderKVPairs.end()) {
    					// Duplicate key
                        stringstream ss;
                        ss << "Duplicate key encountered while meta parsing: " << pInMetaString << endl;
                        Client::log(ss.str(), 1);
    					if (pInStrict)
    						throw Exception();
    				} else {
    					metaHeaderKVPairs.insert(make_pair(metaKey, metaValue));
    				}
    				metaKey.clear();
    				metaValue.clear();
    			}
    		}

    		i++;
    	} // process meta hdr buffer till zero byte is found

    	if (pInStrict && !foundTerminationMarker) {
            stringstream ss;
            ss << "Term marker missing: " << pInMetaString << endl;
            Client::log(ss.str(), 1);
    		throw Exception();
    	}

    	if (!metaKey.empty() && metaValue.empty()) {
    		// Orphaned key found without value element
            stringstream ss;
            ss << "Orphaned key: " << pInMetaString << endl;
            Client::log(ss.str(), 1);
            if (pInStrict)
    			throw Exception();
    	}

    	if (i<metaLength) {
    		// Extra trailing characters found
            stringstream ss;
            ss << "Extra trailing characters found: " << pInMetaString << endl;
            Client::log(ss.str(), 1);
    		if (pInStrict)
    			throw Exception();
    	}

    	if (metaHeaderKVPairs.count("ENC") && metaHeaderKVPairs.count("KHA")) {
            // v1 meta header always uses MD5 KeyHash (KHA)
            KeyHint tHint(ECSCI_HASHALG_MD5, (*metaHeaderKVPairs.find("KHA")).second, CryptoHelper::getEncryptionAlgorithm((*metaHeaderKVPairs.find("ENC")).second));
            setKeyHint(&tHint);
    	}

    	if (metaHeaderKVPairs.count("DSC")) {
    		setDescription((*metaHeaderKVPairs.find("DSC")).second);
    	}

    	if (metaHeaderKVPairs.count("CHA")) {
    		setContentHash((*metaHeaderKVPairs.find("CHA")).second);
    	}

    	if (metaHeaderKVPairs.count("TGS")) {
    		string tagStr=(*metaHeaderKVPairs.find("TGS")).second;

    		memset(metaBuf, 0, 10240);
    		memcpy(metaBuf, tagStr.c_str(), tagStr.size());

    		char *tok=strtok(metaBuf, ",");

    		tagSet.clear();
    		tok=strtok(NULL, ",");
    		while (tok) {
    			//cout << "Parsed token: " << tok << endl;
    			tagSet.insert(string(tok));
    			tok=strtok(NULL, ",");
    		}
    	}
    }

    string DataItem::getMetaDatav1String() {

        string metaDataString;
    	metaDataString.append("v1:ENC:");
        if (keyHint) {
            metaDataString.append(keyHint->getCipherTypeString());
        } else {
            metaDataString.append("NONE");
        }
        metaDataString.append(":");

    	if (keyHint) {
            metaDataString.append("KHA:");
            metaDataString.append(keyHint->getKeyHash());
            metaDataString.append(":");
        }

    	if (getDescription().empty()) {
    		metaDataString.append("DSC:");
    		metaDataString.append(getDescription());
    		metaDataString.append(":");
    	}

    	if (!getTagSet().empty()) {
    		metaDataString.append("TGS:");
    		set<string> currentTags=getTagSet();
    		set<string>::iterator iter=currentTags.begin();
    		bool tagsAdded=false;
    		while (iter!=currentTags.end()) {
    			string tag=*iter;

    			if (tagsAdded) {
    				metaDataString.append(",");
    			}
    			metaDataString.append(tag);

    			iter++;
    			tagsAdded=true;
    		}
    		metaDataString.append(":");
    	}

        if (dataMode()=="in-memory") {

            // Hash calculation is needed when previous content hash is empty
            if (contentHash.empty()) {
                contentHash.assign(CryptoHelper::getHashMD5AsHexString(getDataPtr(), getDataLength()));
            }
            metaDataString.append("CHA:");
            metaDataString.append(contentHash);
            metaDataString.append(":");
        }

    	// Termination marker, results in two back to back colons = empty key.
    	metaDataString.append(":");

    	return metaDataString;
    }

}
