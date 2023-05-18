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
* Filename      : main.c
* Version       : V1.00
* Programmer(s) : Leonardo Orjuela   
                  
*********************************************************************************************************
*/

/*
*********************************************************************************************************
*                                           INCLUDE FILES AND DEFINE
*********************************************************************************************************
*/
#include <project.h>
#include "stdio.h"
#include "Protocolo.h"
#include "VariablesG.h"
#include "Print.h"
#include "i2c.h"

#if defined (__GNUC__)
    /* Add an explicit reference to the floating point printf library */
    /* to allow the usage of floating point conversion specifiers. */
    /* This is not linked in by default with the newlib-nano library. */
    asm (".global _printf_float");
#endif

#define versionFC 10
#define BUFFER_LEN  64u

char8 *parity[] = { "None", "Odd", "Even", "Mark", "Space" };
char8 *stop[] = { "1", "1.5", "2" };

/*
*********************************************************************************************************
*                                           init( void )
*
* Description : Verifica el serial, inicia los perisfericos, la version y los datos de la estación.
*               
*
* Argument(s) : none
*
* Return(s)   : none
*
* Caller(s)   : main()
*
* Note(s)     : none.
*********************************************************************************************************
*/
void init (void){
    uint8 x;
    CyGlobalIntEnable; /* Enable global interrupts. */
    Surtidor_EnableRxInt();
    RS232_EnableRxInt();
    I2C_1_Start();
    Surtidor_Start();
    RS232_Start();
    USBUART_Start(0u, USBUART_3V_OPERATION);
    EEPROM_Start();
    CyDelay(100);
    /************Lectura de Eeprom***************/
    lado1.dir = EEPROM_ReadByte(0);							//Lado A
	lado2.dir = EEPROM_ReadByte(1);							//Lado B
    lado1.dir = 1;							//Lado A
	lado2.dir = 2;							//Lado B    
    version   = EEPROM_ReadByte(2);							//Version
    ppux10    = EEPROM_ReadByte(3);							//Pppux10
    nombre[0] = EEPROM_ReadByte(4);							//Tamaño del nombre de la estacion
    for(x=1;x<=nombre[0];x++){                              //Nombre de la estacion
        nombre[x] = EEPROM_ReadByte(x+4);							
    }
    nit[0]    = EEPROM_ReadByte(34);					    //Tamaño Nit
    for(x=1;x<=nit[0];x++){                                 //Nit
        nit[x] = EEPROM_ReadByte(x+34);							
    }
    direccion[0] = EEPROM_ReadByte(64);					    //Tamaño direccion
    for(x=1;x<=direccion[0];x++){                           //direccion
        direccion[x] = EEPROM_ReadByte(x+64);							
    }
    telefono[0] = EEPROM_ReadByte(49);					    //Tamaño telefono
    for(x=1;x<=telefono[0];x++){                            //telefono
        telefono[x] = EEPROM_ReadByte(x+49);							
    }
    lado1.grado[0][0] = EEPROM_ReadByte(94);				//Codigo manguera 1 lado 1
    lado1.grado[1][0] = EEPROM_ReadByte(95);				//Codigo manguera 2 lado 1
    lado1.grado[2][0] = EEPROM_ReadByte(96);				//Codigo manguera 3 lado 1
    lado2.grado[0][0] = EEPROM_ReadByte(142);				//Codigo manguera 1 lado 2
    lado2.grado[1][0] = EEPROM_ReadByte(143);				//Codigo manguera 2 lado 2
    lado2.grado[2][0] = EEPROM_ReadByte(144);				//Codigo manguera 3 lado 2  
    nombre_pro1[0]    = EEPROM_ReadByte(97);				//Tamaño nombre producto 1
    for(x=1;x<=nombre_pro1[0];x++){                         //Producto 1
        nombre_pro1[x] = EEPROM_ReadByte(x+97);							
    } 
    nombre_pro2[0]    = EEPROM_ReadByte(112);				//Tamaño nombre producto 2
    for(x=1;x<=nombre_pro2[0];x++){                         //Producto 2
        nombre_pro2[x] = EEPROM_ReadByte(x+112);							
    }
    nombre_pro3[0]    = EEPROM_ReadByte(127);				//Tamaño nombre producto 3
    for(x=1;x<=nombre_pro3[0];x++){                         //Producto 3
        nombre_pro3[x] = EEPROM_ReadByte(x+127);							
    }
}
/*
*********************************************************************************************************
*                                           init_surtidor( void )
*
* Description : Revisa las posiciones del equipo y su estado actual.
*               
*
* Argument(s) : none
*
* Return(s)   : none
*
* Caller(s)   : main()
*
* Note(s)     : none.
*********************************************************************************************************
*/
void init_surtidor( void ){
    uint8 ok=0; 
    uint8 estado,posicion;
    Led_Write(1);
    while(ok!=2){
        ok=0;
        if(lado1.dir==lado2.dir){                       //No hay posiciones guardadas
            posicion = ver_pos();
            if(posicion!=0){
                lado1.dir = (posicion & 0x0F);
                lado2.dir = ((posicion>>4) & 0x0F);
                EEPROM_WriteByte(lado1.dir, 0);
                EEPROM_WriteByte(lado2.dir, 1);
            }
        }
        estado = get_estado(lado1.dir);
        if(estado != 0){
            ok++;
        }
        estado = get_estado(lado2.dir);
        if(estado != 0){
            ok++;
        }
        if(ok!=2){
           lado1.dir=0;
           lado2.dir=0; 
        }
    }
    Led_Write(0);
}
/*
*********************************************************************************************************
*                                        polling_surtidor(uint8 dir, uint8 estado)
*
* Description : Revisa el estado del surtidor.
*               
*
* Argument(s) : dir = Posicion del equipo, estate = estado anterior del equipo
*
* Return(s)   : uint8 con el resultado de la operacion
*
* Caller(s)   : main()
*
* Note(s)     : none.
*********************************************************************************************************
*/
uint8 polling_surtidor(uint8 dir, uint8 estate){
    uint8 estado, producto; 
    estado = get_estado(dir);
    switch(estado){ 
        case 0:                 //error
            estate = (estate & 0x0C);
            return estate;
        break;
    
        case 6:                 //Espera
            estate = (estate & 0x0D) | 1;
            return estate;           
        break;
 
        case 7:                 //Listo
            producto = estado_ex (dir);
            if(producto != 0){
                if(dir == lado1.dir){
                    rventa1.producto = producto;     //ver producto lado1
                }
                else{
                    rventa2.producto = producto;     //ver producto lado2
                }
                CyDelay(150);
                Surtidor_PutChar(0x10|dir);
                return 2;
            }
            CyDelay(150);
            Surtidor_PutChar(0x14);            
        break;
            
        case 8:                 //Autorizado
            return 2;
        break;    
        
        case 9:                 //Surtiendo
            return 2;
        break;
            
        case 0x0A:              //Venta
            CyDelay(100);
            if(venta(dir) == 1){
                return 4;
            }
        break;    
        
        case 0x0B:              //Venta
            CyDelay(100);
            if(venta(dir) == 1){
                return 4;
            }
        break;         
        
    }
    return 0;
    
}
/*
*********************************************************************************************************
*                               uint8 polling_boton(uint8 id, uint8 state)
*
* Description : Revisa el estado del surtidor.
*               
*
* Argument(s) : id = numero del boton, state = estado para saber que accion tomar, state
*
* Return(s)   : none
*
* Caller(s)   : main()
*
* Note(s)     : none.
*********************************************************************************************************
*/
uint8 polling_boton(uint8 id, uint8 state){
    uint8 y;
    y = Botones_Read();
    if((y & id) == 0){                 //Si el boton esta presionado
        Select0_RS232_Write(0);                    //Selecciono la impresora
        Select1_RS232_Write(0);
        switch (state){
            case 0:                                     //error
                RS232_PutString("\n\nNo hay comunicacion con esta posicion.\n\n\n\n\n\n");
                RS232_PutChar(0x1D);
                RS232_PutChar(0x56);
	            RS232_PutChar(0x31);                
            break;
                
            case 1:
                RS232_PutString("\n\nPosicion OK y no hay recibos para imprimir.\n\n\n\n\n\n");
                RS232_PutChar(0x1D);
                RS232_PutChar(0x56);
	            RS232_PutChar(0x31);
            break;                
                
            case 2:
                RS232_PutString("\n\nPosicion ocupada.\n\n\n\n\n\n");
                RS232_PutChar(0x1D);
                RS232_PutChar(0x56);
	            RS232_PutChar(0x31);            
            break;    

            case 4:
                if(id == 1){
                    imprimir(lado1.dir, lado1.grado[rventa1.producto - 1][0], rventa1.ppu, rventa1.volumen, rventa1.dinero);
                }
                else{
                    imprimir(lado2.dir, lado2.grado[rventa2.producto - 1][0], rventa2.ppu, rventa2.volumen, rventa2.dinero);
                }
                state = 1;
            break;
            
            case 5:
                if(id == 2){
                    imprimir(lado1.dir, lado1.grado[rventa1.producto - 1][0], rventa1.ppu, rventa1.volumen, rventa1.dinero);
                }
                else{
                    imprimir(lado2.dir, lado2.grado[rventa2.producto - 1][0], rventa2.ppu, rventa2.volumen, rventa2.dinero);
                }
                state = 1;
            break;
            
        }
        while(Botones_Read()!=3);        
    }
    return state;
}

void polling_usb(void){
    uint16 count;
    uint8 buffer[BUFFER_LEN], x; 
    if(USBUART_IsConfigurationChanged() != 0u) 
    {
        if(USBUART_GetConfiguration() != 0u) 
        {
            USBUART_CDC_Init();
        }
    } 
    if(USBUART_GetConfiguration() != 0u){
        if(USBUART_DataIsReady() != 0u){
            count = USBUART_GetAll(buffer);
            if(count != 0u){
                Led_Write(1);
                while(USBUART_CDCIsReady() == 0u);
                if(buffer[count-1] == '*'){    
                    switch(buffer[0]){
                        case '0':                               //Prueba de conexion
                            USBUART_PutString("OK");
                        break;
                        
                        case '1':                             //Version
                            if((buffer[2] == '6') || (buffer[2] == '7')){
                                version = buffer[2] & 0x0F;
                                USBUART_PutString("OK");
                                EEPROM_WriteByte(version,2);
                            }else{
                                USBUART_PutString("ERROR");
                            }      
                        break;
                            
                        case '2':                             //PPUX10
                            if((buffer[2] == '1') || (buffer[2] == '0')){
                                ppux10 = buffer[2];
                                EEPROM_WriteByte(ppux10,3);
                                USBUART_PutString("OK");
                            }else{
                                USBUART_PutString("ERROR");
                            }                        
                        break;
                            
                        case '3':                             //Nombre
                            if(count <= 33){
                                nombre[0] = count - 3;
                                for(x=2; x<(count-1); x++){
                                    nombre[x-1] = buffer[x];
                                }
                                for(x=0; x<=nombre[0]; x++){
                                    EEPROM_WriteByte(nombre[x],(x+4));
                                }
                                USBUART_PutChar(nombre[0]);
                            }else{
                                USBUART_PutString("ERROR");
                            }                        
                        break;  
                            
                        case '4':                             //Nit
                            if(count <= 18){
                                nit[0] = count - 3;
                                for(x=2; x<(count-1); x++){
                                    nit[x-1] = buffer[x];
                                }
                                for(x=0; x<=nit[0]; x++){
                                    EEPROM_WriteByte(nit[x],(x+34));
                                }                        
                                USBUART_PutString("OK");
                            }else{
                                USBUART_PutString("ERROR");
                            }                        
                        break;
                            
                        case '5':                             //Telefono
                            if(count <= 18){
                                telefono[0] = count - 3;
                                for(x=2; x<(count-1); x++){
                                    telefono[x-1] = buffer[x];
                                }
                                for(x=0; x<=telefono[0]; x++){
                                    EEPROM_WriteByte(telefono[x],(x+49));
                                }                        
                                USBUART_PutString("OK");
                            }else{
                                USBUART_PutString("ERROR");
                            }                        
                        break;
                            
                        case '6':                             //Direccion
                            if(count <= 33){
                                direccion[0] = count - 3;
                                for(x=2; x<(count-1); x++){
                                    direccion[x-1] = buffer[x];
                                }
                                for(x=0; x<=direccion[0]; x++){
                                    EEPROM_WriteByte(direccion[x],(x+64));
                                }                        
                                USBUART_PutString("OK");
                            }else{
                                USBUART_PutString("ERROR");
                            }                        
                        break;

                        case '7':                             //Producto Manguera 1 Lado 1
                            if((buffer[2] == '1') || (buffer[2] == '2') || (buffer[2] == '3')){
                                lado1.grado[0][0] = buffer[2];
                                EEPROM_WriteByte(lado1.grado[0][0],94);
                                USBUART_PutString("OK");
                            }else{
                                USBUART_PutString("ERROR");
                            }                        
                        break; 
                            
                        case '8':                             //Producto Manguera 2 Lado 1
                            if((buffer[2] == '1') || (buffer[2] == '2') || (buffer[2] == '3')){
                                lado1.grado[1][0] = buffer[2];
                                EEPROM_WriteByte(lado1.grado[1][0],95);
                                USBUART_PutString("OK");
                            }else{
                                USBUART_PutString("ERROR");
                            }                        
                        break; 
                            
                        case '9':                             //Producto Manguera 3 Lado 1
                            if((buffer[2] == '1') || (buffer[2] == '2') || (buffer[2] == '3')){
                                lado1.grado[2][0] = buffer[2];
                                EEPROM_WriteByte(lado1.grado[2][0],96);
                                USBUART_PutString("OK");
                            }else{
                                USBUART_PutString("ERROR");
                            }                        
                        break;  
                            
                        case 'A':                            //Producto Manguera 1 Lado 2
                            if((buffer[2] == '1') || (buffer[2] == '2') || (buffer[2] == '3')){
                                lado2.grado[0][0] = buffer[2];
                                EEPROM_WriteByte(lado2.grado[0][0],142);
                                USBUART_PutString("OK");
                            }else{
                                USBUART_PutString("ERROR");
                            }                        
                        break; 
                            
                        case 'B':                            //Producto Manguera 2 Lado 2
                            if((buffer[2] == '1') || (buffer[2] == '2') || (buffer[2] == '3')){
                                lado2.grado[1][0] = buffer[2];
                                EEPROM_WriteByte(lado2.grado[1][0],143);
                                USBUART_PutString("OK");
                            }else{
                                USBUART_PutString("ERROR");
                            }                        
                        break;
                            
                        case 'C':                            //Producto Manguera 3 Lado 2
                            if((buffer[2] == '1') || (buffer[2] == '2') || (buffer[2] == '3')){
                                lado2.grado[2][0] = buffer[2];
                                EEPROM_WriteByte(lado2.grado[2][0],144);
                                USBUART_PutString("OK");
                            }else{
                                USBUART_PutString("ERROR");
                            }                        
                        break;
                        
                        case 'D':                             //Nombre Producto 1
                            if(count <= 18){
                                nombre_pro1[0] = count - 3;
                                for(x=2; x<(count-1); x++){
                                    nombre_pro1[x-1] = buffer[x];
                                }
                                for(x=0; x<=nit[0]; x++){
                                    EEPROM_WriteByte(nombre_pro1[x],(x+97));
                                }                        
                                USBUART_PutString("OK");
                            }else{
                                USBUART_PutString("ERROR");
                            }                        
                        break; 
                            
                        case 'E':                             //Nombre Producto 2
                            if(count <= 18){
                                nombre_pro2[0] = count - 3;
                                for(x=2; x<(count-1); x++){
                                    nombre_pro2[x-1] = buffer[x];
                                }
                                for(x=0; x<=nit[0]; x++){
                                    EEPROM_WriteByte(nombre_pro2[x],(x+112));
                                }                        
                                USBUART_PutString("OK");
                            }else{
                                USBUART_PutString("ERROR");
                            }                        
                        break; 
                            
                        case 'F':                             //Nombre Producto 3
                            if(count <= 18){
                                nombre_pro3[0] = count - 3;
                                for(x=2; x<(count-1); x++){
                                    nombre_pro3[x-1] = buffer[x];
                                }
                                for(x=0; x<=nit[0]; x++){
                                    EEPROM_WriteByte(nombre_pro3[x],(x+127));
                                }                        
                                USBUART_PutString("OK");
                            }else{
                                USBUART_PutString("ERROR");
                            }                        
                        break;  
                            
                        case 'G':                            //Hora y Fecha
                            if((buffer[2] <= 0x59) && (buffer[3] <= 0x23) && (buffer[4] <= 0x31) && (buffer[5] <= 0x12)){
                                hora[0] = buffer[2];
                                hora[1] = buffer[3];
                                fecha[0] = buffer[4];
                                fecha[1] = buffer[5];
                                fecha[2] = buffer[6];
                                write_hora();
                                write_fecha();
                                USBUART_PutString("OK");
                            }else{
                                USBUART_PutString("ERROR");
                            }                        
                        break;                    
                            
                    }
                }
                if(count == BUFFER_LEN)
                {
                    while(USBUART_CDCIsReady() == 0u); 
                    USBUART_PutData(NULL, 0u);         
                }                
            }    
        }
    }
}


int main()
{
    init();
    for(;;)
    {
        Led_Write(1);
        polling_usb();       
    }
}

/* [] END OF FILE */
