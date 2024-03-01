//
// Created by nkaffine on 12/2/19.
// Copyright (c) 2019 Fulfil Solutions, Inc. All rights reserved.
//
/**
 * This file contains the implementation of the kabsch algorithm
 * which is used to find the best transformation from a set of
 * points in one coordinate system to a corresponding set
 * of points in another coordainte system.
 */
#include<memory>
#include"Fulfil.DepthCam/aruco/kabsch_helper.h"
#include<vector>
#include<eigen3/Eigen/Geometry>
using fulfil::depthcam::KabschHelper;

/**
 * Helper function grabbed from https://github.com/oleg-alexandrov/projects/blob/master/eigen/Kabsch.cpp.
 */
// Given two sets of 3D points, find the rotation + translation + scale
// which best maps the first set to the second.
// Source: http://en.wikipedia.org/wiki/Kabsch_algorithm

// The input 3D points are stored as columns.
std::shared_ptr<Eigen::Affine3d> Find3DAffineTransform(const std::shared_ptr<Eigen::Matrix3Xd>& in,
    const std::shared_ptr<Eigen::Matrix3Xd>& out) {

    // Default output
    std::shared_ptr<Eigen::Affine3d> A = std::make_shared<Eigen::Affine3d>();
    A->linear() = Eigen::Matrix3d::Identity(3, 3);
    A->translation() = Eigen::Vector3d::Zero();

    if (in->cols() != out->cols()) {
      throw std::runtime_error("Find3DAffineTransform(): input data mis-match");
    }
    // First find the scale, by finding the ratio of sums of some distances,
    // then bring the datasets to the same scale.
    double dist_in = 0, dist_out = 0;
    for (int col = 0; col < in->cols()-1; col++) {
        dist_in  += (in->col(col+1) - in->col(col)).norm();
        dist_out += (out->col(col+1) - out->col(col)).norm();
    }
    if (dist_in <= 0 || dist_out <= 0)
        return A;
    double scale = dist_out/dist_in;
    (*out) /= scale;

    // Find the centroids then shift to the origin
    Eigen::Vector3d in_ctr = Eigen::Vector3d::Zero();
    Eigen::Vector3d out_ctr = Eigen::Vector3d::Zero();
    for (int col = 0; col < in->cols(); col++) {
        in_ctr  += in->col(col);
        out_ctr += out->col(col);
    }
    in_ctr /= in->cols();
    out_ctr /= out->cols();
    for (int col = 0; col < in->cols(); col++) {
        in->col(col)  -= in_ctr;
        out->col(col) -= out_ctr;
    }

    // SVD
    Eigen::MatrixXd Cov = (*in) * out->transpose();
    Eigen::JacobiSVD<Eigen::MatrixXd> svd(Cov, Eigen::ComputeThinU | Eigen::ComputeThinV);

    // Find the rotation
    double d = (svd.matrixV() * svd.matrixU().transpose()).determinant();
    if (d > 0)
        d = 1.0;
    else
        d = -1.0;
    Eigen::Matrix3d I = Eigen::Matrix3d::Identity(3, 3);
    I(2, 2) = d;
    Eigen::Matrix3d R = svd.matrixV() * I * svd.matrixU().transpose();

    // The final transform
    (*A).linear() = scale * R;
    (*A).translation() = scale*(out_ctr - R*in_ctr);

    return A;
}

// Why does this exist? I assume that the algos were supposed to be pass able?
std::shared_ptr<Eigen::Affine3d> KabschHelper::find_translation_between_points(
        std::shared_ptr<Eigen::Matrix3Xd> initial_points,
        std::shared_ptr<Eigen::Matrix3Xd> destination_points)
{
    return Find3DAffineTransform(initial_points, destination_points);
}



Eigen::Affine3d KabschHelper::fit_transform_between_points(Eigen::Matrix3Xd in, Eigen::Matrix3Xd out) {

    // Default output
    Eigen::Affine3d A;
    A.linear() = Eigen::Matrix3d::Identity(3, 3);
    A.translation() = Eigen::Vector3d::Zero();

    if (in.cols() != out.cols()) {
      throw std::runtime_error("Find3DAffineTransform(): input data mis-match");
    }
    // First find the scale, by finding the ratio of sums of some distances,
    // then bring the datasets to the same scale.
    double dist_in = 0;
    double dist_out = 0;
    for (int col = 0; col < in.cols()-1; col++) {
      dist_in  += (in.col(col+1) - in.col(col)).norm();
      dist_out += (out.col(col+1) - out.col(col)).norm();
    }
    if (dist_in <= 0 || dist_out <= 0) { return A; }
    double scale = dist_out/dist_in;
    out /= scale;

    // Find the centroids then shift to the origin
    Eigen::Vector3d in_ctr = Eigen::Vector3d::Zero();
    Eigen::Vector3d out_ctr = Eigen::Vector3d::Zero();
    for (int col = 0; col < in.cols(); col++) {
      in_ctr  += in.col(col);
      out_ctr += out.col(col);
    }
    in_ctr /= in.cols();
    out_ctr /= out.cols();
    for (int col = 0; col < in.cols(); col++) {
      in.col(col)  -= in_ctr;
      out.col(col) -= out_ctr;
    }

    // SVD
    Eigen::MatrixXd Cov = in * out.transpose();
    Eigen::JacobiSVD<Eigen::MatrixXd> svd(Cov, Eigen::ComputeThinU | Eigen::ComputeThinV);

    // Find the rotation
    double determinant = (svd.matrixV() * svd.matrixU().transpose()).determinant();
    determinant = (determinant > 0) ? 1  : -1;
    Eigen::Matrix3d Id_mat = Eigen::Matrix3d::Identity(3, 3);
    Id_mat(2, 2) = determinant;
    Eigen::Matrix3d R = svd.matrixV() * Id_mat * svd.matrixU().transpose();

    // The final transform
    A.linear() = scale * R;
    A.translation() = scale*(out_ctr - R*in_ctr);

    return A;
}

