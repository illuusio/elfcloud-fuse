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

#ifndef ELFCLOUD_VAULT_H_
#define ELFCLOUD_VAULT_H_

#include "Object.h"
#include "Exception.h"
#include "IllegalParameterException.h"
#include "IllegalParameterException.h"
#include "Container.h"
#include "Client.h"

#ifdef ELFCLOUD_LIB
#include <json/json.h>
#include <json/value.h>
#include <curl/curl.h>
#endif

#include <string.h>
#include <list>
#include <stdlib.h>
#include <vector>
#include <iostream>

namespace elfcloud {

class Client;

class Vault: public Container {
private:
    // Private constructors are used e.g. when populating vault lists.
	Vault() {}

	string vaultType;
	string vaultName;
	uint64_t vaultId;
	uint64_t vaultDataItems;
	uint64_t vaultDescendants;
	string lastAccessed;
	string lastModified;
	vector<string> permissions;
	uint64_t sizeBytes;

	void setDataItemCount(const uint64_t pInVaultDataItems);
	void setDescendantCount(const uint64_t pInDescendants);

#ifdef ELFCLOUD_LIB
	Json::Value createRequestRemoveVault();
	Json::Value createRequestAddVault();
	Json::Value createRequestRenameVault(string pInName);
#endif

    bool serverAddVault();
    bool serverRemoveVault();
    bool serverRenameVault(string newVaultName);

    void setId(const uint64_t pInVaultId);
    void setType(const string& pInVaultType);

    void setLastAccessed(string lastAccessed) {
        this->lastAccessed = lastAccessed;
    }

    void setLastModified(string lastModified) {
        this->lastModified = lastModified;
    }

    void setPermissions(vector<string> permissions) {
        this->permissions = permissions;
    }

    void setSizeBytes(uint64_t sizeBytes) {
        this->sizeBytes = sizeBytes;
    }

#ifdef ELFCLOUD_LIB
    static Json::Value createRequestListVault(const string& pInVaultType,
                                              const string& pInVaultRole,
                                              const uint64_t pInVaultId,
                                              const string pInGrouped);

#endif

#ifdef ELFCLOUD_LIB
    void initWithDictionary(Json::Value&);
#endif

protected:

public:
	Vault(Client *pInClient);
    Vault(Client *pInClient, const string pInName, const string pInType);
    virtual ~Vault();

    void assign(std::shared_ptr<Object> pInObject);

    void setName(const string& pInVaultName);
    void remove();

	string getType();
	string getName();
	uint64_t getId() const;
	uint64_t getContainerId() const;

    string getContainerType() const;

	uint64_t getDataItemCount();
	uint64_t getDescendantCount();

    static std::map<std::string, std::list<std::shared_ptr<Vault>>*>* ListVaultsGrouped(Client* pInClient,
                                const string pInVaultType="",
                                const string pInVaultRole="",
                                const uint64_t pInVaultId=0);

    static std::list<std::shared_ptr<Vault>>* ListVaults(Client* pInClient,
                                const string pInVaultType="",
                                const string pInVaultRole="",
                                const uint64_t pInVaultId=0);

    static void FreeVaultSet(std::list<std::shared_ptr<Vault>> *pInVaultList);
    static void FreeVaultSet(std::map<std::string, std::list<shared_ptr<Vault>> *> *pInVaultMap);

    static shared_ptr<Vault> getVault(list<shared_ptr<Vault>> *pInVaultList, std::string pInVaultName);

	string getLastAccessed() const {
		return lastAccessed;
	}

	string getLastModified() const {
		return lastModified;
	}

	vector<string> getPermissions() const {
		return permissions;
	}

	uint64_t getTotalSizeInBytes() const {
		return sizeBytes;
	}

};

}

#endif /* ELFCLOUD_VAULT_H_ */

