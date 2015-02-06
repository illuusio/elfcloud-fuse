/*
 * Copyright (c) 2014-2015, Ilmi Solutions Oy
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following
 * conditions are met:
 *
 * * Redistributions of source code must retain the above copyright notice,
 *   this list of conditions and the following disclaimer.
 * * Redistributions in binary form must reproduce the above copyright notice,
 *   this list of conditions and the following disclaimer
 *   in the documentation and/or other materials provided with the distribution.
 * * Neither the name of the Ilmi Solutions Oy nor the names of its
 *   contributors may be used to endorse or promote products derived
 *   from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 * COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED
 * OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 */

/*
 * Revision info:
 * $Date$
 * $Rev$
 * $Author$
 */

#include "elfcloudfs-cache.hh"

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <cryptopp/whrlpool.h>
#include <cryptopp/hex.h>

#include <map>

using namespace std;
using namespace elfcloud;


/**
  *  Constructor
  */
ElfcloudFSCache::ElfcloudFSCache(
    Client *eclib,
    string originalfilename,
    string cachefilename,
    shared_ptr<elfcloud::Cluster> cluster,
    uint64_t fh
)
{
    m_lOpenCount = 1;
    m_SEclib = eclib;
    m_lFh = fh;
    m_SCluster = cluster;
    m_strCacheFilename = cachefilename;
    m_strOrigFilename = originalfilename;
    m_SCacheFile = 0;
    m_SDataItem = 0;
}

/**
 * Destructor
 */
ElfcloudFSCache::~ElfcloudFSCache(
)
{

}

shared_ptr<elfcloud::Cluster> ElfcloudFSCache::getCluster()
{
    return m_SCluster;
}

FILE *ElfcloudFSCache::getFile()
{
    if( m_SCacheFile == NULL )
    {
        cerr << "ElfcloudFSCache::getFile(): File not open!" << endl;
    }

    return m_SCacheFile;
}

uint64_t ElfcloudFSCache::fileCount()
{
    return m_lOpenCount;
}


uint64_t ElfcloudFSCache::openFile()
{
    return ++m_lOpenCount;
}

uint64_t ElfcloudFSCache::closeFile()
{
    return  --m_lOpenCount;
}

uint64_t ElfcloudFSCache::getFileFh()
{
    return m_lFh;
}


string ElfcloudFSCache::getOriginalFilename()
{
    return m_strOrigFilename;
}

string ElfcloudFSCache::getCacheFilename()
{
    return m_strCacheFilename;
}

bool ElfcloudFSCache::createItemToCache()
{
    m_SCacheFile = fopen(getCacheFilename().data(), "w+");

    if( m_SCacheFile == NULL )
    {
        cerr << "ElfcloudFSCache::fetchItemToCache() Can't open file" << endl;
        return false;
    }

    return true;
}


bool ElfcloudFSCache::fetchItemToCache()
{
    shared_ptr<elfcloud::DataItemFilePassthrough> l_SFile(new DataItemFilePassthrough(m_SEclib));
    string l_SChecksum;
    FILE *l_SChecksumFile = NULL;
    std::stringstream l_strSs;

    l_SFile->setFilePath(getCacheFilename().data());

    try
    {
        l_SFile->setDataItemName(getOriginalFilename());
    }

    catch (elfcloud::Exception &e)
    {
        cerr << "ElfcloudFSCache::fetchItemToCache(): IllegalParameterException: " << e.getCode() << ", " << e.getMsg() << endl;
        return false;

    }

    if( createItemToCache() == false )
    {
        return false;
    }

    try
    {
        if( m_SCluster->fetchDataItem(l_SFile) == false )
        {
            cerr << "ElfcloudFSCache::fetchItemToCache(): Can't fetch item!" << endl;
        }
    }

    catch (elfcloud::Exception &e)
    {
        cerr << "ElfcloudFSCache::fetchItemToCache(): " << e.getCode() << ", " << e.getMsg() << endl;
        fclose(m_SCacheFile);
        m_SCacheFile = NULL;
        return false;

    }

    l_SChecksum = getFileHash(getCacheFilename().data());
    l_strSs << getCacheFilename() << "." << "whirlpool";
    l_SChecksumFile = fopen(l_strSs.str().data(), "w");

    if( l_SChecksumFile == NULL )
    {
        cerr << "ElfcloudFSCache::fetchItemToCache: Cache error" << endl;
        ElfcloudFSCache::removeItemFromCache();
        return false;
    }

    fwrite(l_SChecksum.data(), 128, 1, l_SChecksumFile);
    fclose(l_SChecksumFile);

    return true;
}

bool ElfcloudFSCache::storeItemToCloud()
{
    shared_ptr<elfcloud::DataItemFilePassthrough> l_SFile(new DataItemFilePassthrough(m_SEclib));
    char l_strFileChecksum[128];
    string l_SChecksum;
    FILE *l_SChecksumFile = NULL;
    std::stringstream l_strSs;

    l_strSs << getCacheFilename() << "." << "whirlpool";
    l_SChecksumFile = fopen(l_strSs.str().data(), "r");

    memset(l_strFileChecksum, 0x00, 128);

    if( l_SChecksumFile != NULL )
    {
        fread(l_strFileChecksum, 128, 1, l_SChecksumFile);
        fclose(l_SChecksumFile);

        if( strncmp( l_strFileChecksum, getFileHash(getCacheFilename().data()).data(), 128) == 0 )
        {
            return true;
        }
    }

    l_SFile->setFilePath(getCacheFilename().data());

    try
    {
        l_SFile->setDataItemName(getOriginalFilename());
    }

    catch (elfcloud::Exception &e)
    {
        cerr << "ElfcloudFSCache::storeItemToCloud: IllegalParameterException: " << e.getCode() << ", " << e.getMsg() << endl;
        return false;

    }

    try
    {
        if( m_SCluster->storeDataItem(l_SFile) == false )
        {
            cerr << "ElfcloudFSCache::storeItemToCloud: Can't find store dataitem to cluster";
            cerr << " (" << m_SCluster->getClusterName() << ")!!" << endl;
            return false;
        }
    }

    catch (elfcloud::Exception &e)
    {
        cerr << "ElfcloudFSCache::storeItemToCloud: Exception: " << e.getCode() << ", " << e.getMsg() << endl;
        return false;

    }

    m_SDataItem = l_SFile;

    return true;
}

bool ElfcloudFSCache::removeItemFromCache()
{
    struct stat l_SSb;
    std::stringstream l_strSs;

    l_strSs << getCacheFilename() << "." << "whirlpool";

    if( m_SCacheFile != NULL )
    {
        fclose(m_SCacheFile);
        m_SCacheFile = NULL;
    }

    if (stat(getCacheFilename().data(), &l_SSb) != -1)
    {
        unlink(getCacheFilename().data());
    }

    if (stat(l_strSs.str().data(), &l_SSb) != -1)
    {
        unlink(l_strSs.str().data());
    }


    return true;
}


string ElfcloudFSCache::getFileHash(const char *path)
{
    string l_strResult;
    CryptoPP::Whirlpool l_SWhirlpool;
    CryptoPP::FileSource(path, true,
                         new CryptoPP::HashFilter(l_SWhirlpool, new CryptoPP::HexEncoder(
                                     new CryptoPP::StringSink(l_strResult), true)));
    return l_strResult;

}

shared_ptr < elfcloud::DataItem > ElfcloudFSCache::getStoredDataItem()
{
    return m_SDataItem;
}
