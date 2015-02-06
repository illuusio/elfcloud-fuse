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

#include "KeyRing.h"
#include "API.h"

using namespace std;

namespace elfcloud {

    KeyRing::KeyRing() {
    }

    KeyRing::~KeyRing() {
        keymap_t::iterator iter=cipherKeys.begin();

        while (iter!=cipherKeys.end()) {
            delete iter->second;
            cipherKeys.erase(iter);
            iter=cipherKeys.begin();
        }

    }

    // Ring takes ownership of the key if added
    void KeyRing::addCipherKey(elfcloud::Key *pInKey) {
        keymap_t::const_iterator iter=cipherKeys.find(pInKey->getHint());

        if (iter!=cipherKeys.end()) {
            // Key is already in the map
            throw IllegalParameterException(ECSCI_EXC_DUPLICATE_UNIQUE_IDENTIFIER, "Key hint matches existing key");
        }

        cipherKeys[pInKey->getHint()]=pInKey;

		// TEMP TEMP tto tonttu
		if (cipherKeys.size()==1) {
			// We just added first key, make it the default
			setDefaultCipherKey(pInKey->getHint());
		}
    }

    void KeyRing::deleteCipherKey(const elfcloud::Key *pInKey) {
        KeyHint inKeyHint=pInKey->getHint();
        keymap_t::iterator iter=cipherKeys.find(inKeyHint);

        if (iter!=cipherKeys.end()) {
            if (defaultKeyHint==inKeyHint) {
                defaultKeyHint=KeyHint();
            }

            cipherKeys.erase(iter);
        }
    }

    void KeyRing::setDefaultCipherKey(const std::string pInKeyHash) {
        if (pInKeyHash.empty()) {
            throw IllegalParameterException(ECSCI_EXC_INIT_OR_PARAM_FAILURE, "Key hash not given");
        }

        keymap_t::iterator iter=cipherKeys.begin();

		while (iter!=cipherKeys.end()) {
			if ((*iter).first.getKeyHash().compare(pInKeyHash)==0) {
				defaultKeyHint=(*iter).first;
				return;
			}
			iter++;
		}
    }

    void KeyRing::setDefaultCipherKey(const elfcloud::KeyHint pInKeyHint) {
        if (pInKeyHint.getKeyHash().empty()) {
            throw IllegalParameterException(ECSCI_EXC_INIT_OR_PARAM_FAILURE, "Key hint instance not initialized");
        }

        keymap_t::iterator iter=cipherKeys.find(pInKeyHint);

        if (iter==cipherKeys.end()) {
            throw IllegalParameterException(ECSCI_EXC_INIT_OR_PARAM_FAILURE, "Key hint does not match any keys in the key ring");
        } else {
            defaultKeyHint=pInKeyHint;
        }
    }

    std::list<elfcloud::Key *> KeyRing::getCipherKeys() {

        keymap_t::const_iterator iter=cipherKeys.begin();

        std::list<elfcloud::Key *> keyList;
        while (iter!=cipherKeys.end()) {
            keyList.push_back(Key::CopyKey(iter->second));
			iter++;
        }

        return keyList;
    }

    elfcloud::Key *KeyRing::getCipherKey(elfcloud::KeyHint pInKeyHint) {
        if (pInKeyHint.getKeyHash().size()==0) {
            throw Exception(ECSCI_EXC_KEYMGMT_KEY_NOT_FOUND, "Key hash is empty, key ring not searched");
        }

        keymap_t::iterator iter=cipherKeys.find(pInKeyHint);

        if (iter==cipherKeys.end()) {
            throw Exception(ECSCI_EXC_KEYMGMT_KEY_NOT_FOUND, "Key hint does not match any keys in the key ring");
        }

        return Key::CopyKey(iter->second);
    }

    elfcloud::Key *KeyRing::getDefaultCipherKey() {
        try {
            return getCipherKey(defaultKeyHint);
        }
        catch (Exception &e) {
			Client::log("DataItem/getDefaultCipherKey(): Default key has not been set or cannot be found");
            throw Exception(ECSCI_EXC_KEYMGMT_DEFAULT_KEY_NOT_SET, "Default key has not been set or cannot be found");
        }
    }

} // ns elfcloud
