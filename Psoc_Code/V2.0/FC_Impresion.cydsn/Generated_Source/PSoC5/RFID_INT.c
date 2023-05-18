/*******************************************************************************
* File Name: RFIDINT.c
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

#include "RFID.h"
#include "cyapicallbacks.h"


/***************************************
* Custom Declarations
***************************************/
/* `#START CUSTOM_DECLARATIONS` Place your declaration here */

/* `#END` */

#if (RFID_RX_INTERRUPT_ENABLED && (RFID_RX_ENABLED || RFID_HD_ENABLED))
    /*******************************************************************************
    * Function Name: RFID_RXISR
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
    *  RFID_rxBuffer - RAM buffer pointer for save received data.
    *  RFID_rxBufferWrite - cyclic index for write to rxBuffer,
    *     increments after each byte saved to buffer.
    *  RFID_rxBufferRead - cyclic index for read from rxBuffer,
    *     checked to detect overflow condition.
    *  RFID_rxBufferOverflow - software overflow flag. Set to one
    *     when RFID_rxBufferWrite index overtakes
    *     RFID_rxBufferRead index.
    *  RFID_rxBufferLoopDetect - additional variable to detect overflow.
    *     Set to one when RFID_rxBufferWrite is equal to
    *    RFID_rxBufferRead
    *  RFID_rxAddressMode - this variable contains the Address mode,
    *     selected in customizer or set by UART_SetRxAddressMode() API.
    *  RFID_rxAddressDetected - set to 1 when correct address received,
    *     and analysed to store following addressed data bytes to the buffer.
    *     When not correct address received, set to 0 to skip following data bytes.
    *
    *******************************************************************************/
    CY_ISR(RFID_RXISR)
    {
        uint8 readData;
        uint8 readStatus;
        uint8 increment_pointer = 0u;

    #if(CY_PSOC3)
        uint8 int_en;
    #endif /* (CY_PSOC3) */

    #ifdef RFID_RXISR_ENTRY_CALLBACK
        RFID_RXISR_EntryCallback();
    #endif /* RFID_RXISR_ENTRY_CALLBACK */

        /* User code required at start of ISR */
        /* `#START RFID_RXISR_START` */

        /* `#END` */

    #if(CY_PSOC3)   /* Make sure nested interrupt is enabled */
        int_en = EA;
        CyGlobalIntEnable;
    #endif /* (CY_PSOC3) */

        do
        {
            /* Read receiver status register */
            readStatus = RFID_RXSTATUS_REG;
            /* Copy the same status to readData variable for backward compatibility support 
            *  of the user code in RFID_RXISR_ERROR` section. 
            */
            readData = readStatus;

            if((readStatus & (RFID_RX_STS_BREAK | 
                            RFID_RX_STS_PAR_ERROR |
                            RFID_RX_STS_STOP_ERROR | 
                            RFID_RX_STS_OVERRUN)) != 0u)
            {
                /* ERROR handling. */
                RFID_errorStatus |= readStatus & ( RFID_RX_STS_BREAK | 
                                                            RFID_RX_STS_PAR_ERROR | 
                                                            RFID_RX_STS_STOP_ERROR | 
                                                            RFID_RX_STS_OVERRUN);
                /* `#START RFID_RXISR_ERROR` */

                /* `#END` */
                
            #ifdef RFID_RXISR_ERROR_CALLBACK
                RFID_RXISR_ERROR_Callback();
            #endif /* RFID_RXISR_ERROR_CALLBACK */
            }
            
            if((readStatus & RFID_RX_STS_FIFO_NOTEMPTY) != 0u)
            {
                /* Read data from the RX data register */
                readData = RFID_RXDATA_REG;
            #if (RFID_RXHW_ADDRESS_ENABLED)
                if(RFID_rxAddressMode == (uint8)RFID__B_UART__AM_SW_DETECT_TO_BUFFER)
                {
                    if((readStatus & RFID_RX_STS_MRKSPC) != 0u)
                    {
                        if ((readStatus & RFID_RX_STS_ADDR_MATCH) != 0u)
                        {
                            RFID_rxAddressDetected = 1u;
                        }
                        else
                        {
                            RFID_rxAddressDetected = 0u;
                        }
                    }
                    if(RFID_rxAddressDetected != 0u)
                    {   /* Store only addressed data */
                        RFID_rxBuffer[RFID_rxBufferWrite] = readData;
                        increment_pointer = 1u;
                    }
                }
                else /* Without software addressing */
                {
                    RFID_rxBuffer[RFID_rxBufferWrite] = readData;
                    increment_pointer = 1u;
                }
            #else  /* Without addressing */
                RFID_rxBuffer[RFID_rxBufferWrite] = readData;
                increment_pointer = 1u;
            #endif /* (RFID_RXHW_ADDRESS_ENABLED) */

                /* Do not increment buffer pointer when skip not addressed data */
                if(increment_pointer != 0u)
                {
                    if(RFID_rxBufferLoopDetect != 0u)
                    {   /* Set Software Buffer status Overflow */
                        RFID_rxBufferOverflow = 1u;
                    }
                    /* Set next pointer. */
                    RFID_rxBufferWrite++;

                    /* Check pointer for a loop condition */
                    if(RFID_rxBufferWrite >= RFID_RX_BUFFER_SIZE)
                    {
                        RFID_rxBufferWrite = 0u;
                    }

                    /* Detect pre-overload condition and set flag */
                    if(RFID_rxBufferWrite == RFID_rxBufferRead)
                    {
                        RFID_rxBufferLoopDetect = 1u;
                        /* When Hardware Flow Control selected */
                        #if (RFID_FLOW_CONTROL != 0u)
                            /* Disable RX interrupt mask, it is enabled when user read data from the buffer using APIs */
                            RFID_RXSTATUS_MASK_REG  &= (uint8)~RFID_RX_STS_FIFO_NOTEMPTY;
                            CyIntClearPending(RFID_RX_VECT_NUM);
                            break; /* Break the reading of the FIFO loop, leave the data there for generating RTS signal */
                        #endif /* (RFID_FLOW_CONTROL != 0u) */
                    }
                }
            }
        }while((readStatus & RFID_RX_STS_FIFO_NOTEMPTY) != 0u);

        /* User code required at end of ISR (Optional) */
        /* `#START RFID_RXISR_END` */

        /* `#END` */

    #ifdef RFID_RXISR_EXIT_CALLBACK
        RFID_RXISR_ExitCallback();
    #endif /* RFID_RXISR_EXIT_CALLBACK */

    #if(CY_PSOC3)
        EA = int_en;
    #endif /* (CY_PSOC3) */
    }
    
#endif /* (RFID_RX_INTERRUPT_ENABLED && (RFID_RX_ENABLED || RFID_HD_ENABLED)) */


#if (RFID_TX_INTERRUPT_ENABLED && RFID_TX_ENABLED)
    /*******************************************************************************
    * Function Name: RFID_TXISR
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
    *  RFID_txBuffer - RAM buffer pointer for transmit data from.
    *  RFID_txBufferRead - cyclic index for read and transmit data
    *     from txBuffer, increments after each transmitted byte.
    *  RFID_rxBufferWrite - cyclic index for write to txBuffer,
    *     checked to detect available for transmission bytes.
    *
    *******************************************************************************/
    CY_ISR(RFID_TXISR)
    {
    #if(CY_PSOC3)
        uint8 int_en;
    #endif /* (CY_PSOC3) */

    #ifdef RFID_TXISR_ENTRY_CALLBACK
        RFID_TXISR_EntryCallback();
    #endif /* RFID_TXISR_ENTRY_CALLBACK */

        /* User code required at start of ISR */
        /* `#START RFID_TXISR_START` */

        /* `#END` */

    #if(CY_PSOC3)   /* Make sure nested interrupt is enabled */
        int_en = EA;
        CyGlobalIntEnable;
    #endif /* (CY_PSOC3) */

        while((RFID_txBufferRead != RFID_txBufferWrite) &&
             ((RFID_TXSTATUS_REG & RFID_TX_STS_FIFO_FULL) == 0u))
        {
            /* Check pointer wrap around */
            if(RFID_txBufferRead >= RFID_TX_BUFFER_SIZE)
            {
                RFID_txBufferRead = 0u;
            }

            RFID_TXDATA_REG = RFID_txBuffer[RFID_txBufferRead];

            /* Set next pointer */
            RFID_txBufferRead++;
        }

        /* User code required at end of ISR (Optional) */
        /* `#START RFID_TXISR_END` */

        /* `#END` */

    #ifdef RFID_TXISR_EXIT_CALLBACK
        RFID_TXISR_ExitCallback();
    #endif /* RFID_TXISR_EXIT_CALLBACK */

    #if(CY_PSOC3)
        EA = int_en;
    #endif /* (CY_PSOC3) */
   }
#endif /* (RFID_TX_INTERRUPT_ENABLED && RFID_TX_ENABLED) */


/* [] END OF FILE */
