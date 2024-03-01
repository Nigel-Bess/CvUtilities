#!/bin/bash

set -o errexit -o pipefail -o noclobber -o nounset
# script to start up mongo db container for factory testing
USERNAME="prod"
MONGO_CONF="$HOME/.mongodb/connections/.pioneer_connection.yaml"

AGRC=$#
echo "There are $AGRC arguments to the script"
if  [[ $# -gt 0 ]]; then
  ! getopt --test > /dev/null
  if  [[ ${PIPESTATUS[0]} -ne 4 ]]; then
    echo 'Host lacks necessary version of getopt required to use command line options.'
    echo 'Please directly edit variables USERNAME (for remote db) and MONGO_CONF (path to the .yaml defining password and uri)'
    exit 1
  fi
  LONGOPTS=username:,password:,hostname:,config:
  OPTIONS=u:,p:,h:,c:

  ! PARSED=$(getopt --options=$OPTIONS --longoptions=$LONGOPTS --name "$0" -- "$@")
  if [[ ${PIPESTATUS[0]} -ne 0 ]]; then
      # e.g. return value is 1
      #  then getopt has complained about wrong arguments to stdout
      exit 2
  fi
  # read getopt’s output this way to handle the quoting right:
  eval set -- "$PARSED"

  USER_DEF_CONF=
  PASSWORD=
  MONGO_SERVER=

  # now enjoy the options in order and nicely split until we see --
  while true; do
    echo "Arg $1"
      case "$1" in
          -u|--username)
              USERNAME=$2
              shift 2
              ;;
          -p|--password)
              PASSWORD=$2
              shift 2
              ;;
          -h|--hostname)
              MONGO_SERVER=$2
              shift 2
              ;;
          -c|--config)
              MONGO_CONF="$2"
              USER_DEF_CONF="true"
              shift 2
              ;;
          --)
              shift
              break
              ;;
          *)
              echo "Programming error"
              exit 3
              ;;
      esac
  done

  echo "udf=$USER_DEF_CONF,ms=$MONGO_SERVER,pw=$PASSWORD,conf=$MONGO_CONF"

  if [[ -z "$USER_DEF_CONF" ]] && { [[ -z "$MONGO_SERVER" ]] || [[ -z "$PASSWORD" ]] ; } ; then
    echo "If command line args are given, there must either be a user config path,"\
    "or the mongo server hostname and password (a username can be provided to override the default)."
    exit 4
  fi
fi


function get_db_collection() {
  echo "${1}" | sed -n 's/\([^.]*\)\.*\([^.]*\)\.*/-d \1 -c \2/gp' | sed 's/ -c\s*$//g'
}


if [[ $AGRC -gt 0 ]] && [[ -z "$USER_DEF_CONF" ]] ; then
  echo "Using credentials from command line input!"
  CONNECTION_STR="-p $PASSWORD -h $MONGO_SERVER:27017"

else
  [[ $AGRC -gt 0 ]] && echo -e "Using config .yaml defined by command line input.\nPath:\n  ${MONGO_CONF}" || \
  echo -e "Using config yaml at the default path location.\nPath:\n  ${MONGO_CONF}"
  CONNECTION_STR="--config ${MONGO_CONF}"
  MONGO_SERVER="mdb-*.pioneer.fulfil.ai:27017"
fi 

#db_names=( 'Machines.DepthCameras' 'Logging.TrayCounts' 'Inventory.TrayValidationData' )
#db_names=( 'Recipes.TrayCalibrations' 'Machines.VLSs' 'Machines.DepthCameras' 'Inventory.BagStates' )
db_names=( 'Recipes.TrayCalibrations' 'Machines.MarsDispenses' 'Machines.DepthCameras' 'Machines.VLSs' )



for i in "${db_names[@]}"; do
  data_str="$(get_db_collection "$i")"
  echo -e "Copying records off the factory mongo server from $i "
  mongodump -u $USERNAME $CONNECTION_STR --tlsInsecure --ssl --authenticationDatabase=admin --readPreference=primary $data_str -o local_mongo_data/dump
done

if docker ps | grep mongodb  ; then
  echo "Found running mongo instance!"
elif docker ps -a | grep mongodb  ; then
  echo "Restarting existing mongo container!"
  docker start mongodb
else
  echo "Building mongo container"
  docker pull mongo
  docker run -d -p 27018:27017 --name mongodb mongo
fi


TIMEOUT=15
while [ "$(docker ps | grep mongodb)" = "" ] && [ $TIMEOUT -gt 0 ]; do
    echo "Waiting for mongodb start-up. $TIMEOUT tries left..."
    sleep 2
    # shellcheck disable=SC2219
    let TIMEOUT-=1
done 

for i in "${db_names[@]}"; do
  record="$(echo "$i" | sed -e 's/\.\(..*\)/\/\1.bson/g')"
  echo "Uploading $i from file $record to docker mongodb"
  data_str="$(get_db_collection $i)"
  mongorestore -h localhost:27018 --drop $data_str "local_mongo_data/dump/$record"
done


echo "Transfer from $MONGO_SERVER to local host container complete!"

