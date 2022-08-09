/*
 * @Copyright: nolo
 * @Author: qujunyang
 * @Date: 2022-05-04 19:50:50
 * @LastEditTime: 2022-05-04 19:59:41
 * @LastEditors: qujunyang
 * @FilePath: \NOLOSensor\src\nolo\type.cpp
 * @Description:
 */
#include "utils.h"
#include "type.h"

namespace nolo {

    IMUData::IMUData() {}

    IMUData::IMUData(const uint8_t *data, const std::size_t &size,
                     const NOLO_DEVICE_TYPE &type) {
        switch (type) {
            case NOLO_DEVICE_TYPE::OV580: {
                OV580IMU(data);
                break;
            }
            case NOLO_DEVICE_TYPE::OV680: {
                OV680IMU(data, size);
                break;
            }
            default:
                // TODO: default imu device
                break;
        }
    }

    IMUData::IMUData(const uint8_t *data, const std::size_t &size,
                     const NOLO_DEVICE_TYPE &type,
                     const IMUIntrinsics &intrinsics) {
        _isCalib = true;
        switch (type) {
            case NOLO_DEVICE_TYPE::OV580: {
                OV580IMU(data);
                _cAccel[0] = intrinsics._Ka[0][0] * (_accel[0] - intrinsics._Ba[0]);
                _cAccel[1] = intrinsics._Ka[1][1] * (_accel[1] - intrinsics._Ba[1]);
                _cAccel[2] = intrinsics._Ka[2][2] * (_accel[2] - intrinsics._Ba[2]);

                _cGyro[0] = intrinsics._Kg[0][0] * (_gyro[0] - intrinsics._Bg[0]);
                _cGyro[1] = intrinsics._Kg[1][1] * (_gyro[1] - intrinsics._Bg[1]);
                _cGyro[2] = intrinsics._Kg[2][2] * (_gyro[2] - intrinsics._Bg[2]);
                break;
            }
            case NOLO_DEVICE_TYPE::OV680: {
                OV680IMU(data, size);
                _cAccel[0] = intrinsics._Ka[0][0] * (_accel[0] - intrinsics._Ba[0]);
                _cAccel[1] = intrinsics._Ka[1][1] * (_accel[1] - intrinsics._Ba[1]);
                _cAccel[2] = intrinsics._Ka[2][2] * (_accel[2] - intrinsics._Ba[2]);

                _cGyro[0] = intrinsics._Kg[0][0] * (_gyro[0] - intrinsics._Bg[0]);
                _cGyro[1] = intrinsics._Kg[1][1] * (_gyro[1] - intrinsics._Bg[1]);
                _cGyro[2] = intrinsics._Kg[2][2] * (_gyro[2] - intrinsics._Bg[2]);
                break;
            }
            default:
                // TODO: default imu device
                break;
        }
    }

    void IMUData::OV580IMU(const uint8_t *data) {
        int32_t offset = 44;
        _type = 0;
        _isValid = true;
        OnUnpackNum<uint64_t>(data + offset, &_gyroTimestamp);
        //    static uint64_t last = _gyroTimestamp;
        //    if (_gyroTimestamp - last > 1500000) {
        //      std::cout << "lost imu: " << _gyroTimestamp << " last: " << last
        //                << std::endl;
        //    }
        //    last = _gyroTimestamp;
        offset += 16;
        OnUnpackNum<int16_t>(data + offset, &_gyro[0]);
        offset += 4;
        OnUnpackNum<int16_t>(data + offset, &_gyro[1]);
        offset += 4;
        OnUnpackNum<int16_t>(data + offset, &_gyro[2]);
        offset += 4;

        OnUnpackNum<uint64_t>(data + offset, &_accelTimestamp);
        offset += 16;
        OnUnpackNum<int16_t>(data + offset, &_accel[0]);
        offset += 4;
        OnUnpackNum<int16_t>(data + offset, &_accel[1]);
        offset += 4;
        OnUnpackNum<int16_t>(data + offset, &_accel[2]);

        //    if (_accel[0] > 32767) {
        //      _accel[0] = 65535 - _accel[0];
        //    }
        //    if (_accel[0] < -32767) {
        //      _accel[0] = 65535 + _accel[0];
        //    }
        //    if (_accel[1] > 32767) {
        //      _accel[1] = 65535 - _accel[1];
        //    }
        //    if (_accel[1] < -32767) {
        //      _accel[1] = 65535 + _accel[1];
        //    }
        //    if (_accel[2] > 32767) {
        //      _accel[2] = 65535 - _accel[2];
        //    }
        //    if (_accel[2] < -32767) {
        //      _accel[2] = 65535 + _accel[2];
        //    }
    }

    bool IMUData::isOV680(const uint8_t *data, const std::size_t &size) {
        if (size < 2) {
            return false;
        }

        int32_t offset = 0;
        _type = data[offset];
        OnUnpackNum<int16_t>(data + offset, &_size);
        offset += 2;
        OnUnpackNum<int16_t>(data + offset, &_cmd);
        if (_cmd == 0x036A)
            return true;

        return false;
    }

    void IMUData::OV680IMU(const uint8_t *data, const std::size_t &size) {
        int32_t offset = 4;
        _type = data[offset];
        // std::cout << "type: " << _type << std::endl;
        //    if (data[offset] != 0) {
        //      return;
        //    }
        offset += 1;
        OnUnpackNum<uint64_t>(data + offset, &_gyroTimestamp);
        // std::cout << "imu timestamp: " << _gyroTimestamp << std::endl;
        //    if (_type == 0) {
        //      static uint64_t last = _gyroTimestamp;
        //      if (_gyroTimestamp - last > 1500000) {
        //        std::cout << "lost imu: " << _gyroTimestamp << " last: " << last
        //                  << std::dec << std::endl;
        //      }
        //      last = _gyroTimestamp;
        //    }
        offset += 8;
        OnUnpackNum<int16_t>(data + offset, &_accel[0]);
        offset += 2;
        OnUnpackNum<int16_t>(data + offset, &_accel[1]);
        offset += 2;
        OnUnpackNum<int16_t>(data + offset, &_accel[2]);
        offset += 2;
        OnUnpackNum<int16_t>(data + offset, &_gyro[0]);
        offset += 2;
        OnUnpackNum<int16_t>(data + offset, &_gyro[1]);
        offset += 2;
        OnUnpackNum<int16_t>(data + offset, &_gyro[2]);

        //    std::cout << "gyro norm: "
        //              << _gyro[0] * _gyro[0] + _gyro[1] * _gyro[1] + _gyro[2] *
        //              _gyro[2]
        //              << std::endl;
    }

    NSyncData::NSyncData() {}

    NSyncData::NSyncData(const uint8_t *data, const std::size_t &size) {
        ProcessNSyncData(data, size);
    }

    void NSyncData::ProcessNSyncData(const uint8_t *data, const std::size_t &size) {
        int32_t offset = 0;
        OnUnpackNum<int16_t>(data + offset, &_size);
        offset += 2;
        OnUnpackNum<int16_t>(data + offset, &_cmd);
        offset += 2;
        OnUnpackNum<uint64_t>(data + offset, &_syncTimestamp);
    }

    int getDeviceType(const uint8_t *data) {
        int32_t offset = 2;
        int16_t _cmd = 0;
        OnUnpackNum<int16_t>(data + offset, &_cmd);
        if (_cmd == 0x036A) {
            return 0;
        }
        else if (_cmd == 0x036B) {
            return 1;
        } else {
            return 99;
        }
    }
}