import java.awt.Frame;
import java.awt.Graphics;
import java.awt.event.WindowAdapter;
import java.awt.event.WindowEvent;

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

public class WindowTest {
  public static void main(String[] args) {
    System.setProperty("sun.awt.noerasebackground", "true");
    new WindowTest();
  }

  WindowTest() {
    int width = 256;
    int height = 256;

    /*try {
      Thread.sleep(10000);
    } catch (InterruptedException e) {
    }*/

    display = eglGetDisplay(EGL_DEFAULT_DISPLAY);

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

    config = configs[0];

    int[] contextAttribs = new int[] {
      EGL_CONTEXT_MAJOR_VERSION, 2,
      EGL_CONTEXT_MINOR_VERSION, 0,
      EGL_NONE,
    };

    context = eglCreateContext(display, config, EGL14.EGL_NO_CONTEXT, contextAttribs, 0);
    if (context == null) {
      System.out.format("eglCreateContext failed\n");
      System.exit(1);
    }

    frame = new AppFrame();
    frame.setSize(width, height);
    frame.setLayout(null);
    frame.setVisible(true);
  }
  
  class AppFrame extends Frame {
    public AppFrame() {
      this.addWindowListener(new WindowAdapter() {
        public void windowClosing(WindowEvent evt) {
          System.exit(0);
        }
      });
    }

    public void update(Graphics g) {
      if (isShowing()) {
        paint(g);
      }
    }

    public void paint(Graphics g) {
      if (surface == null) {
        surface = eglCreateWindowSurface(display, config, this, null, 0);
        if (surface == null) {
          System.out.format("eglCreatePbufferSurface failed\n");
          System.exit(1);
        }
      }

      if (!eglMakeCurrent(display, surface, surface, context)) {
        System.out.format("eglMakeCurrent failed\n");
        System.exit(1);
      }

      glClearColor(1.0f, 0.0f, 1.0f, 0.0f);
      glClear(GL_COLOR_BUFFER_BIT);
      eglSwapBuffers(display, surface);
    }

    private EGLSurface surface;
  }

  private Frame frame;
  private EGLDisplay display;
  private EGLConfig config;
  private EGLContext context;
}
