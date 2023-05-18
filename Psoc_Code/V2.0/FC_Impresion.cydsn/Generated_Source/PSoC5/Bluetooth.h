/*******************************************************************************
* File Name: Bluetooth.h
* Version 2.50
*
* Description:
*  Contains the function prototypes and constants available to the UART
*  user module.
*
* Note:
*
********************************************************************************
* Copyright 2008-2015, Cypress Semiconductor Corporation.  All rights reserved.
* You may use this file only in accordance with the license, terms, conditions,
* disclaimers, and limitations in the end user license agreement accompanying
* the software package with which this file was provided.
*******************************************************************************/


#if !defined(CY_UART_Bluetooth_H)
#define CY_UART_Bluetooth_H

#include "cyfitter.h"
#include "cytypes.h"
#include "CyLib.h" /* For CyEnterCriticalSection() and CyExitCriticalSection() functions */


/***************************************
* Conditional Compilation Parameters
***************************************/

#define Bluetooth_RX_ENABLED                     (1u)
#define Bluetooth_TX_ENABLED                     (1u)
#define Bluetooth_HD_ENABLED                     (0u)
#define Bluetooth_RX_INTERRUPT_ENABLED           (1u)
#define Bluetooth_TX_INTERRUPT_ENABLED           (0u)
#define Bluetooth_INTERNAL_CLOCK_USED            (1u)
#define Bluetooth_RXHW_ADDRESS_ENABLED           (0u)
#define Bluetooth_OVER_SAMPLE_COUNT              (8u)
#define Bluetooth_PARITY_TYPE                    (0u)
#define Bluetooth_PARITY_TYPE_SW                 (0u)
#define Bluetooth_BREAK_DETECT                   (0u)
#define Bluetooth_BREAK_BITS_TX                  (13u)
#define Bluetooth_BREAK_BITS_RX                  (13u)
#define Bluetooth_TXCLKGEN_DP                    (1u)
#define Bluetooth_USE23POLLING                   (1u)
#define Bluetooth_FLOW_CONTROL                   (0u)
#define Bluetooth_CLK_FREQ                       (0u)
#define Bluetooth_TX_BUFFER_SIZE                 (4u)
#define Bluetooth_RX_BUFFER_SIZE                 (50u)

/* Check to see if required defines such as CY_PSOC5LP are available */
/* They are defined starting with cy_boot v3.0 */
#if !defined (CY_PSOC5LP)
    #error Component UART_v2_50 requires cy_boot v3.0 or later
#endif /* (CY_PSOC5LP) */

#if defined(Bluetooth_BUART_sCR_SyncCtl_CtrlReg__CONTROL_REG)
    #define Bluetooth_CONTROL_REG_REMOVED            (0u)
#else
    #define Bluetooth_CONTROL_REG_REMOVED            (1u)
#endif /* End Bluetooth_BUART_sCR_SyncCtl_CtrlReg__CONTROL_REG */


/***************************************
*      Data Structure Definition
***************************************/

/* Sleep Mode API Support */
typedef struct Bluetooth_backupStruct_
{
    uint8 enableState;

    #if(Bluetooth_CONTROL_REG_REMOVED == 0u)
        uint8 cr;
    #endif /* End Bluetooth_CONTROL_REG_REMOVED */

} Bluetooth_BACKUP_STRUCT;


/***************************************
*       Function Prototypes
***************************************/

void Bluetooth_Start(void) ;
void Bluetooth_Stop(void) ;
uint8 Bluetooth_ReadControlRegister(void) ;
void Bluetooth_WriteControlRegister(uint8 control) ;

void Bluetooth_Init(void) ;
void Bluetooth_Enable(void) ;
void Bluetooth_SaveConfig(void) ;
void Bluetooth_RestoreConfig(void) ;
void Bluetooth_Sleep(void) ;
void Bluetooth_Wakeup(void) ;

/* Only if RX is enabled */
#if( (Bluetooth_RX_ENABLED) || (Bluetooth_HD_ENABLED) )

    #if (Bluetooth_RX_INTERRUPT_ENABLED)
        #define Bluetooth_EnableRxInt()  CyIntEnable (Bluetooth_RX_VECT_NUM)
        #define Bluetooth_DisableRxInt() CyIntDisable(Bluetooth_RX_VECT_NUM)
        CY_ISR_PROTO(Bluetooth_RXISR);
    #endif /* Bluetooth_RX_INTERRUPT_ENABLED */

    void Bluetooth_SetRxAddressMode(uint8 addressMode)
                                                           ;
    void Bluetooth_SetRxAddress1(uint8 address) ;
    void Bluetooth_SetRxAddress2(uint8 address) ;

    void  Bluetooth_SetRxInterruptMode(uint8 intSrc) ;
    uint8 Bluetooth_ReadRxData(void) ;
    uint8 Bluetooth_ReadRxStatus(void) ;
    uint8 Bluetooth_GetChar(void) ;
    uint16 Bluetooth_GetByte(void) ;
    uint8 Bluetooth_GetRxBufferSize(void)
                                                            ;
    void Bluetooth_ClearRxBuffer(void) ;

    /* Obsolete functions, defines for backward compatible */
    #define Bluetooth_GetRxInterruptSource   Bluetooth_ReadRxStatus

#endif /* End (Bluetooth_RX_ENABLED) || (Bluetooth_HD_ENABLED) */

/* Only if TX is enabled */
#if(Bluetooth_TX_ENABLED || Bluetooth_HD_ENABLED)

    #if(Bluetooth_TX_INTERRUPT_ENABLED)
        #define Bluetooth_EnableTxInt()  CyIntEnable (Bluetooth_TX_VECT_NUM)
        #define Bluetooth_DisableTxInt() CyIntDisable(Bluetooth_TX_VECT_NUM)
        #define Bluetooth_SetPendingTxInt() CyIntSetPending(Bluetooth_TX_VECT_NUM)
        #define Bluetooth_ClearPendingTxInt() CyIntClearPending(Bluetooth_TX_VECT_NUM)
        CY_ISR_PROTO(Bluetooth_TXISR);
    #endif /* Bluetooth_TX_INTERRUPT_ENABLED */

    void Bluetooth_SetTxInterruptMode(uint8 intSrc) ;
    void Bluetooth_WriteTxData(uint8 txDataByte) ;
    uint8 Bluetooth_ReadTxStatus(void) ;
    void Bluetooth_PutChar(uint8 txDataByte) ;
    void Bluetooth_PutString(const char8 string[]) ;
    void Bluetooth_PutArray(const uint8 string[], uint8 byteCount)
                                                            ;
    void Bluetooth_PutCRLF(uint8 txDataByte) ;
    void Bluetooth_ClearTxBuffer(void) ;
    void Bluetooth_SetTxAddressMode(uint8 addressMode) ;
    void Bluetooth_SendBreak(uint8 retMode) ;
    uint8 Bluetooth_GetTxBufferSize(void)
                                                            ;
    /* Obsolete functions, defines for backward compatible */
    #define Bluetooth_PutStringConst         Bluetooth_PutString
    #define Bluetooth_PutArrayConst          Bluetooth_PutArray
    #define Bluetooth_GetTxInterruptSource   Bluetooth_ReadTxStatus

#endif /* End Bluetooth_TX_ENABLED || Bluetooth_HD_ENABLED */

#if(Bluetooth_HD_ENABLED)
    void Bluetooth_LoadRxConfig(void) ;
    void Bluetooth_LoadTxConfig(void) ;
#endif /* End Bluetooth_HD_ENABLED */


/* Communication bootloader APIs */
#if defined(CYDEV_BOOTLOADER_IO_COMP) && ((CYDEV_BOOTLOADER_IO_COMP == CyBtldr_Bluetooth) || \
                                          (CYDEV_BOOTLOADER_IO_COMP == CyBtldr_Custom_Interface))
    /* Physical layer functions */
    void    Bluetooth_CyBtldrCommStart(void) CYSMALL ;
    void    Bluetooth_CyBtldrCommStop(void) CYSMALL ;
    void    Bluetooth_CyBtldrCommReset(void) CYSMALL ;
    cystatus Bluetooth_CyBtldrCommWrite(const uint8 pData[], uint16 size, uint16 * count, uint8 timeOut) CYSMALL
             ;
    cystatus Bluetooth_CyBtldrCommRead(uint8 pData[], uint16 size, uint16 * count, uint8 timeOut) CYSMALL
             ;

    #if (CYDEV_BOOTLOADER_IO_COMP == CyBtldr_Bluetooth)
        #define CyBtldrCommStart    Bluetooth_CyBtldrCommStart
        #define CyBtldrCommStop     Bluetooth_CyBtldrCommStop
        #define CyBtldrCommReset    Bluetooth_CyBtldrCommReset
        #define CyBtldrCommWrite    Bluetooth_CyBtldrCommWrite
        #define CyBtldrCommRead     Bluetooth_CyBtldrCommRead
    #endif  /* (CYDEV_BOOTLOADER_IO_COMP == CyBtldr_Bluetooth) */

    /* Byte to Byte time out for detecting end of block data from host */
    #define Bluetooth_BYTE2BYTE_TIME_OUT (25u)
    #define Bluetooth_PACKET_EOP         (0x17u) /* End of packet defined by bootloader */
    #define Bluetooth_WAIT_EOP_DELAY     (5u)    /* Additional 5ms to wait for End of packet */
    #define Bluetooth_BL_CHK_DELAY_MS    (1u)    /* Time Out quantity equal 1mS */

#endif /* CYDEV_BOOTLOADER_IO_COMP */


/***************************************
*          API Constants
***************************************/
/* Parameters for SetTxAddressMode API*/
#define Bluetooth_SET_SPACE      (0x00u)
#define Bluetooth_SET_MARK       (0x01u)

/* Status Register definitions */
#if( (Bluetooth_TX_ENABLED) || (Bluetooth_HD_ENABLED) )
    #if(Bluetooth_TX_INTERRUPT_ENABLED)
        #define Bluetooth_TX_VECT_NUM            (uint8)Bluetooth_TXInternalInterrupt__INTC_NUMBER
        #define Bluetooth_TX_PRIOR_NUM           (uint8)Bluetooth_TXInternalInterrupt__INTC_PRIOR_NUM
    #endif /* Bluetooth_TX_INTERRUPT_ENABLED */

    #define Bluetooth_TX_STS_COMPLETE_SHIFT          (0x00u)
    #define Bluetooth_TX_STS_FIFO_EMPTY_SHIFT        (0x01u)
    #define Bluetooth_TX_STS_FIFO_NOT_FULL_SHIFT     (0x03u)
    #if(Bluetooth_TX_ENABLED)
        #define Bluetooth_TX_STS_FIFO_FULL_SHIFT     (0x02u)
    #else /* (Bluetooth_HD_ENABLED) */
        #define Bluetooth_TX_STS_FIFO_FULL_SHIFT     (0x05u)  /* Needs MD=0 */
    #endif /* (Bluetooth_TX_ENABLED) */

    #define Bluetooth_TX_STS_COMPLETE            (uint8)(0x01u << Bluetooth_TX_STS_COMPLETE_SHIFT)
    #define Bluetooth_TX_STS_FIFO_EMPTY          (uint8)(0x01u << Bluetooth_TX_STS_FIFO_EMPTY_SHIFT)
    #define Bluetooth_TX_STS_FIFO_FULL           (uint8)(0x01u << Bluetooth_TX_STS_FIFO_FULL_SHIFT)
    #define Bluetooth_TX_STS_FIFO_NOT_FULL       (uint8)(0x01u << Bluetooth_TX_STS_FIFO_NOT_FULL_SHIFT)
#endif /* End (Bluetooth_TX_ENABLED) || (Bluetooth_HD_ENABLED)*/

#if( (Bluetooth_RX_ENABLED) || (Bluetooth_HD_ENABLED) )
    #if(Bluetooth_RX_INTERRUPT_ENABLED)
        #define Bluetooth_RX_VECT_NUM            (uint8)Bluetooth_RXInternalInterrupt__INTC_NUMBER
        #define Bluetooth_RX_PRIOR_NUM           (uint8)Bluetooth_RXInternalInterrupt__INTC_PRIOR_NUM
    #endif /* Bluetooth_RX_INTERRUPT_ENABLED */
    #define Bluetooth_RX_STS_MRKSPC_SHIFT            (0x00u)
    #define Bluetooth_RX_STS_BREAK_SHIFT             (0x01u)
    #define Bluetooth_RX_STS_PAR_ERROR_SHIFT         (0x02u)
    #define Bluetooth_RX_STS_STOP_ERROR_SHIFT        (0x03u)
    #define Bluetooth_RX_STS_OVERRUN_SHIFT           (0x04u)
    #define Bluetooth_RX_STS_FIFO_NOTEMPTY_SHIFT     (0x05u)
    #define Bluetooth_RX_STS_ADDR_MATCH_SHIFT        (0x06u)
    #define Bluetooth_RX_STS_SOFT_BUFF_OVER_SHIFT    (0x07u)

    #define Bluetooth_RX_STS_MRKSPC           (uint8)(0x01u << Bluetooth_RX_STS_MRKSPC_SHIFT)
    #define Bluetooth_RX_STS_BREAK            (uint8)(0x01u << Bluetooth_RX_STS_BREAK_SHIFT)
    #define Bluetooth_RX_STS_PAR_ERROR        (uint8)(0x01u << Bluetooth_RX_STS_PAR_ERROR_SHIFT)
    #define Bluetooth_RX_STS_STOP_ERROR       (uint8)(0x01u << Bluetooth_RX_STS_STOP_ERROR_SHIFT)
    #define Bluetooth_RX_STS_OVERRUN          (uint8)(0x01u << Bluetooth_RX_STS_OVERRUN_SHIFT)
    #define Bluetooth_RX_STS_FIFO_NOTEMPTY    (uint8)(0x01u << Bluetooth_RX_STS_FIFO_NOTEMPTY_SHIFT)
    #define Bluetooth_RX_STS_ADDR_MATCH       (uint8)(0x01u << Bluetooth_RX_STS_ADDR_MATCH_SHIFT)
    #define Bluetooth_RX_STS_SOFT_BUFF_OVER   (uint8)(0x01u << Bluetooth_RX_STS_SOFT_BUFF_OVER_SHIFT)
    #define Bluetooth_RX_HW_MASK                     (0x7Fu)
#endif /* End (Bluetooth_RX_ENABLED) || (Bluetooth_HD_ENABLED) */

/* Control Register definitions */
#define Bluetooth_CTRL_HD_SEND_SHIFT                 (0x00u) /* 1 enable TX part in Half Duplex mode */
#define Bluetooth_CTRL_HD_SEND_BREAK_SHIFT           (0x01u) /* 1 send BREAK signal in Half Duplez mode */
#define Bluetooth_CTRL_MARK_SHIFT                    (0x02u) /* 1 sets mark, 0 sets space */
#define Bluetooth_CTRL_PARITY_TYPE0_SHIFT            (0x03u) /* Defines the type of parity implemented */
#define Bluetooth_CTRL_PARITY_TYPE1_SHIFT            (0x04u) /* Defines the type of parity implemented */
#define Bluetooth_CTRL_RXADDR_MODE0_SHIFT            (0x05u)
#define Bluetooth_CTRL_RXADDR_MODE1_SHIFT            (0x06u)
#define Bluetooth_CTRL_RXADDR_MODE2_SHIFT            (0x07u)

#define Bluetooth_CTRL_HD_SEND               (uint8)(0x01u << Bluetooth_CTRL_HD_SEND_SHIFT)
#define Bluetooth_CTRL_HD_SEND_BREAK         (uint8)(0x01u << Bluetooth_CTRL_HD_SEND_BREAK_SHIFT)
#define Bluetooth_CTRL_MARK                  (uint8)(0x01u << Bluetooth_CTRL_MARK_SHIFT)
#define Bluetooth_CTRL_PARITY_TYPE_MASK      (uint8)(0x03u << Bluetooth_CTRL_PARITY_TYPE0_SHIFT)
#define Bluetooth_CTRL_RXADDR_MODE_MASK      (uint8)(0x07u << Bluetooth_CTRL_RXADDR_MODE0_SHIFT)

/* StatusI Register Interrupt Enable Control Bits. As defined by the Register map for the AUX Control Register */
#define Bluetooth_INT_ENABLE                         (0x10u)

/* Bit Counter (7-bit) Control Register Bit Definitions. As defined by the Register map for the AUX Control Register */
#define Bluetooth_CNTR_ENABLE                        (0x20u)

/*   Constants for SendBreak() "retMode" parameter  */
#define Bluetooth_SEND_BREAK                         (0x00u)
#define Bluetooth_WAIT_FOR_COMPLETE_REINIT           (0x01u)
#define Bluetooth_REINIT                             (0x02u)
#define Bluetooth_SEND_WAIT_REINIT                   (0x03u)

#define Bluetooth_OVER_SAMPLE_8                      (8u)
#define Bluetooth_OVER_SAMPLE_16                     (16u)

#define Bluetooth_BIT_CENTER                         (Bluetooth_OVER_SAMPLE_COUNT - 2u)

#define Bluetooth_FIFO_LENGTH                        (4u)
#define Bluetooth_NUMBER_OF_START_BIT                (1u)
#define Bluetooth_MAX_BYTE_VALUE                     (0xFFu)

/* 8X always for count7 implementation */
#define Bluetooth_TXBITCTR_BREAKBITS8X   ((Bluetooth_BREAK_BITS_TX * Bluetooth_OVER_SAMPLE_8) - 1u)
/* 8X or 16X for DP implementation */
#define Bluetooth_TXBITCTR_BREAKBITS ((Bluetooth_BREAK_BITS_TX * Bluetooth_OVER_SAMPLE_COUNT) - 1u)

#define Bluetooth_HALF_BIT_COUNT   \
                            (((Bluetooth_OVER_SAMPLE_COUNT / 2u) + (Bluetooth_USE23POLLING * 1u)) - 2u)
#if (Bluetooth_OVER_SAMPLE_COUNT == Bluetooth_OVER_SAMPLE_8)
    #define Bluetooth_HD_TXBITCTR_INIT   (((Bluetooth_BREAK_BITS_TX + \
                            Bluetooth_NUMBER_OF_START_BIT) * Bluetooth_OVER_SAMPLE_COUNT) - 1u)

    /* This parameter is increased on the 2 in 2 out of 3 mode to sample voting in the middle */
    #define Bluetooth_RXBITCTR_INIT  ((((Bluetooth_BREAK_BITS_RX + Bluetooth_NUMBER_OF_START_BIT) \
                            * Bluetooth_OVER_SAMPLE_COUNT) + Bluetooth_HALF_BIT_COUNT) - 1u)

#else /* Bluetooth_OVER_SAMPLE_COUNT == Bluetooth_OVER_SAMPLE_16 */
    #define Bluetooth_HD_TXBITCTR_INIT   ((8u * Bluetooth_OVER_SAMPLE_COUNT) - 1u)
    /* 7bit counter need one more bit for OverSampleCount = 16 */
    #define Bluetooth_RXBITCTR_INIT      (((7u * Bluetooth_OVER_SAMPLE_COUNT) - 1u) + \
                                                      Bluetooth_HALF_BIT_COUNT)
#endif /* End Bluetooth_OVER_SAMPLE_COUNT */

#define Bluetooth_HD_RXBITCTR_INIT                   Bluetooth_RXBITCTR_INIT


/***************************************
* Global variables external identifier
***************************************/

extern uint8 Bluetooth_initVar;
#if (Bluetooth_TX_INTERRUPT_ENABLED && Bluetooth_TX_ENABLED)
    extern volatile uint8 Bluetooth_txBuffer[Bluetooth_TX_BUFFER_SIZE];
    extern volatile uint8 Bluetooth_txBufferRead;
    extern uint8 Bluetooth_txBufferWrite;
#endif /* (Bluetooth_TX_INTERRUPT_ENABLED && Bluetooth_TX_ENABLED) */
#if (Bluetooth_RX_INTERRUPT_ENABLED && (Bluetooth_RX_ENABLED || Bluetooth_HD_ENABLED))
    extern uint8 Bluetooth_errorStatus;
    extern volatile uint8 Bluetooth_rxBuffer[Bluetooth_RX_BUFFER_SIZE];
    extern volatile uint8 Bluetooth_rxBufferRead;
    extern volatile uint8 Bluetooth_rxBufferWrite;
    extern volatile uint8 Bluetooth_rxBufferLoopDetect;
    extern volatile uint8 Bluetooth_rxBufferOverflow;
    #if (Bluetooth_RXHW_ADDRESS_ENABLED)
        extern volatile uint8 Bluetooth_rxAddressMode;
        extern volatile uint8 Bluetooth_rxAddressDetected;
    #endif /* (Bluetooth_RXHW_ADDRESS_ENABLED) */
#endif /* (Bluetooth_RX_INTERRUPT_ENABLED && (Bluetooth_RX_ENABLED || Bluetooth_HD_ENABLED)) */


/***************************************
* Enumerated Types and Parameters
***************************************/

#define Bluetooth__B_UART__AM_SW_BYTE_BYTE 1
#define Bluetooth__B_UART__AM_SW_DETECT_TO_BUFFER 2
#define Bluetooth__B_UART__AM_HW_BYTE_BY_BYTE 3
#define Bluetooth__B_UART__AM_HW_DETECT_TO_BUFFER 4
#define Bluetooth__B_UART__AM_NONE 0

#define Bluetooth__B_UART__NONE_REVB 0
#define Bluetooth__B_UART__EVEN_REVB 1
#define Bluetooth__B_UART__ODD_REVB 2
#define Bluetooth__B_UART__MARK_SPACE_REVB 3



/***************************************
*    Initial Parameter Constants
***************************************/

/* UART shifts max 8 bits, Mark/Space functionality working if 9 selected */
#define Bluetooth_NUMBER_OF_DATA_BITS    ((8u > 8u) ? 8u : 8u)
#define Bluetooth_NUMBER_OF_STOP_BITS    (1u)

#if (Bluetooth_RXHW_ADDRESS_ENABLED)
    #define Bluetooth_RX_ADDRESS_MODE    (0u)
    #define Bluetooth_RX_HW_ADDRESS1     (0u)
    #define Bluetooth_RX_HW_ADDRESS2     (0u)
#endif /* (Bluetooth_RXHW_ADDRESS_ENABLED) */

#define Bluetooth_INIT_RX_INTERRUPTS_MASK \
                                  (uint8)((1 << Bluetooth_RX_STS_FIFO_NOTEMPTY_SHIFT) \
                                        | (0 << Bluetooth_RX_STS_MRKSPC_SHIFT) \
                                        | (0 << Bluetooth_RX_STS_ADDR_MATCH_SHIFT) \
                                        | (0 << Bluetooth_RX_STS_PAR_ERROR_SHIFT) \
                                        | (0 << Bluetooth_RX_STS_STOP_ERROR_SHIFT) \
                                        | (0 << Bluetooth_RX_STS_BREAK_SHIFT) \
                                        | (0 << Bluetooth_RX_STS_OVERRUN_SHIFT))

#define Bluetooth_INIT_TX_INTERRUPTS_MASK \
                                  (uint8)((0 << Bluetooth_TX_STS_COMPLETE_SHIFT) \
                                        | (0 << Bluetooth_TX_STS_FIFO_EMPTY_SHIFT) \
                                        | (0 << Bluetooth_TX_STS_FIFO_FULL_SHIFT) \
                                        | (0 << Bluetooth_TX_STS_FIFO_NOT_FULL_SHIFT))


/***************************************
*              Registers
***************************************/

#ifdef Bluetooth_BUART_sCR_SyncCtl_CtrlReg__CONTROL_REG
    #define Bluetooth_CONTROL_REG \
                            (* (reg8 *) Bluetooth_BUART_sCR_SyncCtl_CtrlReg__CONTROL_REG )
    #define Bluetooth_CONTROL_PTR \
                            (  (reg8 *) Bluetooth_BUART_sCR_SyncCtl_CtrlReg__CONTROL_REG )
#endif /* End Bluetooth_BUART_sCR_SyncCtl_CtrlReg__CONTROL_REG */

#if(Bluetooth_TX_ENABLED)
    #define Bluetooth_TXDATA_REG          (* (reg8 *) Bluetooth_BUART_sTX_TxShifter_u0__F0_REG)
    #define Bluetooth_TXDATA_PTR          (  (reg8 *) Bluetooth_BUART_sTX_TxShifter_u0__F0_REG)
    #define Bluetooth_TXDATA_AUX_CTL_REG  (* (reg8 *) Bluetooth_BUART_sTX_TxShifter_u0__DP_AUX_CTL_REG)
    #define Bluetooth_TXDATA_AUX_CTL_PTR  (  (reg8 *) Bluetooth_BUART_sTX_TxShifter_u0__DP_AUX_CTL_REG)
    #define Bluetooth_TXSTATUS_REG        (* (reg8 *) Bluetooth_BUART_sTX_TxSts__STATUS_REG)
    #define Bluetooth_TXSTATUS_PTR        (  (reg8 *) Bluetooth_BUART_sTX_TxSts__STATUS_REG)
    #define Bluetooth_TXSTATUS_MASK_REG   (* (reg8 *) Bluetooth_BUART_sTX_TxSts__MASK_REG)
    #define Bluetooth_TXSTATUS_MASK_PTR   (  (reg8 *) Bluetooth_BUART_sTX_TxSts__MASK_REG)
    #define Bluetooth_TXSTATUS_ACTL_REG   (* (reg8 *) Bluetooth_BUART_sTX_TxSts__STATUS_AUX_CTL_REG)
    #define Bluetooth_TXSTATUS_ACTL_PTR   (  (reg8 *) Bluetooth_BUART_sTX_TxSts__STATUS_AUX_CTL_REG)

    /* DP clock */
    #if(Bluetooth_TXCLKGEN_DP)
        #define Bluetooth_TXBITCLKGEN_CTR_REG        \
                                        (* (reg8 *) Bluetooth_BUART_sTX_sCLOCK_TxBitClkGen__D0_REG)
        #define Bluetooth_TXBITCLKGEN_CTR_PTR        \
                                        (  (reg8 *) Bluetooth_BUART_sTX_sCLOCK_TxBitClkGen__D0_REG)
        #define Bluetooth_TXBITCLKTX_COMPLETE_REG    \
                                        (* (reg8 *) Bluetooth_BUART_sTX_sCLOCK_TxBitClkGen__D1_REG)
        #define Bluetooth_TXBITCLKTX_COMPLETE_PTR    \
                                        (  (reg8 *) Bluetooth_BUART_sTX_sCLOCK_TxBitClkGen__D1_REG)
    #else     /* Count7 clock*/
        #define Bluetooth_TXBITCTR_PERIOD_REG    \
                                        (* (reg8 *) Bluetooth_BUART_sTX_sCLOCK_TxBitCounter__PERIOD_REG)
        #define Bluetooth_TXBITCTR_PERIOD_PTR    \
                                        (  (reg8 *) Bluetooth_BUART_sTX_sCLOCK_TxBitCounter__PERIOD_REG)
        #define Bluetooth_TXBITCTR_CONTROL_REG   \
                                        (* (reg8 *) Bluetooth_BUART_sTX_sCLOCK_TxBitCounter__CONTROL_AUX_CTL_REG)
        #define Bluetooth_TXBITCTR_CONTROL_PTR   \
                                        (  (reg8 *) Bluetooth_BUART_sTX_sCLOCK_TxBitCounter__CONTROL_AUX_CTL_REG)
        #define Bluetooth_TXBITCTR_COUNTER_REG   \
                                        (* (reg8 *) Bluetooth_BUART_sTX_sCLOCK_TxBitCounter__COUNT_REG)
        #define Bluetooth_TXBITCTR_COUNTER_PTR   \
                                        (  (reg8 *) Bluetooth_BUART_sTX_sCLOCK_TxBitCounter__COUNT_REG)
    #endif /* Bluetooth_TXCLKGEN_DP */

#endif /* End Bluetooth_TX_ENABLED */

#if(Bluetooth_HD_ENABLED)

    #define Bluetooth_TXDATA_REG             (* (reg8 *) Bluetooth_BUART_sRX_RxShifter_u0__F1_REG )
    #define Bluetooth_TXDATA_PTR             (  (reg8 *) Bluetooth_BUART_sRX_RxShifter_u0__F1_REG )
    #define Bluetooth_TXDATA_AUX_CTL_REG     (* (reg8 *) Bluetooth_BUART_sRX_RxShifter_u0__DP_AUX_CTL_REG)
    #define Bluetooth_TXDATA_AUX_CTL_PTR     (  (reg8 *) Bluetooth_BUART_sRX_RxShifter_u0__DP_AUX_CTL_REG)

    #define Bluetooth_TXSTATUS_REG           (* (reg8 *) Bluetooth_BUART_sRX_RxSts__STATUS_REG )
    #define Bluetooth_TXSTATUS_PTR           (  (reg8 *) Bluetooth_BUART_sRX_RxSts__STATUS_REG )
    #define Bluetooth_TXSTATUS_MASK_REG      (* (reg8 *) Bluetooth_BUART_sRX_RxSts__MASK_REG )
    #define Bluetooth_TXSTATUS_MASK_PTR      (  (reg8 *) Bluetooth_BUART_sRX_RxSts__MASK_REG )
    #define Bluetooth_TXSTATUS_ACTL_REG      (* (reg8 *) Bluetooth_BUART_sRX_RxSts__STATUS_AUX_CTL_REG )
    #define Bluetooth_TXSTATUS_ACTL_PTR      (  (reg8 *) Bluetooth_BUART_sRX_RxSts__STATUS_AUX_CTL_REG )
#endif /* End Bluetooth_HD_ENABLED */

#if( (Bluetooth_RX_ENABLED) || (Bluetooth_HD_ENABLED) )
    #define Bluetooth_RXDATA_REG             (* (reg8 *) Bluetooth_BUART_sRX_RxShifter_u0__F0_REG )
    #define Bluetooth_RXDATA_PTR             (  (reg8 *) Bluetooth_BUART_sRX_RxShifter_u0__F0_REG )
    #define Bluetooth_RXADDRESS1_REG         (* (reg8 *) Bluetooth_BUART_sRX_RxShifter_u0__D0_REG )
    #define Bluetooth_RXADDRESS1_PTR         (  (reg8 *) Bluetooth_BUART_sRX_RxShifter_u0__D0_REG )
    #define Bluetooth_RXADDRESS2_REG         (* (reg8 *) Bluetooth_BUART_sRX_RxShifter_u0__D1_REG )
    #define Bluetooth_RXADDRESS2_PTR         (  (reg8 *) Bluetooth_BUART_sRX_RxShifter_u0__D1_REG )
    #define Bluetooth_RXDATA_AUX_CTL_REG     (* (reg8 *) Bluetooth_BUART_sRX_RxShifter_u0__DP_AUX_CTL_REG)

    #define Bluetooth_RXBITCTR_PERIOD_REG    (* (reg8 *) Bluetooth_BUART_sRX_RxBitCounter__PERIOD_REG )
    #define Bluetooth_RXBITCTR_PERIOD_PTR    (  (reg8 *) Bluetooth_BUART_sRX_RxBitCounter__PERIOD_REG )
    #define Bluetooth_RXBITCTR_CONTROL_REG   \
                                        (* (reg8 *) Bluetooth_BUART_sRX_RxBitCounter__CONTROL_AUX_CTL_REG )
    #define Bluetooth_RXBITCTR_CONTROL_PTR   \
                                        (  (reg8 *) Bluetooth_BUART_sRX_RxBitCounter__CONTROL_AUX_CTL_REG )
    #define Bluetooth_RXBITCTR_COUNTER_REG   (* (reg8 *) Bluetooth_BUART_sRX_RxBitCounter__COUNT_REG )
    #define Bluetooth_RXBITCTR_COUNTER_PTR   (  (reg8 *) Bluetooth_BUART_sRX_RxBitCounter__COUNT_REG )

    #define Bluetooth_RXSTATUS_REG           (* (reg8 *) Bluetooth_BUART_sRX_RxSts__STATUS_REG )
    #define Bluetooth_RXSTATUS_PTR           (  (reg8 *) Bluetooth_BUART_sRX_RxSts__STATUS_REG )
    #define Bluetooth_RXSTATUS_MASK_REG      (* (reg8 *) Bluetooth_BUART_sRX_RxSts__MASK_REG )
    #define Bluetooth_RXSTATUS_MASK_PTR      (  (reg8 *) Bluetooth_BUART_sRX_RxSts__MASK_REG )
    #define Bluetooth_RXSTATUS_ACTL_REG      (* (reg8 *) Bluetooth_BUART_sRX_RxSts__STATUS_AUX_CTL_REG )
    #define Bluetooth_RXSTATUS_ACTL_PTR      (  (reg8 *) Bluetooth_BUART_sRX_RxSts__STATUS_AUX_CTL_REG )
#endif /* End  (Bluetooth_RX_ENABLED) || (Bluetooth_HD_ENABLED) */

#if(Bluetooth_INTERNAL_CLOCK_USED)
    /* Register to enable or disable the digital clocks */
    #define Bluetooth_INTCLOCK_CLKEN_REG     (* (reg8 *) Bluetooth_IntClock__PM_ACT_CFG)
    #define Bluetooth_INTCLOCK_CLKEN_PTR     (  (reg8 *) Bluetooth_IntClock__PM_ACT_CFG)

    /* Clock mask for this clock. */
    #define Bluetooth_INTCLOCK_CLKEN_MASK    Bluetooth_IntClock__PM_ACT_MSK
#endif /* End Bluetooth_INTERNAL_CLOCK_USED */


/***************************************
*       Register Constants
***************************************/

#if(Bluetooth_TX_ENABLED)
    #define Bluetooth_TX_FIFO_CLR            (0x01u) /* FIFO0 CLR */
#endif /* End Bluetooth_TX_ENABLED */

#if(Bluetooth_HD_ENABLED)
    #define Bluetooth_TX_FIFO_CLR            (0x02u) /* FIFO1 CLR */
#endif /* End Bluetooth_HD_ENABLED */

#if( (Bluetooth_RX_ENABLED) || (Bluetooth_HD_ENABLED) )
    #define Bluetooth_RX_FIFO_CLR            (0x01u) /* FIFO0 CLR */
#endif /* End  (Bluetooth_RX_ENABLED) || (Bluetooth_HD_ENABLED) */


/***************************************
* The following code is DEPRECATED and
* should not be used in new projects.
***************************************/

/* UART v2_40 obsolete definitions */
#define Bluetooth_WAIT_1_MS      Bluetooth_BL_CHK_DELAY_MS   

#define Bluetooth_TXBUFFERSIZE   Bluetooth_TX_BUFFER_SIZE
#define Bluetooth_RXBUFFERSIZE   Bluetooth_RX_BUFFER_SIZE

#if (Bluetooth_RXHW_ADDRESS_ENABLED)
    #define Bluetooth_RXADDRESSMODE  Bluetooth_RX_ADDRESS_MODE
    #define Bluetooth_RXHWADDRESS1   Bluetooth_RX_HW_ADDRESS1
    #define Bluetooth_RXHWADDRESS2   Bluetooth_RX_HW_ADDRESS2
    /* Backward compatible define */
    #define Bluetooth_RXAddressMode  Bluetooth_RXADDRESSMODE
#endif /* (Bluetooth_RXHW_ADDRESS_ENABLED) */

/* UART v2_30 obsolete definitions */
#define Bluetooth_initvar                    Bluetooth_initVar

#define Bluetooth_RX_Enabled                 Bluetooth_RX_ENABLED
#define Bluetooth_TX_Enabled                 Bluetooth_TX_ENABLED
#define Bluetooth_HD_Enabled                 Bluetooth_HD_ENABLED
#define Bluetooth_RX_IntInterruptEnabled     Bluetooth_RX_INTERRUPT_ENABLED
#define Bluetooth_TX_IntInterruptEnabled     Bluetooth_TX_INTERRUPT_ENABLED
#define Bluetooth_InternalClockUsed          Bluetooth_INTERNAL_CLOCK_USED
#define Bluetooth_RXHW_Address_Enabled       Bluetooth_RXHW_ADDRESS_ENABLED
#define Bluetooth_OverSampleCount            Bluetooth_OVER_SAMPLE_COUNT
#define Bluetooth_ParityType                 Bluetooth_PARITY_TYPE

#if( Bluetooth_TX_ENABLED && (Bluetooth_TXBUFFERSIZE > Bluetooth_FIFO_LENGTH))
    #define Bluetooth_TXBUFFER               Bluetooth_txBuffer
    #define Bluetooth_TXBUFFERREAD           Bluetooth_txBufferRead
    #define Bluetooth_TXBUFFERWRITE          Bluetooth_txBufferWrite
#endif /* End Bluetooth_TX_ENABLED */
#if( ( Bluetooth_RX_ENABLED || Bluetooth_HD_ENABLED ) && \
     (Bluetooth_RXBUFFERSIZE > Bluetooth_FIFO_LENGTH) )
    #define Bluetooth_RXBUFFER               Bluetooth_rxBuffer
    #define Bluetooth_RXBUFFERREAD           Bluetooth_rxBufferRead
    #define Bluetooth_RXBUFFERWRITE          Bluetooth_rxBufferWrite
    #define Bluetooth_RXBUFFERLOOPDETECT     Bluetooth_rxBufferLoopDetect
    #define Bluetooth_RXBUFFER_OVERFLOW      Bluetooth_rxBufferOverflow
#endif /* End Bluetooth_RX_ENABLED */

#ifdef Bluetooth_BUART_sCR_SyncCtl_CtrlReg__CONTROL_REG
    #define Bluetooth_CONTROL                Bluetooth_CONTROL_REG
#endif /* End Bluetooth_BUART_sCR_SyncCtl_CtrlReg__CONTROL_REG */

#if(Bluetooth_TX_ENABLED)
    #define Bluetooth_TXDATA                 Bluetooth_TXDATA_REG
    #define Bluetooth_TXSTATUS               Bluetooth_TXSTATUS_REG
    #define Bluetooth_TXSTATUS_MASK          Bluetooth_TXSTATUS_MASK_REG
    #define Bluetooth_TXSTATUS_ACTL          Bluetooth_TXSTATUS_ACTL_REG
    /* DP clock */
    #if(Bluetooth_TXCLKGEN_DP)
        #define Bluetooth_TXBITCLKGEN_CTR        Bluetooth_TXBITCLKGEN_CTR_REG
        #define Bluetooth_TXBITCLKTX_COMPLETE    Bluetooth_TXBITCLKTX_COMPLETE_REG
    #else     /* Count7 clock*/
        #define Bluetooth_TXBITCTR_PERIOD        Bluetooth_TXBITCTR_PERIOD_REG
        #define Bluetooth_TXBITCTR_CONTROL       Bluetooth_TXBITCTR_CONTROL_REG
        #define Bluetooth_TXBITCTR_COUNTER       Bluetooth_TXBITCTR_COUNTER_REG
    #endif /* Bluetooth_TXCLKGEN_DP */
#endif /* End Bluetooth_TX_ENABLED */

#if(Bluetooth_HD_ENABLED)
    #define Bluetooth_TXDATA                 Bluetooth_TXDATA_REG
    #define Bluetooth_TXSTATUS               Bluetooth_TXSTATUS_REG
    #define Bluetooth_TXSTATUS_MASK          Bluetooth_TXSTATUS_MASK_REG
    #define Bluetooth_TXSTATUS_ACTL          Bluetooth_TXSTATUS_ACTL_REG
#endif /* End Bluetooth_HD_ENABLED */

#if( (Bluetooth_RX_ENABLED) || (Bluetooth_HD_ENABLED) )
    #define Bluetooth_RXDATA                 Bluetooth_RXDATA_REG
    #define Bluetooth_RXADDRESS1             Bluetooth_RXADDRESS1_REG
    #define Bluetooth_RXADDRESS2             Bluetooth_RXADDRESS2_REG
    #define Bluetooth_RXBITCTR_PERIOD        Bluetooth_RXBITCTR_PERIOD_REG
    #define Bluetooth_RXBITCTR_CONTROL       Bluetooth_RXBITCTR_CONTROL_REG
    #define Bluetooth_RXBITCTR_COUNTER       Bluetooth_RXBITCTR_COUNTER_REG
    #define Bluetooth_RXSTATUS               Bluetooth_RXSTATUS_REG
    #define Bluetooth_RXSTATUS_MASK          Bluetooth_RXSTATUS_MASK_REG
    #define Bluetooth_RXSTATUS_ACTL          Bluetooth_RXSTATUS_ACTL_REG
#endif /* End  (Bluetooth_RX_ENABLED) || (Bluetooth_HD_ENABLED) */

#if(Bluetooth_INTERNAL_CLOCK_USED)
    #define Bluetooth_INTCLOCK_CLKEN         Bluetooth_INTCLOCK_CLKEN_REG
#endif /* End Bluetooth_INTERNAL_CLOCK_USED */

#define Bluetooth_WAIT_FOR_COMLETE_REINIT    Bluetooth_WAIT_FOR_COMPLETE_REINIT

#endif  /* CY_UART_Bluetooth_H */


/* [] END OF FILE */
