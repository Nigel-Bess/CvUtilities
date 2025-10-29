# Dispense scripts

## CI/CD

Because Github cannot handle ARM nor huge CV images, we have to use `clip-tb.whisman.fulfil.ai` as a build box for CI.

### Building and pushing main

After presumably merging a PR to main, you can build and push it as the latest image with: `bash scripts/dab-push-latest.sh` from the build box. This also works for custom branches.

### Pulling (deploying) latest of branch on some AGX

From some AGX/NX, you can update to latest by running: `bash scripts/dab-pull-latest.sh`.  You can pick an arbitrary branch to deploy when prompted.

## Downloading test data

Run `bash scripts/download_dispenses.sh` from the `Fulfil.Dispense` sub-folder and enter the SSH address and password of a running Dispense service to begin downloading all past request data and images to the `Fulfil.Dispense/data` folder.  This folder contains the `raw` sub-folder that matches the exact contents of the running Dispense service as well as a `by-id` folder that organizes raw data by request ID for easy test setup.  Any saved test results you want to store for analysis should go into `Fulfil.Dispense/data/test` with each sub-folder keyed by request ID.

Be sure to have proper BigQuery access to the `facilities` dataset before running.

## Running tests

Tests run via Docker compose and the Dispense test container will share the `Fulfil.Dispense/data` folder with your host machine.

### Testing FED

From the `Fulfil.ComputerVision` root dir, run: 

```
docker compose --profile fed_test up --build
```
