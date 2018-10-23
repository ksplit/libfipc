import csv
import numpy
import os
import re

from script.config import Config

p = re.compile('[0-9]+')

class CSV:

    @classmethod
    def checkDirectory(cls, test_directory, csv_file_name):
        lock_dic = {}
        finish = 0

        # Parse the files
        print('Parsing all data from result...')
        for root, dirs, files in os.walk(test_directory):
            if dirs:
                finish = 1
                for directory in dirs:
                    directory = os.path.join(root, directory)
                    if os.path.isdir(directory):
                        for sub_root, sub_dirs, sub_files in os.walk(directory):
                            lock_dic = cls.parsing(sub_root, sub_files, csv_file_name, lock_dic)
            else:
                if finish == 0:
                    lock_dic = cls.parsing(root, files, csv_file_name, lock_dic)

        return lock_dic

    @classmethod
    def makeAllDict(cls, lock_dic):
        queue_dic = {} 
        mpmc_list = []
        mpsc_list = []
        spsc_list = []

        for keys in lock_dic.keys():
            new_dict = {}
            for idx in range(0,len(Config.queue_value)):
                if keys.split("-")[0] == Config.queue_value[idx]:
                    new_dict[keys] = dict(lock_dic[keys].items())
                    if idx == 0:
                        mpmc_list.append(new_dict)
                    elif idx == 1:
                        mpsc_list.append(new_dict)
                    else:
                        spsc_list.append(new_dict)
                    break

        queue_dic["mpmc"] = mpmc_list
        queue_dic["mpsc"] = mpsc_list
        queue_dic["spsc"] = spsc_list
        return queue_dic

    @classmethod
    def parsing(cls, root, files, csvFName, lock_dic):
        p_list = {}
        c_list = {}
        number_dic = {}
        cntCheck = 0
        transCnt = 0

        split_root = str(root).split('/')
        for r in split_root:
            if "queue" in r:
                queue_case = r.split("_")[1]
                lock_case = r.split("_")[2]

        resultFName = csvFName + "/" + queue_case + "_" + lock_case
        f = open(resultFName+'.csv', 'a')
        wr = csv.writer(f)

        for fname in files:
            if "queue" not in fname:
                continue

            bench, p_cnt, c_cnt,try_cnt,opt = fname.split('-')
            if any(p_cnt == x for x in p_list) == False:
                p_list[p_cnt] = []
                c_list[c_cnt] = []

            f = open(root+"/"+fname,'r')
            l = f.readlines()
            i = 0
            while i < len(l):
                line = l[i]
                i+=1
                if 'finished' in line:
                    if 'Producer' in line:
                        m = p.findall(line)
                        p_list[p_cnt].append(int(m[2]))
                        if cntCheck == 0:
                            transCnt = int(m[1])
                            cntCheck = 1
                    if 'Consumer' in line:
                        m = p.findall(line)
                        c_list[c_cnt].append(int(m[2]))

        key = list(p_list.keys())
        wr.writerow([bench, transCnt])
        wr.writerow(['','avg','std','avg','std'])
        wr.writerow(['cnt','prod','prod','cons','cons'])
        
        for i in range(0,len(key)):
            if p_list[key[i]]==[]:
                continue

            temp_dic = {}
            temp_str = queue_case+"-"+lock_case+"-"+key[i]

            p_avg=numpy.mean(p_list[key[i]])
            p_std=numpy.std(p_list[key[i]])
            c_avg=numpy.mean(c_list[key[i]])
            c_std=numpy.std(c_list[key[i]])

            temp_dic[temp_str+"-"+Config.action_plot_value[0]] = p_avg
            temp_dic[temp_str+"-"+Config.action_plot_value[1]] = p_std
            temp_dic[temp_str+"-"+Config.action_plot_value[2]] = c_avg
            temp_dic[temp_str+"-"+Config.action_plot_value[3]] = c_std

            number_dic[temp_str] = temp_dic
            wr.writerow([key[i],p_avg,p_std,c_avg,c_std])


        for keys in number_dic.keys():
            if keys.split("-")[0] == queue_case and keys.split("-")[1] == lock_case:
                lock_dic[queue_case+"-"+lock_case] = number_dic
        
        f.close()
        return lock_dic