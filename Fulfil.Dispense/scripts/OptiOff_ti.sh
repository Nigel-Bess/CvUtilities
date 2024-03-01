#!/bin/bash

RED='\033[0;31m'
GRN='\033[0;32m'
LIM='\033[1;32m'
CYN='\033[0;36m'
NC='\033[0m'

# safer duration is 1 minutes
DOWN_TIME="40 seconds"


echo -e "${RED}==== Self destruct sequence initiated ===="
echo -e "Deep shutdown scheduled for $(date)${NC}"
echo -e "${LIM}Machine will start boot after $DOWN_TIME, start-up adds ~40 seconds${NC}"
echo -e "${CYN}Searching for available Realsense cameras prior to restart...${NC}"
echo -e "${GRN} $(rs-fw-update -l) ${NC}"

WAKE_TIME=$(date '+%s' -d '+ '"$DOWN_TIME")
sudo hwclock -w && \
sudo bash -c "echo 0 > /sys/class/rtc/rtc0/wakealarm" && \
sudo bash -c "echo $WAKE_TIME > /sys/class/rtc/rtc0/wakealarm" && \
sudo shutdown 0


