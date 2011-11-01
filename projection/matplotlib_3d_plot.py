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


def parse_args():
    import argparse

    parser = argparse.ArgumentParser(
        description='Uses matplotlib to draw a 3D plot.',
        formatter_class=argparse.ArgumentDefaultsHelpFormatter
    )
    parser.add_argument(
        '-l', '--line',
        action='store_true',
        help='Draw a 3D line'
    )
    parser.add_argument(
        '-p', '--points',
        action='store_true',
        help='Draw 3D points (scatter plot)'
    )
    parser.add_argument(
        '--2d',
        metavar='PLANE',
        action='store',
        choices=['xy', 'xz', 'yz'],
        dest='twodee',
        help='Draw 2D graph using one of these planes: xy, xz, yz'
    )
    parser.add_argument(
        '-x', '--limit',
        metavar='LIMIT',
        action='store',
        type=int,
        default=250,
        help='Specify the limit of axes'
    )
    parser.add_argument(
        '-q', '--quiet',
        action='store_true',
        help='Don\'t open the matplotlib window'
    )
    parser.add_argument(
        '-o', '--output',
        metavar='FILE',
        action='append',
        help='Save the plot to a file (can be specified multiple times)'
    )
    parser.add_argument(
        '--dpi',
        action='store',
        type=int,
        default=300,
        help='Specify the DPI when saving the plot'
    )
    parser.add_argument(
        '-f', '--fontsize',
        action='store',
        type=int,
        default=12,
        help='Specify the font size'
    )
    parser.add_argument(
        '-s', '--size',
        action='store',
        type=float,
        nargs=2,
        default=(7.0, 7.0),
        help='Size (in inches)'
    )
    parser.add_argument(
        'input_files',
        metavar='FILE',
        type=file,
        nargs='*',
        help='Optional input files'
    )
    args = parser.parse_args()

    if args.line == args.points:
        args.line = True
        args.points = False

    return args


def matplotlib_configure(args):
    # http://stackoverflow.com/questions/3899980/how-to-change-the-font-size-on-a-matplotlib-plot
    # Default font size is 12
    matplotlib.rcParams['font.size'] = args.fontsize

    # This is not necessary, but doesn't hurt.
    pyplot.close()

    fig = pyplot.figure()
    fig.set_size_inches(args.size)


def plot_2d(args, input_files):
    pyplot.grid(True)

    # if twodee is 'xy', d1 will be 'x' and d2 will be 'y'
    d1 = args.twodee[0]
    d2 = args.twodee[1]

    #colors = ['b', 'g', 'r', 'c', 'm', 'y', 'k']
    #shapes = ['o', '^', 's', '*']

    for (i,f) in enumerate(input_files):
        data = load_data(f)
        if args.points:
            # s=size, c=color, marker='o'
            # Size is hardcoded here
            #pyplot.scatter(
            #    getattr(data, d1), getattr(data, d2),
            #    marker=shapes[i%len(shapes)], c=colors[i%len(colors)],
            #    #s=20, linewidths=0
            #)
            pyplot.scatter(getattr(data, d1), getattr(data, d2))
        elif args.line:
            pyplot.plot(getattr(data, d1), getattr(data, d2))

    #pyplot.axis('equal')

    pyplot.xlim(-args.limit, args.limit)
    pyplot.ylim(-args.limit, args.limit)


def plot_3d(args, input_files):
    # Based on "lines3d_demo.py" and "scatter3d_demo.py" from matplotlib

    # Reducing the margins
    pyplot.subplots_adjust(left=0, right=1, bottom=0, top=1)
    # Default values:
    # left  = 0.125  # the left side of the subplots of the figure
    # right = 0.9    # the right side of the subplots of the figure
    # bottom = 0.1   # the bottom of the subplots of the figure
    # top = 0.9      # the top of the subplots of the figure
    # wspace = 0.2   # the amount of width reserved for blank space between subplots
    # hspace = 0.2   # the amount of height reserved for white space between subplots

    fig = pyplot.gcf()

    #ax = fig.gca(projection='3d')
    ax = fig.add_subplot(111, projection='3d')
    ax.grid(True)

    for f in input_files:
        data = load_data(f)
        if args.points:
            # s=size, c=color, marker='o'
            # Size is hardcoded here
            ax.scatter(data.x, data.y, data.z, s=1, marker='o', linewidths=0)
        elif args.line:
            ax.plot(data.x, data.y, data.z)

    # This is buggy, as of version 1.0.1
    #ax.set_aspect('equal')

    ax.set_xlim3d(-args.limit, args.limit)
    ax.set_ylim3d(-args.limit, args.limit)
    ax.set_zlim3d(-args.limit, args.limit)

    # Rotating the view
    #ax.view_init(elev=30, azim=-60)  # default
    #ax.view_init(elev=45, azim=45)


def main():
    args = parse_args()

    input_files = args.input_files
    if not args.input_files:
        input_files = [sys.stdin]


    matplotlib_configure(args)
    if args.twodee:
        plot_2d(args, input_files)
    else:
        plot_3d(args, input_files)


    if args.output:
        for filename in args.output:
            pyplot.savefig(filename, dpi=args.dpi)

    if not args.quiet:
        pyplot.show()

if __name__ == '__main__':
    main()
