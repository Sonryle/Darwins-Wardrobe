#include "Renderer.h"

#include <game-activity/native_app_glue/android_native_app_glue.h>
#include <GLES3/gl3.h>

void Renderer::initRenderer(android_app* pApp)
{
    // Choose your render attributes
    constexpr EGLint attribs[] = {
            EGL_RENDERABLE_TYPE, EGL_OPENGL_ES2_BIT,
            EGL_SURFACE_TYPE, EGL_WINDOW_BIT,
            EGL_BLUE_SIZE, 8,
            EGL_GREEN_SIZE, 8,
            EGL_RED_SIZE, 8,
            EGL_DEPTH_SIZE, 24,
            EGL_NONE
    };

    // The default display is probably what you want on Android
    auto display = eglGetDisplay(EGL_DEFAULT_DISPLAY);
    eglInitialize(display, nullptr, nullptr);

    // figure out how many configs there are
    EGLint numConfigs;
    eglChooseConfig(display, attribs, nullptr, 0, &numConfigs);

    // choose a config
    auto supportedConfigs = new EGLConfig[numConfigs];
    eglChooseConfig(display, attribs, supportedConfigs, numConfigs, &numConfigs);
    delete[] supportedConfigs;

    // Find a config we like.
    // Could likely just grab the first if we don't care about anything else in the config.
    // Otherwise hook in your own heuristic
    EGLConfig config = nullptr;
    for (int i = 0; i < numConfigs; ++i) {
        EGLint r, g, b, d;
        if (eglGetConfigAttrib(display, supportedConfigs[i], EGL_RED_SIZE, &r) &&
            eglGetConfigAttrib(display, supportedConfigs[i], EGL_GREEN_SIZE, &g) &&
            eglGetConfigAttrib(display, supportedConfigs[i], EGL_BLUE_SIZE, &b) &&
            eglGetConfigAttrib(display, supportedConfigs[i], EGL_DEPTH_SIZE, &d)) {
            if (r == 8 && g == 8 && b == 8 && d == 24) {
                config = supportedConfigs[i];
                break;
            }
        }
    }

    // create the proper window surface
    EGLint format;
    eglGetConfigAttrib(display, config, EGL_NATIVE_VISUAL_ID, &format);
    EGLSurface surface = eglCreateWindowSurface(display, config, pApp->window, nullptr);

    // Create a GLES 3 context
    EGLint contextAttribs[] = {EGL_CONTEXT_CLIENT_VERSION, 2, EGL_NONE};
    EGLContext context = eglCreateContext(display, config, nullptr, contextAttribs);
    eglMakeCurrent(display, surface, surface, context);

    // Copy display and surface into renderer
    display_ = display;
    surface_ = surface;
    context_ = context;

    // setup any other gl related global states
    glClearColor(1.0f, 0.3f, 0.3f, 0.0f);
}

void Renderer::render()
{
    glClear(GL_COLOR_BUFFER_BIT);
    eglSwapBuffers(display_, surface_);
}