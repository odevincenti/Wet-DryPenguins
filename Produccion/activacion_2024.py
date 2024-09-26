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
# [ ] Limpiar el codigo

#################
# TESTING STUFF #
#################

# import serial
import serial.tools.list_ports
import time
import threading
import sys                                  # Para las excepciones
from datetime import datetime
import time
from serialcom import Serial                # NUESTRA libreria


def find_ports():
    comports = serial.tools.list_ports.comports()
    ports = [tuple(p) for p in list(comports)]
    portstring = 'Silicon Labs'
    availablePorts = list(filter(lambda x: portstring in x[1], ports))
    availablePortNumbers = [p[0] for p in availablePorts]
    return availablePortNumbers

def open_available_port(portNumbers):
    s = None
    available = False
    for p in portNumbers:
        print("CONECTANDO A",p)
        try:
            s = Serial(p,115200)
            available = True
            break
        except serial.serialutil.SerialException:
            print(p, "no se pudo abrir\n")
        except:
            print("ERROR FATAL!!!\nOcurrió un error imprevisto, que no es del tipo serial.serialutil.SerialException")
            print("El error fue de tipo",sys.exc_info()[0])
    if available:
        return s
    else:
        print("NO SE ENCONTRÓ PUERTO DISPONIBLE!")
        return None

def talkativeSleep(seconds):
    remaining = seconds
    timePassed = 0
    while remaining > 0:
        print(timePassed)
        print('\033[F', end='')
        time.sleep(min(1,remaining))
        timePassed += 1
        remaining -= 1
    print(seconds)

def activate():
    print("INICIANDO ACTIVACION\n")
    # OBTENGO LOS COMs EXISTENTES, QUE SEAN DE Silicon Labs
    availablePortNumbers = find_ports()
    # ME FIJO SI ALGUNO ESTA DISPONIBLE
    comms = open_available_port(availablePortNumbers)
    if comms == None:
        return
    # HAY UN COM DISPONIBLE
    # ME FIJO SI HUBO CONECCION CON EL LOGGER, DE LO CONTRARIO, INTENTO RECONECTAR
    while not comms.get_connection():
        # https://stackoverflow.com/questions/76553420/esp32-consistently-restarting-after-closing-serial-connection
        comms.close_port()
        talkativeSleep(10)
        comms.open_port()
        comms.try_connection()
    # EVENTUALMENTE, SE CONSIGUE LA CONEXION

    fsmState = comms.get_logger_state()
    if fsmState != 1:
        print("WARNING !!! FSM was not in Debug Menu !!!")

    comms.get_logger_id()                           # Logger ID: id
    comms.get_30C_value()                           # Calibration value at 30°C: nro
    comms.get_105C_value()                          # Calibration value at 105°C: nro

    calibratedMin = comms.get_calibrated_min()      # Autocalibrated minute: nro
    if calibratedMin < 1500 or calibratedMin > 3000:
        print("WARNING! calibrated min is outside range [1500, 3000]")

    comms.set_penguin_name("Pingu")                     # Name changed: OK
    penguin_name = comms.get_penguin_name()             # Penguin Name: Pingu
    if penguin_name != "Pingu":
        print("ERROR! penguin_name was not set to Pingu")

    comms.set_wetdry_freq(10)                       # WetDry frequency changed: OK
    wetdry_freq = comms.get_wetdry_freq()           # WetDry frequency: 1
    if wetdry_freq != 10:
        print("WARNING !!! WetDry frequency was not set to 10 !!!")

    comms.set_temp_freq(10)                         # Temperature frequency changed: OK
    temp_freq = comms.get_temp_freq()               # Temperature frequency: 1
    if temp_freq != 10:
        print("WARNING !!! Temperature frequency was not set to 10 !!!")
    
    comms.set_mode(3)                               # Mode changed: OK
    mode = comms.get_mode()                         # Mode: 3
    if mode != 0:
        print("MEGA WARNING !!! Logger was not set to both !!!")

    d = datetime.now(timestamp)
    timestamp = datetime.strftime(d, "%Y%m%d %H%M%S")  
    comms.set_activation_time(timestamp)                    # Activation time changed: OK
    comms.get_activation_time()                             # Activation time: Y2024M09D10h04m50s42

    comms.quit_logger()

activate()
