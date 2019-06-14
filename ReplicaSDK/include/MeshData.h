// Copyright (c) Facebook, Inc. and its affiliates. All Rights Reserved
#pragma once

#include <pangolin/image/managed_image.h>
#include <Eigen/Core>

struct MeshData {
  MeshData(size_t polygonStride = 3) : polygonStride(polygonStride) {}

  MeshData(const MeshData& other) {
    if (other.vbo.IsValid())
      vbo.CopyFrom(other.vbo);

    if (other.ibo.IsValid())
      ibo.CopyFrom(other.ibo);

    if (other.nbo.IsValid())
      nbo.CopyFrom(other.nbo);

    if (other.cbo.IsValid())
      cbo.CopyFrom(other.cbo);

    polygonStride = other.polygonStride;
  }

  MeshData(MeshData&& other) {
    *this = std::move(other);
  }

  void operator=(MeshData&& other) {
    vbo = (std::move(other.vbo));
    ibo = (std::move(other.ibo));
    nbo = (std::move(other.nbo));
    cbo = (std::move(other.cbo));
    polygonStride = other.polygonStride;
  }

  pangolin::ManagedImage<Eigen::Vector4f> vbo;
  pangolin::ManagedImage<uint32_t> ibo;
  pangolin::ManagedImage<Eigen::Vector4f> nbo;
  pangolin::ManagedImage<Eigen::Matrix<unsigned char, 4, 1>> cbo;
  size_t polygonStride;
};
