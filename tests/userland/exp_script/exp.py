import threading, requests
import sys, os, time, shlex, signal, atexit
import json, getopt
import smtplib
from email.mime.text import MIMEText
from subprocess import *
from time import localtime, strftime

# Default profiling options
RUN_BASE = "/users/ji1/fast-ipc-module/tests/userland"
TEST_LIST = [ 'queue_mpsc_mcslock' ]
systemInfoFName = 'system.info'
expInfoFName = 'exp.info'
iterCnt = 1
opt = [ 1 ]
# usage
def usage():
	print 'Usage: %s -c config.json' % sys.argv[0]

# email notification

def send_email(user, pwd):

	# content
	try:
		server = smtplib.SMTP("smtp.gmail.com", 587)
		server.ehlo()
		server.starttls()
		server.login(user,pwd)
		server.sendmail(user, user, 'Experiments finished')
		server.close()
		print "email successfully sent.\n"
	except:
		print "failed to send mail"

# Main function
def main(argv=None):

	####################
	# Read Configuration
	####################
	config_file = None
	email = None
	pw = None
	try:
		opts,args = getopt.getopt(sys.argv[1:], 'hc:e:p:', ['help', 'config=', 'email=', 'pass='])
	except getopt.GetoptError as err:
		print (err)
		sys.exit(1)

	for o, a in opts:
		if o in ('-h', '--help'):
			usage()
			sys.exit()
		elif o in ('-c', '--config'):
			config_file = a
		elif o in ('-e', '--email'):
			email = a
		elif o in ('-p', '--pass'):
			pw = a
		else:
			assert False, 'unhandled option'

	if config_file == None:
		print "[Error] Please specify the configuration file"
		usage()
		sys.exit(1)

	if email and pw == None:
		print "[Error] Please specify the password"
		sys.exit(1)
	try:
		with open(config_file) as config_file:
			config = json.load(config_file)
	except :
		print "[Error] Cannot find the [ " + config_file + " ] file"
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

	# write experiment information
	expInfoFile = open("%s/%s" % (output_path, expInfoFName), 'w')
	expInfoFile.write("Expreriment Information\n")
	expInfoFile.write("--------------------------------------------------\n")
	expInfoFile.write("1. Expreriment List\n")
	for i in range( 0, len(TEST_LIST) ):
		expInfoFile.write("\n[%d] %s\n" % (i+1, TEST_LIST[i]))
	expInfoFile.write("\n2. Node allocate option: ")

	for i in range( 0, len(opt) ):
		expInfoFile.write("%s" % opt[i])
		if i != (len(opt)-1):
			expInfoFile.write(", ")
	expInfoFile.write("\n\n3. Iterate Count: %d" % iterCnt) 
	expInfoFile.write("\n\n4. Experiment detail info") 


	####################
	# Running benchmark
	####################
	try_cnt=1
	for i in range( 0, len(TEST_LIST) ):
		for ii in range(0, len(producer_count) ):
			for t in range( 0, iterCnt ):
				for o in range (0, len(opt)):
					##########################
					# Setup output directories
					##########################

					exp_output_path = "%s/%s" % (output_path, TEST_LIST[i])

					print "exp_output_path: %s\n" % exp_output_path
					if not os.path.isdir(exp_output_path):
						os.makedirs(exp_output_path)

					outputFile = open("%s/%s-%s-%s-%s-%s" % (exp_output_path, TEST_LIST[i], producer_count[ii], consumer_count[ii], t, opt[o]), 'w')
					print "outputFile: %s" % outputFile

					print "\nDropping page cache"
					cmd = "echo 3 | sudo tee /proc/sys/vm/drop_caches"
					p = Popen( shlex.split(cmd) )
					p.wait()

					start_time = time.time()

					cmd = "%s/%s/%s %s %s %s" % (RUN_BASE,TEST_LIST[i], TEST_LIST[i], producer_count[ii], consumer_count[ii], opt[o])
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
					expInfoFile.write("\n[%d] %s" % (try_cnt, TEST_LIST[i])) 
					expInfoFile.write("\n  * Output path\n %s" % exp_output_path) 
					expInfoFile.write("\n  * Time elapsed\n %rs" % (end_time-start_time))
					try_cnt += 1
					print "Done"			
					print "Waiting... to warm the states"
					time.sleep(40)
					
	expInfoFile.close()
	# send email notification
	send_email(email, pw)

if __name__ == '__main__' :
	sys.exit(main())




