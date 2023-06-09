/*******************************************************************************
* File Name: Surtidor.c
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

#include "Surtidor.h"
#if (Surtidor_INTERNAL_CLOCK_USED)
    #include "Surtidor_IntClock.h"
#endif /* End Surtidor_INTERNAL_CLOCK_USED */


/***************************************
* Global data allocation
***************************************/

uint8 Surtidor_initVar = 0u;

#if (Surtidor_TX_INTERRUPT_ENABLED && Surtidor_TX_ENABLED)
    volatile uint8 Surtidor_txBuffer[Surtidor_TX_BUFFER_SIZE];
    volatile uint8 Surtidor_txBufferRead = 0u;
    uint8 Surtidor_txBufferWrite = 0u;
#endif /* (Surtidor_TX_INTERRUPT_ENABLED && Surtidor_TX_ENABLED) */

#if (Surtidor_RX_INTERRUPT_ENABLED && (Surtidor_RX_ENABLED || Surtidor_HD_ENABLED))
    uint8 Surtidor_errorStatus = 0u;
    volatile uint8 Surtidor_rxBuffer[Surtidor_RX_BUFFER_SIZE];
    volatile uint8 Surtidor_rxBufferRead  = 0u;
    volatile uint8 Surtidor_rxBufferWrite = 0u;
    volatile uint8 Surtidor_rxBufferLoopDetect = 0u;
    volatile uint8 Surtidor_rxBufferOverflow   = 0u;
    #if (Surtidor_RXHW_ADDRESS_ENABLED)
        volatile uint8 Surtidor_rxAddressMode = Surtidor_RX_ADDRESS_MODE;
        volatile uint8 Surtidor_rxAddressDetected = 0u;
    #endif /* (Surtidor_RXHW_ADDRESS_ENABLED) */
#endif /* (Surtidor_RX_INTERRUPT_ENABLED && (Surtidor_RX_ENABLED || Surtidor_HD_ENABLED)) */


/*******************************************************************************
* Function Name: Surtidor_Start
********************************************************************************
*
* Summary:
*  This is the preferred method to begin component operation.
*  Surtidor_Start() sets the initVar variable, calls the
*  Surtidor_Init() function, and then calls the
*  Surtidor_Enable() function.
*
* Parameters:
*  None.
*
* Return:
*  None.
*
* Global variables:
*  The Surtidor_intiVar variable is used to indicate initial
*  configuration of this component. The variable is initialized to zero (0u)
*  and set to one (1u) the first time Surtidor_Start() is called. This
*  allows for component initialization without re-initialization in all
*  subsequent calls to the Surtidor_Start() routine.
*
* Reentrant:
*  No.
*
*******************************************************************************/
void Surtidor_Start(void) 
{
    /* If not initialized then initialize all required hardware and software */
    if(Surtidor_initVar == 0u)
    {
        Surtidor_Init();
        Surtidor_initVar = 1u;
    }

    Surtidor_Enable();
}


/*******************************************************************************
* Function Name: Surtidor_Init
********************************************************************************
*
* Summary:
*  Initializes or restores the component according to the customizer Configure
*  dialog settings. It is not necessary to call Surtidor_Init() because
*  the Surtidor_Start() API calls this function and is the preferred
*  method to begin component operation.
*
* Parameters:
*  None.
*
* Return:
*  None.
*
*******************************************************************************/
void Surtidor_Init(void) 
{
    #if(Surtidor_RX_ENABLED || Surtidor_HD_ENABLED)

        #if (Surtidor_RX_INTERRUPT_ENABLED)
            /* Set RX interrupt vector and priority */
            (void) CyIntSetVector(Surtidor_RX_VECT_NUM, &Surtidor_RXISR);
            CyIntSetPriority(Surtidor_RX_VECT_NUM, Surtidor_RX_PRIOR_NUM);
            Surtidor_errorStatus = 0u;
        #endif /* (Surtidor_RX_INTERRUPT_ENABLED) */

        #if (Surtidor_RXHW_ADDRESS_ENABLED)
            Surtidor_SetRxAddressMode(Surtidor_RX_ADDRESS_MODE);
            Surtidor_SetRxAddress1(Surtidor_RX_HW_ADDRESS1);
            Surtidor_SetRxAddress2(Surtidor_RX_HW_ADDRESS2);
        #endif /* End Surtidor_RXHW_ADDRESS_ENABLED */

        /* Init Count7 period */
        Surtidor_RXBITCTR_PERIOD_REG = Surtidor_RXBITCTR_INIT;
        /* Configure the Initial RX interrupt mask */
        Surtidor_RXSTATUS_MASK_REG  = Surtidor_INIT_RX_INTERRUPTS_MASK;
    #endif /* End Surtidor_RX_ENABLED || Surtidor_HD_ENABLED*/

    #if(Surtidor_TX_ENABLED)
        #if (Surtidor_TX_INTERRUPT_ENABLED)
            /* Set TX interrupt vector and priority */
            (void) CyIntSetVector(Surtidor_TX_VECT_NUM, &Surtidor_TXISR);
            CyIntSetPriority(Surtidor_TX_VECT_NUM, Surtidor_TX_PRIOR_NUM);
        #endif /* (Surtidor_TX_INTERRUPT_ENABLED) */

        /* Write Counter Value for TX Bit Clk Generator*/
        #if (Surtidor_TXCLKGEN_DP)
            Surtidor_TXBITCLKGEN_CTR_REG = Surtidor_BIT_CENTER;
            Surtidor_TXBITCLKTX_COMPLETE_REG = ((Surtidor_NUMBER_OF_DATA_BITS +
                        Surtidor_NUMBER_OF_START_BIT) * Surtidor_OVER_SAMPLE_COUNT) - 1u;
        #else
            Surtidor_TXBITCTR_PERIOD_REG = ((Surtidor_NUMBER_OF_DATA_BITS +
                        Surtidor_NUMBER_OF_START_BIT) * Surtidor_OVER_SAMPLE_8) - 1u;
        #endif /* End Surtidor_TXCLKGEN_DP */

        /* Configure the Initial TX interrupt mask */
        #if (Surtidor_TX_INTERRUPT_ENABLED)
            Surtidor_TXSTATUS_MASK_REG = Surtidor_TX_STS_FIFO_EMPTY;
        #else
            Surtidor_TXSTATUS_MASK_REG = Surtidor_INIT_TX_INTERRUPTS_MASK;
        #endif /*End Surtidor_TX_INTERRUPT_ENABLED*/

    #endif /* End Surtidor_TX_ENABLED */

    #if(Surtidor_PARITY_TYPE_SW)  /* Write Parity to Control Register */
        Surtidor_WriteControlRegister( \
            (Surtidor_ReadControlRegister() & (uint8)~Surtidor_CTRL_PARITY_TYPE_MASK) | \
            (uint8)(Surtidor_PARITY_TYPE << Surtidor_CTRL_PARITY_TYPE0_SHIFT) );
    #endif /* End Surtidor_PARITY_TYPE_SW */
}


/*******************************************************************************
* Function Name: Surtidor_Enable
********************************************************************************
*
* Summary:
*  Activates the hardware and begins component operation. It is not necessary
*  to call Surtidor_Enable() because the Surtidor_Start() API
*  calls this function, which is the preferred method to begin component
*  operation.

* Parameters:
*  None.
*
* Return:
*  None.
*
* Global Variables:
*  Surtidor_rxAddressDetected - set to initial state (0).
*
*******************************************************************************/
void Surtidor_Enable(void) 
{
    uint8 enableInterrupts;
    enableInterrupts = CyEnterCriticalSection();

    #if (Surtidor_RX_ENABLED || Surtidor_HD_ENABLED)
        /* RX Counter (Count7) Enable */
        Surtidor_RXBITCTR_CONTROL_REG |= Surtidor_CNTR_ENABLE;

        /* Enable the RX Interrupt */
        Surtidor_RXSTATUS_ACTL_REG  |= Surtidor_INT_ENABLE;

        #if (Surtidor_RX_INTERRUPT_ENABLED)
            Surtidor_EnableRxInt();

            #if (Surtidor_RXHW_ADDRESS_ENABLED)
                Surtidor_rxAddressDetected = 0u;
            #endif /* (Surtidor_RXHW_ADDRESS_ENABLED) */
        #endif /* (Surtidor_RX_INTERRUPT_ENABLED) */
    #endif /* (Surtidor_RX_ENABLED || Surtidor_HD_ENABLED) */

    #if(Surtidor_TX_ENABLED)
        /* TX Counter (DP/Count7) Enable */
        #if(!Surtidor_TXCLKGEN_DP)
            Surtidor_TXBITCTR_CONTROL_REG |= Surtidor_CNTR_ENABLE;
        #endif /* End Surtidor_TXCLKGEN_DP */

        /* Enable the TX Interrupt */
        Surtidor_TXSTATUS_ACTL_REG |= Surtidor_INT_ENABLE;
        #if (Surtidor_TX_INTERRUPT_ENABLED)
            Surtidor_ClearPendingTxInt(); /* Clear history of TX_NOT_EMPTY */
            Surtidor_EnableTxInt();
        #endif /* (Surtidor_TX_INTERRUPT_ENABLED) */
     #endif /* (Surtidor_TX_INTERRUPT_ENABLED) */

    #if (Surtidor_INTERNAL_CLOCK_USED)
        Surtidor_IntClock_Start();  /* Enable the clock */
    #endif /* (Surtidor_INTERNAL_CLOCK_USED) */

    CyExitCriticalSection(enableInterrupts);
}


/*******************************************************************************
* Function Name: Surtidor_Stop
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
void Surtidor_Stop(void) 
{
    uint8 enableInterrupts;
    enableInterrupts = CyEnterCriticalSection();

    /* Write Bit Counter Disable */
    #if (Surtidor_RX_ENABLED || Surtidor_HD_ENABLED)
        Surtidor_RXBITCTR_CONTROL_REG &= (uint8) ~Surtidor_CNTR_ENABLE;
    #endif /* (Surtidor_RX_ENABLED || Surtidor_HD_ENABLED) */

    #if (Surtidor_TX_ENABLED)
        #if(!Surtidor_TXCLKGEN_DP)
            Surtidor_TXBITCTR_CONTROL_REG &= (uint8) ~Surtidor_CNTR_ENABLE;
        #endif /* (!Surtidor_TXCLKGEN_DP) */
    #endif /* (Surtidor_TX_ENABLED) */

    #if (Surtidor_INTERNAL_CLOCK_USED)
        Surtidor_IntClock_Stop();   /* Disable the clock */
    #endif /* (Surtidor_INTERNAL_CLOCK_USED) */

    /* Disable internal interrupt component */
    #if (Surtidor_RX_ENABLED || Surtidor_HD_ENABLED)
        Surtidor_RXSTATUS_ACTL_REG  &= (uint8) ~Surtidor_INT_ENABLE;

        #if (Surtidor_RX_INTERRUPT_ENABLED)
            Surtidor_DisableRxInt();
        #endif /* (Surtidor_RX_INTERRUPT_ENABLED) */
    #endif /* (Surtidor_RX_ENABLED || Surtidor_HD_ENABLED) */

    #if (Surtidor_TX_ENABLED)
        Surtidor_TXSTATUS_ACTL_REG &= (uint8) ~Surtidor_INT_ENABLE;

        #if (Surtidor_TX_INTERRUPT_ENABLED)
            Surtidor_DisableTxInt();
        #endif /* (Surtidor_TX_INTERRUPT_ENABLED) */
    #endif /* (Surtidor_TX_ENABLED) */

    CyExitCriticalSection(enableInterrupts);
}


/*******************************************************************************
* Function Name: Surtidor_ReadControlRegister
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
uint8 Surtidor_ReadControlRegister(void) 
{
    #if (Surtidor_CONTROL_REG_REMOVED)
        return(0u);
    #else
        return(Surtidor_CONTROL_REG);
    #endif /* (Surtidor_CONTROL_REG_REMOVED) */
}


/*******************************************************************************
* Function Name: Surtidor_WriteControlRegister
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
void  Surtidor_WriteControlRegister(uint8 control) 
{
    #if (Surtidor_CONTROL_REG_REMOVED)
        if(0u != control)
        {
            /* Suppress compiler warning */
        }
    #else
       Surtidor_CONTROL_REG = control;
    #endif /* (Surtidor_CONTROL_REG_REMOVED) */
}


#if(Surtidor_RX_ENABLED || Surtidor_HD_ENABLED)
    /*******************************************************************************
    * Function Name: Surtidor_SetRxInterruptMode
    ********************************************************************************
    *
    * Summary:
    *  Configures the RX interrupt sources enabled.
    *
    * Parameters:
    *  IntSrc:  Bit field containing the RX interrupts to enable. Based on the 
    *  bit-field arrangement of the status register. This value must be a 
    *  combination of status register bit-masks shown below:
    *      Surtidor_RX_STS_FIFO_NOTEMPTY    Interrupt on byte received.
    *      Surtidor_RX_STS_PAR_ERROR        Interrupt on parity error.
    *      Surtidor_RX_STS_STOP_ERROR       Interrupt on stop error.
    *      Surtidor_RX_STS_BREAK            Interrupt on break.
    *      Surtidor_RX_STS_OVERRUN          Interrupt on overrun error.
    *      Surtidor_RX_STS_ADDR_MATCH       Interrupt on address match.
    *      Surtidor_RX_STS_MRKSPC           Interrupt on address detect.
    *
    * Return:
    *  None.
    *
    * Theory:
    *  Enables the output of specific status bits to the interrupt controller
    *
    *******************************************************************************/
    void Surtidor_SetRxInterruptMode(uint8 intSrc) 
    {
        Surtidor_RXSTATUS_MASK_REG  = intSrc;
    }


    /*******************************************************************************
    * Function Name: Surtidor_ReadRxData
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
    *  Surtidor_rxBuffer - RAM buffer pointer for save received data.
    *  Surtidor_rxBufferWrite - cyclic index for write to rxBuffer,
    *     checked to identify new data.
    *  Surtidor_rxBufferRead - cyclic index for read from rxBuffer,
    *     incremented after each byte has been read from buffer.
    *  Surtidor_rxBufferLoopDetect - cleared if loop condition was detected
    *     in RX ISR.
    *
    * Reentrant:
    *  No.
    *
    *******************************************************************************/
    uint8 Surtidor_ReadRxData(void) 
    {
        uint8 rxData;

    #if (Surtidor_RX_INTERRUPT_ENABLED)

        uint8 locRxBufferRead;
        uint8 locRxBufferWrite;

        /* Protect variables that could change on interrupt */
        Surtidor_DisableRxInt();

        locRxBufferRead  = Surtidor_rxBufferRead;
        locRxBufferWrite = Surtidor_rxBufferWrite;

        if( (Surtidor_rxBufferLoopDetect != 0u) || (locRxBufferRead != locRxBufferWrite) )
        {
            rxData = Surtidor_rxBuffer[locRxBufferRead];
            locRxBufferRead++;

            if(locRxBufferRead >= Surtidor_RX_BUFFER_SIZE)
            {
                locRxBufferRead = 0u;
            }
            /* Update the real pointer */
            Surtidor_rxBufferRead = locRxBufferRead;

            if(Surtidor_rxBufferLoopDetect != 0u)
            {
                Surtidor_rxBufferLoopDetect = 0u;
                #if ((Surtidor_RX_INTERRUPT_ENABLED) && (Surtidor_FLOW_CONTROL != 0u))
                    /* When Hardware Flow Control selected - return RX mask */
                    #if( Surtidor_HD_ENABLED )
                        if((Surtidor_CONTROL_REG & Surtidor_CTRL_HD_SEND) == 0u)
                        {   /* In Half duplex mode return RX mask only in RX
                            *  configuration set, otherwise
                            *  mask will be returned in LoadRxConfig() API.
                            */
                            Surtidor_RXSTATUS_MASK_REG  |= Surtidor_RX_STS_FIFO_NOTEMPTY;
                        }
                    #else
                        Surtidor_RXSTATUS_MASK_REG  |= Surtidor_RX_STS_FIFO_NOTEMPTY;
                    #endif /* end Surtidor_HD_ENABLED */
                #endif /* ((Surtidor_RX_INTERRUPT_ENABLED) && (Surtidor_FLOW_CONTROL != 0u)) */
            }
        }
        else
        {   /* Needs to check status for RX_STS_FIFO_NOTEMPTY bit */
            rxData = Surtidor_RXDATA_REG;
        }

        Surtidor_EnableRxInt();

    #else

        /* Needs to check status for RX_STS_FIFO_NOTEMPTY bit */
        rxData = Surtidor_RXDATA_REG;

    #endif /* (Surtidor_RX_INTERRUPT_ENABLED) */

        return(rxData);
    }


    /*******************************************************************************
    * Function Name: Surtidor_ReadRxStatus
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
    *  Surtidor_RX_STS_FIFO_NOTEMPTY.
    *  Surtidor_RX_STS_FIFO_NOTEMPTY clears immediately after RX data
    *  register read.
    *
    * Global Variables:
    *  Surtidor_rxBufferOverflow - used to indicate overload condition.
    *   It set to one in RX interrupt when there isn't free space in
    *   Surtidor_rxBufferRead to write new data. This condition returned
    *   and cleared to zero by this API as an
    *   Surtidor_RX_STS_SOFT_BUFF_OVER bit along with RX Status register
    *   bits.
    *
    *******************************************************************************/
    uint8 Surtidor_ReadRxStatus(void) 
    {
        uint8 status;

        status = Surtidor_RXSTATUS_REG & Surtidor_RX_HW_MASK;

    #if (Surtidor_RX_INTERRUPT_ENABLED)
        if(Surtidor_rxBufferOverflow != 0u)
        {
            status |= Surtidor_RX_STS_SOFT_BUFF_OVER;
            Surtidor_rxBufferOverflow = 0u;
        }
    #endif /* (Surtidor_RX_INTERRUPT_ENABLED) */

        return(status);
    }


    /*******************************************************************************
    * Function Name: Surtidor_GetChar
    ********************************************************************************
    *
    * Summary:
    *  Returns the last received byte of data. Surtidor_GetChar() is
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
    *  Surtidor_rxBuffer - RAM buffer pointer for save received data.
    *  Surtidor_rxBufferWrite - cyclic index for write to rxBuffer,
    *     checked to identify new data.
    *  Surtidor_rxBufferRead - cyclic index for read from rxBuffer,
    *     incremented after each byte has been read from buffer.
    *  Surtidor_rxBufferLoopDetect - cleared if loop condition was detected
    *     in RX ISR.
    *
    * Reentrant:
    *  No.
    *
    *******************************************************************************/
    uint8 Surtidor_GetChar(void) 
    {
        uint8 rxData = 0u;
        uint8 rxStatus;

    #if (Surtidor_RX_INTERRUPT_ENABLED)
        uint8 locRxBufferRead;
        uint8 locRxBufferWrite;

        /* Protect variables that could change on interrupt */
        Surtidor_DisableRxInt();

        locRxBufferRead  = Surtidor_rxBufferRead;
        locRxBufferWrite = Surtidor_rxBufferWrite;

        if( (Surtidor_rxBufferLoopDetect != 0u) || (locRxBufferRead != locRxBufferWrite) )
        {
            rxData = Surtidor_rxBuffer[locRxBufferRead];
            locRxBufferRead++;
            if(locRxBufferRead >= Surtidor_RX_BUFFER_SIZE)
            {
                locRxBufferRead = 0u;
            }
            /* Update the real pointer */
            Surtidor_rxBufferRead = locRxBufferRead;

            if(Surtidor_rxBufferLoopDetect != 0u)
            {
                Surtidor_rxBufferLoopDetect = 0u;
                #if( (Surtidor_RX_INTERRUPT_ENABLED) && (Surtidor_FLOW_CONTROL != 0u) )
                    /* When Hardware Flow Control selected - return RX mask */
                    #if( Surtidor_HD_ENABLED )
                        if((Surtidor_CONTROL_REG & Surtidor_CTRL_HD_SEND) == 0u)
                        {   /* In Half duplex mode return RX mask only if
                            *  RX configuration set, otherwise
                            *  mask will be returned in LoadRxConfig() API.
                            */
                            Surtidor_RXSTATUS_MASK_REG |= Surtidor_RX_STS_FIFO_NOTEMPTY;
                        }
                    #else
                        Surtidor_RXSTATUS_MASK_REG |= Surtidor_RX_STS_FIFO_NOTEMPTY;
                    #endif /* end Surtidor_HD_ENABLED */
                #endif /* Surtidor_RX_INTERRUPT_ENABLED and Hardware flow control*/
            }

        }
        else
        {   rxStatus = Surtidor_RXSTATUS_REG;
            if((rxStatus & Surtidor_RX_STS_FIFO_NOTEMPTY) != 0u)
            {   /* Read received data from FIFO */
                rxData = Surtidor_RXDATA_REG;
                /*Check status on error*/
                if((rxStatus & (Surtidor_RX_STS_BREAK | Surtidor_RX_STS_PAR_ERROR |
                                Surtidor_RX_STS_STOP_ERROR | Surtidor_RX_STS_OVERRUN)) != 0u)
                {
                    rxData = 0u;
                }
            }
        }

        Surtidor_EnableRxInt();

    #else

        rxStatus =Surtidor_RXSTATUS_REG;
        if((rxStatus & Surtidor_RX_STS_FIFO_NOTEMPTY) != 0u)
        {
            /* Read received data from FIFO */
            rxData = Surtidor_RXDATA_REG;

            /*Check status on error*/
            if((rxStatus & (Surtidor_RX_STS_BREAK | Surtidor_RX_STS_PAR_ERROR |
                            Surtidor_RX_STS_STOP_ERROR | Surtidor_RX_STS_OVERRUN)) != 0u)
            {
                rxData = 0u;
            }
        }
    #endif /* (Surtidor_RX_INTERRUPT_ENABLED) */

        return(rxData);
    }


    /*******************************************************************************
    * Function Name: Surtidor_GetByte
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
    uint16 Surtidor_GetByte(void) 
    {
        
    #if (Surtidor_RX_INTERRUPT_ENABLED)
        uint16 locErrorStatus;
        /* Protect variables that could change on interrupt */
        Surtidor_DisableRxInt();
        locErrorStatus = (uint16)Surtidor_errorStatus;
        Surtidor_errorStatus = 0u;
        Surtidor_EnableRxInt();
        return ( (uint16)(locErrorStatus << 8u) | Surtidor_ReadRxData() );
    #else
        return ( ((uint16)Surtidor_ReadRxStatus() << 8u) | Surtidor_ReadRxData() );
    #endif /* Surtidor_RX_INTERRUPT_ENABLED */
        
    }


    /*******************************************************************************
    * Function Name: Surtidor_GetRxBufferSize
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
    *  Surtidor_rxBufferWrite - used to calculate left bytes.
    *  Surtidor_rxBufferRead - used to calculate left bytes.
    *  Surtidor_rxBufferLoopDetect - checked to decide left bytes amount.
    *
    * Reentrant:
    *  No.
    *
    * Theory:
    *  Allows the user to find out how full the RX Buffer is.
    *
    *******************************************************************************/
    uint8 Surtidor_GetRxBufferSize(void)
                                                            
    {
        uint8 size;

    #if (Surtidor_RX_INTERRUPT_ENABLED)

        /* Protect variables that could change on interrupt */
        Surtidor_DisableRxInt();

        if(Surtidor_rxBufferRead == Surtidor_rxBufferWrite)
        {
            if(Surtidor_rxBufferLoopDetect != 0u)
            {
                size = Surtidor_RX_BUFFER_SIZE;
            }
            else
            {
                size = 0u;
            }
        }
        else if(Surtidor_rxBufferRead < Surtidor_rxBufferWrite)
        {
            size = (Surtidor_rxBufferWrite - Surtidor_rxBufferRead);
        }
        else
        {
            size = (Surtidor_RX_BUFFER_SIZE - Surtidor_rxBufferRead) + Surtidor_rxBufferWrite;
        }

        Surtidor_EnableRxInt();

    #else

        /* We can only know if there is data in the fifo. */
        size = ((Surtidor_RXSTATUS_REG & Surtidor_RX_STS_FIFO_NOTEMPTY) != 0u) ? 1u : 0u;

    #endif /* (Surtidor_RX_INTERRUPT_ENABLED) */

        return(size);
    }


    /*******************************************************************************
    * Function Name: Surtidor_ClearRxBuffer
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
    *  Surtidor_rxBufferWrite - cleared to zero.
    *  Surtidor_rxBufferRead - cleared to zero.
    *  Surtidor_rxBufferLoopDetect - cleared to zero.
    *  Surtidor_rxBufferOverflow - cleared to zero.
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
    void Surtidor_ClearRxBuffer(void) 
    {
        uint8 enableInterrupts;

        /* Clear the HW FIFO */
        enableInterrupts = CyEnterCriticalSection();
        Surtidor_RXDATA_AUX_CTL_REG |= (uint8)  Surtidor_RX_FIFO_CLR;
        Surtidor_RXDATA_AUX_CTL_REG &= (uint8) ~Surtidor_RX_FIFO_CLR;
        CyExitCriticalSection(enableInterrupts);

    #if (Surtidor_RX_INTERRUPT_ENABLED)

        /* Protect variables that could change on interrupt. */
        Surtidor_DisableRxInt();

        Surtidor_rxBufferRead = 0u;
        Surtidor_rxBufferWrite = 0u;
        Surtidor_rxBufferLoopDetect = 0u;
        Surtidor_rxBufferOverflow = 0u;

        Surtidor_EnableRxInt();

    #endif /* (Surtidor_RX_INTERRUPT_ENABLED) */

    }


    /*******************************************************************************
    * Function Name: Surtidor_SetRxAddressMode
    ********************************************************************************
    *
    * Summary:
    *  Sets the software controlled Addressing mode used by the RX portion of the
    *  UART.
    *
    * Parameters:
    *  addressMode: Enumerated value indicating the mode of RX addressing
    *  Surtidor__B_UART__AM_SW_BYTE_BYTE -  Software Byte-by-Byte address
    *                                               detection
    *  Surtidor__B_UART__AM_SW_DETECT_TO_BUFFER - Software Detect to Buffer
    *                                               address detection
    *  Surtidor__B_UART__AM_HW_BYTE_BY_BYTE - Hardware Byte-by-Byte address
    *                                               detection
    *  Surtidor__B_UART__AM_HW_DETECT_TO_BUFFER - Hardware Detect to Buffer
    *                                               address detection
    *  Surtidor__B_UART__AM_NONE - No address detection
    *
    * Return:
    *  None.
    *
    * Global Variables:
    *  Surtidor_rxAddressMode - the parameter stored in this variable for
    *   the farther usage in RX ISR.
    *  Surtidor_rxAddressDetected - set to initial state (0).
    *
    *******************************************************************************/
    void Surtidor_SetRxAddressMode(uint8 addressMode)
                                                        
    {
        #if(Surtidor_RXHW_ADDRESS_ENABLED)
            #if(Surtidor_CONTROL_REG_REMOVED)
                if(0u != addressMode)
                {
                    /* Suppress compiler warning */
                }
            #else /* Surtidor_CONTROL_REG_REMOVED */
                uint8 tmpCtrl;
                tmpCtrl = Surtidor_CONTROL_REG & (uint8)~Surtidor_CTRL_RXADDR_MODE_MASK;
                tmpCtrl |= (uint8)(addressMode << Surtidor_CTRL_RXADDR_MODE0_SHIFT);
                Surtidor_CONTROL_REG = tmpCtrl;

                #if(Surtidor_RX_INTERRUPT_ENABLED && \
                   (Surtidor_RXBUFFERSIZE > Surtidor_FIFO_LENGTH) )
                    Surtidor_rxAddressMode = addressMode;
                    Surtidor_rxAddressDetected = 0u;
                #endif /* End Surtidor_RXBUFFERSIZE > Surtidor_FIFO_LENGTH*/
            #endif /* End Surtidor_CONTROL_REG_REMOVED */
        #else /* Surtidor_RXHW_ADDRESS_ENABLED */
            if(0u != addressMode)
            {
                /* Suppress compiler warning */
            }
        #endif /* End Surtidor_RXHW_ADDRESS_ENABLED */
    }


    /*******************************************************************************
    * Function Name: Surtidor_SetRxAddress1
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
    void Surtidor_SetRxAddress1(uint8 address) 
    {
        Surtidor_RXADDRESS1_REG = address;
    }


    /*******************************************************************************
    * Function Name: Surtidor_SetRxAddress2
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
    void Surtidor_SetRxAddress2(uint8 address) 
    {
        Surtidor_RXADDRESS2_REG = address;
    }

#endif  /* Surtidor_RX_ENABLED || Surtidor_HD_ENABLED*/


#if( (Surtidor_TX_ENABLED) || (Surtidor_HD_ENABLED) )
    /*******************************************************************************
    * Function Name: Surtidor_SetTxInterruptMode
    ********************************************************************************
    *
    * Summary:
    *  Configures the TX interrupt sources to be enabled, but does not enable the
    *  interrupt.
    *
    * Parameters:
    *  intSrc: Bit field containing the TX interrupt sources to enable
    *   Surtidor_TX_STS_COMPLETE        Interrupt on TX byte complete
    *   Surtidor_TX_STS_FIFO_EMPTY      Interrupt when TX FIFO is empty
    *   Surtidor_TX_STS_FIFO_FULL       Interrupt when TX FIFO is full
    *   Surtidor_TX_STS_FIFO_NOT_FULL   Interrupt when TX FIFO is not full
    *
    * Return:
    *  None.
    *
    * Theory:
    *  Enables the output of specific status bits to the interrupt controller
    *
    *******************************************************************************/
    void Surtidor_SetTxInterruptMode(uint8 intSrc) 
    {
        Surtidor_TXSTATUS_MASK_REG = intSrc;
    }


    /*******************************************************************************
    * Function Name: Surtidor_WriteTxData
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
    *  Surtidor_txBuffer - RAM buffer pointer for save data for transmission
    *  Surtidor_txBufferWrite - cyclic index for write to txBuffer,
    *    incremented after each byte saved to buffer.
    *  Surtidor_txBufferRead - cyclic index for read from txBuffer,
    *    checked to identify the condition to write to FIFO directly or to TX buffer
    *  Surtidor_initVar - checked to identify that the component has been
    *    initialized.
    *
    * Reentrant:
    *  No.
    *
    *******************************************************************************/
    void Surtidor_WriteTxData(uint8 txDataByte) 
    {
        /* If not Initialized then skip this function*/
        if(Surtidor_initVar != 0u)
        {
        #if (Surtidor_TX_INTERRUPT_ENABLED)

            /* Protect variables that could change on interrupt. */
            Surtidor_DisableTxInt();

            if( (Surtidor_txBufferRead == Surtidor_txBufferWrite) &&
                ((Surtidor_TXSTATUS_REG & Surtidor_TX_STS_FIFO_FULL) == 0u) )
            {
                /* Add directly to the FIFO. */
                Surtidor_TXDATA_REG = txDataByte;
            }
            else
            {
                if(Surtidor_txBufferWrite >= Surtidor_TX_BUFFER_SIZE)
                {
                    Surtidor_txBufferWrite = 0u;
                }

                Surtidor_txBuffer[Surtidor_txBufferWrite] = txDataByte;

                /* Add to the software buffer. */
                Surtidor_txBufferWrite++;
            }

            Surtidor_EnableTxInt();

        #else

            /* Add directly to the FIFO. */
            Surtidor_TXDATA_REG = txDataByte;

        #endif /*(Surtidor_TX_INTERRUPT_ENABLED) */
        }
    }


    /*******************************************************************************
    * Function Name: Surtidor_ReadTxStatus
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
    uint8 Surtidor_ReadTxStatus(void) 
    {
        return(Surtidor_TXSTATUS_REG);
    }


    /*******************************************************************************
    * Function Name: Surtidor_PutChar
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
    *  Surtidor_txBuffer - RAM buffer pointer for save data for transmission
    *  Surtidor_txBufferWrite - cyclic index for write to txBuffer,
    *     checked to identify free space in txBuffer and incremented after each byte
    *     saved to buffer.
    *  Surtidor_txBufferRead - cyclic index for read from txBuffer,
    *     checked to identify free space in txBuffer.
    *  Surtidor_initVar - checked to identify that the component has been
    *     initialized.
    *
    * Reentrant:
    *  No.
    *
    * Theory:
    *  Allows the user to transmit any byte of data in a single transfer
    *
    *******************************************************************************/
    void Surtidor_PutChar(uint8 txDataByte) 
    {
    #if (Surtidor_TX_INTERRUPT_ENABLED)
        /* The temporary output pointer is used since it takes two instructions
        *  to increment with a wrap, and we can't risk doing that with the real
        *  pointer and getting an interrupt in between instructions.
        */
        uint8 locTxBufferWrite;
        uint8 locTxBufferRead;

        do
        { /* Block if software buffer is full, so we don't overwrite. */

        #if ((Surtidor_TX_BUFFER_SIZE > Surtidor_MAX_BYTE_VALUE) && (CY_PSOC3))
            /* Disable TX interrupt to protect variables from modification */
            Surtidor_DisableTxInt();
        #endif /* (Surtidor_TX_BUFFER_SIZE > Surtidor_MAX_BYTE_VALUE) && (CY_PSOC3) */

            locTxBufferWrite = Surtidor_txBufferWrite;
            locTxBufferRead  = Surtidor_txBufferRead;

        #if ((Surtidor_TX_BUFFER_SIZE > Surtidor_MAX_BYTE_VALUE) && (CY_PSOC3))
            /* Enable interrupt to continue transmission */
            Surtidor_EnableTxInt();
        #endif /* (Surtidor_TX_BUFFER_SIZE > Surtidor_MAX_BYTE_VALUE) && (CY_PSOC3) */
        }
        while( (locTxBufferWrite < locTxBufferRead) ? (locTxBufferWrite == (locTxBufferRead - 1u)) :
                                ((locTxBufferWrite - locTxBufferRead) ==
                                (uint8)(Surtidor_TX_BUFFER_SIZE - 1u)) );

        if( (locTxBufferRead == locTxBufferWrite) &&
            ((Surtidor_TXSTATUS_REG & Surtidor_TX_STS_FIFO_FULL) == 0u) )
        {
            /* Add directly to the FIFO */
            Surtidor_TXDATA_REG = txDataByte;
        }
        else
        {
            if(locTxBufferWrite >= Surtidor_TX_BUFFER_SIZE)
            {
                locTxBufferWrite = 0u;
            }
            /* Add to the software buffer. */
            Surtidor_txBuffer[locTxBufferWrite] = txDataByte;
            locTxBufferWrite++;

            /* Finally, update the real output pointer */
        #if ((Surtidor_TX_BUFFER_SIZE > Surtidor_MAX_BYTE_VALUE) && (CY_PSOC3))
            Surtidor_DisableTxInt();
        #endif /* (Surtidor_TX_BUFFER_SIZE > Surtidor_MAX_BYTE_VALUE) && (CY_PSOC3) */

            Surtidor_txBufferWrite = locTxBufferWrite;

        #if ((Surtidor_TX_BUFFER_SIZE > Surtidor_MAX_BYTE_VALUE) && (CY_PSOC3))
            Surtidor_EnableTxInt();
        #endif /* (Surtidor_TX_BUFFER_SIZE > Surtidor_MAX_BYTE_VALUE) && (CY_PSOC3) */

            if(0u != (Surtidor_TXSTATUS_REG & Surtidor_TX_STS_FIFO_EMPTY))
            {
                /* Trigger TX interrupt to send software buffer */
                Surtidor_SetPendingTxInt();
            }
        }

    #else

        while((Surtidor_TXSTATUS_REG & Surtidor_TX_STS_FIFO_FULL) != 0u)
        {
            /* Wait for room in the FIFO */
        }

        /* Add directly to the FIFO */
        Surtidor_TXDATA_REG = txDataByte;

    #endif /* Surtidor_TX_INTERRUPT_ENABLED */
    }


    /*******************************************************************************
    * Function Name: Surtidor_PutString
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
    *  Surtidor_initVar - checked to identify that the component has been
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
    void Surtidor_PutString(const char8 string[]) 
    {
        uint16 bufIndex = 0u;

        /* If not Initialized then skip this function */
        if(Surtidor_initVar != 0u)
        {
            /* This is a blocking function, it will not exit until all data is sent */
            while(string[bufIndex] != (char8) 0)
            {
                Surtidor_PutChar((uint8)string[bufIndex]);
                bufIndex++;
            }
        }
    }


    /*******************************************************************************
    * Function Name: Surtidor_PutArray
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
    *  Surtidor_initVar - checked to identify that the component has been
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
    void Surtidor_PutArray(const uint8 string[], uint8 byteCount)
                                                                    
    {
        uint8 bufIndex = 0u;

        /* If not Initialized then skip this function */
        if(Surtidor_initVar != 0u)
        {
            while(bufIndex < byteCount)
            {
                Surtidor_PutChar(string[bufIndex]);
                bufIndex++;
            }
        }
    }


    /*******************************************************************************
    * Function Name: Surtidor_PutCRLF
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
    *  Surtidor_initVar - checked to identify that the component has been
    *     initialized.
    *
    * Reentrant:
    *  No.
    *
    *******************************************************************************/
    void Surtidor_PutCRLF(uint8 txDataByte) 
    {
        /* If not Initialized then skip this function */
        if(Surtidor_initVar != 0u)
        {
            Surtidor_PutChar(txDataByte);
            Surtidor_PutChar(0x0Du);
            Surtidor_PutChar(0x0Au);
        }
    }


    /*******************************************************************************
    * Function Name: Surtidor_GetTxBufferSize
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
    *  Surtidor_txBufferWrite - used to calculate left space.
    *  Surtidor_txBufferRead - used to calculate left space.
    *
    * Reentrant:
    *  No.
    *
    * Theory:
    *  Allows the user to find out how full the TX Buffer is.
    *
    *******************************************************************************/
    uint8 Surtidor_GetTxBufferSize(void)
                                                            
    {
        uint8 size;

    #if (Surtidor_TX_INTERRUPT_ENABLED)

        /* Protect variables that could change on interrupt. */
        Surtidor_DisableTxInt();

        if(Surtidor_txBufferRead == Surtidor_txBufferWrite)
        {
            size = 0u;
        }
        else if(Surtidor_txBufferRead < Surtidor_txBufferWrite)
        {
            size = (Surtidor_txBufferWrite - Surtidor_txBufferRead);
        }
        else
        {
            size = (Surtidor_TX_BUFFER_SIZE - Surtidor_txBufferRead) +
                    Surtidor_txBufferWrite;
        }

        Surtidor_EnableTxInt();

    #else

        size = Surtidor_TXSTATUS_REG;

        /* Is the fifo is full. */
        if((size & Surtidor_TX_STS_FIFO_FULL) != 0u)
        {
            size = Surtidor_FIFO_LENGTH;
        }
        else if((size & Surtidor_TX_STS_FIFO_EMPTY) != 0u)
        {
            size = 0u;
        }
        else
        {
            /* We only know there is data in the fifo. */
            size = 1u;
        }

    #endif /* (Surtidor_TX_INTERRUPT_ENABLED) */

    return(size);
    }


    /*******************************************************************************
    * Function Name: Surtidor_ClearTxBuffer
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
    *  Surtidor_txBufferWrite - cleared to zero.
    *  Surtidor_txBufferRead - cleared to zero.
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
    void Surtidor_ClearTxBuffer(void) 
    {
        uint8 enableInterrupts;

        enableInterrupts = CyEnterCriticalSection();
        /* Clear the HW FIFO */
        Surtidor_TXDATA_AUX_CTL_REG |= (uint8)  Surtidor_TX_FIFO_CLR;
        Surtidor_TXDATA_AUX_CTL_REG &= (uint8) ~Surtidor_TX_FIFO_CLR;
        CyExitCriticalSection(enableInterrupts);

    #if (Surtidor_TX_INTERRUPT_ENABLED)

        /* Protect variables that could change on interrupt. */
        Surtidor_DisableTxInt();

        Surtidor_txBufferRead = 0u;
        Surtidor_txBufferWrite = 0u;

        /* Enable Tx interrupt. */
        Surtidor_EnableTxInt();

    #endif /* (Surtidor_TX_INTERRUPT_ENABLED) */
    }


    /*******************************************************************************
    * Function Name: Surtidor_SendBreak
    ********************************************************************************
    *
    * Summary:
    *  Transmits a break signal on the bus.
    *
    * Parameters:
    *  uint8 retMode:  Send Break return mode. See the following table for options.
    *   Surtidor_SEND_BREAK - Initialize registers for break, send the Break
    *       signal and return immediately.
    *   Surtidor_WAIT_FOR_COMPLETE_REINIT - Wait until break transmission is
    *       complete, reinitialize registers to normal transmission mode then return
    *   Surtidor_REINIT - Reinitialize registers to normal transmission mode
    *       then return.
    *   Surtidor_SEND_WAIT_REINIT - Performs both options: 
    *      Surtidor_SEND_BREAK and Surtidor_WAIT_FOR_COMPLETE_REINIT.
    *      This option is recommended for most cases.
    *
    * Return:
    *  None.
    *
    * Global Variables:
    *  Surtidor_initVar - checked to identify that the component has been
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
    *     When interrupt appear with Surtidor_TX_STS_COMPLETE status:
    *     SendBreak(2);     - complete Break operation
    *
    * Side Effects:
    *  The Surtidor_SendBreak() function initializes registers to send a
    *  break signal.
    *  Break signal length depends on the break signal bits configuration.
    *  The register configuration should be reinitialized before normal 8-bit
    *  communication can continue.
    *
    *******************************************************************************/
    void Surtidor_SendBreak(uint8 retMode) 
    {

        /* If not Initialized then skip this function*/
        if(Surtidor_initVar != 0u)
        {
            /* Set the Counter to 13-bits and transmit a 00 byte */
            /* When that is done then reset the counter value back */
            uint8 tmpStat;

        #if(Surtidor_HD_ENABLED) /* Half Duplex mode*/

            if( (retMode == Surtidor_SEND_BREAK) ||
                (retMode == Surtidor_SEND_WAIT_REINIT ) )
            {
                /* CTRL_HD_SEND_BREAK - sends break bits in HD mode */
                Surtidor_WriteControlRegister(Surtidor_ReadControlRegister() |
                                                      Surtidor_CTRL_HD_SEND_BREAK);
                /* Send zeros */
                Surtidor_TXDATA_REG = 0u;

                do /* Wait until transmit starts */
                {
                    tmpStat = Surtidor_TXSTATUS_REG;
                }
                while((tmpStat & Surtidor_TX_STS_FIFO_EMPTY) != 0u);
            }

            if( (retMode == Surtidor_WAIT_FOR_COMPLETE_REINIT) ||
                (retMode == Surtidor_SEND_WAIT_REINIT) )
            {
                do /* Wait until transmit complete */
                {
                    tmpStat = Surtidor_TXSTATUS_REG;
                }
                while(((uint8)~tmpStat & Surtidor_TX_STS_COMPLETE) != 0u);
            }

            if( (retMode == Surtidor_WAIT_FOR_COMPLETE_REINIT) ||
                (retMode == Surtidor_REINIT) ||
                (retMode == Surtidor_SEND_WAIT_REINIT) )
            {
                Surtidor_WriteControlRegister(Surtidor_ReadControlRegister() &
                                              (uint8)~Surtidor_CTRL_HD_SEND_BREAK);
            }

        #else /* Surtidor_HD_ENABLED Full Duplex mode */

            static uint8 txPeriod;

            if( (retMode == Surtidor_SEND_BREAK) ||
                (retMode == Surtidor_SEND_WAIT_REINIT) )
            {
                /* CTRL_HD_SEND_BREAK - skip to send parity bit at Break signal in Full Duplex mode */
                #if( (Surtidor_PARITY_TYPE != Surtidor__B_UART__NONE_REVB) || \
                                    (Surtidor_PARITY_TYPE_SW != 0u) )
                    Surtidor_WriteControlRegister(Surtidor_ReadControlRegister() |
                                                          Surtidor_CTRL_HD_SEND_BREAK);
                #endif /* End Surtidor_PARITY_TYPE != Surtidor__B_UART__NONE_REVB  */

                #if(Surtidor_TXCLKGEN_DP)
                    txPeriod = Surtidor_TXBITCLKTX_COMPLETE_REG;
                    Surtidor_TXBITCLKTX_COMPLETE_REG = Surtidor_TXBITCTR_BREAKBITS;
                #else
                    txPeriod = Surtidor_TXBITCTR_PERIOD_REG;
                    Surtidor_TXBITCTR_PERIOD_REG = Surtidor_TXBITCTR_BREAKBITS8X;
                #endif /* End Surtidor_TXCLKGEN_DP */

                /* Send zeros */
                Surtidor_TXDATA_REG = 0u;

                do /* Wait until transmit starts */
                {
                    tmpStat = Surtidor_TXSTATUS_REG;
                }
                while((tmpStat & Surtidor_TX_STS_FIFO_EMPTY) != 0u);
            }

            if( (retMode == Surtidor_WAIT_FOR_COMPLETE_REINIT) ||
                (retMode == Surtidor_SEND_WAIT_REINIT) )
            {
                do /* Wait until transmit complete */
                {
                    tmpStat = Surtidor_TXSTATUS_REG;
                }
                while(((uint8)~tmpStat & Surtidor_TX_STS_COMPLETE) != 0u);
            }

            if( (retMode == Surtidor_WAIT_FOR_COMPLETE_REINIT) ||
                (retMode == Surtidor_REINIT) ||
                (retMode == Surtidor_SEND_WAIT_REINIT) )
            {

            #if(Surtidor_TXCLKGEN_DP)
                Surtidor_TXBITCLKTX_COMPLETE_REG = txPeriod;
            #else
                Surtidor_TXBITCTR_PERIOD_REG = txPeriod;
            #endif /* End Surtidor_TXCLKGEN_DP */

            #if( (Surtidor_PARITY_TYPE != Surtidor__B_UART__NONE_REVB) || \
                 (Surtidor_PARITY_TYPE_SW != 0u) )
                Surtidor_WriteControlRegister(Surtidor_ReadControlRegister() &
                                                      (uint8) ~Surtidor_CTRL_HD_SEND_BREAK);
            #endif /* End Surtidor_PARITY_TYPE != NONE */
            }
        #endif    /* End Surtidor_HD_ENABLED */
        }
    }


    /*******************************************************************************
    * Function Name: Surtidor_SetTxAddressMode
    ********************************************************************************
    *
    * Summary:
    *  Configures the transmitter to signal the next bytes is address or data.
    *
    * Parameters:
    *  addressMode: 
    *       Surtidor_SET_SPACE - Configure the transmitter to send the next
    *                                    byte as a data.
    *       Surtidor_SET_MARK  - Configure the transmitter to send the next
    *                                    byte as an address.
    *
    * Return:
    *  None.
    *
    * Side Effects:
    *  This function sets and clears Surtidor_CTRL_MARK bit in the Control
    *  register.
    *
    *******************************************************************************/
    void Surtidor_SetTxAddressMode(uint8 addressMode) 
    {
        /* Mark/Space sending enable */
        if(addressMode != 0u)
        {
        #if( Surtidor_CONTROL_REG_REMOVED == 0u )
            Surtidor_WriteControlRegister(Surtidor_ReadControlRegister() |
                                                  Surtidor_CTRL_MARK);
        #endif /* End Surtidor_CONTROL_REG_REMOVED == 0u */
        }
        else
        {
        #if( Surtidor_CONTROL_REG_REMOVED == 0u )
            Surtidor_WriteControlRegister(Surtidor_ReadControlRegister() &
                                                  (uint8) ~Surtidor_CTRL_MARK);
        #endif /* End Surtidor_CONTROL_REG_REMOVED == 0u */
        }
    }

#endif  /* EndSurtidor_TX_ENABLED */

#if(Surtidor_HD_ENABLED)


    /*******************************************************************************
    * Function Name: Surtidor_LoadRxConfig
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
    void Surtidor_LoadRxConfig(void) 
    {
        Surtidor_WriteControlRegister(Surtidor_ReadControlRegister() &
                                                (uint8)~Surtidor_CTRL_HD_SEND);
        Surtidor_RXBITCTR_PERIOD_REG = Surtidor_HD_RXBITCTR_INIT;

    #if (Surtidor_RX_INTERRUPT_ENABLED)
        /* Enable RX interrupt after set RX configuration */
        Surtidor_SetRxInterruptMode(Surtidor_INIT_RX_INTERRUPTS_MASK);
    #endif /* (Surtidor_RX_INTERRUPT_ENABLED) */
    }


    /*******************************************************************************
    * Function Name: Surtidor_LoadTxConfig
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
    void Surtidor_LoadTxConfig(void) 
    {
    #if (Surtidor_RX_INTERRUPT_ENABLED)
        /* Disable RX interrupts before set TX configuration */
        Surtidor_SetRxInterruptMode(0u);
    #endif /* (Surtidor_RX_INTERRUPT_ENABLED) */

        Surtidor_WriteControlRegister(Surtidor_ReadControlRegister() | Surtidor_CTRL_HD_SEND);
        Surtidor_RXBITCTR_PERIOD_REG = Surtidor_HD_TXBITCTR_INIT;
    }

#endif  /* Surtidor_HD_ENABLED */


/* [] END OF FILE */
