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

#ifndef ELFCLOUD_KEYIMPL_H_
#define ELFCLOUD_KEYIMPL_H_

#include "Key.h"
#include "Object.h"
#include "KeyHint.h"

#include "API.h"

#include <string>

#include "ext_cryptopp.h"
using namespace CryptoPP;

namespace elfcloud {

class KeyImpl: public Key {
public:
    KeyImpl();
    KeyImpl(const std::string& pInName, const std::string& pInDescription,
             const ECEncryptionAlgorithm pInCipherType, const std::string& pInCipherMode,
             const byte *pInKeyData, const unsigned int pInKeyDataSize,
             const byte *pInIVData, const unsigned int pInIVDataSize,
             const ECHashAlgorithm pInKeyHashAlgorithm);
    ~KeyImpl();

    void getKey(byte *pOutKey, const unsigned int pInBufferLength);
    void getIV(byte *pOutIV, const unsigned int pInBufferLength);

    std::string getName() const;
    std::string getDescription() const;
    std::string getKeyHash() const;
    elfcloud::ECHashAlgorithm getKeyHashAlgorithm() const;
    elfcloud::ECEncryptionAlgorithm getCipherType() const;
    std::string getCipherMode() const;
    std::string info() const;

    const KeyHint getHint() const;

    bool isInitialized() { return initialized; }

private:
    std::string name;
    std::string desc;
    std::string mode;

    bool initialized;

    elfcloud::KeyHint hint;

    std::string generateKeyHashString(const elfcloud::ECHashAlgorithm pInKeyHashAlgorithm);

private:
    CryptoPP::SecByteBlock key;
    CryptoPP::SecByteBlock iv;

public:
    CryptoPP::SecByteBlock getKeySecBlock() const;
    CryptoPP::SecByteBlock getIVSecBlock() const;

};

} // ns elfcloud

#endif

