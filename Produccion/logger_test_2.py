from datetime import datetime
import re
from serialcom import Serial, find_port

def decode_measurements(raw_data, wet_dry_index, temp_index, activation_datetime, deactivation_datetime, CAL_30C, CAL_105C):
    # Convert each byte in raw_data to bits
    bits_list = [bit for byte in raw_data for bit in get_bits(byte)]

    # Create a list with the first wet_dry_index bits
    wet_dry_bits = bits_list[:wet_dry_index]

    # Create a list with the last temp_index*2 bits read backwards
    temp_raw = raw_data[-temp_index*2:][::-1]
    temp = [two_bytes_to_int([a, b]) for a, b in zip(temp_raw[::2], temp_raw[1::2])]
    temp = [(30-CAL_30C*(105-30)/(CAL_105C-CAL_30C)) + t*(105-30)/(CAL_105C-CAL_30C) for t in temp]
    #temp = [ for t in temp]
    # Convert activation and deactivation datetimes to datetime objects
    # activation_datetime = datetime.strptime(activation_datetime, "%Y%m%d %H%M%S")
    # deactivation_datetime = datetime.strptime(deactivation_datetime,  "%Y%m%d %H%M%S")

    # Calculate the time difference between activation and deactivation
    time_difference = deactivation_datetime - activation_datetime

    # Calculate the time step between each datetime
    time_step = time_difference / (len(temp) - 1)
    # print(time_step)
    #time_step = time_step.replace(second= round(time_step.second))
    # print(time_step)
    # Generate the list of datetimes
    datetime_list = [activation_datetime + i * time_step for i in range(len(wet_dry_bits))]
    datetime_list = [dt.replace(microsecond=0) for dt in datetime_list]
    # Return the results
    return list(zip([datetime.strftime(i, "%Y%m%d %H%M%S") for i in datetime_list], wet_dry_bits, temp))

def two_bytes_to_int(byte_list):
    # Ensure that there are two integers in the list
    if len(byte_list) != 2 or not all(isinstance(byte, int) for byte in byte_list):
        raise ValueError("Input should be a list of two integers representing bytes")

    # Combine the two bytes to form a 16-bit integer
    decimal_value = (byte_list[0] << 8) | byte_list[1]

    return decimal_value

def get_bits(byte):
    return [int(bit) for bit in bin(byte)[2:].zfill(8)]

def parse_data_string(data_block):
    # Use regular expression to find all numbers
    data_string = data_block.decode()
    numbers = re.findall(r'\b\d+\b', data_string)
    # Convert the list of strings to a list of integers
    numbers = list(map(int, numbers))
    return numbers

def parse_data(data, index):
    raw_data = parse_data_string(data)
    rundata = decode_measurements(raw_data, index[0], index[1], activation_time, deactivation_time, cal30, cal105)
    return rundata

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
    deactivation_time = datetime.now()
    #deactivation_time = datetime.strftime(d, "%Y%m%d %H%M%S")
    activation_time = datetime.strptime(comms.get_activation_time(), "%Y%m%d %H%M%S")
    comms.toggle_led()                  # Toggled LED ON
    comms.get_mode()                    # Mode: 3
    comms.get_logger_id()               # Logger ID: id
    comms.get_penguin_name()            # Penguin Name: Pingu
    cal30 = comms.get_30C_value()
    cal105 = comms.get_105C_value() 
    index = comms.get_indices()         # Indices: nro      nro
    data = comms.get_field_data()       
    for i, row in enumerate(parse_data(data, index)):
        print(i, row)
    print("\n EVENTS\n")
    comms.get_events()                  
    comms.set_mode(0)                   # Mode changed: OK
    comms.toggle_led()                  # Toggled LED OFF
    comms.get_mode()                    # Mode: 0
    comms.quit_logger()                 # Quit
