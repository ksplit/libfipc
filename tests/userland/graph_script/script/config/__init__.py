import os

class Config:
    # Configuration for redis
    
	numbers = 32+1
	test = 5+1
	
	queue_value = ['mpmc', 'mpsc', 'spsc']
	lock_value = ['spinlock', 'ticketlock', 'mcslock']

	action_plot_value = ['producer_average', 
						'producer_stddev', 
						'consumer_average', 
						'consumer_stddev']

	information_value = ['action_plot_value', 
						'global_time', 
						'producer_start_time_dict', 
						'producer_end_time_dict', 
						'consumer_start_time_dict', 
						'consumer_end_time_dict', 
						'producer_pin_dict', 
						'consumer_pin_dict']

	# Graph Option
	font_size = 30
	labelpad = 200
	pad = 100



	# Topology Graph Option
	topology_option = 2
	marker = "."
	linestyle_list = ["-", "--"]
	draw_list = [queue_value, lock_value]
	topology_color_dict = {
		"spinlock" : 'r',
		"ticketlock" : 'g',
		"mcslock" : 'b',
		"mpmc" : 'c',
		"mpsc" : 'm',
		"spsc" : 'y'
	}

	# Time Graph Option
	time_xlabel_size = 15
	time_ylabel_size = 20
	time_tick_pad = 15
	time_y_thickness = 7
	time_diagonal = 0.015

	time_fig_width = 150
	time_fig_height = 80



	time_set_size_width = 50
	time_set_size_height = 40

	time_number_list = [32]

	time_color_dict = {
		"Producer" : "C0", 
		"Consumer" : "C1"
	}


	

	
	
