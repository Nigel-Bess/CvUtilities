# Fulfil.ComputerVision

New monorepo for Depth Cam repositories. Previously tracked as 4 individual repositories: 
Fulfil.Dispense, Fulfil.DepthCam, Fulfil.CPPUtils, FulfilMongoCpp. Current repo specific to Mars systems 
(not backwards compatible with 3.1).

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
