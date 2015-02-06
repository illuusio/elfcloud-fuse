
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

#ifndef _ELFCLOUDFS_CACHE_H_
#define _ELFCLOUDFS_CACHE_H_

#include <ctype.h>
#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <fuse.h>
#include <limits.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/xattr.h>
#include <time.h>
#include <sstream>


#include <cryptopp/whrlpool.h>
#include <cryptopp/hex.h>
#include <cryptopp/files.h>

#include <API.h>

using namespace std;
using namespace elfcloud;

/**
 *  Main class for ElfcloudFS
 */
class ElfcloudFSCache
{
private:
    FILE *m_SCacheFile;
    shared_ptr <elfcloud::Cluster> m_SCluster;
    shared_ptr <elfcloud::DataItem> m_SDataItem;
    uint64_t m_lOpenCount;
    uint64_t m_lFh;
    string m_strCacheFilename;
    string m_strOrigFilename;
    Client *m_SEclib;


    string getOpenFileCacheWholePathByPath(
        const char *path,
        uint64_t fh
    );

    string getCacheFileNameByPath(
        const char *path,
        uint64_t fh
    );

    string getFileHash(
        const char *path
    );


public:

    /**
     *  Constructor
     * @param eclib Elfcloud lib
     * @param originalfilename Original  name in cloud
     * @param cachefilename Cache name with path
     * @param cluster To which cluste file belongs
     * @param fh File descriptor
     */
    ElfcloudFSCache(
        Client *eclib,
        string originalfilename,
        string cachefilename,
        shared_ptr <elfcloud::Cluster> cluster,
        uint64_t fh
    );

    /**
     * Destructor
     */
    ~ElfcloudFSCache(
    );

    /**
     * Return cluster that dataitem belongs
     * @return cluster or NULL
     */
    shared_ptr <elfcloud::Cluster> getCluster(
    );

    /**
     * Return FILE handle
     * @return file handle
     */
    FILE *getFile(
    );

    /**
     * Return how many times file is opened
     * @return how many opens are for this file
     */
    uint64_t fileCount(
    );

    /**
     * Add one more open to file
     * @return how many opens are for this file
     */
    uint64_t openFile(
    );

    /**
     * Close one file
     * @return how many opens are for this file
     */
    uint64_t closeFile(
    );

    /**
     * File descriptor
     * @return unique file descriptor
     */
    uint64_t getFileFh(
    );

    /**
     * File name
     * @return original file name
     */
    string getOriginalFilename(
    );

    /**
     * return cache file name
     * @return cache file name with whole path
     */
    string getCacheFilename(
    );

    /**
     * Fetch item to cache
     * @return true if succes false if not
     */
    bool fetchItemToCache(
    );

    /**
     * Send file to cloud
     * @return true is success and false if not
     */
    bool storeItemToCloud(
    );

    /**
     * Create item to cache
     * @return true if success and false if not
     */
    bool createItemToCache(
    );

    /**
     * Remove item from cache
     * @return true is success and false if not
     */
    bool removeItemFromCache(
    );


    /**
     * Get dataitem that is stored to Cloud
     * @return elfcloud::DataItem or NULL if not stored yet
     */
    shared_ptr <elfcloud::DataItem> getStoredDataItem(
    );

};

#endif
