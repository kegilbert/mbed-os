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
    
class CpuUsage : private mbed::NonCopyable<CpuUsage> {
public:

    /** Get the current CPU usage. 
     *  CPU Usage is measured is mesaured using time spent in sleeping in specified
     *  sample time period.
     *
     *  @params sample_time - CPU usage is measured every x sample_time in msecs
     */
    CpuUsage(uint32_t sample_time = 1000) : _sample_time(sample_time), _usage(0) {
        _prev_time_idle = mbed_time_idle();
        _timer.attach_us(callback(this, &CpuUsage::calculate_cpu_usage), (_sample_time*1000));
    }
    
    ~CpuUsage() {
    }
    
    uint32_t get_cpu_usage() const {
        return _usage;
    }

private:
    uint32_t _sample_time;
    uint32_t _prev_time_idle;
    LowPowerTicker _timer;
    uint32_t _usage;

    void calculate_cpu_usage(void) {
        uint32_t time_idle = mbed_time_idle();
        if ( time_idle < _prev_time_idle) {
            _usage = 100 - (((~0) - _prev_time_idle) + time_idle) / 10000;
        } else {
            _usage = 100 - (time_idle - _prev_time_idle) / 10000;
        }
        _prev_time_idle = time_idle;
    }
};

/** @}*/
/** @}*/
}
#endif
