/*
 * Copyright (C) 2017 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef ANDROID_SENSOR_DEVICE_UTIL
#define ANDROID_SENSOR_DEVICE_UTIL

#include <android/hidl/manager/1.0/IServiceNotification.h>
#include <hardware/sensors.h>
#include <utils/Log.h>

#include <cmath>
#include <condition_variable>
#include <thread>

using ::android::hardware::hidl_string;
using ::android::hardware::Return;
using ::android::hidl::manager::V1_0::IServiceNotification;

namespace android {
namespace SensorDeviceUtils {

// Quantizes a single value to (a fractional factor of) a sensor's resolution. Typically we
// increase the value of the sensor's nominal resolution to ensure that sensor accuracy
// improvements, like runtime calibration, are not masked during requantization.
inline void quantizeValue(float *value, double resolution, double factor = 0.125) {
    if (resolution == 0) {
        return;
    }

    double incRes = factor * resolution;
    *value = round(static_cast<double>(*value) / incRes) * incRes;
}

// Ensures a sensor event doesn't provide values finer grained than its sensor resolution allows.
void quantizeSensorEventValues(sensors_event_t *event, float resolution);

// Returns the expected resolution value for the given sensor
float resolutionForSensor(const sensor_t &sensor);

class HidlServiceRegistrationWaiter : public IServiceNotification {
public:

    HidlServiceRegistrationWaiter();

    Return<void> onRegistration(const hidl_string &fqName,
                                const hidl_string &name,
                                bool preexisting) override;

    void reset();

    /**
     * Wait for service restart
     *
     * @return true if service is restart since last reset(); false otherwise.
     */
    bool wait();
protected:
    void onFirstRef() override;
private:
    bool mRegistered;

    std::mutex mLock;
    std::condition_variable mCondition;
    bool mRestartObserved;
};

} // namespace SensorDeviceUtils
} // namespace android;

#endif // ANDROID_SENSOR_SERVICE_UTIL
