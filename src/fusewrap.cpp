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


#include "fusewrap.h"
#include "elfcloudfs.hh"

int ec_fusewrap_getattr(const char *path, struct stat *statbuf)
{
    return ElfcloudFS::Instance()->Getattr(path, statbuf);
}

int ec_fusewrap_readlink(const char *path, char *link, size_t size)
{
    return ElfcloudFS::Instance()->Readlink(path, link, size);
}

int ec_fusewrap_mknod(const char *path, mode_t mode, dev_t dev)
{
    return ElfcloudFS::Instance()->Mknod(path, mode, dev);
}

int ec_fusewrap_mkdir(const char *path, mode_t mode)
{
    return ElfcloudFS::Instance()->Mkdir(path, mode);
}

int ec_fusewrap_unlink(const char *path)
{
    return ElfcloudFS::Instance()->Unlink(path);
}

int ec_fusewrap_rmdir(const char *path)
{
    return ElfcloudFS::Instance()->Rmdir(path);
}

int ec_fusewrap_symlink(const char *path, const char *link)
{
    return ElfcloudFS::Instance()->Symlink(path, link);
}

int ec_fusewrap_rename(const char *path, const char *newpath)
{
    return ElfcloudFS::Instance()->Rename(path, newpath);
}

int ec_fusewrap_link(const char *path, const char *newpath)
{
    return ElfcloudFS::Instance()->Link(path, newpath);
}

int ec_fusewrap_chmod(const char *path, mode_t mode)
{
    return ElfcloudFS::Instance()->Chmod(path, mode);
}

int ec_fusewrap_chown(const char *path, uid_t uid, gid_t gid)
{
    return ElfcloudFS::Instance()->Chown(path, uid, gid);
}

int ec_fusewrap_truncate(const char *path, off_t newSize)
{
    return ElfcloudFS::Instance()->Truncate(path, newSize);
}

int ec_fusewrap_utime(const char *path, struct utimbuf *ubuf)
{
    return ElfcloudFS::Instance()->Utime(path, ubuf);
}

int ec_fusewrap_open(const char *path, struct fuse_file_info *fileInfo)
{
    return ElfcloudFS::Instance()->Open(path, fileInfo);
}

int ec_fusewrap_read(const char *path, char *buf, size_t size, off_t offset, struct fuse_file_info *fileInfo)
{
    return ElfcloudFS::Instance()->Read(path, buf, size, offset, fileInfo);
}

int ec_fusewrap_write(const char *path, const char *buf, size_t size, off_t offset, struct fuse_file_info *fileInfo)
{
    return ElfcloudFS::Instance()->Write(path, buf, size, offset, fileInfo);
}

int ec_fusewrap_statfs(const char *path, struct statvfs *statInfo)
{
    return ElfcloudFS::Instance()->Statfs(path, statInfo);
}

int ec_fusewrap_flush(const char *path, struct fuse_file_info *fileInfo)
{
    return ElfcloudFS::Instance()->Flush(path, fileInfo);
}

int ec_fusewrap_release(const char *path, struct fuse_file_info *fileInfo)
{
    return ElfcloudFS::Instance()->Release(path, fileInfo);
}

int ec_fusewrap_fsync(const char *path, int datasync, struct fuse_file_info *fi)
{
    return ElfcloudFS::Instance()->Fsync(path, datasync, fi);
}

int ec_fusewrap_setxattr(const char *path, const char *name, const char *value, size_t size, int flags)
{
    return ElfcloudFS::Instance()->Setxattr(path, name, value, size, flags);
}

int ec_fusewrap_getxattr(const char *path, const char *name, char *value, size_t size)
{
    return ElfcloudFS::Instance()->Getxattr(path, name, value, size);
}

int ec_fusewrap_listxattr(const char *path, char *list, size_t size)
{
    return ElfcloudFS::Instance()->Listxattr(path, list, size);
}

int ec_fusewrap_removexattr(const char *path, const char *name)
{
    return ElfcloudFS::Instance()->Removexattr(path, name);
}

int ec_fusewrap_opendir(const char *path, struct fuse_file_info *fileInfo)
{
    return ElfcloudFS::Instance()->Opendir(path, fileInfo);
}

int ec_fusewrap_readdir(const char *path, void *buf, fuse_fill_dir_t filler, off_t offset, struct fuse_file_info *fileInfo)
{
    return ElfcloudFS::Instance()->Readdir(path, buf, filler, offset, fileInfo);
}

int ec_fusewrap_releasedir(const char *path, struct fuse_file_info *fileInfo)
{
    return ElfcloudFS::Instance()->Releasedir(path, fileInfo);
}

int ec_fusewrap_fsyncdir(const char *path, int datasync, struct fuse_file_info *fileInfo)
{
    return ElfcloudFS::Instance()->Fsyncdir(path, datasync, fileInfo);
}

void *ec_fusewrap_init(struct fuse_conn_info *conn)
{
    ElfcloudFS::Instance()->Init(conn);

    return NULL;
}

int ec_fusewrap_createElfcloudClient(char *configpath)
{
    return ElfcloudFS::Instance()->createElfcloudClient(configpath);
}

int ec_fusewrap_connect(char *username, char *password, long upspeed, long downspeed)
{
    return ElfcloudFS::Instance()->Connect(username, password, upspeed, downspeed);
}

int ec_fusewrap_disconnect()
{
    return ElfcloudFS::Instance()->Disconnect();
}

int ec_fusewrap_free()
{
    delete ElfcloudFS::Instance();
    return 0;
}

