import matplotlib.pyplot as plt
from datetime import datetime
from datetime import datetime
import re

def extract_numbers_from_txt(file_path):
    with open(file_path, 'r') as file:
        content = file.read()

    # Use regular expression to find all numbers
    numbers = re.findall(r'\b\d+\b', content)

    # Convert the list of strings to a list of integers
    numbers = list(map(int, numbers))

    return numbers

import mmap
import re

def extract_index(filepath):

    # Regular expressions to find the desired lines
    pattern_temperature = re.compile(rb'temperature_index\s*=\s*(\d+)')
    pattern_wetdry = re.compile(rb'wet_and_dry_index\s*=\s*(\d+)')

    # Open the file in read mode with mmap
    with open(filepath, 'rb') as bin_file:
        # Create a memory-mapped file
        with mmap.mmap(bin_file.fileno(), 0, access=mmap.ACCESS_READ) as mapped_file:
            # Search for wetdry_index in the mapping
            match_wetdry = pattern_wetdry.search(mapped_file)
            if match_wetdry:
                # Extract the numeric value from the match
                number_wetdry = int(match_wetdry.group(1))
                
                # Print the result
                print(f"The value of wet_dry_index is: {number_wetdry}")

            # Search for temperature_index in the mapping
            match_temperature = pattern_temperature.search(mapped_file)
            if match_temperature:
                # Extract the numeric value from the match
                number_temperature = int(match_temperature.group(1))
                
                # Print the result
                print(f"The value of temperature_index is: {number_temperature}")

    return number_wetdry, number_temperature

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

def plot_data(data_list, file_name, color1='blue', color2='red', linestyle1=None, linestyle2=None):
    # Extracting columns from the list of lists
    datetime_values = [row[0] for row in data_list]
    data1 = [row[1] for row in data_list]
    data2 = [row[2] for row in data_list]

    # Create a single figure with two subplots
    fig, axs = plt.subplots(2, 1, figsize=(16, 8), sharex=True)

    # Plotting the data on each subplot
    axs[0].plot(datetime_values, data1, color=color1, linestyle=linestyle1)
    axs[1].plot(datetime_values, data2, color=color2, linestyle=linestyle2)

    # Formatting x-axis
    date_strings = [date.strftime('%Y-%m-%d %H:%M:%S') for date in set(datetime_values)]
    # Remove duplicates by converting to a set and back to a list
    unique_dates = list(set(date_strings))
    # Sort the list of unique dates
    sorted_unique_dates = sorted(unique_dates)
    # Convert strings to datetime objects
    date_objects = [datetime.strptime(date_str, '%Y-%m-%d %H:%M:%S') for date_str in sorted_unique_dates]
    axs[1].set_xticks(date_objects)
    axs[1].set_xticklabels(sorted_unique_dates, rotation=45, ha='right')
    axs[1].tick_params(axis='x', labelsize=7)

    # Adding titles and labels
    fig.suptitle('Measurements of Device ' + file_name[:-4])
    axs[1].set_ylabel('Temperature [C]')
    axs[0].set_ylabel('Wet-Dry Value')
    axs[1].set_xlabel('Date')

    # Adding legends
    # axs[0].legend()
    axs[1].grid()
    # axs[1].legend()
    axs[0].grid(axis='x')
    # Display the plot
    plt.show()
    plt.savefig("Prueba " + str(run) + "\\PRUEBA_TEMPERATURA_" + str(id) + "_" + str(run) + ".png")

# Example usage:
id = 23
run = 6
wet_dry_index, temp_index = extract_index("Prueba " + str(run) + "\\output_from_msp_PRUEBA_TEMPERATURA_OLI_" + str(id) + "_" + str(run) + ".txt")
file_name = "Prueba " + str(run) + "\\CROPPED_output_from_msp_PRUEBA_TEMPERATURA_OLI_" + str(id) + "_" + str(run) + ".txt"
raw_data = extract_numbers_from_txt(file_name)
# print("raw_data", raw_data)
activation_time = "2023-12-06 21:44:04"  # Replace with your timestamp
deactivation_time = "2023-12-06 22:02:03"  # Replace with your timestamp

result = decode_measurements(raw_data, wet_dry_index, temp_index, activation_time, deactivation_time, CAL_30C=get_cal30(id), CAL_105C=get_cal105(id))

import csv

# Combine the lists into a list of lists
data = list(zip([0]*len(result), [0]*len(result), [row[0] for row in result], [0]*len(result), [row[2] for row in result]))

# Specify the CSV file path
csv_file_path = "Prueba " + str(run) + "\\PRUEBA_TEMPERATURA_" + str(id) + "_" + str(run) + ".csv"

# Write data to the CSV file
with open(csv_file_path, 'w', newline='') as csvfile:
    csv_writer = csv.writer(csvfile)
    
    # Write header if needed
    csv_writer.writerow(['ID', 'Run', 'Timestamp', 'WetDry', 'Temperature'])
    
    # Write the data
    csv_writer.writerows(data)

print(f'Data has been written to {csv_file_path}')

plot_data(result, file_name)
