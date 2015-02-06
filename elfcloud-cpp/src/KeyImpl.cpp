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

#include "KeyImpl.h"
#include "Exception.h"

#include "CryptoHelper.h"

#include <sstream>
#include <iostream>

#include "ext_cryptopp.h"

using namespace CryptoPP;

using std::cout;
using std::endl;

namespace elfcloud {

    KeyImpl::KeyImpl(): initialized(false) {
        //std::cout << "KeyImpl() default constructor" << std::endl;
    }

    KeyImpl::~KeyImpl() {
        //std::cout << "KeyImpl() destructor" << std::endl;
    }

    // Construct KeyImpl object with all KeyImpl data
    KeyImpl::KeyImpl(const std::string& pInName, const std::string& pInDescription,
             const ECEncryptionAlgorithm pInCipherType, const std::string& pInCipherMode,
             const byte *pInKeyData, const unsigned int pInKeyDataSize,
             const byte *pInIVData, const unsigned int pInIVDataSize,
             const ECHashAlgorithm pInKeyHashAlgorithm=ECSCI_HASHALG_MD5)
    {
        //std::cout << "KeyImpl() rich constructor" << std::endl;

        name.assign(pInName);
        desc.assign(pInDescription);
        mode.assign(pInCipherMode);

        hint.setKeyHashAlgorithm(pInKeyHashAlgorithm);
        hint.setCipherType(pInCipherType);

        key=CryptoPP::SecByteBlock(pInKeyDataSize);
        memcpy(key.m_ptr, pInKeyData, pInKeyDataSize);

        iv=CryptoPP::SecByteBlock(pInIVDataSize);
        memcpy(iv.m_ptr, pInIVData, pInIVDataSize);

        hint.setKeyHash(generateKeyHashString(hint.getKeyHashAlgorithm()));

        initialized=true;
    }

    const KeyHint KeyImpl::getHint() const {
        return hint;
    }

    CryptoPP::SecByteBlock KeyImpl::getKeySecBlock() const {
        return key;
    }

    CryptoPP::SecByteBlock KeyImpl::getIVSecBlock() const {
        return iv;
    }

    ECEncryptionAlgorithm KeyImpl::getCipherType() const {
        return hint.getCipherType();
    }

    std::string KeyImpl::getCipherMode() const {
        return mode;
    }

    std::string KeyImpl::getName() const {
        return name;
    }

    std::string KeyImpl::getDescription() const {
        return desc;
    }

    std::string KeyImpl::getKeyHash() const {
        return hint.getKeyHash();
    }

    elfcloud::ECHashAlgorithm KeyImpl::getKeyHashAlgorithm() const {
        return hint.getKeyHashAlgorithm();
    }

    std::string KeyImpl::info() const {
        std::stringstream ss;
        ss << "Key: [Name: " << name << ", Desc: " << desc << ", Cipher Type: " << hint.getCipherTypeString()
           << "Cipher Mode: " << mode
           << ", Key size: " << key.size() << " bytes, Key hash: " << hint.getKeyHash()
           << ", IV size: " << iv.size() << " bytes, key data @ " << &key << "]";

        return ss.str();
    }

    void KeyImpl::getKey(byte *pOutKey, const unsigned int pInBufferLength) {
        if (NULL==pOutKey || pInBufferLength < key.size())
            throw Exception(ECSCI_EXC_INIT_OR_PARAM_FAILURE, "Buffer too short");

        memcpy(pOutKey, key.m_ptr, key.size());
    }

    void KeyImpl::getIV(byte *pOutIV, const unsigned int pInBufferLength) {
        if (NULL==pOutIV || pInBufferLength < iv.size())
            throw Exception(ECSCI_EXC_INIT_OR_PARAM_FAILURE, "Buffer too short");

        memcpy(pOutIV, iv.m_ptr, iv.size());
    }

    std::string KeyImpl::generateKeyHashString(const ECHashAlgorithm pInKeyHashAlgorithm) {

        if (ECSCI_HASHALG_MD5==pInKeyHashAlgorithm) {

            byte hash[Weak::MD5::DIGESTSIZE];

            byte *ivAndKey=new byte[key.size()+iv.size()];
            memcpy(ivAndKey, iv.m_ptr, iv.size());
            memcpy(ivAndKey+iv.size(), key.m_ptr, key.size());

            byte *src=ivAndKey;
            unsigned int srcLen=iv.size()+key.size();

            for (int a=0; a<10000; a++) {
                byte tmp[Weak::MD5::DIGESTSIZE];
                CryptoHelper::getHashMD5AsByteArray(src, srcLen, tmp);
                memcpy(hash, tmp, Weak::MD5::DIGESTSIZE);
                src=hash;
                srcLen=Weak::MD5::DIGESTSIZE;
            }

            delete[] ivAndKey;
            return CryptoHelper::byteArrayToHexString(hash, Weak::MD5::DIGESTSIZE);
        } else {
            throw Exception(ECSCI_EXC_INIT_OR_PARAM_FAILURE, "Hash algorithm not supported by generateKeyHashString");
        }
    }


} // ns elfcloud

