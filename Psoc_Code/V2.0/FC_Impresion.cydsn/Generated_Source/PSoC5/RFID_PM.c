/*******************************************************************************
* File Name: RFID_PM.c
* Version 2.50
*
* Description:
*  This file provides Sleep/WakeUp APIs functionality.
*
* Note:
*
********************************************************************************
* Copyright 2008-2015, Cypress Semiconductor Corporation.  All rights reserved.
* You may use this file only in accordance with the license, terms, conditions,
* disclaimers, and limitations in the end user license agreement accompanying
* the software package with which this file was provided.
*******************************************************************************/

#include "RFID.h"


/***************************************
* Local data allocation
***************************************/

static RFID_BACKUP_STRUCT  RFID_backup =
{
    /* enableState - disabled */
    0u,
};



/*******************************************************************************
* Function Name: RFID_SaveConfig
********************************************************************************
*
* Summary:
*  This function saves the component nonretention control register.
*  Does not save the FIFO which is a set of nonretention registers.
*  This function is called by the RFID_Sleep() function.
*
* Parameters:
*  None.
*
* Return:
*  None.
*
* Global Variables:
*  RFID_backup - modified when non-retention registers are saved.
*
* Reentrant:
*  No.
*
*******************************************************************************/
void RFID_SaveConfig(void)
{
    #if(RFID_CONTROL_REG_REMOVED == 0u)
        RFID_backup.cr = RFID_CONTROL_REG;
    #endif /* End RFID_CONTROL_REG_REMOVED */
}


/*******************************************************************************
* Function Name: RFID_RestoreConfig
********************************************************************************
*
* Summary:
*  Restores the nonretention control register except FIFO.
*  Does not restore the FIFO which is a set of nonretention registers.
*
* Parameters:
*  None.
*
* Return:
*  None.
*
* Global Variables:
*  RFID_backup - used when non-retention registers are restored.
*
* Reentrant:
*  No.
*
* Notes:
*  If this function is called without calling RFID_SaveConfig() 
*  first, the data loaded may be incorrect.
*
*******************************************************************************/
void RFID_RestoreConfig(void)
{
    #if(RFID_CONTROL_REG_REMOVED == 0u)
        RFID_CONTROL_REG = RFID_backup.cr;
    #endif /* End RFID_CONTROL_REG_REMOVED */
}


/*******************************************************************************
* Function Name: RFID_Sleep
********************************************************************************
*
* Summary:
*  This is the preferred API to prepare the component for sleep. 
*  The RFID_Sleep() API saves the current component state. Then it
*  calls the RFID_Stop() function and calls 
*  RFID_SaveConfig() to save the hardware configuration.
*  Call the RFID_Sleep() function before calling the CyPmSleep() 
*  or the CyPmHibernate() function. 
*
* Parameters:
*  None.
*
* Return:
*  None.
*
* Global Variables:
*  RFID_backup - modified when non-retention registers are saved.
*
* Reentrant:
*  No.
*
*******************************************************************************/
void RFID_Sleep(void)
{
    #if(RFID_RX_ENABLED || RFID_HD_ENABLED)
        if((RFID_RXSTATUS_ACTL_REG  & RFID_INT_ENABLE) != 0u)
        {
            RFID_backup.enableState = 1u;
        }
        else
        {
            RFID_backup.enableState = 0u;
        }
    #else
        if((RFID_TXSTATUS_ACTL_REG  & RFID_INT_ENABLE) !=0u)
        {
            RFID_backup.enableState = 1u;
        }
        else
        {
            RFID_backup.enableState = 0u;
        }
    #endif /* End RFID_RX_ENABLED || RFID_HD_ENABLED*/

    RFID_Stop();
    RFID_SaveConfig();
}


/*******************************************************************************
* Function Name: RFID_Wakeup
********************************************************************************
*
* Summary:
*  This is the preferred API to restore the component to the state when 
*  RFID_Sleep() was called. The RFID_Wakeup() function
*  calls the RFID_RestoreConfig() function to restore the 
*  configuration. If the component was enabled before the 
*  RFID_Sleep() function was called, the RFID_Wakeup()
*  function will also re-enable the component.
*
* Parameters:
*  None.
*
* Return:
*  None.
*
* Global Variables:
*  RFID_backup - used when non-retention registers are restored.
*
* Reentrant:
*  No.
*
*******************************************************************************/
void RFID_Wakeup(void)
{
    RFID_RestoreConfig();
    #if( (RFID_RX_ENABLED) || (RFID_HD_ENABLED) )
        RFID_ClearRxBuffer();
    #endif /* End (RFID_RX_ENABLED) || (RFID_HD_ENABLED) */
    #if(RFID_TX_ENABLED || RFID_HD_ENABLED)
        RFID_ClearTxBuffer();
    #endif /* End RFID_TX_ENABLED || RFID_HD_ENABLED */

    if(RFID_backup.enableState != 0u)
    {
        RFID_Enable();
    }
}


/* [] END OF FILE */
