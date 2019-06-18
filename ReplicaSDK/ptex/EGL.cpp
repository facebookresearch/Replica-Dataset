// Copyright (c) Facebook, Inc. and its affiliates. All Rights Reserved
#include "EGL.h"
#include "Assert.h"

#include <EGL/egl.h>
#include <EGL/eglext.h>
#include <GL/glew.h>
#include <dlfcn.h>
#include <cstring>

EGLCtx::EGLCtx(const bool createCtx, const int cudaDevice, const bool createSurface)
    : lib("libEGL.so"), createdCtx(createCtx) {
  // Try find the DLL (we can't build on anything that doesn't have nvidia drivers without
  // dynamically loading)
  handle = dlopen(lib.c_str(), RTLD_LAZY);

  if (nullptr == handle)
    handle = dlopen(
        "/usr/local/lib/libEGL.so",
        RTLD_LAZY); // dgx machines have this location, which is not on the lib search path

  ASSERT(handle, "Can't find " + lib + ", " + dlerror());

  dlerror(); // Clear any existing error

  char* error = NULL;

  // Pull out functions
  eglGetCurrentContext = (EGLContext(*)(void))dlsym(handle, "eglGetCurrentContext");
  error = dlerror();
  ASSERT(error == NULL, "Error loading eglGetCurrentContext from " + lib + ", " + error);

  eglInitialize = (EGLBoolean(*)(EGLDisplay, EGLint*, EGLint*))dlsym(handle, "eglInitialize");
  error = dlerror();
  ASSERT(error == NULL, "Error loading eglInitialize from " + lib + ", " + error);

  eglChooseConfig = (EGLBoolean(*)(EGLDisplay, const EGLint*, EGLConfig*, EGLint, EGLint*))dlsym(
      handle, "eglChooseConfig");
  error = dlerror();
  ASSERT(error == NULL, "Error loading eglChooseConfig from " + lib + ", " + error);

  eglGetProcAddress =
      (__eglMustCastToProperFunctionPointerType(*)(const char*))dlsym(handle, "eglGetProcAddress");
  error = dlerror();
  ASSERT(error == NULL, "Error loading eglGetProcAddress from " + lib + ", " + error);

  eglCreatePbufferSurface =
      (EGLSurface(*)(EGLDisplay, EGLConfig, const EGLint*))dlsym(handle, "eglCreatePbufferSurface");
  error = dlerror();
  ASSERT(error == NULL, "Error loading eglCreatePbufferSurface from " + lib + ", " + error);

  eglBindAPI = (EGLBoolean(*)(EGLenum))dlsym(handle, "eglBindAPI");
  error = dlerror();
  ASSERT(error == NULL, "Error loading eglBindAPI from " + lib + ", " + error);

  eglCreateContext = (EGLContext(*)(EGLDisplay, EGLConfig, EGLContext, const EGLint*))dlsym(
      handle, "eglCreateContext");
  error = dlerror();
  ASSERT(error == NULL, "Error loading eglCreateContext from " + lib + ", " + error);

  eglMakeCurrent = (EGLBoolean(*)(EGLDisplay, EGLSurface, EGLSurface, EGLContext))dlsym(
      handle, "eglMakeCurrent");
  error = dlerror();
  ASSERT(error == NULL, "Error loading eglMakeCurrent from " + lib + ", " + error);

  eglTerminate = (EGLBoolean(*)(EGLDisplay))dlsym(handle, "eglTerminate");
  error = dlerror();
  ASSERT(error == NULL, "Error loading eglTerminate from " + lib + ", " + error);

  if (createCtx) {
    const EGLint configAttribs[] = {EGL_SURFACE_TYPE,
                                    EGL_PBUFFER_BIT,
                                    EGL_BLUE_SIZE,
                                    8,
                                    EGL_GREEN_SIZE,
                                    8,
                                    EGL_RED_SIZE,
                                    8,
                                    EGL_DEPTH_SIZE,
                                    0,
                                    EGL_RENDERABLE_TYPE,
                                    EGL_OPENGL_BIT,
                                    EGL_NONE};

    const EGLint pbufferAttribs[] = {
        EGL_WIDTH,
        1920,
        EGL_HEIGHT,
        1080,
        EGL_NONE,
    };

    EGLDeviceEXT eglDevs[32];
    EGLint numDevices;

    PFNEGLQUERYDEVICESEXTPROC eglQueryDevicesEXT =
        (PFNEGLQUERYDEVICESEXTPROC)eglGetProcAddress("eglQueryDevicesEXT");

    eglQueryDevicesEXT(32, eglDevs, &numDevices);

    ASSERT(numDevices, "Found no GPUs");

    PFNEGLQUERYDEVICEATTRIBEXTPROC eglQueryDeviceAttribEXT =
        reinterpret_cast<PFNEGLQUERYDEVICEATTRIBEXTPROC>(
            eglGetProcAddress("eglQueryDeviceAttribEXT"));

    int eglDevId = 0;

    // Find the CUDA device asked for
    for (; eglDevId < numDevices; ++eglDevId) {
      EGLAttrib cudaDevNumber;

      if (eglQueryDeviceAttribEXT(eglDevs[eglDevId], EGL_CUDA_DEVICE_NV, &cudaDevNumber) ==
          EGL_FALSE)
        continue;

      if (cudaDevNumber == cudaDevice) {
        break;
      }
    }

    PFNEGLGETPLATFORMDISPLAYEXTPROC eglGetPlatformDisplayEXT =
        (PFNEGLGETPLATFORMDISPLAYEXTPROC)eglGetProcAddress("eglGetPlatformDisplayEXT");

    display = eglGetPlatformDisplayEXT(EGL_PLATFORM_DEVICE_EXT, eglDevs[eglDevId], 0);
    ASSERT(display != EGL_NO_DISPLAY, "Can't create EGL display");

    EGLint major, minor;
    ASSERT(eglInitialize(display, &major, &minor), "Can't init EGL");

    EGLint numConfigs;
    EGLConfig eglCfg;
    ASSERT(eglChooseConfig(display, configAttribs, &eglCfg, 1, &numConfigs), "Can't configure EGL");

    if (createSurface) {
      surface = eglCreatePbufferSurface(display, eglCfg, pbufferAttribs);
      ASSERT(surface != EGL_NO_SURFACE, "Can't create EGL surface");
    }

    ASSERT(eglBindAPI(EGL_OPENGL_API), "Can't bind EGL OpenGL API");

    context = eglCreateContext(display, eglCfg, EGL_NO_CONTEXT, NULL);
    ASSERT(context != EGL_NO_CONTEXT, "Can't create EGL context");

    if (createSurface) {
      ASSERT(eglMakeCurrent(display, surface, surface, context), "Can't bind EGL context");
    } else {
      ASSERT(
          eglMakeCurrent(display, EGL_NO_SURFACE, EGL_NO_SURFACE, context),
          "Can't bind EGL context");
    }

    GLenum err = glewInit();

#ifdef GLEW_ERROR_NO_GLX_DISPLAY
    if (err == GLEW_ERROR_NO_GLX_DISPLAY) {
        std::cout << "Can't initialize EGL GLEW GLX display, may crash!" << std::endl;
    }
    else 
#endif
    if (err != GLEW_OK) {
        ASSERT(false, "Can't initialize EGL, glewInit failing completely.");
    }

    // Setup default OpenGL parameters
    glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);
    glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);
    glHint(GL_POLYGON_SMOOTH_HINT, GL_NICEST);
    glEnable(GL_BLEND);
    glEnable(GL_LINE_SMOOTH);
    glEnable(GL_DEPTH_TEST);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glLineWidth(1.5);
    glPixelStorei(GL_PACK_ALIGNMENT, 1);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
  }
}

EGLCtx::~EGLCtx() {
  if (createdCtx) {
    ASSERT(
        eglMakeCurrent(display, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT),
        "Can't remove EGL context");
    eglTerminate(display);
  }
  dlclose(handle);
}

// Everything used in PrintInformation(); comes from
// https://github.com/KDAB/eglinfo/blob/master/main.cpp#L310
struct device_property_t {
  int32_t name;
  const char* displayName;
  const char* extension;
  enum Type { String, Attribute } type;
};

static const device_property_t deviceProperties[]{
#ifdef EGL_DRM_DEVICE_FILE_EXT
    {EGL_DRM_DEVICE_FILE_EXT, "DRM device file", "EGL_EXT_device_drm", device_property_t::String},
#endif
#ifdef EGL_CUDA_DEVICE_NV
    {EGL_CUDA_DEVICE_NV, "CUDA device", "EGL_NV_device_cuda", device_property_t::Attribute}
#endif
};

static const int devicePropertiesSize = sizeof(deviceProperties) / sizeof(device_property_t);

void EGLCtx::PrintInformation() {
  PFNEGLQUERYDEVICESEXTPROC eglQueryDevicesEXT =
      reinterpret_cast<PFNEGLQUERYDEVICESEXTPROC>(eglGetProcAddress("eglQueryDevicesEXT"));
  EGLDeviceEXT devices[32];
  EGLint num_devices;
  if (!eglQueryDevicesEXT(32, devices, &num_devices)) {
    std::cout << "Failed to query devices." << std::endl;
    return;
  }
  if (num_devices == 0) {
    std::cout << "Found no devices." << std::endl;
    return;
  }

  std::cout << "Found " << num_devices << " device(s)." << std::endl;
  PFNEGLQUERYDEVICEATTRIBEXTPROC eglQueryDeviceAttribEXT =
      reinterpret_cast<PFNEGLQUERYDEVICEATTRIBEXTPROC>(
          eglGetProcAddress("eglQueryDeviceAttribEXT"));
  PFNEGLQUERYDEVICESTRINGEXTPROC eglQueryDeviceStringEXT =
      reinterpret_cast<PFNEGLQUERYDEVICESTRINGEXTPROC>(
          eglGetProcAddress("eglQueryDeviceStringEXT"));

  for (int i = 0; i < num_devices; ++i) {
    std::cout << "Device " << i << ":" << std::endl;
    EGLDeviceEXT device = devices[i];
    const char* devExts = eglQueryDeviceStringEXT(device, EGL_EXTENSIONS);
    if (devExts) {
      std::cout << "  Device Extensions: ";
      if (std::strlen(devExts))
        std::cout << devExts << std::endl;
      else
        std::cout << "none" << std::endl;
    } else {
      std::cout << "  Failed to retrieve device extensions." << std::endl;
    }

    for (int j = 0; j < devicePropertiesSize; ++j) {
      const auto property = deviceProperties[j];
      if (!devExts || strstr(devExts, property.extension) == nullptr)
        continue;
      switch (property.type) {
        case device_property_t::String: {
          const char* value = eglQueryDeviceStringEXT(device, property.name);
          std::cout << "  " << property.displayName << ": " << value << std::endl;
          break;
        }
        case device_property_t::Attribute: {
          EGLAttrib attrib;
          if (eglQueryDeviceAttribEXT(device, property.name, &attrib) == EGL_FALSE)
            break;
          std::cout << "  " << property.displayName << ": " << attrib << std::endl;
          break;
        }
      }
    }

    std::cout << std::endl;
  }
}
