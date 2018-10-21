#!/bin/bash
# @Description: This script turns on/off the sibling thread
# 

usage() { echo "Usage: $0 [-o <0|1>] [-h] [-s]" 1>&2; exit 1; }

ht_show() { echo "$(cat /sys/devices/system/cpu/cpu*/topology/thread_siblings_list | sort | uniq)"; exit 1;}

ht_set() {
  sudo chmod 777 online
  echo $1 > online
  sudo chmod 644 online
}

ht_on() {
  echo '[sibling thread on]'
  # off -> on
  if cat /sys/devices/system/cpu/offline | grep '-'; then
   cd /sys/devices/system/cpu
    IFS='-' read -ra PRESENT <<< "$(cat ./present)"
    IFS=',' read -ra OFFLINE <<< "$(cat ./offline)"

    # POSSIBLE CPUs might be different from PRESENT CPUs
    for i in ${OFFLINE[@]}; do 
      IFS='-' read -ra LIST <<< "$i"
      for i in $(seq ${LIST[0]} ${LIST[1]}); do
        if (( $i < ${PRESENT[1]} )); then 
          cd cpu$i
          ht_set 1 
          cd ..
        fi
      done
    done
  fi
  exit 1
}

ht_off() {
  echo '[sibling thread off]'
  # on -> off
  cd /sys/devices/system/cpu
  cat ./cpu*/topology/thread_siblings_list | sort | uniq |
  while read -r line; do
    IFS=',' read -ra CORE <<< "$line"
    if [ ! -z ${CORE[1]} ];then
      cd cpu${CORE[1]}
      ht_set 0
      cd ..
    fi 
  done 
  exit 1
}

while getopts ":hso:" opt; do
  case $opt in
    o)
      # -o 1: turn on sibling thread
     if [ $OPTARG == 1 ]; then
        ht_on
      fi
       
      # -o 0: turn off sibling thread
      if [ $OPTARG == 0 ]; then
        ht_off
      fi
      
      # wrong option
      echo 'wrong option'
      usage
      ;;
    s)
      ht_show
      ;;
    *)
      usage
      ;;
   esac
done
