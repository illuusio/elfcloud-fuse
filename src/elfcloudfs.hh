
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

#ifndef _ELFCLOUDFS_H_
#define _ELFCLOUDFS_H_

#include "elfcloudfs-cache.hh"
#include "elfcloudfs-dircache.hh"

#include <ctype.h>
#include <sstream>
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

#include <API.h>

using namespace std;
using namespace elfcloud;

/**
 *  Main class for ElfcloudFS
 */
class ElfcloudFS
{
private:
    shared_ptr <elfcloud::Vault> m_SCurrentVault;
    Client *m_SEclib;
    list <shared_ptr <Vault>> *m_SVaults;
    map <string, ElfcloudDirCache *>m_SDirs;
    map <string, shared_ptr <elfcloud::DataItem>> m_SFiles;
    map <string, ElfcloudFSCache *>m_SOpenFile;
    map <string, string> m_SCacheFile;
    uint64_t m_lFh;

    static ElfcloudFS *m_SInstance;

    ///
    // Find Vault by vault name
    // @param name Vault name
    // @return NULL if not found or Vault
    //
    shared_ptr <elfcloud::Vault> getVaultByName(
        const char *name
    );

    ///
    // Split path to it's components /some/dir/is to some,dir,is
    // @param path Current path
    // @return NULL if not found or Splitted path in vector
    //
    vector <string> getSplittedPath(
        const char *path
    );

    shared_ptr <elfcloud::Cluster> getClusterByName(
        shared_ptr <elfcloud::Cluster> cluster,
        const char *name
    );

    ElfcloudDirCache *getClusterByPath(
        const char *path
    );

    mode_t getUnixPermissions(
        vector <string> permissions
    );

    ElfcloudFSCache *getOpenFileCacheByPath(
        const char *path,
        uint64_t fh
    );

    string getOpenFileCacheWholePathByPath(
        const char *path,
        uint64_t fh
    );

    string getCacheFileNameByPath(
        const char *path,
        uint64_t fh
    );

public:

    /**
     * Return ElfcloudFS instance
     * @return instance of ElfcloudFS object
     */
    static ElfcloudFS *Instance(
    );

    /**
     *  Constructor
     */
    ElfcloudFS(
    );

    /**
     * Destructor
     */
    ~ElfcloudFS(
    );

    /**
     * Set file/dir attributes
     * @param path directory
     * @param statbuf where to store these values
     * @return ERRNO or 0 if correct
     */
    int Getattr(
        const char *path,
        struct stat *statbuf
    );

    /**
     * Link location
     * @param path directory
     * @param link where link destination is
     * @param size what is size of this object
     * @return ERRNO or 0 if correct
     */
    int Readlink(
        const char *path,
        char *link,
        size_t size
    );

    /**
     * Link location
     * @param path directory
     * @param mode what mode we want to use
     * @param dev Create device
     * @return ERRNO or 0 if correct
     */
    int Mknod(
        const char *path,
        mode_t mode,
        dev_t dev
    );

    /**
     * Link location
     * @param path directory
     * @param mode Change mode
     * @return ERRNO or 0 if correct
     */
    int Mkdir(
        const char *path,
        mode_t mode
    );

    /**
     * Unlink file
     * @param path directory
     * @return ERRNO or 0 if correct
     */
    int Unlink(
        const char *path
    );

    /**
     * Remove dir
     * @param path directory
     * @return ERRNO or 0 if correct
     */
    int Rmdir(
        const char *path
    );

    /**
     * Symbolic link
     * @param path directory
     * @param link where link destination is
     * @return ERRNO or 0 if correct
     */
    int Symlink(
        const char *path,
        const char *link
    );

    /**
     * Rename file or dir
     * @param path directory
     * @param newpath rename to what?
     * @return ERRNO or 0 if correct
     */
    int Rename(
        const char *path,
        const char *newpath
    );

    /**
     * Link location
     * @param path directory
     * @param newpath where link destination is
     * @return ERRNO or 0 if correct
     */
    int Link(
        const char *path,
        const char *newpath
    );

    /**
     * Change mod
     * @param path directory
     * @param mode to what mode we want it?
     * @return ERRNO or 0 if correct
     */
    int Chmod(
        const char *path,
        mode_t mode
    );

    /**
     * Change owner
     * @param path directory
     * @param uid User id
     * @param gid Group id
     * @return ERRNO or 0 if correct
     */
    int Chown(
        const char *path,
        uid_t uid,
        gid_t gid
    );

    /**
     * Remove
     * @param path directory
     * @param newSize to what size we want it to be
     * @return ERRNO or 0 if correct
     */
    int Truncate(
        const char *path,
        off_t newSize
    );

    /**
     * nano time of file
     * @param path directory
     * @param ubuf Time buffer
     * @return ERRNO or 0 if correct
     */
    int Utime(
        const char *path,
        struct utimbuf *ubuf
    );

    /**
     * Open file
     * @param path directory
     * @param fileInfo File info
     * @return ERRNO or 0 if correct
     */
    int Open(
        const char *path,
        struct fuse_file_info *fileInfo
    );

    /**
     * Read from filesystem
     * @param path directory
     * @param buf Where to read data
     * @param size size of data
     * @param offset from offset to read
     * @param fileInfo File info
     * @return ERRNO or 0 if correct
     */
    int Read(
        const char *path,
        char *buf,
        size_t size,
        off_t offset,
        struct fuse_file_info *fileInfo
    );

    /**
     * Write to filesyste,
     * @param path directory
     * @param buf data to write
     * @param size size of data
     * @param offset where to write it
     * @param fileInfo file Info
     * @return ERRNO or 0 if correct
     */
    int Write(
        const char *path,
        const char *buf,
        size_t size,
        off_t offset,
        struct fuse_file_info *fileInfo
    );

    /**
     * Show stats of filesystem
     * @param path directory
     * @param statInfo Stats struct
     * @return ERRNO or 0 if correct
     */
    int Statfs(
        const char *path,
        struct statvfs *statInfo
    );

    /**
     * Flush files from cache
     * @param path directory
     * @param fileInfo Filei info
     * @return ERRNO or 0 if correct
     */
    int Flush(
        const char *path,
        struct fuse_file_info *fileInfo
    );

    /**
     * Release file
     * @param path directory
     * @param fileInfo file info
     * @return ERRNO or 0 if correct
     */
    int Release(
        const char *path,
        struct fuse_file_info *fileInfo
    );

    /**
     * Sync file
     * @param path directory
     * @param datasync Data sync
     * @param fi file info
     * @return ERRNO or 0 if correct
     */
    int Fsync(
        const char *path,
        int datasync,
        struct fuse_file_info *fi
    );

    /**
     * Sett Extra Attribute
     * @param path directory
     * @param name name of file
     * @param value what to set with value
     * @param size Size of value
     * @param flags what flags are set
     * @return ERRNO or 0 if correct
     */
    int Setxattr(
        const char *path,
        const char *name,
        const char *value,
        size_t size,
        int flags
    );

    /**
     * Get Extra attributes
     * @param path directory
     * @param name file name
     * @param value what attribute
     * @param size of name
     * @return ERRNO or 0 if correct
     */
    int Getxattr(
        const char *path,
        const char *name,
        char *value,
        size_t size
    );

    /**
     * List extra attributes
     * @param path directory
     * @param list where to list them
     * @param size size of list
     * @return ERRNO or 0 if correct
     */
    int Listxattr(
        const char *path,
        char *list,
        size_t size
    );

    /**
     * Read Extra attributes
     * @param path directory
     * @param name file name
     * @return ERRNO or 0 if correct
     */
    int Removexattr(
        const char *path,
        const char *name
    );

    /**
     * Opendir
     * @param path directory
     * @param fileInfo File info
     * @return ERRNO or 0 if correct
     */
    int Opendir(
        const char *path,
        struct fuse_file_info *fileInfo
    );

    /**
     * Read some dir
     * @param path directory
     * @param buf some buffer
     * @param filler Filler
     * @param offset
     * @param fileInfo File ino
     * @return ERRNO or 0 if correct
     */
    int Readdir(
        const char *path,
        void *buf,
        fuse_fill_dir_t filler,
        off_t offset,
        struct fuse_file_info *fileInfo
    );

    /**
     * Release some dir
     * @param path directory
     * @param fileInfo File info
     * @return ERRNO or 0 if correct
     */
    int Releasedir(
        const char *path,
        struct fuse_file_info *fileInfo
    );

    /**
     * Sync dir
     * @param path directory
     * @param datasync data sync
     * @param fileInfo fileinfo
     * @return ERRNO or 0 if correct
     */
    int Fsyncdir(
        const char *path,
        int datasync,
        struct fuse_file_info *fileInfo
    );

    /**
     * Init
     * @param conn fuse conn info
     * @return ERRNO or 0 if correct
     */
    int Init(
        struct fuse_conn_info *conn
    );

    /**
     * Remove path destination file
     * @param path directory
     * @param offset what is size of this object
     * @param fileInfo File info
     * @return ERRNO or 0 if correct
     */
    int Truncate(
        const char *path,
        off_t offset,
        struct fuse_file_info *fileInfo
    );

    /**
     * Create Elfcloud client and prepare config for it
     * @param configpath Path to config file normal ~/.elfcloud/userconfig.xml
     * @return below zero if not ok 0 is ok
     */
    int createElfcloudClient(
        char *configpath
    );

    /**
     * Connect to Elfcloud instace
     * @param username username of user something@something.tld
     * @param password password of user
     * @param upspeed Speed for uploading
     * @param downspeed Speed for download
     * @return below zero if not ok 0 is ok
     */
    int Connect(
        char *username,
        char *password,
        long upspeed,
        long downspeed
    );

    /**
     * Disconnect Elfcloud instace
     * @return below zero if not ok 0 is ok
     */
    int Disconnect(
    );
};



#endif
