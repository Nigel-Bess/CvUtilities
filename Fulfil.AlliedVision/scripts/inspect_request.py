# Run this with `python3 scripts/inspect_request.py "<My Request ID>"`

import sys
import json
import subprocess
req_id = sys.argv[1]

dir = "data/by-id/" + req_id + "/"

subprocess.run(["open", dir + "canny_edges.jpeg"])
subprocess.run(["open", dir + "hull_image.jpeg"])
subprocess.run(["open", dir + "markers_detected.jpeg"])
subprocess.run(["open", dir + "region_of_interest.jpeg"])
subprocess.run(["open", dir + "rgb_contours.jpeg"])

label_file = dir + "result.json"
with open(label_file) as f:
    j = json.load(f)
    print(j)