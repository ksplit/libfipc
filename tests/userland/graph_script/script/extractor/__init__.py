import csv
import numpy
import os
import re
import json

from script.config import Config

p = re.compile('[0-9]+')

class Extractor:

	@classmethod
	def __new__(cls, self, directory, date, hyper_option):
		root_list, file_list = cls.checkValidDirectory(directory)
		all_data = cls.parsingDirectory(root_list, file_list, hyper_option)
		file_name = cls.makeResultFileName(directory, date)
		return cls.extract(all_data, file_name, hyper_option)

	@classmethod
	def checkValidDirectory(cls, test_directory):

		'''
		Verifying that the directory contains result files.
		'''

		root_list = []
		file_list = []
		finish = 0

		for root, dirs, files in os.walk(test_directory):
			if dirs:
				finish = 1
				for directory in dirs:
					directory = os.path.join(root, directory)
					if os.path.isdir(directory):
						for sub_root, sub_dirs, sub_files in os.walk(directory):
							root_list.append(sub_root)
							file_list.append(sub_files)
			else:
				if finish == 0:
					root_list.append(root)
					file_list.append(files)

		return root_list, file_list

	@classmethod
	def parsingDirectory(cls, root_list, file_list, hyper_option):

		'''
		Make dictionary which contains all data based on queues and locks.
		'''


		all_data = [[[[ {} for test in range(Config.test)]
						for number in range(Config.numbers[hyper_option])] 
						for lock in range(len(Config.lock_value))] 
						for queue in range(len(Config.queue_value))]
		
		for index in range(len(root_list)):
			root = root_list[index]
			files = file_list[index]

			split_root = str(root).split("/")
			for r in split_root:
				if "queue" in r:
					queue_case = r.split("_")[1]
					lock_case = r.split("_")[2]
					queue_idx = Config.queue_value.index(queue_case)
					lock_idx = Config.lock_value.index(lock_case)

			for fname in files:
				if "queue" not in fname:
					continue

				_, p_cnt, c_cnt, trans_cnt, opt = fname.split("-")
				number_case = int(p_cnt)
                
				f = open(root+"/"+fname,'r')
				l = f.readlines()
				i = 0

				producer_cycle_list = []
				consumer_cycle_list = []

				while i < len(l):
					line = l[i]
					m = p.findall(line)
					i += 1

					# Checking the test number and initialize each information dictionary
					if "try" in line:						
						test_case = int(m[0])+1
						
						global_time = 0
						producer_pin_dict = {}
						consumer_pin_dict = {}
						producer_start_time_dict = {}
						producer_end_time_dict = {}
						consumer_start_time_dict = {}
						consumer_end_time_dict = {}

					# Checking the tid of producers and consumers
					elif "tid" in line:
						if "Producer" in line:
							producer_pin_dict[int(m[0])] = int(m[1])
						if "Consumer" in line:
							consumer_pin_dict[int(m[0])] = int(m[1])
                    
					# Checking the time of producers,consumers and global
					elif "/" in line:
						if "Producer" in line:
							producer_start_time_dict[int(m[0])] = int(m[1])
							producer_end_time_dict[int(m[0])] = int(m[2])
						if "Consumer" in line:
							consumer_start_time_dict[int(m[0])] = int(m[1])
							consumer_end_time_dict[int(m[0])] = int(m[2])
						if "Global" in line:
							global_time = int(m[1]) - int(m[0])

					# Test finished 
					elif "finished" in line:
						if "Producer" in line:
							producer_cycle_list.append(int(m[2]))
						if "Consumer" in line:
							consumer_cycle_list.append(int(m[2]))
						else:
							all_data[queue_idx][lock_idx][number_case][test_case][Config.information_value[1]] = global_time
							all_data[queue_idx][lock_idx][number_case][test_case][Config.information_value[2]] = producer_start_time_dict
							all_data[queue_idx][lock_idx][number_case][test_case][Config.information_value[3]] = producer_end_time_dict
							all_data[queue_idx][lock_idx][number_case][test_case][Config.information_value[4]] = consumer_start_time_dict
							all_data[queue_idx][lock_idx][number_case][test_case][Config.information_value[5]] = consumer_end_time_dict
							all_data[queue_idx][lock_idx][number_case][test_case][Config.information_value[6]] = producer_pin_dict
							all_data[queue_idx][lock_idx][number_case][test_case][Config.information_value[7]] = consumer_pin_dict

				# While loop end...
                
				# Each result file has only one action_plot_value so put the data in the 0th of the test case
				action_plot_dic = {}

				action_plot_dic["transaction_count"] = trans_cnt
				action_plot_dic[Config.action_plot_value[0]] = numpy.mean(producer_cycle_list)
				action_plot_dic[Config.action_plot_value[1]] = numpy.std(producer_cycle_list)
				action_plot_dic[Config.action_plot_value[2]] = numpy.mean(consumer_cycle_list)
				action_plot_dic[Config.action_plot_value[3]] = numpy.std(consumer_cycle_list)
				
				all_data[queue_idx][lock_idx][number_case][0][Config.information_value[0]] = action_plot_dic

		return all_data
