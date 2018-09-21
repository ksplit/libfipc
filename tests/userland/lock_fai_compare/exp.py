import threading, requests
import sys,os,time,shlex,signal, atexit
import json, getopt
from subprocess import *
from time import localtime, strftime

RUN_BASE = "/users/achaccha/fast-ipc-module/tests/userland"
TEST_LIST = ['lock_fai_compare' ]
systemInfoFName = 'system.info'

# Default profiling options

def usage():
	print 'Usage: %s -c config.json' % sys.argv[0]

# Main function

def main(argv=None):

	####################
	# Read Configuration
	####################
	
	# write system information
	date = strftime("%Y%m%d-%H%M", localtime())
	output_path = "./results/%s" % date
	if not os.path.isdir(output_path):
		os.makedirs(output_path)

	sysInfoFile = open("%s/%s" % (output_path, systemInfoFName), 'a')
	sysInfoFile.write("--------------------------------------------------\n")
	cmd = "lscpu" 
	p = Popen( shlex.split(cmd), stdout=PIPE )
	for line in p.stdout.readlines():
		sysInfoFile.write(line)
	p.wait()

	sysInfoFile.close()
	#
	# Running benchmark
	#
	#lock_var = [1,2,3,4]
        lock = ['mcs', 'spin', 'ticket', 'FAI']
	lock_var = [4]
	transactions = [ 10000 ,100000, 1000000, 10000000]
	thread_num = [1,2,4,8,16,32]

	for i in range( 0, len(TEST_LIST) ):
		for l in lock_var:
			for ts in transactions:
				for tn in thread_num:	
					for t in range( 0, 1 ):
					##########################
					# Setup output directories
					##########################

						exp_output_path = "%s/%s" % (output_path, TEST_LIST[i])

						print "exp_output_path: %s\n" % exp_output_path
						if not os.path.isdir(exp_output_path):
							os.makedirs(exp_output_path)
						
						outputFile = open("%s/%s-%s" % (exp_output_path, TEST_LIST[i], lock[l-1]), 'a' )
						#outputFile = open("%s/%s-%s-%s-%s" % (exp_output_path, TEST_LIST[i], lock[l-1], ts, tn), 'a' )
						print "outputFile: %s" % outputFile

						print "\nDropping page cache"
						cmd = "echo 3 | sudo tee /proc/sys/vm/drop_caches"
						p = Popen( shlex.split(cmd) )
						p.wait()

						start_time = time.time()
						cmd = "%s/%s/%s %s %s %s" % (RUN_BASE, TEST_LIST[i], TEST_LIST[i], l, ts, tn)
						print("------------------------------\n")
						print "\n[ (%d) try ] %s" % (t, cmd)
						p = Popen( cmd, stdout=PIPE,  shell=True )

						for line in p.stdout.readlines():
							print line
							outputFile.write(line)
						outputFile.close()

						end_time = time.time()
						print "Time elapsed: ", end_time-start_time
						print "Outpute path: ", output_path
		
						print "Done"			
						print "Waiting... to warm the states"
						time.sleep(30)


if __name__ == '__main__' :
	sys.exit(main())




