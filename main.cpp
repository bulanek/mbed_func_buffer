#include <mbed.h>
#include <zdebug.h>

#include "uart_com.h"
#include "diag_com.h"
#include "thread_handle_com.h"

// TODO::TURBYHO: nakonfigurovat spravne hodiny u cinske desky -> otestovat
// TODO::TURBYHO: nadefinovat nazvy pinu u obou desek (targets)!

// example of a condition by target
// mbed compile -t GCC_ARM -m CH_ECH_BOARD
#ifdef TARGET_CH_ECH_BOARD
    #warning CHINA BOARD!
#endif

// mbed compile -t GCC_ARM -m P_ECH_BOARD
#ifdef TARGET_P_ECH_BOARD
    #warning PACKETA BOARD!
#endif

int main(void) {

    // See if a debugger is attached
    //uint32_t dhcsr = CoreDebug->DHCSR;
    //bool debuggerAttached = dhcsr & CoreDebug_DHCSR_C_DEBUGEN_Msk;
    //zdebug_if(debuggerAttached,"SEGGER:%d\n", (int)debuggerAttached);    
    
    #ifdef TARGET_P_ECH_BOARD    
    /*
    Upon reset, a pull-down resistor might be present on PA8, PD0, or PD2, depending on the voltage level on PB0, PA9, 
    PC6, PA10, PD1, and PD3. In order to disable this resistor, strobe the UCPDx_STROBE bit of the SYSCFG_CFGR1 
    register during start-up sequence.
    */
    SET_BIT(SYSCFG->CFGR1,SYSCFG_CFGR1_UCPD1_STROBE);
    SET_BIT(SYSCFG->CFGR1,SYSCFG_CFGR1_UCPD2_STROBE);
    #endif

    ZBOX_LOG("CPU::F:%luHz", SystemCoreClock);
#ifdef MBED_DEBUG
    HAL_DBGMCU_EnableDBGSleepMode();
#endif

    uart_initialize();

    while (1)
    {
        ZBOX_LOG("main thread loop ");
         thread_handle_packet_process();
    }

    return 0;
}
