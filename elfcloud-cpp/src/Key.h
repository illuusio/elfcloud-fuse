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

#ifndef ELFCLOUD_KEY_H_
#define ELFCLOUD_KEY_H_

#include <string>

#include "Object.h"
#include "Types.h"

namespace elfcloud {

class KeyHint;
class KeyImpl;

class Key: public Object {
protected:
    Key();

public:
    virtual ~Key();
    static Key *CreateKey();
    static Key *CreateKey(const std::string& pInName, const std::string& pInDescription,
             const ECEncryptionAlgorithm pInCipherType, const std::string& pInCipherMode,
             const byte *pInKeyData, const unsigned int pInKeyDataSize,
             const byte *pInIVData, const unsigned int pInIVDataSize,
             const ECHashAlgorithm pInKeyHashAlgorithm);
    static Key *CopyKey(const elfcloud::Key *pInKey);

    virtual void getKey(byte *pOutKey, const unsigned int pInBufferLength) = 0;
    virtual void getIV(byte *pOutIV, const unsigned int pInBufferLength) = 0;

    virtual std::string getName() const = 0;
    virtual std::string getDescription() const = 0;
    virtual std::string getKeyHash() const = 0;
    virtual elfcloud::ECHashAlgorithm getKeyHashAlgorithm() const = 0;
    virtual elfcloud::ECEncryptionAlgorithm getCipherType() const = 0;
    virtual std::string getCipherMode() const = 0;
    virtual std::string info() const = 0;

    virtual const KeyHint getHint() const = 0;
};

}

#endif /* ELFCLOUD_KEY_H_ */

