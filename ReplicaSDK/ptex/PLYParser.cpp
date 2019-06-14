// Copyright (c) Facebook, Inc. and its affiliates. All Rights Reserved
#include "PLYParser.h"
#include "Assert.h"

#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>
#include <experimental/filesystem>
#include <fstream>
#include <set>

void PLYParse(MeshData& meshData, const std::string& filename) {
  std::vector<std::string> comments;
  std::vector<std::string> objInfo;

  std::string lastElement;
  std::string lastProperty;

  enum Properties { POSITION = 0, NORMAL, COLOR, NUM_PROPERTIES };

  size_t numVertices = 0;

  size_t positionDimensions = 0;
  size_t normalDimensions = 0;
  size_t colorDimensions = 0;

  std::vector<Properties> vertexLayout;

  size_t numFaces = 0;

  std::ifstream file(filename, std::ios::binary);

  // Header parsing
  {
    std::string line;

    while (std::getline(file, line)) {
      std::istringstream ls(line);
      std::string token;
      ls >> token;

      if (token == "ply" || token == "PLY" || token == "") {
        // Skip preamble line
        continue;
      } else if (token == "comment") {
        // Just store these incase
        comments.push_back(line.erase(0, 8));
      } else if (token == "format") {
        // We can only parse binary data, so check that's what it is
        std::string s;
        ls >> s;
        ASSERT(
            s == "binary_little_endian",
            "Can only parse binary files... why are you using ASCII anyway?");
      } else if (token == "element") {
        std::string name;
        size_t size;
        ls >> name >> size;

        if (name == "vertex") {
          // Pull out the number of vertices
          numVertices = size;
        } else if (name == "face") {
          // Pull out number of faces
          numFaces = size;
        } else {
          ASSERT(false, "Can't parse element (%)", name);
        }

        // Keep track of what element we parsed last to associate the properties that follow
        lastElement = name;
      } else if (token == "property") {
        std::string type, name;
        ls >> type;

        // Special parsing for list properties (e.g. faces)
        bool isList = false;

        if (type == "list") {
          isList = true;

          std::string countType;
          ls >> countType >> type;

          ASSERT(
              countType == "uchar" || countType == "uint8",
              "Don't understand count type (%)",
              countType);

          ASSERT(type == "int", "Don't understand index type (%)", type);

          ASSERT(
              lastElement == "face",
              "Only expecting list after face element, not after (%)",
              lastElement);
        }

        ASSERT(
            type == "float" || type == "int" || type == "uchar" || type == "uint8",
            "Don't understand type (%)",
            type);

        ls >> name;

        // Collecting vertex property information
        if (lastElement == "vertex") {
          ASSERT(type != "int", "Don't support 32-bit integer properties");

          // Position information
          if (name == "x") {
            positionDimensions = 1;
            vertexLayout.push_back(Properties::POSITION);
            ASSERT(type == "float", "Don't support 8-bit integer positions");
          } else if (name == "y") {
            ASSERT(lastProperty == "x", "Properties should follow x, y, z, (w) order");
            positionDimensions = 2;
          } else if (name == "z") {
            ASSERT(lastProperty == "y", "Properties should follow x, y, z, (w) order");
            positionDimensions = 3;
          } else if (name == "w") {
            ASSERT(lastProperty == "z", "Properties should follow x, y, z, (w) order");
            positionDimensions = 4;
          }

          // Normal information
          if (name == "nx") {
            normalDimensions = 1;
            vertexLayout.push_back(Properties::NORMAL);
            ASSERT(type == "float", "Don't support 8-bit integer normals");
          } else if (name == "ny") {
            ASSERT(lastProperty == "nx", "Properties should follow nx, ny, nz order");
            normalDimensions = 2;
          } else if (name == "nz") {
            ASSERT(lastProperty == "ny", "Properties should follow nx, ny, nz order");
            normalDimensions = 3;
          }

          // Color information
          if (name == "red") {
            colorDimensions = 1;
            vertexLayout.push_back(Properties::COLOR);
            ASSERT(type == "uchar" || type == "uint8", "Don't support non-8-bit integer colors");
          } else if (name == "green") {
            ASSERT(
                lastProperty == "red", "Properties should follow red, green, blue, (alpha) order");
            colorDimensions = 2;
          } else if (name == "blue") {
            ASSERT(
                lastProperty == "green",
                "Properties should follow red, green, blue, (alpha) order");
            colorDimensions = 3;
          } else if (name == "alpha") {
            ASSERT(
                lastProperty == "blue", "Properties should follow red, green, blue, (alpha) order");
            colorDimensions = 4;
          }
        } else if (lastElement == "face") {
          ASSERT(isList, "No idea what to do with properties following faces");
        } else {
          ASSERT(false, "No idea what to do with properties before elements");
        }

        lastProperty = name;
      } else if (token == "obj_info") {
        // Just store these incase
        objInfo.push_back(line.erase(0, 9));
      } else if (token == "end_header") {
        // Done reading!
        break;
      } else {
        // Something unrecognised
        ASSERT(false);
      }
    }

    // Check things make sense.
    ASSERT(numVertices > 0);
    ASSERT(positionDimensions > 0);
  }

  meshData.vbo.Reinitialise(numVertices, 1);
  meshData.vbo.Fill(Eigen::Vector4f(0, 0, 0, 1));

  if (normalDimensions) {
    meshData.nbo.Reinitialise(numVertices, 1);
    meshData.nbo.Fill(Eigen::Vector4f(0, 0, 0, 1));
  }

  if (colorDimensions) {
    meshData.cbo.Reinitialise(numVertices, 1);
    meshData.cbo.Fill(Eigen::Matrix<unsigned char, 4, 1>(0, 0, 0, 255));
  }

  // Can only be FLOAT32 or UINT8
  const size_t positionBytes = positionDimensions * sizeof(float); // floats
  const size_t normalBytes = normalDimensions * sizeof(float); // floats
  const size_t colorBytes = colorDimensions * sizeof(uint8_t); // bytes

  const size_t vertexPacketSizeBytes = positionBytes + normalBytes + colorBytes;

  size_t positionOffsetBytes = 0;
  size_t normalOffsetBytes = 0;
  size_t colorOffsetBytes = 0;

  size_t offsetSoFarBytes = 0;

  for (size_t i = 0; i < vertexLayout.size(); i++) {
    if (vertexLayout[i] == Properties::POSITION) {
      positionOffsetBytes = offsetSoFarBytes;
      offsetSoFarBytes += positionBytes;
    } else if (vertexLayout[i] == Properties::NORMAL) {
      normalOffsetBytes = offsetSoFarBytes;
      offsetSoFarBytes += normalBytes;
    } else if (vertexLayout[i] == Properties::COLOR) {
      colorOffsetBytes = offsetSoFarBytes;
      offsetSoFarBytes += colorBytes;
    } else {
      ASSERT(false);
    }
  }

  // Close after parsing header and re-open memory mapped
  const size_t postHeader = file.tellg();

  file.close();

  const size_t fileSize = std::experimental::filesystem::file_size(filename);

  int fd = open(filename.c_str(), O_RDONLY, 0);
  void* mmappedData = mmap(NULL, fileSize, PROT_READ, MAP_PRIVATE | MAP_POPULATE, fd, 0);

  // Parse each vertex packet and unpack
  char* bytes = &(((char*)mmappedData)[postHeader]);

  for (size_t i = 0; i < numVertices; i++) {
    char* nextBytes = &bytes[vertexPacketSizeBytes * i];

    memcpy(meshData.vbo[i].data(), &nextBytes[positionOffsetBytes], positionBytes);

    if (normalDimensions)
      memcpy(meshData.nbo[i].data(), &nextBytes[normalOffsetBytes], normalBytes);

    if (colorDimensions)
      memcpy(meshData.cbo[i].data(), &nextBytes[colorOffsetBytes], colorBytes);
  }

  const size_t bytesSoFar = postHeader + vertexPacketSizeBytes * numVertices;

  bytes = &(((char*)mmappedData)[postHeader + vertexPacketSizeBytes * numVertices]);

  if (numFaces > 0) {
    // Read first face to get number of indices;
    const uint8_t faceDimensions = *bytes;

    ASSERT(faceDimensions == 3 || faceDimensions == 4);

    const size_t countBytes = 1;
    const size_t faceBytes = faceDimensions * sizeof(uint32_t); // uint32_t
    const size_t facePacketSizeBytes = countBytes + faceBytes;

    const size_t predictedFaces = (fileSize - bytesSoFar) / facePacketSizeBytes;

    // Not sure what to do here
    //    if(predictedFaces < numFaces)
    //    {
    //        std::cout << "Skipping " << numFaces - predictedFaces << " missing faces" <<
    //        std::endl;
    //    }
    //    else if(numFaces < predictedFaces)
    //    {
    //        std::cout << "Ignoring " << predictedFaces - numFaces << " extra faces" << std::endl;
    //    }

    numFaces = std::min(numFaces, predictedFaces);

    meshData.ibo.Reinitialise(numFaces * faceDimensions, 1);

    for (size_t i = 0; i < numFaces; i++) {
      char* nextBytes = &bytes[facePacketSizeBytes * i];

      memcpy(&meshData.ibo[i * faceDimensions], &nextBytes[countBytes], faceBytes);
    }

    meshData.polygonStride = faceDimensions;
  } else {
    meshData.polygonStride = 0;
  }

  munmap(mmappedData, fileSize);

  close(fd);
}
