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
 * $Date $
 * $Rev $
 * $Author $
 */


#include <iostream>
#include <list>

#include <API.h>

#include <stdio.h>
#include <memory>

#include <getopt.h>
#include <termios.h>

#include <cryptopp/sha.h>
#include <cryptopp/whrlpool.h>
#include <cryptopp/base64.h>

extern char *optarg;
extern int optind,
       opterr,
       optopt;

#define ELFCLOUD_FETCH_COUNT 2

using namespace std;
using namespace elfcloud;

int readBytesFromFile(const string pInFilename, const unsigned int pInOffset, unsigned int pInLength, byte *pOutBuffer)
{

    FILE *hFile;

    hFile = fopen(pInFilename.c_str(), "rb");

    if (hFile == NULL)
    {
        cout << pInFilename << " doesn't exist" << endl;
        return -1;
    }

    fseek(hFile, 0L, SEEK_END);
    int fileLength = ftell(hFile);
    fseek(hFile, pInOffset, SEEK_SET);

    if (pInLength == 0)
    {
        pInLength = fileLength;
    }

    if ((pInOffset + pInLength) > fileLength)
    {
        cout << "File too short " << fileLength << " bytes when attempted to read " << pInLength << " bytes from offset " << pInOffset << endl;
        fclose(hFile);
        return -2;
    }

    unsigned int result = fread (pOutBuffer, 1, pInLength, hFile);

    if (result != pInLength)
    {
        cout << "Read too short, returned " << result << " expected " << pInLength << endl;
        fclose(hFile);
        return -3;
    }

    fclose(hFile);
    return result;
}

int writeToFile(const string pInFilename, const unsigned int pInBufferOffset, const unsigned int pInBufferLength, const byte *pInBuffer)
{

    FILE *hFile;

    hFile = fopen(pInFilename.c_str(), "wb");

    if (hFile == NULL)
    {
        cout << pInFilename << " doesn't exist" << endl;
        return -1;
    }

    unsigned int blocksWritten = fwrite(pInBuffer + pInBufferOffset, pInBufferLength, 1, hFile);

    if (blocksWritten != 1)
    {
        cout << "File write length mismatch, attempted to write " << pInBufferLength << ", result was " << blocksWritten << endl;
        fclose(hFile);
        return -2;
    }

    fclose(hFile);
    return 0;
}

/**
 *  Testing Elfcloud library
 */
class LibraryTest
{

public:
    /**
     *  Create library testin with preset login
     */
    LibraryTest();
    /**
     *  Create library testin with user and password
     * @param username username
     * @param password Password
     */
    LibraryTest(char *username, char *password);
    /**
     * Destructor
     */
    ~LibraryTest();

    /**
     * Client to Elfcloud
     */
    Client *eclib;
    /**
     *  To test lib we fetch files from Clusters
     *  We can choose how many with ELFCLOUD_FETCH_COUNT
     */
    int fetched;

    /**
     * Enable debug code in Curl
     * @return 0 if good 1 if not
     */
    int enableDebug();

    /**
     * Read config file
     * @return 0 if good 1 if not
     */
    int readBeaverUserConfig();

    /**
     *  List Vaults and files in them
     * @return 0 if good 1 if not
     */
    int listVaults();
    /**
     * List cluster info and files
     * @param clusters Map of clusters (Dirs)
     * @return 0 if good 1 if not
     */
    int listClusters(list<shared_ptr <elfcloud::Cluster>> *clusters);
    /**
     * List files is cluster
     * @param cluster Cluster that files are on
     * @param dataitems Items that belongs to cluster
     * @return 0 if good 1 if not
     */
    int listDataItems(shared_ptr<elfcloud::Cluster> cluster, list<shared_ptr<elfcloud::DataItem>> *dataitems);
    /**
     *  List permissions
     *  @param permissions Strings of permissions
     * @return 0 if good 1 if not
     */
    int listPermissions(vector<string> permissions);
};

LibraryTest::LibraryTest()
{
    eclib = new Client();
    eclib->setPasswordAuthenticationCredentials("your.credentials@here.fi", "andpassword");
    fetched = 0;
}

LibraryTest::LibraryTest(char *username, char *password)
{
    eclib = new Client();
    eclib->setPasswordAuthenticationCredentials(username, password);
    eclib->setConf("http.limit.receive.Bps", "100240");
    eclib->setConf("http.limit.send.Bps", "100240");

    fetched = 0;
}

LibraryTest::~LibraryTest()
{
    delete eclib;
}

int LibraryTest::enableDebug()
{
    eclib->setConf("http.json.output", "1");
    eclib->setConf("http.data-api.header.output", "1");
    return 0;
}

int LibraryTest::readBeaverUserConfig()
{
    // must point to elfcloud.fi
    // Beaver generated userconfig.xml,
    // used to read crypto keys from on
    // client init
    eclib->readUserConfig("userconfig.xml");
    return 0;
}

int LibraryTest::listDataItems(shared_ptr<elfcloud::Cluster> cluster, list<shared_ptr<elfcloud::DataItem>> *dataitems)
{

    for (list<shared_ptr <elfcloud::DataItem>>::iterator iter = dataitems->begin(); iter != dataitems->end(); iter++)
    {
        shared_ptr<elfcloud::DataItem> l_SItem = (*iter);
        cout << "DataItem: " << l_SItem->getDataItemName();
        cout << " DataItems: " << l_SItem->getDescription();
        cout << " Size: " << l_SItem->getDataLength() << endl;

        /*if(fetched < ELFCLOUD_FETCH_COUNT)
        {
            fetched ++;

            if( cluster->fetchDataItem((*l_SItem)) == true)
            {
                CryptoPP::SHA256 l_SSHA256;
                CryptoPP::Whirlpool l_SWhirlpool;
                byte l_cSHA256Bytes[l_SSHA256.DigestSize()];
                byte l_cWhirpoolBytes[l_SWhirlpool.DigestSize()];
                l_SSHA256.CalculateDigest(l_cSHA256Bytes, (const byte *)l_SItem->getDataPtr(), l_SItem->getDataLength());
                l_SWhirlpool.CalculateDigest(l_cWhirpoolBytes, (const byte *)l_SItem->getDataPtr(), l_SItem->getDataLength());

                CryptoPP::Base64Encoder l_SEncoderSHA256;
                CryptoPP::Base64Encoder l_SEncoderWhirpool;
                std::string l_strOutputSHA256;
                std::string l_strOutputWhirlpool;
                l_SEncoderSHA256.Attach( new CryptoPP::StringSink( l_strOutputSHA256 ) );
                l_SEncoderSHA256.Put( l_cSHA256Bytes, sizeof(l_cSHA256Bytes) );
                l_SEncoderSHA256.MessageEnd();

                l_SEncoderWhirpool.Attach( new CryptoPP::StringSink( l_strOutputWhirlpool ) );
                l_SEncoderWhirpool.Put( l_cWhirpoolBytes, sizeof(l_cWhirpoolBytes) );
                l_SEncoderWhirpool.MessageEnd();


                cout << " Base64 SHA256 Digest: " << l_strOutputSHA256 << endl;
                cout << " Base64 Whirlpool Digest: " << l_strOutputWhirlpool << endl;
            }

            else
            {
                cout << "Can't fetch item!" << endl;
            }
        }*/

    }

    return 0;
}


int LibraryTest::listClusters(list<shared_ptr<elfcloud::Cluster>> *clusters)
{
    for (list<shared_ptr<elfcloud::Cluster>>::iterator iter = clusters->begin(); iter != clusters->end(); iter++)
    {
        // Zero fetch count so we allways test fetching
        fetched = 0;

        shared_ptr<elfcloud::Cluster> l_SItem = (*iter);
        cout << "Cluster: " << l_SItem->getClusterName();
        cout << " Items: " << l_SItem->getClusterDataItems();
        cout << " Size: " << l_SItem->getSizeBytes() << endl;
        listPermissions(l_SItem->getPermissions());
        listDataItems(l_SItem, l_SItem->listDataItems());
        listClusters(l_SItem->listClusters());

    }

    return 0;
}

int LibraryTest::listPermissions(vector<string> permissions)
{
    cout << " Permissions: [ ";

    for (int i = 0; i < permissions.size(); ++i)
    {
        cout << permissions.at( i ) << ", ";
    }

    cout << "])" << endl;

    return 0;
}

int LibraryTest::listVaults()
{

    try
    {
        list<shared_ptr <Vault>> *listVaults = Vault::ListVaults(eclib);

        for (list<shared_ptr <Vault>>::iterator iter = listVaults->begin(); iter != listVaults->end(); iter++)
        {
            shared_ptr<Vault> tmp = *iter;
            vector<string> l_strPermissions;

            cout << "Vault: " << tmp->getName();
            cout << " type: " << tmp->getType();
            cout << " ID: " << tmp->getId();
            cout << " ContainerId: " << tmp->getContainerId() << endl;

            cout << " (Last modified: " << tmp->getLastModified();
            cout << " Size in bytes: " << tmp->getTotalSizeInBytes();
            cout << " Descendant count: " << tmp->getDescendantCount() << " [";
            cout << listPermissions(tmp->getPermissions());

            std::map<std::string, std::list<shared_ptr<elfcloud::Object>>*> *mapContents = tmp->listContents();

            for (std::map<std::string, std::list<shared_ptr<elfcloud::Object>>*>::iterator mapiter = mapContents->begin(); mapiter != mapContents->end(); mapiter++)
            {
                std::string l_strObjName = (*mapiter).first;
                list<shared_ptr<elfcloud::Object>> *l_SObject = (*mapiter).second;
                elfcloud::Cluster *l_SCurCluster = NULL;
                cout << "Object: " << l_strObjName << endl;

                if( !l_strObjName.compare("dataitems") )
                {
                    listDataItems(0, (list<shared_ptr <elfcloud::DataItem>> *)l_SObject);
                }

                else if ( !l_strObjName.compare("clusters") )
                {
                    listClusters((list<shared_ptr<elfcloud::Cluster>> *)l_SObject);
                }

            }
        }

        Vault::FreeVaultSet(listVaults);
    }

    catch (elfcloud::Exception &e)
    {
        cout << "Exception: " << e.getCode() << ", " << e.getMsg() << endl;
    }

    return 0;
}

/**
 * Create test
 * @param username username
 * @param password password
 */
int test(char *username, char *password)
{
    LibraryTest *libTest = new LibraryTest(username, password);

    // Enable stdout printing of JSON requests and responses
    libTest->enableDebug();

    libTest->readBeaverUserConfig();

    libTest->listVaults();

    delete libTest;

    return 0;

}

int main (int argc, char *argv[])
{
    char l_cChar = 0;
    char l_strUsername[1024];
    char l_strPassword[1024];

    memset( l_strUsername, 0x00, 1024 );
    memset( l_strPassword, 0x00, 1024 );

    // This will be done while given options are acceptable
    while ((l_cChar = getopt(argc, argv, "u:p:?")) != -1)
    {
        switch (l_cChar)
        {
        case 'u':
            strncpy( l_strUsername, optarg, 1024);
            break;

        case 'p':
            strncpy( l_strPassword, optarg, 1024);
            break;

        case '?':
            cout << "Usage:" << endl;
            cout << " -u Username" << endl;
            cout << " -p password or null then it's asked" << endl;
            return(0);
            break;
        }
    }

    if( strlen(l_strPassword) == 0 )
    {
        struct termios l_SOld;
        struct termios l_SNew;
        char l_strLine[1024];

        cout << "Password:";

        /* Turn echoing off and fail if we can't. */
        if (tcgetattr (fileno(stdin), &l_SOld) != 0)
        {
            return -1;
        }

        l_SNew = l_SOld;
        l_SNew.c_lflag &= ~ECHO;

        if (tcsetattr (fileno(stdin), TCSAFLUSH, &l_SNew) != 0)
        {
            return -1;
        }

        /* Read the password. */
        memset(l_strLine, 0x00, 1024);
        fgets(l_strLine, 1024, stdin);

        /* Restore terminal. */
        (void) tcsetattr (fileno(stdin), TCSAFLUSH, &l_SOld);

        if( strlen(l_strLine) >= 8 )
        {
            strncpy( l_strPassword, l_strLine, strlen(l_strLine) - 1);
        }

        else
        {
            cout << "Too sort password! Exiting!" << endl;
            return(-1);
        }

    }


    cout << "main()" << endl;
    int res = test(l_strUsername, l_strPassword);
    cout << "Test result " << res << endl;

    return(0);
}

