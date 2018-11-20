Oraganizing directory like results directory in graph_script.


# To run with default argument, Extract CSV files and Graph files from results folder 
python main.py results/<machine_hyper>/<policy>


# To run with all argument *same as default (-a, --all)
python main.py results/<machine_hyper>/<policy> -a


# To run with csv argument (-c, --csv), Extract CSV files from results folder
python main.py results/<machine_hyper>/<policy> -c


# To run with graph argument (-g, --graph), Extract Graph files from csv folder
python main.py csv/<machine_hyper>/<policy>  -g


# To run with topology argument (-q, --topology), Extract Topology Graph files from csv folder
python main.py csv/<machine_hyper>/<policy>  -q 


# To run with time argument (-t, --time), Extract Time Graph files from csv folder
python main.py csv/<machine_hyper>/<policy>  -t  


# To run with log argument (-l, --log), Extract Topology Log graph files from csv folder
python main.py csv/<machine_hyper>/<policy>  -l  

