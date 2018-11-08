import os
import json

from script.config import Config
from script.extractor import Extractor

class TimeDataExtractor(Extractor):

    @classmethod
    def makeResultFileName(cls, directory, date):
        file_name = "./graph/%s/time" % ( directory.split('/')[1]+'-'+date )
        if not os.path.isdir(file_name):
            os.makedirs(file_name)

        return file_name

    @classmethod
    def extract(cls, objs, file_name, hyper_option):
        time_data = [[[ {}  for number in range(Config.numbers[hyper_option])] 
                    for lock in range(len(Config.lock_value))] 
                    for queue in range(len(Config.queue_value))]

        for queue in Config.queue_value:
            for lock in Config.lock_value:
                queue_idx = Config.queue_value.index(queue)
                lock_idx = Config.lock_value.index(lock)
                
                for number in range(Config.numbers[hyper_option]):
                    if number == 0:
                        continue
    
                    try:
                        time_data[queue_idx][lock_idx][number][Config.information_value[1]] = objs[queue_idx][lock_idx][number][1][Config.information_value[1]]
                        time_data[queue_idx][lock_idx][number][Config.information_value[2]] = objs[queue_idx][lock_idx][number][1][Config.information_value[2]]
                        time_data[queue_idx][lock_idx][number][Config.information_value[3]] = objs[queue_idx][lock_idx][number][1][Config.information_value[3]]
                        time_data[queue_idx][lock_idx][number][Config.information_value[4]] = objs[queue_idx][lock_idx][number][1][Config.information_value[4]]
                        time_data[queue_idx][lock_idx][number][Config.information_value[5]] = objs[queue_idx][lock_idx][number][1][Config.information_value[5]]
                    except:
                        continue
                    
        return time_data, file_name
