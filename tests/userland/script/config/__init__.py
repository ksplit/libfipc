import os

class Config:
    # Configuration for redis
    
    numbers = [i for i in range(1,33)]

    queue_value = ['mpmc', 'mpsc', 'spsc']
    lock_value = ['spinlock', 'ticketlock', 'mcslock']
    action_plot_value = ['prod_avg', 'prod_std', 'cons_avg', 'cons_std']
    color_value = ['r', 'g', 'b']

    draw_list = [queue_value, lock_value]