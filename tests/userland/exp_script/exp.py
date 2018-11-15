import threading, requests
import sys, os, time, shlex, signal, atexit
import json, getopt
import smtplib
import io
from email.mime.text import MIMEText
from subprocess import *
from time import localtime, strftime

# Default profiling options
RUN_BASE = "/users/ji1/fast-ipc-module/tests/userland"
TEST_LIST = [ 'queue_mpmc_spinlock' ]
expInfoFName = 'exp.info'
iterCnt = 1
opt = [ 1 ]

# Show usage
def usage():
	print 'Usage: %s -c config.json' % sys.argv[0]

# Send google email notification
def send_email(user, pwd, resultDir):

	# content
	try:
		server = smtplib.SMTP("smtp.gmail.com", 587)
		server.ehlo()
		server.starttls()
		server.login(user,pw)
		msg= 'Experiments finished.\ndirectory: %s' % resultDic	
		server.sendmail(user, user, msg)
		server.close()
		print "email successfully sent.\n"
	except:
		print "failed to send mail"

# Main function
def main(argv=None):

	# Read Configuration
	config_file = None
	email = None
	pw = None
        
        # thread count
	producer_count = range(1,2)
	consumer_count = range(1,2)

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
			config_file = a
		else:
			assert False, 'unhandled option'

	if config_file != None:

		try:
			with open(config_file) as config_file:
				config = json.load(config_file)
		except :
			print "[Error] Cannot find the [ " + config_file + " ] file"
			usage()
			sys.exit(1)

		email = config["notification"]["id"]
		pw = config["notification"]["pw"]

	# Write system information
	date = strftime("%Y%m%d-%H%M", localtime())
	output_path = "./results/%s" % date
	if not os.path.isdir(output_path):
		os.makedirs(output_path)

	expInfoFile = open("%s/%s" % (output_path, expInfoFName), 'w')
	expInfoFile.write("Machine Information\n")
	expInfoFile.write("--------------------------------------------------\n")
	cmd = "lscpu" 
	p = Popen( shlex.split(cmd), stdout=PIPE )
	for line in p.stdout.readlines():
		expInfoFile.write(line)
	p.wait()

	# write experiment information
	expInfoFile.write("\nExpreriment Information\n")
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

	# Running benchmark
	for i in range( 0, len(TEST_LIST) ):
		for ii in range(0, len(producer_count) ):
			for t in range( 0, iterCnt ):
				for o in range (0, len(opt)):

					# Setup output directories
					exp_output_path = "%s/%s" % (output_path, TEST_LIST[i])

					print "exp_output_path: %s\n" % exp_output_path
					if not os.path.isdir(exp_output_path):
						os.makedirs(exp_output_path)

					outputFile = open("%s/%s-%s-%s-%s-%s" % (exp_output_path, TEST_LIST[i], producer_count[ii], consumer_count[ii], t, opt[o]), 'w')
					print "outputFile: %s" % outputFile

                                        # Dropping cache
					#print "\nDropping page cache"
					#cmd = "echo 3 | sudo tee /proc/sys/vm/drop_caches"
					#p = Popen( shlex.split(cmd) )
					#p.wait()

					start_time = time.time()

					cmd = "%s/%s/%s %s %s %s" % (RUN_BASE,TEST_LIST[i], TEST_LIST[i], producer_count[ii], consumer_count[ii], opt[o])
					print("------------------------------\n")
					print "\n[ (%d) try ] %s" % (t, cmd)
					p = Popen( cmd, stdout=PIPE,  shell=True )
					
					tryCnt = "try %d\n" % t
					print tryCnt

					outputFile.write(tryCnt)
					for line in p.stdout.readlines():
						print line
						outputFile.write(line)
					outputFile.close()

					end_time = time.time()
					print "Time elapsed: ", end_time-start_time
					print "Outpute path: ", output_path
					expInfoFile.write("\n[%d] %s" % (t+1, TEST_LIST[i])) 
					expInfoFile.write("\n  * Output path\n %s" % exp_output_path) 
					expInfoFile.write("\n  * Time elapsed\n %rs" % (end_time-start_time))
					print "Done"			
					#print "Waiting... to warm the states"
					#time.sleep(40)
					
	expInfoFile.close()
	# send email notification
	send_email(email, pw, os.path.abspath(output_path))

if __name__ == '__main__' :
	sys.exit(main())




