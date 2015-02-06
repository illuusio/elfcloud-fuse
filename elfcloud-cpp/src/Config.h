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

#ifndef ELFCLOUD_CONFIG_H_
#define ELFCLOUD_CONFIG_H_

#include "Object.h"

#include <rapidxml.hpp>
#include <rapidxml_print.hpp>

#include "ext_cryptopp.h"

namespace elfcloud {

class KeyRing;

class Config: public elfcloud::Object {
private:
    rapidxml::xml_document<> *userConfigXmlDoc;
    CryptoPP::SecByteBlock *userConfigXmlText;

    void releaseConfigXml();

public:
	Config();
	~Config();

    void readUserConfig(const std::string &pInPath);
    void importKeysToKeyRing(elfcloud::KeyRing *pInRing);

};

} // ns elfcloud

#endif

