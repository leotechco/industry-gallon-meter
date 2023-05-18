#!/usr/bin/ python3
import socket
import time
import requests
import sqlite3
import os
import serial


casos = {255:'error', 0:'config', 1:'espera', 2:'pet_autorizar', 3:'surtiendo', 4:'venta', 5:'abrir_turno', 6:'sincronizar', 7:'calibrar', 8:'calibrar', 9:'calibrar', 10:'listo', 12:'leer_ibutton', 13:'reset_ibutton'}

class Protocolo:
    def __init__(self):
        self.new_estado = 0
        self.encabezado = 0x7E
        self.origen_mfc = 0x0F
        self.origen_dragon = 0x0D
        self.final_1 = 0xEF
        self.final_2 = 0xFE
        self.ack = 3
        self.nack = 0
        self.error = 0xFF
        self.error_conect = 0xFE
        self.timeout = 5
        self.b_ibutton = 0
        self.ppu = 0
        self.fecha_i = None
        self.cliente = ''
        self.no_recibo = '0'
        self.direccion = '0'
        self.placa  = '0'
        self.imprimir = 0
        self.informa_ventas = ''
        self.ibutton = [0,0,0,0,0,0,0,0]

    def enviar_recibir(self, l, trama, size):
        l.write(trama)
        total_data = l.read(size)
        return total_data

    def estado(self, l):
        trama = [self.encabezado, self.origen_dragon, 1, 0, self.final_1, self.final_1]
        total_data = self.enviar_recibir(l, trama, 7)
        print(total_data)
        if len(total_data) == 7:
            if total_data[0] == self.encabezado and total_data[1] == self.origen_mfc and total_data[2] == 1:
                estado = total_data[4]
                return estado
            else:
                return self.error
        else:
            return self.error

    def espera(self, sock):
        print('Espera')

    def config(self,sock):
        dia = int(time.strftime("%d"))
        mes = int(time.strftime("%m"))
        ano = int(time.strftime("%y"))
        hora = int(time.strftime("%H"))
        minutos = int(time.strftime("%M"))
        trama = [self.encabezado, self.origen_dragon, 2, 41, dia, mes, ano, hora, minutos, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, self.final_1, self.final_1]
        total_data = self.enviar_recibir(sock, trama, 7)
        print(total_data)
        if len(total_data) == 7:
            if total_data[0] == self.encabezado and total_data[1] == self.origen_mfc and total_data[2] == 2:
                estado = total_data[4]
                return estado
            else:
                return self.error
        else:
            return self.error

    def leer_ibutton(self,sock):
        if self.b_ibutton == 2:
            self.b_ibutton = 0
            trama = [self.encabezado, self.origen_dragon, 0x0C, 8, mfc.ibutton[0], mfc.ibutton[1], mfc.ibutton[2], mfc.ibutton[3], mfc.ibutton[4], mfc.ibutton[5], mfc.ibutton[6], mfc.ibutton[7], self.final_1, self.final_1]
            total_data = self.enviar_recibir(sock, trama, 7)
            print(total_data)
            if len(total_data) == 7:
                if total_data[0] == self.encabezado and total_data[1] == self.origen_mfc and total_data[2] == 0x0C:
                    estado = total_data[4]
                    return estado
                else:
                    return self.error
            else:
                return self.error
        else:
            self.b_ibutton = 1

    def pet_autorizar(self, sock):
        trama = [self.encabezado, self.origen_dragon, 0x03, 0, self.final_1, self.final_1]
        total_data = self.enviar_recibir(sock, trama, 15)
        print(total_data)
        if len(total_data) == 15:
            if total_data[0] == self.encabezado and total_data[1] == self.origen_mfc and total_data[2] == 0x03:
                id = ''
                for i in range(12,4,-1):
                    id += format(str(hex(total_data[i]).split('x')[-1]), "0>2s")
                f = open("clientes.txt", "r")
                clientes = f.read()
                indice_1 = clientes.find(id)
                inicio = 0
                fin = 0

                id = id.upper()
                indice_2 = clientes.find(id)
                indice = 0
                if indice_1 >= 1:
                    indice = indice_1
                    centro = indice
                if indice_2 >= 1:
                    indice = indice_2
                    centro = indice
                if indice >= 1:
                    while clientes[centro] != '{':
                        centro -= 1
                        inicio += 1
                    centro = indice
                    while clientes[centro] != '{':
                        centro += 1
                        fin += 1
                    msg = clientes[(indice-inicio):(indice+fin)]
                    msg = msg.replace('"','')
                    inicio = msg.find('Nombre')
                    fin = msg.find('VehiculoID')
                    self.cliente = msg[inicio:fin]
                    self.cliente = self.cliente.replace('Nombre:','')
                    self.cliente = self.cliente.replace(',', '')
                    inicio = msg.find('Placa')
                    fin = msg.find('ChipID')
                    self.placa = msg[inicio:fin]
                    self.placa = self.placa.replace('Placa:', '')
                    self.placa = self.placa.replace(',', '')
                    bd = sqlite3.connect("carrotaller.db")
                    print("Base de datos abierta")
                    cursor = bd.cursor()
                    sentencia = "SELECT ppu FROM precios WHERE id=1;"
                    cursor.execute(sentencia)
                    reg_ppu = cursor.fetchall()
                    self.ppu = reg_ppu[0]
                    self.ppu = str(self.ppu[0])
                    cursor.close()
                    bd.close()
                    f.close()
                    trama = [self.encabezado, self.origen_dragon, 0x04, 0, 3, int(self.ppu[0]), int(self.ppu[1]), int(self.ppu[2]), int(self.ppu[3]), 0]
                    trama[3] = len(msg)+6
                    trama_fin = [self.final_1, self.final_1]
                    trama_centro = list(msg)
                    for i in range(len(trama_centro)):
                        trama_centro[i] = ord(trama_centro[i])
                    trama_total = trama + trama_centro + trama_fin
                    print(trama_total)
                    total_data = self.enviar_recibir(sock, trama_total, 7)
                    if len(total_data) == 7:
                        if total_data[0] == self.encabezado and total_data[1] == self.origen_mfc and total_data[2] == 0x04:
                            self.fecha_i = time.strftime("%Y%m%d%H%M%S")
                            estado = total_data[4]
                            return estado
                        else:
                            return self.error
                    else:
                        return self.error
                else:
                    msg = 'No se encuentra el Chip'
                    trama = [self.encabezado, self.origen_dragon, 0x04, 0, 4, 0, 0, 0, 0, 0]
                    trama[3] = len(msg) + 6
                    trama_fin = [self.final_1, self.final_1]
                    trama_centro = list(msg)
                    for i in range(len(trama_centro)):
                        trama_centro[i] = ord(trama_centro[i])
                    trama_total = trama + trama_centro + trama_fin
                    total_data = self.enviar_recibir(sock, trama_total, 7)
                    if len(total_data) == 7:
                        if total_data[0] == self.encabezado and total_data[1] == self.origen_mfc and total_data[2] == 0x04:
                            estado = total_data[4]
                            return estado
                        else:
                            return self.error
                    else:
                        return self.error
            else:
                return self.error
        else:
            return self.error

    def surtiendo(self,sock):
        print('surtiendo')

    def listo(self,sock):
        print('listo')

    def calibrar(self,sock):
        print('calibrando')

    def venta(self,sock):
        print('venta')
        trama = [self.encabezado, self.origen_dragon, 5, 0, self.final_1, self.final_2]
        total_data = self.enviar_recibir(sock, trama, 32)
        print(total_data)
        if len(total_data) == 32:
            if total_data[0] == self.encabezado and total_data[1] == self.origen_mfc and total_data[2] == 5:
                id = ''
                km = ''
                for i in range(12, 4, -1):
                    id += format(str(hex(total_data[i]).split('x')[-1]), "0>2s")
                for i in range(20, 30):
                    if total_data[i] < 10:
                        km += str(total_data[i])
                fecha_f = time.strftime("%Y%m%d%H%M%S")
                id = id.upper()
                volumen = total_data[13] + (total_data[14] * 10) + (total_data[15] * 100) + (total_data[16] * 1000) + (total_data[17] * 10000) + (total_data[18] * 100000) + (total_data[19] * 1000000)
                ppu = int(self.ppu)
                dinero = round((ppu * volumen)/1000)
                placa = self.placa
                estado = total_data[4]
                bd = sqlite3.connect("carrotaller.db")
                print("Base de datos abierta")
                cursor = bd.cursor()
                sync = 0
                sentencia = "INSERT INTO ventas(fecha_i, fecha_f,  chip, dinero, volumen, ppu, km, placa, Sync) VALUES (?,?,?,?,?,?,?,?,?)"
                cursor.execute(sentencia, [self.fecha_i, fecha_f,  id, dinero, volumen, ppu, km, placa, sync])
                bd.commit()
                cursor.close()
                bd.close()
                self.imprimir = 1
                return estado
            else:
                return self.error
        else:
            return self.error

    def reset_ibutton(self,sock):
        self.b_ibutton = 0
        trama = [self.encabezado, self.origen_dragon, 0x0D, 1, 1, self.final_1, self.final_2]
        total_data = self.enviar_recibir(sock, trama, 7)
        print(total_data)
        if len(total_data) == 7:
            if total_data[0] == self.encabezado and total_data[1] == self.origen_mfc and total_data[2] == 0x0C:
                estado = total_data[4]
                return estado
            else:
                return self.error
        else:
            return self.error

    def sincronizar(self,sock):
        trama = [self.encabezado, self.origen_dragon, 7, 1, 0, self.final_1, self.final_1]
        trama_1 = "http://fcservices.distracom.com.co/TestRestPos/TramaRestService.svc/GetProccesSaleRest/"
        bd = sqlite3.connect("carrotaller.db")
        print("Base de datos abierta")
        cursor = bd.cursor()
        sentencia = "SELECT * FROM ventas WHERE sync = 0;"
        cursor.execute(sentencia)
        registros = cursor.fetchall()
        error = 0
        if len(registros) > 0:
            self.informa_ventas = '    Informa de Ventas Subidas\n'
            for venta in registros:
                print('Sincronizando...')
                id_reg = venta[0]
                fecha_i = venta[1]
                fecha_f = venta[2]
                id = venta[3]
                dinero = venta[4]
                volumen = venta[5]
                ppu = venta[6]
                km = venta[7]
                placa = venta[8]
                trama_2 = "F2;{};{};{};{};I-{};0;0;2;1;0;0;0;{};0;0;{};1;1;9999000;TRUE;0;MFC105".format(fecha_i,fecha_f,volumen,dinero,id,km,ppu)
                trama_total = trama_1 + trama_2
                try:
                    r = requests.get(trama_total)
                    if r.status_code == 200:
                        respon_jason = r.json()
                        texto_jason = respon_jason['GetProccesSaleRestResult']
                        ack = texto_jason.count('01ACK')
                        if ack == 1:
                            sentencia = "UPDATE ventas SET sync = 1 WHERE id = ?;"
                            cursor.execute(sentencia, [id_reg])
                            bd.commit()
                            self.informa_ventas += "Fecha: {}-{}-{} {}:{}\nPlaca: {}\nDinero: ${}\nVolumen: G{}\n\n".format(fecha_f[0:4], fecha_f[4:6], fecha_f[6:8], fecha_f[8:10], fecha_f[10:12], placa, dinero,(volumen/1000))
                            self.imprimir = 2
                except OSError as msg:
                    error += 1
                    print(msg)
        cursor.close()
        bd.close()
        print("Base de datos cerrada")
        if error == 0:
            trama[4] = 2
            total_data = self.enviar_recibir(sock, trama, 7)
            print(total_data)
            if len(total_data) == 7:
                if total_data[0] == self.encabezado and total_data[1] == self.origen_mfc and total_data[2] == 7:
                    estado = total_data[4]
                    return estado
                else:
                    return self.error
            else:
                return self.error
        else:
            trama[4] = 3
            total_data = self.enviar_recibir(sock, trama, 7)
            print(total_data)
            if len(total_data) == 7:
                if total_data[0] == self.encabezado and total_data[1] == self.origen_mfc and total_data[2] == 7:
                    estado = total_data[4]
                    return estado
                else:
                    return self.error
            else:
                return self.error

    def abrir_turno(self, sock):
        trama = [self.encabezado, self.origen_dragon, 7, 1, 0, self.final_1, self.final_1]
        try:
            r = requests.get(
                "http://fcservices.distracom.com.co/TestRestPos/TramaRestService.svc/GetInitialSettings/344;1")
            if r.status_code == 200:
                respon_jason = r.json()
                texto_jason = respon_jason['GetInitialSettingsResult']
                pos_ppu = texto_jason.find('MFC')
                ppu = texto_jason[(pos_ppu + 11):(pos_ppu + 15)]
                x = ppu.replace(",", "")
                self.ppu = x
                bd = sqlite3.connect("carrotaller.db")
                print("Base de datos abierta")
                cursor = bd.cursor()
                sentencia = "SELECT * FROM precios;"
                cursor.execute(sentencia)
                registros = cursor.fetchall()
                if len(registros) == 0:
                    sentencia = "INSERT INTO precios(ppu) VALUES (?)"
                    cursor.execute(sentencia, [int(self.ppu)])
                    bd.commit()
                else:
                    sentencia = "UPDATE precios SET ppu = ? WHERE rowid = 1;"
                    cursor.execute(sentencia, [int(self.ppu)])
                    bd.commit()
                cursor.close()
                bd.close()
                print("Base de datos cerrada")
            else:
                trama[4] = 3
                total_data = self.enviar_recibir(sock, trama, 7)
                print(total_data)
                if len(total_data) == 7:
                    if total_data[0] == self.encabezado and total_data[1] == self.origen_mfc and total_data[2] == 7:
                        estado = total_data[4]
                        return estado
                    else:
                        return self.error
                else:
                    return self.error
            r = requests.get(
                "http://fcservices.distracom.com.co/TestRestPos/TramaRestService.svc/GetInitialVehiculeByEDS/344")
            if r.status_code == 200:
                respon_jason = r.json()
                texto_jason = respon_jason['GetInitialVehiculeByEDSResult']
                print(texto_jason)
                clientes = open("clientes.txt", "w")
                clientes.write(texto_jason)
                clientes.close()
                trama[4] = 1
                total_data = self.enviar_recibir(sock, trama, 7)
                print(total_data)
                if len(total_data) == 7:
                    if total_data[0] == self.encabezado and total_data[1] == self.origen_mfc and total_data[2] == 7:
                        estado = total_data[4]
                        return estado
                    else:
                        return self.error
                else:
                    return self.error
            else:
                trama[4] = 3
                total_data = self.enviar_recibir(sock, trama, 7)
                print(total_data)
                if len(total_data) == 7:
                    if total_data[0] == self.encabezado and total_data[1] == self.origen_mfc and total_data[2] == 7:
                        estado = total_data[4]
                        return estado
                    else:
                        return self.error
                else:
                    return self.error
        except OSError as msg:
            print(msg)
            trama[4] = 3
            total_data = self.enviar_recibir(sock, trama, 7)
            print(total_data)
            if len(total_data) == 7:
                if total_data[0] == self.encabezado and total_data[1] == self.origen_mfc and total_data[2] == 7:
                    estado = total_data[4]
                    return estado
                else:
                    return self.error
            else:
                return self.error

    def selec_caso(self, caso, sock):
        method_name = caso
        method = getattr(self, method_name, lambda: "error")
        return method(sock)

    def error(self, sock):
        print('error')


mfc = Protocolo()

def init_bd():
    bd = sqlite3.connect("carrotaller.db")
    print("Base de datos abierta")
    cursor = bd.cursor()
    tablas = [
        """
            CREATE TABLE IF NOT EXISTS ventas(
                id INTEGER PRIMARY KEY,
                fecha_i TEXT NOT NULL,
                fecha_f TEXT NOT NULL,
                chip    TEXT NOT NULL,
                dinero  INT  NOT NULL,
                volumen INT  NOT NULL,
                ppu     INT  NOT NULL,
                km      TEXT NOT NULL,
                placa   TEXT,
                sync    INT NOT NULL
            );
        """,
        """
            CREATE TABLE IF NOT EXISTS precios(
                id INTEGER PRIMARY KEY,
                ppu   INT NOT NULL
            );
        """
    ]
    for tabla in tablas:
        cursor.execute(tabla)
    print("Tablas creadas correctamente")
    cursor.close()
    bd.close()
    print("Base de datos cerrada")

def imprimir(ok):
    if ok == 2:
        printMACAddress = '00:1B:35:12:16:3D'
        port = 1
        s = socket.socket(socket.AF_BLUETOOTH, socket.SOCK_STREAM, socket.BTPROTO_RFCOMM)
        s.settimeout(10)
        print(mfc.informa_ventas)
        try:
            s.connect((printMACAddress, port))
            s.send(bytes(mfc.informa_ventas, 'UTF-8'))
            s.close()
            mfc.imprimir = 0
        except OSError as msg:
            s.close()
            print(msg)

    if ok == 1:
        bd = sqlite3.connect("carrotaller.db")
        print("Base de datos abierta")
        cursor = bd.cursor()
        sentencia = "SELECT * FROM ventas WHERE fecha_i=?;"
        cursor.execute(sentencia,[mfc.fecha_i])
        registros = cursor.fetchall()
        for venta in registros:
            no_rec  = venta[0]
            dinero = venta[4]
            volumen = venta[5]/1000
            ppu = venta[6]
            km = venta[7]
        cliente = mfc.cliente
        if len(cliente) >= 20:
            cliente = cliente[0:18]
        placa = mfc.placa
        fecha = time.strftime("%Y-%m-%d %H:%M:%S")
        cursor.close()
        bd.close()
        texto_recibo = "     ESTACION DE SERVICIO     \u000d\u000a    MINCIVIL CARROTALLER      \u000d\u000a" \
                       "     NIT: 811.009.788-8       \u000d\u000a        " \
                       "TEL:2894490           \u000d\u000a " \
                       "DIR: AUTOPISTA NORTE KM 26   \u000d\u000a          " \
                       "Medellin            \u000d\u000a---------------------------\u000d\u000a" \
                       "Cliente:{: >19}   \u000d\u000a" \
                       "Contrato:     No Encontrado   \u000d\u000a" \
                       "---------------------------\u000d\u000a " \
                       "FECHA {}    \u000d\u000aNUMERO DE RECIBO: {:0>10}  \u000d\u000a  " \
                       "EQUIPO - CARA - MANGUERA    \u000d\u000a   MFC105 -   1 -      1      \u000d\u000a                           \u000d\u000a " \
                       "CANTIDAD -  PRODUCTO  - PPU  \u000d\u000a   {} -   DIESEL - $ {}\u000d\u000a---------------------------\u000d\u000a" \
                       "Placa:                  {: >6}\u000d\u000aCedula:                      0\u000d\u000a" \
                       "Nit  :                       0\u000d\u000a---------------------------\u000d\u000a      " \
                       "Total: ${: >7}         \u000d\u000a                           \u000d\u000a" \
                       "".format(cliente,fecha,no_rec,volumen,ppu,placa,dinero)
        printMACAddress = '00:1B:35:12:16:3D'
        port = 1
        s = socket.socket(socket.AF_BLUETOOTH, socket.SOCK_STREAM, socket.BTPROTO_RFCOMM)
        s.settimeout(10)
        print(texto_recibo)
        try:
            s.connect((printMACAddress, port))
            s.send(bytes(texto_recibo, 'UTF-8'))
            s.close()
            mfc.imprimir = 0
        except OSError as msg:
            s.close()
            print(msg)

def poll_mfc(ser):
        mfc.new_estado = mfc.estado(ser)
        print(mfc.new_estado)
        if mfc.new_estado != 255:
            mfc.selec_caso(casos[mfc.new_estado], ser)

def poll_ibutton():
    if mfc.b_ibutton == 1:
        ibMACAddress = '98:D3:31:20:5F:9D'
        port = 1
        s = socket.socket(socket.AF_BLUETOOTH, socket.SOCK_STREAM, socket.BTPROTO_RFCOMM)
        s.settimeout(10)
        try:
            print('Leyendo en Ibutton')
            s.connect((ibMACAddress, port))
            trama = [73, 100, 67, 104, 105, 112]
            mfc.ibutton = mfc.enviar_recibir(s, trama, 8)
            print(mfc.ibutton)
            if len(mfc.ibutton) == 8:
                if mfc.ibutton[0] != 255:
                    mfc.b_ibutton = 2
            s.close()
        except OSError as msg:
            s.close()
            print(msg)

def main():
    time.sleep(15)
    ser_mfc = serial.Serial('/dev/ttyUSB0', timeout=2)  # open serial port
    init_bd()
    while 1:
        poll_mfc(ser_mfc)
        poll_ibutton()
        imprimir(mfc.imprimir)
        time.sleep(1)

main()
