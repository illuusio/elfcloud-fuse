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

#ifndef ELFCLOUD_CLUSTER_H_
#define ELFCLOUD_CLUSTER_H_

#include "Object.h"
#include "Exception.h"
#include "IllegalParameterException.h"
#include "Container.h"
#include "Client.h"
//#include "ServerConnection.h"

#ifdef ELFCLOUD_LIB
#include <json/json.h>
#include <json/value.h>
#include <curl/curl.h>
#endif

#include <string.h>
#include <list>
#include <stdlib.h>

using namespace std;

namespace elfcloud {

class Cluster: public elfcloud::Container {
private:
	Cluster();
	string clusterName;
	uint64_t clusterDescendants;
	uint64_t clusterParentId;
	uint64_t clusterDataItems;
	uint64_t clusterId;

	Container *parentContainer;

	string lastAccessed;
	string lastModified;
	vector<string> permissions;
	uint64_t sizeBytes;

#ifdef ELFCLOUD_LIB
    void initWithDictionary(Json::Value&);
#endif

public:
	Cluster(Client* pInClient);
	virtual ~Cluster();

	void assign(std::shared_ptr<Object> pInObject);

	void setClusterID(const uint64_t pInClusterID);
	void setClusterName(const string& pInClusterName);
	void setClusterDescendants(const uint64_t pInClusterDescendants);
	void setClusterDataItems(const uint64_t pInClusterKeyValues);
	void setClusterParentId(const uint64_t pInClusterParentId);

	string getClusterName();
	uint64_t getClusterDescendants();
	uint64_t getClusterParentId();
	uint64_t getClusterDataItems();
	uint64_t getClusterId() const;
	uint64_t getContainerId() const;
	string getContainerType() const;

#ifdef ELFCLOUD_LIB
	static Json::Value createRequestListClusters(const uint64_t pInClusterParentId);
    Json::Value createRequestAddCluster(const string& pInClusterName, const uint64_t pInClusterParentId);
	Json::Value createRequestRemoveCluster(const uint64_t pInClusterId);
	Json::Value createRequestRenameCluster(const uint64_t pInClusterId, const string& pInClusterName);

#endif
	bool addCluster(Container* pInParentContainer);
	bool removeCluster();
	bool renameCluster(const string& pInClusterName);

	string getLastAccessed() const {
		return lastAccessed;
	}

	void setLastAccessed(string lastAccessed) {
		this->lastAccessed = lastAccessed;
	}

	string getLastModified() const {
		return lastModified;
	}

	void setLastModified(string lastModified) {
		this->lastModified = lastModified;
	}

	vector<string> getPermissions() const {
		return permissions;
	}

	void setPermissions(vector<string> permissions) {
		this->permissions = permissions;
	}

    uint64_t getSizeBytes() const {
		return sizeBytes;
	}

	void setSizeBytes(uint64_t sizeBytes) {
		this->sizeBytes = sizeBytes;
	}
};
}
#endif

