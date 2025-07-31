# Only to be called by rsync_pre_dispense_from.sh
import os
from os import path
import shutil
import json
from datetime import date, datetime

from google.cloud import bigquery

RAW = path.join("data", "raw")
BY_ID = path.join("data", "by-id")
TEST = path.join("data", "test")
LABELS = path.join("data", "labels")

client = bigquery.Client()

def json_serial(obj):
    """JSON serializer for objects not serializable by default json code"""

    if isinstance(obj, (datetime, date)):
        return obj.isoformat()
    raise TypeError ("Type %s not serializable" % type(obj))

# Downloads and saves results saved in BQ to same path as the request
def download_requests_from_bq(req_paths):
    print("Cross-referencing BQ for saved request data...")
    id_to_path = {}
    for req_path in req_paths:
        id_to_path[req_path.split(path.sep)[-1]] = req_path
    found_req_ids = set()
    # map batches' req_ids to BQ query matches
    batch_size = 500
    batches = [req_paths[i:i + batch_size] for i in range(0, len(req_paths), batch_size)]
    for batch in batches:
        quoted_ids = ['"' + req_path.split(path.sep)[-1] + '"' for req_path in batch]
        batch_csv = ",".join(quoted_ids)
        print('\nSELECT * FROM `fulfil-web.facilities.depth_camera_queries` WHERE command_id IN (' + batch_csv + ')')
        query = ('SELECT * FROM `fulfil-web.facilities.depth_camera_queries` WHERE command_id IN (' + batch_csv + ')')

        query_job = client.query(query)  # API request
        rows = query_job.result()  # Waits for query to finish
        for row in rows:
            found_req_ids.add(row.command_id)
            req_path = id_to_path[row.command_id]
            # Convert BQ row to JSON and save to file
            filename = path.join(req_path, row.request_type + "Request.json")
            with open(filename, 'w') as file:
                file.write(json.dumps(dict(row), default=json_serial))
    print("Found " + str(len(found_req_ids)) + " / " + str(len(req_paths)) + " requests in BQ")

def main():
    os.makedirs(RAW, exist_ok=True)
    os.makedirs(BY_ID, exist_ok=True)
    os.makedirs(TEST, exist_ok=True)
    os.makedirs(LABELS, exist_ok=True)

    raw_req_paths = []  # All successfully found request IDs for associated input images
    for day_folder in os.listdir(RAW):
        if not day_folder.startswith("saved_images_"):
            continue
        for cam_type in os.listdir(path.join(RAW, day_folder)):
            for req_id in os.listdir(path.join(RAW, day_folder, cam_type)):
                req_path = path.join("data", "raw", day_folder, cam_type, req_id)
                raw_req_paths.append(req_path)
    download_requests_from_bq(raw_req_paths)

    # Copy all raw data into by-id (request ID) folder format
    for day_folder in os.listdir(RAW):
        if not day_folder.startswith("saved_images_"):
            continue
        for cam_type in os.listdir(path.join(RAW, day_folder)):
            for req_id in os.listdir(path.join(RAW, day_folder, cam_type)):
                req_path = path.join("data", "raw", day_folder, cam_type, req_id)
                os.makedirs(path.join(BY_ID, cam_type), exist_ok=True)
                clean_location = path.join(BY_ID, cam_type, req_id)
                os.makedirs(clean_location, exist_ok=True)
                shutil.copytree(req_path, clean_location, dirs_exist_ok=True)
    print(str(len(raw_req_paths)) + " input request folders written to " + BY_ID)

main()
