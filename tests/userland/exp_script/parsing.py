import os, sys, getopt
import csv, re, numpy
import json
import pandas as pd
import matplotlib as mpl
import matplotlib.pyplot as plt
from time import localtime, strftime
# list storing cycles/message

p = re.compile('[0-9]+')
queue_value = ['mpmc', 'mpsc', 'spsc']
lock_value = ['spinlock', 'ticketlock', 'mcslock']
action_plot_value = ['prod_avg', 'prod_std', 'cons_avg', 'cons_std']

queue_dic = {}
lock_dic = {}

mpmc_list = []
mpsc_list = []
spsc_list = []

# Usage
def usage():
    print('Usage: %s -t <testDir> -f <resultFName>' % sys.argv[0])

def drawGraph(graph, graph_name, queue_idx, numbers):
    graph_name = graph_name.upper()
    for lock in range(len(lock_value)):
        for action_plot in range(len(action_plot_value)):
            if action_plot % 2 == 1:
                continue

            if lock == 0:
                color = 'r'
            elif lock == 1:
                color = 'b'
            elif lock == 2:
                color = 'g'

            if action_plot == 0:
                linestyle = "-"
            elif action_plot == 2:
                linestyle = "--"

            marker = "."
            label = lock_value[lock]+"_"+action_plot_value[action_plot]
            fmt = color+marker+linestyle
            print(fmt)
            plt.errorbar(
                numbers, 
                graph[queue_idx][lock][action_plot], 
                yerr=graph[queue_idx][lock][action_plot+1],
                fmt=fmt,
                label=label,
                linewidth=1, # width of plot line
                elinewidth=0.5,# width of error bar line
                ecolor='k', # color of error bar
                capsize=5, # cap length for error bar
                capthick=0.5
            )

    plt.xlabel('Producer(Consumer) Num')
    plt.ylabel('Average of Cycle per message')
    plt.title(graph_name)
    plt.legend(prop={'size':15}, labelspacing=2)
    #plt.yscale('log')
    
    fig = plt.gcf()
    fig.set_size_inches(18.5, 10.5)
    #plt.show()
    fig.savefig("./"+graph_name+'.pdf')


def extractGraphData(result_dic):
    numbers = [i for i in range(1,33)]
    graph = [[[[0 for number in numbers] for action_plot in range(len(action_plot_value))] for lock in range(len(lock_value))] for queue in range(len(queue_value))]
    for queue_case in result_dic:
        graph_name = queue_case
        queue_idx = 0
        if result_dic[queue_case]:
            for lock_case in result_dic[queue_case]:
                lock_idx = 0
                for number_case in lock_case.values():
                    for value_case in number_case.values():
                        for plot_case in value_case:
                            category_list = plot_case.split("-")

                            queue_name = category_list[0]
                            lock_name = category_list[1]
                            number_name = category_list[2]
                            action_plot_name = category_list[3]
                      
                            action_plot_idx = 0

                            for idx in range(0,4):
                                if action_plot_value[idx] == action_plot_name:
                                    action_plot_idx = idx
                                if idx != 3:
                                    if lock_value[idx] == lock_name:
                                        lock_idx = idx
                                    if queue_value[idx] == queue_name:
                                        queue_idx = idx
                            #print(number_name)
                            graph[queue_idx][lock_idx][action_plot_idx][int(number_name)-1] = value_case[plot_case]
        else:
            continue
        
        drawGraph(graph, graph_name, queue_idx, numbers)

def parsing(root, testDir, files, resultFile):

    p_list = {}
    c_list = {}
    number_dic = {}
    cntCheck = 0
    transCnt = 0

    root = str(root).split('/')
    for r in root:
        if "queue" in r:
            queue_case = r.split("_")[1]
            lock_case = r.split("_")[2]

    resultFName = resultFile + "/" + queue_case + "_" + lock_case
    f = open(resultFName+'.csv', 'a')
    wr = csv.writer(f)

    for fname in files:
        if "queue" not in fname:
            continue

        bench, p_cnt, c_cnt,try_cnt,opt = fname.split('-')
        if any(p_cnt == x for x in p_list) == False:
            p_list[p_cnt] = []
            c_list[c_cnt] = []

        f = open(testDir+"/"+fname,'r')
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

        temp_dic[temp_str+"-"+action_plot_value[0]] = p_avg
        temp_dic[temp_str+"-"+action_plot_value[1]] = p_std
        temp_dic[temp_str+"-"+action_plot_value[2]] = c_avg
        temp_dic[temp_str+"-"+action_plot_value[3]] = c_std

        number_dic[temp_str] = temp_dic
        wr.writerow([key[i],p_avg,p_std,c_avg,c_std])


    for keys in number_dic.keys():
        if keys.split("-")[0] == queue_case and keys.split("-")[1] == lock_case:
            lock_dic[queue_case+"-"+lock_case] = number_dic
    
    f.close()

# Main function
def main(argv=None):

    resultFName = None
    testFName = None
    drawFlag = False

    # get arguments
    try:
        opts, args = getopt.getopt(sys.argv[1:], 'hf:t:g', ['help', 'file=', 'test=', 'graph='])
    except getopt.GetoptError as err:
        print (err)
        sys.exit(1)

    for o, a in opts:
        if o in ('-h', '--help'):
            usage()
            sys.exit()
        elif o in ('-t', '--test'):
            testDir = a
        elif o in ('-g', '--graph'):
            drawFlag = True
        else:
            assert False, 'unhandled option'

    if testDir == None:
        usage()
        sys.exit()

    date = strftime("%Y%m%d-%H%M", localtime())
    resultFName = "./csv/%s" % date
    if not os.path.isdir(resultFName):
        os.makedirs(resultFName)

    finish = 0
    # Parse the files
    for root, dirs, files in os.walk(testDir):
        if dirs:
            finish = 1
            for directory in dirs:
                directory = os.path.join(root, directory)
                if os.path.isdir(directory):
                    for sub_root, sub_dirs, sub_files in os.walk(directory):
                        parsing(sub_root, directory, sub_files, resultFName)

        else:
            if finish == 0:
                parsing(root, testDir, files, resultFName)

    for keys in lock_dic.keys():
        new_dict = {}
        for idx in range(0,len(queue_value)):
            if keys.split("-")[0] == queue_value[idx]:
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

    if drawFlag == True:
        extractGraphData(queue_dic)

    #print(json.dumps(queue_dic, indent=4))


if __name__ == "__main__":
    sys.exit(main())