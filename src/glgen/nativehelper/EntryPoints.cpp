#include <jni.h>

#include "JNIHelp.h"

JNIEXPORT jint JNICALL JNI_OnLoad(JavaVM *vm, void *reserved) {
  JNIEnv* env;
  int err = vm->GetEnv((void**) &env, JNI_VERSION_1_8);
  if (err != 0) {
    return -1;
  }

  err = jniHelpInitialize(env);
  if (err != 0) {
    return err;
  }

  return JNI_VERSION_1_8;
}
