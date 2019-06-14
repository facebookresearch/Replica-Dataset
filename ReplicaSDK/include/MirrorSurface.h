// Copyright (c) Facebook, Inc. and its affiliates. All Rights Reserved
#pragma once
#include <pangolin/gl/gl.h>
#include <pangolin/image/managed_image.h>
#include <pangolin/utils/picojson.h>
#include <vector>

// Homogenise input
template <typename Derived>
EIGEN_STRONG_INLINE static Eigen::
    Matrix<typename Derived::Scalar, Derived::RowsAtCompileTime + 1, 1>
    Unproject(const Eigen::MatrixBase<Derived>& v) {
  Eigen::Matrix<typename Derived::Scalar, Derived::RowsAtCompileTime + 1, 1> out;
  out.template head<Derived::RowsAtCompileTime>() = v;
  out.template tail<1>()[0] = static_cast<typename Derived::Scalar>(1.0);
  return out;
}

// represents mirror plane and boundary
class MirrorSurface {
 public:
  MirrorSurface(const picojson::value& json);
  virtual ~MirrorSurface();

  const std::vector<Eigen::Vector3f>& Boundary_w() const;

  const std::vector<Eigen::Vector2f>& Boundary_mani() const;

  const std::vector<Eigen::Vector3f> BoundingRect_w() const;

  const Eigen::Vector4f& Equation() const;

  const Eigen::Vector3f& Centroid() const;

  const float& Reflectivity() const;

  size_t NumBoundaryPoints() const;

  Eigen::Matrix<float, 2, 4> T_manifold_plane() const;

  void GenerateMask(pangolin::ManagedImage<float>& image, int w, int h);

 private:
  bool InBoundary(const Eigen::Vector4f& p_w);

  Eigen::Vector3f centroid_w;
  Eigen::Vector4f plane_w;

  Eigen::Matrix<float, 2, 4> T_mani_plane;
  Eigen::Matrix3f T_plane_mani;

  std::vector<Eigen::Vector2f> boundary_mani;
  std::vector<Eigen::Vector3f> boundary_w;

  std::vector<Eigen::Vector3f> bounding_rect_w;

  float reflectivity;
};
