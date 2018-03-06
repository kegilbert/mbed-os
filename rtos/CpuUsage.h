/* mbed Microcontroller Library
 * Copyright (c) 2006-2013 ARM Limited
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#if DEVICE_LOWPOWERTIMER

#ifndef CPU_USAGE_H
#define CPU_USAGE_H

#include "platform/NonCopyable.h"
#include "LowPowerTicker.h"

using namespace mbed;

extern uint32_t mbed_time_idle(void);

namespace rtos {
/** \addtogroup rtos */
/** @{*/
/**
 * \defgroup CpuUsage CPU Usage class
 * @{
 */

typedef struct cpu_usage_stats {
    float usage;
    float mean_usage;
    float variance;
} cpu_usage_stats_t;

class CpuUsage : private mbed::NonCopyable<CpuUsage> {
public:

    /** Get the current CPU usage.
     *  CPU Usage is measured is mesaured using time spent in sleeping in specified
     *  sample time period.
     *
     *  @params sample_time - CPU usage is measured every x sample_time in msecs
     */
    CpuUsage(uint32_t sample_time = 1000, uint32_t record_window_length = 10) : _sample_time(sample_time), _record_window_length(record_window_length) {
        _prev_time_idle = mbed_time_idle();
        _timer.attach_us(callback(this, &CpuUsage::calculate_cpu_usage), (_sample_time*1000));
    }

    ~CpuUsage() {
    }

    cpu_usage_stats_t get_cpu_usage_stats() const {
        return _usage_stats;
    }

private:
    uint32_t _sample_time;
    uint32_t _prev_time_idle;
    uint32_t _record_index;
    uint32_t _record_window_length;
    LowPowerTicker _timer;
    cpu_usage_stats_t _usage_stats;


    void calculate_cpu_usage(void) {
        uint32_t time_idle = mbed_time_idle();
        if ( time_idle < _prev_time_idle) {
            _usage_stats.usage = 100 - (((~0) - _prev_time_idle) + time_idle) / 10000;
        } else {
            _usage_stats.usage = 100 - (time_idle - _prev_time_idle) / 10000;
        }
        _prev_time_idle = time_idle;

        /* Calculate mean usage and variance */
        /* TODO: Make alpha value configurable */
        _usage_stats.variance = (_usage_stats.variance * 0.9/*alpha constant*/) +
                                    (abs(_usage_stats.usage - _usage_stats.mean_usage) * 0.1/*1-alpha*/);

        _usage_stats.mean_usage = (_usage_stats.mean_usage * 0.9) + (_usage_stats.usage * 0.1);
        _record_index =  (_record_index > _record_window_length) ? _record_window_length :
                                                                   _record_index + 1;
    }
};

/** @}*/
/** @}*/
}
#endif
#endif // DEVICE_LOWPOWERTIMER
