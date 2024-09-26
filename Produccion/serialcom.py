import serial
import serial.tools.list_ports
import time
import threading

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
#define GO_BACK               '<'
#define GO_FORWARD            '>'
#define MASTER_CONTROL        '~'

SHORT_WAIT_TIME = 0.2
MEDIUM_WAIT_TIME = 2
LONG_WAIT_TIME = 3*60

# TODO: mensaje al inicio OK tomarlo bien
# TODO: descartar o alertar sobre mensajes "no atrapados"

class Serial:
	
    def __init__(self, port, baudrate) -> None:
        self.port = port
        self.serial = serial.Serial(port=port, baudrate=baudrate, timeout=SHORT_WAIT_TIME)
        self.try_connection()

    def close_port(self) -> None:
        self.port.close()

    def open_port(self) -> None:
        self.port.close() # por las dudas
        self.port.open()

    def try_connection(self):
        res = self.read_serial_until()
        print("Initial Read", res)
        self.write_serial('S')
        time.sleep(5*SHORT_WAIT_TIME)
        res = self.read_serial()
        print(res)
        self.connected = False
        if res == 'OK\n':
            self.connected = True
            print("Logger connected!")
        else:
            self.connected = False
            print("Logger not connected")
        return self.connected

    def read_serial_and_print(self, wait_time: float = SHORT_WAIT_TIME, print_description: str = ""):
        res = self.read_serial(wait_time)
        print(print_description, res)
        return res

    def write_serial(self, text):
        self.serial.write(text.encode())
        return
    
    def read_serial_until(self, last_line:bytes = b'OK\n', timeout:float = SHORT_WAIT_TIME, first_timeout:float = -1):
        data = []
        if first_timeout == -1:
            first_timeout = timeout
        self.serial.timeout = first_timeout
        data.append(self.serial.readline())
        self.serial.timeout = timeout
        while data[-1] != last_line and data[-1] != b'':
            data.append(self.serial.readline())
        return data

    def read_serial(self,timeout: float = SHORT_WAIT_TIME):
        self.serial.timeout = timeout
        data = self.serial.readline()
        return data.decode()   
    
    def get_connection(self):
        return self.connected
    
    def get_logger_state(self):
        self.write_serial('F')
        time.sleep(SHORT_WAIT_TIME)
        return int(self.read_serial_and_print(print_description= "Logger State:"))
    
    def get_logger_id(self):
        self.write_serial('I')
        time.sleep(SHORT_WAIT_TIME)
        return int(self.read_serial_and_print(print_description= "Logger ID:"))
    
    def get_penguin_name(self):
        self.write_serial('N')
        time.sleep(SHORT_WAIT_TIME)
        return self.read_serial_and_print(print_description= "Penguin Name:")[:-1]

    def set_penguin_name(self, penguin_name):
        self.write_serial('n' + penguin_name + ']')
        time.sleep(SHORT_WAIT_TIME)
        return self.read_serial_and_print(print_description= "Name changed:")

    def get_wetdry_freq(self):
        self.write_serial('W')
        time.sleep(SHORT_WAIT_TIME)
        try: 
            r = int(self.read_serial_and_print(print_description= "WetDry frequency:"))
        except ValueError:
            r = int(self.read_serial_and_print(print_description= "WetDry frequency:"))
        return r

    def set_wetdry_freq(self, wetdry_f):
        self.write_serial('w' + str(wetdry_f) + ']')
        return self.read_serial_and_print(print_description= "WetDry frequency changed:")
    
    def get_temp_freq(self):
        self.write_serial('T')
        time.sleep(SHORT_WAIT_TIME)
        return int(self.read_serial_and_print(print_description= "Temperature frequency:"))
    
    def set_temp_freq(self, temp_f):
        self.write_serial('t' + str(temp_f) + ']')
        return self.read_serial_and_print(print_description= "Temperature frequency changed:")
    
    def get_calibrated_min(self):
        self.write_serial('C')
        time.sleep(SHORT_WAIT_TIME)
        return int(self.read_serial_and_print(print_description= "Calibrated minute:"))

    def set_calibrated_min(self, calmin):
        self.write_serial('c' + str(calmin) + ']')
        return self.read_serial_and_print(print_description= "Calibrated minute changed:")
    
    def autocalibrate_min(self):
        self.write_serial('k')
        print("Autocalibrating minute...")
        time.sleep(60)
        return self.read_serial_and_print(print_description= "Autocalibrated minute:")

    def get_activation_time(self):
        self.write_serial('A')
        # self.write_serial('N')
        time.sleep(SHORT_WAIT_TIME)
        return self.read_serial_and_print(print_description= "Activation time:")[:-1]
    
    def set_activation_time(self, acttime):
        self.write_serial('a'+ acttime + ']')
        # self.write_serial('n' + acttime + ']')
        time.sleep(SHORT_WAIT_TIME)
        self.read_serial(SHORT_WAIT_TIME)
        self.read_serial(SHORT_WAIT_TIME)
        return self.read_serial_and_print(print_description= "Activation time changed:")

    def get_30C_value(self):
        self.write_serial('3')
        time.sleep(SHORT_WAIT_TIME)
        try: 
            r = int(self.read_serial_and_print(print_description= "Calibration value at 30°C:"))
        except ValueError:
            r = int(self.read_serial_and_print(print_description= "Calibration value at 30°C:"))
        return r

    def get_105C_value(self):
        self.write_serial('1')
        time.sleep(SHORT_WAIT_TIME)
        return int(self.read_serial_and_print(print_description= "Calibration value at 105°C:"))
    
    def get_mode(self):
        self.write_serial('M')
        time.sleep(SHORT_WAIT_TIME)
        try:
            res = self.read_serial_and_print(print_description= "Mode:")
        except ValueError:
            res = self.read_serial_and_print(print_description= "Mode:")
        return res
    
    def set_mode(self, mode):
        self.write_serial('m' + str(mode))
        return self.read_serial_and_print(print_description= "Mode changed:")

    def quit_logger(self):
        #time.sleep(SHORT_WAIT_TIME)
        self.write_serial('Q')
        time.sleep(MEDIUM_WAIT_TIME)
        return self.read_serial_and_print(print_description= "Quit")

    def deactivate_logger(self):
        pass
        # self.write_serial('q')
        # return self.read_serial_and_print(print_description= "Deactivated:")

    def toggle_led(self):
        self.write_serial('L')
        return self.read_serial_and_print(print_description= "Toggled LED")

    def get_field_data(self): #corroborar que tome TODAS las lineas
        self.write_serial('D')
        self.serial.timeout = SHORT_WAIT_TIME
        last_line = b'OK'
        data = []
        init = time.time()
        while (time.time() - init) < 3*60:
            r = self.serial.readline()
            if r != b'':
                data.append(r)
            if data[-1] == last_line:
                break
        # res = self.read_serial_until(timeout=SHORT_WAIT_TIME, first_timeout=LONG_WAIT_TIME)
        # for line_number, line in enumerate(data):
        #     print(line_number, line)
        r = b''.join(data[1:])
        return r
    
    def get_events(self): #corroborar que tome TODAS las lineas
        self.write_serial('E')
        #return self.read_serial_and_print(print_description= "Events:", wait_time= MEDIUM_WAIT_TIME)
        time.sleep(SHORT_WAIT_TIME)
        res = self.read_serial_until(timeout=SHORT_WAIT_TIME, first_timeout=MEDIUM_WAIT_TIME)
        for line_number, line in enumerate(res):
            print(line_number, line.decode())
        return res

    def get_indices(self):
        self.write_serial('X')
        time.sleep(SHORT_WAIT_TIME)
        res = self.read_serial()
        print("Indices:", res)
        return [int(i) for i in res[:-1].split('\t')]

def check_presence(event: threading.Event, interval: float=0.1):
    while True:
        if event.is_set():
            if find_port() == '':
                event.clear()
        time.sleep(interval)

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
    
# import serial.tools.list_ports

# ports = list(serial.tools.list_ports.comports())
# for p in ports:
#     print(p.hwid)

# port = Serial('COM3', 115200)

# while True:
#     data = port.read_serial()
#     print(data)
#     cs = input("Enter a control sequence: ")
#     port.write_serial(cs)


