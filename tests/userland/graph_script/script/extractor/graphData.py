import numpy
import os
import json

from script.config import Config
from script.extractor import Extractor

class GraphDataExtractor(Extractor):

    @classmethod
    def makeResultFileName(cls, date):
        graph_file_name = "./graph/%s/graph" % date
        if not os.path.isdir(graph_file_name):
            os.makedirs(graph_file_name)
        return graph_file_name

    @classmethod
    def extract(cls, objs, file_name):
        graph_data = [[[[0 for number in range(Config.numbers)] 
                    for action_plot in range(len(Config.action_plot_value))] 
                    for lock in range(len(Config.lock_value))] 
                    for queue in range(len(Config.queue_value))]

        for queue in Config.queue_value:
            for lock in Config.lock_value:
                queue_idx = Config.queue_value.index(queue)
                lock_idx = Config.lock_value.index(lock)
                
                for number in range(Config.numbers):
                    if number == 0:
                        continue
                    
                    try:
                        graph_data[queue_idx][lock_idx][0][number] = objs[queue_idx][lock_idx][number][0][Config.information_value[0]][Config.action_plot_value[0]]
                        graph_data[queue_idx][lock_idx][1][number] = objs[queue_idx][lock_idx][number][0][Config.information_value[0]][Config.action_plot_value[1]]
                        graph_data[queue_idx][lock_idx][2][number] = objs[queue_idx][lock_idx][number][0][Config.information_value[0]][Config.action_plot_value[2]]
                        graph_data[queue_idx][lock_idx][3][number] = objs[queue_idx][lock_idx][number][0][Config.information_value[0]][Config.action_plot_value[3]]
                    except:
                        continue
        
        return graph_data, file_name
