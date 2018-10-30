import csv
import numpy
import os
import json

from script.config import Config
from script.extractor import Extractor

class CSVExtractor(Extractor):

    @classmethod
    def makeResultFileName(cls, date):
        file_name = "./csv/%s" % date
        if not os.path.isdir(file_name):
            os.makedirs(file_name)

        return file_name

    @classmethod
    def extract(cls, objs, file):
        for queue in Config.queue_value:
            for lock in Config.lock_value:
                queue_idx = Config.queue_value.index(queue)
                lock_idx = Config.lock_value.index(lock)
                
                bench = queue+"-"+lock
                result_file_name = file + "/" + bench

                try:
                    transaction_count = objs[queue_idx][lock_idx][1][0][Config.information_value[0]]["transaction_count"]
                except:
                    continue

                f = open(result_file_name+'.csv', 'a')
                wr = csv.writer(f)

                wr.writerow([bench, transaction_count])
                wr.writerow(['','avg','std','avg','std'])
                wr.writerow(['cnt','prod','prod','cons','cons'])
                
                for number in range(1, Config.numbers+1):
                    try:
                        producer_avg = objs[queue_idx][lock_idx][number][0][Config.information_value[0]][Config.action_plot_value[0]]
                        producer_std = objs[queue_idx][lock_idx][number][0][Config.information_value[0]][Config.action_plot_value[1]]
                        consumer_avg = objs[queue_idx][lock_idx][number][0][Config.information_value[0]][Config.action_plot_value[2]]
                        consumer_std = objs[queue_idx][lock_idx][number][0][Config.information_value[0]][Config.action_plot_value[3]]

                        wr.writerow([number,producer_avg,producer_std,consumer_avg,consumer_std])
                    except:
                        continue

                f.close()
