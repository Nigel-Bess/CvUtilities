#pragma once

namespace nlohmann {
  template <>
  struct adl_serializer<Eigen::Vector3d> {
    static void to_json(json& j, const Eigen::Vector3d& v) { j = json::array({ v.x(), v.y(), v.z() }); }
    static void from_json(const json& j, Eigen::Vector3d& v) { v = { j.at(0), j.at(1), j.at(2) }; }
  };

    template <>
  struct adl_serializer<Eigen::Vector3i> {
    static void to_json(json& j, const Eigen::Vector3i& v) {
      j = json::array({ v.x(), v.y(), v.z() });
    }
    static void from_json(const json& j, Eigen::Vector3i& v) {
      v = Eigen::Vector3i{
        j.at(0).get<int>(),
        j.at(1).get<int>(),
        j.at(2).get<int>()
      };
    }
  };

  template <>
  struct adl_serializer<Eigen::Matrix3d> {
    static void to_json(json& j, const Eigen::Matrix3d& m) {
      j = json::array({
        json::array({m(0,0), m(0,1), m(0,2)}),
        json::array({m(1,0), m(1,1), m(1,2)}),
        json::array({m(2,0), m(2,1), m(2,2)})
      });
    }

    static void from_json(const json& j, Eigen::Matrix3d& m) {
      for (int r = 0; r < 3; ++r)
        for (int c = 0; c < 3; ++c)
          m(r,c) = j.at(r).at(c);
    }
  };
}
