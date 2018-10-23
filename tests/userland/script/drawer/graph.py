import pandas as pd
import matplotlib as mpl
import matplotlib.pyplot as plt
from script.config import Config

class Graph:

    @classmethod
    def extractGraphData(cls, result_dic):
        graph = [[[[0 for number in Config.numbers] for action_plot in range(len(Config.action_plot_value))] for lock in range(len(Config.lock_value))] for queue in range(len(Config.queue_value))]
        for queue_case in result_dic:
            #graph_name = queue_case
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
                                    if Config.action_plot_value[idx] == action_plot_name:
                                        action_plot_idx = idx
                                    if idx != 3:
                                        if Config.lock_value[idx] == lock_name:
                                            lock_idx = idx
                                        if Config.queue_value[idx] == queue_name:
                                            queue_idx = idx
                                
                                graph[queue_idx][lock_idx][action_plot_idx][int(number_name)-1] = value_case[plot_case]
            else:
                continue
        
        return graph

    @classmethod
    def drawGraph(cls, graph, graphFName, opt):
        if opt == 2:
            opt_list = [0,1]
        else:
            opt_list = [opt]

        for opt in opt_list:
            first_option = Config.draw_list[opt]
            second_option = Config.draw_list[(opt+1)%2]
            for first in range(len(first_option)):
                resultFName = graphFName + "/" + first_option[first]
                graph_name = first_option[first].upper()
                for second in range(len(second_option)):
                    for action_plot_idx in range(len(Config.action_plot_value)):
                        if action_plot_idx % 2 == 1:
                            continue

                        color = Config.color_value[second]
                        marker = "."

                        if action_plot_idx == 0:
                            linestyle = "-"
                        elif action_plot_idx == 2:
                            linestyle = "--"

                        if opt == 0:
                            queue_idx = first
                            lock_idx = second
                        elif opt == 1:
                            queue_idx = second
                            lock_idx = first
                        
                        label = second_option[second]+"_"+Config.action_plot_value[action_plot_idx]
                        fmt = color+marker+linestyle
                        
                        plt.errorbar(
                            Config.numbers, 
                            graph[queue_idx][lock_idx][action_plot_idx], 
                            yerr=graph[queue_idx][lock_idx][action_plot_idx+1],
                            fmt=fmt,
                            label=label,
                            linewidth=1, # 1 width of plot line
                            elinewidth=0.5,# 0.5 width of error bar line
                            ecolor='k', # color of error bar
                            capsize=5, # 5 cap length for error bar
                            capthick=0.5 # 0.5
                        )
                        

                plt.xlabel('Producer(Consumer) Num')
                plt.ylabel('Average of Cycle per message')
                plt.title(graph_name)
                plt.legend(loc='upper left', labelspacing=2, prop={'size':15} )
                
                fig = plt.gcf()
                fig.set_size_inches(18.5, 10.5)
                fig.savefig(resultFName+'.pdf')
                fig.clear()


    