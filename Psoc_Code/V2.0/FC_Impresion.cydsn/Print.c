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
* Filename      : Print.h
* Version       : V1.00
* Programmer(s) : Leonardo Orjuela   
                  
*********************************************************************************************************
*/

/*
*********************************************************************************************************
*                                             INCLUDE FILES
*********************************************************************************************************
*/
#include <project.h>
#include "VariablesG.h"
#include "i2c.h"

void imprimir(uint8 pos, uint8 prod, uint8 *ppu, uint8 *volumen, uint8 *dinero){
    uint8 x, digito;
    
	RS232_PutString("      ESTACION DE SERVICIO\n");	
    //RS232_PutString("        TEXACO CALIFORNIA\n");
	for(x=0;x<=((31-nombre[0])/2);x++){						//NOMBRE
		RS232_PutChar(0x20);
	}
	for(x=1;x<=nombre[0];x++){
		RS232_PutChar(nombre[x]);			
	}
    RS232_PutChar(10);
	if(nit[0]>=1){
		for(x=0;x<=((31-(nit[0]+4))/2);x++){				//NIT
			RS232_PutChar(0x20);			
		}											
		RS232_PutString("NIT ");
		for(x=1;x<=nit[0];x++){
			RS232_PutChar(nit[x]);			
		}
		RS232_PutChar(10);
	}
	if(telefono[0]>=1){
		for(x=0;x<=((31-(telefono[0]+4))/2);x++){	        //TELEFONO
			RS232_PutChar(0x20);			
		}											
		RS232_PutString("TEL:");
		for(x=1;x<=telefono[0];x++){
			RS232_PutChar(telefono[x]);			
		}			
	    RS232_PutChar(10);
	    RS232_PutChar(10);
	}
	for(x=0;x<=((31-direccion[0])/2);x++){			        //DIRECCION
		RS232_PutChar(0x20);			
	}
	for(x=1;x<=direccion[0];x++){
		RS232_PutChar(direccion[x]);			
	}
    RS232_PutChar(10);
    RS232_PutChar(10);
    RS232_PutChar(10);    																	
	RS232_PutString("FECHA: ");                             //FECHA	
	if(leer_fecha()==1){
		RS232_PutChar(((fecha[0]&0x30)>>4)+48);
		RS232_PutChar((fecha[0]&0x0F)+48);
		RS232_PutChar('/');
		RS232_PutChar(((fecha[1]&0x10)>>4)+48);
		RS232_PutChar((fecha[1]&0x0F)+48);	
		RS232_PutChar('/');
		RS232_PutChar(((fecha[2]&0xF0)>>4)+48);
		RS232_PutChar((fecha[2]&0x0F)+48);			
	}
    RS232_PutChar(10);	                                							
	RS232_PutString("HORA:  ");                             //HORA
	if(leer_hora()==1){										
		RS232_PutChar(((hora[1]&0x30)>>4)+48);
		RS232_PutChar((hora[1]&0x0F)+48);
		RS232_PutChar(':');
		RS232_PutChar(((hora[0]&0xF0)>>4)+48);
		RS232_PutChar((hora[0]&0x0F)+48);
	}
    RS232_PutChar(10);
    RS232_PutChar(10);																	
    RS232_PutString("POSICION:  ");                         //POSICION
    RS232_PutChar((pos/10)+48);
	RS232_PutChar((pos%10)+48);
    RS232_PutChar(10);    																		
	RS232_PutString("PRODUCTO:  ");                         //PRODUCTO 
    if(prod=='1'){
		for(x=1;x<=nombre_pro1[0];x++){
			RS232_PutChar(nombre_pro1[x]);			
		}		
	}
	else if(prod=='2'){
		for(x=1;x<=nombre_pro2[0];x++){
			RS232_PutChar(nombre_pro2[x]);			
		}	
	}
	else if(prod=='3'){
		for(x=1;x<=nombre_pro3[0];x++){
			RS232_PutChar(nombre_pro3[x]);			
		}	
	}	
	RS232_PutChar(10);																		
	RS232_PutString("PPU:       $/G ");                      //PPU
	digito=0;
	for(x=4;x>=1;x--){
        if((ppu[x]==0)&&(digito==0)){
            
        }
        else{			
            digito=1;
            RS232_PutChar(ppu[x]+48);
        }	
	}
	RS232_PutChar(ppu[0]+48);
	if(ppux10=='1'){
		RS232_PutChar(48);	
	}
	RS232_PutChar(10);    																		
	RS232_PutString("VOLUMEN:   G ");                      //VOLUMEN
	digito=0;
	for(x=6;x>=1;x--){
        if((volumen[x]==0)&&(digito==0)){
			if(x==3){
				RS232_PutChar('0');
				digito=1;
			}      	    
        }
        else{
			if(x==2){
				RS232_PutChar(',');
			}
            digito=1;
            RS232_PutChar(volumen[x]+48);
        }	
	}
	RS232_PutChar(volumen[0]+48);
    RS232_PutChar(10);										//DINERO							
	RS232_PutString("DINERO:    $ ");
    digito=0;	
	for(x=6;x>=1;x--){
        if((dinero[x]==0)&&(digito==0)){
            
        }
        else{
            digito=1;
            RS232_PutChar(dinero[x]+48);
        }	
	}
	if(version == 6){
		RS232_PutChar(dinero[0]+48);
	} 
    RS232_PutChar(10);
    RS232_PutChar(10);
	RS232_PutChar(10);                																											
	RS232_PutString("      GRACIAS POR SU COMPRA");                     //LEMA1
	RS232_PutChar(10); 																											
	RS232_PutString("          VUELVA PRONTO");                        //LEMA2
    RS232_PutChar(10); 
    RS232_PutChar(10); 
    RS232_PutChar(10);
    RS232_PutChar(10);
	RS232_PutChar(10); 
    RS232_PutChar(0x1D);
    RS232_PutChar(0x56);
	RS232_PutChar(0x31);    
}

/* [] END OF FILE */
