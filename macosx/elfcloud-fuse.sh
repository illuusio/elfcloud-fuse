#!/bin/bash

APP_LOCATION="$(dirname $0)/.."
if [ "${APP_LOCATION}" == "./.." ]; then
      APP_LOCATION="$(pwd)/.."
fi

# There must be first argument, second argument and if there -psn_ it laucnhed from finder
if [ "${1}" = "" ] || [ "${2}" = "" ] || [[ "${1}" == -psn_* ]]; then
  echo "usage: ${APP_LOCATION}/Resources/bin/elfcloud-fuse your@domain.fi /your/mount/point"

  DIALOG_TEXT="Currently elfCLOUD-FUSE doesn't have GUI for mounting.\nPlease use this application from commandline:\n${APP_LOCATION}/Resources/bin/elfcloud-fuse your@domain.fi /your/mount/point"

  # Only if we are launched from Finder we show graphics
  if [[ "${1}" == -psn_* ]]; then
     osascript -e 'tell application "Finder"' -e 'activate' -e "display dialog \"${DIALOG_TEXT}\"" -e 'end tell' > /dev/null
  fi

  exit 1
fi

echo "This is wrapper script for Mac OS X elfCLOUD-FUSE. Binary is located: '${APP_LOCATION}/Resources/bin/elfcloud-fuse'"

${APP_LOCATION}/Resources/bin/elfcloud-fuse ${1} ${2} ${3} ${4} ${5} ${6} ${7} ${8}

