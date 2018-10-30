import pandas as pd
import matplotlib as mpl
import matplotlib.pyplot as plt
import numpy as np
import json
from matplotlib.collections import PolyCollection

from script.config import Config
from script.graph import Graph

class TimeGraph(Graph):

	@classmethod
	def drawGraph(cls, objs, file):		
		for queue in Config.queue_value:
			for lock in Config.lock_value:
				queue_idx = Config.queue_value.index(queue)
				lock_idx = Config.lock_value.index(lock)
				
				for number in Config.time_number_list:
					if Config.information_value[1] not in objs[queue_idx][lock_idx][number].keys():
						continue

					graph_name = queue+"_"+lock+"_"+str(number)
					result_file_name = file + "/" + graph_name
					graph_name = graph_name.upper()

					producer = ["Producer {}".format(i) for i in range(number)]
					consumer = ["Consumer {}".format(i) for i in range(number)]

					total_number = number * 2

					begin_list = list(objs[queue_idx][lock_idx][number][Config.information_value[4]].values()) + list(objs[queue_idx][lock_idx][number][Config.information_value[2]].values())
					end_list = list(objs[queue_idx][lock_idx][number][Config.information_value[5]].values()) + list(objs[queue_idx][lock_idx][number][Config.information_value[3]].values())
					event_list = consumer + producer

					max_begin_time = np.max(begin_list)
					min_end_time = np.min(end_list)
					max_end_time = np.max(end_list)

					fig = plt.figure(figsize=(Config.time_fig_width,Config.time_fig_height))
					ax = fig.add_subplot(111)    
					ax1 = fig.add_subplot(121)
					ax2 = fig.add_subplot(122)

					ax1_xstart = 0
					ax1_xend = pow(10, len(str(max_begin_time)))
					ax2_xstart = pow(10, len(str(min_end_time)))
					ax2_xend = pow(10, len(str(max_end_time)))

					#if ax2_xstart == ax2_xend:
					#	ax2_xstart = int(ax2_xstart / 10)
					
					ax_xrange = int(ax2_xend / 10)

					ax1_tick = [i for i in range(ax1_xstart, ax1_xend, ax_xrange)]
					ax2_tick = [i for i in range(ax2_xstart, ax2_xend+ax_xrange, ax_xrange)]

					ax1.set_xlim(0, max_begin_time)
					ax2.set_xlim(min_end_time, max_end_time)

					ax1.set_xticks(ax1_tick)
					ax2.set_xticks(ax2_tick)

					ax1.set_xticklabels(ax1_tick, fontsize=Config.time_xlabel_size)
					ax2.set_xticklabels(ax2_tick, fontsize=Config.time_xlabel_size)
					ax1.set_yticklabels(event_list, fontsize=Config.time_ylabel_size)

					ax1.spines['right'].set_visible(False)
					ax2.spines['left'].set_visible(False)

					ax.spines['top'].set_visible(False)
					ax.spines['bottom'].set_visible(False)

					ax.grid(True)
					
					ax1.yaxis.tick_left()
					ax2.yaxis.tick_left()

					ax1.set_yticks([(i*10)+5 for i in range(1, total_number+1)])
					
					ax.tick_params(labelcolor='w', top=False, bottom=False, left=False, right=False)
					ax1.tick_params(pad=Config.time_tick_pad)
					ax2.tick_params(pad=Config.time_tick_pad, labelleft=False, )
					
					for i in range(len(begin_list)):
						action = event_list[i].split(" ")[0]
						ax1.broken_barh([(begin_list[i], ax1_xend-begin_list[i])], ((i+1)*10, Config.time_y_thickness), facecolors=Config.time_color_dict[action])

					for i in range(len(end_list)):
						action = event_list[i].split(" ")[0]
						ax2.broken_barh([(min_end_time, end_list[i]-min_end_time)], ((i+1)*10, Config.time_y_thickness), facecolors=Config.time_color_dict[action])

					
					kwargs = dict(transform=ax1.transAxes, color='k', clip_on=False)
					ax1.plot((1-Config.time_diagonal, 1+Config.time_diagonal), (-Config.time_diagonal, +Config.time_diagonal), **kwargs)
					ax1.plot((1-Config.time_diagonal, 1+Config.time_diagonal), (1-Config.time_diagonal, 1+Config.time_diagonal), **kwargs)

					kwargs.update(transform=ax2.transAxes)
					ax2.plot((-Config.time_diagonal,+Config.time_diagonal), (1-Config.time_diagonal,1+Config.time_diagonal), **kwargs)
					ax2.plot((-Config.time_diagonal,+Config.time_diagonal), (-Config.time_diagonal,+Config.time_diagonal), **kwargs)

					ax.set_xlabel('Time', fontSize=Config.font_size, labelpad=Config.labelpad)
					ax.set_ylabel('Producer or Consumer Number', fontSize=Config.font_size, labelpad = Config.labelpad)
					ax.set_title(graph_name, fontSize=Config.font_size, pad = Config.pad)
					
					fig.set_size_inches(Config.time_set_size_width, Config.time_set_size_height)
					fig.savefig(result_file_name+'.pdf')
					fig.clear()

					
					
