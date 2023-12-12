import serial
import serial.tools.list_ports
import time
import threading
import datetime as dt

from keyboard import is_pressed

# CONTROL CHARS
#define CONNECT_TO_MSP  'S'
#define GET_FSM_STATE   'F'
#define GET_LOGGER_ID   'I'
#define GET_NAME        'N'
#define SET_NAME        'n'
#define GET_WET_AND_DRY_FREC  'W'
#define SET_WET_AND_DRY_FREC  'w'
#define GET_TEMPERATURE_FREC  'T'
#define SET_TEMPERATURE_FREC  't'
#define GET_OPERATING_MODE  'M'
#define SET_OPRETAING_MODE  'm'
#define SET_OPERATING_MODE__INACTIVE    '0'
#define SET_OPERATING_MODE__WET_AND_DRY '1'
#define SET_OPERATING_MODE__TEMPERATURE '2'
#define SET_OPERATING_MODE__BOTH        '3'
#define GET_CALIBRATED_MINUTE 'C'
#define SET_CALIBRATED_MINUTE 'c'
#define GET_ACTIVATION_TIME   'A'
#define SET_ACTIVATION_TIME   'a'
#define GET_AT_30C            '3'
#define GET_AT_105C           '1'
#define ACTIVATE_LOGGER       'Q'
#define DEACTIVATE_LOGGER     'q'
#define TOGGLE_LED            'L'
#define GET_DATA_FAST         'D'
#define GET_EVENTS            'E'
#define GET_ALL_INDEX         'X'

# PASOS PARA ACTIVAR
# S: Conectar (devuelve OK)
# m: SET OPERATING MODE
# 3: Wetdry y temp
# M: Para chequear
# Q: Activado (ANOTAR HORARIO)

# PASOS PARA DESACTIVAR
# S: Conectar (ANOTAR HORARIO)
# X: Indices
# D: Datos
# m: SET OPERATING MODE
# 0: Desactivar
# M: Para chequear
# q: Desactivado

def find_port():
    ports = [tuple(p) for p in list(serial.tools.list_ports.comports())]
    # print(ports)
    portstring = 'Silicon Labs'
    myport = list(filter(lambda x: portstring in x[1], ports))
    # print(myport)
    if myport != []:
        port = myport[0][0]
        # print(port)
        return port
    else:
        # print("Port not detected :(")
        return ''

id = 23
run = 8

print("Connected to logger " + str(id) + " in run " + str(run))
esp = serial.Serial(port=find_port(), baudrate=115200, timeout=1)

with open("output_from_msp_" + str(id) + "_T" + str(run) + "C.txt",'w') as file:
    while(True):
        for i in esp.readlines():
            file.write(i.decode('utf-8','replace'))
            print(i.decode(),end='')
        if(is_pressed('Left shift')):
            input_from_pc = input("INPUT FOR MSP ")
            esp.write(input_from_pc.encode())
            if input_from_pc == 'Q' or input_from_pc == 'S':
                now = dt.datetime.now()
                file.write("Time: " + now.strftime('%Y-%m-%d %H:%M:%S'))
                print("Time: ", now)
