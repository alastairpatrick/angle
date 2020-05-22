import java.nio.ByteBuffer;
import java.nio.ByteOrder;
import java.nio.IntBuffer;

import android.opengl.EGL14;
import android.opengl.EGLConfig;
import android.opengl.EGLContext;
import android.opengl.EGLDisplay;
import android.opengl.EGLSurface;

import static android.opengl.EGL14.*;
import static android.opengl.EGL15.*;
import static android.opengl.GLES20.*;

public class JavaSmokeTest {
  public static void main(String[] args) {
    int width = 16;
    int height = 16;

    /*try {
      Thread.sleep(10000);
    } catch (InterruptedException e) {
    }*/

    EGLDisplay display = eglGetDisplay(EGL_DEFAULT_DISPLAY);

    if (!eglInitialize(display, null, 0, null, 0)) {
      System.out.format("eglInitialize failed\n");
      System.exit(1);
    }

    int[] configAttribs = new int[] {
      EGL_COLOR_BUFFER_TYPE, EGL_RGB_BUFFER,
      EGL_RED_SIZE, 8,
      EGL_GREEN_SIZE, 8,
      EGL_BLUE_SIZE, 8,
      EGL_ALPHA_SIZE, 8,
      EGL_CONFORMANT, EGL_OPENGL_ES2_BIT,
      EGL_SURFACE_TYPE, EGL_PBUFFER_BIT,
      EGL_NONE,
    };

    EGLConfig[] configs = new EGLConfig[1];
    int[] numConfigs = new int[1];
    if (!eglChooseConfig(display, configAttribs, 0, configs, 0, 1, numConfigs, 0)) {
      System.out.format("eglChooseConfig failed\n");
      System.exit(1);
    }

    if (numConfigs[0] < 1) {
      System.out.format("No EGL configs");
      System.exit(1);
    }

    int[] surfaceAttribs = new int[] {
      EGL_WIDTH, width,
      EGL_HEIGHT, height,
      EGL_NONE,
    };

    EGLSurface surface = eglCreatePbufferSurface(display, configs[0], surfaceAttribs, 0);
    if (surface == null) {
      System.out.format("eglCreatePbufferSurface failed\n");
      System.exit(1);
    }

    int[] contextAttribs = new int[] {
      EGL_CONTEXT_MAJOR_VERSION, 2,
      EGL_CONTEXT_MINOR_VERSION, 0,
      EGL_NONE,
    };

    EGLContext context = eglCreateContext(display, configs[0], EGL14.EGL_NO_CONTEXT, contextAttribs, 0);
    if (context == null) {
      System.out.format("eglCreateContext failed\n");
      System.exit(1);
    }

    if (!eglMakeCurrent(display, surface, surface, context)) {
      System.out.format("eglMakeCurrent failed\n");
      System.exit(1);
    }

    glClearColor(1.0f, 0.0f, 1.0f, 0.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    ByteBuffer pixels = ByteBuffer.allocateDirect(width * height * 4);
    pixels.order(ByteOrder.nativeOrder());
    glReadPixels(0, 0, width, height, GL_RGBA, GL_UNSIGNED_BYTE, pixels);

    IntBuffer pixels32 = pixels.asIntBuffer();
    for (int i = 0; i < width * height; ++i) {
      int color = pixels32.get(i);
      if (color != 0x00FF00FF) {
        System.out.format("Pixel at %d was not expected color %x\n", i, color);
        System.exit(1);
      }
    }

    eglDestroyContext(display, context);
    eglDestroySurface(display, surface);
    eglTerminate(display);

    System.out.format("Test passed\n");
  }
}
