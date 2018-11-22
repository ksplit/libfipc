import csv
import os
import json

from script.config import Config
from script.extractor import Extractor

class TimeDataExtractor(Extractor):

    @classmethod
    def extract(cls, files, hyper_option):
        time_data = [[[ {}  for number in range(Config.numbers[hyper_option])] 
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

            number = 0
            for read in reader:
                try:
                
                    if Config.time_test_number == int(read[2]):
                        if "Global" in read:
                            global_time = int(read[5])
                            time_data[queue_idx][lock_idx][number][Config.information_value[7]] = global_time
                        else:
                            number = int(read[0])
                            time_list = [int(i) for i in read[5:]]
                            
                            # producer_start
                            if read[3] == 'P' and read[4] == 'Start':
                                time_data[queue_idx][lock_idx][number][Config.information_value[3]] = time_list
                            # producer_end
                            if read[3] == 'P' and read[4] == 'End':
                                time_data[queue_idx][lock_idx][number][Config.information_value[4]] = time_list
                            # consumer_start
                            if read[3] == 'C' and read[4] == 'Start':
                                time_data[queue_idx][lock_idx][number][Config.information_value[5]] = time_list
                            # consumer_end
                            if read[3] == 'C' and read[4] == 'End':
                                time_data[queue_idx][lock_idx][number][Config.information_value[6]] = time_list
                except:
                    continue

            f.close() 

        return time_data
