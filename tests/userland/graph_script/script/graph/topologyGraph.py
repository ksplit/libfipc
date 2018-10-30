import json
import pandas as pd
import matplotlib as mpl
import matplotlib.pyplot as plt

from script.config import Config
from script.graph import Graph

class TopologyGraph(Graph):
    
    @classmethod
    def drawGraph(cls, objs, file):

        opts = Config.topology_option
        if opts == 2:
            opt_list = [0,1]
        else:
            opt_list = [opts]

        for opt in opt_list:
            first_option = Config.draw_list[opt]
            second_option = Config.draw_list[(opt+1)%2]

            # Queue or Lock Graph based on Option
            for first in range(len(first_option)):
                result_file_name = file + "/" + first_option[first]
                graph_name = first_option[first].upper()

                for second in range(len(second_option)):
                    for action_plot_idx in range(len(Config.action_plot_value)):
                        
                        # stddev data
                        if action_plot_idx % 2 == 1:
                            continue

                        if opt == 0:
                            queue_idx = first
                            lock_idx = second
                        elif opt == 1:
                            lock_idx = first
                            queue_idx = second
                            
                        #color = Config.color_value[second]
                        color = Config.topology_color_dict[second_option[second]]
                        marker = Config.marker

                        if action_plot_idx == 0:
                            linestyle = Config.linestyle_list[0]
                        elif action_plot_idx == 2:
                            linestyle = Config.linestyle_list[1]    
                        
                        xData = [i for i in range(Config.numbers)]
                        label = second_option[second]+"_"+Config.action_plot_value[action_plot_idx]
                        fmt = color+marker+linestyle

                        plt.errorbar(
                            x=xData, 
                            y=objs[queue_idx][lock_idx][action_plot_idx], 
                            yerr=objs[queue_idx][lock_idx][action_plot_idx+1],
                            fmt=fmt,
                            label=label,
                            linewidth=1,    # 1 width of plot line
                            elinewidth=0.5, # 0.5 width of error bar line
                            ecolor='k',     # color of error bar
                            capsize=5,      # 5 cap length for error bar
                            capthick=0.5    # 0.5
                        )
                        

                plt.xlabel('Producer(Consumer) Num', fontSize=Config.font_size)
                plt.ylabel('Average of Cycle per message', fontSize=Config.font_size)
                plt.title(graph_name, fontSize=Config.font_size)
                plt.legend(loc='upper left', labelspacing=2, prop={'size':15} )
                
                fig = plt.gcf()
                fig.set_size_inches(18.5, 10.5)
                fig.savefig(result_file_name+'.pdf')
                fig.clear()
