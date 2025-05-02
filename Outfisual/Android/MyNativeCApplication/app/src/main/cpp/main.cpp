#include <jni.h>

#include <game-activity/GameActivity.cpp>
#include <game-text-input/gametextinput.cpp>

extern "C"
{
    #include <game-activity/native_app_glue/android_native_app_glue.c>
    #include <EGL/egl.h>
    #include <GLES2/gl2.h>
    #include "Renderer.h"

    // Callback function for android commands
    void handle_cmd(android_app *pApp, int32_t cmd) {
        switch (cmd)
        {
            case APP_CMD_INIT_WINDOW:
                pApp->userData = new Renderer(pApp);
                break;
            case APP_CMD_TERM_WINDOW:
                if (pApp->userData)
                {
                    auto *pRenderer = reinterpret_cast<Renderer *>(pApp->userData);
                    pApp->userData = nullptr;
                    delete pRenderer;
                }
                break;
            default:
                break;
        }
    }

    // Process all pending events
    void process_events(struct android_app *pApp)
    {
        bool done = false;
        while (!done) {
            int events;
            android_poll_source *pSource;
            int result = ALooper_pollOnce(0, nullptr, &events,
                                          reinterpret_cast<void**>(&pSource));
            switch (result)
            {
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
                    if (pSource)
                        pSource->process(pApp, pSource);
            }
        }
    }

    // main entry point
    void android_main(struct android_app *pApp) {
        pApp->onAppCmd = handle_cmd;

        // game loop
        while (!pApp->destroyRequested)
        {
            // process android events
            process_events(pApp);

            // render scene
            if(pApp->userData != nullptr)
                reinterpret_cast<Renderer*>(pApp->userData)->render();
        }
    }
}