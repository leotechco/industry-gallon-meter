/*******************************************************************************
* File Name: RFID.c
* Version 2.50
*
* Description:
*  This file provides all API functionality of the UART component
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
#if (RFID_INTERNAL_CLOCK_USED)
    #include "RFID_IntClock.h"
#endif /* End RFID_INTERNAL_CLOCK_USED */


/***************************************
* Global data allocation
***************************************/

uint8 RFID_initVar = 0u;

#if (RFID_TX_INTERRUPT_ENABLED && RFID_TX_ENABLED)
    volatile uint8 RFID_txBuffer[RFID_TX_BUFFER_SIZE];
    volatile uint8 RFID_txBufferRead = 0u;
    uint8 RFID_txBufferWrite = 0u;
#endif /* (RFID_TX_INTERRUPT_ENABLED && RFID_TX_ENABLED) */

#if (RFID_RX_INTERRUPT_ENABLED && (RFID_RX_ENABLED || RFID_HD_ENABLED))
    uint8 RFID_errorStatus = 0u;
    volatile uint8 RFID_rxBuffer[RFID_RX_BUFFER_SIZE];
    volatile uint8 RFID_rxBufferRead  = 0u;
    volatile uint8 RFID_rxBufferWrite = 0u;
    volatile uint8 RFID_rxBufferLoopDetect = 0u;
    volatile uint8 RFID_rxBufferOverflow   = 0u;
    #if (RFID_RXHW_ADDRESS_ENABLED)
        volatile uint8 RFID_rxAddressMode = RFID_RX_ADDRESS_MODE;
        volatile uint8 RFID_rxAddressDetected = 0u;
    #endif /* (RFID_RXHW_ADDRESS_ENABLED) */
#endif /* (RFID_RX_INTERRUPT_ENABLED && (RFID_RX_ENABLED || RFID_HD_ENABLED)) */


/*******************************************************************************
* Function Name: RFID_Start
********************************************************************************
*
* Summary:
*  This is the preferred method to begin component operation.
*  RFID_Start() sets the initVar variable, calls the
*  RFID_Init() function, and then calls the
*  RFID_Enable() function.
*
* Parameters:
*  None.
*
* Return:
*  None.
*
* Global variables:
*  The RFID_intiVar variable is used to indicate initial
*  configuration of this component. The variable is initialized to zero (0u)
*  and set to one (1u) the first time RFID_Start() is called. This
*  allows for component initialization without re-initialization in all
*  subsequent calls to the RFID_Start() routine.
*
* Reentrant:
*  No.
*
*******************************************************************************/
void RFID_Start(void) 
{
    /* If not initialized then initialize all required hardware and software */
    if(RFID_initVar == 0u)
    {
        RFID_Init();
        RFID_initVar = 1u;
    }

    RFID_Enable();
}


/*******************************************************************************
* Function Name: RFID_Init
********************************************************************************
*
* Summary:
*  Initializes or restores the component according to the customizer Configure
*  dialog settings. It is not necessary to call RFID_Init() because
*  the RFID_Start() API calls this function and is the preferred
*  method to begin component operation.
*
* Parameters:
*  None.
*
* Return:
*  None.
*
*******************************************************************************/
void RFID_Init(void) 
{
    #if(RFID_RX_ENABLED || RFID_HD_ENABLED)

        #if (RFID_RX_INTERRUPT_ENABLED)
            /* Set RX interrupt vector and priority */
            (void) CyIntSetVector(RFID_RX_VECT_NUM, &RFID_RXISR);
            CyIntSetPriority(RFID_RX_VECT_NUM, RFID_RX_PRIOR_NUM);
            RFID_errorStatus = 0u;
        #endif /* (RFID_RX_INTERRUPT_ENABLED) */

        #if (RFID_RXHW_ADDRESS_ENABLED)
            RFID_SetRxAddressMode(RFID_RX_ADDRESS_MODE);
            RFID_SetRxAddress1(RFID_RX_HW_ADDRESS1);
            RFID_SetRxAddress2(RFID_RX_HW_ADDRESS2);
        #endif /* End RFID_RXHW_ADDRESS_ENABLED */

        /* Init Count7 period */
        RFID_RXBITCTR_PERIOD_REG = RFID_RXBITCTR_INIT;
        /* Configure the Initial RX interrupt mask */
        RFID_RXSTATUS_MASK_REG  = RFID_INIT_RX_INTERRUPTS_MASK;
    #endif /* End RFID_RX_ENABLED || RFID_HD_ENABLED*/

    #if(RFID_TX_ENABLED)
        #if (RFID_TX_INTERRUPT_ENABLED)
            /* Set TX interrupt vector and priority */
            (void) CyIntSetVector(RFID_TX_VECT_NUM, &RFID_TXISR);
            CyIntSetPriority(RFID_TX_VECT_NUM, RFID_TX_PRIOR_NUM);
        #endif /* (RFID_TX_INTERRUPT_ENABLED) */

        /* Write Counter Value for TX Bit Clk Generator*/
        #if (RFID_TXCLKGEN_DP)
            RFID_TXBITCLKGEN_CTR_REG = RFID_BIT_CENTER;
            RFID_TXBITCLKTX_COMPLETE_REG = ((RFID_NUMBER_OF_DATA_BITS +
                        RFID_NUMBER_OF_START_BIT) * RFID_OVER_SAMPLE_COUNT) - 1u;
        #else
            RFID_TXBITCTR_PERIOD_REG = ((RFID_NUMBER_OF_DATA_BITS +
                        RFID_NUMBER_OF_START_BIT) * RFID_OVER_SAMPLE_8) - 1u;
        #endif /* End RFID_TXCLKGEN_DP */

        /* Configure the Initial TX interrupt mask */
        #if (RFID_TX_INTERRUPT_ENABLED)
            RFID_TXSTATUS_MASK_REG = RFID_TX_STS_FIFO_EMPTY;
        #else
            RFID_TXSTATUS_MASK_REG = RFID_INIT_TX_INTERRUPTS_MASK;
        #endif /*End RFID_TX_INTERRUPT_ENABLED*/

    #endif /* End RFID_TX_ENABLED */

    #if(RFID_PARITY_TYPE_SW)  /* Write Parity to Control Register */
        RFID_WriteControlRegister( \
            (RFID_ReadControlRegister() & (uint8)~RFID_CTRL_PARITY_TYPE_MASK) | \
            (uint8)(RFID_PARITY_TYPE << RFID_CTRL_PARITY_TYPE0_SHIFT) );
    #endif /* End RFID_PARITY_TYPE_SW */
}


/*******************************************************************************
* Function Name: RFID_Enable
********************************************************************************
*
* Summary:
*  Activates the hardware and begins component operation. It is not necessary
*  to call RFID_Enable() because the RFID_Start() API
*  calls this function, which is the preferred method to begin component
*  operation.

* Parameters:
*  None.
*
* Return:
*  None.
*
* Global Variables:
*  RFID_rxAddressDetected - set to initial state (0).
*
*******************************************************************************/
void RFID_Enable(void) 
{
    uint8 enableInterrupts;
    enableInterrupts = CyEnterCriticalSection();

    #if (RFID_RX_ENABLED || RFID_HD_ENABLED)
        /* RX Counter (Count7) Enable */
        RFID_RXBITCTR_CONTROL_REG |= RFID_CNTR_ENABLE;

        /* Enable the RX Interrupt */
        RFID_RXSTATUS_ACTL_REG  |= RFID_INT_ENABLE;

        #if (RFID_RX_INTERRUPT_ENABLED)
            RFID_EnableRxInt();

            #if (RFID_RXHW_ADDRESS_ENABLED)
                RFID_rxAddressDetected = 0u;
            #endif /* (RFID_RXHW_ADDRESS_ENABLED) */
        #endif /* (RFID_RX_INTERRUPT_ENABLED) */
    #endif /* (RFID_RX_ENABLED || RFID_HD_ENABLED) */

    #if(RFID_TX_ENABLED)
        /* TX Counter (DP/Count7) Enable */
        #if(!RFID_TXCLKGEN_DP)
            RFID_TXBITCTR_CONTROL_REG |= RFID_CNTR_ENABLE;
        #endif /* End RFID_TXCLKGEN_DP */

        /* Enable the TX Interrupt */
        RFID_TXSTATUS_ACTL_REG |= RFID_INT_ENABLE;
        #if (RFID_TX_INTERRUPT_ENABLED)
            RFID_ClearPendingTxInt(); /* Clear history of TX_NOT_EMPTY */
            RFID_EnableTxInt();
        #endif /* (RFID_TX_INTERRUPT_ENABLED) */
     #endif /* (RFID_TX_INTERRUPT_ENABLED) */

    #if (RFID_INTERNAL_CLOCK_USED)
        RFID_IntClock_Start();  /* Enable the clock */
    #endif /* (RFID_INTERNAL_CLOCK_USED) */

    CyExitCriticalSection(enableInterrupts);
}


/*******************************************************************************
* Function Name: RFID_Stop
********************************************************************************
*
* Summary:
*  Disables the UART operation.
*
* Parameters:
*  None.
*
* Return:
*  None.
*
*******************************************************************************/
void RFID_Stop(void) 
{
    uint8 enableInterrupts;
    enableInterrupts = CyEnterCriticalSection();

    /* Write Bit Counter Disable */
    #if (RFID_RX_ENABLED || RFID_HD_ENABLED)
        RFID_RXBITCTR_CONTROL_REG &= (uint8) ~RFID_CNTR_ENABLE;
    #endif /* (RFID_RX_ENABLED || RFID_HD_ENABLED) */

    #if (RFID_TX_ENABLED)
        #if(!RFID_TXCLKGEN_DP)
            RFID_TXBITCTR_CONTROL_REG &= (uint8) ~RFID_CNTR_ENABLE;
        #endif /* (!RFID_TXCLKGEN_DP) */
    #endif /* (RFID_TX_ENABLED) */

    #if (RFID_INTERNAL_CLOCK_USED)
        RFID_IntClock_Stop();   /* Disable the clock */
    #endif /* (RFID_INTERNAL_CLOCK_USED) */

    /* Disable internal interrupt component */
    #if (RFID_RX_ENABLED || RFID_HD_ENABLED)
        RFID_RXSTATUS_ACTL_REG  &= (uint8) ~RFID_INT_ENABLE;

        #if (RFID_RX_INTERRUPT_ENABLED)
            RFID_DisableRxInt();
        #endif /* (RFID_RX_INTERRUPT_ENABLED) */
    #endif /* (RFID_RX_ENABLED || RFID_HD_ENABLED) */

    #if (RFID_TX_ENABLED)
        RFID_TXSTATUS_ACTL_REG &= (uint8) ~RFID_INT_ENABLE;

        #if (RFID_TX_INTERRUPT_ENABLED)
            RFID_DisableTxInt();
        #endif /* (RFID_TX_INTERRUPT_ENABLED) */
    #endif /* (RFID_TX_ENABLED) */

    CyExitCriticalSection(enableInterrupts);
}


/*******************************************************************************
* Function Name: RFID_ReadControlRegister
********************************************************************************
*
* Summary:
*  Returns the current value of the control register.
*
* Parameters:
*  None.
*
* Return:
*  Contents of the control register.
*
*******************************************************************************/
uint8 RFID_ReadControlRegister(void) 
{
    #if (RFID_CONTROL_REG_REMOVED)
        return(0u);
    #else
        return(RFID_CONTROL_REG);
    #endif /* (RFID_CONTROL_REG_REMOVED) */
}


/*******************************************************************************
* Function Name: RFID_WriteControlRegister
********************************************************************************
*
* Summary:
*  Writes an 8-bit value into the control register
*
* Parameters:
*  control:  control register value
*
* Return:
*  None.
*
*******************************************************************************/
void  RFID_WriteControlRegister(uint8 control) 
{
    #if (RFID_CONTROL_REG_REMOVED)
        if(0u != control)
        {
            /* Suppress compiler warning */
        }
    #else
       RFID_CONTROL_REG = control;
    #endif /* (RFID_CONTROL_REG_REMOVED) */
}


#if(RFID_RX_ENABLED || RFID_HD_ENABLED)
    /*******************************************************************************
    * Function Name: RFID_SetRxInterruptMode
    ********************************************************************************
    *
    * Summary:
    *  Configures the RX interrupt sources enabled.
    *
    * Parameters:
    *  IntSrc:  Bit field containing the RX interrupts to enable. Based on the 
    *  bit-field arrangement of the status register. This value must be a 
    *  combination of status register bit-masks shown below:
    *      RFID_RX_STS_FIFO_NOTEMPTY    Interrupt on byte received.
    *      RFID_RX_STS_PAR_ERROR        Interrupt on parity error.
    *      RFID_RX_STS_STOP_ERROR       Interrupt on stop error.
    *      RFID_RX_STS_BREAK            Interrupt on break.
    *      RFID_RX_STS_OVERRUN          Interrupt on overrun error.
    *      RFID_RX_STS_ADDR_MATCH       Interrupt on address match.
    *      RFID_RX_STS_MRKSPC           Interrupt on address detect.
    *
    * Return:
    *  None.
    *
    * Theory:
    *  Enables the output of specific status bits to the interrupt controller
    *
    *******************************************************************************/
    void RFID_SetRxInterruptMode(uint8 intSrc) 
    {
        RFID_RXSTATUS_MASK_REG  = intSrc;
    }


    /*******************************************************************************
    * Function Name: RFID_ReadRxData
    ********************************************************************************
    *
    * Summary:
    *  Returns the next byte of received data. This function returns data without
    *  checking the status. You must check the status separately.
    *
    * Parameters:
    *  None.
    *
    * Return:
    *  Received data from RX register
    *
    * Global Variables:
    *  RFID_rxBuffer - RAM buffer pointer for save received data.
    *  RFID_rxBufferWrite - cyclic index for write to rxBuffer,
    *     checked to identify new data.
    *  RFID_rxBufferRead - cyclic index for read from rxBuffer,
    *     incremented after each byte has been read from buffer.
    *  RFID_rxBufferLoopDetect - cleared if loop condition was detected
    *     in RX ISR.
    *
    * Reentrant:
    *  No.
    *
    *******************************************************************************/
    uint8 RFID_ReadRxData(void) 
    {
        uint8 rxData;

    #if (RFID_RX_INTERRUPT_ENABLED)

        uint8 locRxBufferRead;
        uint8 locRxBufferWrite;

        /* Protect variables that could change on interrupt */
        RFID_DisableRxInt();

        locRxBufferRead  = RFID_rxBufferRead;
        locRxBufferWrite = RFID_rxBufferWrite;

        if( (RFID_rxBufferLoopDetect != 0u) || (locRxBufferRead != locRxBufferWrite) )
        {
            rxData = RFID_rxBuffer[locRxBufferRead];
            locRxBufferRead++;

            if(locRxBufferRead >= RFID_RX_BUFFER_SIZE)
            {
                locRxBufferRead = 0u;
            }
            /* Update the real pointer */
            RFID_rxBufferRead = locRxBufferRead;

            if(RFID_rxBufferLoopDetect != 0u)
            {
                RFID_rxBufferLoopDetect = 0u;
                #if ((RFID_RX_INTERRUPT_ENABLED) && (RFID_FLOW_CONTROL != 0u))
                    /* When Hardware Flow Control selected - return RX mask */
                    #if( RFID_HD_ENABLED )
                        if((RFID_CONTROL_REG & RFID_CTRL_HD_SEND) == 0u)
                        {   /* In Half duplex mode return RX mask only in RX
                            *  configuration set, otherwise
                            *  mask will be returned in LoadRxConfig() API.
                            */
                            RFID_RXSTATUS_MASK_REG  |= RFID_RX_STS_FIFO_NOTEMPTY;
                        }
                    #else
                        RFID_RXSTATUS_MASK_REG  |= RFID_RX_STS_FIFO_NOTEMPTY;
                    #endif /* end RFID_HD_ENABLED */
                #endif /* ((RFID_RX_INTERRUPT_ENABLED) && (RFID_FLOW_CONTROL != 0u)) */
            }
        }
        else
        {   /* Needs to check status for RX_STS_FIFO_NOTEMPTY bit */
            rxData = RFID_RXDATA_REG;
        }

        RFID_EnableRxInt();

    #else

        /* Needs to check status for RX_STS_FIFO_NOTEMPTY bit */
        rxData = RFID_RXDATA_REG;

    #endif /* (RFID_RX_INTERRUPT_ENABLED) */

        return(rxData);
    }


    /*******************************************************************************
    * Function Name: RFID_ReadRxStatus
    ********************************************************************************
    *
    * Summary:
    *  Returns the current state of the receiver status register and the software
    *  buffer overflow status.
    *
    * Parameters:
    *  None.
    *
    * Return:
    *  Current state of the status register.
    *
    * Side Effect:
    *  All status register bits are clear-on-read except
    *  RFID_RX_STS_FIFO_NOTEMPTY.
    *  RFID_RX_STS_FIFO_NOTEMPTY clears immediately after RX data
    *  register read.
    *
    * Global Variables:
    *  RFID_rxBufferOverflow - used to indicate overload condition.
    *   It set to one in RX interrupt when there isn't free space in
    *   RFID_rxBufferRead to write new data. This condition returned
    *   and cleared to zero by this API as an
    *   RFID_RX_STS_SOFT_BUFF_OVER bit along with RX Status register
    *   bits.
    *
    *******************************************************************************/
    uint8 RFID_ReadRxStatus(void) 
    {
        uint8 status;

        status = RFID_RXSTATUS_REG & RFID_RX_HW_MASK;

    #if (RFID_RX_INTERRUPT_ENABLED)
        if(RFID_rxBufferOverflow != 0u)
        {
            status |= RFID_RX_STS_SOFT_BUFF_OVER;
            RFID_rxBufferOverflow = 0u;
        }
    #endif /* (RFID_RX_INTERRUPT_ENABLED) */

        return(status);
    }


    /*******************************************************************************
    * Function Name: RFID_GetChar
    ********************************************************************************
    *
    * Summary:
    *  Returns the last received byte of data. RFID_GetChar() is
    *  designed for ASCII characters and returns a uint8 where 1 to 255 are values
    *  for valid characters and 0 indicates an error occurred or no data is present.
    *
    * Parameters:
    *  None.
    *
    * Return:
    *  Character read from UART RX buffer. ASCII characters from 1 to 255 are valid.
    *  A returned zero signifies an error condition or no data available.
    *
    * Global Variables:
    *  RFID_rxBuffer - RAM buffer pointer for save received data.
    *  RFID_rxBufferWrite - cyclic index for write to rxBuffer,
    *     checked to identify new data.
    *  RFID_rxBufferRead - cyclic index for read from rxBuffer,
    *     incremented after each byte has been read from buffer.
    *  RFID_rxBufferLoopDetect - cleared if loop condition was detected
    *     in RX ISR.
    *
    * Reentrant:
    *  No.
    *
    *******************************************************************************/
    uint8 RFID_GetChar(void) 
    {
        uint8 rxData = 0u;
        uint8 rxStatus;

    #if (RFID_RX_INTERRUPT_ENABLED)
        uint8 locRxBufferRead;
        uint8 locRxBufferWrite;

        /* Protect variables that could change on interrupt */
        RFID_DisableRxInt();

        locRxBufferRead  = RFID_rxBufferRead;
        locRxBufferWrite = RFID_rxBufferWrite;

        if( (RFID_rxBufferLoopDetect != 0u) || (locRxBufferRead != locRxBufferWrite) )
        {
            rxData = RFID_rxBuffer[locRxBufferRead];
            locRxBufferRead++;
            if(locRxBufferRead >= RFID_RX_BUFFER_SIZE)
            {
                locRxBufferRead = 0u;
            }
            /* Update the real pointer */
            RFID_rxBufferRead = locRxBufferRead;

            if(RFID_rxBufferLoopDetect != 0u)
            {
                RFID_rxBufferLoopDetect = 0u;
                #if( (RFID_RX_INTERRUPT_ENABLED) && (RFID_FLOW_CONTROL != 0u) )
                    /* When Hardware Flow Control selected - return RX mask */
                    #if( RFID_HD_ENABLED )
                        if((RFID_CONTROL_REG & RFID_CTRL_HD_SEND) == 0u)
                        {   /* In Half duplex mode return RX mask only if
                            *  RX configuration set, otherwise
                            *  mask will be returned in LoadRxConfig() API.
                            */
                            RFID_RXSTATUS_MASK_REG |= RFID_RX_STS_FIFO_NOTEMPTY;
                        }
                    #else
                        RFID_RXSTATUS_MASK_REG |= RFID_RX_STS_FIFO_NOTEMPTY;
                    #endif /* end RFID_HD_ENABLED */
                #endif /* RFID_RX_INTERRUPT_ENABLED and Hardware flow control*/
            }

        }
        else
        {   rxStatus = RFID_RXSTATUS_REG;
            if((rxStatus & RFID_RX_STS_FIFO_NOTEMPTY) != 0u)
            {   /* Read received data from FIFO */
                rxData = RFID_RXDATA_REG;
                /*Check status on error*/
                if((rxStatus & (RFID_RX_STS_BREAK | RFID_RX_STS_PAR_ERROR |
                                RFID_RX_STS_STOP_ERROR | RFID_RX_STS_OVERRUN)) != 0u)
                {
                    rxData = 0u;
                }
            }
        }

        RFID_EnableRxInt();

    #else

        rxStatus =RFID_RXSTATUS_REG;
        if((rxStatus & RFID_RX_STS_FIFO_NOTEMPTY) != 0u)
        {
            /* Read received data from FIFO */
            rxData = RFID_RXDATA_REG;

            /*Check status on error*/
            if((rxStatus & (RFID_RX_STS_BREAK | RFID_RX_STS_PAR_ERROR |
                            RFID_RX_STS_STOP_ERROR | RFID_RX_STS_OVERRUN)) != 0u)
            {
                rxData = 0u;
            }
        }
    #endif /* (RFID_RX_INTERRUPT_ENABLED) */

        return(rxData);
    }


    /*******************************************************************************
    * Function Name: RFID_GetByte
    ********************************************************************************
    *
    * Summary:
    *  Reads UART RX buffer immediately, returns received character and error
    *  condition.
    *
    * Parameters:
    *  None.
    *
    * Return:
    *  MSB contains status and LSB contains UART RX data. If the MSB is nonzero,
    *  an error has occurred.
    *
    * Reentrant:
    *  No.
    *
    *******************************************************************************/
    uint16 RFID_GetByte(void) 
    {
        
    #if (RFID_RX_INTERRUPT_ENABLED)
        uint16 locErrorStatus;
        /* Protect variables that could change on interrupt */
        RFID_DisableRxInt();
        locErrorStatus = (uint16)RFID_errorStatus;
        RFID_errorStatus = 0u;
        RFID_EnableRxInt();
        return ( (uint16)(locErrorStatus << 8u) | RFID_ReadRxData() );
    #else
        return ( ((uint16)RFID_ReadRxStatus() << 8u) | RFID_ReadRxData() );
    #endif /* RFID_RX_INTERRUPT_ENABLED */
        
    }


    /*******************************************************************************
    * Function Name: RFID_GetRxBufferSize
    ********************************************************************************
    *
    * Summary:
    *  Returns the number of received bytes available in the RX buffer.
    *  * RX software buffer is disabled (RX Buffer Size parameter is equal to 4): 
    *    returns 0 for empty RX FIFO or 1 for not empty RX FIFO.
    *  * RX software buffer is enabled: returns the number of bytes available in 
    *    the RX software buffer. Bytes available in the RX FIFO do not take to 
    *    account.
    *
    * Parameters:
    *  None.
    *
    * Return:
    *  uint8: Number of bytes in the RX buffer. 
    *    Return value type depends on RX Buffer Size parameter.
    *
    * Global Variables:
    *  RFID_rxBufferWrite - used to calculate left bytes.
    *  RFID_rxBufferRead - used to calculate left bytes.
    *  RFID_rxBufferLoopDetect - checked to decide left bytes amount.
    *
    * Reentrant:
    *  No.
    *
    * Theory:
    *  Allows the user to find out how full the RX Buffer is.
    *
    *******************************************************************************/
    uint8 RFID_GetRxBufferSize(void)
                                                            
    {
        uint8 size;

    #if (RFID_RX_INTERRUPT_ENABLED)

        /* Protect variables that could change on interrupt */
        RFID_DisableRxInt();

        if(RFID_rxBufferRead == RFID_rxBufferWrite)
        {
            if(RFID_rxBufferLoopDetect != 0u)
            {
                size = RFID_RX_BUFFER_SIZE;
            }
            else
            {
                size = 0u;
            }
        }
        else if(RFID_rxBufferRead < RFID_rxBufferWrite)
        {
            size = (RFID_rxBufferWrite - RFID_rxBufferRead);
        }
        else
        {
            size = (RFID_RX_BUFFER_SIZE - RFID_rxBufferRead) + RFID_rxBufferWrite;
        }

        RFID_EnableRxInt();

    #else

        /* We can only know if there is data in the fifo. */
        size = ((RFID_RXSTATUS_REG & RFID_RX_STS_FIFO_NOTEMPTY) != 0u) ? 1u : 0u;

    #endif /* (RFID_RX_INTERRUPT_ENABLED) */

        return(size);
    }


    /*******************************************************************************
    * Function Name: RFID_ClearRxBuffer
    ********************************************************************************
    *
    * Summary:
    *  Clears the receiver memory buffer and hardware RX FIFO of all received data.
    *
    * Parameters:
    *  None.
    *
    * Return:
    *  None.
    *
    * Global Variables:
    *  RFID_rxBufferWrite - cleared to zero.
    *  RFID_rxBufferRead - cleared to zero.
    *  RFID_rxBufferLoopDetect - cleared to zero.
    *  RFID_rxBufferOverflow - cleared to zero.
    *
    * Reentrant:
    *  No.
    *
    * Theory:
    *  Setting the pointers to zero makes the system believe there is no data to
    *  read and writing will resume at address 0 overwriting any data that may
    *  have remained in the RAM.
    *
    * Side Effects:
    *  Any received data not read from the RAM or FIFO buffer will be lost.
    *
    *******************************************************************************/
    void RFID_ClearRxBuffer(void) 
    {
        uint8 enableInterrupts;

        /* Clear the HW FIFO */
        enableInterrupts = CyEnterCriticalSection();
        RFID_RXDATA_AUX_CTL_REG |= (uint8)  RFID_RX_FIFO_CLR;
        RFID_RXDATA_AUX_CTL_REG &= (uint8) ~RFID_RX_FIFO_CLR;
        CyExitCriticalSection(enableInterrupts);

    #if (RFID_RX_INTERRUPT_ENABLED)

        /* Protect variables that could change on interrupt. */
        RFID_DisableRxInt();

        RFID_rxBufferRead = 0u;
        RFID_rxBufferWrite = 0u;
        RFID_rxBufferLoopDetect = 0u;
        RFID_rxBufferOverflow = 0u;

        RFID_EnableRxInt();

    #endif /* (RFID_RX_INTERRUPT_ENABLED) */

    }


    /*******************************************************************************
    * Function Name: RFID_SetRxAddressMode
    ********************************************************************************
    *
    * Summary:
    *  Sets the software controlled Addressing mode used by the RX portion of the
    *  UART.
    *
    * Parameters:
    *  addressMode: Enumerated value indicating the mode of RX addressing
    *  RFID__B_UART__AM_SW_BYTE_BYTE -  Software Byte-by-Byte address
    *                                               detection
    *  RFID__B_UART__AM_SW_DETECT_TO_BUFFER - Software Detect to Buffer
    *                                               address detection
    *  RFID__B_UART__AM_HW_BYTE_BY_BYTE - Hardware Byte-by-Byte address
    *                                               detection
    *  RFID__B_UART__AM_HW_DETECT_TO_BUFFER - Hardware Detect to Buffer
    *                                               address detection
    *  RFID__B_UART__AM_NONE - No address detection
    *
    * Return:
    *  None.
    *
    * Global Variables:
    *  RFID_rxAddressMode - the parameter stored in this variable for
    *   the farther usage in RX ISR.
    *  RFID_rxAddressDetected - set to initial state (0).
    *
    *******************************************************************************/
    void RFID_SetRxAddressMode(uint8 addressMode)
                                                        
    {
        #if(RFID_RXHW_ADDRESS_ENABLED)
            #if(RFID_CONTROL_REG_REMOVED)
                if(0u != addressMode)
                {
                    /* Suppress compiler warning */
                }
            #else /* RFID_CONTROL_REG_REMOVED */
                uint8 tmpCtrl;
                tmpCtrl = RFID_CONTROL_REG & (uint8)~RFID_CTRL_RXADDR_MODE_MASK;
                tmpCtrl |= (uint8)(addressMode << RFID_CTRL_RXADDR_MODE0_SHIFT);
                RFID_CONTROL_REG = tmpCtrl;

                #if(RFID_RX_INTERRUPT_ENABLED && \
                   (RFID_RXBUFFERSIZE > RFID_FIFO_LENGTH) )
                    RFID_rxAddressMode = addressMode;
                    RFID_rxAddressDetected = 0u;
                #endif /* End RFID_RXBUFFERSIZE > RFID_FIFO_LENGTH*/
            #endif /* End RFID_CONTROL_REG_REMOVED */
        #else /* RFID_RXHW_ADDRESS_ENABLED */
            if(0u != addressMode)
            {
                /* Suppress compiler warning */
            }
        #endif /* End RFID_RXHW_ADDRESS_ENABLED */
    }


    /*******************************************************************************
    * Function Name: RFID_SetRxAddress1
    ********************************************************************************
    *
    * Summary:
    *  Sets the first of two hardware-detectable receiver addresses.
    *
    * Parameters:
    *  address: Address #1 for hardware address detection.
    *
    * Return:
    *  None.
    *
    *******************************************************************************/
    void RFID_SetRxAddress1(uint8 address) 
    {
        RFID_RXADDRESS1_REG = address;
    }


    /*******************************************************************************
    * Function Name: RFID_SetRxAddress2
    ********************************************************************************
    *
    * Summary:
    *  Sets the second of two hardware-detectable receiver addresses.
    *
    * Parameters:
    *  address: Address #2 for hardware address detection.
    *
    * Return:
    *  None.
    *
    *******************************************************************************/
    void RFID_SetRxAddress2(uint8 address) 
    {
        RFID_RXADDRESS2_REG = address;
    }

#endif  /* RFID_RX_ENABLED || RFID_HD_ENABLED*/


#if( (RFID_TX_ENABLED) || (RFID_HD_ENABLED) )
    /*******************************************************************************
    * Function Name: RFID_SetTxInterruptMode
    ********************************************************************************
    *
    * Summary:
    *  Configures the TX interrupt sources to be enabled, but does not enable the
    *  interrupt.
    *
    * Parameters:
    *  intSrc: Bit field containing the TX interrupt sources to enable
    *   RFID_TX_STS_COMPLETE        Interrupt on TX byte complete
    *   RFID_TX_STS_FIFO_EMPTY      Interrupt when TX FIFO is empty
    *   RFID_TX_STS_FIFO_FULL       Interrupt when TX FIFO is full
    *   RFID_TX_STS_FIFO_NOT_FULL   Interrupt when TX FIFO is not full
    *
    * Return:
    *  None.
    *
    * Theory:
    *  Enables the output of specific status bits to the interrupt controller
    *
    *******************************************************************************/
    void RFID_SetTxInterruptMode(uint8 intSrc) 
    {
        RFID_TXSTATUS_MASK_REG = intSrc;
    }


    /*******************************************************************************
    * Function Name: RFID_WriteTxData
    ********************************************************************************
    *
    * Summary:
    *  Places a byte of data into the transmit buffer to be sent when the bus is
    *  available without checking the TX status register. You must check status
    *  separately.
    *
    * Parameters:
    *  txDataByte: data byte
    *
    * Return:
    * None.
    *
    * Global Variables:
    *  RFID_txBuffer - RAM buffer pointer for save data for transmission
    *  RFID_txBufferWrite - cyclic index for write to txBuffer,
    *    incremented after each byte saved to buffer.
    *  RFID_txBufferRead - cyclic index for read from txBuffer,
    *    checked to identify the condition to write to FIFO directly or to TX buffer
    *  RFID_initVar - checked to identify that the component has been
    *    initialized.
    *
    * Reentrant:
    *  No.
    *
    *******************************************************************************/
    void RFID_WriteTxData(uint8 txDataByte) 
    {
        /* If not Initialized then skip this function*/
        if(RFID_initVar != 0u)
        {
        #if (RFID_TX_INTERRUPT_ENABLED)

            /* Protect variables that could change on interrupt. */
            RFID_DisableTxInt();

            if( (RFID_txBufferRead == RFID_txBufferWrite) &&
                ((RFID_TXSTATUS_REG & RFID_TX_STS_FIFO_FULL) == 0u) )
            {
                /* Add directly to the FIFO. */
                RFID_TXDATA_REG = txDataByte;
            }
            else
            {
                if(RFID_txBufferWrite >= RFID_TX_BUFFER_SIZE)
                {
                    RFID_txBufferWrite = 0u;
                }

                RFID_txBuffer[RFID_txBufferWrite] = txDataByte;

                /* Add to the software buffer. */
                RFID_txBufferWrite++;
            }

            RFID_EnableTxInt();

        #else

            /* Add directly to the FIFO. */
            RFID_TXDATA_REG = txDataByte;

        #endif /*(RFID_TX_INTERRUPT_ENABLED) */
        }
    }


    /*******************************************************************************
    * Function Name: RFID_ReadTxStatus
    ********************************************************************************
    *
    * Summary:
    *  Reads the status register for the TX portion of the UART.
    *
    * Parameters:
    *  None.
    *
    * Return:
    *  Contents of the status register
    *
    * Theory:
    *  This function reads the TX status register, which is cleared on read.
    *  It is up to the user to handle all bits in this return value accordingly,
    *  even if the bit was not enabled as an interrupt source the event happened
    *  and must be handled accordingly.
    *
    *******************************************************************************/
    uint8 RFID_ReadTxStatus(void) 
    {
        return(RFID_TXSTATUS_REG);
    }


    /*******************************************************************************
    * Function Name: RFID_PutChar
    ********************************************************************************
    *
    * Summary:
    *  Puts a byte of data into the transmit buffer to be sent when the bus is
    *  available. This is a blocking API that waits until the TX buffer has room to
    *  hold the data.
    *
    * Parameters:
    *  txDataByte: Byte containing the data to transmit
    *
    * Return:
    *  None.
    *
    * Global Variables:
    *  RFID_txBuffer - RAM buffer pointer for save data for transmission
    *  RFID_txBufferWrite - cyclic index for write to txBuffer,
    *     checked to identify free space in txBuffer and incremented after each byte
    *     saved to buffer.
    *  RFID_txBufferRead - cyclic index for read from txBuffer,
    *     checked to identify free space in txBuffer.
    *  RFID_initVar - checked to identify that the component has been
    *     initialized.
    *
    * Reentrant:
    *  No.
    *
    * Theory:
    *  Allows the user to transmit any byte of data in a single transfer
    *
    *******************************************************************************/
    void RFID_PutChar(uint8 txDataByte) 
    {
    #if (RFID_TX_INTERRUPT_ENABLED)
        /* The temporary output pointer is used since it takes two instructions
        *  to increment with a wrap, and we can't risk doing that with the real
        *  pointer and getting an interrupt in between instructions.
        */
        uint8 locTxBufferWrite;
        uint8 locTxBufferRead;

        do
        { /* Block if software buffer is full, so we don't overwrite. */

        #if ((RFID_TX_BUFFER_SIZE > RFID_MAX_BYTE_VALUE) && (CY_PSOC3))
            /* Disable TX interrupt to protect variables from modification */
            RFID_DisableTxInt();
        #endif /* (RFID_TX_BUFFER_SIZE > RFID_MAX_BYTE_VALUE) && (CY_PSOC3) */

            locTxBufferWrite = RFID_txBufferWrite;
            locTxBufferRead  = RFID_txBufferRead;

        #if ((RFID_TX_BUFFER_SIZE > RFID_MAX_BYTE_VALUE) && (CY_PSOC3))
            /* Enable interrupt to continue transmission */
            RFID_EnableTxInt();
        #endif /* (RFID_TX_BUFFER_SIZE > RFID_MAX_BYTE_VALUE) && (CY_PSOC3) */
        }
        while( (locTxBufferWrite < locTxBufferRead) ? (locTxBufferWrite == (locTxBufferRead - 1u)) :
                                ((locTxBufferWrite - locTxBufferRead) ==
                                (uint8)(RFID_TX_BUFFER_SIZE - 1u)) );

        if( (locTxBufferRead == locTxBufferWrite) &&
            ((RFID_TXSTATUS_REG & RFID_TX_STS_FIFO_FULL) == 0u) )
        {
            /* Add directly to the FIFO */
            RFID_TXDATA_REG = txDataByte;
        }
        else
        {
            if(locTxBufferWrite >= RFID_TX_BUFFER_SIZE)
            {
                locTxBufferWrite = 0u;
            }
            /* Add to the software buffer. */
            RFID_txBuffer[locTxBufferWrite] = txDataByte;
            locTxBufferWrite++;

            /* Finally, update the real output pointer */
        #if ((RFID_TX_BUFFER_SIZE > RFID_MAX_BYTE_VALUE) && (CY_PSOC3))
            RFID_DisableTxInt();
        #endif /* (RFID_TX_BUFFER_SIZE > RFID_MAX_BYTE_VALUE) && (CY_PSOC3) */

            RFID_txBufferWrite = locTxBufferWrite;

        #if ((RFID_TX_BUFFER_SIZE > RFID_MAX_BYTE_VALUE) && (CY_PSOC3))
            RFID_EnableTxInt();
        #endif /* (RFID_TX_BUFFER_SIZE > RFID_MAX_BYTE_VALUE) && (CY_PSOC3) */

            if(0u != (RFID_TXSTATUS_REG & RFID_TX_STS_FIFO_EMPTY))
            {
                /* Trigger TX interrupt to send software buffer */
                RFID_SetPendingTxInt();
            }
        }

    #else

        while((RFID_TXSTATUS_REG & RFID_TX_STS_FIFO_FULL) != 0u)
        {
            /* Wait for room in the FIFO */
        }

        /* Add directly to the FIFO */
        RFID_TXDATA_REG = txDataByte;

    #endif /* RFID_TX_INTERRUPT_ENABLED */
    }


    /*******************************************************************************
    * Function Name: RFID_PutString
    ********************************************************************************
    *
    * Summary:
    *  Sends a NULL terminated string to the TX buffer for transmission.
    *
    * Parameters:
    *  string[]: Pointer to the null terminated string array residing in RAM or ROM
    *
    * Return:
    *  None.
    *
    * Global Variables:
    *  RFID_initVar - checked to identify that the component has been
    *     initialized.
    *
    * Reentrant:
    *  No.
    *
    * Theory:
    *  If there is not enough memory in the TX buffer for the entire string, this
    *  function blocks until the last character of the string is loaded into the
    *  TX buffer.
    *
    *******************************************************************************/
    void RFID_PutString(const char8 string[]) 
    {
        uint16 bufIndex = 0u;

        /* If not Initialized then skip this function */
        if(RFID_initVar != 0u)
        {
            /* This is a blocking function, it will not exit until all data is sent */
            while(string[bufIndex] != (char8) 0)
            {
                RFID_PutChar((uint8)string[bufIndex]);
                bufIndex++;
            }
        }
    }


    /*******************************************************************************
    * Function Name: RFID_PutArray
    ********************************************************************************
    *
    * Summary:
    *  Places N bytes of data from a memory array into the TX buffer for
    *  transmission.
    *
    * Parameters:
    *  string[]: Address of the memory array residing in RAM or ROM.
    *  byteCount: Number of bytes to be transmitted. The type depends on TX Buffer
    *             Size parameter.
    *
    * Return:
    *  None.
    *
    * Global Variables:
    *  RFID_initVar - checked to identify that the component has been
    *     initialized.
    *
    * Reentrant:
    *  No.
    *
    * Theory:
    *  If there is not enough memory in the TX buffer for the entire string, this
    *  function blocks until the last character of the string is loaded into the
    *  TX buffer.
    *
    *******************************************************************************/
    void RFID_PutArray(const uint8 string[], uint8 byteCount)
                                                                    
    {
        uint8 bufIndex = 0u;

        /* If not Initialized then skip this function */
        if(RFID_initVar != 0u)
        {
            while(bufIndex < byteCount)
            {
                RFID_PutChar(string[bufIndex]);
                bufIndex++;
            }
        }
    }


    /*******************************************************************************
    * Function Name: RFID_PutCRLF
    ********************************************************************************
    *
    * Summary:
    *  Writes a byte of data followed by a carriage return (0x0D) and line feed
    *  (0x0A) to the transmit buffer.
    *
    * Parameters:
    *  txDataByte: Data byte to transmit before the carriage return and line feed.
    *
    * Return:
    *  None.
    *
    * Global Variables:
    *  RFID_initVar - checked to identify that the component has been
    *     initialized.
    *
    * Reentrant:
    *  No.
    *
    *******************************************************************************/
    void RFID_PutCRLF(uint8 txDataByte) 
    {
        /* If not Initialized then skip this function */
        if(RFID_initVar != 0u)
        {
            RFID_PutChar(txDataByte);
            RFID_PutChar(0x0Du);
            RFID_PutChar(0x0Au);
        }
    }


    /*******************************************************************************
    * Function Name: RFID_GetTxBufferSize
    ********************************************************************************
    *
    * Summary:
    *  Returns the number of bytes in the TX buffer which are waiting to be 
    *  transmitted.
    *  * TX software buffer is disabled (TX Buffer Size parameter is equal to 4): 
    *    returns 0 for empty TX FIFO, 1 for not full TX FIFO or 4 for full TX FIFO.
    *  * TX software buffer is enabled: returns the number of bytes in the TX 
    *    software buffer which are waiting to be transmitted. Bytes available in the
    *    TX FIFO do not count.
    *
    * Parameters:
    *  None.
    *
    * Return:
    *  Number of bytes used in the TX buffer. Return value type depends on the TX 
    *  Buffer Size parameter.
    *
    * Global Variables:
    *  RFID_txBufferWrite - used to calculate left space.
    *  RFID_txBufferRead - used to calculate left space.
    *
    * Reentrant:
    *  No.
    *
    * Theory:
    *  Allows the user to find out how full the TX Buffer is.
    *
    *******************************************************************************/
    uint8 RFID_GetTxBufferSize(void)
                                                            
    {
        uint8 size;

    #if (RFID_TX_INTERRUPT_ENABLED)

        /* Protect variables that could change on interrupt. */
        RFID_DisableTxInt();

        if(RFID_txBufferRead == RFID_txBufferWrite)
        {
            size = 0u;
        }
        else if(RFID_txBufferRead < RFID_txBufferWrite)
        {
            size = (RFID_txBufferWrite - RFID_txBufferRead);
        }
        else
        {
            size = (RFID_TX_BUFFER_SIZE - RFID_txBufferRead) +
                    RFID_txBufferWrite;
        }

        RFID_EnableTxInt();

    #else

        size = RFID_TXSTATUS_REG;

        /* Is the fifo is full. */
        if((size & RFID_TX_STS_FIFO_FULL) != 0u)
        {
            size = RFID_FIFO_LENGTH;
        }
        else if((size & RFID_TX_STS_FIFO_EMPTY) != 0u)
        {
            size = 0u;
        }
        else
        {
            /* We only know there is data in the fifo. */
            size = 1u;
        }

    #endif /* (RFID_TX_INTERRUPT_ENABLED) */

    return(size);
    }


    /*******************************************************************************
    * Function Name: RFID_ClearTxBuffer
    ********************************************************************************
    *
    * Summary:
    *  Clears all data from the TX buffer and hardware TX FIFO.
    *
    * Parameters:
    *  None.
    *
    * Return:
    *  None.
    *
    * Global Variables:
    *  RFID_txBufferWrite - cleared to zero.
    *  RFID_txBufferRead - cleared to zero.
    *
    * Reentrant:
    *  No.
    *
    * Theory:
    *  Setting the pointers to zero makes the system believe there is no data to
    *  read and writing will resume at address 0 overwriting any data that may have
    *  remained in the RAM.
    *
    * Side Effects:
    *  Data waiting in the transmit buffer is not sent; a byte that is currently
    *  transmitting finishes transmitting.
    *
    *******************************************************************************/
    void RFID_ClearTxBuffer(void) 
    {
        uint8 enableInterrupts;

        enableInterrupts = CyEnterCriticalSection();
        /* Clear the HW FIFO */
        RFID_TXDATA_AUX_CTL_REG |= (uint8)  RFID_TX_FIFO_CLR;
        RFID_TXDATA_AUX_CTL_REG &= (uint8) ~RFID_TX_FIFO_CLR;
        CyExitCriticalSection(enableInterrupts);

    #if (RFID_TX_INTERRUPT_ENABLED)

        /* Protect variables that could change on interrupt. */
        RFID_DisableTxInt();

        RFID_txBufferRead = 0u;
        RFID_txBufferWrite = 0u;

        /* Enable Tx interrupt. */
        RFID_EnableTxInt();

    #endif /* (RFID_TX_INTERRUPT_ENABLED) */
    }


    /*******************************************************************************
    * Function Name: RFID_SendBreak
    ********************************************************************************
    *
    * Summary:
    *  Transmits a break signal on the bus.
    *
    * Parameters:
    *  uint8 retMode:  Send Break return mode. See the following table for options.
    *   RFID_SEND_BREAK - Initialize registers for break, send the Break
    *       signal and return immediately.
    *   RFID_WAIT_FOR_COMPLETE_REINIT - Wait until break transmission is
    *       complete, reinitialize registers to normal transmission mode then return
    *   RFID_REINIT - Reinitialize registers to normal transmission mode
    *       then return.
    *   RFID_SEND_WAIT_REINIT - Performs both options: 
    *      RFID_SEND_BREAK and RFID_WAIT_FOR_COMPLETE_REINIT.
    *      This option is recommended for most cases.
    *
    * Return:
    *  None.
    *
    * Global Variables:
    *  RFID_initVar - checked to identify that the component has been
    *     initialized.
    *  txPeriod - static variable, used for keeping TX period configuration.
    *
    * Reentrant:
    *  No.
    *
    * Theory:
    *  SendBreak function initializes registers to send 13-bit break signal. It is
    *  important to return the registers configuration to normal for continue 8-bit
    *  operation.
    *  There are 3 variants for this API usage:
    *  1) SendBreak(3) - function will send the Break signal and take care on the
    *     configuration returning. Function will block CPU until transmission
    *     complete.
    *  2) User may want to use blocking time if UART configured to the low speed
    *     operation
    *     Example for this case:
    *     SendBreak(0);     - initialize Break signal transmission
    *         Add your code here to use CPU time
    *     SendBreak(1);     - complete Break operation
    *  3) Same to 2) but user may want to initialize and use the interrupt to
    *     complete break operation.
    *     Example for this case:
    *     Initialize TX interrupt with "TX - On TX Complete" parameter
    *     SendBreak(0);     - initialize Break signal transmission
    *         Add your code here to use CPU time
    *     When interrupt appear with RFID_TX_STS_COMPLETE status:
    *     SendBreak(2);     - complete Break operation
    *
    * Side Effects:
    *  The RFID_SendBreak() function initializes registers to send a
    *  break signal.
    *  Break signal length depends on the break signal bits configuration.
    *  The register configuration should be reinitialized before normal 8-bit
    *  communication can continue.
    *
    *******************************************************************************/
    void RFID_SendBreak(uint8 retMode) 
    {

        /* If not Initialized then skip this function*/
        if(RFID_initVar != 0u)
        {
            /* Set the Counter to 13-bits and transmit a 00 byte */
            /* When that is done then reset the counter value back */
            uint8 tmpStat;

        #if(RFID_HD_ENABLED) /* Half Duplex mode*/

            if( (retMode == RFID_SEND_BREAK) ||
                (retMode == RFID_SEND_WAIT_REINIT ) )
            {
                /* CTRL_HD_SEND_BREAK - sends break bits in HD mode */
                RFID_WriteControlRegister(RFID_ReadControlRegister() |
                                                      RFID_CTRL_HD_SEND_BREAK);
                /* Send zeros */
                RFID_TXDATA_REG = 0u;

                do /* Wait until transmit starts */
                {
                    tmpStat = RFID_TXSTATUS_REG;
                }
                while((tmpStat & RFID_TX_STS_FIFO_EMPTY) != 0u);
            }

            if( (retMode == RFID_WAIT_FOR_COMPLETE_REINIT) ||
                (retMode == RFID_SEND_WAIT_REINIT) )
            {
                do /* Wait until transmit complete */
                {
                    tmpStat = RFID_TXSTATUS_REG;
                }
                while(((uint8)~tmpStat & RFID_TX_STS_COMPLETE) != 0u);
            }

            if( (retMode == RFID_WAIT_FOR_COMPLETE_REINIT) ||
                (retMode == RFID_REINIT) ||
                (retMode == RFID_SEND_WAIT_REINIT) )
            {
                RFID_WriteControlRegister(RFID_ReadControlRegister() &
                                              (uint8)~RFID_CTRL_HD_SEND_BREAK);
            }

        #else /* RFID_HD_ENABLED Full Duplex mode */

            static uint8 txPeriod;

            if( (retMode == RFID_SEND_BREAK) ||
                (retMode == RFID_SEND_WAIT_REINIT) )
            {
                /* CTRL_HD_SEND_BREAK - skip to send parity bit at Break signal in Full Duplex mode */
                #if( (RFID_PARITY_TYPE != RFID__B_UART__NONE_REVB) || \
                                    (RFID_PARITY_TYPE_SW != 0u) )
                    RFID_WriteControlRegister(RFID_ReadControlRegister() |
                                                          RFID_CTRL_HD_SEND_BREAK);
                #endif /* End RFID_PARITY_TYPE != RFID__B_UART__NONE_REVB  */

                #if(RFID_TXCLKGEN_DP)
                    txPeriod = RFID_TXBITCLKTX_COMPLETE_REG;
                    RFID_TXBITCLKTX_COMPLETE_REG = RFID_TXBITCTR_BREAKBITS;
                #else
                    txPeriod = RFID_TXBITCTR_PERIOD_REG;
                    RFID_TXBITCTR_PERIOD_REG = RFID_TXBITCTR_BREAKBITS8X;
                #endif /* End RFID_TXCLKGEN_DP */

                /* Send zeros */
                RFID_TXDATA_REG = 0u;

                do /* Wait until transmit starts */
                {
                    tmpStat = RFID_TXSTATUS_REG;
                }
                while((tmpStat & RFID_TX_STS_FIFO_EMPTY) != 0u);
            }

            if( (retMode == RFID_WAIT_FOR_COMPLETE_REINIT) ||
                (retMode == RFID_SEND_WAIT_REINIT) )
            {
                do /* Wait until transmit complete */
                {
                    tmpStat = RFID_TXSTATUS_REG;
                }
                while(((uint8)~tmpStat & RFID_TX_STS_COMPLETE) != 0u);
            }

            if( (retMode == RFID_WAIT_FOR_COMPLETE_REINIT) ||
                (retMode == RFID_REINIT) ||
                (retMode == RFID_SEND_WAIT_REINIT) )
            {

            #if(RFID_TXCLKGEN_DP)
                RFID_TXBITCLKTX_COMPLETE_REG = txPeriod;
            #else
                RFID_TXBITCTR_PERIOD_REG = txPeriod;
            #endif /* End RFID_TXCLKGEN_DP */

            #if( (RFID_PARITY_TYPE != RFID__B_UART__NONE_REVB) || \
                 (RFID_PARITY_TYPE_SW != 0u) )
                RFID_WriteControlRegister(RFID_ReadControlRegister() &
                                                      (uint8) ~RFID_CTRL_HD_SEND_BREAK);
            #endif /* End RFID_PARITY_TYPE != NONE */
            }
        #endif    /* End RFID_HD_ENABLED */
        }
    }


    /*******************************************************************************
    * Function Name: RFID_SetTxAddressMode
    ********************************************************************************
    *
    * Summary:
    *  Configures the transmitter to signal the next bytes is address or data.
    *
    * Parameters:
    *  addressMode: 
    *       RFID_SET_SPACE - Configure the transmitter to send the next
    *                                    byte as a data.
    *       RFID_SET_MARK  - Configure the transmitter to send the next
    *                                    byte as an address.
    *
    * Return:
    *  None.
    *
    * Side Effects:
    *  This function sets and clears RFID_CTRL_MARK bit in the Control
    *  register.
    *
    *******************************************************************************/
    void RFID_SetTxAddressMode(uint8 addressMode) 
    {
        /* Mark/Space sending enable */
        if(addressMode != 0u)
        {
        #if( RFID_CONTROL_REG_REMOVED == 0u )
            RFID_WriteControlRegister(RFID_ReadControlRegister() |
                                                  RFID_CTRL_MARK);
        #endif /* End RFID_CONTROL_REG_REMOVED == 0u */
        }
        else
        {
        #if( RFID_CONTROL_REG_REMOVED == 0u )
            RFID_WriteControlRegister(RFID_ReadControlRegister() &
                                                  (uint8) ~RFID_CTRL_MARK);
        #endif /* End RFID_CONTROL_REG_REMOVED == 0u */
        }
    }

#endif  /* EndRFID_TX_ENABLED */

#if(RFID_HD_ENABLED)


    /*******************************************************************************
    * Function Name: RFID_LoadRxConfig
    ********************************************************************************
    *
    * Summary:
    *  Loads the receiver configuration in half duplex mode. After calling this
    *  function, the UART is ready to receive data.
    *
    * Parameters:
    *  None.
    *
    * Return:
    *  None.
    *
    * Side Effects:
    *  Valid only in half duplex mode. You must make sure that the previous
    *  transaction is complete and it is safe to unload the transmitter
    *  configuration.
    *
    *******************************************************************************/
    void RFID_LoadRxConfig(void) 
    {
        RFID_WriteControlRegister(RFID_ReadControlRegister() &
                                                (uint8)~RFID_CTRL_HD_SEND);
        RFID_RXBITCTR_PERIOD_REG = RFID_HD_RXBITCTR_INIT;

    #if (RFID_RX_INTERRUPT_ENABLED)
        /* Enable RX interrupt after set RX configuration */
        RFID_SetRxInterruptMode(RFID_INIT_RX_INTERRUPTS_MASK);
    #endif /* (RFID_RX_INTERRUPT_ENABLED) */
    }


    /*******************************************************************************
    * Function Name: RFID_LoadTxConfig
    ********************************************************************************
    *
    * Summary:
    *  Loads the transmitter configuration in half duplex mode. After calling this
    *  function, the UART is ready to transmit data.
    *
    * Parameters:
    *  None.
    *
    * Return:
    *  None.
    *
    * Side Effects:
    *  Valid only in half duplex mode. You must make sure that the previous
    *  transaction is complete and it is safe to unload the receiver configuration.
    *
    *******************************************************************************/
    void RFID_LoadTxConfig(void) 
    {
    #if (RFID_RX_INTERRUPT_ENABLED)
        /* Disable RX interrupts before set TX configuration */
        RFID_SetRxInterruptMode(0u);
    #endif /* (RFID_RX_INTERRUPT_ENABLED) */

        RFID_WriteControlRegister(RFID_ReadControlRegister() | RFID_CTRL_HD_SEND);
        RFID_RXBITCTR_PERIOD_REG = RFID_HD_TXBITCTR_INIT;
    }

#endif  /* RFID_HD_ENABLED */


/* [] END OF FILE */
