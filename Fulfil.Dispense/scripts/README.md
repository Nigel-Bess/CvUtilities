# Dispense scripts

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
