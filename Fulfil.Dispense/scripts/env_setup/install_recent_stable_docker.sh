#!/bin/bash

echo -e "********************************************"
echo -e "*** Begin Docker Install (latest stable) ***"
echo -e "********************************************"

echo "Removing old installations of Docker on system to avoid conflicts."
sudo apt-get remove -y docker docker-engine docker.io containerd runc

echo -e "\nInstalling the Docker dependencies"
sudo apt-get update && sudo apt-get install -y \
    apt-transport-https \
    ca-certificates \
    curl \
    gnupg \
    lsb-release


echo -e "\nGetting Docker GPG key"
docker_gpg="/usr/share/keyrings/docker-archive-keyring.gpg"
[ -f "$docker_gpg" ] && sudo mv "$docker_gpg" "$docker_gpg.old" && sudo apt-get update
curl -fsSL https://download.docker.com/linux/ubuntu/gpg | sudo gpg --dearmor -o $docker_gpg

echo \
"deb [arch=amd64 signed-by=/usr/share/keyrings/docker-archive-keyring.gpg] https://download.docker.com/linux/ubuntu \
$(lsb_release -cs) stable" | sudo tee /etc/apt/sources.list.d/docker.list > /dev/null

echo -e "\nInstalling Docker repos"
sudo apt-get update && sudo apt-get install -y docker-ce docker-ce-cli containerd.io

# Docker Group creation (allow non-root user to run)
NEW_GROUP="docker"
echo -e "\nValidating that non-root user \"$USER\" is able to run docker commands!"
[ "$(getent group $NEW_GROUP)" != "" ] && echo "Group \"$NEW_GROUP\" already exists!" || sudo groupadd $NEW_GROUP
[ -z "$(id -nG "$USER" | grep -E "\b$NEW_GROUP\b")" ] && sudo usermod -aG $NEW_GROUP $USER || echo "User \"$USER\" is already in group \"$NEW_GROUP\""

PERMISSION_ERR="Error loading config file: /home/$USER/.docker/config.json"
INSTALL_SUCCESS_MSG="your installation appears to be working correctly"
DAEMON_ERR="Cannot connect to the Docker daemon"

function verify_docker_config_permissions {
  local docker_config=/home/"$USER"/.docker
  local permissions=7

  if [ -z "$1" ]; then
    echo 'Docker installation test output was empty! Try testing commands in interactive shell and if '\
    'that fails rebooting your machine. Enjoy debugging this script!!!'
    exit 55
  fi

  local docker_test_out="${1}"
  local daemon_error=$(echo -e "$docker_test_out" | grep "$DAEMON_ERR")

  if [ ! -z "$daemon_error" ] ; then
    echo 'Issue connecting to docker daemon! Run the script again. You may need to reload '\
    'the shell. Docker error output:'
    echo "--------------------------"
    echo -e "$docker_test_out" | grep "$DAEMON_ERR"
    echo -e "--------------------------\n"
    exit 66
  fi

  local permissions_error="$(echo -e "$docker_test_out" | grep "$PERMISSION_ERR")"
  if [ -z "$permissions_error" ]; then
    echo "Congrats! No permissions error was thrown when user \"$USER\" performed a docker command!"
    ((permissions=permissions-1))
  else
    echo -e 'Detected config load error for user "'"$USER"'". Error thrown by Docker:\n\t'"$permissions_error"'!'
    echo -e 'This is most likely caused by attempting to use docker as root before configuring user group access. '\
    'Updating config file permissions assuming default path '"$docker_config"'\n'
    sudo chown "$USER":"$USER" "${docker_config}" -R
    sudo chmod g+rwx "$HOME/.docker" -R
  fi
  if [ -e "$docker_config" ]; then
    [ -w "$docker_config" ] && ((permissions=permissions-2)) || echo "*** Warning *** user still lacks write access!"
    [ -r "$docker_config" ] && ((permissions=permissions-4)) || echo "*** Warning *** user still lacks write access!"
  else
      echo "Due to setup procedure root docker config folder does not exist. No permissions adjustments necessary."
      ((permissions=permissions-6))
  fi
  if [ $permissions -le 1 ]; then
      echo "Docker config read/write permissions set properly for user \"$USER\"!"
  else
      echo 'Issue with read/write docker config permissions for non-root user '"\"$USER\""'! '\
      'Exiting with code '"$permissions"'! Monitor installation for issues.'
  fi
  return $permissions
}

# shellcheck disable=SC2120
function get_docker_grp_output {
  local docker_cmd="${1:-"docker run hello-world"}"
  local cmd_callable="\\\$($docker_cmd 2>&1 )"
  [ $(id -gn) != "$NEW_GROUP" ] && echo "Current user \"$USER\" is running as group \"$(id -gn)\". Changing group to \"$NEW_GROUP\" to run installation tests."
  local grp_out=$(newgrp $NEW_GROUP << RUNASNEWGRP
  [ \$(id -gn) != $NEW_GROUP ] && echo "There was an issue setting user \"\$USER\" to run as group \"$NEW_GROUP\" (currently running as \"\$(id -gn)\"). Please reboot your computer and try again! If issues persist reach out to your friendly neighborhood linux dev (probably Amber)." || echo "User \"\$USER\" is now set to run as group \"\$(id -gn)\"."
  echo "Testing installation by running command ( $docker_cmd )..."
  echo "--------------------------"
  echo "\\$cmd_callable"
  echo -e "--------------------------\n"
RUNASNEWGRP
)
  echo -e "$grp_out"
}

# Get output of docker command using docker group
DOCKER_TEST_OUT="$(echo -e "$(get_docker_grp_output)")"
echo -e "$DOCKER_TEST_OUT"

usr_config_access="$(verify_docker_config_permissions "$DOCKER_TEST_OUT")"
usr_permission_code=$?
echo -e "\n$usr_config_access\nDocker config permission status code is $usr_permission_code!"

if [ "$usr_permission_code" != 0 ]; then
  echo "Docker group setup returned with a non-zero error code. Rerunning routine for validation!"
  DOCKER_TEST_OUT="$(echo -e "$(get_docker_grp_output)")"
  usr_config_access="$(verify_docker_config_permissions "$DOCKER_TEST_OUT")"
  usr_permission_code=$?
  echo "Permission status code after a second pass at group setup is $usr_permission_code!"
  [  "$usr_permission_code" == 0 ] && echo 'Non-root user '"\"$USER\""' appears to have the correct permissions. '\
  'Setup exited with status '"$usr_permission_code"'! Rebooting your computer is still recommended after script '\
  'is complete!' || echo 'Non-root user exited with a non-zero error code on second pass. Try rebooting the machine. If the only '\
  'error is due to config permissions, you can delete /home/'"$USER"'/.docker (but you will lose any custom settings).'
fi
FOUND_SUCCESS_MSG=$(echo -e "$DOCKER_TEST_OUT" | grep "$INSTALL_SUCCESS_MSG")
[ -z "$(echo -e "$FOUND_SUCCESS_MSG")" ] && echo -e '\nPOTENTIAL INSTALL ERROR: Failed to detect '\
'success message ('"$INSTALL_SUCCESS_MSG"'). Please check output and rerun test command.\n' || echo -e '\nSuccess '\
'message found! Please review the command output to ensure no unexpected issues occurred.\n'


echo -e "$DOCKER_TEST_OUT"
echo -e "###### Docker installation complete! ######\n"
newgrp $NEW_GROUP
