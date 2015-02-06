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

#ifndef ELFCLOUD_KEYHINT_H_
#define ELFCLOUD_KEYHINT_H_

#include "Object.h"
#include "Types.h"

#include <string>

namespace elfcloud {

class KeyHint: public Object {
public:
    KeyHint();
	KeyHint(const ECHashAlgorithm pInKeyHashAlgorithm, const std::string& pInKeyHash, const ECEncryptionAlgorithm pInCipherType);
    ~KeyHint();

    std::string getKeyHash() const;
    ECEncryptionAlgorithm getCipherType() const;
    ECHashAlgorithm getKeyHashAlgorithm() const;

    std::string getCipherTypeString() const;
    std::string getKeyHashAlgorithmString() const;

    void setKeyHash(const std::string &pInKeyHash);
    void setCipherType(const ECEncryptionAlgorithm pInIntendedCipher);
    void setKeyHashAlgorithm(const ECHashAlgorithm pInKeyHashAlgorithm);

    std::string info() const;

    bool operator==(const KeyHint &other) const;
    bool operator!=(const KeyHint &other) const;
    bool operator<(const KeyHint &other) const;
    bool operator>(const KeyHint &other) const;

private:
    ECEncryptionAlgorithm cipherType;
    std::string cipherTypeString;

    ECHashAlgorithm keyHashAlgorithm;
    std::string keyHashAlgorithmString;

    std::string keyHash;
};

}

#endif /* ELFCLOUD_KEYHINT_H_ */


