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

#include "CryptoHelper.h"

#include "IllegalParameterException.h"
#include "Key.h"

#include <string>
#include <iostream>
#include <fstream>
#include <sstream>
#include <iomanip>
#include "KeyImpl.h"

#include "ext_cryptopp.h"

using namespace std;
using namespace CryptoPP;

namespace elfcloud {

    CryptoHelper::CryptoHelper() {
        streamEncryption=0;
        streamDecryption=0;
        streamMD5HashEncrypted=0;
        streamMD5HashDecrypted=0;
    }

    CryptoHelper::~CryptoHelper() {
        if (streamEncryption) {
            delete ((CFB_Mode<AES>::Encryption*) streamEncryption);
            streamEncryption=0;
        }
        if (streamDecryption) {
            delete ((CFB_Mode<AES>::Decryption*) streamDecryption);
            streamDecryption=0;
        }
        if (streamMD5HashEncrypted) {
            delete streamMD5HashEncrypted;
            streamMD5HashEncrypted=0;
            delete streamMD5HashDecrypted;
            streamMD5HashDecrypted=0;
        }
    }

    // static
    string CryptoHelper::getHashAlgorithmName(enum ECHashAlgorithm pInHashAlg) {
        switch (pInHashAlg) {
            case ECSCI_HASHALG_MD5: {
                return "MD5";
            }
            case ECSCI_HASHALG_SHA256: {
                return "SHA256";
            }
            default: {
                throw Exception();
            }
        }
    }

    // static
    enum ECHashAlgorithm CryptoHelper::getHashAlgorithm(const std::string &pInHashAlgorithmName) {
    	if (!pInHashAlgorithmName.compare("MD5")) return ECSCI_HASHALG_MD5;
    	if (!pInHashAlgorithmName.compare("SHA256")) return ECSCI_HASHALG_SHA256;
    	throw Exception();
    }

    // static
    string CryptoHelper::getEncryptionAlgorithmName(enum ECEncryptionAlgorithm pInEncMode) {
    	switch (pInEncMode) {
    		case ECSCI_ENCALG_AES128: {
    			return "AES128";
    		}
    		case ECSCI_ENCALG_AES192: {
    			return "AES192";
    		}
    		case ECSCI_ENCALG_AES256: {
    			return "AES256";
    		}
    		default: {
    			throw new Exception(ECSCI_EXC_INIT_OR_PARAM_FAILURE, "getEncryptionAlgorithmName(): Unknown encryption algorithm");
    		}
    	}
    }

    // static
    enum ECEncryptionAlgorithm CryptoHelper::getEncryptionAlgorithm(const string &pInEncryptionAlgorithmName) {
    	if (!pInEncryptionAlgorithmName.compare("AES128")) return elfcloud::ECSCI_ENCALG_AES128;
    	if (!pInEncryptionAlgorithmName.compare("AES192")) return ECSCI_ENCALG_AES192;
    	if (!pInEncryptionAlgorithmName.compare("AES256")) return ECSCI_ENCALG_AES256;
    	throw new Exception(ECSCI_EXC_INIT_OR_PARAM_FAILURE, "Unknown encryption algorithm name given");
    }

    SecByteBlock* CryptoHelper::copyByteBufferToSecByteBlock(const byte* pInData, const unsigned int pInDataLength) {
    	SecByteBlock *sBB = new SecByteBlock(pInDataLength);
    	memcpy(sBB->m_ptr, pInData, pInDataLength);
    	return sBB;
    }

    bool CryptoHelper::encryptDataStreamBegin(const elfcloud::Key *pInKey) {
        if (streamEncryption) {
            delete ((CFB_Mode<AES>::Encryption*) streamEncryption);
            streamEncryption=0;
        }

        if (NULL!=pInKey) {
            ECEncryptionAlgorithm alg=pInKey->getHint().getCipherType();
            if ((ECSCI_ENCALG_AES128==alg || ECSCI_ENCALG_AES192==alg || ECSCI_ENCALG_AES256==alg) && pInKey->getCipherMode()=="CFB8") {
                try {
                    streamEncryption=new CFB_Mode<AES>::Encryption(((KeyImpl*) pInKey)->getKeySecBlock().m_ptr, ((KeyImpl*) pInKey)->getKeySecBlock().size(), ((KeyImpl*) pInKey)->getIVSecBlock().m_ptr, 1);
                    return true;
                } catch (...) {
                }
            } else {
                throw Exception(ECSCI_EXC_ENCRYPTION_ERROR, "Unknown cipher type or mode");
            }
        }
        return false;
    }

    bool CryptoHelper::encryptDataStreamContinue(const byte *pInData, byte *pOutData, const unsigned int pInDataSize) {
        try {
            ((CFB_Mode<AES>::Encryption*) streamEncryption)->ProcessData(pOutData, pInData, pInDataSize); 
            return true;
        } catch (...) {
            return false;
        }
    }

    bool CryptoHelper::encryptData(const elfcloud::Key *pInKey, const byte *pInData, byte *pOutData, const unsigned int pInDataSize)
    {
        if (NULL!=pInKey) {
            ECEncryptionAlgorithm alg=pInKey->getHint().getCipherType();
            if ((ECSCI_ENCALG_AES128==alg || ECSCI_ENCALG_AES192==alg || ECSCI_ENCALG_AES256==alg) && pInKey->getCipherMode()=="CFB8") {
                try {
                    CFB_Mode<AES>::Encryption cfbEncryption(((KeyImpl*) pInKey)->getKeySecBlock().m_ptr, ((KeyImpl*) pInKey)->getKeySecBlock().size(), ((KeyImpl*) pInKey)->getIVSecBlock().m_ptr, 1);
                    cfbEncryption.ProcessData(pOutData, pInData, pInDataSize);
                    return true;
                } catch (...) {
                }
            } else {
                throw Exception(ECSCI_EXC_ENCRYPTION_ERROR, "Unknown cipher type or mode");
            }
        }

        return false;
    }

    bool CryptoHelper::encryptDataInPlace(const Key *pInKey, byte *pInOutData, const unsigned int pInOutDataSize)
    {
        return encryptData(pInKey, pInOutData, pInOutData, pInOutDataSize);
    }

    bool CryptoHelper::decryptDataStreamBegin(const elfcloud::Key *pInKey) {
        if (streamDecryption) {
            delete ((CFB_Mode<AES>::Decryption*) streamDecryption);
            streamDecryption=0;
        }

        if (streamMD5HashEncrypted) {
            delete streamMD5HashEncrypted;
            delete streamMD5HashDecrypted;
            streamMD5HashEncrypted=0;
            streamMD5HashDecrypted=0;
        }

        streamMD5HashEncrypted=new Weak::MD5();
        streamMD5HashDecrypted=new Weak::MD5();

        if (NULL!=pInKey) {
            ECEncryptionAlgorithm alg=pInKey->getHint().getCipherType();
            if ((ECSCI_ENCALG_AES128==alg || ECSCI_ENCALG_AES192==alg || ECSCI_ENCALG_AES256==alg) && pInKey->getCipherMode()=="CFB8") {
                try {
                    streamDecryption=new CFB_Mode<AES>::Decryption(((KeyImpl*) pInKey)->getKeySecBlock().m_ptr, ((KeyImpl*) pInKey)->getKeySecBlock().size(), ((KeyImpl*) pInKey)->getIVSecBlock().m_ptr, 1);
                    return true;
                } catch (...) {
                }
            } else {
                throw Exception(ECSCI_EXC_ENCRYPTION_ERROR, "Unknown cipher type or mode");
            }
        }
        return false;
    }

    bool CryptoHelper::decryptDataStreamContinue(const byte *pInData, byte *pOutData, const unsigned int pInDataSize) {
        try {
            streamMD5HashEncrypted->Update(pInData, pInDataSize);
            ((CFB_Mode<AES>::Decryption*) streamDecryption)->ProcessData(pOutData, pInData, pInDataSize); 
            streamMD5HashDecrypted->Update(pOutData, pInDataSize);
            return true;
        } catch (...) {
            return false;
        }
    }

    std::string CryptoHelper::getHashEncryptedDataStream() {
        if (!streamMD5HashEncrypted) {
            return "not initialized";
        }

        byte digest[Weak::MD5::DIGESTSIZE];
        streamMD5HashEncrypted->Final(digest);
    	return byteArrayToHexString(digest, Weak::MD5::DIGESTSIZE);
    }

    std::string CryptoHelper::getHashDecryptedDataStream() {
        if (!streamMD5HashDecrypted) {
            return "not initialized";
        }

        byte digest[Weak::MD5::DIGESTSIZE];
        streamMD5HashDecrypted->Final(digest);
        return byteArrayToHexString(digest, Weak::MD5::DIGESTSIZE);
    }

    bool CryptoHelper::decryptData(const Key *pInKey, const byte *pInData, byte *pOutData, const unsigned int pInDataSize) {
        if (NULL!=pInKey) {
            ECEncryptionAlgorithm alg=pInKey->getHint().getCipherType();
            if ((ECSCI_ENCALG_AES128==alg || ECSCI_ENCALG_AES192==alg || ECSCI_ENCALG_AES256==alg) && pInKey->getCipherMode()=="CFB8") {
            	try {
            		CFB_Mode<AES>::Decryption cfbDecryption(((KeyImpl*) pInKey)->getKeySecBlock().m_ptr, ((KeyImpl*) pInKey)->getKeySecBlock().size(), ((KeyImpl*) pInKey)->getIVSecBlock().m_ptr, 1);
            		cfbDecryption.ProcessData(pOutData, pInData, pInDataSize);
            		return true;
            	} catch (...) {
            	}
            } else {
                throw Exception(ECSCI_EXC_ENCRYPTION_ERROR, "Unknown cipher type or mode");
            }
        }

        return false;
    }

    bool CryptoHelper::decryptDataInPlace(const Key *pInKey, byte *pInOutData, const unsigned int pInOutDataSize) {
        return decryptData(pInKey, pInOutData, pInOutData, pInOutDataSize);
    }

    bool CryptoHelper::getHashMD5AsByteArray(byte *pInData, unsigned int pInDataSize, byte *pOutHash) {
    	if (!pInData || !pOutHash || !pInDataSize) {
    		return false;
    	}
    	memset(pOutHash, 0, Weak::MD5::DIGESTSIZE);
    	Weak::MD5().CalculateDigest(pOutHash, pInData, pInDataSize);
    	return true;
    }

    string CryptoHelper::getHashMD5AsHexString(byte *pInData, unsigned int pInDataSize) {
    	if (!pInData || !pInDataSize) {
    		return "Invalid input for MD5 hash";
    	}

    	byte digest[Weak::MD5::DIGESTSIZE];
    	memset(digest, 0, Weak::MD5::DIGESTSIZE);

    	Weak::MD5().CalculateDigest(digest, pInData, pInDataSize);
    	return byteArrayToHexString(digest, Weak::MD5::DIGESTSIZE);
    }

    // static
    string CryptoHelper::byteArrayToHexString(const byte* pInByteArray, const unsigned int pInByteArrayLength) {
    	stringstream ss;
    	for (unsigned int i = 0; i < pInByteArrayLength; i++)
    		ss << setw(2) << setfill('0') << hex << (unsigned int) pInByteArray[i];
    	return ss.str();
    }

    // static
    void CryptoHelper::HexStringToByteArray(const std::string &pInHexString, byte *pOutBuffer, unsigned int *pOutBytesWritten) {
        string decoded;
        StringSource ss(pInHexString, true, new HexDecoder(new StringSink(decoded)));

        for (unsigned int i=0; i<decoded.size(); i++) {
            pOutBuffer[i]=decoded.at(i);
        }
        *pOutBytesWritten=decoded.size();
    }

    // static
    string CryptoHelper::base64Encode(byte *pInData, unsigned int pInDataSize) {
    	string output;
    	Base64Encoder encoder;
    	encoder.Attach(new StringSink(output));
    	encoder.Put( pInData, pInDataSize );
    	encoder.MessageEnd();

        // Remove trailing linefeed
        output.erase(output.end()-1, output.end());
        return output;
    }

    // static
    bool CryptoHelper::base64Decode(const string pInEncodedData, byte *pOutDataBuffer, const unsigned int pInDataBufferSize, unsigned int *pOutDataSize) {
    	Base64Decoder decoder;
    	ArraySink *aSink=new ArraySink(pOutDataBuffer, pInDataBufferSize);
    	decoder.Attach(aSink);
    	decoder.Put((byte*) pInEncodedData.c_str(), pInEncodedData.size());
    	decoder.MessageEnd();
    	*pOutDataSize=aSink->TotalPutLength();
    	return true;
    }

#ifdef ELFCLOUD_LIB
    // static
    CryptoPP::SecByteBlock *CryptoHelper::readFile(std::string pInFilename, const unsigned int extraBuffer) {

        FILE *fileHandle=fopen(pInFilename.c_str(), "rb");
        if (fileHandle==NULL) {
            throw Exception(ECSCI_EXC_GENERIC_FILE_OPEN_ERROR, "Unable to open file for binary read");
        }

        fseek(fileHandle, 0L, SEEK_END);
        unsigned int fileLength=ftell(fileHandle);
        fclose(fileHandle);

    	CryptoPP::SecByteBlock *secBlock;
    	secBlock = new CryptoPP::SecByteBlock(fileLength+extraBuffer);
    	if (!secBlock->m_ptr) {
    		throw Exception(ECSCI_EXC_GENERIC_MEM_ALLOC_ERROR, "Unable to allocate single buffer for the file");
        }

        ifstream fin(pInFilename.c_str(), ios_base::in | ios_base::binary);
        if (fin.good()) {
            fin.read((char*) secBlock->m_ptr, fileLength);
            if (fin.fail()) {
    			delete secBlock;
    			secBlock=NULL;
            }
    		fin.close();
        }

    	if (secBlock)
    		return secBlock;

    	throw Exception(ECSCI_EXC_GENERIC_FILE_READ_ERROR, "Failed to read the file contents");
    }

    // static
    void CryptoHelper::writeFile(std::string pInFilename, CryptoPP::SecByteBlock *pInData) {
    	return;
    }
#endif

} // ns elfcloud


