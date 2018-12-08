""" 
 @File     : exp.py
 @Author   : Jiwon Jeon

 NOTE: This script runs simple increment test.
 
"""
import threading, requests
import sys,os,time,shlex
import json, getopt
from subprocess import *
from time import localtime, strftime

exp_path = None
output_path = "./results/"
EXP_INFO = "exp.info"
run_cnt = 1
exp_info_file = None
exp_start_time = None
exp_end_time = None
thread_cnt = 0
trx_cnt = 100

def usage():
    """ Print script usage. """
    print 'Usage: %s -p <exp_file_path> -t <run_cnt> -x <transaction_cnt>' % sys.argv[0]

def write_exp_info():
    """ This function writes experiment information. """
    if not os.path.isdir(output_path):
        os.makedirs(output_path)

    exp_info_file = open("%s/%s" % (output_path, EXP_INFO), 'w')
    p = Popen( shlex.split("lscpu"), stdout=PIPE )
    for line in p.stdout.readlines():
        exp_info_file.write(line)
    p.wait()
     
    exp_info_file.write("\n------------------------------------------------------\n")
    exp_info_file.write("transaction count: %d\n" % trx_cnt)
    exp_info_file.write("thtread count: %d\n" % thread_cnt)
    exp_info_file.write("output path: %s\n" % (os.path.abspath(output_path)))
    exp_info_file.write("elapsed time: %s\n" % (str(exp_end_time-exp_start_time)))
    exp_info_file.close()

def run_exp(argv=None):
    """ This function runs experiment """
    global exp_start_time, exp_end_time
    lock = ['mcs', 'spin', 'ticket', 'FAI']
    exp_start_time = time.time()
    for l in range(1, len(lock)+1):
        for tn in range(1, thread_cnt+1):	
            for t in range( 0, run_cnt ):
                outputFile = open("%s/%s" % (output_path, lock[l-1]), 'a' )
                print "outputFile: %s" % outputFile
                #print "\nDropping page cache"
                #cmd = "echo 3 | sudo tee /proc/sys/vm/drop_caches"
                #p = Popen( shlex.split(cmd) )
                #p.wait()
                start_time = time.time()
                cmd = "%s %s %s %s" % (exp_path, l, trx_cnt, tn)
                print(cmd)
                print("------------------------------\n\n")
                print "[ try (%d) ] %s" % (t, cmd)
                p = Popen( cmd, stdout=PIPE,  shell=True )
        
                for line in p.stdout.readlines():
                    print line
                    outputFile.write(line)
                outputFile.close()
        
                end_time = time.time()
                print "Time elapsed: ", end_time-start_time
                print "Outpute path: ", output_path
                print "Done"			
                #time.sleep(1)
    exp_end_time = time.time()    

if __name__ == '__main__' :
    try:
        opts, args = getopt.getopt(sys.argv[1:], 'hp:t:x:', ['help', 'path=', 'thread_cnt=', 'transaction_cnt='])
    except getopt.GetoptError as err:
        print(err)
        sys.exit(1)

    for o, a in opts:
        if o in ('-h', '--help'):
            usage()
            sys.exit()
        elif o in ('-p', '--path'):
            exp_path = a
        elif o in ('-t', '--thread_cnt'):
            thread_cnt = int(a)
        elif o in ('-x', '--transaction_cnt'):
            trx_cnt = int(a)
        else:
            assert False, 'unhandled option'
            
    if thread_cnt is None or exp_path is None:
        usage()
        sys.exit()
    
    date = strftime("%Y%m%d-%H%M", localtime())
    output_path += date
    if not os.path.isdir(output_path):
        os.makedirs(output_path)	
    
    run_exp()
    sys.exit(write_exp_info())
