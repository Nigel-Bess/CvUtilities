#!/usr/bin/python3

import configparser
from pymongo import MongoClient
from bson.objectid import ObjectId

def write_coords(f, camera, type, mongo_calib, mode):
    if mode == "dispense":
        header = "[" + camera + "_" + type + "]\n"
    else:
        header = "[" + camera + "_" + mode + "_" + type + "]\n"

    f.write(header)
    temp = mongo_calib[type]["x"]
    f.write("x = ")
    for i in range(len(temp)):
        f.write(str(temp[i]) + " ")
    f.write("\n")

    temp = mongo_calib[type]["y"]
    f.write("y = ")
    for i in range(len(temp)):
        f.write(str(temp[i]) + " ")
    f.write("\n")

    temp = mongo_calib[type]["z"]
    f.write("depth = ")
    for i in range(len(temp)):
        f.write(str(temp[i]) + " ")
    f.write("\n")

    f.write("\n")

def write_to_ini(mongo, machine, camera, mode):
    search_str = "last_tray_camera_calibration_"+mode
    doc = mongo["Machines"]["DepthCameras"].find_one({"VLS": ObjectId(machine)})
    qid = doc[search_str]

    mongo_calib = mongo["Recipes"]["TrayCalibrations"].find_one({"_id": qid})

    file_name = "../configs/tray_calibration_data_" + mode + ".ini"
    f = open(file_name, "w")

    f.write("[config_details]\n")
    f.write("hostname = " + mongo_calib["machine_hostname"] + "\n")
    f.write("date_updated = " + mongo_calib["calibration_timestamp"].isoformat() + "\n")
    f.write(camera + "_recipe_id_" + mode + " = " + str(qid) + "\n")

    f.write("\n")

    write_coords(f, camera, "pixel_locations", mongo_calib, mode)
    write_coords(f, camera, "camera_coordinates", mongo_calib, mode)
    write_coords(f, camera, "tray_coordinates", mongo_calib, mode)

    f.close()

if __name__=="__main__":
    main_parser = configparser.ConfigParser(inline_comment_prefixes=";")
    main_parser.read("../configs/AGX_specific_main.ini")
    tray_camera = main_parser.get("device_specific", "tray_cam")
    machine = main_parser.get("device_specific", "vls0_id")

    mongo_parser = configparser.ConfigParser(inline_comment_prefixes=";")
    mongo_parser.read("../configs/secret/mongo_conn_config.ini")
    mongo_conn = mongo_parser.get("connection_info", "conn_string")

    client = MongoClient()
    client = MongoClient(mongo_conn)

    write_to_ini(client, machine, tray_camera, "dispense")
    write_to_ini(client, machine, tray_camera, "hover")
    write_to_ini(client, machine, tray_camera, "tongue")
