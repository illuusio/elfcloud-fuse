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



#include "fusewrap.h"

#include <fuse.h>
#include <stdio.h>
#include <getopt.h>
#include <termios.h>
#include <stddef.h>

/**
 * Satisfying FUSE options parsing 
 */
struct ecOptionsData
{

    /**
     * Pointer to password 
     */
    char *password;

    /**
     * Pointer to username
     */
    char *username;

    /**
     * Pointer to config 
     */
    char *config;

    /**
     * File where password is read
     */
    char *passfile;

    /**
     * Max speed up
     */
    long maxSpeedUp;

    /**
     * Max speed down
     */
    long maxSpeedDown;
} ec;

#define EC_FUSE_OPT2(one, two, offset, key) \
        {one, offsetof(struct ecOptionsData, offset), key}, \
        {two, offsetof(struct ecOptionsData, offset), key}
#define EC_FUSE_OPT3(one, two, three, offset, key) \
        {one, offsetof(struct ecOptionsData, offset), key}, \
        {two, offsetof(struct ecOptionsData, offset), key}, \
        {three, offsetof(struct ecOptionsData, offset), key}


//struct fuse_operations elfcloudfs_oper;

/**
 * Struct that contains elfcloud params
 */
struct elfcloud_params
{

    /**
     * Username
     */
    char username[1024];

    /**
     * Password
     */
    char password[1024];

    /**
     * Where you can find userconfig.xml
     */
    char userConfig[1024];

    /**
     * Max upload speed
     */
    long maxSpeedUp;

   /**
     * Max download speed
     */
    long maxSpeedDown;

   /**
    * Mount dir
    */
    char mountpoint[1024];


   /**
    * Password file
    */
    char passfile[1024];
};

static struct elfcloud_params m_SParams;
static struct fuse_operations elfcloudfs_oper;

static int _ec_processOptions(
    void *data,
    const char *arg,
    int key,
    struct fuse_args *outargs
)
{
    switch (key)
    {
        case -2:
            if (m_SParams.username[0] == 0x00)
            {
                if (strchr(arg, '@') != NULL)
                {
                    strncpy(m_SParams.username, arg, 1024);
                    break;
                }
                else
                {
                    printf("Username must contain chae '@' now it's: %s (with -h or --help help)\n", arg);
                    return -1;
                }
            }
            else if (m_SParams.mountpoint[0] == 0x00)
            {
                strncpy(m_SParams.mountpoint, arg, 1024);
            }
            else
            {
                fprintf(stderr, "Too much non-stadard options: %s!\n", arg);
            }
            break;
        default:
            break;

    }
    return 0;
}



int _ec_askPassword(
)
{
    if (ec.password == NULL && ec.passfile == NULL)
    {
        struct termios l_SOld;
        struct termios l_SNew;
        int l_iNread = 0;
        char l_strLine[1024];

        printf("Elfcloud.fi Password: ");

        /* Turn echoing off and fail if we can't. */
        if (tcgetattr(fileno(stdin), &l_SOld) != 0)
            return -1;
        l_SNew = l_SOld;
        l_SNew.c_lflag &= ~ECHO;
        if (tcsetattr(fileno(stdin), TCSAFLUSH, &l_SNew) != 0)
            return -1;

        /* Read the password. */
        /* l_iNread = getline(&l_strLine, &l_iSize, stdin); */
        memset(l_strLine, 0x00, 1024);
        fgets(l_strLine, 1024, stdin);

        /* Restore terminal. */
        (void) tcsetattr(fileno(stdin), TCSAFLUSH, &l_SOld);

        if (strlen(l_strLine) >= 8)
        {
            strncpy(m_SParams.password, l_strLine, strlen(l_strLine) - 1);
        }
        else
        {
            printf("Too sort password! Exiting!\n");;
            return (-1);
        }

        printf("\n");
        return 0;
    }
    else if (strlen(ec.passfile) > 0)
    {
        FILE *l_pFile = fopen(ec.passfile, "r");
        int i = 0;
        if (l_pFile == NULL)
        {
            printf("Can't find password file: (%s)", ec.passfile);
            return -1;
        }
        fread(m_SParams.password, 1024, 1, l_pFile);
        for (i = 0; i < strlen(m_SParams.password); i++)
        {
            if (m_SParams.password[i] == '\n' || m_SParams.password[i] == '\r')
            {
                m_SParams.password[i] = 0x00;
            }
        }

        fclose(l_pFile);
        l_pFile = NULL;
        return 0;
    }


    printf("Can't set password.. exiting!\n");
    return -1;
}

int main(
    int argc,
    char *argv[]
)
{
    struct fuse_args l_SArgs = FUSE_ARGS_INIT(argc, argv);
    int l_iMultithreaded = 1;
    int l_iForeground = 1;
    int l_iRtn = 0;
    struct stat l_SStat;
    struct fuse_chan *l_SCh;
    struct fuse *l_SFuse = NULL;

    int i,
     fuse_stat;

    static const struct fuse_opt l_SOptions[] = {
        EC_FUSE_OPT3("-p %s", "--password=%s", "password=%s", password, -1),
        EC_FUSE_OPT3("-u %s", "--username=%s", "username=%s", username, -1),
        EC_FUSE_OPT3("-c %s", "--config=%s", "config=%s", config, -1),
        EC_FUSE_OPT3("-k %s", "--password-file=%s", "passwordfile=%s", passfile, -1),
        EC_FUSE_OPT3("-D %ld", "--download-max-speed=%ld", "download-max-speed=%ld", maxSpeedDown, -1),
        EC_FUSE_OPT3("-U %ld", "--upload-max-speed=%ld", "upload-max-speed=%ld", maxSpeedUp, -1),
        FUSE_OPT_END
    };

    elfcloudfs_oper.getattr = ec_fusewrap_getattr;
    elfcloudfs_oper.readlink = ec_fusewrap_readlink;
    elfcloudfs_oper.getdir = NULL;
    elfcloudfs_oper.mknod = ec_fusewrap_mknod;
    elfcloudfs_oper.mkdir = ec_fusewrap_mkdir;
    elfcloudfs_oper.unlink = ec_fusewrap_unlink;
    elfcloudfs_oper.rmdir = ec_fusewrap_rmdir;
    elfcloudfs_oper.symlink = ec_fusewrap_symlink;
    elfcloudfs_oper.rename = ec_fusewrap_rename;
    elfcloudfs_oper.link = ec_fusewrap_link;
    elfcloudfs_oper.chmod = ec_fusewrap_chmod;
    elfcloudfs_oper.chown = ec_fusewrap_chown;
    elfcloudfs_oper.truncate = ec_fusewrap_truncate;
    elfcloudfs_oper.utime = ec_fusewrap_utime;
    elfcloudfs_oper.open = ec_fusewrap_open;
    elfcloudfs_oper.read = ec_fusewrap_read;
    elfcloudfs_oper.write = ec_fusewrap_write;
    elfcloudfs_oper.statfs = ec_fusewrap_statfs;
    elfcloudfs_oper.flush = ec_fusewrap_flush;
    elfcloudfs_oper.release = ec_fusewrap_release;
    elfcloudfs_oper.fsync = ec_fusewrap_fsync;
    elfcloudfs_oper.setxattr = NULL;
    elfcloudfs_oper.getxattr = NULL;
    elfcloudfs_oper.listxattr = NULL;
    elfcloudfs_oper.removexattr = NULL;
    elfcloudfs_oper.opendir = ec_fusewrap_opendir;
    elfcloudfs_oper.readdir = ec_fusewrap_readdir;
    elfcloudfs_oper.releasedir = ec_fusewrap_releasedir;
    elfcloudfs_oper.fsyncdir = ec_fusewrap_fsyncdir;
    elfcloudfs_oper.init = ec_fusewrap_init;

    memset(m_SParams.password, 0x00, 1024);
    snprintf(m_SParams.userConfig, 1024, "%s/.elfcloud/userconfig.xml", getenv("HOME"));
    memset(m_SParams.mountpoint, 0x00, 1024);

    ec.password = NULL;
    ec.username = NULL;
    ec.maxSpeedDown = -1;
    ec.maxSpeedUp = -1;

    /* Parse options */
    if (fuse_opt_parse(&l_SArgs, &ec, l_SOptions, _ec_processOptions) == -1)
    {
        fprintf(stderr, "Can't parse options!\n");
        return 1;
    }

    if (_ec_askPassword() == -1)
    {
        fprintf(stderr, "No good password.. exiting!\n");
        return 1;
    }

    printf("mount point: %s (multi: %d, foreground: %d)\n", m_SParams.mountpoint, l_iMultithreaded, l_iForeground);

    if (m_SParams.mountpoint == NULL)
    {
        printf("Can't parse mount point.. bad arguments!\n");
        return -1;
    }

    l_iRtn = stat(m_SParams.mountpoint, &l_SStat);
    if (l_iRtn == -1)
    {
        printf("Mount point not good or not exist (%s)!\n", m_SParams.mountpoint);
        printf("Probably elfcloud-fuse have crashed and mounpoint is unclean\n");
        printf("Please try root or sudo 'umount %s' if it helps\n", m_SParams.mountpoint);
        perror(m_SParams.mountpoint);
    }

    l_SCh = fuse_mount(m_SParams.mountpoint, &l_SArgs);

    if (!l_SCh)
    {
        printf("Do you have permission to read and write to directory!\n");
        printf("because can't mount: (%s)!\n", m_SParams.mountpoint);
        return -1;
    }

    l_SFuse = fuse_new(l_SCh, &l_SArgs, &elfcloudfs_oper, sizeof(struct fuse_operations), NULL);

    if (l_SFuse == NULL)
    {
        printf("Mount point not good or not exist (%s)!\n", m_SParams.mountpoint);
        perror(m_SParams.mountpoint);
        fuse_unmount(m_SParams.mountpoint, l_SCh);
        return -1;

    }

    if (ec_fusewrap_createElfcloudClient(m_SParams.userConfig) < 0)
    {
        fuse_destroy(l_SFuse);
        fuse_unmount(m_SParams.mountpoint, l_SCh);
        return -1;
    }

    if (ec_fusewrap_connect(m_SParams.username, m_SParams.password, ec.maxSpeedUp, ec.maxSpeedDown) < 0)
    {
        fuse_destroy(l_SFuse);
        fuse_unmount(m_SParams.mountpoint, l_SCh);
        ec_fusewrap_disconnect();
        ec_fusewrap_free();
        return -1;
    }

    memset(m_SParams.username, 0x00, 1024);
    memset(m_SParams.password, 0x00, 1024);

    l_iRtn = fuse_daemonize(1);

    l_iRtn = fuse_loop(l_SFuse);

    fuse_opt_free_args(&l_SArgs);
    fuse_unmount(m_SParams.mountpoint, l_SCh);
    fuse_destroy(l_SFuse);
    ec_fusewrap_disconnect();
    ec_fusewrap_free();

    return fuse_stat;
}
