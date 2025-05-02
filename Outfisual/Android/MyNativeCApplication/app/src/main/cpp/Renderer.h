#ifndef ANDROID_OUTFISUAL_RENDERER
#define ANDROID_OUTFISUAL_RENDERER

#include <EGL/egl.h>
#include <GLES2/gl2.h>

struct android_app;

struct Renderer
{
    Renderer(android_app* pApp) :
            app_(pApp),
            display_(EGL_NO_DISPLAY),
            surface_(EGL_NO_SURFACE),
            context_(EGL_NO_CONTEXT),
            width_(-1),
            height_(-1) {
        initRenderer(pApp);
    }

    void render();

private:

    void initRenderer(android_app* pApp);

    android_app *app_;
    EGLDisplay display_;
    EGLSurface surface_;
    EGLContext context_;
    EGLint width_;
    EGLint height_;
};

#endif