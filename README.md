## What is it?

Elfcloud.fi is a Finland based credibly secure cloud storage service. You
can save and backup to, work in and share from within the cloud with
indisputable confidence of your data remaining private, integral and available.

https://secure.elfcloud.fi/en/

Elfcloud-FUSE is FUSE implemtation that provides filesystem like access to elfcloud.fi.

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

## Depencies
http://www.cryptopp.com/ - Crypto++ Library is a free C++ class library of cryptographic schemes
https://github.com/open-source-parsers/jsoncpp - JSON is a lightweight data-interchange format
http://curl.haxx.se/ - CURL is a command line tool and library for transferring data with URL syntax
http://www.cmake.org/ - CMake, the cross-platform, open-source build system

## Building Linux and Mac OS X
Install depencies and after that

<pre>
git clone https://github.com/illuusio/elfcloud-fuse.git
cd elfcloud-fuse
cmake .
make
</pre>

### Running
You can test application with (You need Elfcloud.fi account)

main/elfcloud-fuse your@domain.com /mount/point/that/you/can/write/and/read

After that you should be asked your password

<pre>
cd /mount/point/that/you/can/write/and/read
ls
</pre>

##Problems
If Elfcloud-fuse crashes FUSE pipes that it holds are not clearly released so you need to umount with root mountpoint

<pre>
umount /mount/point/that/you/can/write/and/read
</pre>

### Bugs
Report bugs on Github
