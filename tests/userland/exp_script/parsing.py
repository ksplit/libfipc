import os, sys
import csv, re, numpy

cnt=['1','2','4','8','16']
p_list={'1':[], '2':[], '4':[], '8':[], '16':[]}
c_list={'1':[], '2':[], '4':[], '8':[], '16':[]}

def usage():
    print 'Usage: %s -f result.csv' % sys.argv[0]

def main(argv=None):

    resultFName = None
    testFName = None

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
        elif o in ('f', '--file'):
            resultFName = a
        elif t in ('t', '--test'):
            testFName = a
        else:
            assert False, 'unhandled option'
    
    p = re.compile('[0-9]+')
    resultFile = open(resultFName,'a')
    wr = csv.writer(resultFile)

    for root, dirs, files in os.walk(RESULT_PATH_BENCH):
        for fname in files:
            bench,p_cnt, c_cnt,try_cnt,opt = fname.split('-')

            f = open(RESULT_PATH_BENCH+'/'+fname,'r')
            l = f.readlines()
            i = 0
            while i < len(l):
                line = l[i]
                i+=1
                if 'finished' in line:
                    if 'Producer' in line:
                        m = p.findall(line)
                        print 'producer ( ' + p_cnt +' )' +m[2]

                        p_list[p_cnt].append(int(m[2]));    
                    if 'Consumer' in line:
                        m = p.findall(line)
                        c_list[c_cnt].append(int(m[2]));
        print bench
        print 'producer '
        print p_list
        print 'Consumer'
        print c_list    
        key = p_list.keys()
        wr.writerow([bench,'10^2'])
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


        p_list={'1':[], '2':[], '4':[], '8':[], '16':[]}
        c_list={'1':[], '2':[], '4':[], '8':[], '16':[]}


                        

    f = open('result_spsc.csv','a')
    wr = csv.writer(f)
    wr.writerow([RESULT_PATH_BENCH])
    for i in range(0,len(p_list)):
        wr.writerow([cnt[i],p_list[cnt[i]], c_list[cnt[i]]])
    
    f.close()
if __name__ == "__main__":
    sys.exit(main())