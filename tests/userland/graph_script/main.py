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

# Main function
def main(argv=None):

    draw_flag = False
    hyper_option = True
    draw_option = 2

    # get arguments
    try:
        opts, args = getopt.getopt(sys.argv[1:], 'ht:g:', ['help', 'test=', 'graph='])
    except getopt.GetoptError as err:
        print (err)
        sys.exit(1)

    for o, a in opts:
        if o in ('-h', '--help'):
            usage()
            sys.exit()
        elif o in ('-t', '--test'):
            test_directory = a
        elif o in ('-g', '--graph'):
            draw_flag = True
            draw_option = int(a)
        else:
            assert False, 'unhandled option'

    if test_directory == None:
        usage()
        sys.exit()

    if 'off' in test_directory:
        hyper_option = False

    date = strftime("%m%d_%H%M", localtime())

    topology_csv_files = TopologyCSV(test_directory, date, hyper_option)
    time_csv_files = TimeCSV(test_directory, date, hyper_option)

    topology_data = TopologyDataExtractor(topology_csv_files, hyper_option)
    time_data = TimeDataExtractor(time_csv_files, hyper_option)
    
    TopologyGraph(topology_data, test_directory, date, hyper_option)
    TimeGraph(time_data, test_directory, date, hyper_option)
    
if __name__ == "__main__":
    sys.exit(main())