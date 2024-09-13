from datetime import datetime
from serialcom import Serial, find_port

comms = None
port = find_port()
if port != '':
    if comms is not None:
        comms.serial.close()
        del comms
        comms = None
    comms = Serial(port, 115200)

while not comms.get_connection():
    comms.try_connection()

if comms.get_connection():
    comms.toggle_led()                  # Toggled LED ON
    comms.get_logger_id()               # Logger ID: id
    comms.set_penguin_name("Pingu")     # Name changed: OK
    comms.get_penguin_name()            # Penguin Name: Pingu
    comms.autocalibrate_min()           # Autocalibrated minute: nro
    # comms.get_30C_value()               # Calibration value at 30°C: nro
    # comms.get_105C_value()              # Calibration value at 105°C: nro
    comms.set_mode(3)                   # Mode changed: OK
    comms.set_wetdry_freq(1)            # WetDry frequency changed: OK
    comms.get_wetdry_freq()             # WetDry frequency: 1
    comms.set_temp_freq(1)              # Temperature frequency changed: OK
    comms.get_temp_freq()               # Temperature frequency: 1
    comms.toggle_led()                  # Toggled LED OFF
    comms.get_mode()                    # Mode: 3
    d = datetime.now()
    timestamp = datetime.strftime(d, "%Y%m%d %H%M%S")  
    comms.set_activation_time(timestamp)                    # Activation time changed: OK
    comms.get_activation_time()                             # Activation time: Y2024M09D10h04m50s42
    comms.quit_logger()                 # Quit
