# Fulfil.ComputerVision

Monorepo for Depth Cam repositories. Previously tracked as 4 individual repositories: 
Fulfil.Dispense, Fulfil.DepthCam, Fulfil.CPPUtils, FulfilMongoCpp. Current repo specific to Mars systems 
(not backwards compatible with 3.1).

Useful documentation:
The most up-to-date documentation for running with this system can be found at the [Depth Cam and DAB Commands Guide](https://docs.google.com/document/d/14k9Jj3RgEqf9b27t3sbYC5RS6eEuN8hgaCXUnPTA268/edit#heading=h.5wo37tlgfgcn).
The most up-to-date documentation for setup & general guide for Docker integration can be found at the [CV Docker Container Setup & Guide](https://docs.google.com/document/d/16MDraQAOxy66sDTvkCYprN8X7HRokDZPE1xLYn9MRx0/edit?pli=1#heading=h.tbyc1yaxb04b).


## Development

As of August 2025 just about all things CV run in Docker both in production and for development to ensure we have consistent builds across the many
environments CV code must run in.  We break out the highest level Docker Compose definitions on a per machine or platform basis, with the default
`docker-compose.yml` representing a typical dev environment such as an Intel/AMD Mac or PC, whereas `docker-compose.dab.yml` is intended
to run on DAB machines (which are assumed to be ARM64 Jetson boards).  The docker compose files define "services" which can either be proper services
or offline test entrypoints of different kinds.

### Setup

Only Docker (with Docker Compose) and the Gcloud SDK (for authentication) are necessary for developing against this repo, and we generally use either VS Code or Visual Studio for Python and C++ development.  In Slack ask `#user-access-requests` for access to GCP Artifact Registry to pull Docker images by running the following commands in a terminal:

```
gcloud auth login
gcloud auth configure-docker
```

Then test that you can pull private images with:

`docker pull gcr.io/fulfil-web/cv-dispense/realsense-amd:latest`

### Building

(See Deploying for info on building for ARM 64, this assumes you're building locally on a typical AMD64 computer)

You can build any of the *.amd.Dockerfile files directly, such as Dispense:

`docker build . -f Dispense.amd.Dockerfile`

or similar for services like AlliedVision.Dockerfile that is assumed to be AMD since it never runs on ARM processors (when unspecified in the file name).

### Running

We generally use vanilla docker compose commands to start all appropriate services per platform type, such as a DAB:

`cd /home/fulfil/code/Fulfil.ComputerVision && docker compose -f docker-compose.dab.yml up -d`

note having to opt into the DAB environment via the `-f` flag and not necessary for local development.

Restarting:

`cd /home/fulfil/code/Fulfil.ComputerVision && docker compose -f docker-compose.dab.yml restart`

Stopping:

`cd /home/fulfil/code/Fulfil.ComputerVision && docker compose -f docker-compose.dab.yml down`

### Deploying

Because building giant ARM64 images in Github Actions is nearly impossible, we instead use a dedicated CI build box in Whisman that can
be used to replicate Github's current AMD images that are successfully built as a CI check on Pull requests.

Fulfil.AlliedVision and Fulfil.TCS are Kubernetes apps with idealized Github CI/CD deployments that are fully automatic; GH Action build and pushes images and cloud backend's Flux CD system automatically deploys latest releases, the remainder here describes deployments for Jetson board / AGX / Xavier / Orin boards.

#### 1. Login to build box

`ssh fulfil@clip-tb.whisman.fulfil.ai` while connected to VPN.

Ask someone on CV for access if needed.

#### 2. Build and push a Production ARM image to GCP Artifact Registry

See the README.md's of the appropriate sub-directory but an example build of Dispense would be:

```
cd /home/fulfil/code/Fulfil.ComputerVision
git checkout main
git pull
docker build . -f Dispense.arm.Dockerfile -t main
docker tag main gcr.io/fulfil-web/cv-dispense/main:latest
docker push gcr.io/fulfil-web/cv-dispense/main:latest
```

which as a shortcut you can run with `bash /home/fulfil/code/Fulfil.ComputerVision/Fulfil.Dispense/scripts/dab-push-latest.sh` from the build box

#### 3. Pull latest build per machine

All machines point to their respective `main:latest` docker images in Google by default, but each production machine
will never attempt to update it's image unless told to do so in vanilla docker fashion.  An example is pulling and
restarting the last-pushed Dispense Docker image on some DAB:


```
cd /home/fulfil/code/Fulfil.ComputerVision
docker compose -f docker-compose.dab.yml pull
docker compose -f docker-compose.dab.yml down
docker compose -f docker-compose.dab.yml up -d
```

which as a shortcut you can run with `bash /home/fulfil/code/Fulfil.ComputerVision/Fulfil.Dispense/scripts/dab-pull-latest.sh` on the target
machine to update and restart services on.


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

### Setup

```
gcloud auth login
gcloud config set project fulfil-web
gcloud auth configure-docker
```

### Running Dev Containers Plugin for VS Code

Pure Docker is an option but for the fastest and most complete dev experience you can use VS Code + Dev Containers

#### Prerequisites
- Docker installed on your system
- VS Code installed with the [Dev Containers](https://marketplace.visualstudio.com/items?itemName=ms-vscode.remote-containers) extension

#### Steps

##### 1. Build a Stable Branch and Tag it Locally
Build your Docker image using the following command, for the TCS repo in this case:
```bash
docker build . -f TCS.Dockerfile -t tcs
```

This creates a tagged image (`tcs`) that you can use for running your container.

##### 2. Run the Container with Mapped Folders
Run a new container, mapping specific folders between your host machine and the container:
```bash
docker run -it \
-v "/home/burt/Fulfil.ComputerVision/Fulfil.TCS/src:/home/fulfil/code/Fulfil.ComputerVision/Fulfil.TCS/src" \
-v "/home/burt/Fulfil.ComputerVision/Fulfil.TCS/include:/home/fulfil/code/Fulfil.ComputerVision/Fulfil.TCS/include" \
tcs bash
```

This command:
- Starts an interactive container (`-it`)
- Maps your `src` and `include` folders to the container's file system so both VS Codes are in-sync but only where it counts.
- Runs a Bash shell in the container that can subsequently be used for faster `make` builds rather than slower, full docker builds.  Native make can much better leverage compilation caching.

##### 3. Attach VS Code to Running Container
Open VS Code on your host machine, then:
1. Option Actions (Ctrl+shift P?) -> **Attach to Running Container**
2. Select the container from the list (it will have a random name like `cheerful_hamilton` or similar)

This sets up VS Code in the container and allows you to edit files directly within the container environment.  You will want to install the C++ plugin when prompted.

##### 4. Edit Files in VS Code
- Make changes to your code in either VS Code on your host machine or within the container.
- Changes will be synced between the host and container since the folders are mapped.
- Either host docker build or the bash container's make will rebuild from shared src.


### Useful make commands
make shell: ssh into the container
make docker-update: re-generates C++ files from protos in your local both inside and outside the container (🔑)
make push: builds the container and pushes to the registry
make tag: tags the container (mostly for deployment)
make push: pushes the image to GCR (mostly for deployment)
make up: starts the container
make down: stops the container
make reset: restarts the container
