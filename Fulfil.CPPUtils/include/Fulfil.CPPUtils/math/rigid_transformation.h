#pragma once
#include <Fulfil.CPPUtils/json/nlohmann_serializers.h>

struct RigidTransformation {
    Eigen::Matrix3d Rotation = Eigen::Matrix3d::Identity();
    Eigen::Vector3d Translation = Eigen::Vector3d::Zero();

    /**
     * Applies this rigid transform to a single 3D point.
     *
     * Interprets the input as a point expressed in the local coordinate system
     * and returns its position in the parent (world) coordinate system:
     *
     *   p_out = R * p_in + t
     */
    inline Eigen::Vector3d Apply(const Eigen::Vector3d& vec) const {
        return Rotation * vec + Translation;
    }

    /**
     * Returns the inverse of this rigid transform.
     *
     * The inverse converts points from the parent (world) coordinate system
     * back into this transform's local coordinate system.
     *
     * For a rigid transform, the inverse is computed analytically:
     *  - rotation is the transpose of the original rotation
     *  - translation is -Rᵀ * t
     */
    inline RigidTransformation Inverse() const {
        auto rInv = Rotation.transpose();
        return { rInv, -rInv * Translation };
    }

    /**
     * Applies this rigid transform to a batch of 3D points.
     *
     * Expects a 3×N matrix where each column represents a point.
     * The transform is applied to every column, returning a new 3×N matrix
     * in the parent (world) coordinate system:
     *
     *   P_out = R * P_in + t (applied column-wise)
     */
    inline Eigen::Matrix3Xd Apply(const Eigen::Matrix3Xd& points) const {
        return (Rotation * points).colwise() + Translation;
    }
};

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(RigidTransformation, Rotation, Translation)