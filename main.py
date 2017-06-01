
#!/usr/bin/env python3
# -*- coding: utf-8 -*-
'''
	Network architecture II -> Tier 1 -> Builder
	Luis Félix Rodríguez Cano, Álvaro Molinero Fernandez and Alberto García Merino
	
	USAGE: python3 main.py '{"datagram_length": 1052, "MTUs": [1500, 712, 400, 1500]}'
	
	Execute in this web shell https://www.tutorialspoint.com/execute_python3_online.php
	First copy and paste all the code and go to file and save file on the top right corner
	Then write python3 main.py '{"datagram_length": 1052, "MTUs": [1500, 712, 400, 1500]}' on the shell
	
	IMPORTANT NOTE!!!
		'The "{}" and "[]" MAY have to be entered by their decimal code 
		(sometimes web shell does not recognize these keys)
		Alt+123 will write "{"
		Alt+125 will write "}"
		Alt+91  will write "["
		Alt+93  will write "]"
'''


import json
import sys

class Datagram():

	fragments = []								 # fragments[link number][fragment number]

	def __init__(self, raw_string):
			self.in_string = raw_string[1:len(raw_string)]					# Remove program name
			self.json_datagram = json.loads(' '.join(self.in_string))		# Convert string to JSON

	def compute_FTW(self):
		input = [self.json_datagram]										# Input for each iteration

		for link in range(0,len((self.json_datagram["MTUs"]))):				# Cycle through links
			self.fragments.append([])
			for each_packet in input:
				iteration_result = self.__single_link(self.json_datagram["MTUs"][link], each_packet)
				for dic in iteration_result:
					self.fragments[link].append(dic)
				self.__assign_offsets(link)

			input = self.fragments[len(self.fragments)-1]

	def __assign_offsets(self, link_id):
		offset = 0
		for dic in self.fragments[link_id]:
			dic["offset"] = offset
			offset = offset + (dic["datagram_length"]-20)//8
			dic["MF"] = 1
		self.fragments[link_id][len(self.fragments[link_id])-1]["MF"] = 0

	def __single_link(self, mtu, packet):

		outdict = []												# return outdict
		
		if mtu < packet["datagram_length"]:							# Fragment
			data_len = packet["datagram_length"] - 20				# Note: 20 is header size
			offset_mult = (mtu-20)//8								# Offset will be multiple of this number
			
			k = 0
			while data_len > mtu:									# For each fragment
				outdict.append({"datagram_length": 0, "MF": 0, "offset": 0})
				outdict[k]["datagram_length"] = offset_mult*8 + 20
				data_len = data_len - offset_mult*8
				k = k + 1

			if data_len > 0:
				outdict.append({"datagram_length": 0, "MF": 0, "offset": 0})
				outdict[k]["datagram_length"] = data_len+20

			return outdict

		else:
			outdict.append({"datagram_length": 0, "MF": 0, "offset": 0})
			outdict[0]["datagram_length"] =	packet["datagram_length"]
			
			return outdict

	def print_fragments(self):
		print("{", end = '')
		for k in range(0,len(self.fragments)-1):
			print("\"" + str(self.json_datagram["MTUs"][k]) + "\": "+ str(self.fragments[k]) + ",")

		print("\"" + str(self.json_datagram["MTUs"][-1]) + "\": "+ str(self.fragments[-1]) + "}")

def main():
	if len(sys.argv) > 1:
		input_datagram = Datagram(sys.argv)					# Create datagram object
		input_datagram.compute_FTW();						# Compute fragments and MFs
		input_datagram.print_fragments();					# Output on screen

if __name__ == '__main__': # NOTE: sys.argv contains input
	main()