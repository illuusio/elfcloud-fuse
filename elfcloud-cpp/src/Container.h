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

#ifndef ELFCLOUD_CONTAINER_H_
#define ELFCLOUD_CONTAINER_H_

#include "Object.h"
#include <stdint.h>

#ifdef ELFCLOUD_LIB
#include <json/json.h>
#include <json/value.h>
#endif

#include <list>
#include <map>

using namespace std;

namespace elfcloud {

class Client;
class DataItem;
class Cluster;
class Key;

class Container: public Object, public Cacheable {

private:
#ifdef ELFCLOUD_LIB
	Json::Value createRequestAddCluster();
	Json::Value createRequestListClusters();
	Json::Value createRequestListDataItems();
	Json::Value createRequestAddCluster(Cluster* pInCluster);
	Json::Value createRequestUpdateDataItem(DataItem& pInDataItem);
	Json::Value createRequestRemoveDataItem(DataItem& pInDataItem);
	Json::Value createRequestListContents();
    Json::Value createRequestRenameDataItem(const DataItem& pInDataItem, const std::string& pInNewName);
    Json::Value createRequestRelocateDataItem(const DataItem& pInDataItem, const Container& pInNewContainer, const std::string pInNewName);
    static Json::Value createRequestGetContainerById(const uint64_t pInContainerId);

    virtual void initWithDictionary(Json::Value&) = 0;
    bool storeDataItemPassthrough(shared_ptr<elfcloud::DataItem> pInDataItem, const elfcloud::Key *pInContentKey);
    bool fetchDataItemPassthrough(shared_ptr<elfcloud::DataItem> pInDataItem);
#endif

protected:
	Container() { client=0; };
	Client *client;

public:
	Container(Client *pInClient);
	virtual ~Container();

	virtual uint64_t getContainerId() const = 0;
    virtual string getContainerType() const = 0;

    // Store data item with key known by the data item, or default to default key
    bool storeDataItem(shared_ptr<elfcloud::DataItem> pInDataItem);

    // Store with REPLACE mode, use default content key. Throws if default key is not set.
    bool storeDataItemWithDefaultKey(shared_ptr<elfcloud::DataItem> pInDataItem);

    // Store with REPLACE mode. Use given key.
    bool storeDataItem(shared_ptr<elfcloud::DataItem> pInDataItem, const elfcloud::Key *pInContentKey);

    // Fetch data item's contents from the server, data is stored into the data item object
    bool fetchDataItem(shared_ptr<elfcloud::DataItem> pInOutDataItem);

    // Remove data item from the server
    bool removeDataItem(shared_ptr<elfcloud::DataItem> pInDataItem);

    std::list<std::shared_ptr<Cluster>>* listClusters();
    std::list<std::shared_ptr<DataItem>>* listDataItems();
	std::map<std::string, std::list<shared_ptr<elfcloud::Object>>*> *listContents();

    bool addClusterToServer(elfcloud::Cluster *pInCluster);
    bool addClusterToServer(shared_ptr<Cluster> pInCluster);
	bool updateDataItem (shared_ptr<DataItem> pInDataItem);

    void renameDataItem(shared_ptr<elfcloud::DataItem> pInDataItem, const std::string& pInNewName);
    void renameDataItem(elfcloud::DataItem *pInDataItem, const std::string& pInNewName);

    // Return from cache, get from server if not found.
	DllExport static shared_ptr<Container> getContainerById(Client *pInClient, uint64_t pInContainerId);

	//bool storeDataItem(elfcloudDataItem& pInDataItem, ...StoreMode.., ..PatchOffset.., ...CryptMode...);
};

}

#endif /* ELFCLOUD_CONTAINER_H_ */
