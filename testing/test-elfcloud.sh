#!/bin/sh
#
# Copyright (c) 2014, Ilmi Solutions Oy
# All rights reserved.
# 
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following
# conditions are met:
# 
# * Redistributions of source code must retain the above copyright notice,
#   this list of conditions and the following disclaimer.
# * Redistributions in binary form must reproduce the above copyright notice,
#   this list of conditions and the following disclaimer
#   in the documentation and/or other materials provided with the distribution.
# * Neither the name of the <ORGANIZATION> nor the names of its
#   contributors may be used to endorse or promote products derived
#   from this software without specific prior written permission.
#  
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
# "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
# LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
# FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
# COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
# INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
# (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
# OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION
# HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
# STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
# ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED
# OF THE POSSIBILITY OF SUCH DAMAGE.
#
#
# Revision info:
# $Date $
# $Rev $
# $Author $
#

function echo_err {
   echo -n -e "\e[31m * ERR[\e[1m${2}\e[21m]: \e[39m"
   echo ${1}
}

function echo_norm {
   echo -n -e "\e[34m * TEST[\e[1m${2}\e[21m]: \e[39m"
   echo ${1}
}


ELFCLOUD_STR="1234567890abcdefghijklmpqrstuwvxyzABCDEFGHIJKLMNTOPQRSTUWVXYZ"
ELFCLOUD_DIR=${ELFCLOUD_STR:0:1}
ELFCLOUD_FIRST=${ELFCLOUD_DIR}

ELFCLOUD_TMPDIR="/tmp/elfcloud-testing"
mkdir -p ${ELFCLOUD_TMPDIR}

if [ ! -f ${ELFCLOUD_TMPDIR}/alice.txt ]; then
  wget https://www.gutenberg.org/files/19033/19033-8.txt -O ${ELFCLOUD_DIR}/alice.txt
  cp ${ELFCLOUD_TMPDIR}/alice.txt ${ELFCLOUD_TMPDIR}/alice-gz.txt
  cp ${ELFCLOUD_TMPDIR}/alice.txt ${ELFCLOUD_TMPDIR}/alice-xz.txt
  cp ${ELFCLOUD_TMPDIR}/alice.txt ${ELFCLOUD_TMPDIR}/alice-bz2.txt
  gzip -9 ${ELFCLOUD_TMPDIR}/alice-gz.txt
  xz -9 ${ELFCLOUD_TMPDIR}/alice-xz.txt
  bzip2 -9 ${ELFCLOUD_TMPDIR}/alice-bz2.txt
fi

for str in `seq 2 44`; do
  echo -e "\e[33mPattern ${str}/44 start\e[39m"
  ELFCLOUD_DIR+="/${ELFCLOUD_STR:0:${str}}"
  echo_norm "Mkdir test ${str}/44 (Dir length: ${#ELFCLOUD_DIR})" mkdir
  mkdir -p ${ELFCLOUD_DIR}
  if [ "$?" != "0" ]; then
     echo_err "Can't mkdir ${ELFCLOUD_DIR}" mkdir
  fi

  cp -v ${ELFCLOUD_TMPDIR}/* ${ELFCLOUD_DIR} > ${ELFCLOUD_DIR}/cp.out
  if [ "$?" != "0" ]; then
     echo_err "Can't cp stuff to ${ELFCLOUD_DIR} (check: ${ELFCLOUD_DIR}/cp.out)" cp
  fi

  for file in alice.txt alice-gz.txt.gz alice-xz.txt.xz alice-bz2.txt.bz2; do
     echo -e "\e[33mLoop '${file}' pattern start\e[39m"
    echo_norm "SHA256SUM file ${ELFCLOUD_DIR}/${file}" sha256sum
    sha256sum ${ELFCLOUD_DIR}/${file} >> ${ELFCLOUD_DIR}/SHA256SUM

    if [ "$?" != "0" ]; then
       echo_err "Can't sha256sum ${ELFCLOUD_DIR}/${file}" sha256sum
    fi

    echo_norm "Cat ${ELFCLOUD_DIR}/SHA256SUM" cat
    cat ${ELFCLOUD_DIR}/SHA256SUM > ${ELFCLOUD_DIR}/cat.out

    if [ "$?" != "0" ]; then
       echo_err "Can't cat ${ELFCLOUD_DIR}/SHA256SUM (check ${ELFCLOUD_DIR}/cat.out)" cat
    fi

    echo_norm "Move test: ${ELFCLOUD_DIR}/${file} ${ELFCLOUD_DIR}/${file}.new" mv
    mv ${ELFCLOUD_DIR}/${file} ${ELFCLOUD_DIR}/${file}.new

    if [ "$?" != "0" ]; then
       echo_err "Can't mv ${ELFCLOUD_DIR}/${file} to ${ELFCLOUD_DIR}/${file}.new" mv
    fi

    echo_norm "Move test: ${ELFCLOUD_DIR}/${file}.new ${ELFCLOUD_DIR}/${file}" mv
    mv ${ELFCLOUD_DIR}/${file}.new ${ELFCLOUD_DIR}/${file}
    if [ "$?" != "0" ]; then
       echo_err "Can't mv ${ELFCLOUD_DIR}/${file}.new to ${ELFCLOUD_DIR}/${file}" mv
    fi

    echo_norm "checking SHA256SUM ${ELFCLOUD_DIR}/${file}" sha256sum

    sha256sum -c ${ELFCLOUD_DIR}/SHA256SUM > ${ELFCLOUD_DIR}/sha256sum.out
    if [ "$?" != "0" ]; then
       echo_err "Can't check sha256sum ${ELFCLOUD_DIR}/SHA256SUM (check ${ELFCLOUD_DIR}/sha256sum.out)" sha256sum
    fi

    case "${file}" in
      *.gz)
        echo_norm "Gzip uncompress test: ${ELFCLOUD_DIR}/${file}" gzip
        gzip -t ${ELFCLOUD_DIR}/${file} > ${ELFCLOUD_DIR}/gzip.out
        if [ "$?" != "0" ]; then
           echo_err "Can't gzip -t ${ELFCLOUD_DIR}/${file} (check ${ELFCLOUD_DIR}/gzip.out)" gzip
        fi
      ;;
      *.xz)
        echo_norm "Xz uncompress test: ${ELFCLOUD_DIR}/${file}" xz
        xz -t ${ELFCLOUD_DIR}/${file} > ${ELFCLOUD_DIR}/xz.out
        if [ "$?" != "0" ]; then
           echo_err "Can't xz -t ${ELFCLOUD_DIR}/${file} (check ${ELFCLOUD_DIR}/xz.out)" xz
        fi
      ;;
      *.bz2)
        echo_norm "Bzip2 uncompress test: ${ELFCLOUD_DIR}/${file}" bzip2
        bzip2 -t ${ELFCLOUD_DIR}/${file} > ${ELFCLOUD_DIR}/bzip2.out
        if [ "$?" != "0" ]; then
           echo_err "Can't bzip2 -t ${ELFCLOUD_DIR}/${file} (check ${ELFCLOUD_DIR}/bzip2.out)" bzip2
        fi
      ;;
    esac

   echo -e "\e[33mLoop '${file}' pattern done\e[39m\n"

  done
done

rm -rfv ${ELFCLOUD_FIRST}
