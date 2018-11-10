import pandas as pd
import matplotlib as mpl
import matplotlib.pyplot as plt

from script.config import Config

class Graph:
    
	@classmethod
	def __new__(cls, self, objs, directory, date, hyper_option):
		graph_directory = cls.makeGraphDirectoryName(directory, date)
		cls.drawGraph(objs, graph_directory, hyper_option)
		return True
        