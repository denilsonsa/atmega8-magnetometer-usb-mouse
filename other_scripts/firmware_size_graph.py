#!/usr/bin/env python
# -*- coding: utf-8 -*-
# vi:ts=4 sw=4 et

from __future__ import division
from __future__ import print_function

from itertools import izip
import matplotlib.pyplot as pyplot


class Row(object):
    def __init__(self, rev, node, size1, size2, date, desc):
        self.rev = int(rev)
        self.node = node
        self.size1 = int(size1)
        self.size2 = int(size2)
        # Might be a great idea to convert this to a datetime object
        self.date = date
        self.desc = desc

    def __repr__(self):
        return 'Row({rev}, {node}, {size1}, {size2}, {date}, {desc})'.format(**self.__dict__)

def load_data(f):
    data = []

    for line in f:
        line = line.strip()
        # Ignoring comments and empty lines
        if line.startswith('#') or line == '':
            continue

        fields=line.split('\t', 5)
        row = Row(*fields)
        data.append(row)

    return data

def show_legend():
    l = pyplot.legend(loc='upper left', fancybox=True, shadow=True)
    #l = pyplot.legend(loc='best', fancybox=True, shadow=True)

    # Gray background
    l.get_frame().set_facecolor('0.90')

    for t in l.get_texts():
        t.set_fontsize('x-small')

def plot_but_skip_zeros(revs, sizes, *args, **kwargs):
    data = [
        x for x in izip(revs, sizes)
        if x[1] > 0
    ]
    pyplot.plot(
        [x[0] for x in data],
        [x[1] for x in data],
        *args, **kwargs
    )

def main():
    with open('size_vs_commit.txt', 'r') as f:
        data = load_data(f)

    max_rev = max(row.rev for row in data)
    flash_size = 8*1024
    bootloader_limit = 6*1024

    plot_but_skip_zeros(
        (row.rev for row in data),
        (row.size1 for row in data),
        label='make all'
    )
    plot_but_skip_zeros(
        (row.rev for row in data),
        (row.size2 for row in data),
        label='make combine'
    )

    pyplot.grid(True)
    show_legend()

    pyplot.xlim(0, max_rev)
    pyplot.xticks(range(0, max_rev+1, 25))
    pyplot.xlabel('Revision number')

    pyplot.ylim(0, flash_size)
    pyplot.yticks(range(0, flash_size+1, 512))
    pyplot.ylabel('Size (bytes)')

    # Bootloader limit
    pyplot.axhline(y=bootloader_limit, color='r')
    pyplot.annotate(
        u'Bootloader', xy=(0, bootloader_limit),
        xytext=(5, 5), textcoords='offset points',
        ha='left', va='bottom',
        color='r'
    )


    pyplot.show()

if __name__ == '__main__':
    main()
