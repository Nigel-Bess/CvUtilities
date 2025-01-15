# Assuming you have Gcloud SDK installed and user access to the factory-media folder in fulfil-web GCP project,
# run this to download all PLM prod Repack images

# Run from Fulfil.AlliedVision: `python3 scripts/repack_download_dataset.py`

# Downloads all Repack data, formats it into COCO and allows for fast tool import + review.

from os import makedirs, listdir
from os.path import isdir, exists
import subprocess
import shutil
import json

coco_dataset_dir = "../../coco-annotator/datasets/cv-repack"
if not exists("../../coco-annotator/datasets"):
    print("\033[93m " + coco_dataset_dir + " sibling folder of Fulfil.ComputerVision doesn't exist, clone coco-annotator to same dir as Fulfil.ComputerVision \033[0m")
    print("See ../README.md's 'Testing + Debugging' section")
    exit(1)

# Step 1: Import GCS client and rsync to ./data from cloud bucket
# Messy, but use command line gsutil rsync since there's really no other great way
# to get fast rsync behavior, and it should work across all OSes anyway
print("\033[93m Assuming you have Gcloud SDK installed and authorized for factory-media bucket access... ( https://cloud.google.com/sdk/docs/install ) \033[0m")
print("\033[92m 1. Downloading raw PLM Repack dataset from GCS \033[0m")
dst_path = 'data/raw'
if isdir(dst_path) == False:
    makedirs(dst_path)
subprocess.run(["gsutil", "-m", "rsync", "-r", "gs://factory-media/plm/repack", dst_path])

# Step 2: Post-process raw Repack data into convenient request_id-indexed format
print("\033[92m 2. Post-processing results by request ID (./data/by-id) for easy debugging \033[0m")
if exists("data/by-id"):
    shutil.rmtree("data/by-id")
makedirs("data/by-id")
by_id_count = 0
for date_folder in listdir("data/raw"):
    for cam_folder in listdir("data/raw/" + date_folder):
        for req_id in listdir("data/raw/" + date_folder + "/" + cam_folder):
            dest_dir = "data/by-id/" + req_id
            shutil.copytree("data/raw/" + date_folder + "/" + cam_folder + "/" + req_id, dest_dir)
            by_id_count += 1
            # Hack the json label to include camera ID so context isn't lost
            result_file = "data/by-id/" + req_id + "/result.json"
            if exists(result_file):
                labels = {}
                with open(result_file) as f:
                    labels = json.load(f)
                    labels["cameraId"] = cam_folder
                # Write JSON back to file
                with open(result_file, "w") as f:
                    json.dump(labels, f)
print("data/by-id dataset size: " + str(by_id_count))
print("You'll probably want to run `sudo python3 scripts/coco/repack_generate_coco.py` next!`")
