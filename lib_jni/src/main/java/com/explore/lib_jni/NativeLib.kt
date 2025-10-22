package com.explore.lib_jni

import android.util.Log



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