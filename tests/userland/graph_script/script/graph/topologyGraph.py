import json
import pandas as pd
import math
import matplotlib as mpl
import matplotlib.pyplot as plt
import matplotlib.patches as mpatches
import numpy as np
import os

from matplotlib.lines import Line2D

from script.config import Config
from script.graph import Graph

class TopologyGraph(Graph):

    @classmethod
    def makeGraphDirectoryName(cls, directory, date):
        graph_directory = "./graph/topology/%s" % ( directory.split('/')[1]+'-'+date )
        if not os.path.isdir(graph_directory):
            os.makedirs(graph_directory)

        return graph_directory

    @classmethod
    def drawGraph(cls, objs, directory, hyper_option):

        '''
        
        Drawing queue compare graph and lock compare graph based on topology_option
        
        topology_option = 0 : Drawing graph which compare lock in each queue
        topology_option = 1 : Drawing graph which compare queue in each lock
        topology_option = 2 : Drawing both graph

        '''
        
        opts = Config.topology_option
        if opts == 2:
            opt_list = [0,1]
        else:
            opt_list = [opts]

        for opt in opt_list:
            first_option = Config.topology_draw_option[opt]
            second_option = Config.topology_draw_option[(opt+1)%2]

            for first in range(len(first_option)):
                result_file_name = directory + "/" + first_option[first]
                graph_name = first_option[first].upper()
                
                fig = plt.figure(figsize=(Config.topology_fig_width,Config.topology_fig_height))
                ax = fig.subplots(1, 1)
                
                upper_y = 0
                topology_plt_list = []

                for second in range(len(second_option)):
                    for action_plot_idx in range(len(Config.action_plot_value)):
                        
                        # Standard Deviation Data
                        if action_plot_idx % 2 == 1:
                            continue

                        if opt == 0:
                            queue_idx = first
                            lock_idx = second
                        elif opt == 1:
                            lock_idx = first
                            queue_idx = second

                        # There are no data in list so continue...
                        if objs[queue_idx][lock_idx][action_plot_idx].count(0) > 1:
                            continue

                        color = Config.topology_color_dict[second_option[second]]
                        marker = Config.topology_marker_dict[second_option[second]]

                        if action_plot_idx == 0:    # Producer Average 
                            linestyle = Config.topology_linestyle_list[0]
                        elif action_plot_idx == 2:  # Consumer Average
                            linestyle = Config.topology_linestyle_list[1]    
                        
                        xData = [i for i in range(Config.numbers[hyper_option])]    # X axis data based on hyper option
                        fmt = color+marker+linestyle 

                        topology_plt = plt.errorbar(
                            x=xData, 
                            y=objs[queue_idx][lock_idx][action_plot_idx], 
                            fmt=fmt,
                            linewidth=Config.topology_linewidth,    # width of plot line
                            markersize=Config.topology_marker_size,
                            markeredgecolor=Config.topology_marker_edge_color,
                            markeredgewidth=Config.topology_marker_edge_width
                            #yerr=objs[queue_idx][lock_idx][action_plot_idx+1],
                            #label=label,
                            #elinewidth=0.5, # width of error bar line
                            #ecolor='k',     # color of error bar
                            #capsize=5,      # cap length for error bar
                            #capthick=0.5    # 
                        )

                        # Errobar list for first legend
                        topology_plt_list.append(topology_plt)

                        # Find max Y value and insert in upper_y
                        max_y = np.max(objs[queue_idx][lock_idx][action_plot_idx])
                        if max_y > upper_y:
                            upper_y = int(max_y)
                
                # Setting Y range
                if upper_y != 0:
                    ax_yrange, ax_ylimit = cls.setYrange(upper_y)
                else:
                    continue
                
                ax_xtick = [i for i in range(0, Config.numbers[hyper_option], Config.topology_xrange_size)]
                ax.set_xlim(0, Config.numbers[hyper_option]-1)
                ax.set_xticks(ax_xtick)
                ax.set_xticklabels(ax_xtick, fontsize=Config.topology_xtick_fontsize)

                '''
                ax_ytick = [i for i in range(0, ax_ylimit, ax_yrange)]
                ax.set_ylim(0, ax_ylimit)
                ax.set_yticks(ax_ytick)
                ax.set_yticklabels(ax_ytick, fontsize=Config.topology_ytick_fontsize)
                ax.tick_params(pad=Config.topology_tick_pad)
                '''
                
                ax.set_yscale('log')
                ax.tick_params(labelsize=Config.topology_ytick_fontsize, pad=Config.topology_tick_pad)

                ax.set_xlabel('Producer(Consumer) Num', fontSize=Config.topology_fontsize, labelpad=Config.topology_label_pad)
                ax.set_ylabel('Average of Cycle per message', fontSize=Config.topology_fontsize, labelpad=Config.topology_label_pad)
                ax.set_title(graph_name, fontSize=Config.topology_fontsize, pad=Config.topology_title_pad)

                box = ax.get_position()
                ax.set_position([box.x0+0.02, box.y0, box.width * Config.topology_width_ratio, box.height * Config.topology_height_ratio])
                first_legend_elements = cls.buildLegendData()
                first_legend, second_legend = cls.buildLegend(first_legend_elements, topology_plt_list, second_option)
    
                plt.gca().add_artist(first_legend)
                plt.grid(True)
                
                fig.savefig(result_file_name+'.pdf')
                fig.clear()

    @classmethod
    def setYrange(cls, objs):

        '''   
        Setting Y range for visualizing more effetively
        '''

        y_range_digit = len(str(objs))-3
        y_range = pow(10, y_range_digit)
        old_proximity = math.floor(objs/y_range)+1
        
        if old_proximity % 25  == 0:
            new_proximity = old_proximity
        else:
            new_proximity = (int(old_proximity / 25) + 1) * 25

        y_limit = new_proximity*y_range
        y_range_new = int(y_limit / 5)
        
        return y_range_new, y_limit

    @classmethod
    def buildLegendData(cls):

        '''
        Make Producer and Consumer Legend Data
        '''
        return [Line2D([0], [0], 
                    color='black', 
                    lw=Config.topology_linewidth, 
                    label='Producer', 
                    linestyle=Config.topology_linestyle_list[0]),
                Line2D([0],[0],
                    color='black', 
                    lw=Config.topology_linewidth, 
                    label='Consumer', 
                    linestyle=Config.topology_linestyle_list[1])
            ]

    @classmethod
    def buildLegend(cls, first_objs, second_objs, opt):

        '''
        Make 2 Legend
        
        1. Producer, Consumer Legend
        2. Queue or Lock Legend(based on opt) 
        '''

        first_legend = plt.legend(
                    handles=first_objs,
                    loc="lower left",
                    ncol=2,
                    bbox_to_anchor=(Config.topology_bbox_x,Config.topology_bbox_y_first,Config.topology_bbox_x_width,Config.topology_bbox_y_height),
                    #loc='center left', 
                    #bbox_to_anchor=(Config.topology_bbox_x, Config.topology_bbox_y_first),
                    borderaxespad=0,
                    fancybox=True, 
                    prop={'size':Config.topology_legend_size},
                    labelspacing=2, 
                    shadow=True
                )

        second_legend = plt.legend(
                    [second_objs[p] for p in range(0,len(second_objs),2)], 
                    opt, 
                    loc="lower left",
                    ncol=len(second_objs),
                    bbox_to_anchor=(Config.topology_bbox_x,Config.topology_bbox_y_second,Config.topology_bbox_x_width,Config.topology_bbox_y_height),
                    #loc='center left', 
                    #bbox_to_anchor=(Config.topology_bbox_x, Config.topology_bbox_y_second),
                    borderaxespad=0,
                    fancybox=True, 
                    prop={'size':Config.topology_legend_size},
                    labelspacing=2,
                    shadow=True 
                )

        return first_legend, second_legend
                