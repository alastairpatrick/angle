#pragma once
#include <jni.h>

#ifndef NELEM
#define NELEM(x) ((int) (sizeof(x) / sizeof((x)[0])))
#endif

int jniHelpInitialize(JNIEnv* env);
int jniThrowException(JNIEnv* env, const char* className, const char* msg);
void *jniGetBufferPointer(JNIEnv *env, jobject buffer, jarray *array, jint *remaining, jint *offset);
void *jniGetDirectBufferPointer(JNIEnv *env, jobject buffer);
void* jniGetWindowHandle(JNIEnv *env, jobject window);
