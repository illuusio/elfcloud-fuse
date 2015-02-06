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

#include "Key.h"
#include "KeyImpl.h"
#include "Exception.h"

#include "Object.h"
#include "CryptoHelper.h"
#include "KeyHint.h"

#include <string>
#include <sstream>
#include <iostream>

namespace elfcloud {

    Key::Key() {
    }

    Key::~Key() {
    }

    Key *Key::CreateKey() {
        return new KeyImpl();
    }

    Key *Key::CreateKey(const std::string& pInName, const std::string& pInDescription,
             const ECEncryptionAlgorithm pInCipherType, const std::string& pInCipherMode,
             const byte *pInKeyData, const unsigned int pInKeyDataSize,
             const byte *pInIVData, const unsigned int pInIVDataSize,
             const ECHashAlgorithm pInKeyHashAlgorithm=ECSCI_HASHALG_MD5) {
        return new KeyImpl(pInName, pInDescription, pInCipherType, pInCipherMode,pInKeyData,pInKeyDataSize,pInIVData,pInIVDataSize,pInKeyHashAlgorithm);
    }

    Key *Key::CopyKey(const elfcloud::Key *pInKey) {
        return new KeyImpl(*((KeyImpl*) pInKey));
    }

} // ns elfcloud

