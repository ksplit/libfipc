import os

class Config:
	#numbers = 32+1
	test = 5+1

	numbers = {True:32+1, False:16+1}
	
	queue_value = ['mpmc', 'mpsc', 'spsc']
	lock_value = ['spinlock', 'ticketlock', 'mcslock', 'lockfree']

	lockless_value = ['lockfree']

	action_plot_value = ['producer_average', 
						'producer_stddev', 
						'consumer_average', 
						'consumer_stddev']

	information_value = ['transaction_count',
						'producer_cycle_list', 
						'consumer_cycle_list',
						'producer_start_time_list', 
						'producer_end_time_list', 
						'consumer_start_time_list', 
						'consumer_end_time_list',
						'global_time'
						]

	time_test_number = 1

	'''
	Topology Graph Option 
	'''

	# fig size
	topology_fig_width = 60 	# 30
	topology_fig_height = 50 	# 30
	topology_width_ratio = 1
	topology_height_ratio = 0.9

	# font size
	topology_fontsize = 100
	topology_xtick_fontsize = 60
	topology_ytick_fontsize = 60

	# legend position
	topology_bbox_x = 0
	topology_bbox_x_width = 3
	topology_bbox_y_first = 1.08
	topology_bbox_y_second = 1.01
	topology_bbox_y_height = 4

	topology_linewidth = 4		# legend line width
	topology_legend_size = 70	# legend size

	topology_tick_pad = 70		# tick padding
	topology_label_pad = 70		# label padding
	topology_title_pad = 400		# title padding
	
	topology_xrange_size = 4	# x axis thread number range size

	topology_marker_size = 30			# marker size
	topology_marker_edge_color = 'k'	# marker edge color
	topology_marker_edge_width = 5		# marker edge width
	
	# graph pivot option
	topology_option = 2			
	topology_draw_option = [queue_value, lock_value]
	
	# Producer, Consumer line style list
	topology_linestyle_list = ["-", "--"]	

	# Each case's marker shape dict
	topology_marker_dict = {
		"spinlock" : 's',
		"ticketlock" : '^',
		"mcslock" : 'o',
		"lockfree" : 'd',
		"mpmc" : 's',
		"mpsc" : '^',
		"spsc" : 'o'
	}

	# Each case's color dict
	topology_color_dict = {
		"spinlock" : 'r',
		"ticketlock" : 'g',
		"mcslock" : 'b',
		"lockfree" : 'k',
		"mpmc" : 'c',
		"mpsc" : 'm',
		"spsc" : 'y'
	}


	'''
	Time Graph Option
	'''

	# fig size
	time_fig_width = 80
	time_fig_height = 50

	# font size
	time_fontsize = 120
	time_xtick_fontsize = 80
	time_ytick_fontsize = 80

	# legend position
	time_bbox_x = 1.01
	time_bbox_y = 0.95

	time_linewidth = 10			# legend line width
	time_legend_size = 50		# legend size

	time_tick_pad = 15			# tick padding
	time_label_pad = 200		# label padding
	time_title_pad = 100		# title padding

	time_y_thickness = 4		# bar line width

	time_diagonal = 0.01		# broken bar diagonal length

	
	# ax1, ax2 x axis margin ratio
	time_ax1_margin_ratio = 1.05
	time_ax2_margin_ratio = 0.05
	time_ax2_xstart_tick_margin_ratio = 0.1
	time_ax2_xend_tick_margin_ratio = 0.01
	
	# Producer, Consumer line style list
	time_linestyle_list = ["-"]

	# number dict accordint to thread number
	time_number_dict = {True:[32], False:[16]}

	# Each case's color dict
	time_color_dict = {
		"Producer" : "b", 
		"Consumer" : "r"
	}


	

	
	
