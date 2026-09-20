/*******************************************************************************
* File Name:   main.c
*
* Description: This application demonstrates how to toggle a GPIO using
*              an EPU interrupt in a PPCA core application. The EPU is
*              configured to generate an interrupt on a timer event and
*              the EPU ISR toggles a GPIO pin.
*
* Related Document: See README.md
*
*
********************************************************************************
* (c) 2026, Infineon Technologies AG, or an affiliate of Infineon
* Technologies AG. All rights reserved.
* This software, associated documentation and materials ("Software") is
* owned by Infineon Technologies AG or one of its affiliates ("Infineon")
* and is protected by and subject to worldwide patent protection, worldwide
* copyright laws, and international treaty provisions. Therefore, you may use
* this Software only as provided in the license agreement accompanying the
* software package from which you obtained this Software. If no license
* agreement applies, then any use, reproduction, modification, translation, or
* compilation of this Software is prohibited without the express written
* permission of Infineon.
*
* Disclaimer: UNLESS OTHERWISE EXPRESSLY AGREED WITH INFINEON, THIS SOFTWARE
* IS PROVIDED AS-IS, WITH NO WARRANTY OF ANY KIND, EXPRESS OR IMPLIED,
* INCLUDING, BUT NOT LIMITED TO, ALL WARRANTIES OF NON-INFRINGEMENT OF
* THIRD-PARTY RIGHTS AND IMPLIED WARRANTIES SUCH AS WARRANTIES OF FITNESS FOR A
* SPECIFIC USE/PURPOSE OR MERCHANTABILITY.
* Infineon reserves the right to make changes to the Software without notice.
* You are responsible for properly designing, programming, and testing the
* functionality and safety of your intended application of the Software, as
* well as complying with any legal requirements related to its use. Infineon
* does not guarantee that the Software will be free from intrusion, data theft
* or loss, or other breaches ("Security Breaches"), and Infineon shall have
* no liability arising out of any Security Breaches. Unless otherwise
* explicitly approved by Infineon, the Software may not be used in any
* application where a failure of the Product or any consequences of the use
* thereof can reasonably be expected to result in personal injury.
*******************************************************************************/

/*******************************************************************************
* Header Files
*******************************************************************************/
#include "cy_pdl.h"
#include "cycfg.h"
#include <stdio.h>


/*******************************************************************************
* Macros
*******************************************************************************/
/* Delay in milliseconds for main loop */
#define LOOP_DELAY_MS           (1000U)

/* Memory addresses for PPCA inter-core communication */
#define PPCA_M1_VAR_ADDRESS     (0x20040400U)
#define PPCA_M3_VAR_ADDRESS     (0x20040800U)


/*******************************************************************************
* Global Variables
*******************************************************************************/
/* Pointer to shared variable for inter-core communication */
uint32_t *var = (uint32_t *)PPCA_M1_VAR_ADDRESS;

/* EPU combiner interrupt configuration */
const cy_stc_sysint_t combiner0_interrupt_config =
{
    .intrSrc = EPU_BLK_IRQ_EPU_0,   /* Running on PPCA core, value is 5 */
    .intrPriority = 2U
};


/*******************************************************************************
* Function Prototypes
*******************************************************************************/
void combiner0_interrupt_handler(void);


/*******************************************************************************
* Function Definitions
*******************************************************************************/

/*******************************************************************************
* Function Name: combiner0_interrupt_handler
********************************************************************************
* Summary:
*  EPU combiner interrupt handler. Clears the interrupt and toggles the user LED.
*
* Parameters:
*  void
*
* Return:
*  void
*
*******************************************************************************/
void combiner0_interrupt_handler(void)
{
    /* Clear the EPU interrupt using 0th EPU IRQ */
    Cy_PPCA_EPU_ClearInterrupt(EPU_BLK_EPU_IRQ0_HW);

    /* Toggle the user LED */
    Cy_GPIO_Inv(CYBSP_USER_LED_PORT, CYBSP_USER_LED_PIN);
}


/*******************************************************************************
* Function Name: main
********************************************************************************
* Summary:
*  Main function for the PPCA core. Configures the EPU to generate interrupts
*  on timer events and toggles a GPIO in the EPU ISR.
*
* Parameters:
*  void
*
* Return:
*  int
*
*******************************************************************************/
int main(void)
{
    /* Enable global interrupts */
    __enable_irq();

    /* EPU configuration - enable exclusive access and EPU block */
    Cy_PPCA_EPU_EnableExclusiveAccess(EPU_BLK_HW, true);
    Cy_PPCA_EPU_Enable(EPU_BLK_HW);

    /* Configure EPU processing units and combiner */
    Cy_PPCA_EPU_PU_T1_Configure(put1_0_HW, put1_0_INDEX, &put1_0_put1_config);
    Cy_PPCA_EPU_PU_T1_Enable(put1_0_HW, put1_0_INDEX, put1_0_ENABLE_MODE);
    Cy_PPCA_EPU_Combo_Configure(combiner0_HW, combiner0_INDEX, &combiner0_combo_config);

    /* Initialize and enable the EPU combiner interrupt */
    Cy_SysInt_Init(&combiner0_interrupt_config, &combiner0_interrupt_handler);
    NVIC_EnableIRQ((IRQn_Type)combiner0_interrupt_config.intrSrc);

    /* Configure EPU interrupt source and mask */
    Cy_PPCA_EPU_InterruptSourceSelect(EPU_BLK_EPU_IRQ0_HW, false, epuIrqSrc0);
    Cy_PPCA_EPU_SetInterruptMask(EPU_BLK_EPU_IRQ0_HW);

    /* Initialize, enable, and start the timer */
    Cy_TCPWM_Counter_Init(TIMER_BLK_HW, TIMER_BLK_NUM, &TIMER_BLK_config);
    Cy_TCPWM_Counter_Enable(TIMER_BLK_HW, TIMER_BLK_NUM);
    Cy_TCPWM_TriggerStart_Single(TIMER_BLK_HW, TIMER_BLK_NUM);

    for (;;)
    {
        Cy_SysLib_Delay(LOOP_DELAY_MS);
    }
}


/* [] END OF FILE */
