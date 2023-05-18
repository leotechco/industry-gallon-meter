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
* Filename      : Protocolo.c
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
/*
*********************************************************************************************************
*                                         get_estado( void )
*
* Description : Pregunta al surtidor el estado y lo retorna
*               
*
* Argument(s) : none
*
* Return(s)   : none
*
* Caller(s)   : 
*
* Note(s)     : none.
*********************************************************************************************************
*/

uint8 get_estado(uint8 dir){
    uint8 estado = 0;
    Surtidor_ClearRxBuffer();	
    Surtidor_PutChar(dir);
    CyDelay(100);
    if(Surtidor_GetRxBufferSize()==1){
        estado=Surtidor_ReadRxData();
    	if((estado&0x0F)==dir){                 //Revisa que el dato q llego corresponda a la direccion
    		estado=(estado&0xF0)>>4;
    	    Surtidor_ClearRxBuffer();	
    	    return estado;
    	}
    	else{                                   //si no hay dato valido se retorna 0               
    	    Surtidor_ClearRxBuffer();
    	    return 0;	
    	}
    }
    else{
    	Surtidor_ClearRxBuffer();
    	return 0;	    
    }
}

/*
*********************************************************************************************************
*                                         uint8 ver_pos( void )
*
* Description : Lee las pos del equipo.
*               
*
* Argument(s) : none
*
* Return(s)   : Las dos direcciones en un solo dato
*
* Caller(s)   : main()
*
* Note(s)     : none.
*********************************************************************************************************
*/

uint8 ver_pos (void){
    uint8 x, pos1, pos2;
    pos1=0xFF;                  //Inicia pos en ff
    pos2=0xFF;
	CyDelay(5);
    if(get_estado(0)!=0){       //Se revisa si la pos es la 16 para ponerla en la 2
		pos2=0;
	}	
    for(x=1;x<=15;x++){         //Se miran las pos desde la 1 hasta las 15
		CyDelay(5);
        if(get_estado(x)!=0){
			if(pos1==0xFF){
				pos1=x;
			}
			else if(pos2==0xFF){
				pos2=x;
			}
		}
	}
	if((pos1!=0xFF)&&(pos2!=0xFF)){         //se verifica q las dos pos tomaron direccion
		return ((pos2 & 0x0F)<<4)|(pos1 & 0x0F);
	}
	else{                                   //se retorna 0 si no hay direcciones
		return 0;
	}
}

/*
*********************************************************************************************************
*                                         estado_ex(uint8 val)
*
* Description : 
*               
*
* Argument(s) : none
*
* Return(s)   : none
*
* Caller(s)   : 
*
* Note(s)     : none.
*********************************************************************************************************
*/
uint8 estado_ex(uint8 val){
    uint8 rx_extend[19],x,comand_exten[9]={0xFF,0xE9,0xFE,0xE0,0xE1,0xE0,0xFB,0xEE,0xF0};
    Surtidor_PutChar(0x20|val);
    CyDelay(100);
    if(Surtidor_GetRxBufferSize()>=1){
        if(Surtidor_ReadRxData()==(0xD0|val)){
            Surtidor_ClearRxBuffer();
            for(x=0;x<=8;x++){
               Surtidor_PutChar(comand_exten[x]); 
               CyDelay(5); 
            }
            CyDelay(200);
            if(Surtidor_GetRxBufferSize()>=19){
                for(x=0;x<=18;x++){
                   rx_extend[x]=Surtidor_ReadRxData(); 
                }
                Surtidor_ClearRxBuffer();
                if((rx_extend[0]==0xBA)&&(rx_extend[17]==0x8D)&&(rx_extend[18]==0x8A)&&(rx_extend[12]==0xB0)){
                    switch(rx_extend[14]){
                        case 0xB1:
                            return 1;
                        break;
                        
                        case 0xB2:
                            return 2;
                        break;
                        
                        case 0xB3:
                            return 3;
                        break;
                        
                        default:
                            return 0;
                    }
                }
                else{
                    return 0;
                }
            }
            else{
                return 0;
            }
        }
        else{
            return 0;
        }
    }
    else{
        return 0;
    }
}


uint8 venta(uint8 dir){
	uint8 x;
	Surtidor_ClearRxBuffer();
	Surtidor_PutChar(0x40|dir);
    CyDelay(300);
	if((version!=7)&&(Surtidor_GetRxBufferSize()==33)){
		if((Surtidor_rxBuffer[0]==0xFF)&&(Surtidor_rxBuffer[2]==0xF8)&&(Surtidor_rxBuffer[32]==0xF0)){
			if(dir==lado1.dir){
				rventa1.producto=(Surtidor_rxBuffer[9]&0x0F)+1;
				for(x=0;x<=3;x++){
					rventa1.ppu[x]=(Surtidor_rxBuffer[x+12]&0x0F);
				}
				for(x=0;x<=5;x++){
					rventa1.volumen[x]=(Surtidor_rxBuffer[x+17]&0x0F);
				}			
				for(x=0;x<=5;x++){
					rventa1.dinero[x]=(Surtidor_rxBuffer[x+24]&0x0F);
				}
			}
			else{
				rventa2.producto=(Surtidor_rxBuffer[9]&0x0F)+1;
				for(x=0;x<=3;x++){
					rventa2.ppu[x]=(Surtidor_rxBuffer[x+12]&0x0F);
				}
				for(x=0;x<=5;x++){
					rventa2.volumen[x]=(Surtidor_rxBuffer[x+17]&0x0F);
				}			
				for(x=0;x<=5;x++){
					rventa2.dinero[x]=(Surtidor_rxBuffer[x+24]&0x0F);
				}			
			}
			return 1;
		}
		else{
			return 0;
		}
	}
	else if((version==7)&&(Surtidor_GetRxBufferSize()==39)){
		if((Surtidor_rxBuffer[0]==0xFF)&&(Surtidor_rxBuffer[2]==0xF8)&&(Surtidor_rxBuffer[38]==0xF0)){
			if(dir==lado1.dir){
				rventa1.producto=(Surtidor_rxBuffer[9]&0x0F)+1;
				for(x=0;x<=5;x++){
					rventa1.ppu[x]=(Surtidor_rxBuffer[x+12]&0x0F);
				}
				for(x=0;x<=7;x++){
					rventa1.volumen[x]=(Surtidor_rxBuffer[x+19]&0x0F);
				}			
				for(x=0;x<=7;x++){
					rventa1.dinero[x]=(Surtidor_rxBuffer[x+28]&0x0F);
				}
			}
			else{
				rventa2.producto=(Surtidor_rxBuffer[9]&0x0F)+1;
				for(x=0;x<=5;x++){
					rventa2.ppu[x]=(Surtidor_rxBuffer[x+12]&0x0F);
				}
				for(x=0;x<=7;x++){
					rventa2.volumen[x]=(Surtidor_rxBuffer[x+19]&0x0F);
				}			
				for(x=0;x<=7;x++){
					rventa2.dinero[x]=(Surtidor_rxBuffer[x+28]&0x0F);
				}			
			}
			return 1;
		}
		else{
			return 0;
		}
	}
	return 0;
}


/* [] END OF FILE */
