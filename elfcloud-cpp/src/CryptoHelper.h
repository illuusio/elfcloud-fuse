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

#ifndef ELFCLOUD_CRYPTOHELPER_H_
#define ELFCLOUD_CRYPTOHELPER_H_

#include "Object.h"

#include <string>

//#ifdef ELFCLOUD_LIB
#include "ext_cryptopp.h"
//#endif

#include "API.h"

namespace elfcloud {

class Key;

class CryptoHelper: public elfcloud::Object {
private:
    void *streamEncryption;
    void *streamDecryption;
    CryptoPP::Weak::MD5 *streamMD5HashEncrypted;
    CryptoPP::Weak::MD5 *streamMD5HashDecrypted;

public:
    CryptoHelper();
    virtual ~CryptoHelper();

    static std::string getHashAlgorithmName(elfcloud::ECHashAlgorithm pInHashAlg);
    static enum ECHashAlgorithm getHashAlgorithm(const std::string &pInHashAlgorithmName);

    static std::string getEncryptionAlgorithmName(elfcloud::ECEncryptionAlgorithm pInEncMode);
    static enum ECEncryptionAlgorithm getEncryptionAlgorithm(const std::string &pInEncryptionModeName);

    // ENCRYPT

    static bool encryptData(const elfcloud::Key *pInKey, const byte *pInData, byte *pOutData, const unsigned int pInDataSize);
    static bool encryptDataInPlace(const elfcloud::Key *pInKey, byte *pInOutData, const unsigned int pInOutDataSize);

    bool encryptDataStreamBegin(const elfcloud::Key *pInKey);
    bool encryptDataStreamContinue(const byte *pInData, byte *pOutData, const unsigned int pInDataSize);

    // DECRYPT

    static bool decryptData(const elfcloud::Key *pInKey, const byte *pInData, byte *pOutData, const unsigned int pInDataSize);
    static bool decryptDataInPlace(const elfcloud::Key *pInKey, byte *pInOutData, const unsigned int pInOutDataSize);

    bool decryptDataStreamBegin(const elfcloud::Key *pInKey);
    bool decryptDataStreamContinue(const byte *pInData, byte *pOutData, const unsigned int pInDataSize);
    std::string getHashEncryptedDataStream();
    std::string getHashDecryptedDataStream();

    // HASH

    static bool getHashMD5AsByteArray(byte *pInData, unsigned int pInDataSize, byte *pOutHash);

    static std::string base64Encode(byte *pInData, unsigned int pInDataSize);
    static bool base64Decode(const std::string pInEncodedData, byte *pOutDataBuffer, const unsigned int pInDataBufferSize, unsigned int *pOutDataSize);

    static std::string getHashMD5AsHexString(byte *pInData, unsigned int pInDataSize);

    // GENERIC
    static std::string byteArrayToHexString(const byte* pInByteArray, const unsigned int pInByteArrayLength);
    static void HexStringToByteArray(const std::string &pInHexString, byte *pOutBuffer, unsigned int *pOutBytesWritten);

    static CryptoPP::SecByteBlock* copyByteBufferToSecByteBlock(const byte* pInData, const unsigned int pInDataLength);
    static CryptoPP::SecByteBlock *readFile(std::string pInFilename, const unsigned int extraBuffer=0);
    static void writeFile(std::string pInFilename, CryptoPP::SecByteBlock *pInData);
};

}
#endif /* ELFCLOUD_CRYPTOHELPER_H_ */
