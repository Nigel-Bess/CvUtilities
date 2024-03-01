#!/usr/bin/python3

import os
import sys
import datetime
import configparser

parser = configparser.ConfigParser(inline_comment_prefixes=';')
parser.read(os.path.expanduser("~/code/Fulfil.Dispense/configs/AGX_specific_main.ini"))

jpeg_folder = parser.get("device_specific", "vid_gen_base_buffer_dir")
live_folder = parser.get("device_specific", "live_visualize_drop_path")
images_folder = os.path.expanduser("~/Videos")
audit_folder = os.path.expanduser("~/Videos/audit_images")
fed_test_folder = os.path.expanduser("~/Videos/fed_testing")
logs_folder = os.path.expanduser("~/code/Fulfil.Dispense/logs")
core_folder = os.path.expanduser("~/code/Fulfil.Dispense/rs-core-logs")

def delete_jpegs():
    for subdir, dir, files in os.walk(jpeg_folder, topdown=False):
        for file in files:
            if file.endswith(".jpg"): #delete all jpegs
                os.remove(os.path.join(subdir, file))
        if not os.listdir(subdir) and subdir != jpeg_folder:  # delete PKID folders
            os.rmdir(subdir)

def safe_remove_folder(folder, only_subfolders=False):
    for subdir, dir, files in os.walk(folder, topdown=False):
        for file in files:
            os.remove(os.path.join(subdir, file))
        if not os.listdir(subdir):
            if only_subfolders==True and subdir == folder:
                continue
            else:
                os.rmdir(subdir)

def delete_saved_images(drop_period, tray_period, fed_period):
    subfolders = [f.path for f in os.scandir(images_folder) if f.is_dir()]
    today = datetime.datetime.today()
    for subdir in subfolders:
        if "saved_images_" in subdir:
            arr = subdir.split("_")
            date = datetime.datetime(int(arr[2]), int(arr[3]), int(arr[4]))

            # handle deletion of drop target images
            if today - date > datetime.timedelta(drop_period):
                safe_remove_folder(os.path.join(subdir, "Drop_Camera"))

            # handle deletion of item edge / tray validation images
            tray_val_folder = os.path.join(subdir, "Tray_Camera")
            request_types = [f[0] for f in os.walk(tray_val_folder)]
            for request in request_types:
                if "TrayValidation" in request:
                    if today - date > datetime.timedelta(tray_period):
                        safe_remove_folder(request)
                if "FrontEdgeDistance" in request:
                    if today - date > datetime.timedelta(fed_period):
                        safe_remove_folder(request)
            remaining_requests = [f[0] for f in os.walk(tray_val_folder)]
            for temp in remaining_requests[::-1]: #delete remaining empty folders
                if not os.listdir(temp):
                    os.rmdir(temp)
            if not os.listdir(subdir):
                os.rmdir(subdir)

def delete_videos(period):
    today = datetime.datetime.today()
    for subdir, dir, files in os.walk(jpeg_folder, topdown=False):
        for file in files:
            if file.endswith(".mp4"):
                full_path = os.path.join(subdir, file)
                mod_date = datetime.datetime.fromtimestamp(os.path.getmtime(full_path))
                if today-mod_date > datetime.timedelta(period):
                    os.remove(full_path)
        if not os.listdir(subdir) and subdir != jpeg_folder:  # delete PKID folders
            os.rmdir(subdir)

def delete_logs(period):
    today = datetime.datetime.today()
    for subdir, dir, files in os.walk(logs_folder, topdown=False):
        for file in files:
            arr = file.split(".")[0].split("_")[-1].split("-")
            date = datetime.datetime(int(arr[0]), int(arr[1]), int(arr[2]))
            if today-date > datetime.timedelta(period):
                os.remove(os.path.join(subdir, file))
    for subdir, dir, files in os.walk(core_folder, topdown=False):
        for file in files:
            arr = file.split(".")[0].split("_")[2].split("-")
            date = datetime.datetime(int(arr[0]), int(arr[1]), int(arr[2]))
            if today-date > datetime.timedelta(period):
                os.remove(os.path.join(subdir, file))

if __name__=="__main__":

    # crontab on Pioneer bays
    # ~/code/Fulfil.Dispense/disk_clean_util.py 1 500 1 7 7
    # delete drop images and videos every day, FED images every week, never delete tray validation images,
    # and logs every week

    if len(sys.argv) != 6: #don't run if incorrect number of parameters given, protect against old cron jobs
        exit(0)
    drop_image_period = int(sys.argv[1])
    tray_image_period = int(sys.argv[2])
    video_period = int(sys.argv[3])
    fed_period = int(sys.argv[4])
    log_period = int(sys.argv[5])
    delete_jpegs()
    safe_remove_folder(live_folder, only_subfolders=True)
    safe_remove_folder(audit_folder, only_subfolders=True)
    safe_remove_folder(fed_test_folder, only_subfolders=True)
    delete_logs(log_period)
    delete_saved_images(drop_image_period, tray_image_period, fed_period)
    delete_videos(video_period)
