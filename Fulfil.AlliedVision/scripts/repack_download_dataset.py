# Assuming you have Gcloud SDK installed and user access to the factory-media folder in fulfil-web GCP project,
# run this to download all PLM prod Repack images

# Downloads all Repack data, formats it into COCO and allows for fast tool import + review.

from os import makedirs, listdir
from os.path import isdir, exists
import shutil
import json

data = 'data/'
raw = data + 'raw/'
by_id = data + 'by-id/'

if not exists(raw):
    makedirs(raw)
    print("\033[93m Assuming you have Gcloud SDK installed and authorized for factory-media bucket access... ( https://cloud.google.com/sdk/docs/install ) \033[0m")
    print("\033[92m 1. Download raw PLM Repack dataset from GCS by running: \033[0m")
    print("gcloud storage rsync -r 'gs://factory-media/plm/repack' 'Fulfil.AlliedVision/data/raw'")
    print("then run this again")
    exit(1)
else:
    print("\033[93m Assuming your raw dataset is up-to-date, if not, re-run after executing: \033[0m")
    print("gcloud storage rsync -r 'gs://factory-media/plm/repack' 'Fulfil.AlliedVision/data/raw'")

# Step 1: Import GCS client and rsync to ./data from cloud bucket
# Messy, but use command line gcloud storage rsync since there's really no other great way
# to get fast rsync behavior, and it should work across all OSes anyway

print("gcloud storage rsync -r 'gs://factory-media/plm/repack' '" + raw + "'")

# Step 2: Post-process raw Repack data into convenient request_id-indexed format
print("\033[92m 2. Post-processing results by request ID (data/by-id) for easy debugging \033[0m")
if exists(by_id):
    shutil.rmtree(by_id)
makedirs(by_id)
by_id_count = 0
for date_folder in listdir(raw):
    for cam_folder in listdir(raw + date_folder):
        for req_id in listdir(raw + date_folder + "/" + cam_folder):
            dest_dir = by_id + req_id
            shutil.copytree(raw + date_folder + "/" + cam_folder + "/" + req_id, dest_dir)
            by_id_count += 1
            # Hack the json label to include camera ID so context isn't lost
            result_file = by_id + req_id + "/result.json"
            if exists(result_file):
                labels = {}
                with open(result_file) as f:
                    labels = json.load(f)
                    labels["cameraId"] = cam_folder
                # Write JSON back to file
                with open(result_file, "w") as f:
                    json.dump(labels, f)
print("data/by-id dataset size: " + str(by_id_count))
print("Dataset all set!")
