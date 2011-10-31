#!/usr/bin/env python
# -*- coding: utf-8 -*-
# vi:ts=4 sw=4 et

from __future__ import division
from __future__ import print_function

import sys

import matplotlib
from mpl_toolkits.mplot3d import Axes3D
from matplotlib import pyplot

import numpy
#from numpy import linspace


class Data(object):
    def __init__(self):
        self.x = []
        self.y = []
        self.z = []


def load_data(f):
    data = Data()

    for line in f:
        try:
            x, y, z = [float(i) for i in line.strip().split()]
            data.x.append(x)
            data.y.append(y)
            data.z.append(z)
        except:
            pass

    return data


def plot_data(data):
    # Based on "lines3d_demo.py" and "scatter3d_demo.py" from matplotlib

    #matplotlib.rcParams['legend.fontsize'] = 10

    fig = pyplot.figure()
    #ax = fig.gca(projection='3d')
    ax = fig.add_subplot(111, projection='3d')
    ax.set_aspect('equal')

    # s=size, c=color, marker='o'
    ax.scatter(data.x, data.y, data.z, s=1, marker='o', linewidths=0)
    #ax.plot(data.x, data.y, data.z)

    ax.grid(True)

    ax.set_xlim3d(-250, 250)
    ax.set_ylim3d(-250, 250)
    ax.set_zlim3d(-250, 250)

    # Rotating the view
    #ax.view_init(elev=30, azim=-60)  # default
    #ax.view_init(elev=45, azim=45)


def main():
    data = load_data(sys.stdin)

    plot_data(data)
    pyplot.show()


if __name__ == '__main__':
    main()
