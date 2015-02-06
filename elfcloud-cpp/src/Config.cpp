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

#include "Config.h"
#include "CryptoHelper.h"

#include <iostream>

using namespace CryptoPP;
using namespace std;

using rapidxml::xml_node;
using rapidxml::xml_attribute;

namespace elfcloud {

    Config::Config()
    {
        userConfigXmlDoc=NULL;
        userConfigXmlText=NULL;
    }

    Config::~Config()
    {
        releaseConfigXml();
    }

    void Config::releaseConfigXml() {
        if (userConfigXmlDoc) {
            userConfigXmlDoc->clear();
            delete userConfigXmlDoc;
            userConfigXmlDoc=NULL;
        }

        if (userConfigXmlText) {
            delete userConfigXmlText;
            userConfigXmlText=NULL;
        }
    }

    void Config::readUserConfig(const std::string &pInPath) {
        releaseConfigXml();

        userConfigXmlDoc=new rapidxml::xml_document<>();
        userConfigXmlText=CryptoHelper::readFile(pInPath, 1);
        userConfigXmlText->m_ptr[userConfigXmlText->size()-1]=0;
        userConfigXmlDoc->parse<0>((char*) userConfigXmlText->m_ptr);

        xml_node<> *ecRoot=userConfigXmlDoc->first_node("ec:root");
        xml_attribute<> *ecNs=ecRoot->first_attribute("xmlns:ec");
        if (strcmp(ecNs->value(), "https://secure.elfcloud.fi/xml/elfCLOUD")!=0) {
            releaseConfigXml();
            throw Exception(ECSCI_EXC_CONFIG_FILE_FORMAT_ERROR, "Unrecognized userconfig XML namespace");
        }

        //std::cout << *userConfigXmlDoc;
    }

    void Config::importKeysToKeyRing(elfcloud::KeyRing *pInRing) {
        xml_node<> *ecRoot=userConfigXmlDoc->first_node("ec:root");
        if (!ecRoot) throw Exception(ECSCI_EXC_CONFIG_FILE_FORMAT_ERROR, "Unexpected userconfig XML structure");

        xml_node<> *ecKeySet=ecRoot->first_node("ec:KeySet");
        if (!ecKeySet) throw Exception(ECSCI_EXC_CONFIG_FILE_FORMAT_ERROR, "Unexpected userconfig XML structure");

        xml_node<> *ecKey=ecKeySet->first_node("ec:Key");

        while (0!=ecKey) {
            xml_node<> *ecKeyCipher=ecKey->first_node("ec:Cipher");
            if (!ecKeyCipher) throw Exception(ECSCI_EXC_CONFIG_FILE_FORMAT_ERROR, "Unexpected userconfig XML structure");

            xml_node<> *ecItem=ecKeyCipher->first_node("ec:InitializationVector");
            if (!ecItem) throw Exception(ECSCI_EXC_CONFIG_FILE_FORMAT_ERROR, "Unexpected userconfig XML structure");

            xml_attribute<> *encoding=ecItem->first_attribute("encoding");
            if (0==encoding || strcmp(encoding->value(), "hexstring")) throw Exception(ECSCI_EXC_CONFIG_FILE_FORMAT_ERROR, "Unsupported IV encoding scheme");
            //cout << "Key/IV: " << ecItem->value() << endl;
            string iv=ecItem->value();

            ecItem=ecKeyCipher->first_node("ec:Mode");
            if (!ecItem) throw Exception(ECSCI_EXC_CONFIG_FILE_FORMAT_ERROR, "Unexpected userconfig XML structure");
            //cout << "Key/Mode: " << ecItem->value() << endl;
            string cipherMode=ecItem->value();

            ecItem=ecKeyCipher->first_node("ec:Type");
            if (!ecItem) throw Exception(ECSCI_EXC_CONFIG_FILE_FORMAT_ERROR, "Unexpected userconfig XML structure");
            //cout << "Key/Type: " << ecItem->value() << endl;
            string cipherType=ecItem->value();

            ecItem=ecKey->first_node("ec:Data");
            if (!ecItem) throw Exception(ECSCI_EXC_CONFIG_FILE_FORMAT_ERROR, "Unexpected userconfig XML structure");

            encoding=ecItem->first_attribute("encoding");
            if (0==encoding || strcmp(encoding->value(), "hexstring")) throw Exception(ECSCI_EXC_CONFIG_FILE_FORMAT_ERROR, "Unsupported key data encoding scheme");
            //cout << "Key/Data: " << ecItem->value() << endl;
            string keyData=ecItem->value();

            ecItem=ecKey->first_node("ec:Description");
            if (!ecItem) throw Exception(ECSCI_EXC_CONFIG_FILE_FORMAT_ERROR, "Unexpected userconfig XML structure");
            //cout << "Key/Description: " << ecItem->value() << endl;
            string keyDescription=ecItem->value();

            ecItem=ecKey->first_node("ec:Hash");
            if (!ecItem) throw Exception(ECSCI_EXC_CONFIG_FILE_FORMAT_ERROR, "Unexpected userconfig XML structure");

            encoding=ecItem->first_attribute("encoding");
            if (0==encoding || strcmp(encoding->value(), "hexstring")) throw Exception(ECSCI_EXC_CONFIG_FILE_FORMAT_ERROR, "Unsupported key hash encoding scheme");
            //cout << "Key/Hash: " << ecItem->value() << endl;
            string keyHash=ecItem->value();

            ecItem=ecKey->first_node("ec:ShortName");
            if (!ecItem) throw Exception(ECSCI_EXC_CONFIG_FILE_FORMAT_ERROR, "Unexpected userconfig XML structure");
            //cout << "Key/ShortName: " << ecItem->value() << endl;
            string keyName=ecItem->value();

            byte ivDataBin[2048];
            memset(ivDataBin, 14, 2048);
            unsigned int ivDataSizeBytes=10;
            CryptoHelper::HexStringToByteArray(iv, ivDataBin, &ivDataSizeBytes);

            byte keyDataBin[2048];
            memset(keyDataBin, 15, 2048);
            unsigned int keyDataSizeBytes=10;
            CryptoHelper::HexStringToByteArray(keyData, keyDataBin, &keyDataSizeBytes);
#if 0
            cout << "IV-Data round-trip: " << CryptoHelper::byteArrayToHexString(ivDataBin, ivDataSizeBytes) << endl;
            cout << "KeyData round-trip: " << CryptoHelper::byteArrayToHexString(keyDataBin, keyDataSizeBytes) << endl;
#endif            
            pInRing->addCipherKey(Key::CreateKey(keyName, keyDescription, CryptoHelper::getEncryptionAlgorithm(cipherType), cipherMode, keyDataBin, keyDataSizeBytes, ivDataBin, ivDataSizeBytes, ECSCI_HASHALG_MD5));
            ecKey=ecKey->next_sibling("ec:Key");
        }

    }

} // ns elfcloud

