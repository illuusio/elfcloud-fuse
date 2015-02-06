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

#include "elfcloudfs-dircache.hh"

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include <map>

using namespace std;
using namespace elfcloud;


/**
  *  Constructor
  */
ElfcloudDirCache::ElfcloudDirCache(
    Client *eclib,
    string path,
    shared_ptr < elfcloud::Cluster > cluster
)
{
    m_SEclib = eclib;
    m_SCluster = cluster;
    m_strPath = path;

    reloadDirectories();
    reloadFiles();
}

/**
 * Destructor
 */
ElfcloudDirCache::~ElfcloudDirCache(
)
{
}

shared_ptr<elfcloud::Cluster> ElfcloudDirCache::getCluster()
{
    return m_SCluster;
}

bool ElfcloudDirCache::isFile(string fileName)
{
    if( getFile(fileName) != 0x00)
    {
        return true;
    }

    return false;
}

bool ElfcloudDirCache::isDirectory(string cluster)
{

    if( getDirectory(cluster) != 0x00)
    {
        return true;
    }

    return false;
}

shared_ptr<elfcloud::DataItem> ElfcloudDirCache::getFile(string fileName)
{
    std::map<string, shared_ptr<elfcloud::DataItem>>::iterator l_SIMapterator;

    l_SIMapterator = m_SFiles.find(fileName);

    if( l_SIMapterator != m_SFiles.end() )
    {
        return (*l_SIMapterator).second;
    }

    return 0x00;
}

shared_ptr<elfcloud::Cluster> ElfcloudDirCache::getDirectory(string cluster)
{
    std::map<string, shared_ptr<elfcloud::Cluster>>::iterator l_SIMapterator;

    l_SIMapterator = m_SDirs.find(cluster);

    if( l_SIMapterator != m_SDirs.end() )
    {
        return (*l_SIMapterator).second;
    }

    return 0x00;
}

vector<string> ElfcloudDirCache::getFileNames()
{
    std::map<string, shared_ptr<elfcloud::DataItem>>::iterator l_SIMapterator;
    vector<string> l_SNames;

    for(l_SIMapterator = m_SFiles.begin(); l_SIMapterator != m_SFiles.end(); l_SIMapterator++)
    {
        l_SNames.push_back((*l_SIMapterator).first);
    }

    return l_SNames;
}

vector<string> ElfcloudDirCache::getDirectoryNames()
{
    std::map<string, shared_ptr<elfcloud::Cluster>>::iterator l_SIMapterator;
    vector<string> l_SNames;

    for(l_SIMapterator = m_SDirs.begin(); l_SIMapterator != m_SDirs.end(); l_SIMapterator++)
    {
        l_SNames.push_back((*l_SIMapterator).first);
    }

    return l_SNames;
}

bool ElfcloudDirCache::reloadDirectories()
{
    shared_ptr<elfcloud::Cluster> l_STmpCluster = 0x00;
    list < shared_ptr < elfcloud::Cluster >> *l_SListClusters = m_SCluster->listClusters();
    std::map<string, ElfcloudDirCache *>::iterator l_pCache;

    m_SDirs.clear();

    for (list<shared_ptr<elfcloud::Cluster>>::iterator iter = l_SListClusters->begin(); iter != l_SListClusters->end(); iter++)
    {
        l_STmpCluster = (*iter);
        m_SDirs.insert(std::pair<string, shared_ptr<elfcloud::Cluster>>(l_STmpCluster->getClusterName(), l_STmpCluster));
    }

    delete l_SListClusters;

    return true;
}

bool ElfcloudDirCache::reloadFiles()
{
    shared_ptr<elfcloud::DataItem> l_STmpDataitem = 0x00;
    list < shared_ptr < elfcloud::DataItem >> *l_SListDataItems =  m_SCluster->listDataItems();

    m_SFiles.clear();

    for (list<shared_ptr<elfcloud::DataItem>>::iterator iter = l_SListDataItems->begin(); iter != l_SListDataItems->end(); iter++)
    {
        l_STmpDataitem = (*iter);
        m_SFiles.insert(std::pair<string, shared_ptr<elfcloud::DataItem>>(l_STmpDataitem->getDataItemName(), l_STmpDataitem));
    }

    delete l_SListDataItems;
    return true;
}

bool ElfcloudDirCache::removeDirectory(string cluster)
{
    std::map<string, shared_ptr<elfcloud::Cluster>>::iterator l_SIMapterator;
    shared_ptr<elfcloud::Cluster> l_SCluster = getDirectory(cluster);

    if( l_SCluster == 0x00 )
    {
        return false;
    }

    l_SIMapterator = m_SDirs.find(cluster);

    if( l_SIMapterator != m_SDirs.end() )
    {
        m_SDirs.erase(l_SIMapterator);
        return true;
    }

    return false;
}

bool ElfcloudDirCache::removeFile(string fileName)
{
    std::map<string, shared_ptr<elfcloud::DataItem>>::iterator l_SIMapterator;
    shared_ptr<elfcloud::DataItem> l_SDataItem = getFile(fileName);

    if( l_SDataItem == 0x00 )
    {
        return false;
    }

    l_SIMapterator = m_SFiles.find(fileName);

    if( l_SIMapterator != m_SFiles.end() )
    {
        m_SFiles.erase(l_SIMapterator);
    }

    return false;
}

bool ElfcloudDirCache::addDirectory(shared_ptr<elfcloud::Cluster> cluster)
{

    removeDirectory(cluster->getClusterName());

    m_SDirs.insert(std::pair<string, shared_ptr<elfcloud::Cluster>>(cluster->getClusterName(), cluster));

    return true;
}

bool ElfcloudDirCache::addFile(shared_ptr<elfcloud::DataItem> dataitem)
{
    removeFile(dataitem->getDataItemName());

    m_SFiles.insert(std::pair<string, shared_ptr<elfcloud::DataItem>>(dataitem->getDataItemName(), dataitem));

    return true;
}

