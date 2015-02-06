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

#ifndef ELFCLOUD_KEYRING_H_
#define ELFCLOUD_KEYRING_H_

#include <map>
#include <list>

#include "KeyHint.h"

namespace elfcloud {

class Key;

// KeyRing owns pointers to the keys. Copies are given out, free'd by recipient. In adding keys ring assumes key ownership.
typedef std::map<elfcloud::KeyHint, elfcloud::Key*> keymap_t;

    class KeyRing: public elfcloud::Object {
    private:
        keymap_t cipherKeys;
        elfcloud::KeyHint defaultKeyHint;

    public:
    	KeyRing();
    	~KeyRing();

        // MODIFY KEY RING

        void addCipherKey(elfcloud::Key *pInKey);
        void deleteCipherKey(const elfcloud::Key *pInKey);
        void setDefaultCipherKey(const elfcloud::KeyHint pInKeyHint);
	    void setDefaultCipherKey(const std::string pInKeyHash);

        // QUERY KEY RING

        std::list<elfcloud::Key*> getCipherKeys();

        elfcloud::Key *getCipherKey(elfcloud::KeyHint pInKeyHint);
        elfcloud::Key *getDefaultCipherKey();
    };

} // ns elfcloud

#endif // ELFCLOUD_KEYRING_H_

