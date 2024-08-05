# Fulfil.ComputerVision

New monorepo for Depth Cam repositories. Previously tracked as 4 individual repositories: 
Fulfil.Dispense, Fulfil.DepthCam, Fulfil.CPPUtils, FulfilMongoCpp. Current repo specific to Mars systems 
(not backwards compatible with 3.1).

Useful documentation:
The most up-to-date documentation for running with this system can be found at the [Depth Cam and DAB Commands Guide](https://docs.google.com/document/d/14k9Jj3RgEqf9b27t3sbYC5RS6eEuN8hgaCXUnPTA268/edit#heading=h.5wo37tlgfgcn).
The most up-to-date documentation for setup & general guide for Docker integration can be found at the [CV Docker Container Setup & Guide](https://docs.google.com/document/d/16MDraQAOxy66sDTvkCYprN8X7HRokDZPE1xLYn9MRx0/edit?pli=1#heading=h.tbyc1yaxb04b).

## Building the Depth Cam API 
Current depthcam cpp sources use CMake to generate the system. For convenience there is a 
makefile in `Fulfil.Dispense`. To kick off cpp build:
```angular2html
cd Fulfil.Dispense
make
```
If the build succeeds, you will find the executable at `Fulfil.Dispense/build/app/main` when navigating 
from the `Fulfil.ComputerVision` project root.  


## Starting the Mars API

Log on as the fulfil user (or run the command from fulfil userspace). Run the dc-startup script to cycle 
the services. G-Dabs run on `.83`, F-Dabs run on `.88`, Induction Loop runs off of `.62`. 
**Beware the system is set to auto start on boot, which normally takes ~45 seconds to a minute to initiate. 
You may cause a camera resource error if you run startup before the service has a chance to return.** 
If the computer has just come back from boot, check to see if the service has already attempted and failed
to initialize with `sudo systemctl status depthcam.service` prior to running startup commands.
```
# ssh fulfil@10.10.103.83 or 88
cd code
dc-startup
# kills all running shells related to depthcams 
dc-startup kill
```
Unless you are starting right after a power cycle, I recommend checking the depthcam session first (see below). 
You may only need to scroll up to the last command and restart the api from there. If you are using the script,
should start the sessions only if they do not exist. Validate that cameras start properly by logging onto the depthcam 
tmux session and watching until the connection is established (be careful not to quit the session while detaching).
```
tmux a -t depthcam
# should see something like the following lines on clean startup
[10:00:52] [0ms] [-info-] [3403] Start bay manager...
[10:00:52] [0ms] [-info-] [3641] Starting bay 1: FD1
[10:00:52] [0ms] [-info-] [3642] Starting bay 0: FD2
[10:00:52] [0ms] [-info-] [3643] search for connection...
[10:00:52] [0ms] [-info-] [3644] search for connection...
[10:00:53] [351ms] [-info-] [3644] connection established
[10:00:53] [140ms] [-info-] [3643] connection established
```
If the api fails to start, run `rs-fw-update -l` to list out all four cameras and ensure that they are allowing basic 
queries. If that command fails, you may need to power cycle the board.
