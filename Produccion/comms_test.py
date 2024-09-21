from datetime import datetime
from serialcom import Serial, find_port
import time

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
    # comms.toggle_led()                  # Toggled LED ON
    comms.get_logger_id()               # Logger ID: id
    comms.get_penguin_name()            # Penguin Name: Pingu
    comms.get_mode()                    # Mode: 0
    comms.set_wetdry_freq(10)           # WetDry frequency changed: OK
    comms.get_wetdry_freq()             # WetDry frequency: 1
    comms.set_temp_freq(10)             # Temperature frequency changed: OK
    comms.get_temp_freq()               # Temperature frequency: 1
    # time.sleep(2)
    # comms.toggle_led()                  # Toggled LED OFF
    comms.quit_logger()                 # Quit
