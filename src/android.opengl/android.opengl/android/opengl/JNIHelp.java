package android.opengl;

import java.io.InputStream;
import java.io.IOException;
import java.nio.Buffer;
import java.nio.ByteBuffer;
import java.nio.CharBuffer;
import java.nio.FloatBuffer;
import java.nio.DoubleBuffer;
import java.nio.IntBuffer;
import java.nio.LongBuffer;
import java.nio.ShortBuffer;
import java.nio.file.Files;
import java.nio.file.Path;
import java.nio.file.Paths;
import java.nio.file.StandardCopyOption;
import java.util.concurrent.atomic.AtomicBoolean;
import java.util.regex.Pattern;

public class JNIHelp {
  public static Object getBufferFields(Buffer buffer, int[] result) {
    int elementSizeShift;
    if (buffer instanceof ByteBuffer) {
      elementSizeShift = 0;
    } else if (buffer instanceof FloatBuffer) {
      elementSizeShift = 2;
    } else if (buffer instanceof IntBuffer) {
      elementSizeShift = 2;
    } else if (buffer instanceof LongBuffer) {
      elementSizeShift = 3;
    } else if (buffer instanceof ShortBuffer) {
      elementSizeShift = 1;
    } else if (buffer instanceof DoubleBuffer) {
      elementSizeShift = 3;
    } else if (buffer instanceof CharBuffer) {
      elementSizeShift = 1;
    } else {
      elementSizeShift = 0;
    }

    int position = buffer.position();
    int limit = buffer.limit();
    int remainingBytes = (limit - position) << elementSizeShift;
    result[0] = remainingBytes;

    if (buffer.hasArray()) {
      result[1] = buffer.arrayOffset();
      return buffer.array();
    } else {
      result[1] = position << elementSizeShift;
      return null;
    }
  }

  public static void loadNativeLibraryFromJar(String name) {
    // With only three supported operating systems, the shared library extension is sufficient to distinguish them.
    // Could also examine System.getProperty("os.name").
    String filename = System.mapLibraryName(name);
    boolean canDeleteOpen = true;
    String osName;
    if (filename.endsWith(".dll")) {
      osName = "windows";
      canDeleteOpen = false;
    } else if (filename.endsWith(".so")) {
      osName = "linux";
    } else if (filename.endsWith(".dylib")) {
      osName = "osx";
    } else {
      throw new UnsatisfiedLinkError("JANGLE does not support this operating system");
    }

    String arch = System.getProperty("os.arch").toLowerCase();
    if (Pattern.matches("^(amd64|x64|x86_64)", arch)) {
      arch = "x64";
    } else {
      throw new UnsatisfiedLinkError("JANGLE does not support processor architecture '" + arch + "'");
    }

    String resourcePath = "/android/opengl/" + arch + "-" + osName + "/" + filename;
    InputStream inStream = JNIHelp.class.getResourceAsStream(resourcePath);
    if (inStream == null) {
      throw new UnsatisfiedLinkError("Could not find resource '" + resourcePath + "'");
    }

    try {
      Path tempDir = Files.createTempDirectory(null);
      try {
        Path tempFile = tempDir.resolve(filename);
        Files.copy(inStream, tempFile, StandardCopyOption.REPLACE_EXISTING);
        try {
          System.load(tempFile.toAbsolutePath().toString());
        } finally {
          if (canDeleteOpen) {
            Files.delete(tempFile);
          } else {
            tempFile.toFile().deleteOnExit();
          }
        }
      } finally {
        if (canDeleteOpen) {
          Files.delete(tempDir);
        } else {
          tempDir.toFile().deleteOnExit();
        }
      }
    } catch (IOException e) {
      throw new UnsatisfiedLinkError("IO error copying resource '" + resourcePath + "': " + e.getMessage());
    }
  }

  private static final AtomicBoolean loadedNativeLibrary = new AtomicBoolean(false);
  public static void loadNativeLibrary() {
    synchronized (loadedNativeLibrary) {
      if (loadedNativeLibrary.get()) {
        return;
      }

      String libraryName = "JANGLE";
      try {
        loadNativeLibraryFromJar(libraryName);
      } catch (UnsatisfiedLinkError outer) {
        try {
          System.loadLibrary(libraryName);
        } catch (UnsatisfiedLinkError inner) {
          throw outer;
        }
      }

      loadedNativeLibrary.set(true);
    }
  }
  
  public static native int registerEGL14();
  public static native int registerEGL15();
  public static native int registerGLES20();
  public static native int registerGLES30();
  public static native int registerGLES31();
  public static native int registerGLES32();
}
