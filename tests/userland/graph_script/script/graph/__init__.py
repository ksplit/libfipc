import pandas as pd
import matplotlib as mpl
import matplotlib.pyplot as plt

from script.config import Config

class Graph:
    
    @classmethod
    def __new__(cls, self, objs, file, hyper_option):
        cls.drawGraph(objs, file, hyper_option)
        return True
        