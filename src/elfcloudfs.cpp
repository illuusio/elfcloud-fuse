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

#include "elfcloudfs.hh"


#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <map>

using namespace std;
using namespace elfcloud;

ElfcloudFS *ElfcloudFS::m_SInstance = NULL;

#define RETURN_ERRNO(x) (x) == 0 ? 0 : -errno

ElfcloudFS *ElfcloudFS::Instance()
{
    if(m_SInstance == NULL)
    {
        m_SInstance  = new ElfcloudFS();
    }

    return m_SInstance;
}

ElfcloudFS::ElfcloudFS()
{
    m_SEclib = 0x00;
    m_SVaults = 0x00;
    m_SCurrentVault = 0x00;
    m_lFh = 0;
}

ElfcloudFS::~ElfcloudFS()
{
    std::map<string, ElfcloudDirCache *>::iterator l_pCache;

    while(m_SDirs.size() > 0)
    {
        l_pCache = m_SDirs.begin();
        m_SDirs.erase(m_SDirs.begin());
    }

    m_SDirs.clear();
}

int ElfcloudFS::Getattr(const char *path, struct stat *statbuf)
{
    char *l_strSplitter = NULL;
    int l_iDeep = 0;
    char l_strVaultName[1024];
    vector<string> l_SPaths = getSplittedPath(path);
    ElfcloudDirCache *l_SDirCache = NULL;
    mode_t l_iFilePerm = 0;
    list<shared_ptr<elfcloud::Cluster>> *l_SClusters = NULL;
    list<shared_ptr<elfcloud::DataItem>> *l_SDataItems = NULL;
    shared_ptr<elfcloud::Cluster> l_SCluster = 0x00;
    shared_ptr<elfcloud::DataItem> l_SDataItem = 0x00;

    memset(l_strVaultName, 0x00, 1024);

    memset(statbuf, 0, sizeof(struct stat));

    statbuf->st_nlink = 0;
    statbuf->st_gid = 100;
    statbuf->st_uid = 1000;

    // We are at vault level..
    if(!strncmp("/", path, 1024))
    {
        statbuf->st_mode = S_IFDIR | 0700;
        time_t t2  = time (NULL);
        statbuf->st_atime = t2;
        statbuf->st_mtime = t2;
        statbuf->st_ctime = t2;
        return 0;
    }

    else if(l_SPaths.size() == 1)
    {
        shared_ptr<elfcloud::Vault> l_STmpVault = getVaultByName(l_SPaths[0].data());

        if(l_STmpVault != 0x00)
        {
            mode_t l_SPerm = getUnixPermissions(l_STmpVault->getPermissions());
            const char *l_StrTimestr = l_STmpVault->getLastAccessed().data();
            struct tm t;
            strptime(l_StrTimestr, "%Y-%m-%dT%H:%M:%S", &t);
            time_t t2 = mktime(&t);

            // Read this: http://sourceforge.net/mailarchive/message.php?msg_id=29281571
            // So st_nlink for directory means . and .. + how many subdirs there
            // is.
            statbuf->st_nlink = l_STmpVault->getDescendantCount() + 2;
            statbuf->st_mode = S_IFDIR | l_SPerm;

            if(l_SPerm & S_IRUSR)
            {
                statbuf->st_mode |= S_IXUSR;
            }

            statbuf->st_atime = t2;
            statbuf->st_mtime = t2;
            statbuf->st_ctime = t2;
            return 0;
        }

    }

    else if(l_SPaths.size() > 1)
    {
        string l_strTempPath;

        l_SDirCache = getClusterByPath(path);

        if(l_SDirCache != NULL)
        {

            if(!strncmp(l_SDirCache->getCluster()->getClusterName().data(), l_SPaths[l_SPaths.size() - 1].data(), 1024))
            {
                mode_t l_SPerm = getUnixPermissions(l_SDirCache->getCluster()->getPermissions());
                const char *l_StrTimestr = l_SDirCache->getCluster()->getLastAccessed().data();
                struct tm t;

                statbuf->st_mode = S_IFDIR | l_SPerm;

                // If one can read dir he/she can also
                // go to that dir
                if(l_SPerm & S_IRUSR)
                {
                    statbuf->st_mode |= S_IXUSR;
                }

                strptime(l_StrTimestr, "%Y-%m-%dT%H:%M:%S", &t);
                time_t t2 = mktime(&t);

                statbuf->st_nlink = l_SDirCache->getCluster()->getClusterDescendants() + 2;

                statbuf->st_atime = t2;
                statbuf->st_mtime = t2;
                statbuf->st_ctime = t2;
                return 0;
            }
        }

        for(int i = 0; i < l_SPaths.size() - 1; i++ )
        {
            l_strTempPath.append("/");
            l_strTempPath.append(l_SPaths[i]);
        }

        l_SDirCache = getClusterByPath(l_strTempPath.data());

        if(l_SDirCache == NULL)
        {
            return -ENOENT;
        }

        l_iFilePerm = getUnixPermissions(l_SDirCache->getCluster()->getPermissions());

        l_SCluster = l_SDirCache->getDirectory(string(l_SPaths[l_SPaths.size() - 1]));

        if(l_SCluster != 0x00)
        {
            mode_t l_SPerm = getUnixPermissions(l_SCluster->getPermissions());
            const char *l_StrTimestr = l_SCluster->getLastAccessed().data();
            struct tm t;

            statbuf->st_mode = S_IFDIR | l_SPerm;

            // If one can read dir he/she can also
            // go to that dir
            if(l_SPerm & S_IRUSR)
            {
                statbuf->st_mode |= S_IXUSR;
            }

            strptime(l_StrTimestr, "%Y-%m-%dT%H:%M:%S", &t);
            time_t t2 = mktime(&t);

            statbuf->st_nlink = l_SCluster->getClusterDescendants() + 2;

            statbuf->st_atime = t2;
            statbuf->st_mtime = t2;
            statbuf->st_ctime = t2;
            delete l_SDataItems;
            delete l_SClusters;
            return 0;
        }

        l_SDataItem = l_SDirCache->getFile(l_SPaths[l_SPaths.size() - 1]);

        if(l_SDataItem != 0x00)
        {
            const char *l_StrTimestr = l_SDataItem->getLastAccessed().data();
            struct tm t;
            strptime(l_StrTimestr, "%Y-%m-%dT%H:%M:%S", &t);
            time_t t2 = mktime(&t);

            statbuf->st_mode = S_IFREG | l_iFilePerm;
            statbuf->st_size = l_SDataItem->getDataLength();

            statbuf->st_size = l_SDataItem->getDataLength();

            statbuf->st_nlink = 1;

            statbuf->st_atime = t2;

            l_StrTimestr = l_SDataItem->getLastModified().data();
            strptime(l_StrTimestr, "%Y-%m-%dT%H:%M:%S", &t);
            t2 = mktime(&t);

            statbuf->st_mtime = t2;
            statbuf->st_ctime = t2;
            delete l_SDataItems;
            delete l_SClusters;
            return 0;
        }

    }

    return -ENOENT;
}

int ElfcloudFS::Readlink(const char *path, char *link, size_t size)
{
    if(!strcmp("/", path))
    {
        return -ENOENT;
    }


    return -1;
}

int ElfcloudFS::Mknod(const char *path, mode_t mode, dev_t dev)
{

    vector<string> l_SPaths = getSplittedPath(path);
    shared_ptr<elfcloud::Vault> l_SVault = getVaultByName(l_SPaths[0].data());
    shared_ptr<elfcloud::DataItem> l_SDataItem(new elfcloud::DataItem(m_SEclib));
    shared_ptr<elfcloud::Cluster> l_SCluster = 0x00;
    ElfcloudDirCache *l_SDirCache = NULL;

    try
    {
        l_SDataItem->setDataItemName("Defaultname.txt");
    }

    catch(elfcloud::IllegalParameterException &e)
    {
        cerr << "ElfcloudFS::Mknod: IllegalParameterException: " << e.getCode() << ", " << e.getMsg() << endl;
        return -ENOENT;

    }


    if(!strcmp("/", path) || l_SPaths.size() == 1)
    {
        return -ENOENT;
    }

    else if(l_SPaths.size() == 2)
    {
        l_SVault = getVaultByName(l_SPaths[0].data());

        if(l_SVault == 0x00)
        {
            return -ENOENT;
        }

        try
        {
            l_SDataItem->setDataItemName(l_SPaths[1]);
        }

        catch(elfcloud::IllegalParameterException &e)
        {
            cerr << "ElfcloudFS::Mknod: IllegalParameterException: " << e.getCode() << ", " << e.getMsg() << endl;
            return -ENOENT;

        }

        l_SDataItem->setDataPtr(NULL, 0);

        try
        {
            if(l_SVault->storeDataItem(l_SDataItem) == false)
            {
                cerr << "ElfcloudFS::mknod: Can't store dataitem" << endl;
                return -ENOENT;
            }
        }

        catch(elfcloud::Exception &e)
        {
            cerr << "ElfcloudFS::mknod: Exception: " << e.getCode() << ", " << e.getMsg() << endl;
            return -ENOENT;

        }

        return 0;
    }

    else if(l_SPaths.size() > 2)
    {
        string l_strTempPath;
        byte *l_SData = NULL;

        for(int i = 0; i < l_SPaths.size() - 1; i++ )
        {
            l_strTempPath.append("/");
            l_strTempPath.append(l_SPaths[i]);
        }


        l_SDirCache = getClusterByPath(l_strTempPath.data());

        if(l_SDirCache == NULL)
        {
            cerr << "ElfcloudFS::mknod: Can't find cluster" << endl;
            return -ENOENT;
        }

        l_SData = (byte *)malloc(1);
        l_SData[0] = 0x00;

        try
        {
            l_SDataItem->setDataItemName(l_SPaths[l_SPaths.size() - 1]);
        }

        catch(elfcloud::IllegalParameterException &e)
        {
            free(l_SData);
            cerr << "ElfcloudFS::Mknod: IllegalParameterException: " << e.getCode() << ", " << e.getMsg() << endl;
            return -ENOENT;

        }

        l_SDataItem->setDataPtr(l_SData, 1);

        try
        {
            if(l_SDirCache->getCluster()->storeDataItem(l_SDataItem) == false)
            {
                cerr << "ElfcloudFS::mknod: Can't store item" << endl;
                return -ENOENT;
            }
        }

        catch(elfcloud::Exception &e)
        {
            cerr << "ElfcloudFS::mknod: Exception: " << e.getCode() << ", " << e.getMsg() << endl;
            return -ENOENT;

        }

        l_SDirCache->addFile(l_SDataItem);

        return 0;
    }

    return -ENOENT;
}

int ElfcloudFS::Mkdir(const char *path, mode_t mode)
{
    vector<string> l_SPaths = getSplittedPath(path);
    shared_ptr<elfcloud::Vault> l_SVault = 0x00;
    shared_ptr<Cluster> l_SNewCluster(new Cluster(m_SEclib));
    ElfcloudDirCache *l_SDirCache = NULL;


    if(!strcmp("/", path))
    {
        // This can't be happening
        return -ENOENT;
    }

    else if(l_SPaths.size() == 1)
    {
        m_SEclib->addVault(l_SPaths[0], "fi.elfcloud.datastore");
        m_SVaults = Vault::ListVaults(m_SEclib);
        return 0;
    }

    else if(l_SPaths.size() == 2)
    {
        l_SVault = getVaultByName(l_SPaths[0].data());

        if(l_SVault == 0x00)
        {
            return -ENOENT;
        }

        l_SNewCluster->setClusterName(l_SPaths[1]);

        if(l_SVault->addClusterToServer(l_SNewCluster) == false)
        {
            cerr <<  "ElfcloudFS::Mkdir Can't create dir: " << l_SNewCluster->getClusterName() << "!" << endl;
            return -ENOENT;
        }

        return 0;
    }

    else if(l_SPaths.size() > 1)
    {
        string l_strTempPath;
        l_SVault = getVaultByName(l_SPaths[0].data());


        for(int i = 0; i < l_SPaths.size() - 1; i++ )
        {
            l_strTempPath.append("/");
            l_strTempPath.append(l_SPaths[i]);
        }

        l_SDirCache = getClusterByPath(l_strTempPath.data());

        if(l_SDirCache == NULL)
        {
            return -ENOENT;
        }

        l_SNewCluster->setClusterName(l_SPaths[l_SPaths.size() - 1]);
        l_SNewCluster->setClusterParentId(l_SDirCache->getCluster()->getClusterId());

        if(l_SDirCache->getCluster()->addClusterToServer(l_SNewCluster) == false)
        {
            cerr <<  "ElfcloudFS::Mkdir Can't create dir: " << l_SNewCluster->getClusterName() << "!" << endl;
            return -ENOENT;
        }

        l_SDirCache->addDirectory(l_SNewCluster);

        return 0;
    }

    return -ENOENT;
}

int ElfcloudFS::Unlink(const char *path)
{
    vector<string> l_SPaths = getSplittedPath(path);
    ElfcloudDirCache *l_SDirCache = getClusterByPath(path);
    shared_ptr<elfcloud::DataItem> l_SDataItem = 0x00;

    if(!strncmp( path, "/", 1024 ))
    {
        return -ENOENT;
    }

    if(l_SDirCache == NULL)
    {
        cerr << "ElfcloudFS::unlink: Can't unlink" << endl;
        return -ENOENT;
    }

    l_SDataItem = l_SDirCache->getFile(l_SPaths[l_SPaths.size() - 1]);

    if(l_SDataItem != 0x00)
    {
        try
        {
            if(l_SDirCache->getCluster()->removeDataItem(l_SDataItem) == false)
            {
                cerr << "ElfcloudFS::unlink: Can't unlink" << endl;
                return -ENOENT;
            }
        }

        catch(elfcloud::Exception &e)
        {
            cerr << "ElfcloudFS::GetClusterByName: Exception: " << e.getCode() << ", " << e.getMsg() << endl;
            return -ENOENT;
        }

        l_SDirCache->removeFile(l_SPaths[l_SPaths.size() - 1]);
        return 0;
    }

    return -ENOENT;
}

int ElfcloudFS::Rmdir(const char *path)
{
    vector<string> l_SPaths = getSplittedPath(path);
    shared_ptr<elfcloud::Vault> l_SVault = 0x00;
    shared_ptr<elfcloud::Cluster> l_SCluster = 0x00;
    std::map<string, ElfcloudDirCache *>::iterator l_SIMapterator;
    ElfcloudDirCache *l_SDirCache = NULL;
    string l_strTempPath;

    if(!strncmp( path, "/", 1024 ))
    {
        return -ENOENT;
    }

    if(l_SPaths.size() == 1)
    {
        l_SVault = getVaultByName(l_SPaths[0].data());

        if(l_SVault == 0x00)
        {
            cerr << "ElfcloudFS::rmdir: Can't find vault" << endl;
            return -ENOENT;
        }

        l_SVault->remove();
        m_SVaults = Vault::ListVaults(m_SEclib);
    }

    else
    {
        l_SDirCache = getClusterByPath(path);

        l_SIMapterator = m_SDirs.find(string(path));

        if(l_SDirCache == NULL)
        {
            cerr << "ElfcloudFS::rmdir: Can't find cluster" << endl;
            return -ENOENT;
        }

        if(l_SDirCache->getCluster()->removeCluster() == false)
        {
            cerr << "ElfcloudFS::rmdir: Can't remove cluster" << endl;
            return -ENOENT;
        }

        if(l_SIMapterator != m_SDirs.end())
        {
            m_SDirs.erase(l_SIMapterator);
        }

        for(int i = 0; i < l_SPaths.size() - 1; i++ )
        {
            l_strTempPath.append("/");
            l_strTempPath.append(l_SPaths[i]);
        }

        l_SDirCache = getClusterByPath(l_strTempPath.data());

        if(l_SDirCache != NULL)
        {
            l_SDirCache->removeDirectory(l_SPaths[l_SPaths.size() - 1]);
        }

    }

    return 0;
}

int ElfcloudFS::Symlink(const char *path, const char *link)
{
    printf("symlink(path=%s, link=%s)\n", path, link);
    return -1;
}

int ElfcloudFS::Rename(const char *path, const char *newpath)
{
    vector<string> l_SPaths = getSplittedPath(path);
    vector<string> l_SPathsNew = getSplittedPath(newpath);
    ElfcloudDirCache *l_SDirCache = getClusterByPath(path);
    shared_ptr<elfcloud::DataItem> l_SDataItem = 0x00;

    if(!strncmp( path, "/", 1024 ))
    {
        return -ENOENT;
    }

    if(l_SPaths.size() < 2)
    {
        return -ENOENT;
    }

    if(l_SPathsNew.size() < 2)
    {
        return -ENOENT;
    }

    if(l_SDirCache == NULL)
    {
        cerr << "ElfcloudFS::Rename: Can't find cluster!!";
        return -ENOENT;
    }

    l_SDataItem = l_SDirCache->getFile(l_SPaths[l_SPaths.size() - 1]);

    if(l_SDataItem != 0x00)
    {
        try
        {
            l_SDataItem->setDataItemName(l_SPathsNew[l_SPathsNew.size() - 1]);
        }

        catch(elfcloud::Exception &e)
        {
            cerr << "ElfcloudFS::Rename Caught " << e.getCode() << ", " + e.getMsg() + ", ignoring" << endl;
            return -ENOENT;
        }

        l_SDirCache->reloadFiles();
        return 0;
    }

    return -ENOENT;
}

int ElfcloudFS::Link(const char *path, const char *newpath)
{
    printf("link(path=%s, newPath=%s)\n", path, newpath);
    return -1;
}

int ElfcloudFS::Chmod(const char *path, mode_t mode)
{
    printf("chmod(path=%s, mode=%d)\n", path, mode);
    return -1;
}

int ElfcloudFS::Chown(const char *path, uid_t uid, gid_t gid)
{
    return -1;
}

int ElfcloudFS::Truncate(const char *path, off_t newSize)
{
    printf("truncate(path=%s, newSize=%d\n", path, (int)newSize);
    return 0;
}

int ElfcloudFS::Utime(const char *path, struct utimbuf *ubuf)
{
    vector<string> l_SPaths = getSplittedPath(path);
    shared_ptr<elfcloud::DataItem> l_SDataItem = 0x00;
    struct tm *l_STime = localtime(&ubuf->actime);
    char l_strAccTime[48];
    char l_strModTime[48];
    ElfcloudDirCache *l_SDirCache = NULL;

    memset(l_strAccTime, 0x00, 48);
    memset(l_strModTime, 0x00, 48);

    // 12-12-12T12:12:12.12345+0003
    // Hopefully in some point ill get better way before 2100
    snprintf(l_strAccTime, 48, "%04d-%02d-%02d-T-%02d:%02d:%02d.000000+00:00",
             (1900 + l_STime->tm_year),
             l_STime->tm_mon,
             l_STime->tm_mday,
             l_STime->tm_hour,
             l_STime->tm_min,
             l_STime->tm_sec);

    l_STime = localtime(&ubuf->modtime);

    snprintf(l_strModTime, 48, "%02d-%02d-%02d-T-%02d:%02d:%02d.0000+00:00",
             (1900 + l_STime->tm_year),
             l_STime->tm_mon,
             l_STime->tm_mday,
             l_STime->tm_hour,
             l_STime->tm_min,
             l_STime->tm_sec);


    if(!strncmp( path, "/", 1024 ))
    {
        return -ENOENT;
    }

    l_SDirCache = getClusterByPath(path);

    if(l_SDirCache == NULL)
    {
        cerr << "ElfcloudFS::Utime: Can't find cluster!!";
        return -ENOENT;
    }

    l_SDataItem = l_SDirCache->getFile(l_SPaths[l_SPaths.size() - 1]);

    if(l_SDataItem != 0x00)
    {
        l_SDataItem->setLastModified(l_strModTime);
        l_SDataItem->setLastAccessed(l_strAccTime);
        return 0;
    }

    else
    {
        l_SDirCache->getCluster()->setLastModified(l_strModTime);
        l_SDirCache->getCluster()->setLastAccessed(l_strAccTime);
        return 0;
    }

    return -ENOENT;
}

int ElfcloudFS::Open(const char *path, struct fuse_file_info *fileInfo)
{
    vector<string> l_SPaths = getSplittedPath(path);
    shared_ptr<elfcloud::DataItem> l_SDataItem = 0x00;
    shared_ptr<elfcloud::DataItemFilePassthrough> m_SFile(new DataItemFilePassthrough(m_SEclib));
    FILE *l_SOpenFile = NULL;
    std::stringstream l_strSs;
    string l_strCacheFile;
    ElfcloudFSCache *l_SCacheItem = getOpenFileCacheByPath(path, fileInfo->fh);
    ElfcloudDirCache *l_SDirCache = NULL;

    l_SDirCache = getClusterByPath(path);

    if(l_SDirCache == NULL)
    {
        cerr << "ElfcloudFS::Open Can't find file" << endl;
        return -ENOENT;
    }

    l_SDataItem = l_SDirCache->getFile(l_SPaths[l_SPaths.size() - 1]);


    if(l_SDataItem == 0x00)
    {
        cerr << "ElfcloudFS::Open Can't find directory" << endl;
        return -ENOENT;
    }

    if(l_SCacheItem == NULL)
    {
        l_strCacheFile = getOpenFileCacheWholePathByPath(path, m_lFh);


        l_SCacheItem =  new ElfcloudFSCache(
            m_SEclib,
            l_SPaths[l_SPaths.size() - 1],
            l_strCacheFile,
            l_SDirCache->getCluster(),
            m_lFh
        );


        if(fileInfo->flags & O_RDWR || (fileInfo->flags & O_WRONLY) == 0 || (fileInfo->flags & O_APPEND))
        {
            l_SCacheItem->fetchItemToCache();
        }

        else
        {
            l_SCacheItem->createItemToCache();
        }


        l_strSs << path;

        m_SOpenFile.insert(std::pair<string, ElfcloudFSCache *>(l_strSs.str(), l_SCacheItem));
    }

    else
    {
        l_SCacheItem->openFile();
    }

    fileInfo->fh = m_lFh ++;

    return 0;
}

int ElfcloudFS::Read(const char *path, char *buf, size_t size, off_t offset, struct fuse_file_info *fileInfo)
{
    size_t l_iReaded = 0;
    ElfcloudFSCache *l_SCacheItem = getOpenFileCacheByPath(path, fileInfo->fh);

    if(l_SCacheItem == NULL || l_SCacheItem->getFile() == NULL)
    {
        cerr << "ElfcloudFS::Read: File or CacheItem not found: " << path << endl;
        return -1;
    }

    if(fseek(l_SCacheItem->getFile(), offset, SEEK_SET) < 0)
    {
        cerr << "ElfcloudFS::Read: can't seek to offset!" << endl;
        return -1;
    }

    l_iReaded = fread(buf, 1, size, l_SCacheItem->getFile());

    if(l_iReaded == 0)
    {
        cerr << "ElfcloudFS::Read: Read bytes: " << l_iReaded << " wanted read:" << size << endl;
        return -1;
    }

    return size;
}

int ElfcloudFS::Write(const char *path, const char *buf, size_t size, off_t offset, struct fuse_file_info *fileInfo)
{
    size_t l_iWritten = 0;
    ElfcloudFSCache *l_SCacheItem = getOpenFileCacheByPath(path, fileInfo->fh);

    if(l_SCacheItem == NULL || l_SCacheItem->getFile() == NULL)
    {
        cerr << "ElfcloudFS::Write: File or CacheItem not found: " << path << endl;
        return -1;
    }

    // Probably we just made this file up so it's safe to
    // set offset to 1
    if((fileInfo->flags & O_APPEND) && offset == 1)
    {
        size_t l_iAppenLen = 0;
        offset = 0;
    }

    if(fseek(l_SCacheItem->getFile(), offset, SEEK_SET) < 0)
    {
        cerr << "ElfcloudFS::Write: can't seek to offset!" << endl;
        return -1;
    }

    l_iWritten = fwrite(buf, 1, size, l_SCacheItem->getFile());

    if(l_iWritten == 0)
    {
        cerr << "ElfcloudFS::Write: Write bytes: " << l_iWritten << " wanted write:" << size << endl;
        return -1;
    }

    return l_iWritten;
}

int ElfcloudFS::Flush(const char *path, struct fuse_file_info *fileInfo)
{
    ElfcloudFSCache *l_SCacheItem = getOpenFileCacheByPath(path, fileInfo->fh);

    if(l_SCacheItem != NULL || l_SCacheItem->getFile() != NULL)
    {
        fflush(l_SCacheItem->getFile());
    }

    return 0;
}

int ElfcloudFS::Release(const char *path, struct fuse_file_info *fileInfo)
{
    vector<string> l_SPaths = getSplittedPath(path);
    std::pair<shared_ptr<elfcloud::Cluster>, shared_ptr<elfcloud::DataItem>> l_SPair;
    shared_ptr<elfcloud::Cluster> l_SCluster = 0x00;
    shared_ptr<elfcloud::DataItem> l_SDataItem = 0x00;
    shared_ptr<elfcloud::DataItemFilePassthrough> m_SFile(new DataItemFilePassthrough(m_SEclib));
    ElfcloudFSCache *l_SCacheItem = getOpenFileCacheByPath(path, fileInfo->fh);
    std::map<string, ElfcloudFSCache *>::iterator l_SIMapterator;
    string l_strCacheFile;
    ElfcloudDirCache *l_SDirCache = getClusterByPath(path);

    if(l_SCacheItem == NULL)
    {
        cerr << "There is no " << path << " open but I'll emit it is" << endl;
        return 0;
    }

    if(fileInfo->flags & O_RDWR || fileInfo->flags & O_WRONLY)
    {

        if(l_SCacheItem->storeItemToCloud() == false)
        {
            cerr << "ElfcloudFS::release: Can't store item to cloud" << endl;
            return -ENOENT;
        }

        if(l_SDirCache != NULL)
        {
            l_SDirCache->reloadFiles();
        }

    }

    if(l_SCacheItem->closeFile() <= 0)
    {

        l_SCacheItem->removeItemFromCache();
        l_SIMapterator = m_SOpenFile.find(string(path));

        if(l_SIMapterator != m_SOpenFile.end())
        {
            m_SOpenFile.erase(l_SIMapterator);
        }

        delete l_SCacheItem;

    }


    fileInfo->fh = -1;

    return 0;
}

int ElfcloudFS::Statfs(const char *path, struct statvfs *statInfo)
{
    statInfo->f_namemax = 40;
    statInfo->f_bsize = 4096;
    statInfo->f_frsize = statInfo->f_bsize;
    statInfo->f_blocks = statInfo->f_bfree = statInfo->f_bavail =
                             1000ULL * 1024 * 1024 * 1024 / statInfo->f_frsize;
    statInfo->f_files = statInfo->f_ffree = 1000000000;
    return 0;
}

int ElfcloudFS::Fsync(const char *path, int datasync, struct fuse_file_info *fi)
{
    printf("fsync(path=%s, datasync=%d\n", path, datasync);
    return -1;

}

int ElfcloudFS::Setxattr(const char *path, const char *name, const char *value, size_t size, int flags)
{
    printf("setxattr(path=%s, name=%s, value=%s, size=%d, flags=%d\n",
           path, name, value, (int)size, flags);
    return -1;
}

int ElfcloudFS::Getxattr(const char *path, const char *name, char *value, size_t size)
{
    printf("getxattr(path=%s, name=%s, size=%d\n", path, name, (int)size);
    return -1;
}

int ElfcloudFS::Listxattr(const char *path, char *list, size_t size)
{
    printf("listxattr(path=%s, size=%d)\n", path, (int)size);
    return -1;
}

int ElfcloudFS::Removexattr(const char *path, const char *name)
{
    printf("removexattry(path=%s, name=%s)\n", path, name);
    return -1;
}

int ElfcloudFS::Opendir(const char *path, struct fuse_file_info *fileInfo)
{
    return 0;
}

int ElfcloudFS::Readdir(const char *path, void *buf, fuse_fill_dir_t filler, off_t offset, struct fuse_file_info *fileInfo)
{
    (void) offset;
    (void) fileInfo;
    vector<string> l_SPaths = getSplittedPath(path);
    ElfcloudDirCache *l_SDirCache = NULL;
    vector<string> l_SNames;

    const char l_strCurrent[2] = {'.', 0x00};
    const char l_strParent[3] = {'.', '.', 0x00};

    filler(buf, l_strCurrent, NULL, 0);
    filler(buf, l_strParent, NULL, 0);

    if(!strcmp("/", path))
    {
        for(list<shared_ptr<Vault>>::iterator iter = m_SVaults->begin(); iter != m_SVaults->end(); iter++)
        {
            shared_ptr <Vault> l_STmpVault = *(iter);
            filler(buf, l_STmpVault->getName().data(), NULL, 0);
        }
    }

    else if(l_SPaths.size() == 1)
    {
        // We are at vault root level
        shared_ptr<Vault> l_STmpVault = getVaultByName(l_SPaths[0].data());
        std::map<std::string, std::list<shared_ptr<elfcloud::Object>>*> *mapContents = NULL;

        try
        {
            mapContents = l_STmpVault->listContents();
        }

        catch(elfcloud::Exception &e)
        {
            cerr << "ElfcloudFS::readdir: Exception: " << e.getCode() << ", " << e.getMsg() << endl;
            return -ENOENT;
        }

        for(std::map<std::string, std::list<shared_ptr<elfcloud::Object>>*>::iterator mapiter = mapContents->begin(); mapiter != mapContents->end(); mapiter++)
        {
            std::string l_strObjName = (*mapiter).first;
            list<shared_ptr<elfcloud::Object>> *l_SObject = (*mapiter).second;
            elfcloud::Cluster *l_SCurCluster = NULL;

            if(!l_strObjName.compare("dataitems"))
            {
                list<elfcloud::DataItem *> *l_SDataItems = (list<elfcloud::DataItem *> *) l_SObject;

                for(list<elfcloud::DataItem *>::iterator iter = l_SDataItems->begin(); iter != l_SDataItems->end(); iter++)
                {
                    elfcloud::DataItem *l_SDataItem = (*iter);
                    filler(buf, l_SDataItem->getDataItemName().data(), NULL, 0);
                }
            }

            else if(!l_strObjName.compare("clusters"))
            {
                list<shared_ptr<elfcloud::Cluster>> *l_SClusters = (list<shared_ptr<elfcloud::Cluster>> *) l_SObject;
                int u = 0;


                for(list<shared_ptr<elfcloud::Cluster>>::iterator iter = l_SClusters->begin(); iter != l_SClusters->end(); iter++)
                {
                    shared_ptr<elfcloud::Cluster> l_SCluster = (*iter);
                    filler(buf, l_SCluster->getClusterName().data(), NULL, 0);
                }
            }

        }

        delete mapContents;
    }

    else
    {
        shared_ptr<elfcloud::Cluster> l_SCurCluster = 0x00;
        list<shared_ptr<elfcloud::Cluster>> *l_SClusters = NULL;
        list<shared_ptr<elfcloud::DataItem>> *l_SDataItems = NULL;

        l_SDirCache = getClusterByPath(path);

        if(l_SDirCache == NULL)
        {
            return -ENOENT;
        }

        l_SNames = l_SDirCache->getDirectoryNames();

        for(int i = 0; i < l_SNames.size(); i++ )
        {
            filler(buf, l_SNames[i].data(), NULL, 0);

        }

        l_SNames = l_SDirCache->getFileNames();

        for(int i = 0; i < l_SNames.size(); i++ )
        {
            filler(buf, l_SNames[i].data(), NULL, 0);

        }

    }

    return 0;
}

int ElfcloudFS::Releasedir(const char *path, struct fuse_file_info *fileInfo)
{
    return -1;
}

int ElfcloudFS::Fsyncdir(const char *path, int datasync, struct fuse_file_info *fileInfo)
{
    printf("Fsyncdir %s\n", path);
    return -1;
}

int ElfcloudFS::Init(struct fuse_conn_info *conn)
{
    return -1;
}

int ElfcloudFS::Truncate(const char *path, off_t offset, struct fuse_file_info *fileInfo)
{
    printf("truncate(path=%s, offset=%d)\n", path, (int)offset);
    return -1;
}

int ElfcloudFS::createElfcloudClient(char *configpath)
{
    struct stat buf;

    if(m_SEclib != NULL)
    {
        cerr << "** Elfcloud-fuse there is problem:" << endl;
        cerr << "   Remove client first!" << endl;
        return -1;
    }

    if(stat(configpath, &buf) < 0)
    {
        cerr << "** Elfcloud-fuse there is problem:" << endl;
        cerr << "Can't locate config file: (" << configpath << ")" << endl;
        return -1;

    }

    try
    {
        m_SEclib = new elfcloud::Client();
        m_SEclib->readUserConfig(configpath);
    }

    catch(elfcloud::Exception &e)
    {
        cerr << "** Elfcloud-fuse there is problem:" << endl;
        cerr << "   Exception: " << e.getCode() << ", " << e.getMsg() << endl;
        return -1;
    }

    return 0;
}

int ElfcloudFS::Connect(char *username, char *password, long upspeed, long downspeed)
{
    char l_strSpeed[32];

    memset(l_strSpeed, 0x00, 32);

    if(m_SEclib == NULL)
    {
        cerr << "** Elfcloud-fuse there is problem:" << endl;
        cerr << "   Client not initalized!" << endl;
        return -1;
    }

    try
    {
        m_SEclib->setPasswordAuthenticationCredentials(username, password);

        if(downspeed > 0)
        {
            snprintf(l_strSpeed, 32, "%ld", downspeed);
            m_SEclib->setConf("http.limit.receive.Bps", l_strSpeed);
            memset(l_strSpeed, 0x00, 32);
        }

        if(upspeed > 0)
        {
            snprintf(l_strSpeed, 32, "%ld", upspeed);
            m_SEclib->setConf("http.limit.send.Bps", l_strSpeed);
            memset(l_strSpeed, 0x00, 32);
        }

        m_SVaults = Vault::ListVaults(m_SEclib);
    }

    catch(elfcloud::Exception &e)
    {
        cerr << "** Elfcloud-fuse there is problem:" << endl;
        cerr << "   Exception: " << e.getCode() << ", " << e.getMsg() << endl;
        return -1;
    }

    return 0;
}

int ElfcloudFS::Disconnect()
{
    if(m_SEclib == NULL)
    {
        cerr << "** Elfcloud-fuse there is problem:" << endl;
        cerr << "   Client not initalized!" << endl;
        return -1;
    }

    m_SEclib->clearCache();
    delete m_SVaults;
    delete m_SEclib;
    m_SEclib = NULL;
    return 0;
}

// Private
shared_ptr<elfcloud::Vault> ElfcloudFS::getVaultByName(const char *name)
{
    shared_ptr<Vault> l_STmpVault = 0x00;

    if(name == NULL)
    {
        cerr << "ElfcloudFS::getVaultByName: Vault name was NULL.. abort" << endl;
        return 0x00;
    }

    for(list<shared_ptr<Vault>>::iterator iter = m_SVaults->begin(); iter != m_SVaults->end(); iter++)
    {
        l_STmpVault = *(iter);

        if(!strncmp(l_STmpVault->getName().data(), name, 1024))
        {
            return l_STmpVault;
        }
    }

    return 0x00;
}

vector<string> ElfcloudFS::getSplittedPath(const char *path)
{
    char *l_strSplitter = NULL;
    char l_strPath[1024];
    vector<string> l_SVector;

    memset(l_strPath, 0x00, 1024);
    strncpy(l_strPath, path, 1024);

    l_strSplitter = strtok ((char *)l_strPath, "/");

    while (l_strSplitter != NULL)
    {
        l_SVector.push_back(string(l_strSplitter));
        l_strSplitter = strtok (NULL, "/");
    }

    return l_SVector;
}

ElfcloudDirCache *ElfcloudFS::getClusterByPath(const char *path)
{
    vector<string> l_SPaths = getSplittedPath(path);
    std::map<string, ElfcloudDirCache *>::iterator l_SIMapterator;
    shared_ptr<elfcloud::Vault> l_STmpVault  = 0x00;
    int l_iDepth = 1;
    std::string l_strObjName;
    list<shared_ptr<elfcloud::Object>> *l_SObject = NULL;
    shared_ptr<elfcloud::Cluster> l_SCurCluster = 0x00;
    list<shared_ptr<elfcloud::Cluster>> *l_SClusters = NULL;
    shared_ptr<elfcloud::Cluster> l_SCluster = 0x00;
    shared_ptr<elfcloud::Cluster> l_SIterCluster;
    shared_ptr<elfcloud::Cluster> l_SOldIterCluster;
    int u = 0;
    ElfcloudDirCache *l_SDirCache = NULL;
    string l_strTempPath = "";
    std::map<std::string, std::list<shared_ptr<elfcloud::Object>>*> *l_SMapContents = NULL;

    if(l_SPaths.size() == 0)
    {
        return NULL;
    }

    l_STmpVault = getVaultByName(l_SPaths[0].data());

    // Is there that named Vault?
    if(l_STmpVault == 0x00)
    {
        return NULL;
    }

    // Seek for path from memory map
    l_SIMapterator = m_SDirs.find(string(path));

    // Do we already have path mapped in memory
    // If we do then return it.
    if(l_SIMapterator != m_SDirs.end())
    {
        return (*l_SIMapterator).second;
    }

    for(int i = 0; i < l_SPaths.size() - 1; i++ )
    {
        l_strTempPath.append("/");
        l_strTempPath.append(l_SPaths[i]);
    }

    l_SIMapterator = m_SDirs.find(l_strTempPath);

    if(l_SIMapterator != m_SDirs.end())
    {
        l_SDirCache = (*l_SIMapterator).second;

        if(l_SDirCache->isFile(l_SPaths[l_SPaths.size() - 1]) == true)
        {
            return l_SDirCache;
        }
    }

    try
    {
        l_SMapContents = l_STmpVault->listContents();
    }

    catch(elfcloud::Exception &e)
    {
        cerr << "*ElfcloudFS::getClusterByPath: Exception: " << e.getCode() << ", " << e.getMsg() << endl;
        return 0x00;
    }

    l_SDirCache = NULL;

    // We have to start seeking down DOM tree. If path is /some/path/here
    // We know there should be Vault (some) and Clusters (path) and (here)
    for(std::map<std::string, std::list<shared_ptr<elfcloud::Object>>*>::iterator mapiter = l_SMapContents->begin(); mapiter != l_SMapContents->end(); mapiter++)
    {
        if(mapiter == l_SMapContents->end())
        {
            cerr << "ElfcloudFS::getClusterByPath: Something strange just happend with dir: " << l_STmpVault->getName() << "! trying to cope with it" << endl;
            continue;
        }

        l_strObjName = (*mapiter).first;
        l_SObject = (*mapiter).second;
        l_SCurCluster = 0x00;

        // If it's Cluster then we should check It's what we are seeking for?
        if(!l_strObjName.compare("clusters"))
        {
            l_SClusters = (list<shared_ptr<elfcloud::Cluster>> *) l_SObject;
            u = 0;

            // Then start to run down Clusters
            for(list<shared_ptr<elfcloud::Cluster>>::iterator iter = l_SClusters->begin(); iter != l_SClusters->end(); iter++)
            {
                l_SCluster = (*iter);

                if(l_SPaths.size() == 1)
                {
                    // delete l_SObject;
                    delete l_SMapContents;

                    if(l_SCluster != 0x00)
                    {
                        try
                        {
                            l_SDirCache = new ElfcloudDirCache(m_SEclib, path, l_SCluster);
                            m_SDirs.insert(std::pair<string, ElfcloudDirCache *>(string(path), l_SDirCache));
                        }

                        catch(elfcloud::Exception &e)
                        {
                            cerr << "ElfcloudFS::Readdir: Exception: " << e.getCode() << ", " << e.getMsg() << endl;
                            return NULL;
                        }
                    }

                    return l_SDirCache;
                }

                // We know that this is it.. then we start digging futher
                if(!l_SCluster->getClusterName().compare(l_SPaths[1]))
                {
                    l_SIterCluster = l_SCluster;

                    // Loop.. until we are deep enough
                    for(int i = 2; i < l_SPaths.size(); i ++)
                    {
                        l_SOldIterCluster = l_SIterCluster;
                        l_SIterCluster = getClusterByName(l_SIterCluster, l_SPaths[i].data());

                    }

                    // Map this one to path
                    if(l_SIterCluster != 0x00)
                    {

                        try
                        {
                            l_SDirCache = new ElfcloudDirCache(m_SEclib, path, l_SIterCluster);
                            m_SDirs.insert(std::pair<string, ElfcloudDirCache *>(string(path), l_SDirCache));
                        }

                        catch(elfcloud::Exception &e)
                        {
                            cerr << "ElfcloudFS::Readdir: Exception: " << e.getCode() << ", " << e.getMsg() << endl;
                            return NULL;
                        }

                    }

                    delete l_SMapContents;
                    return l_SDirCache;
                }
            }
        }

    }

    delete l_SMapContents;

    return 0x00;
}

shared_ptr<elfcloud::Cluster> ElfcloudFS::getClusterByName(shared_ptr<elfcloud::Cluster> cluster, const char *name)
{
    list<shared_ptr<elfcloud::Cluster>> *l_SClusters = 0x00;
    shared_ptr<elfcloud::Cluster> l_SItem = 0x00;

    if(cluster == 0x00 || name == NULL)
    {
        return 0x00;
    }

    try
    {
        l_SClusters = cluster->listClusters();
    }

    catch(elfcloud::Exception &e)
    {
        cerr << "ElfcloudFS::GetClusterByName: Exception: " << e.getCode() << ", " << e.getMsg() << endl;
        return 0x00;
    }


    // Try to find wanted cluster by name that is provided
    for(list<shared_ptr<elfcloud::Cluster>>::iterator iter = l_SClusters->begin(); iter != l_SClusters->end(); iter++)
    {
        l_SItem = (*iter);

        if(!strncmp(l_SItem->getClusterName().data(), name, 1024))
        {
            delete l_SClusters;
            return l_SItem;
        }
    }

    delete l_SClusters;
    return 0x00;
}



mode_t ElfcloudFS::getUnixPermissions(vector<string> permissions)
{
    mode_t l_iPermission = 0;

    for(int i = 0; i < permissions.size(); i++)
    {
        // http://unix.stackexchange.com/questions/39710/how-to-get-permission-number-by-string-rw-r-r

        if(!permissions[i].compare("write"))
        {
            l_iPermission |= S_IWUSR;

        } if(!permissions[i].compare("read"))

        {
            l_iPermission |= S_IRUSR;
        }
    }

    // User have no permissions
    return l_iPermission;
}

ElfcloudFSCache *ElfcloudFS::getOpenFileCacheByPath(const char *path, uint64_t fh)
{

    std::map<string, ElfcloudFSCache *>::iterator l_SIMapterator;
    std::stringstream l_strSs;

    l_strSs << path;

    // Seek for path from memory map
    l_SIMapterator = m_SOpenFile.find(l_strSs.str());

    // Do we already have path mapped in memory
    // If we do then return it.
    if(l_SIMapterator != m_SOpenFile.end())
    {
        return (ElfcloudFSCache *)l_SIMapterator->second;
    }

    return NULL;
}

string ElfcloudFS::getOpenFileCacheWholePathByPath(const char *path, uint64_t fh)
{
    std::stringstream l_strSs;

    l_strSs << getenv("HOME") << "/.elfcloud/elfcloudcache_";
    l_strSs << getCacheFileNameByPath(path, fh);

    return l_strSs.str();
}


string ElfcloudFS::getCacheFileNameByPath(const char *path, uint64_t fh)
{
    CryptoPP::Whirlpool l_SWhirlpool;
    byte l_cWhirpoolBytes[l_SWhirlpool.DigestSize()];
    CryptoPP::HexEncoder l_SEncoderWhirpool;
    std::string l_strOutputWhirlpool;
    std::stringstream l_strSs;

    if(path == NULL)
    {
        return string("");
    }

#ifdef strnlen
    l_SWhirlpool.CalculateDigest(l_cWhirpoolBytes, (const byte *)path, strnlen(path, 1024));
#else
    l_SWhirlpool.CalculateDigest(l_cWhirpoolBytes, (const byte *)path, strlen(path));
#endif

    l_SEncoderWhirpool.Attach( new CryptoPP::StringSink( l_strOutputWhirlpool ) );
    l_SEncoderWhirpool.Put( l_cWhirpoolBytes, sizeof(l_cWhirpoolBytes) );
    l_SEncoderWhirpool.MessageEnd();

    l_strSs << l_strOutputWhirlpool << "." << fh;

    return l_strSs.str();
}


