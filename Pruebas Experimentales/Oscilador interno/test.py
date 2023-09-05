import matplotlib.pyplot as plt
from datetime import datetime

def format_data(N_hours):
    full_data = []
    for i in range(N_hours+1):
        filename = str(i) + ".txt"

        file = open(filename, 'r')

        rows = file.readlines()
        data = []
        for row in rows:
            new_data = row[0:len(row)-1].split("\t")
            new_data[0] = int(new_data[0])
            new_data[1] = datetime.fromisoformat(new_data[1][0:len(new_data[1])-1])
            new_data[2] = int(new_data[2])
            new_data[3] = float(new_data[3])
            new_data[4] = float(new_data[4])
            data.append(new_data)

        #for row in data:
        #print(len(data))
        full_data += data
        file.close()

    #full_data = full_data[0:len(full_data)]
    #print(len(full_data))
    #for i in range(44680, 44710, 1):
    #    print(full_data[i])
    return full_data

full_data = format_data(43)

#Analisis de presicion del clock

#Obtain all possible Vcc values with 0.1 difference
v_list = []

for data in full_data:
    v_high = max(data[3], data[4])
    v_high = float(str(v_high)[:3])
    if not v_high in v_list:
        v_list.append(v_high)
v_list.sort(reverse=True)
print("v_list = ", v_list)

#Get a list, each row is a list of ms period for that Voltage
nominal_period_by_v = [[None]]*len(v_list)
total_period_by_v = [[None]]*len(v_list)
outliers_by_v = [[None]]*len(v_list)
print(nominal_period_by_v)

for data in full_data:
    v_high = max(data[3], data[4])
    v_high = float(str(v_high)[:3])
    for index in range(len(v_list)):
        if abs(v_high-v_list[index]) < 0.01:
            total_period_by_v[index].append(data[2])
            #print(total_period_by_v[index])
            if abs(data[2] - 1000) < 20:                 #Admit a 2% deviation for standard measurments  
                nominal_period_by_v[index].append(data[2])
                #print(nominal_period_by_v[index])
            else:
                outliers_by_v[index].append(data[2])     #Otherwise send to outliers list.
                #print(outliers_by_v[index])

#print(nominal_period_by_v)

#Delete initial None value   
for i in range(len(total_period_by_v)):
    nominal_period_by_v[i] = nominal_period_by_v[i][1:]
    outliers_by_v[i] = outliers_by_v[i][1:]
    total_period_by_v[i] = total_period_by_v[i][1:]

for i in range(len(v_list)):
    print("V = ", v_list[i], "V")
    print("\ttotal_period_by_v = ", total_period_by_v[i])
    print("\tlen(nominal_period_by_v) = ", len(nominal_period_by_v[i]))
    print("\toutliers_by_v = ", outliers_by_v[i])
    print("\tlen(outliers_by_v) = ", len(outliers_by_v[i]))
    print("\tmax, min = ", max(nominal_period_by_v[i]), min(nominal_period_by_v[i]))
#plt.hist(total_period_by_v[1], 10000, density=False)
#plt.show()

