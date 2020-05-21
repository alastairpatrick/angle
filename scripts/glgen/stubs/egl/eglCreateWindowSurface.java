    // C function EGLSurface eglCreateWindowSurface ( EGLDisplay dpy, EGLConfig config, EGLNativeWindowType win, const EGLint *attrib_list )

    private static native EGLSurface _eglCreateWindowSurface(
        EGLDisplay dpy,
        EGLConfig config,
        Object component,
        int[] attrib_list,
        int offset
    );

    public static EGLSurface eglCreateWindowSurface(EGLDisplay dpy,
        EGLConfig config,
        Object win,
        int[] attrib_list,
        int offset
    ){
        Component component = (Component) win;
        if (component.isLightweight()) {
            throw new IllegalArgumentException("Component must be heavyweight");
        }
        if (!component.isDisplayable()) {
            throw new IllegalStateException("Component must be displayable");
        }

        return _eglCreateWindowSurface(dpy, config, win, attrib_list, offset);
    }
