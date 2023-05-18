/*******************************************************************************
* File Name: RFID.h
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


#if !defined(CY_UART_RFID_H)
#define CY_UART_RFID_H

#include "cyfitter.h"
#include "cytypes.h"
#include "CyLib.h" /* For CyEnterCriticalSection() and CyExitCriticalSection() functions */


/***************************************
* Conditional Compilation Parameters
***************************************/

#define RFID_RX_ENABLED                     (1u)
#define RFID_TX_ENABLED                     (1u)
#define RFID_HD_ENABLED                     (0u)
#define RFID_RX_INTERRUPT_ENABLED           (0u)
#define RFID_TX_INTERRUPT_ENABLED           (0u)
#define RFID_INTERNAL_CLOCK_USED            (1u)
#define RFID_RXHW_ADDRESS_ENABLED           (0u)
#define RFID_OVER_SAMPLE_COUNT              (8u)
#define RFID_PARITY_TYPE                    (0u)
#define RFID_PARITY_TYPE_SW                 (0u)
#define RFID_BREAK_DETECT                   (0u)
#define RFID_BREAK_BITS_TX                  (13u)
#define RFID_BREAK_BITS_RX                  (13u)
#define RFID_TXCLKGEN_DP                    (1u)
#define RFID_USE23POLLING                   (1u)
#define RFID_FLOW_CONTROL                   (0u)
#define RFID_CLK_FREQ                       (0u)
#define RFID_TX_BUFFER_SIZE                 (4u)
#define RFID_RX_BUFFER_SIZE                 (4u)

/* Check to see if required defines such as CY_PSOC5LP are available */
/* They are defined starting with cy_boot v3.0 */
#if !defined (CY_PSOC5LP)
    #error Component UART_v2_50 requires cy_boot v3.0 or later
#endif /* (CY_PSOC5LP) */

#if defined(RFID_BUART_sCR_SyncCtl_CtrlReg__CONTROL_REG)
    #define RFID_CONTROL_REG_REMOVED            (0u)
#else
    #define RFID_CONTROL_REG_REMOVED            (1u)
#endif /* End RFID_BUART_sCR_SyncCtl_CtrlReg__CONTROL_REG */


/***************************************
*      Data Structure Definition
***************************************/

/* Sleep Mode API Support */
typedef struct RFID_backupStruct_
{
    uint8 enableState;

    #if(RFID_CONTROL_REG_REMOVED == 0u)
        uint8 cr;
    #endif /* End RFID_CONTROL_REG_REMOVED */

} RFID_BACKUP_STRUCT;


/***************************************
*       Function Prototypes
***************************************/

void RFID_Start(void) ;
void RFID_Stop(void) ;
uint8 RFID_ReadControlRegister(void) ;
void RFID_WriteControlRegister(uint8 control) ;

void RFID_Init(void) ;
void RFID_Enable(void) ;
void RFID_SaveConfig(void) ;
void RFID_RestoreConfig(void) ;
void RFID_Sleep(void) ;
void RFID_Wakeup(void) ;

/* Only if RX is enabled */
#if( (RFID_RX_ENABLED) || (RFID_HD_ENABLED) )

    #if (RFID_RX_INTERRUPT_ENABLED)
        #define RFID_EnableRxInt()  CyIntEnable (RFID_RX_VECT_NUM)
        #define RFID_DisableRxInt() CyIntDisable(RFID_RX_VECT_NUM)
        CY_ISR_PROTO(RFID_RXISR);
    #endif /* RFID_RX_INTERRUPT_ENABLED */

    void RFID_SetRxAddressMode(uint8 addressMode)
                                                           ;
    void RFID_SetRxAddress1(uint8 address) ;
    void RFID_SetRxAddress2(uint8 address) ;

    void  RFID_SetRxInterruptMode(uint8 intSrc) ;
    uint8 RFID_ReadRxData(void) ;
    uint8 RFID_ReadRxStatus(void) ;
    uint8 RFID_GetChar(void) ;
    uint16 RFID_GetByte(void) ;
    uint8 RFID_GetRxBufferSize(void)
                                                            ;
    void RFID_ClearRxBuffer(void) ;

    /* Obsolete functions, defines for backward compatible */
    #define RFID_GetRxInterruptSource   RFID_ReadRxStatus

#endif /* End (RFID_RX_ENABLED) || (RFID_HD_ENABLED) */

/* Only if TX is enabled */
#if(RFID_TX_ENABLED || RFID_HD_ENABLED)

    #if(RFID_TX_INTERRUPT_ENABLED)
        #define RFID_EnableTxInt()  CyIntEnable (RFID_TX_VECT_NUM)
        #define RFID_DisableTxInt() CyIntDisable(RFID_TX_VECT_NUM)
        #define RFID_SetPendingTxInt() CyIntSetPending(RFID_TX_VECT_NUM)
        #define RFID_ClearPendingTxInt() CyIntClearPending(RFID_TX_VECT_NUM)
        CY_ISR_PROTO(RFID_TXISR);
    #endif /* RFID_TX_INTERRUPT_ENABLED */

    void RFID_SetTxInterruptMode(uint8 intSrc) ;
    void RFID_WriteTxData(uint8 txDataByte) ;
    uint8 RFID_ReadTxStatus(void) ;
    void RFID_PutChar(uint8 txDataByte) ;
    void RFID_PutString(const char8 string[]) ;
    void RFID_PutArray(const uint8 string[], uint8 byteCount)
                                                            ;
    void RFID_PutCRLF(uint8 txDataByte) ;
    void RFID_ClearTxBuffer(void) ;
    void RFID_SetTxAddressMode(uint8 addressMode) ;
    void RFID_SendBreak(uint8 retMode) ;
    uint8 RFID_GetTxBufferSize(void)
                                                            ;
    /* Obsolete functions, defines for backward compatible */
    #define RFID_PutStringConst         RFID_PutString
    #define RFID_PutArrayConst          RFID_PutArray
    #define RFID_GetTxInterruptSource   RFID_ReadTxStatus

#endif /* End RFID_TX_ENABLED || RFID_HD_ENABLED */

#if(RFID_HD_ENABLED)
    void RFID_LoadRxConfig(void) ;
    void RFID_LoadTxConfig(void) ;
#endif /* End RFID_HD_ENABLED */


/* Communication bootloader APIs */
#if defined(CYDEV_BOOTLOADER_IO_COMP) && ((CYDEV_BOOTLOADER_IO_COMP == CyBtldr_RFID) || \
                                          (CYDEV_BOOTLOADER_IO_COMP == CyBtldr_Custom_Interface))
    /* Physical layer functions */
    void    RFID_CyBtldrCommStart(void) CYSMALL ;
    void    RFID_CyBtldrCommStop(void) CYSMALL ;
    void    RFID_CyBtldrCommReset(void) CYSMALL ;
    cystatus RFID_CyBtldrCommWrite(const uint8 pData[], uint16 size, uint16 * count, uint8 timeOut) CYSMALL
             ;
    cystatus RFID_CyBtldrCommRead(uint8 pData[], uint16 size, uint16 * count, uint8 timeOut) CYSMALL
             ;

    #if (CYDEV_BOOTLOADER_IO_COMP == CyBtldr_RFID)
        #define CyBtldrCommStart    RFID_CyBtldrCommStart
        #define CyBtldrCommStop     RFID_CyBtldrCommStop
        #define CyBtldrCommReset    RFID_CyBtldrCommReset
        #define CyBtldrCommWrite    RFID_CyBtldrCommWrite
        #define CyBtldrCommRead     RFID_CyBtldrCommRead
    #endif  /* (CYDEV_BOOTLOADER_IO_COMP == CyBtldr_RFID) */

    /* Byte to Byte time out for detecting end of block data from host */
    #define RFID_BYTE2BYTE_TIME_OUT (25u)
    #define RFID_PACKET_EOP         (0x17u) /* End of packet defined by bootloader */
    #define RFID_WAIT_EOP_DELAY     (5u)    /* Additional 5ms to wait for End of packet */
    #define RFID_BL_CHK_DELAY_MS    (1u)    /* Time Out quantity equal 1mS */

#endif /* CYDEV_BOOTLOADER_IO_COMP */


/***************************************
*          API Constants
***************************************/
/* Parameters for SetTxAddressMode API*/
#define RFID_SET_SPACE      (0x00u)
#define RFID_SET_MARK       (0x01u)

/* Status Register definitions */
#if( (RFID_TX_ENABLED) || (RFID_HD_ENABLED) )
    #if(RFID_TX_INTERRUPT_ENABLED)
        #define RFID_TX_VECT_NUM            (uint8)RFID_TXInternalInterrupt__INTC_NUMBER
        #define RFID_TX_PRIOR_NUM           (uint8)RFID_TXInternalInterrupt__INTC_PRIOR_NUM
    #endif /* RFID_TX_INTERRUPT_ENABLED */

    #define RFID_TX_STS_COMPLETE_SHIFT          (0x00u)
    #define RFID_TX_STS_FIFO_EMPTY_SHIFT        (0x01u)
    #define RFID_TX_STS_FIFO_NOT_FULL_SHIFT     (0x03u)
    #if(RFID_TX_ENABLED)
        #define RFID_TX_STS_FIFO_FULL_SHIFT     (0x02u)
    #else /* (RFID_HD_ENABLED) */
        #define RFID_TX_STS_FIFO_FULL_SHIFT     (0x05u)  /* Needs MD=0 */
    #endif /* (RFID_TX_ENABLED) */

    #define RFID_TX_STS_COMPLETE            (uint8)(0x01u << RFID_TX_STS_COMPLETE_SHIFT)
    #define RFID_TX_STS_FIFO_EMPTY          (uint8)(0x01u << RFID_TX_STS_FIFO_EMPTY_SHIFT)
    #define RFID_TX_STS_FIFO_FULL           (uint8)(0x01u << RFID_TX_STS_FIFO_FULL_SHIFT)
    #define RFID_TX_STS_FIFO_NOT_FULL       (uint8)(0x01u << RFID_TX_STS_FIFO_NOT_FULL_SHIFT)
#endif /* End (RFID_TX_ENABLED) || (RFID_HD_ENABLED)*/

#if( (RFID_RX_ENABLED) || (RFID_HD_ENABLED) )
    #if(RFID_RX_INTERRUPT_ENABLED)
        #define RFID_RX_VECT_NUM            (uint8)RFID_RXInternalInterrupt__INTC_NUMBER
        #define RFID_RX_PRIOR_NUM           (uint8)RFID_RXInternalInterrupt__INTC_PRIOR_NUM
    #endif /* RFID_RX_INTERRUPT_ENABLED */
    #define RFID_RX_STS_MRKSPC_SHIFT            (0x00u)
    #define RFID_RX_STS_BREAK_SHIFT             (0x01u)
    #define RFID_RX_STS_PAR_ERROR_SHIFT         (0x02u)
    #define RFID_RX_STS_STOP_ERROR_SHIFT        (0x03u)
    #define RFID_RX_STS_OVERRUN_SHIFT           (0x04u)
    #define RFID_RX_STS_FIFO_NOTEMPTY_SHIFT     (0x05u)
    #define RFID_RX_STS_ADDR_MATCH_SHIFT        (0x06u)
    #define RFID_RX_STS_SOFT_BUFF_OVER_SHIFT    (0x07u)

    #define RFID_RX_STS_MRKSPC           (uint8)(0x01u << RFID_RX_STS_MRKSPC_SHIFT)
    #define RFID_RX_STS_BREAK            (uint8)(0x01u << RFID_RX_STS_BREAK_SHIFT)
    #define RFID_RX_STS_PAR_ERROR        (uint8)(0x01u << RFID_RX_STS_PAR_ERROR_SHIFT)
    #define RFID_RX_STS_STOP_ERROR       (uint8)(0x01u << RFID_RX_STS_STOP_ERROR_SHIFT)
    #define RFID_RX_STS_OVERRUN          (uint8)(0x01u << RFID_RX_STS_OVERRUN_SHIFT)
    #define RFID_RX_STS_FIFO_NOTEMPTY    (uint8)(0x01u << RFID_RX_STS_FIFO_NOTEMPTY_SHIFT)
    #define RFID_RX_STS_ADDR_MATCH       (uint8)(0x01u << RFID_RX_STS_ADDR_MATCH_SHIFT)
    #define RFID_RX_STS_SOFT_BUFF_OVER   (uint8)(0x01u << RFID_RX_STS_SOFT_BUFF_OVER_SHIFT)
    #define RFID_RX_HW_MASK                     (0x7Fu)
#endif /* End (RFID_RX_ENABLED) || (RFID_HD_ENABLED) */

/* Control Register definitions */
#define RFID_CTRL_HD_SEND_SHIFT                 (0x00u) /* 1 enable TX part in Half Duplex mode */
#define RFID_CTRL_HD_SEND_BREAK_SHIFT           (0x01u) /* 1 send BREAK signal in Half Duplez mode */
#define RFID_CTRL_MARK_SHIFT                    (0x02u) /* 1 sets mark, 0 sets space */
#define RFID_CTRL_PARITY_TYPE0_SHIFT            (0x03u) /* Defines the type of parity implemented */
#define RFID_CTRL_PARITY_TYPE1_SHIFT            (0x04u) /* Defines the type of parity implemented */
#define RFID_CTRL_RXADDR_MODE0_SHIFT            (0x05u)
#define RFID_CTRL_RXADDR_MODE1_SHIFT            (0x06u)
#define RFID_CTRL_RXADDR_MODE2_SHIFT            (0x07u)

#define RFID_CTRL_HD_SEND               (uint8)(0x01u << RFID_CTRL_HD_SEND_SHIFT)
#define RFID_CTRL_HD_SEND_BREAK         (uint8)(0x01u << RFID_CTRL_HD_SEND_BREAK_SHIFT)
#define RFID_CTRL_MARK                  (uint8)(0x01u << RFID_CTRL_MARK_SHIFT)
#define RFID_CTRL_PARITY_TYPE_MASK      (uint8)(0x03u << RFID_CTRL_PARITY_TYPE0_SHIFT)
#define RFID_CTRL_RXADDR_MODE_MASK      (uint8)(0x07u << RFID_CTRL_RXADDR_MODE0_SHIFT)

/* StatusI Register Interrupt Enable Control Bits. As defined by the Register map for the AUX Control Register */
#define RFID_INT_ENABLE                         (0x10u)

/* Bit Counter (7-bit) Control Register Bit Definitions. As defined by the Register map for the AUX Control Register */
#define RFID_CNTR_ENABLE                        (0x20u)

/*   Constants for SendBreak() "retMode" parameter  */
#define RFID_SEND_BREAK                         (0x00u)
#define RFID_WAIT_FOR_COMPLETE_REINIT           (0x01u)
#define RFID_REINIT                             (0x02u)
#define RFID_SEND_WAIT_REINIT                   (0x03u)

#define RFID_OVER_SAMPLE_8                      (8u)
#define RFID_OVER_SAMPLE_16                     (16u)

#define RFID_BIT_CENTER                         (RFID_OVER_SAMPLE_COUNT - 2u)

#define RFID_FIFO_LENGTH                        (4u)
#define RFID_NUMBER_OF_START_BIT                (1u)
#define RFID_MAX_BYTE_VALUE                     (0xFFu)

/* 8X always for count7 implementation */
#define RFID_TXBITCTR_BREAKBITS8X   ((RFID_BREAK_BITS_TX * RFID_OVER_SAMPLE_8) - 1u)
/* 8X or 16X for DP implementation */
#define RFID_TXBITCTR_BREAKBITS ((RFID_BREAK_BITS_TX * RFID_OVER_SAMPLE_COUNT) - 1u)

#define RFID_HALF_BIT_COUNT   \
                            (((RFID_OVER_SAMPLE_COUNT / 2u) + (RFID_USE23POLLING * 1u)) - 2u)
#if (RFID_OVER_SAMPLE_COUNT == RFID_OVER_SAMPLE_8)
    #define RFID_HD_TXBITCTR_INIT   (((RFID_BREAK_BITS_TX + \
                            RFID_NUMBER_OF_START_BIT) * RFID_OVER_SAMPLE_COUNT) - 1u)

    /* This parameter is increased on the 2 in 2 out of 3 mode to sample voting in the middle */
    #define RFID_RXBITCTR_INIT  ((((RFID_BREAK_BITS_RX + RFID_NUMBER_OF_START_BIT) \
                            * RFID_OVER_SAMPLE_COUNT) + RFID_HALF_BIT_COUNT) - 1u)

#else /* RFID_OVER_SAMPLE_COUNT == RFID_OVER_SAMPLE_16 */
    #define RFID_HD_TXBITCTR_INIT   ((8u * RFID_OVER_SAMPLE_COUNT) - 1u)
    /* 7bit counter need one more bit for OverSampleCount = 16 */
    #define RFID_RXBITCTR_INIT      (((7u * RFID_OVER_SAMPLE_COUNT) - 1u) + \
                                                      RFID_HALF_BIT_COUNT)
#endif /* End RFID_OVER_SAMPLE_COUNT */

#define RFID_HD_RXBITCTR_INIT                   RFID_RXBITCTR_INIT


/***************************************
* Global variables external identifier
***************************************/

extern uint8 RFID_initVar;
#if (RFID_TX_INTERRUPT_ENABLED && RFID_TX_ENABLED)
    extern volatile uint8 RFID_txBuffer[RFID_TX_BUFFER_SIZE];
    extern volatile uint8 RFID_txBufferRead;
    extern uint8 RFID_txBufferWrite;
#endif /* (RFID_TX_INTERRUPT_ENABLED && RFID_TX_ENABLED) */
#if (RFID_RX_INTERRUPT_ENABLED && (RFID_RX_ENABLED || RFID_HD_ENABLED))
    extern uint8 RFID_errorStatus;
    extern volatile uint8 RFID_rxBuffer[RFID_RX_BUFFER_SIZE];
    extern volatile uint8 RFID_rxBufferRead;
    extern volatile uint8 RFID_rxBufferWrite;
    extern volatile uint8 RFID_rxBufferLoopDetect;
    extern volatile uint8 RFID_rxBufferOverflow;
    #if (RFID_RXHW_ADDRESS_ENABLED)
        extern volatile uint8 RFID_rxAddressMode;
        extern volatile uint8 RFID_rxAddressDetected;
    #endif /* (RFID_RXHW_ADDRESS_ENABLED) */
#endif /* (RFID_RX_INTERRUPT_ENABLED && (RFID_RX_ENABLED || RFID_HD_ENABLED)) */


/***************************************
* Enumerated Types and Parameters
***************************************/

#define RFID__B_UART__AM_SW_BYTE_BYTE 1
#define RFID__B_UART__AM_SW_DETECT_TO_BUFFER 2
#define RFID__B_UART__AM_HW_BYTE_BY_BYTE 3
#define RFID__B_UART__AM_HW_DETECT_TO_BUFFER 4
#define RFID__B_UART__AM_NONE 0

#define RFID__B_UART__NONE_REVB 0
#define RFID__B_UART__EVEN_REVB 1
#define RFID__B_UART__ODD_REVB 2
#define RFID__B_UART__MARK_SPACE_REVB 3



/***************************************
*    Initial Parameter Constants
***************************************/

/* UART shifts max 8 bits, Mark/Space functionality working if 9 selected */
#define RFID_NUMBER_OF_DATA_BITS    ((8u > 8u) ? 8u : 8u)
#define RFID_NUMBER_OF_STOP_BITS    (1u)

#if (RFID_RXHW_ADDRESS_ENABLED)
    #define RFID_RX_ADDRESS_MODE    (0u)
    #define RFID_RX_HW_ADDRESS1     (0u)
    #define RFID_RX_HW_ADDRESS2     (0u)
#endif /* (RFID_RXHW_ADDRESS_ENABLED) */

#define RFID_INIT_RX_INTERRUPTS_MASK \
                                  (uint8)((1 << RFID_RX_STS_FIFO_NOTEMPTY_SHIFT) \
                                        | (0 << RFID_RX_STS_MRKSPC_SHIFT) \
                                        | (0 << RFID_RX_STS_ADDR_MATCH_SHIFT) \
                                        | (0 << RFID_RX_STS_PAR_ERROR_SHIFT) \
                                        | (0 << RFID_RX_STS_STOP_ERROR_SHIFT) \
                                        | (0 << RFID_RX_STS_BREAK_SHIFT) \
                                        | (0 << RFID_RX_STS_OVERRUN_SHIFT))

#define RFID_INIT_TX_INTERRUPTS_MASK \
                                  (uint8)((0 << RFID_TX_STS_COMPLETE_SHIFT) \
                                        | (0 << RFID_TX_STS_FIFO_EMPTY_SHIFT) \
                                        | (0 << RFID_TX_STS_FIFO_FULL_SHIFT) \
                                        | (0 << RFID_TX_STS_FIFO_NOT_FULL_SHIFT))


/***************************************
*              Registers
***************************************/

#ifdef RFID_BUART_sCR_SyncCtl_CtrlReg__CONTROL_REG
    #define RFID_CONTROL_REG \
                            (* (reg8 *) RFID_BUART_sCR_SyncCtl_CtrlReg__CONTROL_REG )
    #define RFID_CONTROL_PTR \
                            (  (reg8 *) RFID_BUART_sCR_SyncCtl_CtrlReg__CONTROL_REG )
#endif /* End RFID_BUART_sCR_SyncCtl_CtrlReg__CONTROL_REG */

#if(RFID_TX_ENABLED)
    #define RFID_TXDATA_REG          (* (reg8 *) RFID_BUART_sTX_TxShifter_u0__F0_REG)
    #define RFID_TXDATA_PTR          (  (reg8 *) RFID_BUART_sTX_TxShifter_u0__F0_REG)
    #define RFID_TXDATA_AUX_CTL_REG  (* (reg8 *) RFID_BUART_sTX_TxShifter_u0__DP_AUX_CTL_REG)
    #define RFID_TXDATA_AUX_CTL_PTR  (  (reg8 *) RFID_BUART_sTX_TxShifter_u0__DP_AUX_CTL_REG)
    #define RFID_TXSTATUS_REG        (* (reg8 *) RFID_BUART_sTX_TxSts__STATUS_REG)
    #define RFID_TXSTATUS_PTR        (  (reg8 *) RFID_BUART_sTX_TxSts__STATUS_REG)
    #define RFID_TXSTATUS_MASK_REG   (* (reg8 *) RFID_BUART_sTX_TxSts__MASK_REG)
    #define RFID_TXSTATUS_MASK_PTR   (  (reg8 *) RFID_BUART_sTX_TxSts__MASK_REG)
    #define RFID_TXSTATUS_ACTL_REG   (* (reg8 *) RFID_BUART_sTX_TxSts__STATUS_AUX_CTL_REG)
    #define RFID_TXSTATUS_ACTL_PTR   (  (reg8 *) RFID_BUART_sTX_TxSts__STATUS_AUX_CTL_REG)

    /* DP clock */
    #if(RFID_TXCLKGEN_DP)
        #define RFID_TXBITCLKGEN_CTR_REG        \
                                        (* (reg8 *) RFID_BUART_sTX_sCLOCK_TxBitClkGen__D0_REG)
        #define RFID_TXBITCLKGEN_CTR_PTR        \
                                        (  (reg8 *) RFID_BUART_sTX_sCLOCK_TxBitClkGen__D0_REG)
        #define RFID_TXBITCLKTX_COMPLETE_REG    \
                                        (* (reg8 *) RFID_BUART_sTX_sCLOCK_TxBitClkGen__D1_REG)
        #define RFID_TXBITCLKTX_COMPLETE_PTR    \
                                        (  (reg8 *) RFID_BUART_sTX_sCLOCK_TxBitClkGen__D1_REG)
    #else     /* Count7 clock*/
        #define RFID_TXBITCTR_PERIOD_REG    \
                                        (* (reg8 *) RFID_BUART_sTX_sCLOCK_TxBitCounter__PERIOD_REG)
        #define RFID_TXBITCTR_PERIOD_PTR    \
                                        (  (reg8 *) RFID_BUART_sTX_sCLOCK_TxBitCounter__PERIOD_REG)
        #define RFID_TXBITCTR_CONTROL_REG   \
                                        (* (reg8 *) RFID_BUART_sTX_sCLOCK_TxBitCounter__CONTROL_AUX_CTL_REG)
        #define RFID_TXBITCTR_CONTROL_PTR   \
                                        (  (reg8 *) RFID_BUART_sTX_sCLOCK_TxBitCounter__CONTROL_AUX_CTL_REG)
        #define RFID_TXBITCTR_COUNTER_REG   \
                                        (* (reg8 *) RFID_BUART_sTX_sCLOCK_TxBitCounter__COUNT_REG)
        #define RFID_TXBITCTR_COUNTER_PTR   \
                                        (  (reg8 *) RFID_BUART_sTX_sCLOCK_TxBitCounter__COUNT_REG)
    #endif /* RFID_TXCLKGEN_DP */

#endif /* End RFID_TX_ENABLED */

#if(RFID_HD_ENABLED)

    #define RFID_TXDATA_REG             (* (reg8 *) RFID_BUART_sRX_RxShifter_u0__F1_REG )
    #define RFID_TXDATA_PTR             (  (reg8 *) RFID_BUART_sRX_RxShifter_u0__F1_REG )
    #define RFID_TXDATA_AUX_CTL_REG     (* (reg8 *) RFID_BUART_sRX_RxShifter_u0__DP_AUX_CTL_REG)
    #define RFID_TXDATA_AUX_CTL_PTR     (  (reg8 *) RFID_BUART_sRX_RxShifter_u0__DP_AUX_CTL_REG)

    #define RFID_TXSTATUS_REG           (* (reg8 *) RFID_BUART_sRX_RxSts__STATUS_REG )
    #define RFID_TXSTATUS_PTR           (  (reg8 *) RFID_BUART_sRX_RxSts__STATUS_REG )
    #define RFID_TXSTATUS_MASK_REG      (* (reg8 *) RFID_BUART_sRX_RxSts__MASK_REG )
    #define RFID_TXSTATUS_MASK_PTR      (  (reg8 *) RFID_BUART_sRX_RxSts__MASK_REG )
    #define RFID_TXSTATUS_ACTL_REG      (* (reg8 *) RFID_BUART_sRX_RxSts__STATUS_AUX_CTL_REG )
    #define RFID_TXSTATUS_ACTL_PTR      (  (reg8 *) RFID_BUART_sRX_RxSts__STATUS_AUX_CTL_REG )
#endif /* End RFID_HD_ENABLED */

#if( (RFID_RX_ENABLED) || (RFID_HD_ENABLED) )
    #define RFID_RXDATA_REG             (* (reg8 *) RFID_BUART_sRX_RxShifter_u0__F0_REG )
    #define RFID_RXDATA_PTR             (  (reg8 *) RFID_BUART_sRX_RxShifter_u0__F0_REG )
    #define RFID_RXADDRESS1_REG         (* (reg8 *) RFID_BUART_sRX_RxShifter_u0__D0_REG )
    #define RFID_RXADDRESS1_PTR         (  (reg8 *) RFID_BUART_sRX_RxShifter_u0__D0_REG )
    #define RFID_RXADDRESS2_REG         (* (reg8 *) RFID_BUART_sRX_RxShifter_u0__D1_REG )
    #define RFID_RXADDRESS2_PTR         (  (reg8 *) RFID_BUART_sRX_RxShifter_u0__D1_REG )
    #define RFID_RXDATA_AUX_CTL_REG     (* (reg8 *) RFID_BUART_sRX_RxShifter_u0__DP_AUX_CTL_REG)

    #define RFID_RXBITCTR_PERIOD_REG    (* (reg8 *) RFID_BUART_sRX_RxBitCounter__PERIOD_REG )
    #define RFID_RXBITCTR_PERIOD_PTR    (  (reg8 *) RFID_BUART_sRX_RxBitCounter__PERIOD_REG )
    #define RFID_RXBITCTR_CONTROL_REG   \
                                        (* (reg8 *) RFID_BUART_sRX_RxBitCounter__CONTROL_AUX_CTL_REG )
    #define RFID_RXBITCTR_CONTROL_PTR   \
                                        (  (reg8 *) RFID_BUART_sRX_RxBitCounter__CONTROL_AUX_CTL_REG )
    #define RFID_RXBITCTR_COUNTER_REG   (* (reg8 *) RFID_BUART_sRX_RxBitCounter__COUNT_REG )
    #define RFID_RXBITCTR_COUNTER_PTR   (  (reg8 *) RFID_BUART_sRX_RxBitCounter__COUNT_REG )

    #define RFID_RXSTATUS_REG           (* (reg8 *) RFID_BUART_sRX_RxSts__STATUS_REG )
    #define RFID_RXSTATUS_PTR           (  (reg8 *) RFID_BUART_sRX_RxSts__STATUS_REG )
    #define RFID_RXSTATUS_MASK_REG      (* (reg8 *) RFID_BUART_sRX_RxSts__MASK_REG )
    #define RFID_RXSTATUS_MASK_PTR      (  (reg8 *) RFID_BUART_sRX_RxSts__MASK_REG )
    #define RFID_RXSTATUS_ACTL_REG      (* (reg8 *) RFID_BUART_sRX_RxSts__STATUS_AUX_CTL_REG )
    #define RFID_RXSTATUS_ACTL_PTR      (  (reg8 *) RFID_BUART_sRX_RxSts__STATUS_AUX_CTL_REG )
#endif /* End  (RFID_RX_ENABLED) || (RFID_HD_ENABLED) */

#if(RFID_INTERNAL_CLOCK_USED)
    /* Register to enable or disable the digital clocks */
    #define RFID_INTCLOCK_CLKEN_REG     (* (reg8 *) RFID_IntClock__PM_ACT_CFG)
    #define RFID_INTCLOCK_CLKEN_PTR     (  (reg8 *) RFID_IntClock__PM_ACT_CFG)

    /* Clock mask for this clock. */
    #define RFID_INTCLOCK_CLKEN_MASK    RFID_IntClock__PM_ACT_MSK
#endif /* End RFID_INTERNAL_CLOCK_USED */


/***************************************
*       Register Constants
***************************************/

#if(RFID_TX_ENABLED)
    #define RFID_TX_FIFO_CLR            (0x01u) /* FIFO0 CLR */
#endif /* End RFID_TX_ENABLED */

#if(RFID_HD_ENABLED)
    #define RFID_TX_FIFO_CLR            (0x02u) /* FIFO1 CLR */
#endif /* End RFID_HD_ENABLED */

#if( (RFID_RX_ENABLED) || (RFID_HD_ENABLED) )
    #define RFID_RX_FIFO_CLR            (0x01u) /* FIFO0 CLR */
#endif /* End  (RFID_RX_ENABLED) || (RFID_HD_ENABLED) */


/***************************************
* The following code is DEPRECATED and
* should not be used in new projects.
***************************************/

/* UART v2_40 obsolete definitions */
#define RFID_WAIT_1_MS      RFID_BL_CHK_DELAY_MS   

#define RFID_TXBUFFERSIZE   RFID_TX_BUFFER_SIZE
#define RFID_RXBUFFERSIZE   RFID_RX_BUFFER_SIZE

#if (RFID_RXHW_ADDRESS_ENABLED)
    #define RFID_RXADDRESSMODE  RFID_RX_ADDRESS_MODE
    #define RFID_RXHWADDRESS1   RFID_RX_HW_ADDRESS1
    #define RFID_RXHWADDRESS2   RFID_RX_HW_ADDRESS2
    /* Backward compatible define */
    #define RFID_RXAddressMode  RFID_RXADDRESSMODE
#endif /* (RFID_RXHW_ADDRESS_ENABLED) */

/* UART v2_30 obsolete definitions */
#define RFID_initvar                    RFID_initVar

#define RFID_RX_Enabled                 RFID_RX_ENABLED
#define RFID_TX_Enabled                 RFID_TX_ENABLED
#define RFID_HD_Enabled                 RFID_HD_ENABLED
#define RFID_RX_IntInterruptEnabled     RFID_RX_INTERRUPT_ENABLED
#define RFID_TX_IntInterruptEnabled     RFID_TX_INTERRUPT_ENABLED
#define RFID_InternalClockUsed          RFID_INTERNAL_CLOCK_USED
#define RFID_RXHW_Address_Enabled       RFID_RXHW_ADDRESS_ENABLED
#define RFID_OverSampleCount            RFID_OVER_SAMPLE_COUNT
#define RFID_ParityType                 RFID_PARITY_TYPE

#if( RFID_TX_ENABLED && (RFID_TXBUFFERSIZE > RFID_FIFO_LENGTH))
    #define RFID_TXBUFFER               RFID_txBuffer
    #define RFID_TXBUFFERREAD           RFID_txBufferRead
    #define RFID_TXBUFFERWRITE          RFID_txBufferWrite
#endif /* End RFID_TX_ENABLED */
#if( ( RFID_RX_ENABLED || RFID_HD_ENABLED ) && \
     (RFID_RXBUFFERSIZE > RFID_FIFO_LENGTH) )
    #define RFID_RXBUFFER               RFID_rxBuffer
    #define RFID_RXBUFFERREAD           RFID_rxBufferRead
    #define RFID_RXBUFFERWRITE          RFID_rxBufferWrite
    #define RFID_RXBUFFERLOOPDETECT     RFID_rxBufferLoopDetect
    #define RFID_RXBUFFER_OVERFLOW      RFID_rxBufferOverflow
#endif /* End RFID_RX_ENABLED */

#ifdef RFID_BUART_sCR_SyncCtl_CtrlReg__CONTROL_REG
    #define RFID_CONTROL                RFID_CONTROL_REG
#endif /* End RFID_BUART_sCR_SyncCtl_CtrlReg__CONTROL_REG */

#if(RFID_TX_ENABLED)
    #define RFID_TXDATA                 RFID_TXDATA_REG
    #define RFID_TXSTATUS               RFID_TXSTATUS_REG
    #define RFID_TXSTATUS_MASK          RFID_TXSTATUS_MASK_REG
    #define RFID_TXSTATUS_ACTL          RFID_TXSTATUS_ACTL_REG
    /* DP clock */
    #if(RFID_TXCLKGEN_DP)
        #define RFID_TXBITCLKGEN_CTR        RFID_TXBITCLKGEN_CTR_REG
        #define RFID_TXBITCLKTX_COMPLETE    RFID_TXBITCLKTX_COMPLETE_REG
    #else     /* Count7 clock*/
        #define RFID_TXBITCTR_PERIOD        RFID_TXBITCTR_PERIOD_REG
        #define RFID_TXBITCTR_CONTROL       RFID_TXBITCTR_CONTROL_REG
        #define RFID_TXBITCTR_COUNTER       RFID_TXBITCTR_COUNTER_REG
    #endif /* RFID_TXCLKGEN_DP */
#endif /* End RFID_TX_ENABLED */

#if(RFID_HD_ENABLED)
    #define RFID_TXDATA                 RFID_TXDATA_REG
    #define RFID_TXSTATUS               RFID_TXSTATUS_REG
    #define RFID_TXSTATUS_MASK          RFID_TXSTATUS_MASK_REG
    #define RFID_TXSTATUS_ACTL          RFID_TXSTATUS_ACTL_REG
#endif /* End RFID_HD_ENABLED */

#if( (RFID_RX_ENABLED) || (RFID_HD_ENABLED) )
    #define RFID_RXDATA                 RFID_RXDATA_REG
    #define RFID_RXADDRESS1             RFID_RXADDRESS1_REG
    #define RFID_RXADDRESS2             RFID_RXADDRESS2_REG
    #define RFID_RXBITCTR_PERIOD        RFID_RXBITCTR_PERIOD_REG
    #define RFID_RXBITCTR_CONTROL       RFID_RXBITCTR_CONTROL_REG
    #define RFID_RXBITCTR_COUNTER       RFID_RXBITCTR_COUNTER_REG
    #define RFID_RXSTATUS               RFID_RXSTATUS_REG
    #define RFID_RXSTATUS_MASK          RFID_RXSTATUS_MASK_REG
    #define RFID_RXSTATUS_ACTL          RFID_RXSTATUS_ACTL_REG
#endif /* End  (RFID_RX_ENABLED) || (RFID_HD_ENABLED) */

#if(RFID_INTERNAL_CLOCK_USED)
    #define RFID_INTCLOCK_CLKEN         RFID_INTCLOCK_CLKEN_REG
#endif /* End RFID_INTERNAL_CLOCK_USED */

#define RFID_WAIT_FOR_COMLETE_REINIT    RFID_WAIT_FOR_COMPLETE_REINIT

#endif  /* CY_UART_RFID_H */


/* [] END OF FILE */
