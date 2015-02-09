#!/bin/bash

APP_LOCATION="$(dirname $0)/.."
if [ "${APP_LOCATION}" == "./.." ]; then
      APP_LOCATION="$(pwd)/.."
fi

echo "This is wrapper script for Mac OS X elfCLOUD-FUSE. Binary is located: '${APP_LOCATION}/Resources/bin/elfcloud-fuse'"

${APP_LOCATION}/Resources/bin/elfcloud-fuse ${1} ${2} ${3} ${4} ${5} ${6} ${7} ${8}

