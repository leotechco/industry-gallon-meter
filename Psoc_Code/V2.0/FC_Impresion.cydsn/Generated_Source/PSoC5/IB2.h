/*******************************************************************************
* File Name: IB2.h  
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

#if !defined(CY_PINS_IB2_H) /* Pins IB2_H */
#define CY_PINS_IB2_H

#include "cytypes.h"
#include "cyfitter.h"
#include "cypins.h"
#include "IB2_aliases.h"

/* APIs are not generated for P15[7:6] */
#if !(CY_PSOC5A &&\
	 IB2__PORT == 15 && ((IB2__MASK & 0xC0) != 0))


/***************************************
*        Function Prototypes             
***************************************/    

/**
* \addtogroup group_general
* @{
*/
void    IB2_Write(uint8 value);
void    IB2_SetDriveMode(uint8 mode);
uint8   IB2_ReadDataReg(void);
uint8   IB2_Read(void);
void    IB2_SetInterruptMode(uint16 position, uint16 mode);
uint8   IB2_ClearInterrupt(void);
/** @} general */

/***************************************
*           API Constants        
***************************************/
/**
* \addtogroup group_constants
* @{
*/
    /** \addtogroup driveMode Drive mode constants
     * \brief Constants to be passed as "mode" parameter in the IB2_SetDriveMode() function.
     *  @{
     */
        #define IB2_DM_ALG_HIZ         PIN_DM_ALG_HIZ
        #define IB2_DM_DIG_HIZ         PIN_DM_DIG_HIZ
        #define IB2_DM_RES_UP          PIN_DM_RES_UP
        #define IB2_DM_RES_DWN         PIN_DM_RES_DWN
        #define IB2_DM_OD_LO           PIN_DM_OD_LO
        #define IB2_DM_OD_HI           PIN_DM_OD_HI
        #define IB2_DM_STRONG          PIN_DM_STRONG
        #define IB2_DM_RES_UPDWN       PIN_DM_RES_UPDWN
    /** @} driveMode */
/** @} group_constants */
    
/* Digital Port Constants */
#define IB2_MASK               IB2__MASK
#define IB2_SHIFT              IB2__SHIFT
#define IB2_WIDTH              1u

/* Interrupt constants */
#if defined(IB2__INTSTAT)
/**
* \addtogroup group_constants
* @{
*/
    /** \addtogroup intrMode Interrupt constants
     * \brief Constants to be passed as "mode" parameter in IB2_SetInterruptMode() function.
     *  @{
     */
        #define IB2_INTR_NONE      (uint16)(0x0000u)
        #define IB2_INTR_RISING    (uint16)(0x0001u)
        #define IB2_INTR_FALLING   (uint16)(0x0002u)
        #define IB2_INTR_BOTH      (uint16)(0x0003u) 
    /** @} intrMode */
/** @} group_constants */

    #define IB2_INTR_MASK      (0x01u) 
#endif /* (IB2__INTSTAT) */


/***************************************
*             Registers        
***************************************/

/* Main Port Registers */
/* Pin State */
#define IB2_PS                     (* (reg8 *) IB2__PS)
/* Data Register */
#define IB2_DR                     (* (reg8 *) IB2__DR)
/* Port Number */
#define IB2_PRT_NUM                (* (reg8 *) IB2__PRT) 
/* Connect to Analog Globals */                                                  
#define IB2_AG                     (* (reg8 *) IB2__AG)                       
/* Analog MUX bux enable */
#define IB2_AMUX                   (* (reg8 *) IB2__AMUX) 
/* Bidirectional Enable */                                                        
#define IB2_BIE                    (* (reg8 *) IB2__BIE)
/* Bit-mask for Aliased Register Access */
#define IB2_BIT_MASK               (* (reg8 *) IB2__BIT_MASK)
/* Bypass Enable */
#define IB2_BYP                    (* (reg8 *) IB2__BYP)
/* Port wide control signals */                                                   
#define IB2_CTL                    (* (reg8 *) IB2__CTL)
/* Drive Modes */
#define IB2_DM0                    (* (reg8 *) IB2__DM0) 
#define IB2_DM1                    (* (reg8 *) IB2__DM1)
#define IB2_DM2                    (* (reg8 *) IB2__DM2) 
/* Input Buffer Disable Override */
#define IB2_INP_DIS                (* (reg8 *) IB2__INP_DIS)
/* LCD Common or Segment Drive */
#define IB2_LCD_COM_SEG            (* (reg8 *) IB2__LCD_COM_SEG)
/* Enable Segment LCD */
#define IB2_LCD_EN                 (* (reg8 *) IB2__LCD_EN)
/* Slew Rate Control */
#define IB2_SLW                    (* (reg8 *) IB2__SLW)

/* DSI Port Registers */
/* Global DSI Select Register */
#define IB2_PRTDSI__CAPS_SEL       (* (reg8 *) IB2__PRTDSI__CAPS_SEL) 
/* Double Sync Enable */
#define IB2_PRTDSI__DBL_SYNC_IN    (* (reg8 *) IB2__PRTDSI__DBL_SYNC_IN) 
/* Output Enable Select Drive Strength */
#define IB2_PRTDSI__OE_SEL0        (* (reg8 *) IB2__PRTDSI__OE_SEL0) 
#define IB2_PRTDSI__OE_SEL1        (* (reg8 *) IB2__PRTDSI__OE_SEL1) 
/* Port Pin Output Select Registers */
#define IB2_PRTDSI__OUT_SEL0       (* (reg8 *) IB2__PRTDSI__OUT_SEL0) 
#define IB2_PRTDSI__OUT_SEL1       (* (reg8 *) IB2__PRTDSI__OUT_SEL1) 
/* Sync Output Enable Registers */
#define IB2_PRTDSI__SYNC_OUT       (* (reg8 *) IB2__PRTDSI__SYNC_OUT) 

/* SIO registers */
#if defined(IB2__SIO_CFG)
    #define IB2_SIO_HYST_EN        (* (reg8 *) IB2__SIO_HYST_EN)
    #define IB2_SIO_REG_HIFREQ     (* (reg8 *) IB2__SIO_REG_HIFREQ)
    #define IB2_SIO_CFG            (* (reg8 *) IB2__SIO_CFG)
    #define IB2_SIO_DIFF           (* (reg8 *) IB2__SIO_DIFF)
#endif /* (IB2__SIO_CFG) */

/* Interrupt Registers */
#if defined(IB2__INTSTAT)
    #define IB2_INTSTAT            (* (reg8 *) IB2__INTSTAT)
    #define IB2_SNAP               (* (reg8 *) IB2__SNAP)
    
	#define IB2_0_INTTYPE_REG 		(* (reg8 *) IB2__0__INTTYPE)
#endif /* (IB2__INTSTAT) */

#endif /* CY_PSOC5A... */

#endif /*  CY_PINS_IB2_H */


/* [] END OF FILE */
