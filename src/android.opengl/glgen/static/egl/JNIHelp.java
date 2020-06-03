package android.opengl;

import java.nio.Buffer;
import java.nio.ByteBuffer;
import java.nio.CharBuffer;
import java.nio.FloatBuffer;
import java.nio.DoubleBuffer;
import java.nio.IntBuffer;
import java.nio.LongBuffer;
import java.nio.ShortBuffer;

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

  public static void loadNativeLibrary() {
    System.loadLibrary("JANGLE");
  }
  
  public static native int registerEGL14();
  public static native int registerEGL15();
  public static native int registerGLES20();
  public static native int registerGLES30();
  public static native int registerGLES31();
  public static native int registerGLES32();
}
