#include <assert.h>
#include <stdlib.h>
#include <vector>
#include <iterator>
#include <algorithm>
#include <jni.h>
#include <android/log.h>
#include "sonic/Builder.h"


#define TAG "wavelink"

#define LOGV(...)   __android_log_print(ANDROID_LOG_VERBOSE, TAG, __VA_ARGS__)
#define LOGD(...)   __android_log_print(ANDROID_LOG_DEBUG, TAG, __VA_ARGS__)
#define LOGI(...)   __android_log_print(ANDROID_LOG_INFO, TAG, __VA_ARGS__)
#define LOGW(...)   __android_log_print(ANDROID_LOG_WARN, TAG, __VA_ARGS__)
#define LOGE(...)   __android_log_print(ANDROID_LOG_ERROR, TAG, __VA_ARGS__)


template <class InIt, class OutIt, class T>
inline InIt split(InIt first, InIt last, T const& x, OutIt out)
{
    for (InIt it = first; it != last; ++it)
    {
        if (*it == x)
        {
            return ++it;
        }
        else
        {
            *out++ = *it;
        }
    }

    return last;
}

template <class OutIt>
inline OutIt make_wifi_packet(const char* ip, uint16_t port, char const* ssid, char const* password, const char* mode, OutIt out)
{
    LOGV("make_wifi_packet __begin");

    char const* ptr = ip;
    char const* last = ip + strlen(ip);

    // header
    *out++ = 0xA1;
    //LOGD("%s(%d) tracepoint", __FILE__, __LINE__);

    // ip addr
    std::string ipstr;
    ptr = split(ptr, last, '.', std::back_inserter(ipstr));
    *out++ = atoi(ipstr.c_str());

    ipstr.clear();
    ptr = split(ptr, last, '.', std::back_inserter(ipstr));
    *out++ = atoi(ipstr.c_str());

    ipstr.clear();
    ptr = split(ptr, last, '.', std::back_inserter(ipstr));
    *out++ = atoi(ipstr.c_str());

    ipstr.clear();
    ptr = split(ptr, last, '.', std::back_inserter(ipstr));
    *out++ = atoi(ipstr.c_str());
    //LOGD("%s(%d) tracepoint", __FILE__, __LINE__);

    // ip port
    char* pport = reinterpret_cast<char*>(&port);
    out = std::copy(pport, pport + sizeof(port), out);
    //LOGD("%s(%d) tracepoint", __FILE__, __LINE__);

    // ssid
    out = std::copy(ssid, ssid + strlen(ssid), out);
    *out++ = '\n';
    //LOGD("%s(%d) tracepoint", __FILE__, __LINE__);

    // password
    out = std::copy(password, password + strlen(password), out);
    *out++ = '\n';
    //LOGD("%s(%d) tracepoint", __FILE__, __LINE__);

    // mode
    out = std::copy(mode, mode + strlen(mode), out);

    LOGV("make_wifi_packet __end");
    return out;
}

/*
 * Class:     com_arcsoft_wavelink_WaveBuilder
 * Method:    nativeCreate
 * Signature: ()J
 */
extern "C" jlong JNICALL Java_com_arcsoft_wavelink_WaveBuilder_nativeCreate
  (JNIEnv* env, jclass)
{
    Sonic::CBuilder* builder = new Sonic::CBuilder();
    return (jlong)builder;
}

/*
 * Class:     com_arcsoft_wavelink_WaveBuilder
 * Method:    nativeDestroy
 * Signature: (J)V
 */
extern "C" void JNICALL Java_com_arcsoft_wavelink_WaveBuilder_nativeDestroy
  (JNIEnv* env, jclass, jlong handle)
{
    Sonic::CBuilder* builder = (Sonic::CBuilder*)handle;
    delete builder;
}

/*
 * Class:     com_arcsoft_wavelink_WaveBuilder
 * Method:    nativeSetContent
 * Signature: (JLjava/lang/String;ILjava/lang/String;Ljava/lang/String;Ljava/lang/String;)Z
 */
extern "C" jboolean JNICALL Java_com_arcsoft_wavelink_WaveBuilder_nativeSetContent
  (JNIEnv* env, jclass, jlong handle, jstring ip, jint port, jstring ssid, jstring password, jstring mode)
{
    Sonic::CBuilder* builder = (Sonic::CBuilder*)handle;
    assert(builder != NULL);

    const char* ipstr = env->GetStringUTFChars(ip, NULL);
    const char* ssidstr = env->GetStringUTFChars(ssid, NULL);
    const char* passwdstr = env->GetStringUTFChars(password, NULL);
    const char* modestr = env->GetStringUTFChars(mode, NULL);

    std::vector<char> result;
    make_wifi_packet(ipstr, port, ssidstr, passwdstr, modestr, std::back_inserter(result));

    bool succ = false;
    if (!result.empty())
    {
        succ = builder->SetContent(result.data(), result.size());
    }

    env->ReleaseStringUTFChars(ip, ipstr);
    env->ReleaseStringUTFChars(ssid, ssidstr);
    env->ReleaseStringUTFChars(password, passwdstr);
    env->ReleaseStringUTFChars(mode, modestr);

    return succ;
}

/*
 * Class:     com_arcsoft_wavelink_WaveBuilder
 * Method:    nativeReadWave
 * Signature: (J[B)I
 */
extern "C" jint JNICALL Java_com_arcsoft_wavelink_WaveBuilder_nativeReadWave
  (JNIEnv* env, jclass, jlong handle, jbyteArray buffer)
{
    Sonic::CBuilder* builder = (Sonic::CBuilder*)handle;
    assert(builder != NULL);

    jboolean isCopy = false;
    jbyteArray pArray = (jbyteArray) env->GetPrimitiveArrayCritical(buffer, &isCopy);
    jsize size = env->GetArrayLength(buffer);

    int readBytes = builder->ReadPcm((char*)pArray, size);
    env->ReleasePrimitiveArrayCritical(buffer, pArray, JNI_COMMIT);

    return readBytes;
}

/*
 * Class:     com_arcsoft_wavelink_WaveBuilder
 * Method:    nativeGetDuration
 * Signature: (J)I
 */
extern "C" jint JNICALL Java_com_arcsoft_wavelink_WaveBuilder_nativeGetDuration
  (JNIEnv* env, jclass, jlong handle)
{
    Sonic::CBuilder* builder = (Sonic::CBuilder*)handle;
    assert(builder != NULL);
    return builder->GetDuration();
}

