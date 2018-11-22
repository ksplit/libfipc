import csv
import numpy
import os
import re
import json

from script.config import Config

p = re.compile('[0-9]+')

class Extractor:

	@classmethod
	def __new__(cls, self, directory, hyper_option):
		files = cls.extractFiles(directory)
		return cls.extract(files, hyper_option)

	@classmethod
	def extractFiles(cls, directory):
		result_files = []
		
		for root, dirs, files in os.walk(directory):
			for file in files:
				result_files.append(root+"/"+file)

		return result_files

