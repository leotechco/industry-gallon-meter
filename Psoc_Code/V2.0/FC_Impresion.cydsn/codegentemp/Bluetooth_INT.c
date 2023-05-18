/*******************************************************************************
* File Name: BluetoothINT.c
* Version 2.50
*
* Description:
*  This file provides all Interrupt Service functionality of the UART component
*
********************************************************************************
* Copyright 2008-2015, Cypress Semiconductor Corporation.  All rights reserved.
* You may use this file only in accordance with the license, terms, conditions,
* disclaimers, and limitations in the end user license agreement accompanying
* the software package with which this file was provided.
*******************************************************************************/

#include "Bluetooth.h"
#include "cyapicallbacks.h"


/***************************************
* Custom Declarations
***************************************/
/* `#START CUSTOM_DECLARATIONS` Place your declaration here */

/* `#END` */

#if (Bluetooth_RX_INTERRUPT_ENABLED && (Bluetooth_RX_ENABLED || Bluetooth_HD_ENABLED))
    /*******************************************************************************
    * Function Name: Bluetooth_RXISR
    ********************************************************************************
    *
    * Summary:
    *  Interrupt Service Routine for RX portion of the UART
    *
    * Parameters:
    *  None.
    *
    * Return:
    *  None.
    *
    * Global Variables:
    *  Bluetooth_rxBuffer - RAM buffer pointer for save received data.
    *  Bluetooth_rxBufferWrite - cyclic index for write to rxBuffer,
    *     increments after each byte saved to buffer.
    *  Bluetooth_rxBufferRead - cyclic index for read from rxBuffer,
    *     checked to detect overflow condition.
    *  Bluetooth_rxBufferOverflow - software overflow flag. Set to one
    *     when Bluetooth_rxBufferWrite index overtakes
    *     Bluetooth_rxBufferRead index.
    *  Bluetooth_rxBufferLoopDetect - additional variable to detect overflow.
    *     Set to one when Bluetooth_rxBufferWrite is equal to
    *    Bluetooth_rxBufferRead
    *  Bluetooth_rxAddressMode - this variable contains the Address mode,
    *     selected in customizer or set by UART_SetRxAddressMode() API.
    *  Bluetooth_rxAddressDetected - set to 1 when correct address received,
    *     and analysed to store following addressed data bytes to the buffer.
    *     When not correct address received, set to 0 to skip following data bytes.
    *
    *******************************************************************************/
    CY_ISR(Bluetooth_RXISR)
    {
        uint8 readData;
        uint8 readStatus;
        uint8 increment_pointer = 0u;

    #if(CY_PSOC3)
        uint8 int_en;
    #endif /* (CY_PSOC3) */

    #ifdef Bluetooth_RXISR_ENTRY_CALLBACK
        Bluetooth_RXISR_EntryCallback();
    #endif /* Bluetooth_RXISR_ENTRY_CALLBACK */

        /* User code required at start of ISR */
        /* `#START Bluetooth_RXISR_START` */

        /* `#END` */

    #if(CY_PSOC3)   /* Make sure nested interrupt is enabled */
        int_en = EA;
        CyGlobalIntEnable;
    #endif /* (CY_PSOC3) */

        do
        {
            /* Read receiver status register */
            readStatus = Bluetooth_RXSTATUS_REG;
            /* Copy the same status to readData variable for backward compatibility support 
            *  of the user code in Bluetooth_RXISR_ERROR` section. 
            */
            readData = readStatus;

            if((readStatus & (Bluetooth_RX_STS_BREAK | 
                            Bluetooth_RX_STS_PAR_ERROR |
                            Bluetooth_RX_STS_STOP_ERROR | 
                            Bluetooth_RX_STS_OVERRUN)) != 0u)
            {
                /* ERROR handling. */
                Bluetooth_errorStatus |= readStatus & ( Bluetooth_RX_STS_BREAK | 
                                                            Bluetooth_RX_STS_PAR_ERROR | 
                                                            Bluetooth_RX_STS_STOP_ERROR | 
                                                            Bluetooth_RX_STS_OVERRUN);
                /* `#START Bluetooth_RXISR_ERROR` */

                /* `#END` */
                
            #ifdef Bluetooth_RXISR_ERROR_CALLBACK
                Bluetooth_RXISR_ERROR_Callback();
            #endif /* Bluetooth_RXISR_ERROR_CALLBACK */
            }
            
            if((readStatus & Bluetooth_RX_STS_FIFO_NOTEMPTY) != 0u)
            {
                /* Read data from the RX data register */
                readData = Bluetooth_RXDATA_REG;
            #if (Bluetooth_RXHW_ADDRESS_ENABLED)
                if(Bluetooth_rxAddressMode == (uint8)Bluetooth__B_UART__AM_SW_DETECT_TO_BUFFER)
                {
                    if((readStatus & Bluetooth_RX_STS_MRKSPC) != 0u)
                    {
                        if ((readStatus & Bluetooth_RX_STS_ADDR_MATCH) != 0u)
                        {
                            Bluetooth_rxAddressDetected = 1u;
                        }
                        else
                        {
                            Bluetooth_rxAddressDetected = 0u;
                        }
                    }
                    if(Bluetooth_rxAddressDetected != 0u)
                    {   /* Store only addressed data */
                        Bluetooth_rxBuffer[Bluetooth_rxBufferWrite] = readData;
                        increment_pointer = 1u;
                    }
                }
                else /* Without software addressing */
                {
                    Bluetooth_rxBuffer[Bluetooth_rxBufferWrite] = readData;
                    increment_pointer = 1u;
                }
            #else  /* Without addressing */
                Bluetooth_rxBuffer[Bluetooth_rxBufferWrite] = readData;
                increment_pointer = 1u;
            #endif /* (Bluetooth_RXHW_ADDRESS_ENABLED) */

                /* Do not increment buffer pointer when skip not addressed data */
                if(increment_pointer != 0u)
                {
                    if(Bluetooth_rxBufferLoopDetect != 0u)
                    {   /* Set Software Buffer status Overflow */
                        Bluetooth_rxBufferOverflow = 1u;
                    }
                    /* Set next pointer. */
                    Bluetooth_rxBufferWrite++;

                    /* Check pointer for a loop condition */
                    if(Bluetooth_rxBufferWrite >= Bluetooth_RX_BUFFER_SIZE)
                    {
                        Bluetooth_rxBufferWrite = 0u;
                    }

                    /* Detect pre-overload condition and set flag */
                    if(Bluetooth_rxBufferWrite == Bluetooth_rxBufferRead)
                    {
                        Bluetooth_rxBufferLoopDetect = 1u;
                        /* When Hardware Flow Control selected */
                        #if (Bluetooth_FLOW_CONTROL != 0u)
                            /* Disable RX interrupt mask, it is enabled when user read data from the buffer using APIs */
                            Bluetooth_RXSTATUS_MASK_REG  &= (uint8)~Bluetooth_RX_STS_FIFO_NOTEMPTY;
                            CyIntClearPending(Bluetooth_RX_VECT_NUM);
                            break; /* Break the reading of the FIFO loop, leave the data there for generating RTS signal */
                        #endif /* (Bluetooth_FLOW_CONTROL != 0u) */
                    }
                }
            }
        }while((readStatus & Bluetooth_RX_STS_FIFO_NOTEMPTY) != 0u);

        /* User code required at end of ISR (Optional) */
        /* `#START Bluetooth_RXISR_END` */

        /* `#END` */

    #ifdef Bluetooth_RXISR_EXIT_CALLBACK
        Bluetooth_RXISR_ExitCallback();
    #endif /* Bluetooth_RXISR_EXIT_CALLBACK */

    #if(CY_PSOC3)
        EA = int_en;
    #endif /* (CY_PSOC3) */
    }
    
#endif /* (Bluetooth_RX_INTERRUPT_ENABLED && (Bluetooth_RX_ENABLED || Bluetooth_HD_ENABLED)) */


#if (Bluetooth_TX_INTERRUPT_ENABLED && Bluetooth_TX_ENABLED)
    /*******************************************************************************
    * Function Name: Bluetooth_TXISR
    ********************************************************************************
    *
    * Summary:
    * Interrupt Service Routine for the TX portion of the UART
    *
    * Parameters:
    *  None.
    *
    * Return:
    *  None.
    *
    * Global Variables:
    *  Bluetooth_txBuffer - RAM buffer pointer for transmit data from.
    *  Bluetooth_txBufferRead - cyclic index for read and transmit data
    *     from txBuffer, increments after each transmitted byte.
    *  Bluetooth_rxBufferWrite - cyclic index for write to txBuffer,
    *     checked to detect available for transmission bytes.
    *
    *******************************************************************************/
    CY_ISR(Bluetooth_TXISR)
    {
    #if(CY_PSOC3)
        uint8 int_en;
    #endif /* (CY_PSOC3) */

    #ifdef Bluetooth_TXISR_ENTRY_CALLBACK
        Bluetooth_TXISR_EntryCallback();
    #endif /* Bluetooth_TXISR_ENTRY_CALLBACK */

        /* User code required at start of ISR */
        /* `#START Bluetooth_TXISR_START` */

        /* `#END` */

    #if(CY_PSOC3)   /* Make sure nested interrupt is enabled */
        int_en = EA;
        CyGlobalIntEnable;
    #endif /* (CY_PSOC3) */

        while((Bluetooth_txBufferRead != Bluetooth_txBufferWrite) &&
             ((Bluetooth_TXSTATUS_REG & Bluetooth_TX_STS_FIFO_FULL) == 0u))
        {
            /* Check pointer wrap around */
            if(Bluetooth_txBufferRead >= Bluetooth_TX_BUFFER_SIZE)
            {
                Bluetooth_txBufferRead = 0u;
            }

            Bluetooth_TXDATA_REG = Bluetooth_txBuffer[Bluetooth_txBufferRead];

            /* Set next pointer */
            Bluetooth_txBufferRead++;
        }

        /* User code required at end of ISR (Optional) */
        /* `#START Bluetooth_TXISR_END` */

        /* `#END` */

    #ifdef Bluetooth_TXISR_EXIT_CALLBACK
        Bluetooth_TXISR_ExitCallback();
    #endif /* Bluetooth_TXISR_EXIT_CALLBACK */

    #if(CY_PSOC3)
        EA = int_en;
    #endif /* (CY_PSOC3) */
   }
#endif /* (Bluetooth_TX_INTERRUPT_ENABLED && Bluetooth_TX_ENABLED) */


/* [] END OF FILE */
