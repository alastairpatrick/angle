import java.nio.ByteBuffer;
import java.nio.IntBuffer;

import android.opengl.EGL14;
import android.opengl.EGL15;
import android.opengl.EGLConfig;
import android.opengl.EGLContext;
import android.opengl.EGLDisplay;
import android.opengl.EGLSurface;
import android.opengl.GLES20;

public class JavaSmokeTest {
  public static void main(String[] args) {
    int width = 16;
    int height = 16;

    /*try {
      Thread.sleep(10000);
    } catch (InterruptedException e) {
    }*/

    EGLDisplay display = EGL14.eglGetDisplay(EGL14.EGL_DEFAULT_DISPLAY);

    int[] major = new int[1];
    int[] minor = new int[1];
    if (!EGL14.eglInitialize(display, major, 0, minor, 0)) {
      System.out.format("eglInitialize failed\n");
      System.exit(1);
    }

    System.out.format("Initialized version %d.%d\n", major[0], minor[0]);

    int[] configAttribs = new int[] {
      EGL14.EGL_COLOR_BUFFER_TYPE, EGL14.EGL_RGB_BUFFER,
      EGL14.EGL_RED_SIZE, 8,
      EGL14.EGL_GREEN_SIZE, 8,
      EGL14.EGL_BLUE_SIZE, 8,
      EGL14.EGL_ALPHA_SIZE, 8,
      EGL14.EGL_CONFORMANT, EGL14.EGL_OPENGL_ES2_BIT,
      EGL14.EGL_SURFACE_TYPE, EGL14.EGL_PBUFFER_BIT,
      EGL14.EGL_NONE,
    };

    EGLConfig[] configs = new EGLConfig[1];
    int[] numConfigs = new int[1];
    if (!EGL14.eglChooseConfig(display, configAttribs, 0, configs, 0, 1, numConfigs, 0)) {
      System.out.format("eglChooseConfig failed\n");
      System.exit(1);
    }

    int[] surfaceAttribs = new int[] {
      EGL14.EGL_WIDTH, width,
      EGL14.EGL_HEIGHT, height,
      EGL14.EGL_NONE,
    };

    EGLSurface surface = EGL14.eglCreatePbufferSurface(display, configs[0], surfaceAttribs, 0);
    if (surface == null) {
      System.out.format("eglCreatePbufferSurface failed\n");
      System.exit(1);
    }

    int[] contextAttribs = new int[] {
      EGL15.EGL_CONTEXT_MAJOR_VERSION, 2,
      EGL15.EGL_CONTEXT_MINOR_VERSION, 0,
      EGL14.EGL_NONE,
    };

    EGLContext context = EGL14.eglCreateContext(display, configs[0], null, contextAttribs, 0);
    if (context == null) {
      System.out.format("eglCreateContext failed\n");
      System.exit(1);
    }

    if (!EGL14.eglMakeCurrent(display, surface, surface, context)) {
      System.out.format("eglMakeCurrent failed\n");
      System.exit(1);
    }

    GLES20.glClearColor(1.0f, 0.0f, 1.0f, 0.0f);
    GLES20.glClear(GLES20.GL_COLOR_BUFFER_BIT);

    ByteBuffer pixels = ByteBuffer.allocateDirect(width * height * 4);
    GLES20.glReadPixels(0, 0, width, height, GLES20.GL_RGBA, GLES20.GL_UNSIGNED_BYTE, pixels);

    IntBuffer pixels32 = pixels.asIntBuffer();
    System.out.format("Pixel at 0, 0 = %x\n", pixels32.get(0));

    EGL14.eglDestroyContext(display, context);
    EGL14.eglDestroySurface(display, surface);
    EGL14.eglTerminate(display);
  }
}
