#include <gtest/gtest.h>
#include <vector>

// On Linux, EGL/egl.h includes the X11 headers, which in turn define C macros, including one called "None" which conflicts with a struct in gtest.h. So put the EGL/GL includes last.
#include <EGL/egl.h>
#include <GLES2/gl2.h>

TEST(NativeSmokeTest, NativeSmokeTest) {
  const int width = 16;
  const int height = 16;

  EGLDisplay display = eglGetDisplay(EGL_DEFAULT_DISPLAY);
  ASSERT_NE(display, nullptr);

  ASSERT_TRUE(eglInitialize(display, nullptr, nullptr));

  EGLint config_attrib_list[] = {
    EGL_COLOR_BUFFER_TYPE, EGL_RGB_BUFFER,
    EGL_RED_SIZE, 8,
    EGL_GREEN_SIZE, 8,
    EGL_BLUE_SIZE, 8,
    EGL_ALPHA_SIZE, 8,
    EGL_CONFORMANT, EGL_OPENGL_ES2_BIT,
    EGL_SURFACE_TYPE, EGL_PBUFFER_BIT,
    EGL_NONE,
  };
  EGLint num_configs = 0;
  ASSERT_TRUE(eglChooseConfig(display, config_attrib_list, nullptr, 0, &num_configs));
  ASSERT_GT(num_configs, 0);

  std::vector<EGLConfig> configs(num_configs);
  ASSERT_TRUE(eglChooseConfig(display, config_attrib_list, &configs[0], num_configs, &num_configs));

  EGLint pbuffer_attrib_list[] = {
    EGL_WIDTH, width,
    EGL_HEIGHT, height,
    EGL_NONE,
  };
  EGLSurface surface = eglCreatePbufferSurface(display, configs[0], pbuffer_attrib_list);
  EXPECT_NE(surface, nullptr);

  EGLint context_attrib_list[] = {
    EGL_CONTEXT_MAJOR_VERSION, 2,
    EGL_CONTEXT_MINOR_VERSION, 0,
    EGL_NONE,
  };
  EGLContext context = eglCreateContext(display, configs[0], nullptr, context_attrib_list);
  ASSERT_NE(context, nullptr);

  ASSERT_TRUE(eglMakeCurrent(display, surface, surface, context));

  glClearColor(1.0, 0.0, 1.0, 0.0);
  glClear(GL_COLOR_BUFFER_BIT);

  std::vector<uint32_t> pixels(width * height);
  glReadPixels(0, 0, width, height, GL_RGBA, GL_UNSIGNED_BYTE, &pixels[0]);
  
  EXPECT_EQ(GL_NO_ERROR, glGetError());

  for (int i = 0; i < width * height; ++i) {
    ASSERT_EQ(0x00FF00FF, pixels[i]);
  }

  EXPECT_TRUE(eglDestroyContext(display, context));
  EXPECT_TRUE(eglDestroySurface(display, surface));
  EXPECT_TRUE(eglTerminate(display));
}
