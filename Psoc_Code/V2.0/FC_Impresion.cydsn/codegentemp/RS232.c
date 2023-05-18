/*******************************************************************************
* File Name: RS232.c
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

#include "RS232.h"
#if (RS232_INTERNAL_CLOCK_USED)
    #include "RS232_IntClock.h"
#endif /* End RS232_INTERNAL_CLOCK_USED */


/***************************************
* Global data allocation
***************************************/

uint8 RS232_initVar = 0u;

#if (RS232_TX_INTERRUPT_ENABLED && RS232_TX_ENABLED)
    volatile uint8 RS232_txBuffer[RS232_TX_BUFFER_SIZE];
    volatile uint8 RS232_txBufferRead = 0u;
    uint8 RS232_txBufferWrite = 0u;
#endif /* (RS232_TX_INTERRUPT_ENABLED && RS232_TX_ENABLED) */

#if (RS232_RX_INTERRUPT_ENABLED && (RS232_RX_ENABLED || RS232_HD_ENABLED))
    uint8 RS232_errorStatus = 0u;
    volatile uint8 RS232_rxBuffer[RS232_RX_BUFFER_SIZE];
    volatile uint16 RS232_rxBufferRead  = 0u;
    volatile uint16 RS232_rxBufferWrite = 0u;
    volatile uint8 RS232_rxBufferLoopDetect = 0u;
    volatile uint8 RS232_rxBufferOverflow   = 0u;
    #if (RS232_RXHW_ADDRESS_ENABLED)
        volatile uint8 RS232_rxAddressMode = RS232_RX_ADDRESS_MODE;
        volatile uint8 RS232_rxAddressDetected = 0u;
    #endif /* (RS232_RXHW_ADDRESS_ENABLED) */
#endif /* (RS232_RX_INTERRUPT_ENABLED && (RS232_RX_ENABLED || RS232_HD_ENABLED)) */


/*******************************************************************************
* Function Name: RS232_Start
********************************************************************************
*
* Summary:
*  This is the preferred method to begin component operation.
*  RS232_Start() sets the initVar variable, calls the
*  RS232_Init() function, and then calls the
*  RS232_Enable() function.
*
* Parameters:
*  None.
*
* Return:
*  None.
*
* Global variables:
*  The RS232_intiVar variable is used to indicate initial
*  configuration of this component. The variable is initialized to zero (0u)
*  and set to one (1u) the first time RS232_Start() is called. This
*  allows for component initialization without re-initialization in all
*  subsequent calls to the RS232_Start() routine.
*
* Reentrant:
*  No.
*
*******************************************************************************/
void RS232_Start(void) 
{
    /* If not initialized then initialize all required hardware and software */
    if(RS232_initVar == 0u)
    {
        RS232_Init();
        RS232_initVar = 1u;
    }

    RS232_Enable();
}


/*******************************************************************************
* Function Name: RS232_Init
********************************************************************************
*
* Summary:
*  Initializes or restores the component according to the customizer Configure
*  dialog settings. It is not necessary to call RS232_Init() because
*  the RS232_Start() API calls this function and is the preferred
*  method to begin component operation.
*
* Parameters:
*  None.
*
* Return:
*  None.
*
*******************************************************************************/
void RS232_Init(void) 
{
    #if(RS232_RX_ENABLED || RS232_HD_ENABLED)

        #if (RS232_RX_INTERRUPT_ENABLED)
            /* Set RX interrupt vector and priority */
            (void) CyIntSetVector(RS232_RX_VECT_NUM, &RS232_RXISR);
            CyIntSetPriority(RS232_RX_VECT_NUM, RS232_RX_PRIOR_NUM);
            RS232_errorStatus = 0u;
        #endif /* (RS232_RX_INTERRUPT_ENABLED) */

        #if (RS232_RXHW_ADDRESS_ENABLED)
            RS232_SetRxAddressMode(RS232_RX_ADDRESS_MODE);
            RS232_SetRxAddress1(RS232_RX_HW_ADDRESS1);
            RS232_SetRxAddress2(RS232_RX_HW_ADDRESS2);
        #endif /* End RS232_RXHW_ADDRESS_ENABLED */

        /* Init Count7 period */
        RS232_RXBITCTR_PERIOD_REG = RS232_RXBITCTR_INIT;
        /* Configure the Initial RX interrupt mask */
        RS232_RXSTATUS_MASK_REG  = RS232_INIT_RX_INTERRUPTS_MASK;
    #endif /* End RS232_RX_ENABLED || RS232_HD_ENABLED*/

    #if(RS232_TX_ENABLED)
        #if (RS232_TX_INTERRUPT_ENABLED)
            /* Set TX interrupt vector and priority */
            (void) CyIntSetVector(RS232_TX_VECT_NUM, &RS232_TXISR);
            CyIntSetPriority(RS232_TX_VECT_NUM, RS232_TX_PRIOR_NUM);
        #endif /* (RS232_TX_INTERRUPT_ENABLED) */

        /* Write Counter Value for TX Bit Clk Generator*/
        #if (RS232_TXCLKGEN_DP)
            RS232_TXBITCLKGEN_CTR_REG = RS232_BIT_CENTER;
            RS232_TXBITCLKTX_COMPLETE_REG = ((RS232_NUMBER_OF_DATA_BITS +
                        RS232_NUMBER_OF_START_BIT) * RS232_OVER_SAMPLE_COUNT) - 1u;
        #else
            RS232_TXBITCTR_PERIOD_REG = ((RS232_NUMBER_OF_DATA_BITS +
                        RS232_NUMBER_OF_START_BIT) * RS232_OVER_SAMPLE_8) - 1u;
        #endif /* End RS232_TXCLKGEN_DP */

        /* Configure the Initial TX interrupt mask */
        #if (RS232_TX_INTERRUPT_ENABLED)
            RS232_TXSTATUS_MASK_REG = RS232_TX_STS_FIFO_EMPTY;
        #else
            RS232_TXSTATUS_MASK_REG = RS232_INIT_TX_INTERRUPTS_MASK;
        #endif /*End RS232_TX_INTERRUPT_ENABLED*/

    #endif /* End RS232_TX_ENABLED */

    #if(RS232_PARITY_TYPE_SW)  /* Write Parity to Control Register */
        RS232_WriteControlRegister( \
            (RS232_ReadControlRegister() & (uint8)~RS232_CTRL_PARITY_TYPE_MASK) | \
            (uint8)(RS232_PARITY_TYPE << RS232_CTRL_PARITY_TYPE0_SHIFT) );
    #endif /* End RS232_PARITY_TYPE_SW */
}


/*******************************************************************************
* Function Name: RS232_Enable
********************************************************************************
*
* Summary:
*  Activates the hardware and begins component operation. It is not necessary
*  to call RS232_Enable() because the RS232_Start() API
*  calls this function, which is the preferred method to begin component
*  operation.

* Parameters:
*  None.
*
* Return:
*  None.
*
* Global Variables:
*  RS232_rxAddressDetected - set to initial state (0).
*
*******************************************************************************/
void RS232_Enable(void) 
{
    uint8 enableInterrupts;
    enableInterrupts = CyEnterCriticalSection();

    #if (RS232_RX_ENABLED || RS232_HD_ENABLED)
        /* RX Counter (Count7) Enable */
        RS232_RXBITCTR_CONTROL_REG |= RS232_CNTR_ENABLE;

        /* Enable the RX Interrupt */
        RS232_RXSTATUS_ACTL_REG  |= RS232_INT_ENABLE;

        #if (RS232_RX_INTERRUPT_ENABLED)
            RS232_EnableRxInt();

            #if (RS232_RXHW_ADDRESS_ENABLED)
                RS232_rxAddressDetected = 0u;
            #endif /* (RS232_RXHW_ADDRESS_ENABLED) */
        #endif /* (RS232_RX_INTERRUPT_ENABLED) */
    #endif /* (RS232_RX_ENABLED || RS232_HD_ENABLED) */

    #if(RS232_TX_ENABLED)
        /* TX Counter (DP/Count7) Enable */
        #if(!RS232_TXCLKGEN_DP)
            RS232_TXBITCTR_CONTROL_REG |= RS232_CNTR_ENABLE;
        #endif /* End RS232_TXCLKGEN_DP */

        /* Enable the TX Interrupt */
        RS232_TXSTATUS_ACTL_REG |= RS232_INT_ENABLE;
        #if (RS232_TX_INTERRUPT_ENABLED)
            RS232_ClearPendingTxInt(); /* Clear history of TX_NOT_EMPTY */
            RS232_EnableTxInt();
        #endif /* (RS232_TX_INTERRUPT_ENABLED) */
     #endif /* (RS232_TX_INTERRUPT_ENABLED) */

    #if (RS232_INTERNAL_CLOCK_USED)
        RS232_IntClock_Start();  /* Enable the clock */
    #endif /* (RS232_INTERNAL_CLOCK_USED) */

    CyExitCriticalSection(enableInterrupts);
}


/*******************************************************************************
* Function Name: RS232_Stop
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
void RS232_Stop(void) 
{
    uint8 enableInterrupts;
    enableInterrupts = CyEnterCriticalSection();

    /* Write Bit Counter Disable */
    #if (RS232_RX_ENABLED || RS232_HD_ENABLED)
        RS232_RXBITCTR_CONTROL_REG &= (uint8) ~RS232_CNTR_ENABLE;
    #endif /* (RS232_RX_ENABLED || RS232_HD_ENABLED) */

    #if (RS232_TX_ENABLED)
        #if(!RS232_TXCLKGEN_DP)
            RS232_TXBITCTR_CONTROL_REG &= (uint8) ~RS232_CNTR_ENABLE;
        #endif /* (!RS232_TXCLKGEN_DP) */
    #endif /* (RS232_TX_ENABLED) */

    #if (RS232_INTERNAL_CLOCK_USED)
        RS232_IntClock_Stop();   /* Disable the clock */
    #endif /* (RS232_INTERNAL_CLOCK_USED) */

    /* Disable internal interrupt component */
    #if (RS232_RX_ENABLED || RS232_HD_ENABLED)
        RS232_RXSTATUS_ACTL_REG  &= (uint8) ~RS232_INT_ENABLE;

        #if (RS232_RX_INTERRUPT_ENABLED)
            RS232_DisableRxInt();
        #endif /* (RS232_RX_INTERRUPT_ENABLED) */
    #endif /* (RS232_RX_ENABLED || RS232_HD_ENABLED) */

    #if (RS232_TX_ENABLED)
        RS232_TXSTATUS_ACTL_REG &= (uint8) ~RS232_INT_ENABLE;

        #if (RS232_TX_INTERRUPT_ENABLED)
            RS232_DisableTxInt();
        #endif /* (RS232_TX_INTERRUPT_ENABLED) */
    #endif /* (RS232_TX_ENABLED) */

    CyExitCriticalSection(enableInterrupts);
}


/*******************************************************************************
* Function Name: RS232_ReadControlRegister
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
uint8 RS232_ReadControlRegister(void) 
{
    #if (RS232_CONTROL_REG_REMOVED)
        return(0u);
    #else
        return(RS232_CONTROL_REG);
    #endif /* (RS232_CONTROL_REG_REMOVED) */
}


/*******************************************************************************
* Function Name: RS232_WriteControlRegister
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
void  RS232_WriteControlRegister(uint8 control) 
{
    #if (RS232_CONTROL_REG_REMOVED)
        if(0u != control)
        {
            /* Suppress compiler warning */
        }
    #else
       RS232_CONTROL_REG = control;
    #endif /* (RS232_CONTROL_REG_REMOVED) */
}


#if(RS232_RX_ENABLED || RS232_HD_ENABLED)
    /*******************************************************************************
    * Function Name: RS232_SetRxInterruptMode
    ********************************************************************************
    *
    * Summary:
    *  Configures the RX interrupt sources enabled.
    *
    * Parameters:
    *  IntSrc:  Bit field containing the RX interrupts to enable. Based on the 
    *  bit-field arrangement of the status register. This value must be a 
    *  combination of status register bit-masks shown below:
    *      RS232_RX_STS_FIFO_NOTEMPTY    Interrupt on byte received.
    *      RS232_RX_STS_PAR_ERROR        Interrupt on parity error.
    *      RS232_RX_STS_STOP_ERROR       Interrupt on stop error.
    *      RS232_RX_STS_BREAK            Interrupt on break.
    *      RS232_RX_STS_OVERRUN          Interrupt on overrun error.
    *      RS232_RX_STS_ADDR_MATCH       Interrupt on address match.
    *      RS232_RX_STS_MRKSPC           Interrupt on address detect.
    *
    * Return:
    *  None.
    *
    * Theory:
    *  Enables the output of specific status bits to the interrupt controller
    *
    *******************************************************************************/
    void RS232_SetRxInterruptMode(uint8 intSrc) 
    {
        RS232_RXSTATUS_MASK_REG  = intSrc;
    }


    /*******************************************************************************
    * Function Name: RS232_ReadRxData
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
    *  RS232_rxBuffer - RAM buffer pointer for save received data.
    *  RS232_rxBufferWrite - cyclic index for write to rxBuffer,
    *     checked to identify new data.
    *  RS232_rxBufferRead - cyclic index for read from rxBuffer,
    *     incremented after each byte has been read from buffer.
    *  RS232_rxBufferLoopDetect - cleared if loop condition was detected
    *     in RX ISR.
    *
    * Reentrant:
    *  No.
    *
    *******************************************************************************/
    uint8 RS232_ReadRxData(void) 
    {
        uint8 rxData;

    #if (RS232_RX_INTERRUPT_ENABLED)

        uint16 locRxBufferRead;
        uint16 locRxBufferWrite;

        /* Protect variables that could change on interrupt */
        RS232_DisableRxInt();

        locRxBufferRead  = RS232_rxBufferRead;
        locRxBufferWrite = RS232_rxBufferWrite;

        if( (RS232_rxBufferLoopDetect != 0u) || (locRxBufferRead != locRxBufferWrite) )
        {
            rxData = RS232_rxBuffer[locRxBufferRead];
            locRxBufferRead++;

            if(locRxBufferRead >= RS232_RX_BUFFER_SIZE)
            {
                locRxBufferRead = 0u;
            }
            /* Update the real pointer */
            RS232_rxBufferRead = locRxBufferRead;

            if(RS232_rxBufferLoopDetect != 0u)
            {
                RS232_rxBufferLoopDetect = 0u;
                #if ((RS232_RX_INTERRUPT_ENABLED) && (RS232_FLOW_CONTROL != 0u))
                    /* When Hardware Flow Control selected - return RX mask */
                    #if( RS232_HD_ENABLED )
                        if((RS232_CONTROL_REG & RS232_CTRL_HD_SEND) == 0u)
                        {   /* In Half duplex mode return RX mask only in RX
                            *  configuration set, otherwise
                            *  mask will be returned in LoadRxConfig() API.
                            */
                            RS232_RXSTATUS_MASK_REG  |= RS232_RX_STS_FIFO_NOTEMPTY;
                        }
                    #else
                        RS232_RXSTATUS_MASK_REG  |= RS232_RX_STS_FIFO_NOTEMPTY;
                    #endif /* end RS232_HD_ENABLED */
                #endif /* ((RS232_RX_INTERRUPT_ENABLED) && (RS232_FLOW_CONTROL != 0u)) */
            }
        }
        else
        {   /* Needs to check status for RX_STS_FIFO_NOTEMPTY bit */
            rxData = RS232_RXDATA_REG;
        }

        RS232_EnableRxInt();

    #else

        /* Needs to check status for RX_STS_FIFO_NOTEMPTY bit */
        rxData = RS232_RXDATA_REG;

    #endif /* (RS232_RX_INTERRUPT_ENABLED) */

        return(rxData);
    }


    /*******************************************************************************
    * Function Name: RS232_ReadRxStatus
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
    *  RS232_RX_STS_FIFO_NOTEMPTY.
    *  RS232_RX_STS_FIFO_NOTEMPTY clears immediately after RX data
    *  register read.
    *
    * Global Variables:
    *  RS232_rxBufferOverflow - used to indicate overload condition.
    *   It set to one in RX interrupt when there isn't free space in
    *   RS232_rxBufferRead to write new data. This condition returned
    *   and cleared to zero by this API as an
    *   RS232_RX_STS_SOFT_BUFF_OVER bit along with RX Status register
    *   bits.
    *
    *******************************************************************************/
    uint8 RS232_ReadRxStatus(void) 
    {
        uint8 status;

        status = RS232_RXSTATUS_REG & RS232_RX_HW_MASK;

    #if (RS232_RX_INTERRUPT_ENABLED)
        if(RS232_rxBufferOverflow != 0u)
        {
            status |= RS232_RX_STS_SOFT_BUFF_OVER;
            RS232_rxBufferOverflow = 0u;
        }
    #endif /* (RS232_RX_INTERRUPT_ENABLED) */

        return(status);
    }


    /*******************************************************************************
    * Function Name: RS232_GetChar
    ********************************************************************************
    *
    * Summary:
    *  Returns the last received byte of data. RS232_GetChar() is
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
    *  RS232_rxBuffer - RAM buffer pointer for save received data.
    *  RS232_rxBufferWrite - cyclic index for write to rxBuffer,
    *     checked to identify new data.
    *  RS232_rxBufferRead - cyclic index for read from rxBuffer,
    *     incremented after each byte has been read from buffer.
    *  RS232_rxBufferLoopDetect - cleared if loop condition was detected
    *     in RX ISR.
    *
    * Reentrant:
    *  No.
    *
    *******************************************************************************/
    uint8 RS232_GetChar(void) 
    {
        uint8 rxData = 0u;
        uint8 rxStatus;

    #if (RS232_RX_INTERRUPT_ENABLED)
        uint16 locRxBufferRead;
        uint16 locRxBufferWrite;

        /* Protect variables that could change on interrupt */
        RS232_DisableRxInt();

        locRxBufferRead  = RS232_rxBufferRead;
        locRxBufferWrite = RS232_rxBufferWrite;

        if( (RS232_rxBufferLoopDetect != 0u) || (locRxBufferRead != locRxBufferWrite) )
        {
            rxData = RS232_rxBuffer[locRxBufferRead];
            locRxBufferRead++;
            if(locRxBufferRead >= RS232_RX_BUFFER_SIZE)
            {
                locRxBufferRead = 0u;
            }
            /* Update the real pointer */
            RS232_rxBufferRead = locRxBufferRead;

            if(RS232_rxBufferLoopDetect != 0u)
            {
                RS232_rxBufferLoopDetect = 0u;
                #if( (RS232_RX_INTERRUPT_ENABLED) && (RS232_FLOW_CONTROL != 0u) )
                    /* When Hardware Flow Control selected - return RX mask */
                    #if( RS232_HD_ENABLED )
                        if((RS232_CONTROL_REG & RS232_CTRL_HD_SEND) == 0u)
                        {   /* In Half duplex mode return RX mask only if
                            *  RX configuration set, otherwise
                            *  mask will be returned in LoadRxConfig() API.
                            */
                            RS232_RXSTATUS_MASK_REG |= RS232_RX_STS_FIFO_NOTEMPTY;
                        }
                    #else
                        RS232_RXSTATUS_MASK_REG |= RS232_RX_STS_FIFO_NOTEMPTY;
                    #endif /* end RS232_HD_ENABLED */
                #endif /* RS232_RX_INTERRUPT_ENABLED and Hardware flow control*/
            }

        }
        else
        {   rxStatus = RS232_RXSTATUS_REG;
            if((rxStatus & RS232_RX_STS_FIFO_NOTEMPTY) != 0u)
            {   /* Read received data from FIFO */
                rxData = RS232_RXDATA_REG;
                /*Check status on error*/
                if((rxStatus & (RS232_RX_STS_BREAK | RS232_RX_STS_PAR_ERROR |
                                RS232_RX_STS_STOP_ERROR | RS232_RX_STS_OVERRUN)) != 0u)
                {
                    rxData = 0u;
                }
            }
        }

        RS232_EnableRxInt();

    #else

        rxStatus =RS232_RXSTATUS_REG;
        if((rxStatus & RS232_RX_STS_FIFO_NOTEMPTY) != 0u)
        {
            /* Read received data from FIFO */
            rxData = RS232_RXDATA_REG;

            /*Check status on error*/
            if((rxStatus & (RS232_RX_STS_BREAK | RS232_RX_STS_PAR_ERROR |
                            RS232_RX_STS_STOP_ERROR | RS232_RX_STS_OVERRUN)) != 0u)
            {
                rxData = 0u;
            }
        }
    #endif /* (RS232_RX_INTERRUPT_ENABLED) */

        return(rxData);
    }


    /*******************************************************************************
    * Function Name: RS232_GetByte
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
    uint16 RS232_GetByte(void) 
    {
        
    #if (RS232_RX_INTERRUPT_ENABLED)
        uint16 locErrorStatus;
        /* Protect variables that could change on interrupt */
        RS232_DisableRxInt();
        locErrorStatus = (uint16)RS232_errorStatus;
        RS232_errorStatus = 0u;
        RS232_EnableRxInt();
        return ( (uint16)(locErrorStatus << 8u) | RS232_ReadRxData() );
    #else
        return ( ((uint16)RS232_ReadRxStatus() << 8u) | RS232_ReadRxData() );
    #endif /* RS232_RX_INTERRUPT_ENABLED */
        
    }


    /*******************************************************************************
    * Function Name: RS232_GetRxBufferSize
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
    *  uint16: Number of bytes in the RX buffer. 
    *    Return value type depends on RX Buffer Size parameter.
    *
    * Global Variables:
    *  RS232_rxBufferWrite - used to calculate left bytes.
    *  RS232_rxBufferRead - used to calculate left bytes.
    *  RS232_rxBufferLoopDetect - checked to decide left bytes amount.
    *
    * Reentrant:
    *  No.
    *
    * Theory:
    *  Allows the user to find out how full the RX Buffer is.
    *
    *******************************************************************************/
    uint16 RS232_GetRxBufferSize(void)
                                                            
    {
        uint16 size;

    #if (RS232_RX_INTERRUPT_ENABLED)

        /* Protect variables that could change on interrupt */
        RS232_DisableRxInt();

        if(RS232_rxBufferRead == RS232_rxBufferWrite)
        {
            if(RS232_rxBufferLoopDetect != 0u)
            {
                size = RS232_RX_BUFFER_SIZE;
            }
            else
            {
                size = 0u;
            }
        }
        else if(RS232_rxBufferRead < RS232_rxBufferWrite)
        {
            size = (RS232_rxBufferWrite - RS232_rxBufferRead);
        }
        else
        {
            size = (RS232_RX_BUFFER_SIZE - RS232_rxBufferRead) + RS232_rxBufferWrite;
        }

        RS232_EnableRxInt();

    #else

        /* We can only know if there is data in the fifo. */
        size = ((RS232_RXSTATUS_REG & RS232_RX_STS_FIFO_NOTEMPTY) != 0u) ? 1u : 0u;

    #endif /* (RS232_RX_INTERRUPT_ENABLED) */

        return(size);
    }


    /*******************************************************************************
    * Function Name: RS232_ClearRxBuffer
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
    *  RS232_rxBufferWrite - cleared to zero.
    *  RS232_rxBufferRead - cleared to zero.
    *  RS232_rxBufferLoopDetect - cleared to zero.
    *  RS232_rxBufferOverflow - cleared to zero.
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
    void RS232_ClearRxBuffer(void) 
    {
        uint8 enableInterrupts;

        /* Clear the HW FIFO */
        enableInterrupts = CyEnterCriticalSection();
        RS232_RXDATA_AUX_CTL_REG |= (uint8)  RS232_RX_FIFO_CLR;
        RS232_RXDATA_AUX_CTL_REG &= (uint8) ~RS232_RX_FIFO_CLR;
        CyExitCriticalSection(enableInterrupts);

    #if (RS232_RX_INTERRUPT_ENABLED)

        /* Protect variables that could change on interrupt. */
        RS232_DisableRxInt();

        RS232_rxBufferRead = 0u;
        RS232_rxBufferWrite = 0u;
        RS232_rxBufferLoopDetect = 0u;
        RS232_rxBufferOverflow = 0u;

        RS232_EnableRxInt();

    #endif /* (RS232_RX_INTERRUPT_ENABLED) */

    }


    /*******************************************************************************
    * Function Name: RS232_SetRxAddressMode
    ********************************************************************************
    *
    * Summary:
    *  Sets the software controlled Addressing mode used by the RX portion of the
    *  UART.
    *
    * Parameters:
    *  addressMode: Enumerated value indicating the mode of RX addressing
    *  RS232__B_UART__AM_SW_BYTE_BYTE -  Software Byte-by-Byte address
    *                                               detection
    *  RS232__B_UART__AM_SW_DETECT_TO_BUFFER - Software Detect to Buffer
    *                                               address detection
    *  RS232__B_UART__AM_HW_BYTE_BY_BYTE - Hardware Byte-by-Byte address
    *                                               detection
    *  RS232__B_UART__AM_HW_DETECT_TO_BUFFER - Hardware Detect to Buffer
    *                                               address detection
    *  RS232__B_UART__AM_NONE - No address detection
    *
    * Return:
    *  None.
    *
    * Global Variables:
    *  RS232_rxAddressMode - the parameter stored in this variable for
    *   the farther usage in RX ISR.
    *  RS232_rxAddressDetected - set to initial state (0).
    *
    *******************************************************************************/
    void RS232_SetRxAddressMode(uint8 addressMode)
                                                        
    {
        #if(RS232_RXHW_ADDRESS_ENABLED)
            #if(RS232_CONTROL_REG_REMOVED)
                if(0u != addressMode)
                {
                    /* Suppress compiler warning */
                }
            #else /* RS232_CONTROL_REG_REMOVED */
                uint8 tmpCtrl;
                tmpCtrl = RS232_CONTROL_REG & (uint8)~RS232_CTRL_RXADDR_MODE_MASK;
                tmpCtrl |= (uint8)(addressMode << RS232_CTRL_RXADDR_MODE0_SHIFT);
                RS232_CONTROL_REG = tmpCtrl;

                #if(RS232_RX_INTERRUPT_ENABLED && \
                   (RS232_RXBUFFERSIZE > RS232_FIFO_LENGTH) )
                    RS232_rxAddressMode = addressMode;
                    RS232_rxAddressDetected = 0u;
                #endif /* End RS232_RXBUFFERSIZE > RS232_FIFO_LENGTH*/
            #endif /* End RS232_CONTROL_REG_REMOVED */
        #else /* RS232_RXHW_ADDRESS_ENABLED */
            if(0u != addressMode)
            {
                /* Suppress compiler warning */
            }
        #endif /* End RS232_RXHW_ADDRESS_ENABLED */
    }


    /*******************************************************************************
    * Function Name: RS232_SetRxAddress1
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
    void RS232_SetRxAddress1(uint8 address) 
    {
        RS232_RXADDRESS1_REG = address;
    }


    /*******************************************************************************
    * Function Name: RS232_SetRxAddress2
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
    void RS232_SetRxAddress2(uint8 address) 
    {
        RS232_RXADDRESS2_REG = address;
    }

#endif  /* RS232_RX_ENABLED || RS232_HD_ENABLED*/


#if( (RS232_TX_ENABLED) || (RS232_HD_ENABLED) )
    /*******************************************************************************
    * Function Name: RS232_SetTxInterruptMode
    ********************************************************************************
    *
    * Summary:
    *  Configures the TX interrupt sources to be enabled, but does not enable the
    *  interrupt.
    *
    * Parameters:
    *  intSrc: Bit field containing the TX interrupt sources to enable
    *   RS232_TX_STS_COMPLETE        Interrupt on TX byte complete
    *   RS232_TX_STS_FIFO_EMPTY      Interrupt when TX FIFO is empty
    *   RS232_TX_STS_FIFO_FULL       Interrupt when TX FIFO is full
    *   RS232_TX_STS_FIFO_NOT_FULL   Interrupt when TX FIFO is not full
    *
    * Return:
    *  None.
    *
    * Theory:
    *  Enables the output of specific status bits to the interrupt controller
    *
    *******************************************************************************/
    void RS232_SetTxInterruptMode(uint8 intSrc) 
    {
        RS232_TXSTATUS_MASK_REG = intSrc;
    }


    /*******************************************************************************
    * Function Name: RS232_WriteTxData
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
    *  RS232_txBuffer - RAM buffer pointer for save data for transmission
    *  RS232_txBufferWrite - cyclic index for write to txBuffer,
    *    incremented after each byte saved to buffer.
    *  RS232_txBufferRead - cyclic index for read from txBuffer,
    *    checked to identify the condition to write to FIFO directly or to TX buffer
    *  RS232_initVar - checked to identify that the component has been
    *    initialized.
    *
    * Reentrant:
    *  No.
    *
    *******************************************************************************/
    void RS232_WriteTxData(uint8 txDataByte) 
    {
        /* If not Initialized then skip this function*/
        if(RS232_initVar != 0u)
        {
        #if (RS232_TX_INTERRUPT_ENABLED)

            /* Protect variables that could change on interrupt. */
            RS232_DisableTxInt();

            if( (RS232_txBufferRead == RS232_txBufferWrite) &&
                ((RS232_TXSTATUS_REG & RS232_TX_STS_FIFO_FULL) == 0u) )
            {
                /* Add directly to the FIFO. */
                RS232_TXDATA_REG = txDataByte;
            }
            else
            {
                if(RS232_txBufferWrite >= RS232_TX_BUFFER_SIZE)
                {
                    RS232_txBufferWrite = 0u;
                }

                RS232_txBuffer[RS232_txBufferWrite] = txDataByte;

                /* Add to the software buffer. */
                RS232_txBufferWrite++;
            }

            RS232_EnableTxInt();

        #else

            /* Add directly to the FIFO. */
            RS232_TXDATA_REG = txDataByte;

        #endif /*(RS232_TX_INTERRUPT_ENABLED) */
        }
    }


    /*******************************************************************************
    * Function Name: RS232_ReadTxStatus
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
    uint8 RS232_ReadTxStatus(void) 
    {
        return(RS232_TXSTATUS_REG);
    }


    /*******************************************************************************
    * Function Name: RS232_PutChar
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
    *  RS232_txBuffer - RAM buffer pointer for save data for transmission
    *  RS232_txBufferWrite - cyclic index for write to txBuffer,
    *     checked to identify free space in txBuffer and incremented after each byte
    *     saved to buffer.
    *  RS232_txBufferRead - cyclic index for read from txBuffer,
    *     checked to identify free space in txBuffer.
    *  RS232_initVar - checked to identify that the component has been
    *     initialized.
    *
    * Reentrant:
    *  No.
    *
    * Theory:
    *  Allows the user to transmit any byte of data in a single transfer
    *
    *******************************************************************************/
    void RS232_PutChar(uint8 txDataByte) 
    {
    #if (RS232_TX_INTERRUPT_ENABLED)
        /* The temporary output pointer is used since it takes two instructions
        *  to increment with a wrap, and we can't risk doing that with the real
        *  pointer and getting an interrupt in between instructions.
        */
        uint8 locTxBufferWrite;
        uint8 locTxBufferRead;

        do
        { /* Block if software buffer is full, so we don't overwrite. */

        #if ((RS232_TX_BUFFER_SIZE > RS232_MAX_BYTE_VALUE) && (CY_PSOC3))
            /* Disable TX interrupt to protect variables from modification */
            RS232_DisableTxInt();
        #endif /* (RS232_TX_BUFFER_SIZE > RS232_MAX_BYTE_VALUE) && (CY_PSOC3) */

            locTxBufferWrite = RS232_txBufferWrite;
            locTxBufferRead  = RS232_txBufferRead;

        #if ((RS232_TX_BUFFER_SIZE > RS232_MAX_BYTE_VALUE) && (CY_PSOC3))
            /* Enable interrupt to continue transmission */
            RS232_EnableTxInt();
        #endif /* (RS232_TX_BUFFER_SIZE > RS232_MAX_BYTE_VALUE) && (CY_PSOC3) */
        }
        while( (locTxBufferWrite < locTxBufferRead) ? (locTxBufferWrite == (locTxBufferRead - 1u)) :
                                ((locTxBufferWrite - locTxBufferRead) ==
                                (uint8)(RS232_TX_BUFFER_SIZE - 1u)) );

        if( (locTxBufferRead == locTxBufferWrite) &&
            ((RS232_TXSTATUS_REG & RS232_TX_STS_FIFO_FULL) == 0u) )
        {
            /* Add directly to the FIFO */
            RS232_TXDATA_REG = txDataByte;
        }
        else
        {
            if(locTxBufferWrite >= RS232_TX_BUFFER_SIZE)
            {
                locTxBufferWrite = 0u;
            }
            /* Add to the software buffer. */
            RS232_txBuffer[locTxBufferWrite] = txDataByte;
            locTxBufferWrite++;

            /* Finally, update the real output pointer */
        #if ((RS232_TX_BUFFER_SIZE > RS232_MAX_BYTE_VALUE) && (CY_PSOC3))
            RS232_DisableTxInt();
        #endif /* (RS232_TX_BUFFER_SIZE > RS232_MAX_BYTE_VALUE) && (CY_PSOC3) */

            RS232_txBufferWrite = locTxBufferWrite;

        #if ((RS232_TX_BUFFER_SIZE > RS232_MAX_BYTE_VALUE) && (CY_PSOC3))
            RS232_EnableTxInt();
        #endif /* (RS232_TX_BUFFER_SIZE > RS232_MAX_BYTE_VALUE) && (CY_PSOC3) */

            if(0u != (RS232_TXSTATUS_REG & RS232_TX_STS_FIFO_EMPTY))
            {
                /* Trigger TX interrupt to send software buffer */
                RS232_SetPendingTxInt();
            }
        }

    #else

        while((RS232_TXSTATUS_REG & RS232_TX_STS_FIFO_FULL) != 0u)
        {
            /* Wait for room in the FIFO */
        }

        /* Add directly to the FIFO */
        RS232_TXDATA_REG = txDataByte;

    #endif /* RS232_TX_INTERRUPT_ENABLED */
    }


    /*******************************************************************************
    * Function Name: RS232_PutString
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
    *  RS232_initVar - checked to identify that the component has been
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
    void RS232_PutString(const char8 string[]) 
    {
        uint16 bufIndex = 0u;

        /* If not Initialized then skip this function */
        if(RS232_initVar != 0u)
        {
            /* This is a blocking function, it will not exit until all data is sent */
            while(string[bufIndex] != (char8) 0)
            {
                RS232_PutChar((uint8)string[bufIndex]);
                bufIndex++;
            }
        }
    }


    /*******************************************************************************
    * Function Name: RS232_PutArray
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
    *  RS232_initVar - checked to identify that the component has been
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
    void RS232_PutArray(const uint8 string[], uint8 byteCount)
                                                                    
    {
        uint8 bufIndex = 0u;

        /* If not Initialized then skip this function */
        if(RS232_initVar != 0u)
        {
            while(bufIndex < byteCount)
            {
                RS232_PutChar(string[bufIndex]);
                bufIndex++;
            }
        }
    }


    /*******************************************************************************
    * Function Name: RS232_PutCRLF
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
    *  RS232_initVar - checked to identify that the component has been
    *     initialized.
    *
    * Reentrant:
    *  No.
    *
    *******************************************************************************/
    void RS232_PutCRLF(uint8 txDataByte) 
    {
        /* If not Initialized then skip this function */
        if(RS232_initVar != 0u)
        {
            RS232_PutChar(txDataByte);
            RS232_PutChar(0x0Du);
            RS232_PutChar(0x0Au);
        }
    }


    /*******************************************************************************
    * Function Name: RS232_GetTxBufferSize
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
    *  RS232_txBufferWrite - used to calculate left space.
    *  RS232_txBufferRead - used to calculate left space.
    *
    * Reentrant:
    *  No.
    *
    * Theory:
    *  Allows the user to find out how full the TX Buffer is.
    *
    *******************************************************************************/
    uint8 RS232_GetTxBufferSize(void)
                                                            
    {
        uint8 size;

    #if (RS232_TX_INTERRUPT_ENABLED)

        /* Protect variables that could change on interrupt. */
        RS232_DisableTxInt();

        if(RS232_txBufferRead == RS232_txBufferWrite)
        {
            size = 0u;
        }
        else if(RS232_txBufferRead < RS232_txBufferWrite)
        {
            size = (RS232_txBufferWrite - RS232_txBufferRead);
        }
        else
        {
            size = (RS232_TX_BUFFER_SIZE - RS232_txBufferRead) +
                    RS232_txBufferWrite;
        }

        RS232_EnableTxInt();

    #else

        size = RS232_TXSTATUS_REG;

        /* Is the fifo is full. */
        if((size & RS232_TX_STS_FIFO_FULL) != 0u)
        {
            size = RS232_FIFO_LENGTH;
        }
        else if((size & RS232_TX_STS_FIFO_EMPTY) != 0u)
        {
            size = 0u;
        }
        else
        {
            /* We only know there is data in the fifo. */
            size = 1u;
        }

    #endif /* (RS232_TX_INTERRUPT_ENABLED) */

    return(size);
    }


    /*******************************************************************************
    * Function Name: RS232_ClearTxBuffer
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
    *  RS232_txBufferWrite - cleared to zero.
    *  RS232_txBufferRead - cleared to zero.
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
    void RS232_ClearTxBuffer(void) 
    {
        uint8 enableInterrupts;

        enableInterrupts = CyEnterCriticalSection();
        /* Clear the HW FIFO */
        RS232_TXDATA_AUX_CTL_REG |= (uint8)  RS232_TX_FIFO_CLR;
        RS232_TXDATA_AUX_CTL_REG &= (uint8) ~RS232_TX_FIFO_CLR;
        CyExitCriticalSection(enableInterrupts);

    #if (RS232_TX_INTERRUPT_ENABLED)

        /* Protect variables that could change on interrupt. */
        RS232_DisableTxInt();

        RS232_txBufferRead = 0u;
        RS232_txBufferWrite = 0u;

        /* Enable Tx interrupt. */
        RS232_EnableTxInt();

    #endif /* (RS232_TX_INTERRUPT_ENABLED) */
    }


    /*******************************************************************************
    * Function Name: RS232_SendBreak
    ********************************************************************************
    *
    * Summary:
    *  Transmits a break signal on the bus.
    *
    * Parameters:
    *  uint8 retMode:  Send Break return mode. See the following table for options.
    *   RS232_SEND_BREAK - Initialize registers for break, send the Break
    *       signal and return immediately.
    *   RS232_WAIT_FOR_COMPLETE_REINIT - Wait until break transmission is
    *       complete, reinitialize registers to normal transmission mode then return
    *   RS232_REINIT - Reinitialize registers to normal transmission mode
    *       then return.
    *   RS232_SEND_WAIT_REINIT - Performs both options: 
    *      RS232_SEND_BREAK and RS232_WAIT_FOR_COMPLETE_REINIT.
    *      This option is recommended for most cases.
    *
    * Return:
    *  None.
    *
    * Global Variables:
    *  RS232_initVar - checked to identify that the component has been
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
    *     When interrupt appear with RS232_TX_STS_COMPLETE status:
    *     SendBreak(2);     - complete Break operation
    *
    * Side Effects:
    *  The RS232_SendBreak() function initializes registers to send a
    *  break signal.
    *  Break signal length depends on the break signal bits configuration.
    *  The register configuration should be reinitialized before normal 8-bit
    *  communication can continue.
    *
    *******************************************************************************/
    void RS232_SendBreak(uint8 retMode) 
    {

        /* If not Initialized then skip this function*/
        if(RS232_initVar != 0u)
        {
            /* Set the Counter to 13-bits and transmit a 00 byte */
            /* When that is done then reset the counter value back */
            uint8 tmpStat;

        #if(RS232_HD_ENABLED) /* Half Duplex mode*/

            if( (retMode == RS232_SEND_BREAK) ||
                (retMode == RS232_SEND_WAIT_REINIT ) )
            {
                /* CTRL_HD_SEND_BREAK - sends break bits in HD mode */
                RS232_WriteControlRegister(RS232_ReadControlRegister() |
                                                      RS232_CTRL_HD_SEND_BREAK);
                /* Send zeros */
                RS232_TXDATA_REG = 0u;

                do /* Wait until transmit starts */
                {
                    tmpStat = RS232_TXSTATUS_REG;
                }
                while((tmpStat & RS232_TX_STS_FIFO_EMPTY) != 0u);
            }

            if( (retMode == RS232_WAIT_FOR_COMPLETE_REINIT) ||
                (retMode == RS232_SEND_WAIT_REINIT) )
            {
                do /* Wait until transmit complete */
                {
                    tmpStat = RS232_TXSTATUS_REG;
                }
                while(((uint8)~tmpStat & RS232_TX_STS_COMPLETE) != 0u);
            }

            if( (retMode == RS232_WAIT_FOR_COMPLETE_REINIT) ||
                (retMode == RS232_REINIT) ||
                (retMode == RS232_SEND_WAIT_REINIT) )
            {
                RS232_WriteControlRegister(RS232_ReadControlRegister() &
                                              (uint8)~RS232_CTRL_HD_SEND_BREAK);
            }

        #else /* RS232_HD_ENABLED Full Duplex mode */

            static uint8 txPeriod;

            if( (retMode == RS232_SEND_BREAK) ||
                (retMode == RS232_SEND_WAIT_REINIT) )
            {
                /* CTRL_HD_SEND_BREAK - skip to send parity bit at Break signal in Full Duplex mode */
                #if( (RS232_PARITY_TYPE != RS232__B_UART__NONE_REVB) || \
                                    (RS232_PARITY_TYPE_SW != 0u) )
                    RS232_WriteControlRegister(RS232_ReadControlRegister() |
                                                          RS232_CTRL_HD_SEND_BREAK);
                #endif /* End RS232_PARITY_TYPE != RS232__B_UART__NONE_REVB  */

                #if(RS232_TXCLKGEN_DP)
                    txPeriod = RS232_TXBITCLKTX_COMPLETE_REG;
                    RS232_TXBITCLKTX_COMPLETE_REG = RS232_TXBITCTR_BREAKBITS;
                #else
                    txPeriod = RS232_TXBITCTR_PERIOD_REG;
                    RS232_TXBITCTR_PERIOD_REG = RS232_TXBITCTR_BREAKBITS8X;
                #endif /* End RS232_TXCLKGEN_DP */

                /* Send zeros */
                RS232_TXDATA_REG = 0u;

                do /* Wait until transmit starts */
                {
                    tmpStat = RS232_TXSTATUS_REG;
                }
                while((tmpStat & RS232_TX_STS_FIFO_EMPTY) != 0u);
            }

            if( (retMode == RS232_WAIT_FOR_COMPLETE_REINIT) ||
                (retMode == RS232_SEND_WAIT_REINIT) )
            {
                do /* Wait until transmit complete */
                {
                    tmpStat = RS232_TXSTATUS_REG;
                }
                while(((uint8)~tmpStat & RS232_TX_STS_COMPLETE) != 0u);
            }

            if( (retMode == RS232_WAIT_FOR_COMPLETE_REINIT) ||
                (retMode == RS232_REINIT) ||
                (retMode == RS232_SEND_WAIT_REINIT) )
            {

            #if(RS232_TXCLKGEN_DP)
                RS232_TXBITCLKTX_COMPLETE_REG = txPeriod;
            #else
                RS232_TXBITCTR_PERIOD_REG = txPeriod;
            #endif /* End RS232_TXCLKGEN_DP */

            #if( (RS232_PARITY_TYPE != RS232__B_UART__NONE_REVB) || \
                 (RS232_PARITY_TYPE_SW != 0u) )
                RS232_WriteControlRegister(RS232_ReadControlRegister() &
                                                      (uint8) ~RS232_CTRL_HD_SEND_BREAK);
            #endif /* End RS232_PARITY_TYPE != NONE */
            }
        #endif    /* End RS232_HD_ENABLED */
        }
    }


    /*******************************************************************************
    * Function Name: RS232_SetTxAddressMode
    ********************************************************************************
    *
    * Summary:
    *  Configures the transmitter to signal the next bytes is address or data.
    *
    * Parameters:
    *  addressMode: 
    *       RS232_SET_SPACE - Configure the transmitter to send the next
    *                                    byte as a data.
    *       RS232_SET_MARK  - Configure the transmitter to send the next
    *                                    byte as an address.
    *
    * Return:
    *  None.
    *
    * Side Effects:
    *  This function sets and clears RS232_CTRL_MARK bit in the Control
    *  register.
    *
    *******************************************************************************/
    void RS232_SetTxAddressMode(uint8 addressMode) 
    {
        /* Mark/Space sending enable */
        if(addressMode != 0u)
        {
        #if( RS232_CONTROL_REG_REMOVED == 0u )
            RS232_WriteControlRegister(RS232_ReadControlRegister() |
                                                  RS232_CTRL_MARK);
        #endif /* End RS232_CONTROL_REG_REMOVED == 0u */
        }
        else
        {
        #if( RS232_CONTROL_REG_REMOVED == 0u )
            RS232_WriteControlRegister(RS232_ReadControlRegister() &
                                                  (uint8) ~RS232_CTRL_MARK);
        #endif /* End RS232_CONTROL_REG_REMOVED == 0u */
        }
    }

#endif  /* EndRS232_TX_ENABLED */

#if(RS232_HD_ENABLED)


    /*******************************************************************************
    * Function Name: RS232_LoadRxConfig
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
    void RS232_LoadRxConfig(void) 
    {
        RS232_WriteControlRegister(RS232_ReadControlRegister() &
                                                (uint8)~RS232_CTRL_HD_SEND);
        RS232_RXBITCTR_PERIOD_REG = RS232_HD_RXBITCTR_INIT;

    #if (RS232_RX_INTERRUPT_ENABLED)
        /* Enable RX interrupt after set RX configuration */
        RS232_SetRxInterruptMode(RS232_INIT_RX_INTERRUPTS_MASK);
    #endif /* (RS232_RX_INTERRUPT_ENABLED) */
    }


    /*******************************************************************************
    * Function Name: RS232_LoadTxConfig
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
    void RS232_LoadTxConfig(void) 
    {
    #if (RS232_RX_INTERRUPT_ENABLED)
        /* Disable RX interrupts before set TX configuration */
        RS232_SetRxInterruptMode(0u);
    #endif /* (RS232_RX_INTERRUPT_ENABLED) */

        RS232_WriteControlRegister(RS232_ReadControlRegister() | RS232_CTRL_HD_SEND);
        RS232_RXBITCTR_PERIOD_REG = RS232_HD_TXBITCTR_INIT;
    }

#endif  /* RS232_HD_ENABLED */


/* [] END OF FILE */
