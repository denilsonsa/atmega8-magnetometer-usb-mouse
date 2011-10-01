#!/usr/bin/env python
# -*- coding: utf-8 -*-
# vi:ts=4 sw=4 et

from __future__ import division
from __future__ import print_function

import matplotlib.pyplot as pyplot


class Row(object):
    def __init__(self, rev, node, size, date, desc):
        self.rev = int(rev)
        self.node = node
        self.size = int(size)
        # Might be a great idea to convert this to a datetime object
        self.date = date
        self.desc = desc

def load_data(f):
    data = []

    for line in f:
        line = line.strip()
        # Ignoring comments and empty lines
        if line.startswith('#') or line == '':
            continue

        fields=line.split('\t', 4)
        row = Row(*fields)
        data.append(row)

    return data

def main():
    with open('size_vs_commit.txt', 'r') as f:
        data = load_data(f)

    pyplot.plot(
        [row.rev for row in data],
        [row.size for row in data],
    )

    pyplot.grid(True)
    #pyplo.xlim(0, max_rev)
    #pyplo.ylim(0, 8*1024)

    pyplot.show()

if __name__ == "__main__":
    main()
