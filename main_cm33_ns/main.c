/*******************************************************************************
* File Name:   main.c
*
* Description: This is the main file for the CM33 non-secure application.
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
********************************************************************************/

/*******************************************************************************
* Header Files
*******************************************************************************/
#include "cy_system_ppca_init.h"
#include "cybsp.h"
#include "cy_retarget_io.h"


/*******************************************************************************
* Macros
*******************************************************************************/
/* PPCA core image addresses */
#define CORE0_IMAGE_ADDRESS     CYMEM_CM33_0_m33ns_ppca0_nvm_C_START
#define CORE1_IMAGE_ADDRESS     CYMEM_CM33_0_m33ns_ppca1_nvm_C_START

/* Image configuration */
#define PPCA0_IMAGE_SIZE        CYMEM_CM33_0_ppca0_code_SIZE
#define PPCA1_IMAGE_SIZE        CYMEM_CM33_0_ppca1_code_SIZE

/* Memory addresses for PPCA inter-core communication */
#define PPCA_M1_VAR_ADDRESS     (0x53050400U)
#define PPCA_M3_VAR_ADDRESS     (0x53050800U)


/*******************************************************************************
* Global Variables
*******************************************************************************/
/* Debug UART context */
static cy_stc_scb_uart_context_t DEBUG_UART_context;

/* Debug UART HAL object */
static mtb_hal_uart_t DEBUG_UART_hal_obj;

/* Pointer to shared variable for PPCA core0 communication */
uint32_t *ppca_core0_var = (uint32_t *)PPCA_M1_VAR_ADDRESS;


/*******************************************************************************
* Function Prototypes
*******************************************************************************/


/*******************************************************************************
* Function Definitions
*******************************************************************************/

/*******************************************************************************
* Function Name: main
********************************************************************************
* Summary:
*  Main function for CM33 non-secure application. Initializes the board,
*  debug UART, PPCA configuration, and boots the PPCA cores.
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
    cy_rslt_t result;

    /* Initialize the device and board peripherals */
    result = cybsp_init();

    /* Board init failed. Stop program execution */
    if (result != CY_RSLT_SUCCESS)
    {
        CY_ASSERT(0);
    }

    /* Initialize the debug UART */
    Cy_SCB_UART_Init(DEBUG_UART_HW, &DEBUG_UART_config, &DEBUG_UART_context);
    Cy_SCB_UART_Enable(DEBUG_UART_HW);

    /* Setup the HAL UART */
    result = mtb_hal_uart_setup(&DEBUG_UART_hal_obj, &DEBUG_UART_hal_config,
                                &DEBUG_UART_context, NULL);

    /* HAL UART init failed. Stop program execution */
    if (result != CY_RSLT_SUCCESS)
    {
        CY_ASSERT(0);
    }

    /* Initialize retarget-io to redirect low level IO to UART */
    result = cy_retarget_io_init(&DEBUG_UART_hal_obj);

    /* retarget-io init failed. Stop program execution */
    if (result != CY_RSLT_SUCCESS)
    {
        CY_ASSERT(0);
    }

    /* Enable PPCA and configure output selector */
    Cy_PPCA_Enable(CNFG_PPCA_INOUT_HW);
    Cy_PPCA_CNFG_PPCA_Output_Selector(CNFG_PPCA_INOUTCNFG_HW, &CNFG_PPCA_INOUT_ppcaOutConfig);

    /* \x1b[2J\x1b[;H - ANSI ESC sequence for clear screen */
    printf("\x1b[2J\x1b[;H");
    printf("************************************************************\r\n");
    printf("PSOC Control C3M/P8: GPIO toggle using EPU interrupt\r\n");
    printf("************************************************************\r\n\n");

    /* Enable global interrupts */
    __enable_irq();

    /* Initialize and boot PPCA CPU0 */
    Cy_System_Init_CPU0((void*)CORE0_IMAGE_ADDRESS, PPCA0_IMAGE_SIZE);
    Cy_System_Init_CPU1((void*)CORE1_IMAGE_ADDRESS, PPCA1_IMAGE_SIZE);

    printf("PPCA CM33 cores Boot called\r\n");

    for (;;)
    {
    }
}


/* [] END OF FILE */
