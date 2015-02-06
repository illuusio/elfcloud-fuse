cmake_minimum_required(VERSION 2.8)
project(ElfcloudFS C CXX)

set (APP_MAJOR 15)
set (APP_MINOR 04)
set (APP_VERSION "${APP_MAJOR}.${APP_MINOR}")

option (BUILD_SHARED_LIBS "Build dynamic link libraries" OFF)

set (CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH}
    "${CMAKE_SOURCE_DIR}/CMakeModules/")

find_package (JsonCpp REQUIRED)
find_package (CryptoPP REQUIRED)
find_package (CURL REQUIRED)

# Tweak compiler flags. Because some systems like Ubuntu 12.04 doesn't support
# C++11 we have to rely on C++0x hacks
set (CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -Wall -g")
set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++0x")

find_package (FUSE REQUIRED)
include_directories (${FUSE_INCLUDE_DIR})
add_definitions (-D_FILE_OFFSET_BITS=64 -DFUSE_USE_VERSION=26 -DECDEBUG -DELFCLOUD_LIB -g)

if (APPLE)
    set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -mmacosx-version-min=10.6")
endif (APPLE)

# Packaging config.
set (CPACK_PACKAGE_NAME "elfcloud-fuse")
set (CPACK_PACKAGE_VERSION_MAJOR ${APP_MAJOR})
set (CPACK_PACKAGE_VERSION_MINOR ${APP_MINOR})
set (CPACK_SOURCE_GENERATOR TGZ)
set (CPACK_SOURCE_IGNORE_FILES
     ".*~$"
     ".a$"
#     ".so*$"
     ".svn$"
     "/.svn/"
     ".hg$"
     "/.hg/"
     "patch*$"
     ".*tar.gz$"
     ".*Z$"
     ".*zip$"
     "/_CPack_Packages/"
     "/CPackSourceConfig.cmake$"
     "/CPackConfig.cmake$"
     "/CMakeCache.txt$"
     "/CMakeFiles/"
     "/Makefile*$"
     "/cmake_install.cmake$"
     "/main/elfcloud-fuse$"
     "/elfcloud-cpp/ec-test-cpp$")

#
# CPack.. make package
#
IF (NOT WIN32)
  set(CPACK_GENERATOR "TGZ")
  set(CPACK_SOURCE_GENERATOR "TGZ")

  IF (APPLE)
    set(CPACK_GENERATOR
        "Bundle")
  ENDIF(APPLE)

ELSE (NOT WIN32)
  set(CPACK_GENERATOR "ZIP")
  set(CPACK_SOURCE_GENERATOR "ZIP")
ENDIF(NOT WIN32)

SET(CPACK_PACKAGE_INSTALL_DIRECTORY 
    "elfcloud-fuse-${APP_VERSION}")

set(CPACK_PACKAGE_FILE_NAME
     "elfcloud-fuse-${APP_VERSION}")

set(CPACK_SOURCE_PACKAGE_FILE_NAME
    "elfcloud-fuse_${APP_VERSION}")

set(CPACK_PACKAGE_NAME ${CMAKE_PROJECT_NAME})
# set(CPACK_SET_DESTDIR TRUE)
set(CPACK_DEBIAN_PACKAGE_MAINTAINER "Tuukka Pasanen <pasanen.tuukka@gmail.com>")
set(CPACK_PACKAGE_VENDOR "Tuukka Pasanen <tuukka.pasanen@ilmi.fi>")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Elfcloud-FUSW ${APP_VERSION}")
set(CPACK_PACKAGE_VERSION ${APP_VERSION})

IF (APPLE)
  set(CPACK_PACKAGE_ICON "icon/elfcloud-fuse.icns")
  set(CPACK_BUNDLE_NAME "elfcloud-fuse")
  set(CPACK_BUNDLE_ICON "icon/elfcloud-fuse.icns")
  set(CPACK_BUNDLE_PLIST "icon/Info.plist")
  set(CPACK_BUNDLE_STARTUP_COMMAND "main/elfcloud-fuse")
ENDIF (APPLE)

SET(CPACK_PACKAGE_INSTALL_DIRECTORY 
    "elfcloud-fuse-${APP_VERSION}")

set(CPACK_SOURCE_PACKAGE_FILE_NAME
    "${CMAKE_PROJECT_NAME}_${APP_VERSION}")

#IF (APPLE)
#      # Install needed Frameworks..
#      # In future do this same in Windows and Linux!
#
#      INSTALL(CODE "execute_process(COMMAND dylibbundler -d \"${CMAKE_INSTALL_PREFIX}/lib\" -b -x \"\${CMAKE_INSTALL_PREFIX}/bin/elfcloud-fuse\")")
#
#ENDIF (APPLE)


# Check some random files
# This is probably needles in future! 
include (CheckIncludeFileCXX)
check_include_file_cxx (attr/xattr.h HAVE_ATTR_XATTR_H)
check_include_file_cxx (sys/xattr.h HAVE_SYS_XATTR_H)

check_include_file_cxx (tr1/memory HAVE_TR1_MEMORY)
check_include_file_cxx (tr1/unordered_map HAVE_TR1_UNORDERED_MAP)
check_include_file_cxx (tr1/unordered_set HAVE_TR1_UNORDERED_SET)
check_include_file_cxx (tr1/tuple HAVE_TR1_TUPLE)

check_include_file_cxx (valgrind/valgrind.h HAVE_VALGRIND_VALGRIND_H)
check_include_file_cxx (valgrind/memcheck.h HAVE_VALGRIND_MEMCHECK_H)

# Used with CommonCrypto
check_include_file_cxx (Security/SecRandom.h HAVE_SEC_RANDOM_H)

# Check if xattr functions take extra argument.
include (CheckCXXSourceCompiles)
CHECK_CXX_SOURCE_COMPILES ("#include <sys/types.h>
  #include <sys/xattr.h>
  int main() { getxattr(0,0,0,0,0,0); return 1; } " XATTR_ADD_OPT)

include (CheckFunctionExists)
check_function_exists(lchmod HAVE_LCHMOD)

find_package (Threads)

set (CMAKE_THREAD_PREFER_PTHREAD)
find_program (POD2MAN pod2man)

# Prefix for ElfcloudFS module includes.
include_directories (${ELFCLOUD_BINARY_DIR})
include_directories (${ELFCLOUD_SOURCE_DIR})

IF (NOT APPLE)
     configure_file(${PROJECT_BINARY_DIR}/elfcloud-fuse.dsc.in
     "${PROJECT_BINARY_DIR}/elfcloud-fuse.dsc" @ONLY)

     configure_file(${PROJECT_BINARY_DIR}/elfcloud-fuse.spec.in
     "${PROJECT_BINARY_DIR}/elfcloud-fuse.spec" @ONLY)

     configure_file(${PROJECT_BINARY_DIR}/debian/changelog.in
     "${PROJECT_BINARY_DIR}/changelog" @ONLY)
ENDIF (NOT APPLE)

# Subdirectories.
add_subdirectory(elfcloud-cpp)
add_subdirectory(src)
add_subdirectory(main)


include (CPack)

