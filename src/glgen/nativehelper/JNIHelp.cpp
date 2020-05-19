#include <jni.h>
#include <stdlib.h>

#include "nativehelper/JNIHelp.h"

jint JNICALL register_android_opengl_jni_EGL14(JNIEnv *_env, jclass);
jint JNICALL register_android_opengl_jni_EGL15(JNIEnv *_env, jclass);
jint JNICALL register_android_opengl_jni_GLES20(JNIEnv *_env, jclass);
jint JNICALL register_android_opengl_jni_GLES30(JNIEnv *_env, jclass);
jint JNICALL register_android_opengl_jni_GLES31(JNIEnv *_env, jclass);
jint JNICALL register_android_opengl_jni_GLES32(JNIEnv *_env, jclass);


static jmethodID gAsIntBufferId, gGetBufferFieldsId;
static jclass gJniHelpClass, gNoClassDefFoundError;

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

  gNoClassDefFoundError = (jclass) env->FindClass("java/lang/NoClassDefFoundError");
  if (!gNoClassDefFoundError) {
    return -1;
  }

  JNINativeMethod methods[] = {
    { "registerEGL14", "()I", register_android_opengl_jni_EGL14 },
    { "registerEGL15", "()I", register_android_opengl_jni_EGL15 },
    { "registerGLES20", "()I", register_android_opengl_jni_GLES20 },
    { "registerGLES30", "()I", register_android_opengl_jni_GLES30 },
    { "registerGLES31", "()I", register_android_opengl_jni_GLES31 },
    { "registerGLES32", "()I", register_android_opengl_jni_GLES32 },
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


