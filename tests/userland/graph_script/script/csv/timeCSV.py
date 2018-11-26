import csv
import numpy
import os
import json

from script.config import Config
from script.csv import CSV

class TimeCSV(CSV):

    @classmethod
    def makeCSVDirectoryName(cls, directory):
        directory_info = directory.split('/')

        machine_hyper = directory_info[1]
        policy = directory_info[2]

        if "csv" not in directory:
            csv_directory = "./csv/%s/%s/time" % (machine_hyper, policy)
        else:
            csv_directory = "./%s/time" % (directory)

        if not os.path.isdir(csv_directory):
            os.makedirs(csv_directory)

        return csv_directory

    @classmethod
    def makeCSV(cls, objs, csv_directory, hyper_option):

        for queue in Config.queue_value:
            for lock in Config.lock_value:
                queue_idx = Config.queue_value.index(queue)
                lock_idx = Config.lock_value.index(lock)
                
                bench = queue+"-"+lock
                result_file_name = csv_directory + "/" + bench

                # Verifying if there are data
                try:
                    transaction_count = objs[queue_idx][lock_idx][1][0][Config.information_value[0]]
                except:
                    continue

                f = open(result_file_name+'.csv', 'a')
                wr = csv.writer(f)

                wr.writerow([bench, transaction_count])
                wr.writerow(['p_cnt', 'c_cnt', 'test_number','producer/consumer','start/end/global', 'time'])
                
                for number in range(1, Config.numbers[hyper_option]):
                    for test in range(1, Config.test):
                        try:

                            producer_start_list = objs[queue_idx][lock_idx][number][test][Config.information_value[3]]
                            producer_end_list = objs[queue_idx][lock_idx][number][test][Config.information_value[4]]
                            consumer_start_list = objs[queue_idx][lock_idx][number][test][Config.information_value[5]]
                            consumer_end_list = objs[queue_idx][lock_idx][number][test][Config.information_value[6]]
                            global_time = objs[queue_idx][lock_idx][number][test][Config.information_value[7]]

                            producer_start_row = [number, number, test, 'P', 'Start']
                            for producer_start in producer_start_list:
                                producer_start_row.append(producer_start)

                            producer_end_row = [number, number, test, 'P', 'End']
                            for producer_end in producer_end_list:
                                producer_end_row.append(producer_end)

                            consumer_start_row = [number, number, test, 'C', 'Start']
                            for consumer_start in consumer_start_list:
                                consumer_start_row.append(consumer_start)

                            consumer_end_row = [number, number, test, 'C', 'End']
                            for consumer_end in consumer_end_list:
                                consumer_end_row.append(consumer_end)

                            wr.writerow(producer_start_row)
                            wr.writerow(producer_end_row)
                            wr.writerow(consumer_start_row)
                            wr.writerow(consumer_end_row)
                            wr.writerow([number, number, test,'','Global', global_time])

                        except:
                            continue                

                f.close()

        return csv_directory
