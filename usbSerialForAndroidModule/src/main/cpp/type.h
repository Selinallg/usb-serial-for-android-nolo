/*
 * @Copyright: nolo
 * @Author: qujunyang
 * @Date: 2022-05-04 19:49:47
 * @LastEditTime: 2022-05-04 19:58:43
 * @LastEditors: qujunyang
 * @FilePath: \NOLOSensor\include\nolo\type.h
 * @Description:
 */
#ifndef NOLO_TYPE_H
#define NOLO_TYPE_H

#include <functional>
#include <iostream>

#ifdef WIN32
#define NOLO_DECL_EXPORT __declspec(dllexport)
#define NOLO_DECL_IMPORT __declspec(dllimport)
#define NOLO_DECL_HIDDEN
#else
#define NOLO_DECL_EXPORT __attribute__((visibility("default")))
#define NOLO_DECL_IMPORT __attribute__((visibility("default")))
#define NOLO_DECL_HIDDEN __attribute__((visibility("hidden")))
#endif

#ifdef NOLO_EXPORTS
#define NOLO_API NOLO_DECL_EXPORT
#else
#define NOLO_API NOLO_DECL_IMPORT
#endif

namespace nolo {

    enum class STATUS_TYPE : int32_t {
        OPEN_SUCCESS,
        OPEN_FAILED,
        HAVE_OPENED_DEVICE,
        OPEN_CAMERA_DEVICE_FAILED,
        START_SUCCESS,
        STOP_SUCCESS,
        DEVICE_IS_CLOSED,
        INIT_HID_FAILED,
        OPEN_HID_FAILED,
        CLAIM_HID_FAILED,
        CLAIM_HID_SUCCESS,
        REQUIRE_HID_DATA_ERROR,
        OPEN_HID_SUCCESS,
        HAVE_CAPTURED_DATA,
        START_IMU_STREAM_FAILED,
        START_CAMERA_STREAM_FAILED,
        START_FAILED,
        STOP_FAILED,
        UNSUPPORTED_FPS,
    };

    enum class NOLO_DEVICE_TYPE : int32_t {
        OV580,
        OV680,
    };

    enum class OP_TYPE : int32_t {
        ALL,
        CAMERA,
        IMU,
    };

    enum class RECTIFY_TYPE : int32_t {
        NONE,
        RECTIFY_IMU,
        RECTIFY_CAMERA,
        RECTIFY_ALL,
    };

    enum class NOLO_FPS : int32_t {
        NONE = 0,
        FPS_30 = 30,
        FPS_50 = 50,
        FPS_60 = 60,
    };

    enum class IMU_TYPE : int32_t {
        HEAD = 0,
        LEFT_HAND = 1,
        RIGHT_HAND = 2,
        LEFT_RIGHT_HAND = 3,
        ALL = 4,
    };

// imu parameters
    typedef struct NOLO_API IMUIntrinsics {
        double _Ta[3][3] = {{0}};
        double _Ka[3][3] = {{0}};
        double _Ba[3] = {0};
        double _Tg[3][3] = {{0}};
        double _Kg[3][3] = {{0}};
        double _Bg[3] = {0};

        friend std::ostream &operator<<(std::ostream &os,
                                        const IMUIntrinsics &intrinsics) {
            os << "Ta: [ ";
            for (int32_t i = 0; i < 3; ++i) {
                os << intrinsics._Ta[0][i] << ", ";
            }
            for (int32_t i = 0; i < 3; ++i) {
                os << intrinsics._Ta[1][i] << ", ";
            }
            for (int32_t i = 0; i < 2; ++i) {
                os << intrinsics._Ta[2][i] << ", ";
            }
            os << intrinsics._Ta[2][2] << " ]";

            os << ", Ka: [ ";
            for (int32_t i = 0; i < 3; ++i) {
                os << intrinsics._Ka[0][i] << ", ";
            }
            for (int32_t i = 0; i < 3; ++i) {
                os << intrinsics._Ka[1][i] << ", ";
            }
            for (int32_t i = 0; i < 2; ++i) {
                os << intrinsics._Ka[2][i] << ", ";
            }
            os << intrinsics._Ka[2][2] << " ]";

            os << ", Ba: [ ";
            for (int32_t i = 0; i < 2; ++i) {
                os << intrinsics._Ba[i] << ", ";
            }
            os << intrinsics._Ba[2] << " ]" << std::endl;

            os << "Tg: [ ";
            for (int32_t i = 0; i < 3; ++i) {
                os << intrinsics._Tg[0][i] << ", ";
            }
            for (int32_t i = 0; i < 3; ++i) {
                os << intrinsics._Tg[1][i] << ", ";
            }
            for (int32_t i = 0; i < 2; ++i) {
                os << intrinsics._Tg[2][i] << ", ";
            }
            os << intrinsics._Tg[2][2] << " ]";

            os << ", Kg: [ ";
            for (int32_t i = 0; i < 3; ++i) {
                os << intrinsics._Kg[0][i] << ", ";
            }
            for (int32_t i = 0; i < 3; ++i) {
                os << intrinsics._Kg[1][i] << ", ";
            }
            for (int32_t i = 0; i < 2; ++i) {
                os << intrinsics._Kg[2][i] << ", ";
            }
            os << intrinsics._Kg[2][2] << " ]";

            os << ", Ba: [ ";
            for (int32_t i = 0; i < 2; ++i) {
                os << intrinsics._Bg[i] << ", ";
            }
            os << intrinsics._Bg[2] << " ]" << std::endl;
            return os;
        }
    } IMUIntrinsics;

// imu data packet
    typedef struct NOLO_API IMUData {
        uint64_t _accelTimestamp = 0;
        uint64_t _gyroTimestamp = 0;
        int16_t _size = 0;
        int16_t _cmd = 0;
        // 0 -- x 1 -- y 2 -- z
        int16_t _gyro[3] = {0};
        int16_t _accel[3] = {0};
        double _cGyro[3] = {0};
        double _cAccel[3] = {0};

        bool _isCalib = false;
        // 0 头部 IMU 数据  1 左手 IMU 数据  2 右手 IMU 数据
        int32_t _type = 0;
        bool _isValid = false;

        IMUData();

        IMUData(const uint8_t *data, const std::size_t &size,
                const NOLO_DEVICE_TYPE &type);

        IMUData(const uint8_t *data, const std::size_t &size,
                const NOLO_DEVICE_TYPE &type, const IMUIntrinsics &intrinsics);

        void OV580IMU(const uint8_t *data);

        bool isOV680(const uint8_t *data, const std::size_t &size);

        void OV680IMU(const uint8_t *data, const std::size_t &size);

        friend std::ostream &operator<<(std::ostream &os, const IMUData &data) {
            os << "imu type: " << data._type << std::endl;
            os << "gyro [ " << data._gyroTimestamp << ", " << data._gyro[0] << ", "
               << data._gyro[1] << ", " << data._gyro[2] << " ]" << std::endl;
            os << "accel [ " << data._accelTimestamp << ", " << data._accel[0] << ", "
               << data._accel[1] << ", " << data._accel[2] << " ]" << std::endl;

            if (data._isCalib) {
                os << "calibrated gyro [ " << data._gyroTimestamp << ", "
                   << data._cGyro[0] << ", " << data._cGyro[1] << ", " << data._cGyro[2]
                   << " ]" << std::endl;
                os << "calibrated accel [ " << data._accelTimestamp << ", "
                   << data._cAccel[0] << ", " << data._cAccel[1] << ", "
                   << data._cAccel[2] << " ]" << std::endl;
            }

            return os;
        }
    } IMUData;

// nsync data packet
    typedef struct NOLO_API NSyncData {
        int16_t _size = 0;
        int16_t _cmd = 0;
        uint64_t _syncTimestamp = 0;

        NSyncData();

        NSyncData(const uint8_t *data, const std::size_t &size);

        void ProcessNSyncData(const uint8_t *data, const std::size_t &size);
    } NSyncData; // namespace nolo


    int getDeviceType(const uint8_t *data);
}
#endif  // NOLO_TYPE_H
