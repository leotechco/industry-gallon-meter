/*******************************************************************************
* File Name: Bluetooth_PM.c
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

#include "Bluetooth.h"


/***************************************
* Local data allocation
***************************************/

static Bluetooth_BACKUP_STRUCT  Bluetooth_backup =
{
    /* enableState - disabled */
    0u,
};



/*******************************************************************************
* Function Name: Bluetooth_SaveConfig
********************************************************************************
*
* Summary:
*  This function saves the component nonretention control register.
*  Does not save the FIFO which is a set of nonretention registers.
*  This function is called by the Bluetooth_Sleep() function.
*
* Parameters:
*  None.
*
* Return:
*  None.
*
* Global Variables:
*  Bluetooth_backup - modified when non-retention registers are saved.
*
* Reentrant:
*  No.
*
*******************************************************************************/
void Bluetooth_SaveConfig(void)
{
    #if(Bluetooth_CONTROL_REG_REMOVED == 0u)
        Bluetooth_backup.cr = Bluetooth_CONTROL_REG;
    #endif /* End Bluetooth_CONTROL_REG_REMOVED */
}


/*******************************************************************************
* Function Name: Bluetooth_RestoreConfig
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
*  Bluetooth_backup - used when non-retention registers are restored.
*
* Reentrant:
*  No.
*
* Notes:
*  If this function is called without calling Bluetooth_SaveConfig() 
*  first, the data loaded may be incorrect.
*
*******************************************************************************/
void Bluetooth_RestoreConfig(void)
{
    #if(Bluetooth_CONTROL_REG_REMOVED == 0u)
        Bluetooth_CONTROL_REG = Bluetooth_backup.cr;
    #endif /* End Bluetooth_CONTROL_REG_REMOVED */
}


/*******************************************************************************
* Function Name: Bluetooth_Sleep
********************************************************************************
*
* Summary:
*  This is the preferred API to prepare the component for sleep. 
*  The Bluetooth_Sleep() API saves the current component state. Then it
*  calls the Bluetooth_Stop() function and calls 
*  Bluetooth_SaveConfig() to save the hardware configuration.
*  Call the Bluetooth_Sleep() function before calling the CyPmSleep() 
*  or the CyPmHibernate() function. 
*
* Parameters:
*  None.
*
* Return:
*  None.
*
* Global Variables:
*  Bluetooth_backup - modified when non-retention registers are saved.
*
* Reentrant:
*  No.
*
*******************************************************************************/
void Bluetooth_Sleep(void)
{
    #if(Bluetooth_RX_ENABLED || Bluetooth_HD_ENABLED)
        if((Bluetooth_RXSTATUS_ACTL_REG  & Bluetooth_INT_ENABLE) != 0u)
        {
            Bluetooth_backup.enableState = 1u;
        }
        else
        {
            Bluetooth_backup.enableState = 0u;
        }
    #else
        if((Bluetooth_TXSTATUS_ACTL_REG  & Bluetooth_INT_ENABLE) !=0u)
        {
            Bluetooth_backup.enableState = 1u;
        }
        else
        {
            Bluetooth_backup.enableState = 0u;
        }
    #endif /* End Bluetooth_RX_ENABLED || Bluetooth_HD_ENABLED*/

    Bluetooth_Stop();
    Bluetooth_SaveConfig();
}


/*******************************************************************************
* Function Name: Bluetooth_Wakeup
********************************************************************************
*
* Summary:
*  This is the preferred API to restore the component to the state when 
*  Bluetooth_Sleep() was called. The Bluetooth_Wakeup() function
*  calls the Bluetooth_RestoreConfig() function to restore the 
*  configuration. If the component was enabled before the 
*  Bluetooth_Sleep() function was called, the Bluetooth_Wakeup()
*  function will also re-enable the component.
*
* Parameters:
*  None.
*
* Return:
*  None.
*
* Global Variables:
*  Bluetooth_backup - used when non-retention registers are restored.
*
* Reentrant:
*  No.
*
*******************************************************************************/
void Bluetooth_Wakeup(void)
{
    Bluetooth_RestoreConfig();
    #if( (Bluetooth_RX_ENABLED) || (Bluetooth_HD_ENABLED) )
        Bluetooth_ClearRxBuffer();
    #endif /* End (Bluetooth_RX_ENABLED) || (Bluetooth_HD_ENABLED) */
    #if(Bluetooth_TX_ENABLED || Bluetooth_HD_ENABLED)
        Bluetooth_ClearTxBuffer();
    #endif /* End Bluetooth_TX_ENABLED || Bluetooth_HD_ENABLED */

    if(Bluetooth_backup.enableState != 0u)
    {
        Bluetooth_Enable();
    }
}


/* [] END OF FILE */
