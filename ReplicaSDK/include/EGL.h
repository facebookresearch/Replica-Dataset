// Copyright (c) Facebook, Inc. and its affiliates. All Rights Reserved
#pragma once

#include <string>

class EGLCtx {
 public:
  EGLCtx(const bool createCtx = true, const int cudaDevice = 0, const bool createSurface = true);
  ~EGLCtx();
  EGLCtx(const EGLCtx&) = delete;
  EGLCtx& operator=(const EGLCtx&) = delete;

  void* (*eglGetCurrentContext)(void);

  void PrintInformation();

 private:
  void* display;
  void* surface;
  void* context;
  void* handle;

  const std::string lib;
  const bool createdCtx;

  unsigned int (*eglInitialize)(void*, int32_t*, int32_t*);
  unsigned int (*eglChooseConfig)(void*, const int32_t*, void**, int32_t, int32_t*);
  void (*(*eglGetProcAddress)(const char*))();
  void* (*eglCreatePbufferSurface)(void*, void*, const int32_t*);
  unsigned int (*eglBindAPI)(unsigned int);
  void* (*eglCreateContext)(void*, void*, void*, const int32_t*);
  unsigned int (*eglMakeCurrent)(void*, void*, void*, void*);
  unsigned int (*eglTerminate)(void*);
};
