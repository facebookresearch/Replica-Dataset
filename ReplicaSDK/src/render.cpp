#include <EGL.h>
#include <PTexLib.h>

int main(int argc, char* argv[]) {
  ASSERT(argc == 3, "Usage: ./ReplicaRenderer mesh.ply /path/to/atlases");

  const std::string meshFile(argv[1]);
  const std::string atlasFolder(argv[2]);

  ASSERT(pangolin::FileExists(meshFile));
  ASSERT(pangolin::FileExists(atlasFolder));

  const int width = 1280;
  const int height = 960;

  // Setup EGL
  EGLCtx egl;

  egl.PrintInformation();

  // Setup a framebuffer
  pangolin::GlTexture render(width, height);
  pangolin::GlRenderBuffer renderBuffer(width, height);
  pangolin::GlFramebuffer frameBuffer(render, renderBuffer);

  // Setup a camera
  pangolin::OpenGlRenderState s_cam(
      pangolin::ProjectionMatrixRDF_BottomLeft(
          width,
          height,
          width / 2.0f,
          width / 2.0f,
          (width - 1.0f) / 2.0f,
          (height - 1.0f) / 2.0f,
          0.1f,
          100.0f),
      pangolin::ModelViewLookAtRDF(0, 0, 0, 0, 0, 1, 0, -1, 0));

  // Start at some origin
  Eigen::Matrix4d T_camera_world = s_cam.GetModelViewMatrix();

  // And move to the left
  Eigen::Matrix4d T_new_old = Eigen::Matrix4d::Identity();

  T_new_old.topRightCorner(3, 1) = Eigen::Vector3d(-0.005, 0, 0);

  PTexMesh ptexMesh(meshFile, atlasFolder);

  pangolin::ManagedImage<Eigen::Matrix<uint8_t, 3, 1>> image(width, height);

  // Render some frames
  const size_t numFrames = 100;
  for (size_t i = 0; i < numFrames; i++) {
    std::cout << "\rRendering frame " << i + 1 << "/" << numFrames << "... ";
    std::cout.flush();

    // Render
    frameBuffer.Bind();

    glPushAttrib(GL_VIEWPORT_BIT);
    glViewport(0, 0, width, height);
    glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);

    ptexMesh.Render(s_cam);

    glPopAttrib();
    frameBuffer.Unbind();

    // Download and save
    render.Download(image.ptr, GL_RGB, GL_UNSIGNED_BYTE);

    char filename[1000];
    snprintf(filename, 1000, "frame%06zu.jpg", i);

    pangolin::SaveImage(
        image.UnsafeReinterpret<uint8_t>(),
        pangolin::PixelFormatFromString("RGB24"),
        std::string(filename));

    // Move the camera
    T_camera_world = T_camera_world * T_new_old.inverse();

    s_cam.GetModelViewMatrix() = T_camera_world;
  }
  std::cout << "\rRendering frame " << numFrames << "/" << numFrames << "... done" << std::endl;

  return 0;
}
