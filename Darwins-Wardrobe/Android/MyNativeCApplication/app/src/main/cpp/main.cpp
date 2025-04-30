#include <jni.h>

#include <game-activity/GameActivity.cpp>
#include <game-text-input/gametextinput.cpp>

extern "C" {

#include <game-activity/native_app_glue/android_native_app_glue.c>

void android_main(struct android_app *pApp) {
    __android_log_print(ANDROID_LOG_DEBUG, "AO", "%s", "Heyy");
}
}