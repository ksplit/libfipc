import csv
import numpy
import os
import re
import json

from script.config import Config

p = re.compile('[0-9]+')

class Extractor:

	@classmethod
	def __new__(cls, self, files, hyper_option):
		return cls.extract(files, hyper_option)

