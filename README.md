<h3 id="Nxub8">概述</h3>
JNI（Java Native Interface）是 Java 提供的一种接口机制，允许 Java 代码调用 C/C++ 的本地代码（Native Code），也能让 C/C++ 代码反过来调用 Java 方法。

在 Android 中，JNI 通常用于以下场景：

| 使用场景 | 示例 |
| --- | --- |
| 性能优化 | 图片解码、音视频处理、加密算法 |
| 复用已有 C/C++ 库 | 比如使用 FFmpeg、OpenCV、Opus 等 |
| 安全性要求 | 关键逻辑放在 native 层，防止被反编译 |
| 底层接口调用 | 与硬件或系统底层接口交互 |


<h3 id="zUCQj">JNI 的基本结构</h3>
JNI 的整体流程如下：

```plain
Java 层调用 → JNI 桥接 → C/C++ 实现 → 返回结果给 Java
```

在 Android 中一般步骤如下：

1. 在 Java 层声明 `native` 方法
2. 用 `javac` 和 `javah`（或 Android Studio 自动生成）生成对应的 JNI 函数签名
3. 在 C/C++ 中实现函数逻辑
4. 在 Java 代码中加载 `.so` 动态库并调用

<h4 id="y6Q8W">2.1 <font style="color:rgb(0, 29, 53);">JNI通信的原理过程</font></h4>
<img width="508" height="345" alt="image" src="https://github.com/user-attachments/assets/e43e0e66-beaa-4e81-98de-3dc38ea1c680" />

1. 加载原生库: Java应用程序在需要调用C/C++代码时，会使用来加载一个动态链接库。
2. JNI_OnLoad：在加载完成后，JNI会调用库中的 JNI_OnLoad函数，用于注册JNI函数，建立Java与原生代码之间的映射关系。
3. 方法调用:
    - Java调用C/C++: 当Java代码执行一个声明了关键字的方法时，JVM会通过JNI查找并调用对应原生库中的同名函数。例如，在Java中调用时，会查找函数（函数名格式为Java_+ 全限定类名 + 方法名）。
    - C/C++调用Java: 在原生代码中，也可以通过JNI接口找到并调用已有的Java类和方法，实现反向调用。
4. 数据转换: 在两个代码环境之间传递数据时，JNI会负责将Java的数据类型（如转换为C/C++的数据类型，反之亦然。例如，Java的会转换为C/C++的，而C/C++的也会被转换为Java的
5. 内存管理: JNI接口提供了一系列函数，用于在C/C++中操作Java对象和数据，包括创建和释放内存。
6. 执行结束: 原生代码执行完毕后，JNI接口将结果返回给JVM，继续执行Java代码。 

<h3 id="sY6Lb">基础方法介绍</h3>
NI 的核心是一个名为 `**JNIEnv**` 的指针（实际上是一个函数表），  
它包含了 **操作 Java 对象、调用 Java 方法、访问数组、字符串等功能的 API**。

我们按照常用功能分类来介绍。

---

<h4 id="Mvgon">字符串相关 API</h4>
1️⃣ 获取字符串内容

```plain
const char* str = env->GetStringUTFChars(jstringObj, nullptr);
```

将 Java 字符串 (`jstring`) 转换成 C 字符串 (`const char*`)。

⚠️ 用完后必须释放：

```plain
env->ReleaseStringUTFChars(jstringObj, str);
```

2️⃣ 创建新的 Java 字符串

```plain
jstring newStr = env->NewStringUTF("Hello from C++");
```

---

<h4 id="XK0QC">数组操作</h4>
1️⃣ 获取数组长度

```plain
jsize len = env->GetArrayLength(jintArrayObj);
```

2️⃣ 获取数组元素

```plain
jint* arr = env->GetIntArrayElements(jintArrayObj, nullptr);
```

3️⃣ 修改完成后释放

```plain
env->ReleaseIntArrayElements(jintArrayObj, arr, 0);
```

4️⃣ 创建新的数组

```plain
jintArray newArr = env->NewIntArray(5);
env->SetIntArrayRegion(newArr, 0, 5, arr);
```

---

<h4 id="x2Oj1">对象与类操作</h4>
1️⃣ 获取类引用

```plain
jclass cls = env->GetObjectClass(thiz);
```

2️⃣ 获取字段 ID

```plain
jfieldID fid = env->GetFieldID(cls, "count", "I"); // int 类型字段
```

3️⃣ 访问字段

```plain
jint count = env->GetIntField(thiz, fid);
env->SetIntField(thiz, fid, count + 1);
```

---

<h4 id="uyV2H">方法调用</h4>
1️⃣ 获取方法 ID

```plain
jmethodID mid = env->GetMethodID(cls, "onResult", "(Ljava/lang/String;)V");
```

这里的签名格式 `"()V"` 表示：无参数、无返回值。  
更多类型签名见下表 👇

| Java 类型 | 签名表示 |
| --- | --- |
| `void` | `V` |
| `int` | `I` |
| `boolean` | `Z` |
| `long` | `J` |
| `String` | `Ljava/lang/String;` |
| `int[]` | `[I` |


---

2️⃣ 调用 Java 方法

```plain
jstring msg = env->NewStringUTF("JNI 回调成功！");
env->CallVoidMethod(thiz, mid, msg);
```

3️⃣ 调用静态方法

```plain
jmethodID midStatic = env->GetStaticMethodID(cls, "printLog", "(I)V");
env->CallStaticVoidMethod(cls, midStatic, 123);
```

---

<h4 id="Nt2MQ">异常与引用管理</h4>
1️⃣ 抛出异常

```plain
jclass exc = env->FindClass("java/lang/Exception");
env->ThrowNew(exc, "JNI 调用异常");
```

2️⃣ 删除局部引用

JNI 中的所有对象引用都是局部的（自动回收），但在循环中要手动清理：

```plain
env->DeleteLocalRef(localObj);
```

---

<h4 id="bTrs9">JNI 类型映射总结</h4>

| Java 类型 | JNI 类型 | C/C++ 类型 |
| --- | --- | --- |
| `int` | `jint` | `int` |
| `long` | `jlong` | `long long` |
| `float` | `jfloat` | `float` |
| `boolean` | `jboolean` | `unsigned char` |
| `String` | `jstring` | 通过 `GetStringUTFChars`<br/> 转换 |
| `int[]` | `jintArray` | 通过 `GetIntArrayElements`<br/> 转换 |
| `Object` | `jobject` | Java 对象句柄 |


---

<h3 id="G2Cwz">示例整合</h3>
<h4 id="ZY0Z7">创建 native 方法</h4>

```java
class NativeLib {

    companion object {
        // Used to load the 'lib_jni' library on application startup.
        init {
            System.loadLibrary("lib_jni")
        }
    }

    external fun startTask(name: String): Int

    /**
     * 被 JNI 调用的回调方法
     */
    fun onProgress(percent: Int) {
        Log.d("NativeLib", "Progress: " + percent + "%")
    }

    /**
     * 被 JNI 调用的完成回调
     */
    fun onComplete(result: String?) {
        Log.d("NativeLib", "Completed with result: " + result)
    }

    /**
     * 提供给 C++ 内部再调用的另一个 native 方法
     */
    external fun calculate(x: Int, y: Int): Int
}
```

<h4 id="KtWDF">jni 实现</h4>

```plain
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
```

<h4 id="Y1M0T">应用端调用</h4>

```plain
@Composable
fun Greeting(name: String, modifier: Modifier = Modifier) {
    Text(
        text = "Hello $name!",
        modifier = modifier.clickable {
            // 点击事件
            NativeLib().startTask("点击任务")
        }
    )
}
```

---

<h3 id="qkn9W">总结</h3>

| 功能 | 对应 API |
| --- | --- |
| 字符串操作 | `GetStringUTFChars`<br/> / `NewStringUTF` |
| 数组访问 | `GetArrayLength`<br/> / `GetIntArrayElements` |
| 字段访问 | `GetFieldID`<br/> / `SetIntField` |
| 方法调用 | `GetMethodID`<br/> / `CallVoidMethod` |
| 异常处理 | `ThrowNew` |
| 引用管理 | `DeleteLocalRef` |


