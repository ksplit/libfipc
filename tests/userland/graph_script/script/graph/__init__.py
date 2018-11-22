import pandas as pd
import matplotlib as mpl
import matplotlib.pyplot as plt

from script.config import Config

class Graph:
    
	@classmethod
	def __new__(cls, self, objs, directory, draw_opts):
		graph_directory = cls.makeGraphDirectoryName(directory)
		cls.drawGraph(objs, graph_directory, draw_opts)
		return True
        