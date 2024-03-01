//
// Created by nkaffine on 12/2/19.
// Copyright (c) 2019 Fulfil Solutions, Inc. All rights reserved.
//
#include <stdexcept>
#include <Fulfil.CPPUtils/file_system_util.h>
#include <cstring>
#include <iostream>
#include <memory>
#include <Fulfil.DepthCam/mocks.h>
#include <Fulfil.DepthCam/core.h>
#include <Fulfil.DepthCam/aruco.h>
#include <Fulfil.DepthCam/visualization.h>

using std::shared_ptr;
using std::make_shared;
using std::string;
using fulfil::depthcam::DeviceManager;
using fulfil::depthcam::Session;
using fulfil::depthcam::aruco::MarkerDetectorContainer;
using fulfil::depthcam::aruco::MarkerDetector;
using std::exception;
using fulfil::depthcam::mocks::MockSessionGenerator;
using fulfil::depthcam::aruco::FixedTransformContainer;
using std::stoi;
using fulfil::depthcam::visualization::SessionVisualizer;
using fulfil::depthcam::aruco::Marker;

/**
 * Mock generator will provide a simple program to generate mock cameras for use in testing.
 * The command line arguments should be as following.
 * Arg1 the destination directory where the directory with mock data will be created
 * Arg2 the name of the directory that will be created to contain the mock data
 * Arg3 the type of mock to create (regular, aruco)
 * Arg4 the number of samples to take
 * Arg5 the width (distance between marker 0 and marker 6) of the aruco container if aruco was chosen.
 * Arg6 the height (distance between marker 0 and marker 2) of the aruco conatiner if aruco was chosen.
 *
 * @note if regular mock is chosen, it will default to including invalid depth data points. If aruco mock is chosen,
 * it will default to not including the invalid depth points. This is because it is easy to filter out the invalid
 * depth points from a regular mock after the fact but it is less reliable with an aruco mock.
 */
int main(int argc, char** argv)
{
    if(argc < 5)
    {
        throw std::runtime_error("invalid number of arguments");
    }
    if(!fulfil::utils::FileSystemUtil::directory_exists(argv[1]))
    {
        throw std::runtime_error("destination directory does not exist");
    }
    shared_ptr<DeviceManager> manager = make_shared<DeviceManager>();
    shared_ptr<Session> session = nullptr;
    if(std::strcmp(argv[3], "regular") == 0)
    {
        session = manager->get_first_connected_session(10);
        std::cout << "creating regular mock" << std::endl;
    }
    else if (std::strcmp(argv[3], "aruco") == 0)
    {
        std::cout << "creating aruco mock" << std::endl;
        if(argc < 7)
        {
            throw std::runtime_error("invalid number of arguments");
        }
        float width = std::stod(argv[5]);
        float height = std::stod(argv[6]);
        shared_ptr<Session> tmp_session = manager->get_first_connected_session(10);
        shared_ptr<MarkerDetectorContainer> container = make_shared<MarkerDetectorContainer>(make_shared<MarkerDetector>(),
                tmp_session, false, false, width, height,
                MarkerDetectorContainer::all_centers(), -1);
        shared_ptr<Eigen::Affine3d> transform = container->get_transform();
        session = make_shared<FixedTransformContainer>(tmp_session, transform, false, width, height);
    }
    else if (std::strcmp(argv[3], "tray") == 0)
    {
      std::cout << "creating tray mock" << std::endl;
      if(argc < 7)
      {
        throw std::runtime_error("invalid number of arguments");
      }
      float width = std::stod(argv[5]);
      float height = std::stod(argv[6]);
      shared_ptr<Session> tmp_session = manager->get_first_connected_session(10);
      std::shared_ptr<std::vector<Marker::Coordinate>> corners = std::make_shared<std::vector<Marker::Coordinate>>();
      corners->push_back(Marker::Coordinate::bottomRight);
      corners->push_back(Marker::Coordinate::topCenter);
      corners->push_back(Marker::Coordinate::bottomLeft);
      corners->push_back(Marker::Coordinate::topCenter);
      corners->push_back(Marker::Coordinate::bottomRight);
      corners->push_back(Marker::Coordinate::topCenter);
      corners->push_back(Marker::Coordinate::bottomLeft);
      corners->push_back(Marker::Coordinate::topCenter);
      shared_ptr<MarkerDetectorContainer> container = make_shared<MarkerDetectorContainer>(make_shared<MarkerDetector>(),
                                                                                           tmp_session, false, false, width, height,
                                                                                           corners, -1);
      shared_ptr<Eigen::Affine3d> transform = container->get_transform();
      session = make_shared<FixedTransformContainer>(tmp_session, transform, false, width, height);
    }
    else
    {
        throw std::runtime_error("invalid mock type");
    }
    shared_ptr<std::string> dir_path = make_shared<string>(argv[1]);
    dir_path->append("/");
    dir_path->append(argv[2]);
    int samples = stoi(argv[4]);
    fulfil::utils::FileSystemUtil::create_directory(dir_path->c_str());
    shared_ptr<MockSessionGenerator> generator = make_shared<MockSessionGenerator>(session, 15);
    generator->save_samples(dir_path, samples);
    return 0;
}
