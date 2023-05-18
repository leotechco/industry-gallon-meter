/*******************************************************************************
* File Name: Bluetooth.c
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

#include "Bluetooth.h"
#if (Bluetooth_INTERNAL_CLOCK_USED)
    #include "Bluetooth_IntClock.h"
#endif /* End Bluetooth_INTERNAL_CLOCK_USED */


/***************************************
* Global data allocation
***************************************/

uint8 Bluetooth_initVar = 0u;

#if (Bluetooth_TX_INTERRUPT_ENABLED && Bluetooth_TX_ENABLED)
    volatile uint8 Bluetooth_txBuffer[Bluetooth_TX_BUFFER_SIZE];
    volatile uint8 Bluetooth_txBufferRead = 0u;
    uint8 Bluetooth_txBufferWrite = 0u;
#endif /* (Bluetooth_TX_INTERRUPT_ENABLED && Bluetooth_TX_ENABLED) */

#if (Bluetooth_RX_INTERRUPT_ENABLED && (Bluetooth_RX_ENABLED || Bluetooth_HD_ENABLED))
    uint8 Bluetooth_errorStatus = 0u;
    volatile uint8 Bluetooth_rxBuffer[Bluetooth_RX_BUFFER_SIZE];
    volatile uint8 Bluetooth_rxBufferRead  = 0u;
    volatile uint8 Bluetooth_rxBufferWrite = 0u;
    volatile uint8 Bluetooth_rxBufferLoopDetect = 0u;
    volatile uint8 Bluetooth_rxBufferOverflow   = 0u;
    #if (Bluetooth_RXHW_ADDRESS_ENABLED)
        volatile uint8 Bluetooth_rxAddressMode = Bluetooth_RX_ADDRESS_MODE;
        volatile uint8 Bluetooth_rxAddressDetected = 0u;
    #endif /* (Bluetooth_RXHW_ADDRESS_ENABLED) */
#endif /* (Bluetooth_RX_INTERRUPT_ENABLED && (Bluetooth_RX_ENABLED || Bluetooth_HD_ENABLED)) */


/*******************************************************************************
* Function Name: Bluetooth_Start
********************************************************************************
*
* Summary:
*  This is the preferred method to begin component operation.
*  Bluetooth_Start() sets the initVar variable, calls the
*  Bluetooth_Init() function, and then calls the
*  Bluetooth_Enable() function.
*
* Parameters:
*  None.
*
* Return:
*  None.
*
* Global variables:
*  The Bluetooth_intiVar variable is used to indicate initial
*  configuration of this component. The variable is initialized to zero (0u)
*  and set to one (1u) the first time Bluetooth_Start() is called. This
*  allows for component initialization without re-initialization in all
*  subsequent calls to the Bluetooth_Start() routine.
*
* Reentrant:
*  No.
*
*******************************************************************************/
void Bluetooth_Start(void) 
{
    /* If not initialized then initialize all required hardware and software */
    if(Bluetooth_initVar == 0u)
    {
        Bluetooth_Init();
        Bluetooth_initVar = 1u;
    }

    Bluetooth_Enable();
}


/*******************************************************************************
* Function Name: Bluetooth_Init
********************************************************************************
*
* Summary:
*  Initializes or restores the component according to the customizer Configure
*  dialog settings. It is not necessary to call Bluetooth_Init() because
*  the Bluetooth_Start() API calls this function and is the preferred
*  method to begin component operation.
*
* Parameters:
*  None.
*
* Return:
*  None.
*
*******************************************************************************/
void Bluetooth_Init(void) 
{
    #if(Bluetooth_RX_ENABLED || Bluetooth_HD_ENABLED)

        #if (Bluetooth_RX_INTERRUPT_ENABLED)
            /* Set RX interrupt vector and priority */
            (void) CyIntSetVector(Bluetooth_RX_VECT_NUM, &Bluetooth_RXISR);
            CyIntSetPriority(Bluetooth_RX_VECT_NUM, Bluetooth_RX_PRIOR_NUM);
            Bluetooth_errorStatus = 0u;
        #endif /* (Bluetooth_RX_INTERRUPT_ENABLED) */

        #if (Bluetooth_RXHW_ADDRESS_ENABLED)
            Bluetooth_SetRxAddressMode(Bluetooth_RX_ADDRESS_MODE);
            Bluetooth_SetRxAddress1(Bluetooth_RX_HW_ADDRESS1);
            Bluetooth_SetRxAddress2(Bluetooth_RX_HW_ADDRESS2);
        #endif /* End Bluetooth_RXHW_ADDRESS_ENABLED */

        /* Init Count7 period */
        Bluetooth_RXBITCTR_PERIOD_REG = Bluetooth_RXBITCTR_INIT;
        /* Configure the Initial RX interrupt mask */
        Bluetooth_RXSTATUS_MASK_REG  = Bluetooth_INIT_RX_INTERRUPTS_MASK;
    #endif /* End Bluetooth_RX_ENABLED || Bluetooth_HD_ENABLED*/

    #if(Bluetooth_TX_ENABLED)
        #if (Bluetooth_TX_INTERRUPT_ENABLED)
            /* Set TX interrupt vector and priority */
            (void) CyIntSetVector(Bluetooth_TX_VECT_NUM, &Bluetooth_TXISR);
            CyIntSetPriority(Bluetooth_TX_VECT_NUM, Bluetooth_TX_PRIOR_NUM);
        #endif /* (Bluetooth_TX_INTERRUPT_ENABLED) */

        /* Write Counter Value for TX Bit Clk Generator*/
        #if (Bluetooth_TXCLKGEN_DP)
            Bluetooth_TXBITCLKGEN_CTR_REG = Bluetooth_BIT_CENTER;
            Bluetooth_TXBITCLKTX_COMPLETE_REG = ((Bluetooth_NUMBER_OF_DATA_BITS +
                        Bluetooth_NUMBER_OF_START_BIT) * Bluetooth_OVER_SAMPLE_COUNT) - 1u;
        #else
            Bluetooth_TXBITCTR_PERIOD_REG = ((Bluetooth_NUMBER_OF_DATA_BITS +
                        Bluetooth_NUMBER_OF_START_BIT) * Bluetooth_OVER_SAMPLE_8) - 1u;
        #endif /* End Bluetooth_TXCLKGEN_DP */

        /* Configure the Initial TX interrupt mask */
        #if (Bluetooth_TX_INTERRUPT_ENABLED)
            Bluetooth_TXSTATUS_MASK_REG = Bluetooth_TX_STS_FIFO_EMPTY;
        #else
            Bluetooth_TXSTATUS_MASK_REG = Bluetooth_INIT_TX_INTERRUPTS_MASK;
        #endif /*End Bluetooth_TX_INTERRUPT_ENABLED*/

    #endif /* End Bluetooth_TX_ENABLED */

    #if(Bluetooth_PARITY_TYPE_SW)  /* Write Parity to Control Register */
        Bluetooth_WriteControlRegister( \
            (Bluetooth_ReadControlRegister() & (uint8)~Bluetooth_CTRL_PARITY_TYPE_MASK) | \
            (uint8)(Bluetooth_PARITY_TYPE << Bluetooth_CTRL_PARITY_TYPE0_SHIFT) );
    #endif /* End Bluetooth_PARITY_TYPE_SW */
}


/*******************************************************************************
* Function Name: Bluetooth_Enable
********************************************************************************
*
* Summary:
*  Activates the hardware and begins component operation. It is not necessary
*  to call Bluetooth_Enable() because the Bluetooth_Start() API
*  calls this function, which is the preferred method to begin component
*  operation.

* Parameters:
*  None.
*
* Return:
*  None.
*
* Global Variables:
*  Bluetooth_rxAddressDetected - set to initial state (0).
*
*******************************************************************************/
void Bluetooth_Enable(void) 
{
    uint8 enableInterrupts;
    enableInterrupts = CyEnterCriticalSection();

    #if (Bluetooth_RX_ENABLED || Bluetooth_HD_ENABLED)
        /* RX Counter (Count7) Enable */
        Bluetooth_RXBITCTR_CONTROL_REG |= Bluetooth_CNTR_ENABLE;

        /* Enable the RX Interrupt */
        Bluetooth_RXSTATUS_ACTL_REG  |= Bluetooth_INT_ENABLE;

        #if (Bluetooth_RX_INTERRUPT_ENABLED)
            Bluetooth_EnableRxInt();

            #if (Bluetooth_RXHW_ADDRESS_ENABLED)
                Bluetooth_rxAddressDetected = 0u;
            #endif /* (Bluetooth_RXHW_ADDRESS_ENABLED) */
        #endif /* (Bluetooth_RX_INTERRUPT_ENABLED) */
    #endif /* (Bluetooth_RX_ENABLED || Bluetooth_HD_ENABLED) */

    #if(Bluetooth_TX_ENABLED)
        /* TX Counter (DP/Count7) Enable */
        #if(!Bluetooth_TXCLKGEN_DP)
            Bluetooth_TXBITCTR_CONTROL_REG |= Bluetooth_CNTR_ENABLE;
        #endif /* End Bluetooth_TXCLKGEN_DP */

        /* Enable the TX Interrupt */
        Bluetooth_TXSTATUS_ACTL_REG |= Bluetooth_INT_ENABLE;
        #if (Bluetooth_TX_INTERRUPT_ENABLED)
            Bluetooth_ClearPendingTxInt(); /* Clear history of TX_NOT_EMPTY */
            Bluetooth_EnableTxInt();
        #endif /* (Bluetooth_TX_INTERRUPT_ENABLED) */
     #endif /* (Bluetooth_TX_INTERRUPT_ENABLED) */

    #if (Bluetooth_INTERNAL_CLOCK_USED)
        Bluetooth_IntClock_Start();  /* Enable the clock */
    #endif /* (Bluetooth_INTERNAL_CLOCK_USED) */

    CyExitCriticalSection(enableInterrupts);
}


/*******************************************************************************
* Function Name: Bluetooth_Stop
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
void Bluetooth_Stop(void) 
{
    uint8 enableInterrupts;
    enableInterrupts = CyEnterCriticalSection();

    /* Write Bit Counter Disable */
    #if (Bluetooth_RX_ENABLED || Bluetooth_HD_ENABLED)
        Bluetooth_RXBITCTR_CONTROL_REG &= (uint8) ~Bluetooth_CNTR_ENABLE;
    #endif /* (Bluetooth_RX_ENABLED || Bluetooth_HD_ENABLED) */

    #if (Bluetooth_TX_ENABLED)
        #if(!Bluetooth_TXCLKGEN_DP)
            Bluetooth_TXBITCTR_CONTROL_REG &= (uint8) ~Bluetooth_CNTR_ENABLE;
        #endif /* (!Bluetooth_TXCLKGEN_DP) */
    #endif /* (Bluetooth_TX_ENABLED) */

    #if (Bluetooth_INTERNAL_CLOCK_USED)
        Bluetooth_IntClock_Stop();   /* Disable the clock */
    #endif /* (Bluetooth_INTERNAL_CLOCK_USED) */

    /* Disable internal interrupt component */
    #if (Bluetooth_RX_ENABLED || Bluetooth_HD_ENABLED)
        Bluetooth_RXSTATUS_ACTL_REG  &= (uint8) ~Bluetooth_INT_ENABLE;

        #if (Bluetooth_RX_INTERRUPT_ENABLED)
            Bluetooth_DisableRxInt();
        #endif /* (Bluetooth_RX_INTERRUPT_ENABLED) */
    #endif /* (Bluetooth_RX_ENABLED || Bluetooth_HD_ENABLED) */

    #if (Bluetooth_TX_ENABLED)
        Bluetooth_TXSTATUS_ACTL_REG &= (uint8) ~Bluetooth_INT_ENABLE;

        #if (Bluetooth_TX_INTERRUPT_ENABLED)
            Bluetooth_DisableTxInt();
        #endif /* (Bluetooth_TX_INTERRUPT_ENABLED) */
    #endif /* (Bluetooth_TX_ENABLED) */

    CyExitCriticalSection(enableInterrupts);
}


/*******************************************************************************
* Function Name: Bluetooth_ReadControlRegister
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
uint8 Bluetooth_ReadControlRegister(void) 
{
    #if (Bluetooth_CONTROL_REG_REMOVED)
        return(0u);
    #else
        return(Bluetooth_CONTROL_REG);
    #endif /* (Bluetooth_CONTROL_REG_REMOVED) */
}


/*******************************************************************************
* Function Name: Bluetooth_WriteControlRegister
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
void  Bluetooth_WriteControlRegister(uint8 control) 
{
    #if (Bluetooth_CONTROL_REG_REMOVED)
        if(0u != control)
        {
            /* Suppress compiler warning */
        }
    #else
       Bluetooth_CONTROL_REG = control;
    #endif /* (Bluetooth_CONTROL_REG_REMOVED) */
}


#if(Bluetooth_RX_ENABLED || Bluetooth_HD_ENABLED)
    /*******************************************************************************
    * Function Name: Bluetooth_SetRxInterruptMode
    ********************************************************************************
    *
    * Summary:
    *  Configures the RX interrupt sources enabled.
    *
    * Parameters:
    *  IntSrc:  Bit field containing the RX interrupts to enable. Based on the 
    *  bit-field arrangement of the status register. This value must be a 
    *  combination of status register bit-masks shown below:
    *      Bluetooth_RX_STS_FIFO_NOTEMPTY    Interrupt on byte received.
    *      Bluetooth_RX_STS_PAR_ERROR        Interrupt on parity error.
    *      Bluetooth_RX_STS_STOP_ERROR       Interrupt on stop error.
    *      Bluetooth_RX_STS_BREAK            Interrupt on break.
    *      Bluetooth_RX_STS_OVERRUN          Interrupt on overrun error.
    *      Bluetooth_RX_STS_ADDR_MATCH       Interrupt on address match.
    *      Bluetooth_RX_STS_MRKSPC           Interrupt on address detect.
    *
    * Return:
    *  None.
    *
    * Theory:
    *  Enables the output of specific status bits to the interrupt controller
    *
    *******************************************************************************/
    void Bluetooth_SetRxInterruptMode(uint8 intSrc) 
    {
        Bluetooth_RXSTATUS_MASK_REG  = intSrc;
    }


    /*******************************************************************************
    * Function Name: Bluetooth_ReadRxData
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
    *  Bluetooth_rxBuffer - RAM buffer pointer for save received data.
    *  Bluetooth_rxBufferWrite - cyclic index for write to rxBuffer,
    *     checked to identify new data.
    *  Bluetooth_rxBufferRead - cyclic index for read from rxBuffer,
    *     incremented after each byte has been read from buffer.
    *  Bluetooth_rxBufferLoopDetect - cleared if loop condition was detected
    *     in RX ISR.
    *
    * Reentrant:
    *  No.
    *
    *******************************************************************************/
    uint8 Bluetooth_ReadRxData(void) 
    {
        uint8 rxData;

    #if (Bluetooth_RX_INTERRUPT_ENABLED)

        uint8 locRxBufferRead;
        uint8 locRxBufferWrite;

        /* Protect variables that could change on interrupt */
        Bluetooth_DisableRxInt();

        locRxBufferRead  = Bluetooth_rxBufferRead;
        locRxBufferWrite = Bluetooth_rxBufferWrite;

        if( (Bluetooth_rxBufferLoopDetect != 0u) || (locRxBufferRead != locRxBufferWrite) )
        {
            rxData = Bluetooth_rxBuffer[locRxBufferRead];
            locRxBufferRead++;

            if(locRxBufferRead >= Bluetooth_RX_BUFFER_SIZE)
            {
                locRxBufferRead = 0u;
            }
            /* Update the real pointer */
            Bluetooth_rxBufferRead = locRxBufferRead;

            if(Bluetooth_rxBufferLoopDetect != 0u)
            {
                Bluetooth_rxBufferLoopDetect = 0u;
                #if ((Bluetooth_RX_INTERRUPT_ENABLED) && (Bluetooth_FLOW_CONTROL != 0u))
                    /* When Hardware Flow Control selected - return RX mask */
                    #if( Bluetooth_HD_ENABLED )
                        if((Bluetooth_CONTROL_REG & Bluetooth_CTRL_HD_SEND) == 0u)
                        {   /* In Half duplex mode return RX mask only in RX
                            *  configuration set, otherwise
                            *  mask will be returned in LoadRxConfig() API.
                            */
                            Bluetooth_RXSTATUS_MASK_REG  |= Bluetooth_RX_STS_FIFO_NOTEMPTY;
                        }
                    #else
                        Bluetooth_RXSTATUS_MASK_REG  |= Bluetooth_RX_STS_FIFO_NOTEMPTY;
                    #endif /* end Bluetooth_HD_ENABLED */
                #endif /* ((Bluetooth_RX_INTERRUPT_ENABLED) && (Bluetooth_FLOW_CONTROL != 0u)) */
            }
        }
        else
        {   /* Needs to check status for RX_STS_FIFO_NOTEMPTY bit */
            rxData = Bluetooth_RXDATA_REG;
        }

        Bluetooth_EnableRxInt();

    #else

        /* Needs to check status for RX_STS_FIFO_NOTEMPTY bit */
        rxData = Bluetooth_RXDATA_REG;

    #endif /* (Bluetooth_RX_INTERRUPT_ENABLED) */

        return(rxData);
    }


    /*******************************************************************************
    * Function Name: Bluetooth_ReadRxStatus
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
    *  Bluetooth_RX_STS_FIFO_NOTEMPTY.
    *  Bluetooth_RX_STS_FIFO_NOTEMPTY clears immediately after RX data
    *  register read.
    *
    * Global Variables:
    *  Bluetooth_rxBufferOverflow - used to indicate overload condition.
    *   It set to one in RX interrupt when there isn't free space in
    *   Bluetooth_rxBufferRead to write new data. This condition returned
    *   and cleared to zero by this API as an
    *   Bluetooth_RX_STS_SOFT_BUFF_OVER bit along with RX Status register
    *   bits.
    *
    *******************************************************************************/
    uint8 Bluetooth_ReadRxStatus(void) 
    {
        uint8 status;

        status = Bluetooth_RXSTATUS_REG & Bluetooth_RX_HW_MASK;

    #if (Bluetooth_RX_INTERRUPT_ENABLED)
        if(Bluetooth_rxBufferOverflow != 0u)
        {
            status |= Bluetooth_RX_STS_SOFT_BUFF_OVER;
            Bluetooth_rxBufferOverflow = 0u;
        }
    #endif /* (Bluetooth_RX_INTERRUPT_ENABLED) */

        return(status);
    }


    /*******************************************************************************
    * Function Name: Bluetooth_GetChar
    ********************************************************************************
    *
    * Summary:
    *  Returns the last received byte of data. Bluetooth_GetChar() is
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
    *  Bluetooth_rxBuffer - RAM buffer pointer for save received data.
    *  Bluetooth_rxBufferWrite - cyclic index for write to rxBuffer,
    *     checked to identify new data.
    *  Bluetooth_rxBufferRead - cyclic index for read from rxBuffer,
    *     incremented after each byte has been read from buffer.
    *  Bluetooth_rxBufferLoopDetect - cleared if loop condition was detected
    *     in RX ISR.
    *
    * Reentrant:
    *  No.
    *
    *******************************************************************************/
    uint8 Bluetooth_GetChar(void) 
    {
        uint8 rxData = 0u;
        uint8 rxStatus;

    #if (Bluetooth_RX_INTERRUPT_ENABLED)
        uint8 locRxBufferRead;
        uint8 locRxBufferWrite;

        /* Protect variables that could change on interrupt */
        Bluetooth_DisableRxInt();

        locRxBufferRead  = Bluetooth_rxBufferRead;
        locRxBufferWrite = Bluetooth_rxBufferWrite;

        if( (Bluetooth_rxBufferLoopDetect != 0u) || (locRxBufferRead != locRxBufferWrite) )
        {
            rxData = Bluetooth_rxBuffer[locRxBufferRead];
            locRxBufferRead++;
            if(locRxBufferRead >= Bluetooth_RX_BUFFER_SIZE)
            {
                locRxBufferRead = 0u;
            }
            /* Update the real pointer */
            Bluetooth_rxBufferRead = locRxBufferRead;

            if(Bluetooth_rxBufferLoopDetect != 0u)
            {
                Bluetooth_rxBufferLoopDetect = 0u;
                #if( (Bluetooth_RX_INTERRUPT_ENABLED) && (Bluetooth_FLOW_CONTROL != 0u) )
                    /* When Hardware Flow Control selected - return RX mask */
                    #if( Bluetooth_HD_ENABLED )
                        if((Bluetooth_CONTROL_REG & Bluetooth_CTRL_HD_SEND) == 0u)
                        {   /* In Half duplex mode return RX mask only if
                            *  RX configuration set, otherwise
                            *  mask will be returned in LoadRxConfig() API.
                            */
                            Bluetooth_RXSTATUS_MASK_REG |= Bluetooth_RX_STS_FIFO_NOTEMPTY;
                        }
                    #else
                        Bluetooth_RXSTATUS_MASK_REG |= Bluetooth_RX_STS_FIFO_NOTEMPTY;
                    #endif /* end Bluetooth_HD_ENABLED */
                #endif /* Bluetooth_RX_INTERRUPT_ENABLED and Hardware flow control*/
            }

        }
        else
        {   rxStatus = Bluetooth_RXSTATUS_REG;
            if((rxStatus & Bluetooth_RX_STS_FIFO_NOTEMPTY) != 0u)
            {   /* Read received data from FIFO */
                rxData = Bluetooth_RXDATA_REG;
                /*Check status on error*/
                if((rxStatus & (Bluetooth_RX_STS_BREAK | Bluetooth_RX_STS_PAR_ERROR |
                                Bluetooth_RX_STS_STOP_ERROR | Bluetooth_RX_STS_OVERRUN)) != 0u)
                {
                    rxData = 0u;
                }
            }
        }

        Bluetooth_EnableRxInt();

    #else

        rxStatus =Bluetooth_RXSTATUS_REG;
        if((rxStatus & Bluetooth_RX_STS_FIFO_NOTEMPTY) != 0u)
        {
            /* Read received data from FIFO */
            rxData = Bluetooth_RXDATA_REG;

            /*Check status on error*/
            if((rxStatus & (Bluetooth_RX_STS_BREAK | Bluetooth_RX_STS_PAR_ERROR |
                            Bluetooth_RX_STS_STOP_ERROR | Bluetooth_RX_STS_OVERRUN)) != 0u)
            {
                rxData = 0u;
            }
        }
    #endif /* (Bluetooth_RX_INTERRUPT_ENABLED) */

        return(rxData);
    }


    /*******************************************************************************
    * Function Name: Bluetooth_GetByte
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
    uint16 Bluetooth_GetByte(void) 
    {
        
    #if (Bluetooth_RX_INTERRUPT_ENABLED)
        uint16 locErrorStatus;
        /* Protect variables that could change on interrupt */
        Bluetooth_DisableRxInt();
        locErrorStatus = (uint16)Bluetooth_errorStatus;
        Bluetooth_errorStatus = 0u;
        Bluetooth_EnableRxInt();
        return ( (uint16)(locErrorStatus << 8u) | Bluetooth_ReadRxData() );
    #else
        return ( ((uint16)Bluetooth_ReadRxStatus() << 8u) | Bluetooth_ReadRxData() );
    #endif /* Bluetooth_RX_INTERRUPT_ENABLED */
        
    }


    /*******************************************************************************
    * Function Name: Bluetooth_GetRxBufferSize
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
    *  Bluetooth_rxBufferWrite - used to calculate left bytes.
    *  Bluetooth_rxBufferRead - used to calculate left bytes.
    *  Bluetooth_rxBufferLoopDetect - checked to decide left bytes amount.
    *
    * Reentrant:
    *  No.
    *
    * Theory:
    *  Allows the user to find out how full the RX Buffer is.
    *
    *******************************************************************************/
    uint8 Bluetooth_GetRxBufferSize(void)
                                                            
    {
        uint8 size;

    #if (Bluetooth_RX_INTERRUPT_ENABLED)

        /* Protect variables that could change on interrupt */
        Bluetooth_DisableRxInt();

        if(Bluetooth_rxBufferRead == Bluetooth_rxBufferWrite)
        {
            if(Bluetooth_rxBufferLoopDetect != 0u)
            {
                size = Bluetooth_RX_BUFFER_SIZE;
            }
            else
            {
                size = 0u;
            }
        }
        else if(Bluetooth_rxBufferRead < Bluetooth_rxBufferWrite)
        {
            size = (Bluetooth_rxBufferWrite - Bluetooth_rxBufferRead);
        }
        else
        {
            size = (Bluetooth_RX_BUFFER_SIZE - Bluetooth_rxBufferRead) + Bluetooth_rxBufferWrite;
        }

        Bluetooth_EnableRxInt();

    #else

        /* We can only know if there is data in the fifo. */
        size = ((Bluetooth_RXSTATUS_REG & Bluetooth_RX_STS_FIFO_NOTEMPTY) != 0u) ? 1u : 0u;

    #endif /* (Bluetooth_RX_INTERRUPT_ENABLED) */

        return(size);
    }


    /*******************************************************************************
    * Function Name: Bluetooth_ClearRxBuffer
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
    *  Bluetooth_rxBufferWrite - cleared to zero.
    *  Bluetooth_rxBufferRead - cleared to zero.
    *  Bluetooth_rxBufferLoopDetect - cleared to zero.
    *  Bluetooth_rxBufferOverflow - cleared to zero.
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
    void Bluetooth_ClearRxBuffer(void) 
    {
        uint8 enableInterrupts;

        /* Clear the HW FIFO */
        enableInterrupts = CyEnterCriticalSection();
        Bluetooth_RXDATA_AUX_CTL_REG |= (uint8)  Bluetooth_RX_FIFO_CLR;
        Bluetooth_RXDATA_AUX_CTL_REG &= (uint8) ~Bluetooth_RX_FIFO_CLR;
        CyExitCriticalSection(enableInterrupts);

    #if (Bluetooth_RX_INTERRUPT_ENABLED)

        /* Protect variables that could change on interrupt. */
        Bluetooth_DisableRxInt();

        Bluetooth_rxBufferRead = 0u;
        Bluetooth_rxBufferWrite = 0u;
        Bluetooth_rxBufferLoopDetect = 0u;
        Bluetooth_rxBufferOverflow = 0u;

        Bluetooth_EnableRxInt();

    #endif /* (Bluetooth_RX_INTERRUPT_ENABLED) */

    }


    /*******************************************************************************
    * Function Name: Bluetooth_SetRxAddressMode
    ********************************************************************************
    *
    * Summary:
    *  Sets the software controlled Addressing mode used by the RX portion of the
    *  UART.
    *
    * Parameters:
    *  addressMode: Enumerated value indicating the mode of RX addressing
    *  Bluetooth__B_UART__AM_SW_BYTE_BYTE -  Software Byte-by-Byte address
    *                                               detection
    *  Bluetooth__B_UART__AM_SW_DETECT_TO_BUFFER - Software Detect to Buffer
    *                                               address detection
    *  Bluetooth__B_UART__AM_HW_BYTE_BY_BYTE - Hardware Byte-by-Byte address
    *                                               detection
    *  Bluetooth__B_UART__AM_HW_DETECT_TO_BUFFER - Hardware Detect to Buffer
    *                                               address detection
    *  Bluetooth__B_UART__AM_NONE - No address detection
    *
    * Return:
    *  None.
    *
    * Global Variables:
    *  Bluetooth_rxAddressMode - the parameter stored in this variable for
    *   the farther usage in RX ISR.
    *  Bluetooth_rxAddressDetected - set to initial state (0).
    *
    *******************************************************************************/
    void Bluetooth_SetRxAddressMode(uint8 addressMode)
                                                        
    {
        #if(Bluetooth_RXHW_ADDRESS_ENABLED)
            #if(Bluetooth_CONTROL_REG_REMOVED)
                if(0u != addressMode)
                {
                    /* Suppress compiler warning */
                }
            #else /* Bluetooth_CONTROL_REG_REMOVED */
                uint8 tmpCtrl;
                tmpCtrl = Bluetooth_CONTROL_REG & (uint8)~Bluetooth_CTRL_RXADDR_MODE_MASK;
                tmpCtrl |= (uint8)(addressMode << Bluetooth_CTRL_RXADDR_MODE0_SHIFT);
                Bluetooth_CONTROL_REG = tmpCtrl;

                #if(Bluetooth_RX_INTERRUPT_ENABLED && \
                   (Bluetooth_RXBUFFERSIZE > Bluetooth_FIFO_LENGTH) )
                    Bluetooth_rxAddressMode = addressMode;
                    Bluetooth_rxAddressDetected = 0u;
                #endif /* End Bluetooth_RXBUFFERSIZE > Bluetooth_FIFO_LENGTH*/
            #endif /* End Bluetooth_CONTROL_REG_REMOVED */
        #else /* Bluetooth_RXHW_ADDRESS_ENABLED */
            if(0u != addressMode)
            {
                /* Suppress compiler warning */
            }
        #endif /* End Bluetooth_RXHW_ADDRESS_ENABLED */
    }


    /*******************************************************************************
    * Function Name: Bluetooth_SetRxAddress1
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
    void Bluetooth_SetRxAddress1(uint8 address) 
    {
        Bluetooth_RXADDRESS1_REG = address;
    }


    /*******************************************************************************
    * Function Name: Bluetooth_SetRxAddress2
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
    void Bluetooth_SetRxAddress2(uint8 address) 
    {
        Bluetooth_RXADDRESS2_REG = address;
    }

#endif  /* Bluetooth_RX_ENABLED || Bluetooth_HD_ENABLED*/


#if( (Bluetooth_TX_ENABLED) || (Bluetooth_HD_ENABLED) )
    /*******************************************************************************
    * Function Name: Bluetooth_SetTxInterruptMode
    ********************************************************************************
    *
    * Summary:
    *  Configures the TX interrupt sources to be enabled, but does not enable the
    *  interrupt.
    *
    * Parameters:
    *  intSrc: Bit field containing the TX interrupt sources to enable
    *   Bluetooth_TX_STS_COMPLETE        Interrupt on TX byte complete
    *   Bluetooth_TX_STS_FIFO_EMPTY      Interrupt when TX FIFO is empty
    *   Bluetooth_TX_STS_FIFO_FULL       Interrupt when TX FIFO is full
    *   Bluetooth_TX_STS_FIFO_NOT_FULL   Interrupt when TX FIFO is not full
    *
    * Return:
    *  None.
    *
    * Theory:
    *  Enables the output of specific status bits to the interrupt controller
    *
    *******************************************************************************/
    void Bluetooth_SetTxInterruptMode(uint8 intSrc) 
    {
        Bluetooth_TXSTATUS_MASK_REG = intSrc;
    }


    /*******************************************************************************
    * Function Name: Bluetooth_WriteTxData
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
    *  Bluetooth_txBuffer - RAM buffer pointer for save data for transmission
    *  Bluetooth_txBufferWrite - cyclic index for write to txBuffer,
    *    incremented after each byte saved to buffer.
    *  Bluetooth_txBufferRead - cyclic index for read from txBuffer,
    *    checked to identify the condition to write to FIFO directly or to TX buffer
    *  Bluetooth_initVar - checked to identify that the component has been
    *    initialized.
    *
    * Reentrant:
    *  No.
    *
    *******************************************************************************/
    void Bluetooth_WriteTxData(uint8 txDataByte) 
    {
        /* If not Initialized then skip this function*/
        if(Bluetooth_initVar != 0u)
        {
        #if (Bluetooth_TX_INTERRUPT_ENABLED)

            /* Protect variables that could change on interrupt. */
            Bluetooth_DisableTxInt();

            if( (Bluetooth_txBufferRead == Bluetooth_txBufferWrite) &&
                ((Bluetooth_TXSTATUS_REG & Bluetooth_TX_STS_FIFO_FULL) == 0u) )
            {
                /* Add directly to the FIFO. */
                Bluetooth_TXDATA_REG = txDataByte;
            }
            else
            {
                if(Bluetooth_txBufferWrite >= Bluetooth_TX_BUFFER_SIZE)
                {
                    Bluetooth_txBufferWrite = 0u;
                }

                Bluetooth_txBuffer[Bluetooth_txBufferWrite] = txDataByte;

                /* Add to the software buffer. */
                Bluetooth_txBufferWrite++;
            }

            Bluetooth_EnableTxInt();

        #else

            /* Add directly to the FIFO. */
            Bluetooth_TXDATA_REG = txDataByte;

        #endif /*(Bluetooth_TX_INTERRUPT_ENABLED) */
        }
    }


    /*******************************************************************************
    * Function Name: Bluetooth_ReadTxStatus
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
    uint8 Bluetooth_ReadTxStatus(void) 
    {
        return(Bluetooth_TXSTATUS_REG);
    }


    /*******************************************************************************
    * Function Name: Bluetooth_PutChar
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
    *  Bluetooth_txBuffer - RAM buffer pointer for save data for transmission
    *  Bluetooth_txBufferWrite - cyclic index for write to txBuffer,
    *     checked to identify free space in txBuffer and incremented after each byte
    *     saved to buffer.
    *  Bluetooth_txBufferRead - cyclic index for read from txBuffer,
    *     checked to identify free space in txBuffer.
    *  Bluetooth_initVar - checked to identify that the component has been
    *     initialized.
    *
    * Reentrant:
    *  No.
    *
    * Theory:
    *  Allows the user to transmit any byte of data in a single transfer
    *
    *******************************************************************************/
    void Bluetooth_PutChar(uint8 txDataByte) 
    {
    #if (Bluetooth_TX_INTERRUPT_ENABLED)
        /* The temporary output pointer is used since it takes two instructions
        *  to increment with a wrap, and we can't risk doing that with the real
        *  pointer and getting an interrupt in between instructions.
        */
        uint8 locTxBufferWrite;
        uint8 locTxBufferRead;

        do
        { /* Block if software buffer is full, so we don't overwrite. */

        #if ((Bluetooth_TX_BUFFER_SIZE > Bluetooth_MAX_BYTE_VALUE) && (CY_PSOC3))
            /* Disable TX interrupt to protect variables from modification */
            Bluetooth_DisableTxInt();
        #endif /* (Bluetooth_TX_BUFFER_SIZE > Bluetooth_MAX_BYTE_VALUE) && (CY_PSOC3) */

            locTxBufferWrite = Bluetooth_txBufferWrite;
            locTxBufferRead  = Bluetooth_txBufferRead;

        #if ((Bluetooth_TX_BUFFER_SIZE > Bluetooth_MAX_BYTE_VALUE) && (CY_PSOC3))
            /* Enable interrupt to continue transmission */
            Bluetooth_EnableTxInt();
        #endif /* (Bluetooth_TX_BUFFER_SIZE > Bluetooth_MAX_BYTE_VALUE) && (CY_PSOC3) */
        }
        while( (locTxBufferWrite < locTxBufferRead) ? (locTxBufferWrite == (locTxBufferRead - 1u)) :
                                ((locTxBufferWrite - locTxBufferRead) ==
                                (uint8)(Bluetooth_TX_BUFFER_SIZE - 1u)) );

        if( (locTxBufferRead == locTxBufferWrite) &&
            ((Bluetooth_TXSTATUS_REG & Bluetooth_TX_STS_FIFO_FULL) == 0u) )
        {
            /* Add directly to the FIFO */
            Bluetooth_TXDATA_REG = txDataByte;
        }
        else
        {
            if(locTxBufferWrite >= Bluetooth_TX_BUFFER_SIZE)
            {
                locTxBufferWrite = 0u;
            }
            /* Add to the software buffer. */
            Bluetooth_txBuffer[locTxBufferWrite] = txDataByte;
            locTxBufferWrite++;

            /* Finally, update the real output pointer */
        #if ((Bluetooth_TX_BUFFER_SIZE > Bluetooth_MAX_BYTE_VALUE) && (CY_PSOC3))
            Bluetooth_DisableTxInt();
        #endif /* (Bluetooth_TX_BUFFER_SIZE > Bluetooth_MAX_BYTE_VALUE) && (CY_PSOC3) */

            Bluetooth_txBufferWrite = locTxBufferWrite;

        #if ((Bluetooth_TX_BUFFER_SIZE > Bluetooth_MAX_BYTE_VALUE) && (CY_PSOC3))
            Bluetooth_EnableTxInt();
        #endif /* (Bluetooth_TX_BUFFER_SIZE > Bluetooth_MAX_BYTE_VALUE) && (CY_PSOC3) */

            if(0u != (Bluetooth_TXSTATUS_REG & Bluetooth_TX_STS_FIFO_EMPTY))
            {
                /* Trigger TX interrupt to send software buffer */
                Bluetooth_SetPendingTxInt();
            }
        }

    #else

        while((Bluetooth_TXSTATUS_REG & Bluetooth_TX_STS_FIFO_FULL) != 0u)
        {
            /* Wait for room in the FIFO */
        }

        /* Add directly to the FIFO */
        Bluetooth_TXDATA_REG = txDataByte;

    #endif /* Bluetooth_TX_INTERRUPT_ENABLED */
    }


    /*******************************************************************************
    * Function Name: Bluetooth_PutString
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
    *  Bluetooth_initVar - checked to identify that the component has been
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
    void Bluetooth_PutString(const char8 string[]) 
    {
        uint16 bufIndex = 0u;

        /* If not Initialized then skip this function */
        if(Bluetooth_initVar != 0u)
        {
            /* This is a blocking function, it will not exit until all data is sent */
            while(string[bufIndex] != (char8) 0)
            {
                Bluetooth_PutChar((uint8)string[bufIndex]);
                bufIndex++;
            }
        }
    }


    /*******************************************************************************
    * Function Name: Bluetooth_PutArray
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
    *  Bluetooth_initVar - checked to identify that the component has been
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
    void Bluetooth_PutArray(const uint8 string[], uint8 byteCount)
                                                                    
    {
        uint8 bufIndex = 0u;

        /* If not Initialized then skip this function */
        if(Bluetooth_initVar != 0u)
        {
            while(bufIndex < byteCount)
            {
                Bluetooth_PutChar(string[bufIndex]);
                bufIndex++;
            }
        }
    }


    /*******************************************************************************
    * Function Name: Bluetooth_PutCRLF
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
    *  Bluetooth_initVar - checked to identify that the component has been
    *     initialized.
    *
    * Reentrant:
    *  No.
    *
    *******************************************************************************/
    void Bluetooth_PutCRLF(uint8 txDataByte) 
    {
        /* If not Initialized then skip this function */
        if(Bluetooth_initVar != 0u)
        {
            Bluetooth_PutChar(txDataByte);
            Bluetooth_PutChar(0x0Du);
            Bluetooth_PutChar(0x0Au);
        }
    }


    /*******************************************************************************
    * Function Name: Bluetooth_GetTxBufferSize
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
    *  Bluetooth_txBufferWrite - used to calculate left space.
    *  Bluetooth_txBufferRead - used to calculate left space.
    *
    * Reentrant:
    *  No.
    *
    * Theory:
    *  Allows the user to find out how full the TX Buffer is.
    *
    *******************************************************************************/
    uint8 Bluetooth_GetTxBufferSize(void)
                                                            
    {
        uint8 size;

    #if (Bluetooth_TX_INTERRUPT_ENABLED)

        /* Protect variables that could change on interrupt. */
        Bluetooth_DisableTxInt();

        if(Bluetooth_txBufferRead == Bluetooth_txBufferWrite)
        {
            size = 0u;
        }
        else if(Bluetooth_txBufferRead < Bluetooth_txBufferWrite)
        {
            size = (Bluetooth_txBufferWrite - Bluetooth_txBufferRead);
        }
        else
        {
            size = (Bluetooth_TX_BUFFER_SIZE - Bluetooth_txBufferRead) +
                    Bluetooth_txBufferWrite;
        }

        Bluetooth_EnableTxInt();

    #else

        size = Bluetooth_TXSTATUS_REG;

        /* Is the fifo is full. */
        if((size & Bluetooth_TX_STS_FIFO_FULL) != 0u)
        {
            size = Bluetooth_FIFO_LENGTH;
        }
        else if((size & Bluetooth_TX_STS_FIFO_EMPTY) != 0u)
        {
            size = 0u;
        }
        else
        {
            /* We only know there is data in the fifo. */
            size = 1u;
        }

    #endif /* (Bluetooth_TX_INTERRUPT_ENABLED) */

    return(size);
    }


    /*******************************************************************************
    * Function Name: Bluetooth_ClearTxBuffer
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
    *  Bluetooth_txBufferWrite - cleared to zero.
    *  Bluetooth_txBufferRead - cleared to zero.
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
    void Bluetooth_ClearTxBuffer(void) 
    {
        uint8 enableInterrupts;

        enableInterrupts = CyEnterCriticalSection();
        /* Clear the HW FIFO */
        Bluetooth_TXDATA_AUX_CTL_REG |= (uint8)  Bluetooth_TX_FIFO_CLR;
        Bluetooth_TXDATA_AUX_CTL_REG &= (uint8) ~Bluetooth_TX_FIFO_CLR;
        CyExitCriticalSection(enableInterrupts);

    #if (Bluetooth_TX_INTERRUPT_ENABLED)

        /* Protect variables that could change on interrupt. */
        Bluetooth_DisableTxInt();

        Bluetooth_txBufferRead = 0u;
        Bluetooth_txBufferWrite = 0u;

        /* Enable Tx interrupt. */
        Bluetooth_EnableTxInt();

    #endif /* (Bluetooth_TX_INTERRUPT_ENABLED) */
    }


    /*******************************************************************************
    * Function Name: Bluetooth_SendBreak
    ********************************************************************************
    *
    * Summary:
    *  Transmits a break signal on the bus.
    *
    * Parameters:
    *  uint8 retMode:  Send Break return mode. See the following table for options.
    *   Bluetooth_SEND_BREAK - Initialize registers for break, send the Break
    *       signal and return immediately.
    *   Bluetooth_WAIT_FOR_COMPLETE_REINIT - Wait until break transmission is
    *       complete, reinitialize registers to normal transmission mode then return
    *   Bluetooth_REINIT - Reinitialize registers to normal transmission mode
    *       then return.
    *   Bluetooth_SEND_WAIT_REINIT - Performs both options: 
    *      Bluetooth_SEND_BREAK and Bluetooth_WAIT_FOR_COMPLETE_REINIT.
    *      This option is recommended for most cases.
    *
    * Return:
    *  None.
    *
    * Global Variables:
    *  Bluetooth_initVar - checked to identify that the component has been
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
    *     When interrupt appear with Bluetooth_TX_STS_COMPLETE status:
    *     SendBreak(2);     - complete Break operation
    *
    * Side Effects:
    *  The Bluetooth_SendBreak() function initializes registers to send a
    *  break signal.
    *  Break signal length depends on the break signal bits configuration.
    *  The register configuration should be reinitialized before normal 8-bit
    *  communication can continue.
    *
    *******************************************************************************/
    void Bluetooth_SendBreak(uint8 retMode) 
    {

        /* If not Initialized then skip this function*/
        if(Bluetooth_initVar != 0u)
        {
            /* Set the Counter to 13-bits and transmit a 00 byte */
            /* When that is done then reset the counter value back */
            uint8 tmpStat;

        #if(Bluetooth_HD_ENABLED) /* Half Duplex mode*/

            if( (retMode == Bluetooth_SEND_BREAK) ||
                (retMode == Bluetooth_SEND_WAIT_REINIT ) )
            {
                /* CTRL_HD_SEND_BREAK - sends break bits in HD mode */
                Bluetooth_WriteControlRegister(Bluetooth_ReadControlRegister() |
                                                      Bluetooth_CTRL_HD_SEND_BREAK);
                /* Send zeros */
                Bluetooth_TXDATA_REG = 0u;

                do /* Wait until transmit starts */
                {
                    tmpStat = Bluetooth_TXSTATUS_REG;
                }
                while((tmpStat & Bluetooth_TX_STS_FIFO_EMPTY) != 0u);
            }

            if( (retMode == Bluetooth_WAIT_FOR_COMPLETE_REINIT) ||
                (retMode == Bluetooth_SEND_WAIT_REINIT) )
            {
                do /* Wait until transmit complete */
                {
                    tmpStat = Bluetooth_TXSTATUS_REG;
                }
                while(((uint8)~tmpStat & Bluetooth_TX_STS_COMPLETE) != 0u);
            }

            if( (retMode == Bluetooth_WAIT_FOR_COMPLETE_REINIT) ||
                (retMode == Bluetooth_REINIT) ||
                (retMode == Bluetooth_SEND_WAIT_REINIT) )
            {
                Bluetooth_WriteControlRegister(Bluetooth_ReadControlRegister() &
                                              (uint8)~Bluetooth_CTRL_HD_SEND_BREAK);
            }

        #else /* Bluetooth_HD_ENABLED Full Duplex mode */

            static uint8 txPeriod;

            if( (retMode == Bluetooth_SEND_BREAK) ||
                (retMode == Bluetooth_SEND_WAIT_REINIT) )
            {
                /* CTRL_HD_SEND_BREAK - skip to send parity bit at Break signal in Full Duplex mode */
                #if( (Bluetooth_PARITY_TYPE != Bluetooth__B_UART__NONE_REVB) || \
                                    (Bluetooth_PARITY_TYPE_SW != 0u) )
                    Bluetooth_WriteControlRegister(Bluetooth_ReadControlRegister() |
                                                          Bluetooth_CTRL_HD_SEND_BREAK);
                #endif /* End Bluetooth_PARITY_TYPE != Bluetooth__B_UART__NONE_REVB  */

                #if(Bluetooth_TXCLKGEN_DP)
                    txPeriod = Bluetooth_TXBITCLKTX_COMPLETE_REG;
                    Bluetooth_TXBITCLKTX_COMPLETE_REG = Bluetooth_TXBITCTR_BREAKBITS;
                #else
                    txPeriod = Bluetooth_TXBITCTR_PERIOD_REG;
                    Bluetooth_TXBITCTR_PERIOD_REG = Bluetooth_TXBITCTR_BREAKBITS8X;
                #endif /* End Bluetooth_TXCLKGEN_DP */

                /* Send zeros */
                Bluetooth_TXDATA_REG = 0u;

                do /* Wait until transmit starts */
                {
                    tmpStat = Bluetooth_TXSTATUS_REG;
                }
                while((tmpStat & Bluetooth_TX_STS_FIFO_EMPTY) != 0u);
            }

            if( (retMode == Bluetooth_WAIT_FOR_COMPLETE_REINIT) ||
                (retMode == Bluetooth_SEND_WAIT_REINIT) )
            {
                do /* Wait until transmit complete */
                {
                    tmpStat = Bluetooth_TXSTATUS_REG;
                }
                while(((uint8)~tmpStat & Bluetooth_TX_STS_COMPLETE) != 0u);
            }

            if( (retMode == Bluetooth_WAIT_FOR_COMPLETE_REINIT) ||
                (retMode == Bluetooth_REINIT) ||
                (retMode == Bluetooth_SEND_WAIT_REINIT) )
            {

            #if(Bluetooth_TXCLKGEN_DP)
                Bluetooth_TXBITCLKTX_COMPLETE_REG = txPeriod;
            #else
                Bluetooth_TXBITCTR_PERIOD_REG = txPeriod;
            #endif /* End Bluetooth_TXCLKGEN_DP */

            #if( (Bluetooth_PARITY_TYPE != Bluetooth__B_UART__NONE_REVB) || \
                 (Bluetooth_PARITY_TYPE_SW != 0u) )
                Bluetooth_WriteControlRegister(Bluetooth_ReadControlRegister() &
                                                      (uint8) ~Bluetooth_CTRL_HD_SEND_BREAK);
            #endif /* End Bluetooth_PARITY_TYPE != NONE */
            }
        #endif    /* End Bluetooth_HD_ENABLED */
        }
    }


    /*******************************************************************************
    * Function Name: Bluetooth_SetTxAddressMode
    ********************************************************************************
    *
    * Summary:
    *  Configures the transmitter to signal the next bytes is address or data.
    *
    * Parameters:
    *  addressMode: 
    *       Bluetooth_SET_SPACE - Configure the transmitter to send the next
    *                                    byte as a data.
    *       Bluetooth_SET_MARK  - Configure the transmitter to send the next
    *                                    byte as an address.
    *
    * Return:
    *  None.
    *
    * Side Effects:
    *  This function sets and clears Bluetooth_CTRL_MARK bit in the Control
    *  register.
    *
    *******************************************************************************/
    void Bluetooth_SetTxAddressMode(uint8 addressMode) 
    {
        /* Mark/Space sending enable */
        if(addressMode != 0u)
        {
        #if( Bluetooth_CONTROL_REG_REMOVED == 0u )
            Bluetooth_WriteControlRegister(Bluetooth_ReadControlRegister() |
                                                  Bluetooth_CTRL_MARK);
        #endif /* End Bluetooth_CONTROL_REG_REMOVED == 0u */
        }
        else
        {
        #if( Bluetooth_CONTROL_REG_REMOVED == 0u )
            Bluetooth_WriteControlRegister(Bluetooth_ReadControlRegister() &
                                                  (uint8) ~Bluetooth_CTRL_MARK);
        #endif /* End Bluetooth_CONTROL_REG_REMOVED == 0u */
        }
    }

#endif  /* EndBluetooth_TX_ENABLED */

#if(Bluetooth_HD_ENABLED)


    /*******************************************************************************
    * Function Name: Bluetooth_LoadRxConfig
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
    void Bluetooth_LoadRxConfig(void) 
    {
        Bluetooth_WriteControlRegister(Bluetooth_ReadControlRegister() &
                                                (uint8)~Bluetooth_CTRL_HD_SEND);
        Bluetooth_RXBITCTR_PERIOD_REG = Bluetooth_HD_RXBITCTR_INIT;

    #if (Bluetooth_RX_INTERRUPT_ENABLED)
        /* Enable RX interrupt after set RX configuration */
        Bluetooth_SetRxInterruptMode(Bluetooth_INIT_RX_INTERRUPTS_MASK);
    #endif /* (Bluetooth_RX_INTERRUPT_ENABLED) */
    }


    /*******************************************************************************
    * Function Name: Bluetooth_LoadTxConfig
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
    void Bluetooth_LoadTxConfig(void) 
    {
    #if (Bluetooth_RX_INTERRUPT_ENABLED)
        /* Disable RX interrupts before set TX configuration */
        Bluetooth_SetRxInterruptMode(0u);
    #endif /* (Bluetooth_RX_INTERRUPT_ENABLED) */

        Bluetooth_WriteControlRegister(Bluetooth_ReadControlRegister() | Bluetooth_CTRL_HD_SEND);
        Bluetooth_RXBITCTR_PERIOD_REG = Bluetooth_HD_TXBITCTR_INIT;
    }

#endif  /* Bluetooth_HD_ENABLED */


/* [] END OF FILE */
