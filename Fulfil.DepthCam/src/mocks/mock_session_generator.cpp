//
// Created by nkaffine on 12/2/19.
// Copyright (c) 2019 Fulfil Solutions, Inc. All rights reserved.
//
/**
 * This file contains the implementation for generating mock
 * sessions from sessions with specified number of samples and
 * frames.
 */
#include "Fulfil.DepthCam/mocks/mock_session_generator.h"
#include "Fulfil.DepthCam/coders/matrix3xd_coder.h"
#include <Fulfil.CPPUtils/file_system_util.h>
#include <opencv2/opencv.hpp>
#include <Fulfil.DepthCam/core/transform_depth_session.h>
#include "Fulfil.DepthCam/coders/extrinsics_coder.h"
#include "Fulfil.DepthCam/coders/intrinsics_coder.h"
#include <Fulfil.DepthCam/point_cloud.h>

using fulfil::utils::FileSystemUtil;
using fulfil::depthcam::mocks::MockSessionGenerator;
using std::string;
using std::shared_ptr;
using std::make_shared;
using std::to_string;
using std::endl;
using std::cout;

MockSessionGenerator::MockSessionGenerator(shared_ptr<Session> session, int frames_per_sample)
{
    this->session = session;
    this->frames_per_sample = frames_per_sample;
}

void MockSessionGenerator::save_color_data(shared_ptr<std::string> filename)
{
    cv::imwrite(filename->c_str(), *this->session->get_color_mat());
}

void MockSessionGenerator::save_depth_data(shared_ptr<std::string> filename)
{
    cv::imwrite(filename->c_str(), *this->session->get_depth_mat());
}

void MockSessionGenerator::save_point_cloud(shared_ptr<std::string> directory_path)
{
  shared_ptr<fulfil::depthcam::pointcloud::PointCloud> point_cloud = session->get_point_cloud(true);
  shared_ptr<fulfil::depthcam::pointcloud::CameraPointCloud> camera_cloud = point_cloud->as_camera_cloud();
  shared_ptr<Eigen::Matrix3Xd> data = camera_cloud->get_data();
  point_cloud->encode_to_directory(directory_path);
}

void MockSessionGenerator::save_frame_information(shared_ptr<std::string> frame_directory)
{
    shared_ptr<std::string> rgb_image_name = make_shared<string>();
    rgb_image_name->append(*frame_directory);
    rgb_image_name->append("/color_image.png");
    this->save_color_data(rgb_image_name);

    shared_ptr<std::string> depth_image_name = make_shared<string>();
    depth_image_name->append(*frame_directory);
    depth_image_name->append("/depth_image.png");
    this->save_depth_data(depth_image_name);

    this->save_point_cloud(frame_directory);
}

void MockSessionGenerator::save_samples(shared_ptr<std::string> destination_directory, int num_samples)
{
    if(!FileSystemUtil::directory_exists(destination_directory->c_str()))
    {
      throw std::invalid_argument("invalid destination directory");
    }

    for(int i = 0; i < num_samples; i++)
    {
        shared_ptr<std::string> sample_directory = make_shared<string>();
        sample_directory->append(*destination_directory);
        sample_directory->append("/sample_");
        sample_directory->append(to_string(i));
        FileSystemUtil::create_directory(sample_directory->c_str());
        cout << "processing sample " << i << endl;
        for(int j = 0; j < this->frames_per_sample; j++)
        {
            shared_ptr<std::string> frame_directory = make_shared<string>();
            frame_directory->append(*sample_directory);
            frame_directory->append("/frame_");
            frame_directory->append(to_string(j));
            FileSystemUtil::create_directory(frame_directory->c_str());
            while(true)
            {
                try
                {
                    this->session->refresh();
                    break;
                }
                catch (const std::exception& e)
                {
                    cout << e.what() << endl;
                }
            }
            this->save_frame_information(frame_directory);
        }
    }
}