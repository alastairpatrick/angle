#include <jni.h>
#include <stdlib.h>
#include <string>

#if !defined(NO_JAWT)
#if defined(WIN32)
#include <windows.h>
#else
#include <dlfcn.h>
#endif  // WIN32
#include <jawt.h>
#include <jawt_md.h>
#endif  // NO_JAWT

#include "nativehelper/JNIHelp.h"

jint JNICALL register_android_opengl_jni_EGL14(JNIEnv *_env, jclass);
jint JNICALL register_android_opengl_jni_EGL15(JNIEnv *_env, jclass);
jint JNICALL register_android_opengl_jni_GLES20(JNIEnv *_env, jclass);
jint JNICALL register_android_opengl_jni_GLES30(JNIEnv *_env, jclass);
jint JNICALL register_android_opengl_jni_GLES31(JNIEnv *_env, jclass);
jint JNICALL register_android_opengl_jni_GLES32(JNIEnv *_env, jclass);


static jmethodID gAsIntBufferId, gGetBufferFieldsId, gGetPropertyId;
static jclass gJniHelpClass, gNoClassDefFoundError, gSystemClass;

int jniHelpInitialize(JNIEnv* env) {
  jclass byteBufferClass = env->FindClass("java/nio/ByteBuffer");
  if (!byteBufferClass) {
    return -1;
  }

  gJniHelpClass = (jclass) env->NewGlobalRef(env->FindClass("android/opengl/JNIHelp"));
  if (!gJniHelpClass) {
    return -1;
  }

  gGetBufferFieldsId = env->GetStaticMethodID(gJniHelpClass, "getBufferFields", "(Ljava/nio/Buffer;[I)Ljava/lang/Object;");
  if (!gGetBufferFieldsId) {
    return -1;
  }

  gNoClassDefFoundError = (jclass) env->NewGlobalRef(env->FindClass("java/lang/NoClassDefFoundError"));
  if (!gNoClassDefFoundError) {
    return -1;
  }

  gSystemClass = (jclass) env->NewGlobalRef(env->FindClass("java/lang/System"));
  if (!gSystemClass) {
    return -1;
  }

  gGetPropertyId = env->GetStaticMethodID(gSystemClass, "getProperty", "(Ljava/lang/String;)Ljava/lang/String;");
  if (!gGetPropertyId) {
    return -1;
  }

  JNINativeMethod methods[] = {
    { "registerEGL14", "()I", (void*) register_android_opengl_jni_EGL14 },
    { "registerEGL15", "()I", (void*) register_android_opengl_jni_EGL15 },
    { "registerGLES20", "()I", (void*) register_android_opengl_jni_GLES20 },
    { "registerGLES30", "()I", (void*) register_android_opengl_jni_GLES30 },
    { "registerGLES31", "()I", (void*) register_android_opengl_jni_GLES31 },
    { "registerGLES32", "()I", (void*) register_android_opengl_jni_GLES32 },
  };
  return env->RegisterNatives(gJniHelpClass, methods, NELEM(methods));
}

 int jniThrowException(JNIEnv* env, const char* className, const char* msg) {
  env->ExceptionClear();

  jclass exClass;
  exClass = env->FindClass(className);
  if (exClass == NULL) {
    msg = className;
    exClass = env->FindClass("java/lang/NoClassDefFoundError");
    if (exClass == NULL) {
        abort();
    }
  }

  return env->ThrowNew(exClass, msg);
}


void *jniGetBufferPointer(JNIEnv *env, jobject buffer, jarray *array, jint *remaining, jint *offset) {
  if (!buffer) {
    return nullptr;
  }

  jbyte* pointer = nullptr;
  static thread_local jintArray outputArray;

  if (!outputArray) {
    outputArray = env->NewIntArray(2);
    outputArray = (jintArray) env->NewGlobalRef(outputArray);
    if (env->ExceptionOccurred()) {
      abort();
    }
  }

  *array = (jarray) env->CallStaticObjectMethod(gJniHelpClass, gGetBufferFieldsId, buffer, outputArray);
  jint* outputInts = (jint*) env->GetPrimitiveArrayCritical(outputArray, nullptr);
  *remaining = outputInts[0];
  *offset = outputInts[1];
  env->ReleasePrimitiveArrayCritical(outputArray, outputInts, JNI_ABORT);

  if (!*array) {
    pointer = (jbyte*) env->GetDirectBufferAddress(buffer);
    if (pointer) {
      pointer += *offset;
    }
  }

  return pointer;
}


void *jniGetDirectBufferPointer(JNIEnv *env, jobject buffer) {
  if (!buffer) {
      return nullptr;
  }

  jarray array;
  jint remaining;
  jint offset;
  void* pointer = jniGetBufferPointer(env, buffer, &array, &remaining, &offset);
  if (!pointer) {
    jniThrowException(env, "java/lang/IllegalArgumentException",
                      "Must use a native order direct Buffer");
  }

  return pointer;
}


std::string jniGetSystemProperty(JNIEnv* env, const char* name) {
  std::string result;
  jstring nameString = env->NewStringUTF(name);
  if (name) {
    jstring value = (jstring) env->CallStaticObjectMethod(gSystemClass, gGetPropertyId, nameString);
    if (value) {
      const char* ptr = env->GetStringUTFChars(value, nullptr);
      if (ptr) {
        result = ptr;
        env->ReleaseStringUTFChars(value, ptr);
      }
    }
  }
  return result;
}


void* jniGetWindowHandle(JNIEnv* env, jobject windowObject) {
#if defined(NO_JAWT)
  return nullptr;
#else
  typedef jboolean (JNICALL *JAWT_GetAWTFunc)(JNIEnv*,JAWT*);;
  JAWT_GetAWTFunc pJAWT_GetAWT;

#ifdef WIN32
  std::string javaHomeDir = jniGetSystemProperty(env, "java.home");
  std::string modulePath = javaHomeDir + "\\bin\\jawt.dll";

  HMODULE jawtModule = LoadLibraryEx(modulePath.c_str(), nullptr, LOAD_WITH_ALTERED_SEARCH_PATH);
  if (jawtModule == nullptr) {
    jniThrowException(env, "java/lang/UnsatisfiedLinkError", "Failed to load jawt.dll");
    return nullptr;
  }

#if defined(_WIN64)
  const char* functionName = "JAWT_GetAWT";
#else
  const char* functionName = "_JAWT_GetAWT@8";
#endif
  pJAWT_GetAWT = (JAWT_GetAWTFunc) GetProcAddress(jawtModule, functionName);

#else
  void* jawtModule = dlopen("libjawt.so", RTLD_LAZY | RTLD_GLOBAL);
  if (jawtModule == nullptr) {
    jniThrowException(env, "java/lang/UnsatisfiedLinkError", "Failed to load libjawt.so");
    return nullptr;
  }

  pJAWT_GetAWT = (JAWT_GetAWTFunc) dlsym(jawtModule, "JAWT_GetAWT");
#endif

  if (pJAWT_GetAWT == nullptr) {
    jniThrowException(env, "java/lang/UnsatisfiedLinkError", "Failed to import JAWT_GetAWT");
    return nullptr;
  }

  JAWT awt = {};
  awt.version = JAWT_VERSION_1_7;

  if (!pJAWT_GetAWT(env, &awt)) {
    jniThrowException(env, "java/lang/UnsatisfiedLinkError", "Can't load JAWT");
    return nullptr;
  }

  void* windowHandle = nullptr;

  JAWT_DrawingSurface* ds = awt.GetDrawingSurface(env, windowObject);
  if (ds == nullptr) {
    jniThrowException(env, "java/lang/Error", "Failed to get drawing surface");
  } else {
    jint lock = ds->Lock(ds);
    if ((lock & JAWT_LOCK_ERROR) != 0) {
      jniThrowException(env, "java/lang/Error", "Failed to get drawing surface lock");
    } else {
      JAWT_DrawingSurfaceInfo* dsi = ds->GetDrawingSurfaceInfo(ds);
      if (dsi == nullptr) {
        jniThrowException(env, "java/lang/Error", "Failed to get drawing surface info");
      } else {

#if defined(WIN32)
        JAWT_Win32DrawingSurfaceInfo* wdsi = (JAWT_Win32DrawingSurfaceInfo*)dsi->platformInfo;
        if (wdsi == nullptr) {
          jniThrowException(env, "java/lang/Error", "Failed to get Win32 platform info");
        } else {
          windowHandle = (void*) wdsi->hwnd;
        }
#elif (__APPLE__)
        JAWT_MacOSXDrawingSurfaceInfo* mdsi = (JAWT_MacOSXDrawingSurfaceInfo*)dsi->platformInfo;
        if (mdsi == nullptr) {
          jniThrowException(env, "java/lang/Error", "Failed to get OSX platform info");
        } else {
          // TODO: something useful
        }
#else
        JAWT_X11DrawingSurfaceInfo* xdsi = (JAWT_X11DrawingSurfaceInfo*)dsi->platformInfo;
        if (xdsi == nullptr) {
          jniThrowException(env, "java/lang/Error", "Failed to get X11 platform info");
        } else {
          windowHandle = (void*) xdsi->drawable;
        }
#endif

        ds->FreeDrawingSurfaceInfo(dsi);
      }
      ds->Unlock(ds);
    }
    awt.FreeDrawingSurface(ds);
  }

  return windowHandle;
#endif  // NO_JAWT
}
