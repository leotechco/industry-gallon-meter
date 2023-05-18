/*******************************************************************************
* File Name: Botones.h  
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

#if !defined(CY_PINS_Botones_H) /* Pins Botones_H */
#define CY_PINS_Botones_H

#include "cytypes.h"
#include "cyfitter.h"
#include "cypins.h"
#include "Botones_aliases.h"

/* APIs are not generated for P15[7:6] */
#if !(CY_PSOC5A &&\
	 Botones__PORT == 15 && ((Botones__MASK & 0xC0) != 0))


/***************************************
*        Function Prototypes             
***************************************/    

/**
* \addtogroup group_general
* @{
*/
void    Botones_Write(uint8 value);
void    Botones_SetDriveMode(uint8 mode);
uint8   Botones_ReadDataReg(void);
uint8   Botones_Read(void);
void    Botones_SetInterruptMode(uint16 position, uint16 mode);
uint8   Botones_ClearInterrupt(void);
/** @} general */

/***************************************
*           API Constants        
***************************************/
/**
* \addtogroup group_constants
* @{
*/
    /** \addtogroup driveMode Drive mode constants
     * \brief Constants to be passed as "mode" parameter in the Botones_SetDriveMode() function.
     *  @{
     */
        #define Botones_DM_ALG_HIZ         PIN_DM_ALG_HIZ
        #define Botones_DM_DIG_HIZ         PIN_DM_DIG_HIZ
        #define Botones_DM_RES_UP          PIN_DM_RES_UP
        #define Botones_DM_RES_DWN         PIN_DM_RES_DWN
        #define Botones_DM_OD_LO           PIN_DM_OD_LO
        #define Botones_DM_OD_HI           PIN_DM_OD_HI
        #define Botones_DM_STRONG          PIN_DM_STRONG
        #define Botones_DM_RES_UPDWN       PIN_DM_RES_UPDWN
    /** @} driveMode */
/** @} group_constants */
    
/* Digital Port Constants */
#define Botones_MASK               Botones__MASK
#define Botones_SHIFT              Botones__SHIFT
#define Botones_WIDTH              2u

/* Interrupt constants */
#if defined(Botones__INTSTAT)
/**
* \addtogroup group_constants
* @{
*/
    /** \addtogroup intrMode Interrupt constants
     * \brief Constants to be passed as "mode" parameter in Botones_SetInterruptMode() function.
     *  @{
     */
        #define Botones_INTR_NONE      (uint16)(0x0000u)
        #define Botones_INTR_RISING    (uint16)(0x0001u)
        #define Botones_INTR_FALLING   (uint16)(0x0002u)
        #define Botones_INTR_BOTH      (uint16)(0x0003u) 
    /** @} intrMode */
/** @} group_constants */

    #define Botones_INTR_MASK      (0x01u) 
#endif /* (Botones__INTSTAT) */


/***************************************
*             Registers        
***************************************/

/* Main Port Registers */
/* Pin State */
#define Botones_PS                     (* (reg8 *) Botones__PS)
/* Data Register */
#define Botones_DR                     (* (reg8 *) Botones__DR)
/* Port Number */
#define Botones_PRT_NUM                (* (reg8 *) Botones__PRT) 
/* Connect to Analog Globals */                                                  
#define Botones_AG                     (* (reg8 *) Botones__AG)                       
/* Analog MUX bux enable */
#define Botones_AMUX                   (* (reg8 *) Botones__AMUX) 
/* Bidirectional Enable */                                                        
#define Botones_BIE                    (* (reg8 *) Botones__BIE)
/* Bit-mask for Aliased Register Access */
#define Botones_BIT_MASK               (* (reg8 *) Botones__BIT_MASK)
/* Bypass Enable */
#define Botones_BYP                    (* (reg8 *) Botones__BYP)
/* Port wide control signals */                                                   
#define Botones_CTL                    (* (reg8 *) Botones__CTL)
/* Drive Modes */
#define Botones_DM0                    (* (reg8 *) Botones__DM0) 
#define Botones_DM1                    (* (reg8 *) Botones__DM1)
#define Botones_DM2                    (* (reg8 *) Botones__DM2) 
/* Input Buffer Disable Override */
#define Botones_INP_DIS                (* (reg8 *) Botones__INP_DIS)
/* LCD Common or Segment Drive */
#define Botones_LCD_COM_SEG            (* (reg8 *) Botones__LCD_COM_SEG)
/* Enable Segment LCD */
#define Botones_LCD_EN                 (* (reg8 *) Botones__LCD_EN)
/* Slew Rate Control */
#define Botones_SLW                    (* (reg8 *) Botones__SLW)

/* DSI Port Registers */
/* Global DSI Select Register */
#define Botones_PRTDSI__CAPS_SEL       (* (reg8 *) Botones__PRTDSI__CAPS_SEL) 
/* Double Sync Enable */
#define Botones_PRTDSI__DBL_SYNC_IN    (* (reg8 *) Botones__PRTDSI__DBL_SYNC_IN) 
/* Output Enable Select Drive Strength */
#define Botones_PRTDSI__OE_SEL0        (* (reg8 *) Botones__PRTDSI__OE_SEL0) 
#define Botones_PRTDSI__OE_SEL1        (* (reg8 *) Botones__PRTDSI__OE_SEL1) 
/* Port Pin Output Select Registers */
#define Botones_PRTDSI__OUT_SEL0       (* (reg8 *) Botones__PRTDSI__OUT_SEL0) 
#define Botones_PRTDSI__OUT_SEL1       (* (reg8 *) Botones__PRTDSI__OUT_SEL1) 
/* Sync Output Enable Registers */
#define Botones_PRTDSI__SYNC_OUT       (* (reg8 *) Botones__PRTDSI__SYNC_OUT) 

/* SIO registers */
#if defined(Botones__SIO_CFG)
    #define Botones_SIO_HYST_EN        (* (reg8 *) Botones__SIO_HYST_EN)
    #define Botones_SIO_REG_HIFREQ     (* (reg8 *) Botones__SIO_REG_HIFREQ)
    #define Botones_SIO_CFG            (* (reg8 *) Botones__SIO_CFG)
    #define Botones_SIO_DIFF           (* (reg8 *) Botones__SIO_DIFF)
#endif /* (Botones__SIO_CFG) */

/* Interrupt Registers */
#if defined(Botones__INTSTAT)
    #define Botones_INTSTAT            (* (reg8 *) Botones__INTSTAT)
    #define Botones_SNAP               (* (reg8 *) Botones__SNAP)
    
	#define Botones_0_INTTYPE_REG 		(* (reg8 *) Botones__0__INTTYPE)
	#define Botones_1_INTTYPE_REG 		(* (reg8 *) Botones__1__INTTYPE)
#endif /* (Botones__INTSTAT) */

#endif /* CY_PSOC5A... */

#endif /*  CY_PINS_Botones_H */


/* [] END OF FILE */
