#include "Ifx_Types.h"
#include "IfxCpu.h"
#include "IfxScuWdt.h"
#include "main.h"
#include <stdio.h>
#include <string.h>
IFX_ALIGN(4) IfxCpu_syncEvent g_cpuSyncEvent = 0;
float32 distance;

volatile float right_distance __attribute__((section(".lmu_data"))) = 0.0f;
volatile float left_distance  __attribute__((section(".lmu_data"))) = 0.0f;
volatile float rear_distance  __attribute__((section(".lmu_data"))) = 0.0f;
volatile IfxCpu_mutexLock distLock __attribute__((section(".lmu_data"))) = 0;

unsigned int ToftofValue = 0;

void core0_main(void)
{
    SYSTEM_Init();
    /* !!WATCHDOG0 AND SAFETY WATCHDOG ARE DISABLED HERE!!
     * Enable the watchdogs and service them periodically if it is required
     */
//
//    /* Wait for CPU sync event */
//    IfxCpu_emitEvent(&g_cpuSyncEvent);
    
    // 0:0:0:11:11:11
    /* Define a MAC Address */
    eth_addr_t ethAddr = {
            .addr[0] = 0x00,
            .addr[1] = 0x00,
            .addr[2] = 0x0c,
            .addr[3] = 0x11,
            .addr[4] = 0x11,
            .addr[5] = 0x11   };

    initLwip(ethAddr); /* Initialize LwIP with the MAC address */

    my_printf("NETIF status: up=%d link=%d ip=0x%08X",
                 netif_is_up(&g_Lwip.netif), netif_is_link_up(&g_Lwip.netif),
                 (unsigned)netif_ip4_addr(&g_Lwip.netif)->addr);

    SOMEIPSD_Init();
    SOMEIP_Init();
    DoIP_Init();


    while (1)
    {
        Ifx_Lwip_pollTimerFlags(); /* Poll LwIP timers and trigger protocols execution if required */
        Ifx_Lwip_pollReceiveFlags(); /* Receive data package through ETH */

        GPIO_SetLed(1,1);
        GPIO_SetLed(2,1);
    } /* End of while */

}

//AEB, LWDS
IFX_INTERRUPT(CanRxHandler, 0, ISR_PRIORITY_CAN_RX);
void CanRxHandler (void)
{
    unsigned int rxID;
    unsigned char rxData[8] = {0, };
    int rxLen;
    Can_RecvMsg(&rxID, rxData, &rxLen);

    if (rxID == 0x123)
    {
        my_printf("test\n");
        /*if(LKAS_flag){
            //my_printf("LKAS\n");
            //TODO : Vision CAN Frame ???  logic ??? ???

            //rxData[0] : ??? ?????(0,1)
            //rxData[1] : ?????????? (0:???,1:??2:??
            if (rxData[1] != '0')
            {
                setBeepCycle(50);
            }
            else
            {
                setBeepCycle(0);
            }

        }*/

    }
    else
    {
        ToftofValue = rxData[2] << 16 | rxData[1] << 8 | rxData[0];
        unsigned char dis_status = rxData[3];
        unsigned short signal_strength = rxData[5] << 8 | rxData[4];
        //my_printf("tof distance: %d\n", ToftofValue);
        //my_printf("tof\n");
        if ((ToftofValue < 150 + Braking_Distance) && Car_dir == 1) //????? ??????????
            {

            if (!AEB_flag)
            {
                Motor_All_Stop();

//                my_printf("tof distance: %d\n", ToftofValue);
//                my_printf("Goal dist : %f\n", (100.0 + Braking_Distance));

    //            Motor_movChA_PWM(75, 0);
    //            Motor_movChB_PWM(75, 0);
    //            delay_ms(500);
                my_printf("stop\n");
                HazzardLight = 1;
            }
            //my_printf("stopping\n");
            AEB_flag = 1;
            //??? ??? Debugging??            //TODO : ??? ??? ????????

        }
        else
        {
            HazzardLight = 0;
            AEB_flag = 0;
        }

    }

}

