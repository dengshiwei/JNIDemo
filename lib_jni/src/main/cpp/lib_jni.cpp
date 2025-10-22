#include <jni.h>
#include <string>
#include <android/log.h>
#include <thread>
#include <chrono>

#define LOGD(...) __android_log_print(ANDROID_LOG_DEBUG, "JNI_DEMO", __VA_ARGS__)

void callJavaCallback(JNIEnv* env, jobject thiz, const char* methodName, const char* signature, ...) {
    jclass clazz = env->GetObjectClass(thiz);
    if (clazz == nullptr) {
        return;
    }

    jmethodID method = env ->GetMethodID(clazz, methodName, signature);
    if (method == nullptr) {
        LOGD("⚠️ 未找到 Java 方法: %s", methodName);
        return;
    }

    va_list args;
    va_start(args, signature);
    env->CallVoidMethodV(thiz, method, args);
    va_end(args);
}

extern "C"
JNIEXPORT jint JNICALL
Java_com_explore_lib_1jni_NativeLib_startTask(JNIEnv *env, jobject thiz, jstring name) {
    const char* cname = env->GetStringUTFChars(name, nullptr);
    if (cname == nullptr) {
        return -1;
    }

    LOGD("开始任务: %s", cname);

    // 模拟耗时任务（例如网络请求）
    for (int i = 0; i <= 100; i += 25) {
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
        // 调用 Java 的 onProgress(int)
        callJavaCallback(env, thiz, "onProgress", "(I)V", i);
    }

    std::string result = std::string("任务完成: ") + std::string(cname);
    env->ReleaseStringUTFChars(name, cname);
    jstring jResult = env->NewStringUTF(result.c_str());
    jclass  clazz = env->GetObjectClass(thiz);
    jmethodID completedMethod = env->GetMethodID(clazz, "onComplete", "(Ljava/lang/String;)V");
    if (completedMethod == nullptr) {
        LOGD("⚠️ 未找到 Java 方法: onCompleted");
        return -1;
    }
    env->CallVoidMethod(thiz, completedMethod, jResult);
    // 调用 Java 的另一个 native 方法 calculate()
    jmethodID calcMethod = env->GetMethodID(clazz, "calculate", "(II)I");
    if (calcMethod != nullptr) {
        jint calcResult = env->CallIntMethod(thiz, calcMethod, 7, 5);
        LOGD("JNI 调用 Java calculate(7,5) = %d", calcResult);
    }

    LOGD("任务完成 ✅");
    return 0;
}


extern "C"
JNIEXPORT jint JNICALL
Java_com_explore_lib_1jni_NativeLib_calculate(JNIEnv *env, jobject thiz, jint x, jint y) {
    // 简单计算
    jint result = x * y + 10;
    LOGD("calculate(%d, %d) => %d", x, y, result);
    return result;
}