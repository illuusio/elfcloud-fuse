## What is it?

elfCLOUD.fi is a Finland based credibly secure cloud storage service. You
can save and backup to, work in and share from within the cloud with
indisputable confidence of your data remaining private, integral and available.

https://secure.elfcloud.fi/en/

elfCLOUD-FUSE is FUSE implementation that provides filesystem like access to elfCLOUD.fi.

## FUSE
with FUSE it is possible to implement a fully functional filesystem in a userspace program.  Features include:

* Simple library API
* Simple installation (no need to patch or recompile the kernel)
* Secure implementation
* Userspace - kernel interface is very efficient
* Usable by non privileged users
* Runs on Linux kernels 2.6.X and 3.x
* Has proven very stable over time

FUSE was originally developed to support AVFS but it has since became a separate project.
 Now quite a few other projects are using it. 

http://fuse.sourceforge.net

## Depends
http://www.cryptopp.com/ - Crypto++ Library is a free C++ class library of cryptography schemes
https://github.com/open-source-parsers/jsoncpp - JSON is a lightweight data-interchange format
http://curl.haxx.se/ - CURL is a command line tool and library for transferring data with URL syntax
http://www.cmake.org/ - CMake, the cross-platform, open-source build system

## Building Linux and Mac OS X
Install Depends and after that

<pre>
git clone https://github.com/illuusio/elfcloud-fuse.git
cd elfcloud-fuse
cmake .
make
</pre>

### Running
You can test application with (You need elfCLOUD.fi account)

main/elfcloud-fuse your@domain.com /mount/point/that/you/can/write/and/read

After that you should be asked your password

<pre>
cd /mount/point/that/you/can/write/and/read
ls
</pre>

### Linux and FUSE
FUSE should be automaticly loaded on start or when needed so if problem arises it's you
distribution malconfiguration.


### Mac OS X and OSXFuse
Application is compiled with OSXFuse from: https://osxfuse.github.io/ version <b>2.7.5</b>
If you need newer one or older one please recompile from source.

There is no GUI for bundle with Mac OS X to use. Only commandline from

<pre>
/Applications/elfcloud-fuse.app/Contents/Resources/bin/elfcloud-fuse your@domain.fi /your/mount/point
</pre>

### OSX specific problems
OSXFuse should be loaded to memory before mounting filesystem. It should happen automaticly after boot.
If you can't find it from command (line that contains 'com.github.osxfuse.filesystems.osxfusefs')
<pre>
kextstat
</pre>
then you should try to load it manually from commandline
<pre>
sudo kextload /Library/Filesystems/osxfusefs.fs/Support/osxfusefs.kext
</pre>
of from System center from Apple menu (OSXFuse has own setup there).

more problems solving please read OSXFuse Wiki: https://osxfuse.github.io/

##Problems
If elfCLOUD-FUSE crashes FUSE pipes that it holds are not clearly released so you need to umount with root mount point

<pre>
umount /mount/point/that/you/can/write/and/read
</pre>

### Bugs
Report bugs on Github
