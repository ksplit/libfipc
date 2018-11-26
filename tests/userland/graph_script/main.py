import getopt
import os
import sys
import json

from script.config import Config

from script.csv.topologyCSV import TopologyCSV
from script.csv.timeCSV import TimeCSV

from script.extractor.topologyData import TopologyDataExtractor
from script.extractor.timeData import TimeDataExtractor

from script.graph.timeGraph import TimeGraph
from script.graph.topologyGraph import TopologyGraph

from time import localtime, strftime

'''

default : python main.py results/<machine_hyper>/<policy>
-a      : python main.py results/<machine_hyper>/<policy> -a
-c      : python main.py results/<machine_hyper>/<policy> -c

-g      : python main.py csv/<machine_hyper>/<policy>  -g  
-q      : python main.py csv/<machine_hyper>/<policy>  -q  
-t      : python main.py csv/<machine_hyper>/<policy>  -t  

'''

# Main function
def main(argv=None):

    hyper_option = True

    csv_option = True
    topology_graph_option = True
    time_graph_option = True

    log_option = False

    # get arguments
    try:
        test_directory = sys.argv[1]
        opts, args = getopt.getopt(sys.argv[2:], 'hacgqtl', ['help', 'all=', 'csv=', 'graph=', 'topology=', 'time=', 'log='])
    except getopt.GetoptError as err:
        print (err)
        sys.exit(1)

    for o, a in opts:
        if o in ('-h', '--help'):
            usage()
            sys.exit()

        elif o in ('-a', '--all'):
            continue

        elif o in ('-c', '--csv'):
            topology_graph_option = False
            time_graph_option = False

        elif o in ('-g', '--graph'):
            csv_option = False

        elif o in ('-q', '--topology'):
            csv_option = False
            time_graph_option = False

        elif o in ('-l', '--log'):
            csv_option = False
            time_graph_option = False
            log_option = True

        elif o in('-t', '--time'):
            csv_option = False
            topology_graph_option = False

        else:
            assert False, 'unhandled option'    

    if test_directory == None:
        usage()
        sys.exit()

    if 'off' in test_directory:
        hyper_option = False

    draw_opts = [hyper_option, log_option]
    
    topology_csv_directory = TopologyCSV(test_directory, hyper_option)
    time_csv_directory = TimeCSV(test_directory, hyper_option)
    
    
    if topology_graph_option == True:
        topology_data = TopologyDataExtractor(topology_csv_directory, hyper_option)
        TopologyGraph(topology_data, test_directory, draw_opts)

    if time_graph_option == True:
        time_data = TimeDataExtractor(time_csv_directory, hyper_option)
        TimeGraph(time_data, test_directory, draw_opts)
    
if __name__ == "__main__":
    sys.exit(main())