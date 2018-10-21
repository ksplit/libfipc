#!/bin/bash
# @Description: This script turns on/off the sibling thread
# 

usage() { echo "Usage: $0 [-o <0|1>] [-h]" 1>&2; exit 1; }

ht_set() {
  if [ $1 == 0 ]; then
    echo 'thread off'
    # off -> off
    if cat /sys/devices/system/cpu/offline | grep '-'; then
      echo 'HT was already off'
      exit 1
    fi
    # on -> off
    cd /sys/devices/system/cpu
    cat ./cpu*/topology/thread_siblings_list | sort | uniq |
    while read -r line; do
      IFS=',' read -ra CORE <<< "$line"
      cd cpu${CORE[1]}
      echo 1 | sudo tee online
      cd ..
    done 
    exit 1
  fi
  
  if [ $1 == 1 ]; then
    echo 'thread on'
    # off -> on
    if cat /sys/devices/system/cpu/offline | grep '-'; then
      cd /sys/devices/system/cpu
      IFS='-' read -ra CORE <<< "$(cat /sys/devices/system/cpu/offline)"
      for i in $(seq ${CORE[0]} ${CORE[1]}); do
        cd cpu$i
        sudo chmod 777 online
        echo 1 > online
        sudo chmod 644 online
        cd ..
      done
      exit 1
    fi
    # on -> on
    echo 'HT was already on'
    exit 1
  fi
}

while getopts ":o:" opt; do
  case ${opt} in
    o)
      # -o 1: turn on sibling thread
      # -o 0: turn off sibling thread
      ht_set $OPTARG
      
      # wrong option
      echo 'wrong option'
      usage
      ;;
    ?)
      usage
      ;;
   esac
done
