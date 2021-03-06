/* EGLSurface eglCreateWindowSurface ( EGLDisplay dpy, EGLConfig config, EGLNativeWindowType win, const EGLint *attrib_list ) */
static jobject
android_eglCreateWindowSurface
  (JNIEnv *_env, jobject _this, jobject dpy, jobject config, jobject win, jintArray attrib_list_ref, jint offset) {
    jint _exception = 0;
    const char * _exceptionType = "";
    const char * _exceptionMessage = "";
    EGLSurface _returnValue = (EGLSurface) 0;
    EGLDisplay dpy_native = (EGLDisplay) fromEGLHandle(_env, egldisplayGetHandleID, dpy);
    EGLConfig config_native = (EGLConfig) fromEGLHandle(_env, eglconfigGetHandleID, config);
    int attrib_list_sentinel = 0;
    EGLint *attrib_list_base = (EGLint *) 0;
    jint _remaining;
    EGLint *attrib_list = (EGLint *) 0;
    void* windowHandle = nullptr;

    if (attrib_list_ref) {
        if (offset < 0) {
            _exception = 1;
            _exceptionType = "java/lang/IllegalArgumentException";
            _exceptionMessage = "offset < 0";
            goto exit;
        }
        _remaining = _env->GetArrayLength(attrib_list_ref) - offset;
        attrib_list_base = (EGLint *)
            _env->GetIntArrayElements(attrib_list_ref, (jboolean *)0);
        attrib_list = attrib_list_base + offset;
        attrib_list_sentinel = 0;
        for (int i = _remaining - 1; i >= 0; i--)  {
            if (*((EGLint*)(attrib_list + i)) == EGL_NONE){
                attrib_list_sentinel = 1;
                break;
            }
        }
        if (attrib_list_sentinel == 0) {
            _exception = 1;
            _exceptionType = "java/lang/IllegalArgumentException";
            _exceptionMessage = "attrib_list must contain EGL_NONE!";
            goto exit;
        }
    }

    windowHandle = jniGetWindowHandle(_env, win);
    if (windowHandle) {
        _returnValue = eglCreateWindowSurface(
            (EGLDisplay)dpy_native,
            (EGLConfig)config_native,
            (EGLNativeWindowType)windowHandle,
            (EGLint *)attrib_list
        );
    }

exit:
    if (attrib_list_base) {
        _env->ReleaseIntArrayElements(attrib_list_ref, (jint*) attrib_list_base,
            JNI_ABORT);
    }
    if (_exception) {
        jniThrowException(_env, _exceptionType, _exceptionMessage);
        return nullptr;
    }
    return toEGLHandle(_env, eglsurfaceClass, eglsurfaceConstructor, _returnValue);
}
