# Only to be called by rsync_pre_dispense_from.sh
import os
from os import path
import shutil
import json
from random import shuffle
from datetime import date, datetime

from google.cloud import bigquery

RAW = path.join("data", "raw")

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

def has_required_files(req_path):
    return os.path.exists(path.join(req_path, "PreFrontEdgeDistance", "aligned_depth_image.png")) and \
        os.path.exists(path.join(req_path, "PreFrontEdgeDistance", "raw_depth_image.png")) and \
        os.path.exists(path.join(req_path, "PreFrontEdgeDistance", "color_image.png")) and \
        os.path.exists(path.join(req_path, "PreFrontEdgeDistance", "color_intrinsics")) and \
        os.path.exists(path.join(req_path, "PreFrontEdgeDistance", "depth_intrinsics")) and \
        os.path.exists(path.join(req_path, "PreFrontEdgeDistance", "depth_to_color_extrinsics")) and \
        os.path.exists(path.join(req_path, "PreFrontEdgeDistance", "color_to_depth_extrinsics"))

def file_contains(filename, content_substring):
    if os.path.exists(filename):
        with open(filename, "r") as f:
            return f.read().find(content_substring) >= 0
    return False

# Assumes request_json_filter is a substring that must exist in some JSON request file,
# a key=value pair where key is the filename relative to the request dir and the value
# is the strict substring that must exist in the request file
def filter_req_paths(raw_req_paths, request_json_filter):
    if len(request_json_filter) < 2:
        return raw_req_paths
    split = request_json_filter.index("=")
    filename = request_json_filter[0:split]
    content_substring = request_json_filter[split+1:]
    filtered = [rrp for rrp in raw_req_paths if file_contains(path.join(rrp, filename), content_substring)]
    print("Found {} filtered matching requests of {}".format(len(filtered), len(raw_req_paths)))
    return filtered

# Get a random subset of request IDs in the data/raw dir
def get_shuffled_req_paths(raw_req_paths, max_count):
    req_paths = [rrp for rrp in raw_req_paths]  # Shallow copy
    # Return random subset
    shuffle(req_paths)
    return req_paths[0:max_count]

def main():
    BY_REQ_DIR = path.join("data", os.environ['DATASET_NAME'])
    os.makedirs(RAW, exist_ok=True)
    os.makedirs(BY_REQ_DIR, exist_ok=True)

    raw_req_paths = []  # All successfully found request IDs for associated input images
    for day_folder in os.listdir(RAW):
        if day_folder.startswith("saved_images_"):
            for cam_type in os.listdir(path.join(RAW, day_folder)):
                for req_id in os.listdir(path.join(RAW, day_folder, cam_type)):
                    req_path = path.join("data", "raw", day_folder, cam_type, req_id)
                    if has_required_files(req_path):
                        req_path = path.join("data", "raw", day_folder, cam_type, req_id)
                        raw_req_paths.append(req_path)

    request_json_filter = os.environ['REQUEST_FILTER']
    max_requests = int(os.environ['REQUEST_COUNT'])
    req_paths = filter_req_paths(raw_req_paths, request_json_filter)
    if len(req_paths) == 0:
        raise Error("Filter matched no requests! " + request_json_filter)
    req_paths = get_shuffled_req_paths(req_paths, max_requests)

    # TODO: Re-enable this when or if needed
    #download_requests_from_bq(req_paths)

    # Copy all raw data into by-id (request ID) folder format
    for day_folder in os.listdir(RAW):
        if not day_folder.startswith("saved_images_"):
            continue
        for cam_type in os.listdir(path.join(RAW, day_folder)):
            for req_id in os.listdir(path.join(RAW, day_folder, cam_type)):
                req_path = path.join("data", "raw", day_folder, cam_type, req_id)
                if req_path not in req_paths:
                    continue
                os.makedirs(path.join(BY_REQ_DIR, cam_type), exist_ok=True)
                clean_location = path.join(BY_REQ_DIR, cam_type, req_id)
                os.makedirs(clean_location, exist_ok=True)
                shutil.copytree(req_path, clean_location, dirs_exist_ok=True)
    print(str(len(req_paths)) + " request folders written to " + BY_REQ_DIR)
    print("You may want to archive this to GCS by running:\n\n")
    # TODO!
    print("gcloud storage rsync -r 'gs://factory-media/X' 'X'")

main()
