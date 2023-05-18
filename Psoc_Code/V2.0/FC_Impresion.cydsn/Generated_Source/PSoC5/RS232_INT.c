/*******************************************************************************
* File Name: RS232INT.c
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

#include "RS232.h"
#include "cyapicallbacks.h"


/***************************************
* Custom Declarations
***************************************/
/* `#START CUSTOM_DECLARATIONS` Place your declaration here */

/* `#END` */

#if (RS232_RX_INTERRUPT_ENABLED && (RS232_RX_ENABLED || RS232_HD_ENABLED))
    /*******************************************************************************
    * Function Name: RS232_RXISR
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
    *  RS232_rxBuffer - RAM buffer pointer for save received data.
    *  RS232_rxBufferWrite - cyclic index for write to rxBuffer,
    *     increments after each byte saved to buffer.
    *  RS232_rxBufferRead - cyclic index for read from rxBuffer,
    *     checked to detect overflow condition.
    *  RS232_rxBufferOverflow - software overflow flag. Set to one
    *     when RS232_rxBufferWrite index overtakes
    *     RS232_rxBufferRead index.
    *  RS232_rxBufferLoopDetect - additional variable to detect overflow.
    *     Set to one when RS232_rxBufferWrite is equal to
    *    RS232_rxBufferRead
    *  RS232_rxAddressMode - this variable contains the Address mode,
    *     selected in customizer or set by UART_SetRxAddressMode() API.
    *  RS232_rxAddressDetected - set to 1 when correct address received,
    *     and analysed to store following addressed data bytes to the buffer.
    *     When not correct address received, set to 0 to skip following data bytes.
    *
    *******************************************************************************/
    CY_ISR(RS232_RXISR)
    {
        uint8 readData;
        uint8 readStatus;
        uint8 increment_pointer = 0u;

    #if(CY_PSOC3)
        uint8 int_en;
    #endif /* (CY_PSOC3) */

    #ifdef RS232_RXISR_ENTRY_CALLBACK
        RS232_RXISR_EntryCallback();
    #endif /* RS232_RXISR_ENTRY_CALLBACK */

        /* User code required at start of ISR */
        /* `#START RS232_RXISR_START` */

        /* `#END` */

    #if(CY_PSOC3)   /* Make sure nested interrupt is enabled */
        int_en = EA;
        CyGlobalIntEnable;
    #endif /* (CY_PSOC3) */

        do
        {
            /* Read receiver status register */
            readStatus = RS232_RXSTATUS_REG;
            /* Copy the same status to readData variable for backward compatibility support 
            *  of the user code in RS232_RXISR_ERROR` section. 
            */
            readData = readStatus;

            if((readStatus & (RS232_RX_STS_BREAK | 
                            RS232_RX_STS_PAR_ERROR |
                            RS232_RX_STS_STOP_ERROR | 
                            RS232_RX_STS_OVERRUN)) != 0u)
            {
                /* ERROR handling. */
                RS232_errorStatus |= readStatus & ( RS232_RX_STS_BREAK | 
                                                            RS232_RX_STS_PAR_ERROR | 
                                                            RS232_RX_STS_STOP_ERROR | 
                                                            RS232_RX_STS_OVERRUN);
                /* `#START RS232_RXISR_ERROR` */

                /* `#END` */
                
            #ifdef RS232_RXISR_ERROR_CALLBACK
                RS232_RXISR_ERROR_Callback();
            #endif /* RS232_RXISR_ERROR_CALLBACK */
            }
            
            if((readStatus & RS232_RX_STS_FIFO_NOTEMPTY) != 0u)
            {
                /* Read data from the RX data register */
                readData = RS232_RXDATA_REG;
            #if (RS232_RXHW_ADDRESS_ENABLED)
                if(RS232_rxAddressMode == (uint8)RS232__B_UART__AM_SW_DETECT_TO_BUFFER)
                {
                    if((readStatus & RS232_RX_STS_MRKSPC) != 0u)
                    {
                        if ((readStatus & RS232_RX_STS_ADDR_MATCH) != 0u)
                        {
                            RS232_rxAddressDetected = 1u;
                        }
                        else
                        {
                            RS232_rxAddressDetected = 0u;
                        }
                    }
                    if(RS232_rxAddressDetected != 0u)
                    {   /* Store only addressed data */
                        RS232_rxBuffer[RS232_rxBufferWrite] = readData;
                        increment_pointer = 1u;
                    }
                }
                else /* Without software addressing */
                {
                    RS232_rxBuffer[RS232_rxBufferWrite] = readData;
                    increment_pointer = 1u;
                }
            #else  /* Without addressing */
                RS232_rxBuffer[RS232_rxBufferWrite] = readData;
                increment_pointer = 1u;
            #endif /* (RS232_RXHW_ADDRESS_ENABLED) */

                /* Do not increment buffer pointer when skip not addressed data */
                if(increment_pointer != 0u)
                {
                    if(RS232_rxBufferLoopDetect != 0u)
                    {   /* Set Software Buffer status Overflow */
                        RS232_rxBufferOverflow = 1u;
                    }
                    /* Set next pointer. */
                    RS232_rxBufferWrite++;

                    /* Check pointer for a loop condition */
                    if(RS232_rxBufferWrite >= RS232_RX_BUFFER_SIZE)
                    {
                        RS232_rxBufferWrite = 0u;
                    }

                    /* Detect pre-overload condition and set flag */
                    if(RS232_rxBufferWrite == RS232_rxBufferRead)
                    {
                        RS232_rxBufferLoopDetect = 1u;
                        /* When Hardware Flow Control selected */
                        #if (RS232_FLOW_CONTROL != 0u)
                            /* Disable RX interrupt mask, it is enabled when user read data from the buffer using APIs */
                            RS232_RXSTATUS_MASK_REG  &= (uint8)~RS232_RX_STS_FIFO_NOTEMPTY;
                            CyIntClearPending(RS232_RX_VECT_NUM);
                            break; /* Break the reading of the FIFO loop, leave the data there for generating RTS signal */
                        #endif /* (RS232_FLOW_CONTROL != 0u) */
                    }
                }
            }
        }while((readStatus & RS232_RX_STS_FIFO_NOTEMPTY) != 0u);

        /* User code required at end of ISR (Optional) */
        /* `#START RS232_RXISR_END` */

        /* `#END` */

    #ifdef RS232_RXISR_EXIT_CALLBACK
        RS232_RXISR_ExitCallback();
    #endif /* RS232_RXISR_EXIT_CALLBACK */

    #if(CY_PSOC3)
        EA = int_en;
    #endif /* (CY_PSOC3) */
    }
    
#endif /* (RS232_RX_INTERRUPT_ENABLED && (RS232_RX_ENABLED || RS232_HD_ENABLED)) */


#if (RS232_TX_INTERRUPT_ENABLED && RS232_TX_ENABLED)
    /*******************************************************************************
    * Function Name: RS232_TXISR
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
    *  RS232_txBuffer - RAM buffer pointer for transmit data from.
    *  RS232_txBufferRead - cyclic index for read and transmit data
    *     from txBuffer, increments after each transmitted byte.
    *  RS232_rxBufferWrite - cyclic index for write to txBuffer,
    *     checked to detect available for transmission bytes.
    *
    *******************************************************************************/
    CY_ISR(RS232_TXISR)
    {
    #if(CY_PSOC3)
        uint8 int_en;
    #endif /* (CY_PSOC3) */

    #ifdef RS232_TXISR_ENTRY_CALLBACK
        RS232_TXISR_EntryCallback();
    #endif /* RS232_TXISR_ENTRY_CALLBACK */

        /* User code required at start of ISR */
        /* `#START RS232_TXISR_START` */

        /* `#END` */

    #if(CY_PSOC3)   /* Make sure nested interrupt is enabled */
        int_en = EA;
        CyGlobalIntEnable;
    #endif /* (CY_PSOC3) */

        while((RS232_txBufferRead != RS232_txBufferWrite) &&
             ((RS232_TXSTATUS_REG & RS232_TX_STS_FIFO_FULL) == 0u))
        {
            /* Check pointer wrap around */
            if(RS232_txBufferRead >= RS232_TX_BUFFER_SIZE)
            {
                RS232_txBufferRead = 0u;
            }

            RS232_TXDATA_REG = RS232_txBuffer[RS232_txBufferRead];

            /* Set next pointer */
            RS232_txBufferRead++;
        }

        /* User code required at end of ISR (Optional) */
        /* `#START RS232_TXISR_END` */

        /* `#END` */

    #ifdef RS232_TXISR_EXIT_CALLBACK
        RS232_TXISR_ExitCallback();
    #endif /* RS232_TXISR_EXIT_CALLBACK */

    #if(CY_PSOC3)
        EA = int_en;
    #endif /* (CY_PSOC3) */
   }
#endif /* (RS232_TX_INTERRUPT_ENABLED && RS232_TX_ENABLED) */


/* [] END OF FILE */
