// Copyright (c) Facebook, Inc. and its affiliates. All Rights Reserved
// Render mirrors and glass using OpenGL, reflecting scene around mirror plane
#pragma once
#include <pangolin/gl/gl.h>
#include <pangolin/gl/glsl.h>
#include <Eigen/Core>
#include "MirrorSurface.h"

class MirrorRenderer {
 public:
  MirrorRenderer(
      const std::vector<MirrorSurface>& mirrors,
      const int width,
      const int height,
      const std::string shadir)
      : surfaceOffset(0.0025f) {
    // create render target
    colorTex.Reinitialise(width, height, GL_RGBA8, true, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
    depthTex.Reinitialise(
        width, height, GL_DEPTH_COMPONENT24, true, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);

    frameBuffer.AttachColour(colorTex);
    frameBuffer.Bind();
    glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, depthTex.tid, 0);
    glDrawBuffer(GL_NONE);
    ASSERT(glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE);
    frameBuffer.Unbind();

    // load shader
    ASSERT(pangolin::FileExists(shadir), "Shader directory not found!");
    shader.AddShaderFromFile(pangolin::GlSlVertexShader, shadir + "/mirror.vert", {}, {shadir});
    shader.AddShaderFromFile(pangolin::GlSlFragmentShader, shadir + "/mirror.frag", {}, {shadir});
    shader.Link();

    // create masks
    maskTextures.resize(mirrors.size());
    for (size_t i = 0; i < mirrors.size(); i++) {
      GenerateMaskTexture((MirrorSurface&)mirrors[i], maskTextures[i], 256, 256);
    }
  }

  ~MirrorRenderer() {}

  pangolin::GlTexture& GetMaskTexture(size_t i) {
    return maskTextures[i];
  }

  void Render(
      const MirrorSurface& surface,
      pangolin::GlTexture& maskTexture,
      const pangolin::OpenGlRenderState& cam,
      const bool drawDepth=false
      ) {
    shader.Bind();
    shader.SetUniform("MVP_matrix", cam.GetProjectionModelViewMatrix());
    shader.SetUniform("MV_matrix", cam.GetModelViewMatrix());
    shader.SetUniform("reflectivity", surface.Reflectivity());
    shader.SetUniform("texSize", (float)colorTex.width, (float)colorTex.height);

    glActiveTexture(GL_TEXTURE0);
    colorTex.Bind();

    glActiveTexture(GL_TEXTURE1);
    maskTexture.Bind();

    if (!drawDepth) {
      glEnable(GL_BLEND);
      glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    }

    glDisable(GL_CULL_FACE);

    Draw(surface, surfaceOffset);

    glDisable(GL_BLEND);

    glActiveTexture(GL_TEXTURE1);
    maskTexture.Unbind();
    glActiveTexture(GL_TEXTURE0);
    colorTex.Unbind();

    shader.Unbind();
  }

  void DrawNormal(const MirrorSurface& surface, float len) {
    Eigen::Vector3f p = surface.Boundary_w()[0] + surface.Equation().head<3>() * len;
    glBegin(GL_LINES);
    glVertex3fv((GLfloat*)surface.Boundary_w()[0].data());
    glVertex3fv((GLfloat*)p.data());
    glEnd();
  }

  void CaptureReflection(
      const MirrorSurface& mirror,
      PTexMesh& ptexMesh,
      const pangolin::OpenGlRenderState& cam,
      GLenum frontFace,
      const bool drawDepth=false,
      const float depthScale=1.0f) {
    if (!InView(mirror, cam))
      return;
    // render reflections to texture
    pangolin::OpenGlRenderState reflectCam(
        cam.GetProjectionMatrix(), cam.GetModelViewMatrix() * GetReflectionMatrix(mirror));

    // Check which side of the surface we're on to render the right clip plane
    const Eigen::Vector4d t =
        ((Eigen::Matrix4d)cam.GetModelViewMatrix().Inverse()).topRightCorner(4, 1);
    const double signFlip = mirror.Equation().cast<double>().dot(t) > 0 ? 1.0 : -1.0;

    Eigen::Vector4f plane = mirror.Equation();
    plane(3) -= surfaceOffset;

    BeginDrawScene(frontFace);
    if (drawDepth)
      ptexMesh.RenderDepth(reflectCam, depthScale, signFlip * plane);
    else
      ptexMesh.Render(reflectCam, signFlip * plane);
    EndDrawScene(frontFace);
  }

  void DisplayTexture() {
    glDisable(GL_DEPTH_TEST);
    colorTex.RenderToViewport();
    glEnable(GL_DEPTH_TEST);
  }

 private:
  Eigen::Matrix4f GetReflectionMatrix(const MirrorSurface& surface) {
    Eigen::Vector3f n = surface.Equation().head<3>();
    float d = surface.Equation()(3);
    Eigen::Matrix4f m;
    m << 1 - 2 * n(0) * n(0), -2 * n(0) * n(1), -2 * n(0) * n(2), -2 * n(0) * d, -2 * n(0) * n(1),
        1 - 2 * n(1) * n(1), -2 * n(1) * n(2), -2 * n(1) * d, -2 * n(0) * n(2), -2 * n(1) * n(2),
        1 - 2 * n(2) * n(2), -2 * n(2) * d, 0, 0, 0, 1;
    return m;
  }

  void Draw(const MirrorSurface& surface, const float renderOffset) {
    glNormal3fv((GLfloat*)surface.Equation().data());

    glBegin(GL_QUADS);

    const Eigen::Vector3f normal = surface.Equation().head<3>().normalized();

    Eigen::Vector3f vertex;

    vertex = surface.BoundingRect_w()[0] + renderOffset * normal;
    glTexCoord2f(0.0f, 0.0f);
    glVertex3fv((GLfloat*)vertex.data());

    vertex = surface.BoundingRect_w()[1] + renderOffset * normal;
    glTexCoord2f(1.0f, 0.0f);
    glVertex3fv((GLfloat*)vertex.data());

    vertex = surface.BoundingRect_w()[3] + renderOffset * normal;
    glTexCoord2f(1.0f, 1.0f);
    glVertex3fv((GLfloat*)vertex.data());

    vertex = surface.BoundingRect_w()[2] + renderOffset * normal;
    glTexCoord2f(0.0f, 1.0f);
    glVertex3fv((GLfloat*)vertex.data());

    glEnd();
  }

  void BeginDrawScene(const GLenum frontFace) {
    frameBuffer.Bind();
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glFrontFace(frontFace == GL_CW ? GL_CCW : GL_CW); // reflection reverses facing
    glEnable(GL_CLIP_DISTANCE0);
    glPushAttrib(GL_VIEWPORT_BIT);
    glViewport(0, 0, colorTex.width, colorTex.height);
  }

  void EndDrawScene(const GLenum frontFace) {
    frameBuffer.Unbind();
    glFrontFace(frontFace);
    glDisable(GL_CLIP_DISTANCE0);
    glPopAttrib();
  }

  bool InView(const MirrorSurface& surface, const pangolin::OpenGlRenderState& cam) {
    const Eigen::Matrix4f mvp = ((Eigen::Matrix4d)cam.GetProjectionModelViewMatrix()).cast<float>();

    for (size_t i = 0; i < surface.Boundary_w().size(); i++) {
      Eigen::Vector4f p = mvp * Unproject(surface.Boundary_w()[i]);

      p = p / p(3);

      if (p(0) > -1 && p(0) < 1 && p(1) > -1 && p(1) < 1 && p(2) > -1 && p(2) < 1) {
        return true;
      }
    }

    return false;
  }

  void GenerateMaskTexture(
      MirrorSurface& surface,
      pangolin::GlTexture& tex,
      int width = 256,
      int height = 256) {
    pangolin::ManagedImage<float> image;
    surface.GenerateMask(image, width, height);
    tex.Reinitialise(width, height, GL_LUMINANCE8, true, 0, GL_LUMINANCE, GL_FLOAT, image.ptr);
  }

  const float surfaceOffset;

  pangolin::GlSlProgram shader;

  pangolin::GlFramebuffer frameBuffer;
  pangolin::GlTexture depthTex;
  pangolin::GlTexture colorTex;

  std::vector<pangolin::GlTexture> maskTextures;
};
