// Copyright (c) Facebook, Inc. and its affiliates. All Rights Reserved
#include "MirrorSurface.h"

// create Eigen matrix from Json
inline Eigen::MatrixXd EigenMatrixFromJson(const picojson::value& json) {
  const int rows = int(json.size());
  if (rows > 0 && json[0].is<picojson::array>()) {
    // matrix
    const int cols = int(json[0].size());
    Eigen::MatrixXd mat(rows, cols);
    for (size_t r = 0; r < json.size(); ++r) {
      for (size_t c = 0; c < json[0].size(); ++c) {
        mat(r, c) = json[r][c].get<double>();
      }
    }
    return mat;
  } else {
    Eigen::MatrixXd arr(json.size(), 1);
    for (size_t r = 0; r < json.size(); ++r) {
      arr(r, 0) = json[r].get<double>();
    }
    return arr;
  }
}

MirrorSurface::MirrorSurface(const picojson::value& json) {
  centroid_w = EigenMatrixFromJson(json["centroid_w"]).cast<float>();
  plane_w = EigenMatrixFromJson(json["plane_w"]).cast<float>();

  T_mani_plane = EigenMatrixFromJson(json["T_mani_plane"]).cast<float>();
  T_plane_mani = EigenMatrixFromJson(json["T_plane_mani"]).cast<float>();

  picojson::value boundary_maniJson = json["boundary_mani"];
  picojson::value boundary_wJson = json["boundary_w"];
  picojson::value bounding_rect_wJson = json["bounding_rect_w"];

  for (size_t i = 0; i < boundary_maniJson.size(); i++) {
    boundary_mani.push_back(EigenMatrixFromJson(boundary_maniJson[i]).cast<float>());
  }

  for (size_t i = 0; i < boundary_wJson.size(); i++) {
    boundary_w.push_back(EigenMatrixFromJson(boundary_wJson[i]).cast<float>());
  }

  for (size_t i = 0; i < bounding_rect_wJson.size(); i++) {
    bounding_rect_w.push_back(EigenMatrixFromJson(bounding_rect_wJson[i]).cast<float>());
  }

  reflectivity = json["reflectivity"].get<double>();
}

MirrorSurface::~MirrorSurface() {}

const Eigen::Vector4f& MirrorSurface::Equation() const {
  return plane_w;
}

const std::vector<Eigen::Vector3f>& MirrorSurface::Boundary_w() const {
  return boundary_w;
}

const std::vector<Eigen::Vector2f>& MirrorSurface::Boundary_mani() const {
  return boundary_mani;
}

const std::vector<Eigen::Vector3f> MirrorSurface::BoundingRect_w() const {
  return bounding_rect_w;
}

const Eigen::Vector3f& MirrorSurface::Centroid() const {
  return centroid_w;
}

const float& MirrorSurface::Reflectivity() const {
  return reflectivity;
}

size_t MirrorSurface::NumBoundaryPoints() const {
  return boundary_mani.size();
}

Eigen::Matrix<float, 2, 4> MirrorSurface::T_manifold_plane() const {
  return T_mani_plane;
}

bool MirrorSurface::InBoundary(const Eigen::Vector4f& p_w) {
  Eigen::Vector2f pointInMani = (T_mani_plane * p_w).head<2>();

  int v, u;

  bool inBoundary = false;

  int nVert = boundary_mani.size();
  for (v = 0, u = nVert - 1; v < nVert; u = v++) {
    if (((boundary_mani[v](1) > pointInMani(1)) != (boundary_mani[u](1) > pointInMani(1))) &&
        (pointInMani(0) < (boundary_mani[u](0) - boundary_mani[v](0)) *
                 (pointInMani(1) - boundary_mani[v](1)) /
                 (boundary_mani[u](1) - boundary_mani[v](1)) +
             boundary_mani[v](0))) {
      inBoundary = !inBoundary;
    }
  }

  return inBoundary;
}

void MirrorSurface::GenerateMask(pangolin::ManagedImage<float>& image, int w, int h) {
  std::vector<Eigen::Vector2f> bounding_rect_mani;

  for (size_t i = 0; i < bounding_rect_w.size(); i++) {
    bounding_rect_mani.push_back(T_mani_plane * Unproject(bounding_rect_w[i]));
  }

  image.Reinitialise(w, h);

  for (int y = 0; y < h; y++) {
    for (int x = 0; x < w; x++) {
      const float u = 1.0f - ((x + 0.5f) / (float)w);
      const float v = 1.0f - ((y + 0.5f) / (float)h);
      const Eigen::Vector2f p0 = u * bounding_rect_mani[0] + (1.0f - u) * bounding_rect_mani[1];
      const Eigen::Vector2f p1 = u * bounding_rect_mani[2] + (1.0f - u) * bounding_rect_mani[3];
      const Eigen::Vector2f p_mani = v * p0 + (1.0f - v) * p1;
      const Eigen::Vector3f p_w = T_plane_mani * Unproject(p_mani);
      image(x, y) = InBoundary(Eigen::Vector4f(p_w(0), p_w(1), p_w(2), 1.0f)) ? 1.0f : 0.0f;
    }
  }
}
