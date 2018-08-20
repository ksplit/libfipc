import threading, requests
import sys,os,time,shlex,signal, atexit
import json, getopt
from subprocess import *
from time import localtime, strftime

RUN_BASE = "/users/ji1/fast-ipc-module/tests/userland"
TEST_LIST = [ 'queue_mpmc_spinlock', 'queue_mpsc_spinlock' ]
systemInfoFName = 'system.info'


# Default profiling options

def usage():
	print 'Usage: %s -c config.json' % sys.argv[0]

# Main function

def main(argv=None):

	####################
	# Read Configuration
	####################
	config_file = None

	try:
		opts,args = getopt.getopt(sys.argv[1:], 'hc:', ['help', 'config='])
	except getopt.GetoptError as err:
		print (err)
		sys.exit(1)

	for o, a in opts:
		if o in ('-h', '--help'):
			usage()
			sys.exit()
		elif o in ('-c', '--config'):
			config_file = a;
		else:
			assert False, 'unhandled option'

	if config_file == None:
		print "[Error] Please specify the configuration file"
		usage()
		sys.exit(1)

	try:
		with open(config_file) as config_file:
			config = json.load(config_file)
	except :
		print "[Error] Cannot find the " + config_file +" file"
		usage()
		sys.exit(1)

	producer_count = config["producer"]["count"]
	consumer_count = config["consumer"]["count"]


	# write system information
	date = strftime("%Y%m%d-%H%M", localtime())
	output_path = "./results/%s" % date
	if not os.path.isdir(output_path):
		os.makedirs(output_path)

	sysInfoFile = open("%s/%s" % (output_path, systemInfoFName), 'w')
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
	for i in range( 0, len(TEST_LIST) ):
		for ii in range(0, len(producer_count) ):
			for t in range( 0, 3 ):

				##########################
				# Setup output directories
				##########################

				exp_output_path = "%s/%s" % (output_path, TEST_LIST[i])

				print "exp_output_path: %s\n" % exp_output_path
				if not os.path.isdir(exp_output_path):
					os.makedirs(exp_output_path)

				outputFile = open("%s/%s-%s-%s-%s" % (exp_output_path, TEST_LIST[i], producer_count[ii], consumer_count[ii], t), 'w')
				print "outputFile: %s" % outputFile

				print "\nDropping page cache"
				cmd = "echo 3 | sudo tee /proc/sys/vm/drop_caches"
				p = Popen( shlex.split(cmd) )
				p.wait()

				start_time = time.time()
				cmd = "%s/%s/%s %s %s" % (RUN_BASE,TEST_LIST[i], TEST_LIST[i], producer_count[ii], consumer_count[ii])
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




