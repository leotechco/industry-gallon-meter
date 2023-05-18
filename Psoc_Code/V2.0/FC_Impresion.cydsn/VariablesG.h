/*
*********************************************************************************************************
*                                         FC_Impresion CODE
*
*                             (c) Copyright 2016; Distracom S.A
*
*               All rights reserved.  Protected by international copyright laws.
*               Knowledge of the source code may NOT be used to develop a similar product.
*               Please help us continue to provide the Embedded community with the finest
*               software available.  Your honesty is greatly appreciated.
*********************************************************************************************************
*/

/*
*********************************************************************************************************
*
*                                            FC_Impresion CODE
*
*                                             CYPRESS PSoC5LP
*                                                with the
*                                            CY8C5888LTI-LP097
*
* Filename      : VariablesG.c
* Version       : V1.00
* Programmer(s) : Leonardo Orjuela   
                  
*********************************************************************************************************
*/

/*
*********************************************************************************************************
*                                             INCLUDE FILES
*********************************************************************************************************
*/

#ifndef VARIABLESG_H
#define VARIABLESG_H
#include <project.h>	   

/****************************************************************************************************************
						Variables para manejo de datos del surtidor
****************************************************************************************************************/	
volatile uint8 	ppux10;
volatile uint8 	nombre_pro1[17],nombre_pro2[17],nombre_pro3[17];	//Nombre de cada producto
volatile uint8 	version;											//7 - 6

struct surtidor{
    volatile uint8 dir;
    volatile uint8 totales[3][24];   	//los 12 primeros son volumen y los segundos pesos
	volatile uint8 ppu[3][5];
	volatile uint8 grado[3][1];			//Contiene el codigo del producto en cada grado
	volatile uint8 mangueras;
};
struct surtidor lado1;
struct surtidor lado2;
/****************************************************************************************************************
								Variables para manejo de recibo
****************************************************************************************************************/
volatile uint8 nombre[33];
volatile uint8 nit[15];
volatile uint8 telefono[20];
volatile uint8 direccion[33];
volatile uint8 lema1[32];
volatile uint8 lema2[32];
volatile uint8 id_logo[2];
volatile uint8 km[2];
uint8 fecha[3];
uint8 hora[2];
struct recibo{					//Datos venta
    uint8 impresiones;
    uint8 posicion;
    uint8 grado;
    uint8 ppu[6];
    uint8 dinero[9];	
    uint8 volumen[9];
	uint8 producto;
	uint8 manguera;
    uint8 preset[8]; 		//el bit 0 es el tipo de preset	
	uint8 id[16];
	uint8 km[11];
	uint8 placa[9];
	uint8 imprimir;			//Bandera de impresion
	uint8 tipo_venta;		// 0 Contado 1 credito 2 autoservicio
	uint8 cedula[11];
	uint8 password[11];
	uint8 turno;
	uint8 autorizado;		//Autorizado 0 venta mux local - 0x30 turno cerrado - 0x31 venta con nsx y turno abierto - 0x32 venta CDG   	
};
struct recibo rventa1;   	//datos de la venta actual lado 1
struct recibo rventa2;   	//datos de la venta actual lado 2

/****************************************************************************************************************
								Variables para manejo de Impresora
****************************************************************************************************************/
uint8	print1[2];													//Puerto de la impresora
uint8	print2[2];



#endif

//[] END OF FILE
