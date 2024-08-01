# Warning: May have dated info 

## GCC 9+

### Install

Assuming you're on Ubuntu 18.04:

`sudo add-apt-repository ppa:ubuntu-toolchain-r/test`<br/>
(If the add-apt-repository command cannot be found, run this:  `sudo apt install software-properties-common`)

`sudo apt update` <br/>
`sudo apt install gcc-9` <br/>
`sudo apt install g++-9` <br/>
`sudo update-alternatives --install /usr/bin/gcc gcc /usr/bin/gcc-9 60 --slave /usr/bin/g++ g++ /usr/bin/g++-9`

(These instructions originally came from first answer and 2 comments of: https://askubuntu.com/questions/1140183/install-gcc-9-on-ubuntu-18-04)

### Verify Installation

`gcc --version` should show a version of 9 or higher <br/>
`g++ --version` should show a version of 9 or higher

<br/>

## CMAKE 3.20+

### Install

`sudo apt remove --purge --auto-remove cmake` <br/>

`sudo apt update` <br/>
`sudo apt install -y software-properties-common lsb-release` <br/>
`sudo apt clean all` <br/>

`wget -O - https://apt.kitware.com/keys/kitware-archive-latest.asc 2>/dev/null | gpg --dearmor - | sudo tee /etc/apt/trusted.gpg.d/kitware.gpg >/dev/null`<br/>
`sudo apt-add-repository "deb https://apt.kitware.com/ubuntu/ $(lsb_release -cs) main"`<br/>

`sudo apt update` <br/>

***
Note: If running 'sudo apt update' gets the following error:
Err:7 https://apt.kitware.com/ubuntu bionic InRelease
The following signatures couldn't be verified because the public key is not available: NO_PUBKEY 6AF7F09730B3F0A4
Fetched 11.0 kB in 1s (7552 B/s)

Then you can: Copy the public key 6AF7F09730B3F0A4 and run this command:
`sudo apt-key adv --keyserver keyserver.ubuntu.com --recv-keys 6AF7F09730B3F0A4` <br/>
***

`sudo apt install kitware-archive-keyring` <br/>
`sudo rm /etc/apt/trusted.gpg.d/kitware.gpg` <br/>

(These instructions originally came from this link, Method A:)
https://askubuntu.com/questions/355565/how-do-i-install-the-latest-version-of-cmake-from-the-command-line

### Verify Installation

`cmake --version` should show version 3.20 or higher

<br/>

# Troubleshooting

## After running cpp_dependencies.sh

Terminal may not open after you run cpp_dependencies.sh.
This is because Python has updated.

In order to fix this, you can use ctrl + alt + f3 to switch to a terminal that operates outside of the desktop. You will not be able to use any more applications at this point.

Now, type in the command: sudo apt-get --reinstall install python3-minimal

At this point, if you restart the computer, you should be able to get Terminal running again.

# Removals 

## librealsense

If there is an old installation of librealsense, you can remove with:

```
sudo rm /usr/local/bin/realsense-viewer \
/usr/local/bin/rs-align \
/usr/local/bin/rs-align-advanced \
/usr/local/bin/rs-ar-advanced \
/usr/local/bin/rs-ar-basic \
/usr/local/bin/rs-benchmark \
/usr/local/bin/rs-callback \
/usr/local/bin/rs-capture \
/usr/local/bin/rs-color \
/usr/local/bin/rs-convert \
/usr/local/bin/rs-data-collect \
/usr/local/bin/rs-depth \
/usr/local/bin/rs-depth-quality \
/usr/local/bin/rs-distance \
/usr/local/bin/rs-enumerate-devices \
/usr/local/bin/rs-fw-logger \
/usr/local/bin/rs-fw-update \
/usr/local/bin/rs-gl \
/usr/local/bin/rs-hello-realsense \
/usr/local/bin/rs-measure \
/usr/local/bin/rs-motion \
/usr/local/bin/rs-multicam \
/usr/local/bin/rs-pointcloud \
/usr/local/bin/rs-pose \
/usr/local/bin/rs-pose-and-image \
/usr/local/bin/rs-pose-predict \
/usr/local/bin/rs-post-processing \
/usr/local/bin/rs-record \
/usr/local/bin/rs-record-playback \
/usr/local/bin/rs-rosbag-inspector \
/usr/local/bin/rs-save-to-disk \
/usr/local/bin/rs-sensor-control \
/usr/local/bin/rs-software-device \
/usr/local/bin/rs-terminal \
/usr/local/bin/rs-tracking-and-depth \
/usr/local/bin/rs-trajectory

sudo rm -rf /usr/local/include/librealsense2 &&  \
sudo rm -rf /usr/local/include/librealsense2-gl &&  \
sudo rm -rf /usr/local/lib/cmake/realsense2 &&  \
sudo rm -rf /usr/local/lib/cmake/realsense2-gl &&  \
sudo rm /usr/local/lib/librealsense2-gl.so* &&  \
sudo rm /usr/local/lib/librealsense2.so* && \
sudo rm /usr/local/lib/librealsense-file.a && \
sudo rm /usr/local/lib/pkgconfig/realsense2-gl.pc && \
sudo rm /usr/local/lib/pkgconfig/realsense2.pc
```




