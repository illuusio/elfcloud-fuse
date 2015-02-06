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

#include "KeyHint.h"
#include "Exception.h"

#include <sstream>
#include "CryptoHelper.h"

namespace elfcloud {

    // Default constructor sets default value of MD5 hash algorithm and AES256 cipher type
    KeyHint::KeyHint() {
        setKeyHashAlgorithm(ECSCI_HASHALG_MD5);
        setCipherType(ECSCI_ENCALG_AES256);
    }

    KeyHint::KeyHint(const ECHashAlgorithm pInKeyHashAlgorithm, const std::string& pInKeyHash, const ECEncryptionAlgorithm pInCipherType) {
        setKeyHashAlgorithm(pInKeyHashAlgorithm);
        setCipherType(pInCipherType);
        setKeyHash(pInKeyHash);
    }

    KeyHint::~KeyHint() {
    }

    bool KeyHint::operator<(const KeyHint &other) const {
        //cout << "Comparing with-< hints: " << this->getKeyHash() << " vs. " << other.getKeyHash() << endl;
        if (keyHash.compare(other.keyHash)<0)
            return true;
        else
            return false;
    }

    bool KeyHint::operator>(const KeyHint &other) const {
        //cout << "Comparing with-> hints: " << this->getKeyHash() << " vs. " << other.getKeyHash() << endl;
        if (keyHash.compare(other.keyHash)>0)
            return true;
        else
            return false;
    }

    bool KeyHint::operator==(const KeyHint &other) const {
        //cout << "Comparing hints: " << this->getKeyHash() << " vs. " << other.getKeyHash() << endl;
        if (0==this->keyHash.compare(other.keyHash) && this->keyHashAlgorithm==other.keyHashAlgorithm &&
            this->cipherType==other.cipherType) {
            return true;
        } else {
            return false;
        }
    }

    bool KeyHint::operator!=(const KeyHint &other) const {
        return !(*this==other);
    }

    ECEncryptionAlgorithm KeyHint::getCipherType() const {
        return cipherType;
    }

    ECHashAlgorithm KeyHint::getKeyHashAlgorithm() const {
        return keyHashAlgorithm;
    }

    std::string KeyHint::getKeyHash() const {
        return keyHash;
    }

    void KeyHint::setKeyHash(const std::string &pInKeyHash) {
        keyHash.assign(pInKeyHash);
    }

    void KeyHint::setCipherType(const ECEncryptionAlgorithm pInCipherType) {
        try {
            // getEncryptionAlgorithmName will throw if the enum is unknown
            cipherTypeString.assign(CryptoHelper::getEncryptionAlgorithmName(pInCipherType));
            cipherType=pInCipherType;
        }
        catch (Exception &e) {
            // Invalid cipher type value
            throw e;
        }
    }

    void KeyHint::setKeyHashAlgorithm(const ECHashAlgorithm pInKeyHashAlgorithm) {
        try {
            // getHashAlgorithName will throw if the enum is unknown
            keyHashAlgorithmString.assign(CryptoHelper::getHashAlgorithmName(pInKeyHashAlgorithm));
            keyHashAlgorithm=pInKeyHashAlgorithm;
        }
        catch (Exception &e) {
            // Invalid algorithm value
            throw e;
        }
    }

    std::string KeyHint::getCipherTypeString() const {
        return cipherTypeString;
    }

    std::string KeyHint::getKeyHashAlgorithmString() const {
        return keyHashAlgorithmString;
    }

    std::string KeyHint::info() const {
        std::stringstream ss;
        ss << "KeyHint: [Hash: " << getKeyHash() << ", Hash Algorithm: << " << keyHashAlgorithmString << ", Cipher Type: " << cipherTypeString << "]";
        return ss.str();
    }

}

