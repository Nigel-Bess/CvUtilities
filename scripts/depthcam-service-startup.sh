#!/bin/bash

function script_directory()
{
    f=${0%/*}
    if [ -d "$f" ]; then
        dir="$f"
    else
        dir=$(dirname "$f")
    fi
    dir=$(cd "$dir" && /bin/pwd)
    echo "$dir/"
}

DUMPDIR=/var/lib/apport/coredump

function LATEST_COREOUT {  ls -Art $DUMPDIR | grep Fulfil_Dispense_build_app_main | tail -n 1 | xargs -I % echo $DUMPDIR/%; }
function GETLOGTIME { date -d "${1} minutes ago" "+%b %_d %H:%M"; }
function TIME_SUFFIX { date -d "${1} minutes ago" +%Y-%m-%d_%H-%M-%S-UTC%z; }



PATH_TO_SCRIPT=$( script_directory )

echo "Running startup from script at $PATH_TO_SCRIPT"


CRASH_INFO="${PATH_TO_SCRIPT}rs-core-logs"
[ ! -d $CRASH_INFO ] && mkdir $CRASH_INFO
cd $CRASH_INFO

APP_PATH="build/app/main"
LOG_OBS_WINDOW=15 # in minutes
SERVICE_START="$(date "+%b. %0d, %Y at %H:%M:%S UTC%z")"
SERVICE_LOG="${CRASH_INFO}/service_reports_$(date +%Y-%m-%d).log"
NON_CAMERA_CRASH_LIMIT=10 # number of times the service can crash with the same error for a non-camera related reason before pausing restart

SERVICE_NAME="DepthCam Dispense API"
SLACK_WEBHOOK=$(cat "${PATH_TO_SCRIPT}configs/secret/slack_api_credentials")

SLACK_SCRIPT="$(dirname  "$PATH_TO_SCRIPT" )/Fulfil.TrayCountAPI/scripts/slack_alerts.sh"
echo "Looking for slack functionality at $SLACK_SCRIPT"
source "$SLACK_SCRIPT" "$SLACK_WEBHOOK" "$SERVICE_NAME"

KERNLOG="${CRASH_INFO}/kernel_logs_$(TIME_SUFFIX 0).log"
END="$(GETLOGTIME 0)" # at termination of program get time for kernel logs
START="$(GETLOGTIME ${LOG_OBS_WINDOW})" # go LOG_OBS_WINDOW minutes back in kernel logs

function GET_NUM_DC_FAILS { $(grep CameraStatus ${SERVICE_LOG} | tail -n 3 | grep NoCamerasConnected | wc -l); }

function get_recent_kern_log {
    local startline; startline=$(grep -m 1 -n "${START}" /var/log/kern.log | cut -f1 -d: | grep [0-9])
    local lines_found=$?
    if [[ ${lines_found} -eq 0 ]]; then
        echo -e "Creating kernel log file ${KERNLOG}" | tee -a "$SERVICE_LOG"
        tail --lines=+"${startline}" /var/log/kern.log >> "${KERNLOG}"
    else
        echo -e "Failed to find entries in kern log during obs window! File ${KERNLOG} will not be generated." | tee -a "$SERVICE_LOG"
    fi
}

function clear_redundant_kernel_logs {
    file_prefix="kernel_logs_"
    for f in $(ls -Art "$CRASH_INFO" | grep -E "$file_prefix")
        do
        if [[ $f > "${file_prefix}$(TIME_SUFFIX $LOG_OBS_WINDOW)" ]] ; then
           rm "${CRASH_INFO}/${f}"
           echo -e "Deleted redundant kernel log ${f}" | tee -a "$SERVICE_LOG"
        fi
    done
}

function send_notification_msg {
   echo "Sending slack Alert!"
   local msg="${1}"
   local alert="${2}" # can be null
   make_msg_header_section "$msg" "${alert}"
   close_and_send_message
}

function check_cameras {
    local rs_cameras="$(rs-fw-update -l)"
    CONNECTED_CAMERAS=$(echo -e "$rs_cameras" | grep -oP "\d\) Name: Intel RealSense \w\d\d\d, serial number: \K\d+")
    local cam_status=0
    if [[ "" != "$CONNECTED_CAMERAS" ]]; then
        echo "Found cameras ( $( echo -e "$CONNECTED_CAMERAS" | tr '\n' ' ' )) connected to device!"
        SERVICE_STARTUP_MSG="${SERVICE_STARTUP_MSG} Cameras detected on device:\n$( echo -e "$rs_cameras" | cut -d, -f 1,2,5)"
        echo -e "$(echo -e "$rs_cameras")" >> "$SERVICE_LOG"
        local cam_match_count=0
        local cam_detected_count=0
        for cam in $(echo $CONNECTED_CAMERAS) ; do
            local cam_match=$(grep $cam "${PATH_TO_SCRIPT}configs/AGX_specific_main.ini")
            cam_detected_count=$(expr $cam_detected_count + 1)
            if [[ $cam_match != "" ]] ; then
                cam_match_count=$(expr $cam_match_count + 1)
            fi
        done

        if [[ $cam_detected_count -gt $cam_match_count ]] ; then
            SERVICE_STARTUP_MSG="${SERVICE_STARTUP_MSG}\n*Registering $(( $cam_detected_count - $cam_match_count )) more cameras* than what was in config files, indicating a *camera swap on bay!* Check configs!"
            cam_status=11
            if [[ $cam_match_count -lt 2 ]]; then
                SERVICE_STARTUP_MSG="${SERVICE_STARTUP_MSG}\n*Warning: DepthCam Death!* Possible camera swaps have forced a camera not found error!\n"
                cam_status=33
            fi
            echo -e "$SERVICE_STARTUP_MSG" | tee -a "$SERVICE_LOG"
        fi
        echo -e "\nCameraStatus: $(echo "$cam_detected_count Connected, $cam_match_count Matches")" | tee -a "$SERVICE_LOG"
        return ${cam_status}
    else
        cam_status=23
        echo -e "Warning! No cameras found connected to device!" | tee -a $SERVICE_LOG
        # Disable service or perform shutdown procedure
        echo -e "\nCameraStatus: NoCamerasConnected" >> $SERVICE_LOG
        NUM_DC_CON_FAIL=$(grep CameraStatus "${SERVICE_LOG}" | tail -n 3 | grep -E "NoCamerasConnected" | wc -l)
        RECENT_CAM_CONNECT=$(grep CameraStatus "${SERVICE_LOG}" | tail -n 3 | grep -m1 -n -v "NoCamerasConnected" | grep -oP "^\d+")
        if [[ "" != "$RECENT_CAM_CONNECT" ]]; then
                  NUM_DC_CON_FAIL=$((NUM_DC_CON_FAIL-RECENT_CAM_CONNECT+1))
        fi
        SERVICE_STARTUP_MSG="${SERVICE_STARTUP_MSG} *No Depth Cameras detected*. There have been $NUM_DC_CON_FAIL consecutive failed attempts to reboot and reconnect with the camera out of the last 3!"
        echo -e "$SERVICE_STARTUP_MSG" >> "$SERVICE_LOG"
        if [[ "$NUM_DC_CON_FAIL" -ge "3" ]] ; then
            echo "After 3 or more consecutive failures, service will no longer attempt to start depthcams without operator intervention." | tee -a $SERVICE_LOG
            SERVICE_STARTUP_MSG="${SERVICE_STARTUP_MSG}\n*Warning: DepthCam Death!* 3 or more consecutive failed attempts to connect to camera after restart is a critical fail. Service is disabled until engineer can check camera connections."
            return 33
	      fi
	      return ${cam_status}
    fi
    return ${cam_status}
}

function check_exits {
  regex='tmux shell exited with status: ([2-9]|[0-9][0-9]+)'
  idx=0
  prev=0
  while read -r line; do
      if [[ $line =~ $regex ]]; then
        if [[ $idx -ne 0 ]]; then
          if [[ ${BASH_REMATCH[1]} -ne $prev ]]; then
            return 0
          fi
        fi
        prev=${BASH_REMATCH[1]}
      else
        return 0
      fi
      ((idx=$idx + 1))
  done < <(tac "$SERVICE_LOG" | sed '/Auto-restart of depthcam service paused/q' | tac | grep "tmux shell exited with status:" | tail -n $NON_CAMERA_CRASH_LIMIT )
  if [ $idx -ge "${NON_CAMERA_CRASH_LIMIT}" ] ; then
    echo "After ${NON_CAMERA_CRASH_LIMIT} or more consecutive failures error code $prev, service will no longer attempt to start depthcams without operator intervention." | tee -a $SERVICE_LOG
    echo "Auto-restart of depthcam service paused" | tee -a $SERVICE_LOG
    return ${prev}
  else
    return 0
  fi
}

TMUX_NAME="depthcam"

# executed before program start
echo -e "-------------------------\n" >>  "$SERVICE_LOG"
echo -e "Service application bring up started on ${SERVICE_START}\n"  >>  "$SERVICE_LOG"
SERVICE_STARTUP_MSG="Dispense service start requested!"

check_cameras
CAM_CHECK=$?

if [[ $CAM_CHECK -eq 0 ]]; then
    make_msg_header_section "$SERVICE_STARTUP_MSG"
    add_new_msg_body_section "Camera check completed without issue! Attempting service run! :muscle: "
    close_and_send_message
    echo "Camera check and Dispense routine complete. Service start pending."
elif [[ $CAM_CHECK -lt 30 ]]; then
    sleep 1
    make_msg_header_section "$SERVICE_STARTUP_MSG" "Amber"
    add_new_msg_body_section "Camera check completed. *Non-fatal issues found with cameras.*  :face_with_head_bandage: Attempting service run after brief sleep!"
    close_and_send_message
    echo "Camera check and Dispense routine complete. Service start pending."
else
    make_msg_header_section "$SERVICE_STARTUP_MSG :sob:" "here"
    add_new_msg_body_section "Camera check completed with *fatal issues found on cameras*. Service will be disabled! :skull_and_crossbones: "
    close_and_send_message
    echo "Fatal depthcam errors encountered. Service stopped pending engineer check!"
    exit 33
fi

declare -A errors
errors["3"]="VLS Mongo IDs Not Properly Defined"
errors["4"]="No Cameras Found"
errors["5"]="Neither Bay Meets Starting Condition"
errors["6"]="Issue building Sensors, Bays or Runners"
errors["10"]="No Color Data Found"
errors["11"]="No Depth Data Found"
errors["12"]="No Aligned Depth Data Found"
errors["13"]="Could not connect to Trinamic motion control board"
errors["14"]="Motion control parameters did not load properly"
errors["15"]="Received motion request but motor not configured to run"
errors["18"]="Error saving images - check disk space"
errors["19"]="Error saving videos - check disk space"

check_exits
EXIT_CHECK=$?
#EXIT_CHECK=0
if [[ $EXIT_CHECK -ne 0 ]]; then
  make_msg_header_section "$SERVICE_STARTUP_MSG :sob:" "here"
  add_new_msg_body_section "Exit code check completed with *fatal issues found*. Service will be disabled! :skull_and_crossbones: "
  add_new_msg_body_section "Service will no longer restart automatically due to $NON_CAMERA_CRASH_LIMIT consecutive occurrences of error code $EXIT_CHECK: ${errors[$EXIT_CHECK]}"
  close_and_send_message
  echo "Fatal depthcam errors encountered. Service stopped pending engineer check!"
  exit 33
fi

echo "Run application $PATH_TO_SCRIPT$APP_PATH"

OLD_CORE="$( LATEST_COREOUT )"

tmux new -s "$TMUX_NAME" -n disp -d "trap 'tmux wait-for -S dispense_term' EXIT SIGINT ; cd $CRASH_INFO ; ulimit -S -c unlimited $PATH_TO_SCRIPT$APP_PATH ; $PATH_TO_SCRIPT$APP_PATH"
tmux set-option remain-on-exit on
pane=$(tmux display-message -p -t "$TMUX_NAME" "#{pane_id}")
echo "Dispense API tmux shell has been started!"
tmux wait-for dispense_term
status=$(tmux display-message -p -t "$pane" "#{pane_dead_status}")
tmux kill-session -t $TMUX_NAME

make_msg_header_section "*ATTENTION: $SERVICE_NAME has exited!*  If the service is enabled, the downed session will restart shortly!\n"
add_new_msg_body_section "*Service Restart Pending...*"
close_and_send_message

# executed after program stop
echo "Dispense API tmux shell has exited!"
echo -e "tmux shell exited with status: $status" | tee -a "$SERVICE_LOG"

CRASH_TIME="$(TIME_SUFFIX 0)"
NEW_CORE="$( LATEST_COREOUT )"
echo -e "\nThere are currently $(ls $DUMPDIR | grep -c Fulfil_Dispense_build_app_main ) coredump files on disk." | tee -a  "$SERVICE_LOG"
if [[ "$OLD_CORE" != "$NEW_CORE" ]] ; then
    CORE_OUT_CRASH_INFO="${CRASH_INFO}/debugger_core_out_$CRASH_TIME.txt"
    echo -e "Found core out at $NEW_CORE! Likely due to uncaught crash! Creating debugger core out file $CORE_OUT_CRASH_INFO" | tee -a "$SERVICE_LOG"
    gdb -ex bt -ex quit "${PATH_TO_SCRIPT}${APP_PATH}" -c "$NEW_CORE" > "$CORE_OUT_CRASH_INFO"
else 
    echo -e "Did NOT find new core out! Safe exit from application?" | tee -a  "$SERVICE_LOG"
fi


echo -e "Service was started on ${SERVICE_START} and dispense code completed on $(date "+%b. %0d, %Y at %H:%M:%S UTC%z")." | tee -a  "$SERVICE_LOG"
echo "Collecting kernel logs from ${START} to ${END}" | tee -a "$SERVICE_LOG"
clear_redundant_kernel_logs # delete redundant kernel logs (end falls within obs window)
get_recent_kern_log # create new kernel log snippet.



exit 0

