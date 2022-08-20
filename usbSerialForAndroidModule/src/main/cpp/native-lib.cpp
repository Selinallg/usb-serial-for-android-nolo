#include <jni.h>
#include <string>
#include "type.h"
#include <android/log.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include "imudata.h"

#define TAG "TerminalFragment-jni" // 这个是自定义的LOG的标识
#define LOGD(...) __android_log_print(ANDROID_LOG_DEBUG,TAG ,__VA_ARGS__) // 定义LOGD类型
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO,TAG ,__VA_ARGS__) // 定义LOGI类型
#define LOGW(...) __android_log_print(ANDROID_LOG_WARN,TAG ,__VA_ARGS__) // 定义LOGW类型
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR,TAG ,__VA_ARGS__) // 定义LOGE类型
#define LOGF(...) __android_log_print(ANDROID_LOG_FATAL,TAG ,__VA_ARGS__) // 定义LOGF类型

uint64_t GetTimeNano(clockid_t clk_id)
{
    struct timespec t;
    clock_gettime(clk_id, &t);
    uint64_t result = t.tv_sec * 1000000000LL + t.tv_nsec;
    return result;
}

static jmethodID _method_imu = nullptr;
static jmethodID _method_nsync = nullptr;

static nolo::ImuCallBack _imuCallBack = nullptr;
void setImuDataCB(nolo::ImuCallBack cb){
    if (!_imuCallBack && cb) {
        _imuCallBack = cb;
    }
}


extern "C"
JNIEXPORT void JNICALL   //com.module.usbserialforandroid
Java_com_module_usbserialforandroid_TerminalClient_updateDatas(JNIEnv *env, jobject thiz, jbyteArray datas) {

    // TODO: implement updateDatas()
    jbyte* bBuffer = env->GetByteArrayElements(datas, NULL);
    jsize bSize = env->GetArrayLength(datas); //获取长度
    if (bBuffer) {
        unsigned char *buf = (unsigned char *) bBuffer;
        int type = nolo::getDeviceType(buf);
        if (type == 0) {
            nolo::IMUData *imuData = new nolo::IMUData(buf, bSize, nolo::NOLO_DEVICE_TYPE::OV680);
            ////// callback
            if (!_method_imu) {
                jclass clz = env->GetObjectClass(thiz);
                if (clz) {
                    _method_imu = env->GetMethodID(clz, "imuData", "(JSSSSSS)V");
                }
            }
            ////////////
            if (_method_imu && imuData) {
                env->CallVoidMethod(thiz, _method_imu, (jlong) imuData->_gyroTimestamp,
                                    imuData->_accel[0], imuData->_accel[1], imuData->_accel[2],
                                    imuData->_gyro[0], imuData->_gyro[1], imuData->_gyro[2]);

                if (_imuCallBack) {
                    nolo::ImuData imudata = {
                            ._gyroTimestamp = imuData->_gyroTimestamp,
                            ._size = imuData->_size,
                            ._cmd = imuData->_cmd,
                            // 0 -- x 1 -- y 2 -- z
                            ._gyro[0] =  imuData->_gyro[0],
                            ._gyro[1] =  imuData->_gyro[1],
                            ._gyro[2] =  imuData->_gyro[2],
                            ._accel[0] = imuData->_accel[0],
                            ._accel[1] = imuData->_accel[1],
                            ._accel[2] = imuData->_accel[2],
                            ._cGyro[0] = imuData->_cGyro[0],
                            ._cGyro[1] = imuData->_cGyro[1],
                            ._cGyro[2] = imuData->_cGyro[2],
                            ._cAccel[0] = imuData->_cAccel[0],
                            ._cAccel[1] = imuData->_cAccel[1],
                            ._cAccel[2] = imuData->_cAccel[2],
                    };
                    (*_imuCallBack)(imudata);
                }
                //LOGD("TerminalFragment_updateDatas 33333 time:%lld, acc0:%d, acc1:%d, acc2:%d, gyro0:%d, gyro1:%d, gyro2:%d", imuData->_gyroTimestamp,
                 //    imuData->_accel[0], imuData->_accel[1], imuData->_accel[2],
                  //   imuData->_gyro[0], imuData->_gyro[1], imuData->_gyro[2]);
            }
        }
        else if (type == 1) {
            ////// callback
            if (!_method_nsync) {
                jclass clz = env->GetObjectClass(thiz);
                if (clz) {
                    _method_nsync = env->GetMethodID(clz, "nsyncData", "(J)V");
                }
            }
            ////////////
            nolo::NSyncData* nsync = new nolo::NSyncData(buf, bSize);
            if (_method_nsync && nsync) {

                //
                //env->CallVoidMethod(thiz, _method_nsync, (jlong) nsync->_syncTimestamp);
                env->CallVoidMethod(thiz, _method_nsync, (jlong) GetTimeNano(CLOCK_MONOTONIC));
            }
        } else {

        }
    }
}