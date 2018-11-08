import pandas as pd
import matplotlib as mpl
import matplotlib.pyplot as plt
import matplotlib.gridspec as gridspec
import numpy as np
import json

from matplotlib.collections import PolyCollection
from matplotlib.lines import Line2D

from script.config import Config
from script.graph import Graph

class TimeGraph(Graph):

	@classmethod
	def drawGraph(cls, objs, file, hyper_option):

		'''
        Drawing Time(Starvation) Graph
        '''

		for queue in Config.queue_value:
			for lock in Config.lock_value:
				queue_idx = Config.queue_value.index(queue)
				lock_idx = Config.lock_value.index(lock)
				
				for number in Config.time_number_dict[hyper_option]:
					if Config.information_value[1] not in objs[queue_idx][lock_idx][number].keys():
						continue

					graph_name = queue+"_"+lock+"_"+str(number)
					result_file_name = file + "/" + graph_name
					graph_name = graph_name.upper()		

					producer = ["P{}".format(i) for i in range(number, 0, -1)]
					consumer = ["C{}".format(i) for i in range(number, 0, -1)]

					total_number = number * 2

					'''
					begin_list : Producer and Consumer begin time List
					end_list : Producer and Consumer end time List
					event_list : Producer and Consumer Number List
					'''
					begin_list = list(objs[queue_idx][lock_idx][number][Config.information_value[4]].values())[::-1] + list(objs[queue_idx][lock_idx][number][Config.information_value[2]].values())[::-1]
					end_list = list(objs[queue_idx][lock_idx][number][Config.information_value[5]].values())[::-1]+ list(objs[queue_idx][lock_idx][number][Config.information_value[3]].values())[::-1]
					event_list = consumer + producer

					min_begin_time = np.min(begin_list)
					max_begin_time = np.max(begin_list)
					min_end_time = np.min(end_list)
					max_end_time = np.max(end_list)

					fig = plt.figure(figsize=(Config.time_fig_width,Config.time_fig_height))
					
					# Split sub plot at a specific ratio because of the big difference between left and right graph values
					ax = fig.add_subplot(1,1,1)    
					gs = gridspec.GridSpec(1,6)	
					ax1 =fig.add_subplot(gs[0])
					ax2 =fig.add_subplot(gs[1:])	

					ax1_xstart, ax1_xend = cls.setax1LimitRange(max_begin_time)
					ax2_xstart, ax2_xend, ax2_gap = cls.setax2LimitRange(max_end_time, min_end_time)

					ax1_average_time = int((max_begin_time+min_begin_time) / 2)
					ax2_average_time = int((max_end_time+min_end_time) / 2)
					
					'''
					Tick Setting...

					ax2_tick_xstart, ax2_tick_xend, ax2_xrange = cls.setax2TickRange(ax2_xstart, ax2_xend, ax2_gap) 
					ax1_tick = [i for i in range(ax1_xstart, ax1_xend + 1, ax1_xend)]
					ax2_tick = [i for i in range(ax2_tick_xstart, ax2_tick_xend + 1, ax2_xrange)]
					'''

					ax1_tick = [min_begin_time, ax1_average_time, max_begin_time]
					ax2_tick = [min_end_time, ax2_average_time, max_end_time]

					ax1.set_xlim(ax1_xstart, ax1_xend)
					ax2.set_xlim(ax2_xstart, ax2_xend)

					ax1.set_xticks(ax1_tick)
					ax2.set_xticks(ax2_tick)

					ax1.set_xticklabels(ax1_tick, fontsize=Config.time_xtick_fontsize)
					ax2.set_xticklabels(ax2_tick, fontsize=Config.time_xtick_fontsize)
					ax1.set_yticklabels(event_list, fontsize=Config.time_ytick_fontsize)

					ax1.spines['right'].set_visible(False)
					ax2.spines['left'].set_visible(False)

					ax.spines['top'].set_visible(False)
					ax.spines['bottom'].set_visible(False)
					
					ax1.yaxis.tick_left()
					ax2.yaxis.tick_left()

					ax1.grid(True, axis='x')
					ax2.grid(True, axis='x')

					ax1.set_yticks([(i*10)+4 for i in range(1, total_number+1)])
					
					ax.tick_params(labelcolor='w', top=False, bottom=False, left=False, right=False)
					ax1.tick_params(pad=Config.time_tick_pad)
					ax2.tick_params(pad=Config.time_tick_pad, labelleft=False)
					
					for i in range(len(begin_list)):
						if event_list[i][0] == 'P':
							action = "Producer"
						else:
							action = "Consumer"

						ax1.broken_barh([(begin_list[i], ax1_xend-begin_list[i])], ((i+1)*10, Config.time_y_thickness), facecolors=Config.time_color_dict[action])

					for i in range(len(end_list)):
						if event_list[i][0] == 'P':
							action = "Producer"
						else:
							action = "Consumer"

						ax2.broken_barh([(ax2_xstart, end_list[i]-ax2_xstart)], ((i+1)*10, Config.time_y_thickness), facecolors=Config.time_color_dict[action])
					
					# Setting Broken bar diagonal size and position
					kwargs = dict(transform=ax1.transAxes, color='k', clip_on=False)
					ax1.plot((1-Config.time_diagonal, 1+Config.time_diagonal), (-Config.time_diagonal, +Config.time_diagonal), **kwargs)
					ax1.plot((1-Config.time_diagonal, 1+Config.time_diagonal), (1-Config.time_diagonal, 1+Config.time_diagonal), **kwargs)

					kwargs.update(transform=ax2.transAxes)
					ax2.plot((-Config.time_diagonal,+Config.time_diagonal), (1-Config.time_diagonal,1+Config.time_diagonal), **kwargs)
					ax2.plot((-Config.time_diagonal,+Config.time_diagonal), (-Config.time_diagonal,+Config.time_diagonal), **kwargs)

					# Make Legend
					legend_elements = cls.buildLegendData()
					legend = cls.buildLegend(legend_elements)

					ax.set_xlabel('Time', fontSize=Config.time_fontsize, labelpad=Config.time_label_pad)
					ax.set_ylabel('Producer or Consumer Number', fontSize=Config.time_fontsize, labelpad = Config.time_label_pad)
					ax.set_title(graph_name, fontSize=Config.time_fontsize, pad = Config.time_title_pad)

					fig.savefig(result_file_name+'.pdf')
					fig.clear()

	@classmethod
	def setax1LimitRange(cls, max_begin_time):

		'''
		Setting ax1 x axis range
		'''

		ax1_xstart 	= 0
		ax1_xend 	= int (max_begin_time  * Config.time_ax1_margin_ratio)
		
		return ax1_xstart, ax1_xend

	@classmethod
	def setax2LimitRange(cls, max_end_time, min_end_time):

		'''
		Setting ax2 x axis range
		'''

		ax2_gap 	= max_end_time - min_end_time

		ax2_xstart 	= int(min_end_time - ax2_gap * Config.time_ax2_margin_ratio)
		ax2_xend 	= int(max_end_time + ax2_gap * Config.time_ax2_margin_ratio)

		return ax2_xstart, ax2_xend, ax2_gap

	@classmethod
	def setax2TickRange(cls, min_end_time, max_end_time, ax2_gap):

		'''
		Setting ax2 x axis tick range
		'''

		temp = len(str(int(ax2_gap * Config.time_ax2_xstart_tick_margin_ratio))) - 1 
		temp2 = len(str(int(ax2_gap * Config.time_ax2_xend_tick_margin_ratio))) - 1
					
		ax2_tick_xstart = pow(10,temp) * int(min_end_time / pow (10, temp ) + 1 )
		ax2_tick_xend 	= pow(10, temp2) * int(max_end_time / pow (10, temp2 ) - 1)

		ax2_xrange = int( (ax2_tick_xend-ax2_tick_xstart) / 5)

		return ax2_tick_xstart, ax2_tick_xend, ax2_xrange

	@classmethod
	def buildLegendData(cls):

		'''
		Make Producer, Consumer Legend Data
		'''

		return [Line2D([0], [0], 
					color=Config.time_color_dict["Producer"], 
					lw=Config.time_linewidth, 
					label='Producer', 
					linestyle=Config.time_linestyle_list[0]),
                 Line2D([0],[0],
					color=Config.time_color_dict["Consumer"], 
					lw=Config.time_linewidth, 
					label='Consumer', 
					linestyle=Config.time_linestyle_list[0])
				]

	@classmethod
	def buildLegend(cls, objs):

		'''
		Make Producer, Consumer Legend
		'''

		legend = plt.legend(
		                handles=objs,
		                loc='center left', 
		                labelspacing=2, 
		                bbox_to_anchor=(Config.time_bbox_x, Config.time_bbox_y),
		                borderaxespad=0,
		                fancybox=True, 
		                shadow=True,
		                prop={'size':Config.time_legend_size}
		        	)
		return legend				
