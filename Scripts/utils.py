import numpy as np

R_base_values = [1, 1.2, 1.5, 1.8, 2.2, 2.7, 3.3, 3.9, 4.7, 5.1, 5.6, 6.8, 8.2]

C_values = np.array([10E-12, 15E-12, 22E-12, 33E-12, 47E-12, 100E-12, 120E-12, 130E-12, 150E-12, 180E-12, 220E-12, 330E-12, 470E-12, 560E-12, 680E-12, 750E-12, 820E-12, 
		     1E-9, 1.5E-9, 2E-9, 2.2E-9, 3.3E-9, 4.7E-9, 5E-9, 5.6E-9, 6.8E-9, 10E-9, 15E-9, 22E-9, 33E-9, 47E-9, 68E-9, 100E-9, 150E-9, 200E-9, 220E-9, 330E-9, 470E-9, 680E-9, 
			 1E-6, 1.5E-6, 2E-6, 2.2E-6, 3.3E-6, 4.7E-6, 5E-6, 5.6E-6, 6.8E-6, 10E-6, 22E-6, 47E-6, 100E-6, 220E-6, 330E-6, 470E-6])
R_values = (np.array([R_base_values[i]*10**j for j in range(6) for i in range(len(R_base_values))])).flatten()

def to_unit(num, sf=3):
	"""Round a number to the specified significant figures (default is 3) and add the corresponding unit prefix.

	Args:
		var (float): number to be formatted.
		sf (int, optional): Significant figures. Defaults to 3.

	Returns:
		string: formatted number and unit prefix.
	"""
	units = ['', 'k', 'M', 'G', 'T', 'p', 'n', 'u', 'm']
	e_count = 0
	value, e_count = get_exp(num)

	if e_count > 4 or e_count < -4:
		return '{:.{}g}'.format(num, sf-1).rstrip('0').rstrip('.')
	else:
		return np.format_float_positional(value, precision=sf, unique=True, fractional=False, trim='-') + units[e_count]
	
def get_exp(num):
	"""Normalize a number to its engineering notation.

	Args:
		num (float): number to be normalized.

	Returns:
		float: normalized number.
		int: exponent of 1000 that num was divided by.
	"""
	e_count = 0
	value = num
	while e_count <= 4 and e_count >= -4:
		if 1 <= value < 1E3:
			break
		if value >= 1E3:
			value /= 1E3
			e_count += 1
		if value < 1:
			value *= 1E3
			e_count -= 1
	return value, e_count

def get_unit_prefix(num):
	prefix = ['', 'k', 'M', 'G', 'T', 'p', 'n', 'u', 'm']
	x, e_count = get_exp(num)

	if e_count > 4:
		unit_prefix = prefix[4]
	elif e_count < -4:
		unit_prefix = prefix[4]
	else:
		unit_prefix = prefix[e_count]

	return unit_prefix

def parallel(R1, R2):
	R = 1/(1/R1 + 1/R2)
	return R
