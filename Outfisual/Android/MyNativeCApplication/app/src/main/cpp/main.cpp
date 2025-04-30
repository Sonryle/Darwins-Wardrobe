#include <jni.h>

#include <game-activity/GameActivity.cpp>
#include <game-text-input/gametextinput.cpp>

extern "C"
{
    #include <game-activity/native_app_glue/android_native_app_glue.c>
    #include <EGL/egl.h>
    #include <GLES3/gl3.h>

    bool window_ready = false;
    void handle_cmd(android_app* app, int32_t cmd) {
        if (cmd == APP_CMD_INIT_WINDOW && app->window != nullptr) {
            window_ready = true;
        }
    }

    void android_main(struct android_app *pApp) {
        __android_log_print(ANDROID_LOG_DEBUG, "AO", "%s", "Welcome to C++!");
        pApp->onAppCmd = handle_cmd;
        // Choose your render attributes
        constexpr EGLint attribs[] = {
                EGL_RENDERABLE_TYPE, EGL_OPENGL_ES3_BIT,
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

        // get the list of configurations
        std::unique_ptr<EGLConfig[]> supportedConfigs(new EGLConfig[numConfigs]);
        eglChooseConfig(display, attribs, supportedConfigs.get(), numConfigs, &numConfigs);

        // Find a config we like.
        // Could likely just grab the first if we don't care about anything else in the config.
        // Otherwise hook in your own heuristic
        auto config = *std::find_if(
                supportedConfigs.get(),
                supportedConfigs.get() + numConfigs,
                [&display](const EGLConfig &config) {
                    EGLint red, green, blue, depth;
                    if (eglGetConfigAttrib(display, config, EGL_RED_SIZE, &red)
                        && eglGetConfigAttrib(display, config, EGL_GREEN_SIZE, &green)
                        && eglGetConfigAttrib(display, config, EGL_BLUE_SIZE, &blue)
                        && eglGetConfigAttrib(display, config, EGL_DEPTH_SIZE, &depth)) {
                        return red == 8 && green == 8 && blue == 8 && depth == 24;
                    }
                    return false;
                });

        // create the proper window surface
        EGLint format;
        eglGetConfigAttrib(display, config, EGL_NATIVE_VISUAL_ID, &format);
        while (!window_ready) {
            int events;
            android_poll_source* source;
            if (ALooper_pollOnce(0, nullptr, &events, (void**)&source) >= 0 && source)
                source->process(pApp, source);
            __android_log_print(ANDROID_LOG_DEBUG, "AO", "%s", "Window is not ready");
        }
        EGLSurface surface = eglCreateWindowSurface(display, config, pApp->window, nullptr);

        // Create a GLES 3 context
        EGLint contextAttribs[] = {EGL_CONTEXT_CLIENT_VERSION, 3, EGL_NONE};
        EGLContext context = eglCreateContext(display, config, nullptr, contextAttribs);
        eglMakeCurrent(display, surface, surface, context);

        // setup any other gl related global states
        glClearColor(1.0f, 0.0f, 0.0f, 0.0f);
        do
        {
// Process all pending events before running game logic.
            bool done = false;
            while (!done) {
                // 0 is non-blocking.
                int timeout = 0;
                int events;
                android_poll_source *pSource;
                int result = ALooper_pollOnce(timeout, nullptr, &events,
                                              reinterpret_cast<void**>(&pSource));
                switch (result) {
                    case ALOOPER_POLL_TIMEOUT:
                        [[clang::fallthrough]];
                    case ALOOPER_POLL_WAKE:
                        // No events occurred before the timeout or explicit wake. Stop checking for events.
                        done = true;
                        break;
                    case ALOOPER_EVENT_ERROR:
                        break;
                    case ALOOPER_POLL_CALLBACK:
                        break;
                    default:
                        if (pSource) {
                            pSource->process(pApp, pSource);
                        }
                }
            }

            glClear(GL_COLOR_BUFFER_BIT);
            eglSwapBuffers(display, surface);
            __android_log_print(ANDROID_LOG_DEBUG, "AO", "%s", "Display Cleared!");
        } while (!pApp->destroyRequested);
    }
}