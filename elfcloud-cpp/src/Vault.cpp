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

#include "Vault.h"
#include "Cluster.h"
#include "Client.h"
#include "ServerConnection.h"
#include "Container.h"

#include <memory>

namespace elfcloud {

Vault::Vault(Client *pInClient, const string pInName, const string pInType): Container(pInClient) {
    Client::log("Vault()", 9);

    vaultName=pInName;
    vaultType=pInType;

    if (!serverAddVault()) {
        throw IllegalParameterException();
    }
}

Vault::Vault(Client *pInClient): Container(pInClient) {
    Client::log("Vault()", 9);
}

Vault::~Vault() {
    Client::log("~Vault()", 9);
}

void Vault::assign(std::shared_ptr<Object> pInObject) {
    // Convert base class pointer to specific type shared_ptr and copy all values.
    // assign() is used when updating existing cached object copy with new object data, through updates
    // the other pointer holders can receive immediate updates (considering concurrency) without any need
    // for updating their pointers or objects.
    shared_ptr<Vault> v=std::dynamic_pointer_cast<Vault>(pInObject);
    vaultType=v->vaultType;
    vaultName=v->vaultName;
    vaultId=v->vaultId;
    vaultDataItems=v->vaultDataItems;
    vaultDescendants=v->vaultDescendants;
    lastAccessed=v->lastAccessed;
    lastModified=v->lastModified;
    permissions=v->permissions;
    sizeBytes=v->sizeBytes;
}

void Vault::setType(const string& pInType) {

	if (pInType.empty()) {
		throw IllegalParameterException();
	}
	vaultType = pInType;
}

void Vault::setName(const string& pInVaultName) {

	if (pInVaultName.empty()) {
		throw IllegalParameterException();
	}

    if (vaultName.compare(pInVaultName)) {
        if (serverRenameVault(pInVaultName)) {
            vaultName = pInVaultName;
        } else
            throw Exception();
    }
}

void Vault::setId(const uint64_t pInVaultId) {
	vaultId = pInVaultId;
}

void Vault::setDataItemCount(const uint64_t pInVaultDataItems) {
	vaultDataItems = pInVaultDataItems;
}

string Vault::getType() {
	return vaultType;
}

uint64_t Vault::getDescendantCount() {
	return vaultDescendants;
}

string Vault::getName() {
	return vaultName;
}

uint64_t Vault::getId() const {
	return vaultId;
}

uint64_t Vault::getContainerId() const {
	return getId();
}

Json::Value Vault::createRequestAddVault() {

	Json::Value add_vault(Json::objectValue);
	add_vault["method"] = Json::Value("add_vault");
	add_vault["params"] = Json::Value(Json::objectValue);
	add_vault["params"]["vault_type"] = Json::Value(getType());
	add_vault["params"]["name"] = Json::Value(getName());
	return add_vault;
}

Json::Value Vault::createRequestListVault(const string& pInVaultType,
                                          const string& pInVaultRole,
                                          const uint64_t pInVaultId,
                                          const string pInGrouped) {

	Json::Value list_vaults(Json::objectValue);
	list_vaults["method"] = Json::Value("list_vaults");
	list_vaults["params"] = Json::Value(Json::objectValue);
    if (!pInGrouped.empty()) {
        list_vaults["params"]["grouped"] = Json::Value(true);
    }
    if (!pInVaultType.empty())
        list_vaults["params"]["vault_type"] = Json::Value(pInVaultType);
    if (!pInVaultRole.empty())
        list_vaults["params"]["role"] = Json::Value(pInVaultRole);
    if (pInVaultId)
        list_vaults["params"]["id_"] = Json::Value((Json::Value::UInt64) pInVaultId);

	return list_vaults;
}

std::map<std::string, std::list<std::shared_ptr<Vault>>*>* Vault::ListVaultsGrouped(Client* pInClient,
                                const string pInVaultType,
                                const string pInVaultRole,
                                const uint64_t pInVaultId) {

    ServerConnection *serverConn=pInClient->getServerConnection();

    Json::Value req = Vault::createRequestListVault(pInVaultType, pInVaultRole, pInVaultId, "true");
    Json::Value groupList = serverConn->performServerJSONRequest(req);

    Json::Value listOwn = groupList.get("own", Json::Value::null);
    Json::Value listAccount = groupList.get("account", Json::Value::null);
    Json::Value listOther = groupList.get("other", Json::Value::null);

    map<string, list<std::shared_ptr<Vault>>*> *result=new map<string, list<std::shared_ptr<Vault>>*>;

    Json::Value::iterator iterVaults = listOwn.begin();
    list<shared_ptr<Vault>> *resultList=new list<shared_ptr<Vault>>;
    result->insert(make_pair(string("own"), resultList));
    while (iterVaults!=listOwn.end()) {
        shared_ptr<Vault> vault(new Vault(pInClient));
        Json::Value aVault = *iterVaults;
        vault->initWithDictionary(aVault);
        vault=std::dynamic_pointer_cast<Vault>(pInClient->setCacheContainer(vault));
        resultList->push_back(vault);
        iterVaults++;
    }

    iterVaults = listAccount.begin();
    resultList=new list<shared_ptr<Vault>>;
    result->insert(make_pair(string("account"), resultList));
    while (iterVaults!=listAccount.end()) {
        shared_ptr<Vault> vault(new Vault(pInClient));
        Json::Value aVault = *iterVaults;
        vault->initWithDictionary(aVault);
        vault=std::dynamic_pointer_cast<Vault>(pInClient->setCacheContainer(vault));
        resultList->push_back(vault);
        iterVaults++;
    }

    iterVaults = listOther.begin();
    resultList=new list<shared_ptr<Vault>>;
    result->insert(make_pair(string("other"), resultList));
    while (iterVaults!=listOther.end()) {
        shared_ptr<Vault> vault(new Vault(pInClient));
        Json::Value aVault = *iterVaults;
        vault->initWithDictionary(aVault);
        vault=std::dynamic_pointer_cast<Vault>(pInClient->setCacheContainer(vault));
        resultList->push_back(vault);
        iterVaults++;
    }

    return result;
}

shared_ptr<Vault> Vault::getVault(list<shared_ptr<Vault>> *pInVaultList, std::string pInVaultName) {

    list<shared_ptr<Vault>>::iterator iter=pInVaultList->begin();

    while (iter!=pInVaultList->end()) {
        std::cout << (*iter)->getName() << " vs. " << pInVaultName << endl;
        if (!(*iter)->getName().compare(pInVaultName)) {
            return *iter;
        }
        iter++;
    }

    return shared_ptr<Vault>();
}

std::list<std::shared_ptr<Vault>> *Vault::ListVaults(Client *pInClient,
                                const string pInVaultType,
                                const string pInVaultRole,
                                const uint64_t pInVaultId) {

	ServerConnection *serverConn=pInClient->getServerConnection();

	Json::Value req = Vault::createRequestListVault(pInVaultType, pInVaultRole, pInVaultId, "");
	Json::Value vaults = serverConn->performServerJSONRequest(req);

	Json::Value::iterator iterVaults = vaults.begin();

    std::list<std::shared_ptr<Vault>> *result=new std::list<std::shared_ptr<Vault>>;
	while (iterVaults!=vaults.end()) {
		std::shared_ptr<Vault> vault(new Vault(pInClient));
		Json::Value aVault = *iterVaults;
		vault->initWithDictionary(aVault);

        // setCacheContainer returns either 'vault' back or pointer to the cached object that was updated
        // In the cache-hit case, the vault instance created here is immediately released.
        // Cast is safe because if different pointer is returned, it must be a vault due to ID match.
        vault=std::dynamic_pointer_cast<Vault>(pInClient->setCacheContainer(vault));
		result->push_back(vault);
		iterVaults++;
	}

	return result;
}

void Vault::FreeVaultSet(list<std::shared_ptr<Vault>> *pInVaultList) {
    if (!pInVaultList) {
        return;
    }

    pInVaultList->clear();
    delete pInVaultList;
}

void Vault::FreeVaultSet(std::map<std::string, std::list<shared_ptr<Vault>> *> *pInVaultMap) {
    if (!pInVaultMap) {
        return;
    }

    for (std::map<std::string, std::list<shared_ptr<Vault>>*>::iterator it=pInVaultMap->begin(); it!=pInVaultMap->end(); it++) {
        FreeVaultSet((*it).second);
    }

    delete pInVaultMap;
}

// Name and vault type must be set in the Vault instance.
// On successful return (true), Vault id has been populated in the object.
bool Vault::serverAddVault() {

	if (getName().empty() || getType().empty()) {
		return false;
	}

	try {
		Json::Value jsonAddVault = createRequestAddVault();
		Json::Value jsonAddVaultResp = client->getServerConnection()->performServerJSONRequest(jsonAddVault);
        initWithDictionary(jsonAddVaultResp);
	} catch (Exception& ex) {
		return false;
	}

	return true;
}

Json::Value Vault::createRequestRemoveVault() {
	Json::Value remove_vault(Json::objectValue);
	remove_vault["method"] = Json::Value("remove_vault");
	remove_vault["params"] = Json::Value(Json::objectValue);
	remove_vault["params"]["vault_id"] = Json::Value((Json::Value::UInt64) getId());
	return remove_vault;
}

Json::Value Vault::createRequestRenameVault(string newVaultName) {
	Json::Value rename_vault(Json::objectValue);
	rename_vault["method"] = Json::Value("rename_vault");
	rename_vault["params"] = Json::Value(Json::objectValue);
	rename_vault["params"]["vault_name"] = Json::Value(newVaultName);
	rename_vault["params"]["vault_id"] = Json::Value((Json::Value::UInt64) getId());
	return rename_vault;
}

bool Vault::serverRemoveVault() {
	ServerConnection *serverConn=client->getServerConnection();
	Json::Value jsonRemoveVault = createRequestRemoveVault();

	try {
		Json::Value jsonRemoveVaultResp = serverConn->performServerJSONRequest(jsonRemoveVault);
	} catch (Exception& ex) {
		return false;
	}
	return true;
}

void Vault::remove() {
    if (!serverRemoveVault())
        throw Exception();
}

bool Vault::serverRenameVault(string newVaultName) {
	try {
		Json::Value jsonRenameVault = createRequestRenameVault(newVaultName);
		Json::Value jsonRenameVaultResp = client->getServerConnection()->performServerJSONRequest(jsonRenameVault);
	} catch (Exception& ex) {
		return false;
	}

	return true;
}

void Vault::initWithDictionary(Json::Value& pInVaultDict) {
	if (!pInVaultDict.isMember("type")) throw Exception();
	if (pInVaultDict.get("type", Json::Value::null).asString().compare("vault")) throw Exception();

	if (!pInVaultDict.isMember("id")) throw Exception();
	setId(pInVaultDict.get("id", Json::Value::null).asUInt64());

	if (!pInVaultDict.isMember("dataitems")) throw Exception();
	setDataItemCount(pInVaultDict.get("dataitems", Json::Value::null).asUInt64());

	if (!pInVaultDict.isMember("descendants")) throw Exception();
	setDescendantCount(pInVaultDict.get("descendants", Json::Value::null).asUInt64());

	if (!pInVaultDict.isMember("name")) throw Exception();
	vaultName.assign(pInVaultDict.get("name", Json::Value::null).asString());

	if (!pInVaultDict.isMember("vault_type")) throw Exception();
	setType(pInVaultDict.get("vault_type", Json::Value::null).asString());

	if (!pInVaultDict.isMember("size")) throw Exception();
	setSizeBytes(pInVaultDict.get("size", Json::Value::null).asUInt64());

	if (!pInVaultDict.isMember("last_accessed_date")) throw Exception();
	setLastAccessed((pInVaultDict.get("last_accessed_date", Json::Value::null).asString()));

	if (!pInVaultDict.isMember("modified_date")) throw Exception();
	setLastModified((pInVaultDict.get("modified_date", Json::Value::null).asString()));

	if (!pInVaultDict.isMember("permissions")) throw Exception();

	Json::Value perms=pInVaultDict.get("permissions", Json::Value::null);

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

void Vault::setDescendantCount(const uint64_t pInDescendants) {
	this->vaultDescendants=pInDescendants;
}

string Vault::getContainerType() const {
    return "vault";
}

}
