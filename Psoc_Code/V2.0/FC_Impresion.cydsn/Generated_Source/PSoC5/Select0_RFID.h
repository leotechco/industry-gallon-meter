/*******************************************************************************
* File Name: Select0_RFID.h  
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

#if !defined(CY_PINS_Select0_RFID_H) /* Pins Select0_RFID_H */
#define CY_PINS_Select0_RFID_H

#include "cytypes.h"
#include "cyfitter.h"
#include "cypins.h"
#include "Select0_RFID_aliases.h"

/* APIs are not generated for P15[7:6] */
#if !(CY_PSOC5A &&\
	 Select0_RFID__PORT == 15 && ((Select0_RFID__MASK & 0xC0) != 0))


/***************************************
*        Function Prototypes             
***************************************/    

/**
* \addtogroup group_general
* @{
*/
void    Select0_RFID_Write(uint8 value);
void    Select0_RFID_SetDriveMode(uint8 mode);
uint8   Select0_RFID_ReadDataReg(void);
uint8   Select0_RFID_Read(void);
void    Select0_RFID_SetInterruptMode(uint16 position, uint16 mode);
uint8   Select0_RFID_ClearInterrupt(void);
/** @} general */

/***************************************
*           API Constants        
***************************************/
/**
* \addtogroup group_constants
* @{
*/
    /** \addtogroup driveMode Drive mode constants
     * \brief Constants to be passed as "mode" parameter in the Select0_RFID_SetDriveMode() function.
     *  @{
     */
        #define Select0_RFID_DM_ALG_HIZ         PIN_DM_ALG_HIZ
        #define Select0_RFID_DM_DIG_HIZ         PIN_DM_DIG_HIZ
        #define Select0_RFID_DM_RES_UP          PIN_DM_RES_UP
        #define Select0_RFID_DM_RES_DWN         PIN_DM_RES_DWN
        #define Select0_RFID_DM_OD_LO           PIN_DM_OD_LO
        #define Select0_RFID_DM_OD_HI           PIN_DM_OD_HI
        #define Select0_RFID_DM_STRONG          PIN_DM_STRONG
        #define Select0_RFID_DM_RES_UPDWN       PIN_DM_RES_UPDWN
    /** @} driveMode */
/** @} group_constants */
    
/* Digital Port Constants */
#define Select0_RFID_MASK               Select0_RFID__MASK
#define Select0_RFID_SHIFT              Select0_RFID__SHIFT
#define Select0_RFID_WIDTH              1u

/* Interrupt constants */
#if defined(Select0_RFID__INTSTAT)
/**
* \addtogroup group_constants
* @{
*/
    /** \addtogroup intrMode Interrupt constants
     * \brief Constants to be passed as "mode" parameter in Select0_RFID_SetInterruptMode() function.
     *  @{
     */
        #define Select0_RFID_INTR_NONE      (uint16)(0x0000u)
        #define Select0_RFID_INTR_RISING    (uint16)(0x0001u)
        #define Select0_RFID_INTR_FALLING   (uint16)(0x0002u)
        #define Select0_RFID_INTR_BOTH      (uint16)(0x0003u) 
    /** @} intrMode */
/** @} group_constants */

    #define Select0_RFID_INTR_MASK      (0x01u) 
#endif /* (Select0_RFID__INTSTAT) */


/***************************************
*             Registers        
***************************************/

/* Main Port Registers */
/* Pin State */
#define Select0_RFID_PS                     (* (reg8 *) Select0_RFID__PS)
/* Data Register */
#define Select0_RFID_DR                     (* (reg8 *) Select0_RFID__DR)
/* Port Number */
#define Select0_RFID_PRT_NUM                (* (reg8 *) Select0_RFID__PRT) 
/* Connect to Analog Globals */                                                  
#define Select0_RFID_AG                     (* (reg8 *) Select0_RFID__AG)                       
/* Analog MUX bux enable */
#define Select0_RFID_AMUX                   (* (reg8 *) Select0_RFID__AMUX) 
/* Bidirectional Enable */                                                        
#define Select0_RFID_BIE                    (* (reg8 *) Select0_RFID__BIE)
/* Bit-mask for Aliased Register Access */
#define Select0_RFID_BIT_MASK               (* (reg8 *) Select0_RFID__BIT_MASK)
/* Bypass Enable */
#define Select0_RFID_BYP                    (* (reg8 *) Select0_RFID__BYP)
/* Port wide control signals */                                                   
#define Select0_RFID_CTL                    (* (reg8 *) Select0_RFID__CTL)
/* Drive Modes */
#define Select0_RFID_DM0                    (* (reg8 *) Select0_RFID__DM0) 
#define Select0_RFID_DM1                    (* (reg8 *) Select0_RFID__DM1)
#define Select0_RFID_DM2                    (* (reg8 *) Select0_RFID__DM2) 
/* Input Buffer Disable Override */
#define Select0_RFID_INP_DIS                (* (reg8 *) Select0_RFID__INP_DIS)
/* LCD Common or Segment Drive */
#define Select0_RFID_LCD_COM_SEG            (* (reg8 *) Select0_RFID__LCD_COM_SEG)
/* Enable Segment LCD */
#define Select0_RFID_LCD_EN                 (* (reg8 *) Select0_RFID__LCD_EN)
/* Slew Rate Control */
#define Select0_RFID_SLW                    (* (reg8 *) Select0_RFID__SLW)

/* DSI Port Registers */
/* Global DSI Select Register */
#define Select0_RFID_PRTDSI__CAPS_SEL       (* (reg8 *) Select0_RFID__PRTDSI__CAPS_SEL) 
/* Double Sync Enable */
#define Select0_RFID_PRTDSI__DBL_SYNC_IN    (* (reg8 *) Select0_RFID__PRTDSI__DBL_SYNC_IN) 
/* Output Enable Select Drive Strength */
#define Select0_RFID_PRTDSI__OE_SEL0        (* (reg8 *) Select0_RFID__PRTDSI__OE_SEL0) 
#define Select0_RFID_PRTDSI__OE_SEL1        (* (reg8 *) Select0_RFID__PRTDSI__OE_SEL1) 
/* Port Pin Output Select Registers */
#define Select0_RFID_PRTDSI__OUT_SEL0       (* (reg8 *) Select0_RFID__PRTDSI__OUT_SEL0) 
#define Select0_RFID_PRTDSI__OUT_SEL1       (* (reg8 *) Select0_RFID__PRTDSI__OUT_SEL1) 
/* Sync Output Enable Registers */
#define Select0_RFID_PRTDSI__SYNC_OUT       (* (reg8 *) Select0_RFID__PRTDSI__SYNC_OUT) 

/* SIO registers */
#if defined(Select0_RFID__SIO_CFG)
    #define Select0_RFID_SIO_HYST_EN        (* (reg8 *) Select0_RFID__SIO_HYST_EN)
    #define Select0_RFID_SIO_REG_HIFREQ     (* (reg8 *) Select0_RFID__SIO_REG_HIFREQ)
    #define Select0_RFID_SIO_CFG            (* (reg8 *) Select0_RFID__SIO_CFG)
    #define Select0_RFID_SIO_DIFF           (* (reg8 *) Select0_RFID__SIO_DIFF)
#endif /* (Select0_RFID__SIO_CFG) */

/* Interrupt Registers */
#if defined(Select0_RFID__INTSTAT)
    #define Select0_RFID_INTSTAT            (* (reg8 *) Select0_RFID__INTSTAT)
    #define Select0_RFID_SNAP               (* (reg8 *) Select0_RFID__SNAP)
    
	#define Select0_RFID_0_INTTYPE_REG 		(* (reg8 *) Select0_RFID__0__INTTYPE)
#endif /* (Select0_RFID__INTSTAT) */

#endif /* CY_PSOC5A... */

#endif /*  CY_PINS_Select0_RFID_H */


/* [] END OF FILE */
