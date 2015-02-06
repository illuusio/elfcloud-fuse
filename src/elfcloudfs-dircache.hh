
/*
 * Copyright (c) 2015, Ilmi Solutions Oy
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
 * * Neither the name of the <ORGANIZATION> nor the names of its
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

#ifndef _ELFCLOUDFS_DIRCACHE_H_
#define _ELFCLOUDFS_DIRCACHE_H_

#include <API.h>

using namespace std;
using namespace elfcloud;

/**
 *  Main class for ElfcloudFS
 */
class ElfcloudDirCache
{
private:
    Client *m_SEclib;
    shared_ptr <elfcloud::Cluster> m_SCluster;
    string m_strPath;
    long l_STime;

    map <string, shared_ptr <elfcloud::Cluster>> m_SDirs;

    map <string, shared_ptr <elfcloud::DataItem>> m_SFiles;


public:

    /**
     *  Constructor
     * @param eclib Elfcloud lib
     * @param path What path this prensents
     * @param cluster Cluster for this path
     */
    ElfcloudDirCache(
        Client *eclib,
        string path,
        shared_ptr <elfcloud::Cluster> cluster
    );

    /**
     * Destructor
     */
    ~ElfcloudDirCache(
    );

    /**
     * Return cluster that is in this Object
     * @return Cluster
     */
    shared_ptr <elfcloud::Cluster> getCluster(
    );

    /**
     * List of sub-clusters
     * @return sub-clusters
     */
    list <shared_ptr <elfcloud::Cluster>> *getClusterList(
                                       );

    /**
     * List of Dataitems
     * @return dataitems
     */
    list <shared_ptr < elfcloud::DataItem>> *getDataitemList(
                                         );

    /**
     * Does file exist in dir
     * @param fileName filene of file
     * @return true if exists false if not
     */
    bool isFile(
        string fileName
    );

    /**
     * Does Directory exist in dir
     * @param cluster filene of cluster
     * @return true is exists false if not
     */
    bool isDirectory(
        string cluster
    );

    /**
     * Get filenames DataItems
     * @param fileName what Dataitem we want?
     * @return Dataitem or NULL if not exist
     */

    shared_ptr <elfcloud::DataItem> getFile(
        string fileName
    );

    /**
     * Get cluster that is mapped with that name
     * @param cluster Cluster name
     * @return Cluster or NULL if not exist
     */

    shared_ptr <elfcloud::Cluster> getDirectory(
        string cluster
    );

    /**
     * Get filenames in vector
     * @return Filenames as strings in vector
     */

    vector <string> getFileNames(
    );

    /**
     * Get Cluster names in vector
     * @return Cluster names as strings in vector
     */
    vector <string> getDirectoryNames(
    );

    /**
     * Reload Clusters/Directories
     *
     * @return true is success false if not
     */

    bool reloadDirectories(
    );

    /**
     * Reload Files/Dataitems
     *
     * @return true is success false if not
     */
    bool reloadFiles(
    );

    /**
     * Remove directory from list
     * @param cluster Directory name
     * @return true if success false if not
     */
    bool removeDirectory(
        string cluster
    );

    /**
     * Remove File from list
     * @param dataitem Filename name
     * @return true if success false if not
     */
    bool removeFile(
        string dataitem
    );

    /**
     * Add directory to list
     * @param cluster Cluster to add
     * @return true if success false if not
     */

    bool addDirectory(
        shared_ptr <elfcloud::Cluster> cluster
    );


    /**
     * Add file to list
     * @param dataitem File to add
     * @return true if success false if not
     */
    bool addFile(
        shared_ptr <elfcloud::DataItem> dataitem
    );


};

#endif
