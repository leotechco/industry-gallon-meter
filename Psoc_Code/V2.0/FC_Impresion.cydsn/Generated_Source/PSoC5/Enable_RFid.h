/*******************************************************************************
* File Name: Enable_RFid.h  
* Version 2.20
*
* Description:
*  This file contains Pin function prototypes and register defines
*
* Note:
*
********************************************************************************
* Copyright 2008-2015, Cypress Semiconductor Corporation.  All rights reserved.
* You may use this file only in accordance with the license, terms, conditions, 
* disclaimers, and limitations in the end user license agreement accompanying 
* the software package with which this file was provided.
*******************************************************************************/

#if !defined(CY_PINS_Enable_RFid_H) /* Pins Enable_RFid_H */
#define CY_PINS_Enable_RFid_H

#include "cytypes.h"
#include "cyfitter.h"
#include "cypins.h"
#include "Enable_RFid_aliases.h"

/* APIs are not generated for P15[7:6] */
#if !(CY_PSOC5A &&\
	 Enable_RFid__PORT == 15 && ((Enable_RFid__MASK & 0xC0) != 0))


/***************************************
*        Function Prototypes             
***************************************/    

/**
* \addtogroup group_general
* @{
*/
void    Enable_RFid_Write(uint8 value);
void    Enable_RFid_SetDriveMode(uint8 mode);
uint8   Enable_RFid_ReadDataReg(void);
uint8   Enable_RFid_Read(void);
void    Enable_RFid_SetInterruptMode(uint16 position, uint16 mode);
uint8   Enable_RFid_ClearInterrupt(void);
/** @} general */

/***************************************
*           API Constants        
***************************************/
/**
* \addtogroup group_constants
* @{
*/
    /** \addtogroup driveMode Drive mode constants
     * \brief Constants to be passed as "mode" parameter in the Enable_RFid_SetDriveMode() function.
     *  @{
     */
        #define Enable_RFid_DM_ALG_HIZ         PIN_DM_ALG_HIZ
        #define Enable_RFid_DM_DIG_HIZ         PIN_DM_DIG_HIZ
        #define Enable_RFid_DM_RES_UP          PIN_DM_RES_UP
        #define Enable_RFid_DM_RES_DWN         PIN_DM_RES_DWN
        #define Enable_RFid_DM_OD_LO           PIN_DM_OD_LO
        #define Enable_RFid_DM_OD_HI           PIN_DM_OD_HI
        #define Enable_RFid_DM_STRONG          PIN_DM_STRONG
        #define Enable_RFid_DM_RES_UPDWN       PIN_DM_RES_UPDWN
    /** @} driveMode */
/** @} group_constants */
    
/* Digital Port Constants */
#define Enable_RFid_MASK               Enable_RFid__MASK
#define Enable_RFid_SHIFT              Enable_RFid__SHIFT
#define Enable_RFid_WIDTH              1u

/* Interrupt constants */
#if defined(Enable_RFid__INTSTAT)
/**
* \addtogroup group_constants
* @{
*/
    /** \addtogroup intrMode Interrupt constants
     * \brief Constants to be passed as "mode" parameter in Enable_RFid_SetInterruptMode() function.
     *  @{
     */
        #define Enable_RFid_INTR_NONE      (uint16)(0x0000u)
        #define Enable_RFid_INTR_RISING    (uint16)(0x0001u)
        #define Enable_RFid_INTR_FALLING   (uint16)(0x0002u)
        #define Enable_RFid_INTR_BOTH      (uint16)(0x0003u) 
    /** @} intrMode */
/** @} group_constants */

    #define Enable_RFid_INTR_MASK      (0x01u) 
#endif /* (Enable_RFid__INTSTAT) */


/***************************************
*             Registers        
***************************************/

/* Main Port Registers */
/* Pin State */
#define Enable_RFid_PS                     (* (reg8 *) Enable_RFid__PS)
/* Data Register */
#define Enable_RFid_DR                     (* (reg8 *) Enable_RFid__DR)
/* Port Number */
#define Enable_RFid_PRT_NUM                (* (reg8 *) Enable_RFid__PRT) 
/* Connect to Analog Globals */                                                  
#define Enable_RFid_AG                     (* (reg8 *) Enable_RFid__AG)                       
/* Analog MUX bux enable */
#define Enable_RFid_AMUX                   (* (reg8 *) Enable_RFid__AMUX) 
/* Bidirectional Enable */                                                        
#define Enable_RFid_BIE                    (* (reg8 *) Enable_RFid__BIE)
/* Bit-mask for Aliased Register Access */
#define Enable_RFid_BIT_MASK               (* (reg8 *) Enable_RFid__BIT_MASK)
/* Bypass Enable */
#define Enable_RFid_BYP                    (* (reg8 *) Enable_RFid__BYP)
/* Port wide control signals */                                                   
#define Enable_RFid_CTL                    (* (reg8 *) Enable_RFid__CTL)
/* Drive Modes */
#define Enable_RFid_DM0                    (* (reg8 *) Enable_RFid__DM0) 
#define Enable_RFid_DM1                    (* (reg8 *) Enable_RFid__DM1)
#define Enable_RFid_DM2                    (* (reg8 *) Enable_RFid__DM2) 
/* Input Buffer Disable Override */
#define Enable_RFid_INP_DIS                (* (reg8 *) Enable_RFid__INP_DIS)
/* LCD Common or Segment Drive */
#define Enable_RFid_LCD_COM_SEG            (* (reg8 *) Enable_RFid__LCD_COM_SEG)
/* Enable Segment LCD */
#define Enable_RFid_LCD_EN                 (* (reg8 *) Enable_RFid__LCD_EN)
/* Slew Rate Control */
#define Enable_RFid_SLW                    (* (reg8 *) Enable_RFid__SLW)

/* DSI Port Registers */
/* Global DSI Select Register */
#define Enable_RFid_PRTDSI__CAPS_SEL       (* (reg8 *) Enable_RFid__PRTDSI__CAPS_SEL) 
/* Double Sync Enable */
#define Enable_RFid_PRTDSI__DBL_SYNC_IN    (* (reg8 *) Enable_RFid__PRTDSI__DBL_SYNC_IN) 
/* Output Enable Select Drive Strength */
#define Enable_RFid_PRTDSI__OE_SEL0        (* (reg8 *) Enable_RFid__PRTDSI__OE_SEL0) 
#define Enable_RFid_PRTDSI__OE_SEL1        (* (reg8 *) Enable_RFid__PRTDSI__OE_SEL1) 
/* Port Pin Output Select Registers */
#define Enable_RFid_PRTDSI__OUT_SEL0       (* (reg8 *) Enable_RFid__PRTDSI__OUT_SEL0) 
#define Enable_RFid_PRTDSI__OUT_SEL1       (* (reg8 *) Enable_RFid__PRTDSI__OUT_SEL1) 
/* Sync Output Enable Registers */
#define Enable_RFid_PRTDSI__SYNC_OUT       (* (reg8 *) Enable_RFid__PRTDSI__SYNC_OUT) 

/* SIO registers */
#if defined(Enable_RFid__SIO_CFG)
    #define Enable_RFid_SIO_HYST_EN        (* (reg8 *) Enable_RFid__SIO_HYST_EN)
    #define Enable_RFid_SIO_REG_HIFREQ     (* (reg8 *) Enable_RFid__SIO_REG_HIFREQ)
    #define Enable_RFid_SIO_CFG            (* (reg8 *) Enable_RFid__SIO_CFG)
    #define Enable_RFid_SIO_DIFF           (* (reg8 *) Enable_RFid__SIO_DIFF)
#endif /* (Enable_RFid__SIO_CFG) */

/* Interrupt Registers */
#if defined(Enable_RFid__INTSTAT)
    #define Enable_RFid_INTSTAT            (* (reg8 *) Enable_RFid__INTSTAT)
    #define Enable_RFid_SNAP               (* (reg8 *) Enable_RFid__SNAP)
    
	#define Enable_RFid_0_INTTYPE_REG 		(* (reg8 *) Enable_RFid__0__INTTYPE)
#endif /* (Enable_RFid__INTSTAT) */

#endif /* CY_PSOC5A... */

#endif /*  CY_PINS_Enable_RFid_H */


/* [] END OF FILE */
