# Depth Cam Computer SetUp
A place to get some instruction, resources, and troubleshooting tips on DepthCam computer bring up. Mostly focused
on the [ConnectTech Rudi](https://connecttech.com/product/rudi-agx-embedded-system-nvidia-jetson-agx-xavier/) module.


## AGX Flashing
All Jetsons use a custom linux kernel. Nvidia provides sources for the kernel and drivers as well as the
proper version of cuda and other development software tools. The
[Nvidia flashing script docs](https://docs.nvidia.com/jetson/archives/r34.1/DeveloperGuide/text/SD/FlashingSupport.html)
are a useful resource. The drivers and device tree is specific to
the devkit. ConnectTech provides custom BSPs to run their carrier boards. When updating a BSP, you must
ensure that you are downloading one that supports the Rudi _and_ provides the specialized driver to run
the IntelReasense Cameras. Avoid patching the kernel, as it can sometimes override the device
tree. You can find instructions to flash ConnectTech boards with a cloned image

### Version Info
We are currently using JP 5.1.1, L4T version 35.3.1 as the base image. Choose this in the SDK drop down.
The realsense BSP we are using is the
[Intel Realsense AGX 35.3.1 V2](https://connecttech.com/ftp/Drivers/CTI-L4T-AGX-35.3.1-INTELRS-V002.tgz) release.

## Building a new Image

ConnectTech provides instructions to bring up their board in the BSP README. They also have additional guides
to bring up the [base kernel image](https://connecttech.com/resource-center/kdb373/) and install
the [SDK components](https://connecttech.com/resource-center/kdb374/) (which is required for cuda support). Note that 
there are Rudi-NX Orin specific changes in the connect tech bsp read me.  


### Installing Dependencies
To speed up builds on a Jetson, consider changing the power mode from the default:
```angular2html
sudo nvpmodel -m 3 # Less likely to overheat during builds
sudo nvpmodel -m 0 # Max power, you will want to run the production applications in this setting
```


### Depthcam CPP dependencies

The following code should work for both your x86 host and both Rudis.

Enter the `~/code/Fulfil.ComputerVision/scripts/env_setup` directory run the following command to install custom dependencies and log output:
```angular2html
BUILD=$(date '+%Y_%m_%d-H%H-M%M') ;  ./multi_arch_gpu_install.sh 2>&1 | tee "multi_arch_installs_out_${BUILD}.txt"
```
This is largely unsupervised, though you will need to answer some y/n options throughout the process. These
are normally asked at points where a review of build configurations is advised.

**If installing on a Jetson,** your environment parameters should look like the following:
```angular2html
Dependencies will be installed at /usr by script. If CMake install location is changed Fulfil.Dispense may not compile.
Current environment/build params on host:
-- Operating System:  Ubuntu 20.04.5 LTS
-- Processor Type:    aarch64
-- GPU Architecture:  7.2
-- Cuda Gencode:      72
-- Build with Cuda:   ON
-- Install Prefix:    /usr
-- Install Directory: /usr/include
-- Python Binary:     /usr/bin/python3
-- Python Version:    3.8.10
-- Python Libraries:  lib/python3.8
-- Num Build Threads: 6

-- Eigen3 Version:    3.3.9
-- OpenCV Version:    4.6.0
-- Realsense Version: 2.54.2
-- MongoC Version:    1.21.1
-- MongoCXX Version:  3.6.6
-- SpdLog Version:    1.11.0
```

**WARNING: If installing to an NX, change** `JETSON_CUDA_ARCH` **from 7.2 to 8.7!**

If `Build with Cuda` is set to `OFF` then you need to go back and install the SDK components. You can
doublecheck this by running `nvcc --version`  where the expected output would look like the following:

```angular2html
nvcc: NVIDIA (R) Cuda compiler driver
Copyright (c) 2005-2022 NVIDIA Corporation
Built on Sun_Oct_23_22:16:07_PDT_2022
Cuda compilation tools, release 11.4, V11.4.315
Build cuda_11.4.r11.4/compiler.31964100_0
```

After the installation, you may want to clear or mv the cpp dependencies off the device. You may
run out of disk space.

### Install the Protobuf / Python Dependencies

Next you will need to install protobuf / gRPC. At this point, a system level gRPC is
not required and can be skipped. Output logging is done in script.
```angular2html
BUILD=$(date '+%Y_%m_%d-H%H-M%M') ;  sudo ./install_sys_grpc_protobuf.sh  2>&1 | tee "grpc_proto_${BUILD}.txt"
```
Do not change the versions in this script unless the corresponding version has been
updated in the `Fulfil.TrayCountAPI/scripts/agx_env_setup.sh` file. Validate that the versions looks correct
post build at `/root/sources`, and be sure to remove build folders to preserve disk space.

Next you will need to install the python dependencies used by the count api by running:
```angular2html
cd ~/code/Fulfil.TrayCountAPI/scripts/
BUILD=$(date '+%Y_%m_%d-H%H-M%M') ; bash agx_env_setup.sh 2>&1 | tee "count_dep_install_out_${BUILD}.txt"
```

## Cloning / Restoring an Image
ConnectTech provides instructions to [clone and restore an image](https://connecttech.com/resource-center/kdb-378-cloning-jetson-modules-with-connect-tech-board-support-package/) and install
with their BSP intact. Though Amber only tested this on boards that were booting off eMMC.
This will produce a sparse .img and a .img.raw (full disk). While cloning an NVMe boot is untested, there is a
way to restore a eMMC clone to an NVMe (even though the new drive is much larger).

## eMMC vs NVMe

The eMMC on board has limited storage (~30G), but there are slots for NVMe cards to get installed.
ConnectTech provides instructions to flash a device to "boot" from NVMe. Though all you are actually
doing is mounting the rootfs on the device because the Jetson AGX Xavier series devices use boot
firmware that is stored only on internal eMMC memory. Therefore it's internal eMMC must be flashed.
ConnectTech provides instructions on how to [boot the device from NVMe](https://connecttech.com/resource-center/kdb377-booting-off-external-media-cti-jetson-carriers/#1689702992608-171b0591-809f),
however, they do not allow you to flash with a cloned image.

**The following instructions assume physical access to the NVMe**. To flash a large NVMe with a
cloned image, copy `create_sd_image3_ff_custom.sh` into the JetPack
`Linux_for_Tegra` folder and `l4t_create_default_user_ff_custom.sh` to `Linux_for_Tegra/tools/`.
Plug the external media into the host computer (if using an adapter, the card will likely show up as and /dev/sd*).
Review the connect tech instructions, and where relevant substitute those instructions with the following:

```angular2html
# mount image to rootfs
sudo mount -o loop < cloned_image_name >.raw /mnt/rootfs
# second check disks
df -h
sudo lsblk 
sudo fdisk -l 
# replace device name with your own
NEW_HOSTNAME=< desired hostname > ; sudo ./create_sd_image3_ff_custom.sh  /dev/sdb fulfil FreshEngr ${NEW_HOSTNAME} | tee -a ${NEW_HOSTNAME}-create-sd.txt
sudo lsblk | grep -B1 -A 2 -E "sd|rootfs|mnt"
# For your record file 
NEW_HOSTNAME=< desired hostname > ; echo -e "\n$(df -h | grep -E "sd|rootfs|mnt|cti")\n\n$(sudo lsblk | grep -B1 sdb)\n\n$(sudo fdisk -l | grep -A 10 sdb)\n\n$( sudo parted /dev/sdb print devices)\n\n/dev/sdb ID info:\n$(sudo blkid /dev/sdb)\n\n/dev/sdb1 ID info:\n$(sudo blkid /dev/sdb1)" | tee -a ${NEW_HOSTNAME}-create-sd.txt
sudo umount /dev/sdb1 && echo "Unmounted Device Partition!" || echo "Partition not mounted!"
```
Install the NVMe in the Jetson and then set the device to boot from the new root filesystem location.
```angular2html
sudo ./flash.sh cti/xavier/rudi-agx/intel-realsense nvme0n1p1
```
