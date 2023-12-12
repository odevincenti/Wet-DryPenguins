import matplotlib.pyplot as plt
from datetime import datetime
import numpy as np
import re

def extract_numbers_from_txt(file_path):
    with open(file_path, 'r') as file:
        content = file.read()

    # Use regular expression to find all numbers
    numbers = re.findall(r'\b\d+\b', content)

    # Convert the list of strings to a list of integers
    numbers = list(map(int, numbers))

    return numbers

def get_bits(byte):
    # Ensure the input is a valid byte (0 to 255)
    if not (0 <= byte <= 255):
        raise ValueError("Input must be a byte (0 to 255)")

    # Convert the byte to a binary string and remove the '0b' prefix
    binary_string = bin(byte)[2:]

    # Pad the binary string to 8 bits if needed
    padded_binary_string = binary_string.zfill(8)

    # Convert the binary string to a list of integers
    bits = [int(bit) for bit in padded_binary_string]

    return bits

def two_bytes_to_int(byte_list):
    # Ensure that there are two integers in the list
    if len(byte_list) != 2 or not all(isinstance(byte, int) for byte in byte_list):
        raise ValueError("Input should be a list of two integers representing bytes")

    # Combine the two bytes to form a 16-bit integer
    decimal_value = (byte_list[0] << 8) | byte_list[1]

    return decimal_value

# Function to convert a byte to a list of bits
def get_bits(byte):
    return [int(bit) for bit in bin(byte)[2:].zfill(8)]

def get_cal30(id):
    if id == 6:
        return 2183
    elif id == 23:
        return 2207
    else:
        print("ERROR")
    
def get_cal105(id):
    if id == 6:
        return 2755
    elif id == 23:
        return 2779
    else:
        print("ERROR")

def decode_measurements(raw_data, wet_dry_index, temp_index, activation_datetime, deactivation_datetime, CAL_30C=2207, CAL_105C=2779):

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
    activation_datetime = datetime.strptime(activation_datetime, "%Y-%m-%d %H:%M:%S")
    deactivation_datetime = datetime.strptime(deactivation_datetime, "%Y-%m-%d %H:%M:%S")

    # Calculate the time difference between activation and deactivation
    time_difference = deactivation_datetime - activation_datetime

    # Calculate the time step between each datetime
    time_step = time_difference / (len(temp) - 1)
    print(time_step)
    #time_step = time_step.replace(second= round(time_step.second))
    print(time_step)
    # Generate the list of datetimes
    datetime_list = [activation_datetime + i * time_step for i in range(len(wet_dry_bits))]
    datetime_list = [dt.replace(microsecond=0) for dt in datetime_list]
    # Return the results
    return list(zip(datetime_list, wet_dry_bits, temp))

def plot_data(data_list, color1='blue', color2='red', linestyle1=None, linestyle2=None):
    # Extracting columns from the list of lists
    datetime_values = [row[0] for row in data_list]
    data1 = [row[1] for row in data_list]
    data2 = [row[2] for row in data_list]

    # Create a single figure with two subplots
    # fig, axs = plt.subplots(2, 1, figsize=(16, 8), sharex=True)
    fig, axs = plt.subplots(1, 1, figsize=(12, 6))

    # Plotting the data on each subplot
    # axs[0].plot(datetime_values, data1, color=color1, linestyle=linestyle1)
    axs.plot(datetime_values, data2, color=color2, linestyle=linestyle2)
    axs.plot(datetime_values, run*np.ones_like(datetime_values), color="blue")

    # Formatting x-axis
    date_strings = [date.strftime('%Y-%m-%d %H:%M:%S') for date in set(datetime_values)]
    # Remove duplicates by converting to a set and back to a list
    unique_dates = list(set(date_strings))
    # Sort the list of unique dates
    sorted_unique_dates = sorted(unique_dates)
    # Convert strings to datetime objects
    date_objects = [datetime.strptime(date_str, '%Y-%m-%d %H:%M:%S') for date_str in sorted_unique_dates]
    axs.set_xticks(date_objects)
    axs.set_xticklabels(sorted_unique_dates, rotation=45, ha='right')
    axs.tick_params(axis='x', labelsize=7)

    # Adding titles and labels
    fig.suptitle('Measurements of Device ' + str(id))
    axs.set_ylabel('Temperature [°C]')
    axs.set_xlabel('Date')

    # Adding legends
    # axs[0].legend()
    axs.grid()
    # axs[1].legend()
    # Display the plot
    plt.savefig("Prueba " + str(run) + "C\\PRUEBA_TEMPERATURA_" + str(id) + "_T" + str(run) + "C.png")
    plt.show()

# Example usage:
id = 23
run = 8
wet_dry_index = 34
temp_index = 34
activation_time = "2023-12-11 07:33:13"
deactivation_time = "2023-12-11 08:07:27"
file_name = "Prueba " + str(run) + "C\\output_from_msp_" + str(id) + "_T" + str(run) + "C_data.txt"
raw_data = extract_numbers_from_txt(file_name)
# print("raw_data", raw_data)

result = decode_measurements(raw_data, wet_dry_index, temp_index, activation_time, deactivation_time, CAL_30C=get_cal30(id), CAL_105C=get_cal105(id))

import csv

# Combine the lists into a list of lists
data = list(zip([id]*len(result), [run]*len(result), [row[0] for row in result], [0]*len(result), [row[2] for row in result]))

# Specify the CSV file path
csv_file_path = "Prueba " + str(run) + "C\\PRUEBA_TEMPERATURA_" + str(id) + "_" + str(run) + "C.csv"

# Write data to the CSV file
with open(csv_file_path, 'w', newline='') as csvfile:
    csv_writer = csv.writer(csvfile)
    
    # Write header if needed
    csv_writer.writerow(['ID', 'Run', 'Timestamp', 'WetDry', 'Temperature'])
    
    # Write the data
    csv_writer.writerows(data)

print(f'Data has been written to {csv_file_path}')

plot_data(result)
