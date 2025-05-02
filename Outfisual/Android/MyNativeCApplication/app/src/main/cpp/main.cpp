#include <jni.h>

#include <game-activity/GameActivity.cpp>
#include <game-text-input/gametextinput.cpp>

extern "C"
{
    #include <game-activity/native_app_glue/android_native_app_glue.c>
    #include <EGL/egl.h>
    #include <GLES2/gl2.h>
    #include "Renderer.h"

    void handle_cmd(android_app *pApp, int32_t cmd) {
        switch (cmd) {
            case APP_CMD_INIT_WINDOW:
                // A new window is created, associate a renderer with it. You may replace this with a
                // "game" class if that suits your needs. Remember to change all instances of userData
                // if you change the class here as a reinterpret_cast is dangerous this in the
                // android_main function and the APP_CMD_TERM_WINDOW handler case.
                pApp->userData = new Renderer(pApp);
                break;
            case APP_CMD_TERM_WINDOW:
                // The window is being destroyed. Use this to clean up your userData to avoid leaking
                // resources.
                //
                // We have to check if userData is assigned just in case this comes in really quickly
                if (pApp->userData) {
                    //
                    auto *pRenderer = reinterpret_cast<Renderer *>(pApp->userData);
                    pApp->userData = nullptr;
                    delete pRenderer;
                }
                break;
            default:
                break;
        }
    }

    void android_main(struct android_app *pApp) {
        __android_log_print(ANDROID_LOG_DEBUG, "AO", "%s", "Welcome to C++!");
        pApp->onAppCmd = handle_cmd;

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

            if(pApp->userData != nullptr)
            {
                auto *pRenderer = reinterpret_cast<Renderer *>(pApp->userData);
                pRenderer->render();
                __android_log_print(ANDROID_LOG_DEBUG, "AO", "%s", "Display Cleared!");
            }
        } while (!pApp->destroyRequested);
    }
}