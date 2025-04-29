#include <jni.h>

#include <android/log.h>

extern "C" {
/*!
 * This the main entry point for a native activity
 */
void android_main(struct android_app *pApp) {
    __android_log_print(ANDROID_LOG_DEBUG, "AO", "Running....");
}
}