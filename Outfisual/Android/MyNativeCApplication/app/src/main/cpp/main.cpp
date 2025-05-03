#include <jni.h>

#include "Renderer.h"
#include "AndroidOut.h"

#include <game-activity/native_app_glue/android_native_app_glue.c>
#include <game-activity/GameActivity.cpp>
#include <game-text-input/gametextinput.cpp>

extern "C"
{
    /*!
     * Callback function for handling commands sent to this Android Application
     * @param pApp the app where the commands are coming from
     * @param cmd the command to handle
     */
    void handle_cmd(android_app *pApp, int32_t cmd)
    {
        switch (cmd)
        {
            case APP_CMD_INIT_WINDOW:
                // When window is initialised, initialise a new Renderer along with it
                pApp->userData = new Renderer(pApp);
                break;
            case APP_CMD_TERM_WINDOW:
                // When window is terminated, delete the Renderer
                if (pApp->userData != nullptr)
                {
                    delete reinterpret_cast<Renderer*>(pApp->userData);
                    pApp->userData = nullptr;
                }
                break;
            default:
                break;
        }
    }

    /*!
     * Processes all pending events from this Android Application
     * @param pApp the app which contains pending events
     */
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
                case ALOOPER_POLL_WAKE:
                    done = true;
                    break;
                case ALOOPER_EVENT_ERROR:
                    aout << "ALooper_pollOnce returned an error" << std::endl;
                    break;
                case ALOOPER_POLL_CALLBACK:
                    break;
                default:
                    if (pSource)
                        pSource->process(pApp, pSource);
            }
        }
    }

    /*!
     * Main entry point of the C++ program for Android
     * @param pApp The app that runs the main loop
     */
    void android_main(struct android_app *pApp)
    {
        // Assign callback functions
        pApp->onAppCmd = handle_cmd;

        // Game loop
        while (!pApp->destroyRequested)
        {
            // Process android events
            process_events(pApp);

            // Render scene
            if(pApp->userData != nullptr)
                reinterpret_cast<Renderer*>(pApp->userData)->render();
        }
    }
}