# NOTAS:
# - Se asume que el minuto calibrado ya se encuentra ... calibrado
# - Se realiza un get de todos los datos del micro (exceptuando por supuesto el field data y los eventos)
# - Se trata de realizar la coneccion lo mas pronto posible, para reducir chances de que se desconecte (por cuestiones mecanicas) el logger de la base
# - Si hay un error de coneccion, se reintenta lo mas pronto posible
#   - CABE MENCIONAR que ante un error de coneccion, se esperara 10 segundos antes de reconectar (por si el logger quedo en UART, y por tanto deba apagarse por inactividad)

# from datetime import datetime
# from serialcom import Serial, find_port
# import time

# TODO ANTES DE ENTREGAR
# [ ] Reemplazar portstring = 'Dispositivo' por 'Silicon Labs'
# [ ] Limpiar el codigo

#################
# TESTING STUFF #
#################

# import serial
import serial.tools.list_ports
import time
import threading
import sys

# OBTENGO LOS COMs EXISTENTES, QUE SEAN DE Silicon Labs

comports = serial.tools.list_ports.comports()
# print("COMPORTS =", comports)
ports = [tuple(p) for p in list(comports)]
# print("PORTS =", ports)
portstring = 'Dispositivo' #'Silicon Labs'
availablePorts = list(filter(lambda x: portstring in x[1], ports))
# print("AVAILABLE PORTS =", availablePorts)
availablePortNumbers = [p[0] for p in availablePorts]
# print("AVAILABLE PORT NUMBERS =", availablePortNumbers)
# TODO: printear solo lo importante de esto (hay algo? Un mensage de OK?)

# ME FIJO SI ALGUNO ESTA DISPONIBLE

# availablePortNumbers = ['COM2','COM3','COM4','COM5']

s = None
available = False
for i,p in enumerate(availablePortNumbers):
    # comms = Serial(p, 115200)
    # print(i, "\t" , p)
    print("CONECTANDO A",p)
# TODO: usar nuestro creado Serial y no el serial.Serial de la libreria
    try:
        s = serial.Serial(p,115200)
        available = True
        break
    except serial.serialutil.SerialException as e:
        # print("SERIAL EXCEPTION =",e)
        print(p, "no se pudo abrir")
        # print(e)
    except:
        print("ERROR FATAL!!!\nOcurrio un error imprevisto, que no es del tipo serial.serialutil.SerialException")
        print("El error fue de tipo",sys.exc_info()[0])
    # print(s)
    # s.timeout = 1
    # print(s.readline())
# print("AVAILABLE =",available)
# print(s)

if not available:
    print("NO SE ENCONTRO PUERTO DISPONIBLE!")
else:
    print("SE CONECTO A",s.port,"!!!")
