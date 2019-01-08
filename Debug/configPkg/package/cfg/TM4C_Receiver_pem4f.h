/*
 *  Do not modify this file; it is automatically 
 *  generated and any modifications will be overwritten.
 *
 * @(#) xdc-B06
 */

#include <xdc/std.h>

#include <ti/sysbios/knl/Swi.h>
extern const ti_sysbios_knl_Swi_Handle gdo0_swi;

#include <ti/sysbios/knl/Clock.h>
extern const ti_sysbios_knl_Clock_Handle channel_timeout;

#include <ti/sysbios/knl/Task.h>
extern const ti_sysbios_knl_Task_Handle usb_tx_task;

#include <ti/sysbios/knl/Task.h>
extern const ti_sysbios_knl_Task_Handle change_channel_task;

#include <ti/sysbios/knl/Semaphore.h>
extern const ti_sysbios_knl_Semaphore_Handle change_channel_sem;

#include <ti/sysbios/knl/Mailbox.h>
extern const ti_sysbios_knl_Mailbox_Handle pkt_mbx;

extern int xdc_runtime_Startup__EXECFXN__C;

extern int xdc_runtime_Startup__RESETFXN__C;

