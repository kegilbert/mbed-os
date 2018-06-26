#include "lwip/tcp.h"
#include "platform/mbed_critical.h"

struct tcp_event_t tcp_events[TCP_EVENT_LOG_DEPTH];

static uint8_t tcp_events_i = 0;

struct tcp_event_t* get_tcp_events_log(void) {
    return tcp_events;
}

uint8_t tcp_event_log_size(void) {
    return tcp_events_i;
}

void clear_tcp_events(void) {
    tcp_events_i = 0;
}

/*
*   C version of Kernel::get_ms_count(), duplicate source
*/
static uint64_t get_kernel_ticks_extended(void) {
    if (sizeof osKernelGetTickCount() == sizeof(uint64_t)) {
        return osKernelGetTickCount();
    } else /* assume 32-bit */ {
        // Based on suggestion in CMSIS-RTOS 2.1.1 docs, but with reentrancy
        // protection for the tick memory. We use critical section rather than a
        // mutex, as hopefully this method can be callable from interrupt later -
        // only thing currently preventing it is that pre CMSIS RTOS 2.1.1, it's
        // not defined as safe.
        // We assume this is called multiple times per 32-bit wrap period (49 days).
        static uint32_t tick_h, tick_l;

        core_util_critical_section_enter();
        // The 2.1.1 API says this is legal from an ISR - we assume this means
        // it's also legal with interrupts disabled. RTX implementation kind
        // of conflates the two.
        uint32_t tick32 = osKernelGetTickCount();
        if (tick32 < tick_l) {
            tick_h++;
        }
        tick_l = tick32;
        uint64_t ret = ((uint64_t) tick_h << 32) | tick_l;
        core_util_critical_section_exit();
        return ret;
    }
}

void tcp_change_state_logging(struct tcp_pcb *pcb, enum tcp_state new_state) {
    pcb->state = new_state;

    if (tcp_events_i == TCP_EVENT_LOG_DEPTH) {
        printf("WARNING: EXCEEDED TCP EVENT LOG DEPTH\r\n");
    } else {
        tcp_events[tcp_events_i].local_ip    = pcb->local_ip;
        tcp_events[tcp_events_i].remote_ip   = pcb->remote_ip;
        tcp_events[tcp_events_i].local_port  = pcb->local_port;
        tcp_events[tcp_events_i].remote_port = pcb->remote_port;
        tcp_events[tcp_events_i].state       = new_state;
        tcp_events[tcp_events_i].timestamp   = get_kernel_ticks_extended();
        strncpy(tcp_events[tcp_events_i].thread_name, osThreadGetName(osThreadGetId()), 32);

        tcp_events_i += 1;
#if TCP_EVENT_DEBUG
        printf("PCB: %x | DATA: %d:%d:%d, %d, %d, %s\r\n",
            pcb, pcb->remote_ip, pcb->local_port, pcb->remote_port, new_state, osKernelGetTickCount(), osThreadGetName(osThreadGetId()));
#endif
    }
}
