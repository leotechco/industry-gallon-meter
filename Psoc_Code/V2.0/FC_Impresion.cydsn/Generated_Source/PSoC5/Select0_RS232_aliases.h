/*******************************************************************************
* File Name: Select0_RS232.h  
* Version 2.20
*
* Description:
*  This file contains the Alias definitions for Per-Pin APIs in cypins.h. 
*  Information on using these APIs can be found in the System Reference Guide.
*
* Note:
*
********************************************************************************
* Copyright 2008-2015, Cypress Semiconductor Corporation.  All rights reserved.
* You may use this file only in accordance with the license, terms, conditions, 
* disclaimers, and limitations in the end user license agreement accompanying 
* the software package with which this file was provided.
*******************************************************************************/

#if !defined(CY_PINS_Select0_RS232_ALIASES_H) /* Pins Select0_RS232_ALIASES_H */
#define CY_PINS_Select0_RS232_ALIASES_H

#include "cytypes.h"
#include "cyfitter.h"


/***************************************
*              Constants        
***************************************/
#define Select0_RS232_0			(Select0_RS232__0__PC)
#define Select0_RS232_0_INTR	((uint16)((uint16)0x0001u << Select0_RS232__0__SHIFT))

#define Select0_RS232_INTR_ALL	 ((uint16)(Select0_RS232_0_INTR))

#endif /* End Pins Select0_RS232_ALIASES_H */


/* [] END OF FILE */
