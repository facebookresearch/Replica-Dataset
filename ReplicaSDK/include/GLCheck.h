// Copyright (c) Facebook, Inc. and its affiliates. All Rights Reserved
#pragma once
#include <pangolin/gl/gl.h>
#include <pangolin/utils/log.h>

namespace {
 constexpr float minGLVersion = 4.3;
}

// Check that the runtime GL version is compatible
bool checkGLVersion() {
  const GLubyte* glStr = glGetString(GL_VERSION);	  
  
  if(glStr == nullptr) {
    pango_print_error("No openGL version found. Do you have an openGL context (e.g. from glewInit()) ?\n");
    return false;
  }
  
  const std::string glVersionStr(reinterpret_cast<const char*>(glStr));
  
  if(std::stof(glVersionStr) < minGLVersion) {
    pango_print_error("Insufficient OpenGL version: %s minimum required is %.1f\n", glVersionStr.c_str(), minGLVersion);
    pango_print_error("Mesa users: try forcing compatibility with MESA_GL_VERSION_OVERRIDE=%.1f"
		   " MESA_GLSL_VERSION_OVERRIDE=%d\n", minGLVersion, static_cast<int>(minGLVersion*100));
    return false;
  }
  return true;
}


