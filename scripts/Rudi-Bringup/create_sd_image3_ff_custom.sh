#!/bin/bash

# Fulfil Custom Edit on top of script
# wget https://connecttech.com/ftp/dropbox/extflash5.tgz
# instructions found here:
# https://connecttech.com/resource-center/kdb377-booting-off-external-media-cti-jetson-carriers/#1689702992608-171b0591-809f

# Copyright (c) 2020-2021, ConnectTech.  All Rights Reserved.
#
# For Help please contact support@connecttech.com
#
# This script prepares an sd card with rootfs. With this, target system
# can be run with filesystem on SD Card.
 
set -e

function usage()
{
    echo "Usage:"
    echo "  Run the script in 'Linux_for_Tegra' directory"
    echo
    echo "  ${script_name} <sd_device> <carrier_profile>"
    echo "      sd_device   - path to sd device, ex: /dev/sde"
    echo "      carrier_profile - path to your carrier profile, ex: cti/xavier-nx/photon/base"
    echo "  To find your sd_device you can use any of the following" 
    echo "      fdisk -l" 
    echo "      The linux application called 'disks'" 
    echo
    echo "  Example: ${script_name} /dev/sde cti/xavier-nx/photon/base"
    exit 1
}

function progress_bar()
{
    dir_old=`du -bs ${mntdir} | cut -f 1`
    sleep 2
    while :
    do
        dir_now=`du -bs ${mntdir} | cut -f 1`
        echo -n -e "Size: "${dir_now}'\r'

        if [ $dir_old -eq $dir_now ]; then
            echo "Copying done. Syncing..."
            break
        fi

        dir_old=$dir_now
        sleep 0.05
    done
}

function create_gpt()
{

    echo -e "${FC_CY}-------------------------------------------------"
    echo "Step 1:   Creating parition:"
    echo -e "-------------------------------------------------${FC_NC}"

    echo "Are you sure you would like to delete ${sd_dev}?"
    echo "Press any key to proceed. Press CTRL+C to exit"
    read
    sgdisk -Z -o -n 1 -t 8300 -c 1:"PARTLABEL" ${sd_dev}

    echo
    echo "Following are the partitions if created successfully"
    ls ${sd_dev}*
    echo
}


function print_blkid()
{

    echo -e "${FC_CY}-------------------------------------------------"
    echo "Step 2: Printing PARTUUID for the device:"
    echo -e "-------------------------------------------------${FC_NC}"

    blkid ${sd_dev_p}

    dev_uuid=$(blkid -o value -s PARTUUID ${sd_dev_p})

    echo ${dev_uuid} 

    echo
    echo "PARTUUID=${dev_uuid} saved to ${bootloader_dir}/l4t-rootfs-uuid.txt and ${bootloader_dir}/l4t-rootfs-uuid.txt_ext"
    echo
}

function create_blkid()
{
    
    echo -e "${FC_CY}-------------------------------------------------"
    echo "Step 2: Creating PARTUUID for the device:"
    echo -e "-------------------------------------------------${FC_NC}"

    blkid ${sd_dev_p}

    dev_uuid=$(blkid -o value -s PARTUUID ${sd_dev_p})

    echo ${dev_uuid} > ${bootloader_dir}/l4t-rootfs-uuid.txt_ext
    echo ${dev_uuid} > ${bootloader_dir}/l4t-rootfs-uuid.txt
    sync

    echo
    echo "PARTUUID=${dev_uuid} saved to ${bootloader_dir}/l4t-rootfs-uuid.txt and ${bootloader_dir}/l4t-rootfs-uuid.txt_ext"    
    echo
}

function copy_rootfs()
{
    echo -e "${FC_CY}-------------------------------------------------"
    echo "Step 3: Copying rootfs:"
    echo -e "-------------------------------------------------${FC_NC}"
    
	if mount | grep ${sd_dev_p} > /dev/null; then
		umount ${sd_dev_p}
	fi

    if mount | grep ${mntdir} > /dev/null; then
        umount ${mntdir}
    fi

    mkfs.ext4 ${sd_dev_p}

    echo "Mounting the device to /mnt/cti"    
    mkdir -p ${mntdir}

    mount ${sd_dev_p} ${mntdir}

    if mount | grep ${mntdir} > /dev/null; then
        echo "SD card mounted"
    else
        echo "Failed to mount sd card"
        exit 1
    fi

    #echo -e "Performing preflash to ensure sig files get copied..."
    #echo -e "./flash_ext.sh ${1} external"
    #./flash_ext.sh ${1} external
    #echo -e "Flash succesful"

    echo -e "\033[5mCopying rootfs. It takes a while...\033[0m"

    progress_bar &

    cd ${rfs_dir}
    tar -cpf - * | ( cd ${mntdir} ; sudo tar -xpf - )
    sync
    umount ${mntdir}
    echo "Unmounted Media"
    echo "Success: Created media with rootfs. Safe to remove media."

}

function check_req()
{
    if [ -z  ${sd_dev} ]
    then
        echo "No arguments"
        echo
        usage
        exit 1
    fi

    if [ ! -b  ${sd_dev} ]
    then
        echo "sd device not found"
        exit 1
    fi


    if [ ! -d ${rfs_dir} ]
    then
        echo "rootfs does not exist at ${rfs_dir}, make sure that you have properly mounted .img file and check path!"
        exit 1
    fi
    
    if ! type gdisk > /dev/null; then
        echo "Parition tool gdisk does not exist.Install gdisk."
        exit 1
    fi
	
	if ! type sgdisk > /dev/null; then
        echo "Parition tool gdisk does not exist.Install gdisk."
        exit 1
    fi

    if ! type blkid > /dev/null; then
        echo "Tool blkid does not exist. Install blkid"
        exit 1
    fi

    if ! type mkfs.ext4 > /dev/null; then
        echo "ext4 build filesystem tool does not exist. Install mkfs"
        exit 1
    fi
}
function print_ver()
{
    echo "name: ${script_name}"
    echo "version: ${ver}"
}

#-------------------------------------------#
#               Main                        #
#-------------------------------------------#

#version
ver=cti-v0.2-fulfil-custom

sd_dev=${1}
username=${2}
password=${3}
hostname=${4:-rudi-agx-tester}
#cprofile=${2}

if [ $# -lt 3 ] ; then
	echo "Usage: ./create_sd_image3.sh [DEVICE] [USERNAME] [PASSWORD] [[optional: HOSTNAME]]"
	exit 1
fi

echo "Creating default user..."
./tools/l4t_create_default_user_ff_custom.sh -u $username -p $password -a -n $hostname --accept-license
echo "Default user created."

if [[ $sd_dev == /dev/sd* ]]
then
	sd_dev_p=${sd_dev}1
else
#for mmcblk and nvme devices
	sd_dev_p=${sd_dev}p1
fi
script_name="$(basename "${0}")"
l4t_dir="$(cd "$(dirname "${0}")" && pwd)"
bootloader_dir="${l4t_dir}/bootloader"
#rfs_dir="${l4t_dir}/rootfs"
rfs_dir="/mnt/rootfs"
mntdir="/mnt/cti"

# font colour
FC_CY='\033[0;33m'  #cyan
FC_NC='\033[0m'     #no colour

print_ver
check_req
create_gpt
create_blkid
#print_blkid
exit
copy_rootfs ${2}
wait
