import os, sys, getopt
import csv, re, numpy

# list storing cycles/message


# Usage
def usage():
    print 'Usage: %s -t <testDir> -f <resultFName>' % sys.argv[0]

# Main function
def main(argv=None):

    resultFName = None
    testFName = None
    transCnt = 0
    cntCheck = 0
    p_list = {}
    c_list = {}

    # get arguments
    try:
        opts, args = getopt.getopt(sys.argv[1:], 'hf:t:', ['help', 'file=', 'test='])
    except getopt.GetoptError as err:
        print (err)
        sys.exit(1)

    for o, a in opts:
        if o in ('-h', '--help'):
            usage()
            sys.exit()
        elif o in ('-f', '--file'):
            resultFName= a
        elif o in ('-t', '--test'):
            testDir = a
        else:
            assert False, 'unhandled option'
    
    if resultFName == None:
        usage()
        sys.exit()

    if testDir == None:
        usage()
        sys.exit()

    p = re.compile('[0-9]+')
    f = open(resultFName+'.csv', 'a')
    wr = csv.writer(f)

    # Parse the files
    for root, dirs, files in os.walk(testDir):
        for fname in files:
            bench, p_cnt, c_cnt,try_cnt,opt = fname.split('-')
            if any(p_cnt in x for x in p_list) == False:
                p_list[p_cnt] = []
                c_list[c_cnt] = []

            f = open(testDir+'/'+fname,'r')
            l = f.readlines()
            i = 0
            while i < len(l):
                line = l[i]
                i+=1
                if 'finished' in line:
                    if 'Producer' in line:
                        m = p.findall(line)
                        p_list[p_cnt].append(int(m[2]))  
			if cntCheck == 0:
                            transCnt = int(m[1])
                            cntCheck = 1
                    if 'Consumer' in line:
                        m = p.findall(line)
                        c_list[c_cnt].append(int(m[2]))
        print bench
        print 'producer '
        print p_list
        print 'Consumer'
        print c_list    
        key = p_list.keys()
        wr.writerow([bench, transCnt])
        wr.writerow(['','avg','std','avg','std'])
        wr.writerow(['cnt','prod','prod','cons','cons'])

        for i in range(0,len(key)):
        	print 'key: '+key[i]
        	if p_list[key[i]]==[]:
        		continue
        	p_avg=numpy.mean(p_list[key[i]])
        	p_std=numpy.std(p_list[key[i]])
        	c_avg=numpy.mean(c_list[key[i]])
        	c_std=numpy.std(c_list[key[i]])
        	wr.writerow([key[i],p_avg,p_std,c_avg,c_std])

        # initialize 
        cntCheck = 0
        p_list = {}
        c_list = {}
                        
    # Make csv file
    wr.writerow([resultFName])
    for i in range(0,len(p_list)):
        wr.writerow([cnt[i],p_list[cnt[i]], c_list[cnt[i]]])
    
    f.close()

if __name__ == "__main__":
    sys.exit(main())
