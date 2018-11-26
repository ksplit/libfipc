import csv
import numpy
import os
import json
import sys

from script.config import Config
from script.extractor import Extractor

class TopologyDataExtractor(Extractor):

    @classmethod
    def extract(cls, files, hyper_option):
        topology_data = [[[[0 for number in range(Config.numbers[hyper_option])] 
                    for action_plot in range(len(Config.action_plot_value))] 
                    for lock in range(len(Config.lock_value))] 
                    for queue in range(len(Config.queue_value))]

        for file in files:
            f = open(file, mode='r')
            reader = csv.reader(f)

            file_info_list = file.split('/')
            queue_lock_info = file_info_list[len(file_info_list)-1]
            
            queue, lock = queue_lock_info.split('.')[0].split('-')

            queue_idx = Config.queue_value.index(queue)
            lock_idx = Config.lock_value.index(lock)

            cursor = sys.maxsize
            for read in reader:
                if "p_avg" in read:
                    cursor = reader.line_num
                
                if reader.line_num > cursor:
                    number = int(read[0])
                    producer_avg = float(read[2])
                    producer_std = float(read[3])
                    consumer_avg = float(read[4])
                    consumer_std = float(read[5])

                    topology_data[queue_idx][lock_idx][0][number] = producer_avg
                    topology_data[queue_idx][lock_idx][1][number] = producer_std
                    topology_data[queue_idx][lock_idx][2][number] = consumer_avg
                    topology_data[queue_idx][lock_idx][3][number] = consumer_std

            f.close() 
        
        return topology_data  