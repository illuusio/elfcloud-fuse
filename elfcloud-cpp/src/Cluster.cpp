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

#include "Cluster.h"
#include "ServerConnection.h"

using namespace std;

namespace elfcloud {

Cluster::Cluster(Client *pInClient): Container(pInClient) {
	clusterDescendants = 0;
	clusterDataItems = 0;
	parentContainer = 0;
	clusterId = 0;
}

Cluster::~Cluster() {
}

void Cluster::assign(std::shared_ptr<Object> pInObject) {

    shared_ptr<Cluster> c=std::dynamic_pointer_cast<Cluster>(pInObject);

    clusterName.assign(c->clusterName);
    clusterDescendants=c->clusterDescendants;
    clusterParentId=c->clusterParentId;
    clusterDataItems=c->clusterDataItems;
    clusterId=c->clusterId;

    parentContainer=c->parentContainer;

    lastAccessed.assign(c->lastAccessed);
    lastModified.assign(c->lastModified);
    permissions=c->permissions;
    sizeBytes=c->sizeBytes;
}

void Cluster::setClusterID(const uint64_t pInClusterID) {

	if (pInClusterID == 0) {
		throw IllegalParameterException();
	}
	clusterId = pInClusterID;
}

void Cluster::setClusterName(const string& pInClusterName) {
	if (pInClusterName.empty()) {
		throw IllegalParameterException();
	}
	if (clusterId>0) {
		if (!renameCluster(pInClusterName)) {
			throw IllegalParameterException();
		}
	}
	clusterName=pInClusterName;
}

void Cluster::setClusterDescendants(const uint64_t pInClusterDescendants) {
	clusterDescendants = pInClusterDescendants;
}

void Cluster::setClusterDataItems(const uint64_t pInClusterDataItems) {
	clusterDataItems = pInClusterDataItems;
}

void Cluster::setClusterParentId(const uint64_t pInClusterParentId) {
	if(pInClusterParentId==0) {
		throw IllegalParameterException();
	}
	clusterParentId = pInClusterParentId;
}

string Cluster::getClusterName() {
	return clusterName;
}

uint64_t Cluster::getClusterDescendants() {
	return clusterDescendants;
}

uint64_t Cluster::getClusterParentId() {
	return clusterParentId;
}

uint64_t Cluster::getClusterDataItems() {
	return clusterDataItems;
}

uint64_t Cluster::getClusterId() const {
	return clusterId;
}

uint64_t Cluster::getContainerId() const {
	return getClusterId();
}

bool Cluster::addCluster(Container* pInParentContainer) {

	if (getClusterName().empty() || NULL==pInParentContainer) {
		return false;
	}

	bool res=pInParentContainer->addClusterToServer(this);
	if (res) {
		parentContainer=pInParentContainer;
	}

	return res;
}

Json::Value Cluster::createRequestListClusters(const uint64_t pInClusterParentId){
	Json::Value list_clusters(Json::objectValue);
	list_clusters["method"] = Json::Value("list_clusters");
	list_clusters["params"] = Json::Value(Json::objectValue);
	list_clusters["params"]["parent_id"] = Json::Value((Json::Value::UInt64) pInClusterParentId);

	return list_clusters;
}

Json::Value Cluster::createRequestAddCluster(const string& pInClusterName,
		const uint64_t pInClusterParentId) {

	Json::Value add_cluster(Json::objectValue);
	add_cluster["method"] = Json::Value("add_cluster");
	add_cluster["params"] = Json::Value(Json::objectValue);
	add_cluster["params"]["parent_id"] = Json::Value((Json::Value::UInt64) pInClusterParentId);
	add_cluster["params"]["name"] = Json::Value(pInClusterName);

	return add_cluster;
}

Json::Value Cluster::createRequestRenameCluster(const uint64_t pInClusterId, const string& pInClusterName) {
	Json::Value rename_cluster(Json::objectValue);
	rename_cluster["method"] = Json::Value("rename_cluster");
	rename_cluster["params"] = Json::Value(Json::objectValue);
	rename_cluster["params"]["cluster_id"] = Json::Value((Json::Value::UInt64) pInClusterId);
	rename_cluster["params"]["name"] = Json::Value(pInClusterName);
	return rename_cluster;
}

bool Cluster::removeCluster() {

	ServerConnection *serverConn=client->getServerConnection();
	Json::Value jsonRemoveCluster = createRequestRemoveCluster(getClusterId());

		try {
			Json::Value jsonRemoveClusterResp = serverConn->performServerJSONRequest(jsonRemoveCluster);
		} catch (Exception& ex) {
			return false;
		}
		return true;
}

bool Cluster::renameCluster(const string& pInClusterName){
	try {
		Json::Value jsonRenameCluster = createRequestRenameCluster(this->clusterId, pInClusterName);
		Json::Value jsonRenameClusterResp = client->getServerConnection()->performServerJSONRequest(jsonRenameCluster);
		setClusterName(jsonRenameClusterResp.get("name", Json::Value::null).asString());
	} catch (Exception& ex) {
		return false;
	}
		return true;
}

Json::Value Cluster::createRequestRemoveCluster(const uint64_t pInClusterId) {

	Json::Value remove_cluster(Json::objectValue);
	remove_cluster["method"] = Json::Value("remove_cluster");
	remove_cluster["params"] = Json::Value(Json::objectValue);
	remove_cluster["params"]["cluster_id"] = Json::Value((Json::Value::UInt64) pInClusterId);

	return remove_cluster;
}

void Cluster::initWithDictionary(Json::Value& pInClusterDict) {

	if (!pInClusterDict.isObject()) throw Exception();

	if (!pInClusterDict.isMember("type")) throw Exception();
	if (pInClusterDict.get("type", Json::Value::null).asString().compare("cluster")) throw Exception();

	// Set name before id in order not to trigger server side cluster rename call
	if (!pInClusterDict.isMember("name")) throw Exception();
	setClusterName(pInClusterDict.get("name", Json::Value::null).asString());

	if (!pInClusterDict.isMember("id")) throw Exception();
	setClusterID(pInClusterDict.get("id", Json::Value::null).asUInt64());

	if (!pInClusterDict.isMember("dataitems")) throw Exception();
	setClusterDataItems(pInClusterDict.get("dataitems", Json::Value::null).asUInt64());

	if (!pInClusterDict.isMember("descendants")) throw Exception();
	setClusterDescendants(pInClusterDict.get("descendants", Json::Value::null).asUInt64());

	if (!pInClusterDict.isMember("size")) throw Exception();
	setSizeBytes(pInClusterDict.get("size", Json::Value::null).asUInt64());

	if (!pInClusterDict.isMember("last_accessed_date")) throw Exception();
	setLastAccessed((pInClusterDict.get("last_accessed_date", Json::Value::null).asString()));

	if (!pInClusterDict.isMember("modified_date")) throw Exception();
	setLastModified((pInClusterDict.get("modified_date", Json::Value::null).asString()));

	if (!pInClusterDict.isMember("permissions")) throw Exception();

	Json::Value perms=pInClusterDict.get("permissions", Json::Value::null);

	if (!perms.isArray()) throw Exception();

	Json::Value::iterator iterPerms = perms.begin();

	permissions.clear();
	while (iterPerms!=perms.end()) {
		Json::Value aPerm = *iterPerms;
		permissions.push_back(aPerm.asString());
		iterPerms++;
	}

	return;
}

string Cluster::getContainerType() const {
    return "cluster";
}

}
