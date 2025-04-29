#include <jni.h>
#include <string>

extern "C" JNIEXPORT jstring JNICALL
Java_com_sonryle_mynativecapplication_MainActivity_joeStringFromJNI(
        JNIEnv* env,
        jobject /* this */) {
    std::string hello = "Hello fro C++";
    return env->NewStringUTF(hello.c_str());
}