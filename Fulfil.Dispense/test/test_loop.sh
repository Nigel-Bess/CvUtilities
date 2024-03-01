#!/bin/bash

NUM_LOOP=${1:-1}

echo -e "\nRun request loop $NUM_LOOP times!\n"

for r in $(seq 1 $NUM_LOOP ) ; do
  ../build/app/dispense_test_client get_state
  ../build/app/dispense_test_client drop_target
  ../build/app/dispense_test_client pre_LFR
  ../build/app/dispense_test_client pre_tray
  ../build/app/dispense_test_client start_tray_video
  ../build/app/dispense_test_client start_lfb_video
  ../build/app/dispense_test_client pre_tray
  ../build/app/dispense_test_client post_LFR
  ../build/app/dispense_test_client tray_validation
  echo "Extend Video for 3 seconds"
  echo; for i in $(seq 1 3 ) ; do printf "%s..." "$i" ; sleep 1 ; done ; echo
  ../build/app/dispense_test_client stop_tray_video
  ../build/app/dispense_test_client stop_lfb_video
done