import re

f = open("FAI.txt", "r")
s = f.readlines()
thread_num = []
increment = []
average_time = []
result = []
for sentence in s:
	if "Thread" in sentence:
		sentence = sentence.replace(",", "")
		numbers = re.findall("\d+", sentence)
		isThreadNum = True
		for number in numbers:
			if isThreadNum:
				thread_num.append(number)
				isThreadNum = False
			else:
				increment.append(number)
				isThreadNum = True
	elif "Average" in sentence:
		sentence = sentence.replace(",", "")
		numbers = re.findall("\d+", sentence)
		isAverageTime = True
		for number in numbers:
			if isAverageTime:
				average_time.append(number)
				isAverageTime = False
			else:
				isAverageTime = True

testcase = len(thread_num)

for test in range(testcase):
	result_str = thread_num[test] + " " + increment[test] + " " + average_time[test]
	print(result_str)
	result.append(result_str) 

newf = open("FAI_result.txt", "w")
newf.writelines(["%s\n" % item for item in result])

