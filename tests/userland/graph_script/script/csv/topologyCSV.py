import csv
import numpy
import os
import json

from script.config import Config
from script.csv import CSV

class TopologyCSV(CSV):

    @classmethod
    def makeCSVDirectoryName(cls, directory, date):
        csv_directory = "./csv/topology/%s" % ( directory.split('/')[1]+'-'+date )
        if not os.path.isdir(csv_directory):
            os.makedirs(csv_directory)

        return csv_directory

    @classmethod
    def makeCSV(cls, objs, csv_directory, hyper_option):
        csv_file_list = []
        
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
                wr.writerow(['p_cnt', 'c_cnt', 'test_number','producer/consumer','cycle'])
                
                producer_avg_list = []
                producer_std_list = []
                consumer_avg_list = []
                consumer_std_list = []

                for number in range(1, Config.numbers[hyper_option]):
                    producer_total_cycle_list = []
                    consumer_total_cycle_list = []
                    for test in range(1, Config.test):
                        try:

                            producer_cycle_list = objs[queue_idx][lock_idx][number][test][Config.information_value[1]]
                            consumer_cycle_list = objs[queue_idx][lock_idx][number][test][Config.information_value[2]]

                            producer_row = [number, number, test, 'P']
                            for producer in producer_cycle_list:
                                producer_row.append(producer)
                                producer_total_cycle_list.append(producer)
                            
                            consumer_row = [number, number, test, 'C']
                            for consumer in consumer_cycle_list:
                                consumer_row.append(consumer)
                                consumer_total_cycle_list.append(consumer)

                            wr.writerow(producer_row)
                            wr.writerow(consumer_row)
                        except:
                            continue

                    producer_avg_list.append(numpy.mean(producer_total_cycle_list)) 
                    producer_std_list.append(numpy.std(producer_total_cycle_list))
                    consumer_avg_list.append(numpy.mean(consumer_total_cycle_list))
                    consumer_std_list.append(numpy.mean(consumer_total_cycle_list))

                wr.writerow(['result'])
                wr.writerow(['p_cnt', 'c_cnt', 'p_avg', 'p_std', 'c_avg', 'c_std'])

                for number in range(1, Config.numbers[hyper_option]):
                    try:
                        wr.writerow([number, number, producer_avg_list[number-1], producer_std_list[number-1], 
                            consumer_avg_list[number-1], consumer_std_list[number-1]])
                    except:
                        continue

                csv_file_list.append(f)
                f.close()

        return csv_file_list
