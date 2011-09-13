#!/usr/bin/env python
# -*- coding: utf-8 -*-
# vi:ts=4 sw=4 et

import numpy
import re
import sys

from numpy import matrix, array, cross


def vector(param):
    '''Constructor for a 3D vector using numpy.matrix'''
    return numpy.matrix(param, dtype='f')


class State(object):
    CALIBRATION_NAMES = ['topleft', 'topright', 'bottomright', 'bottomleft']

    def __init__(self):
        for name in self.CALIBRATION_NAMES:
            setattr(self, name, vector([0,0,0]))


def reset():
    global state
    state = State()


def main():
    re_vector_line = re.compile(r'^\s*([-\d]+)\s*([-\d]+)\s*([-\d]+)')

    global state
    reset()
    line_number = 0

    while True:
        line_number += 1

        try:
            line = raw_input().strip()
        except EOFError:
            break

        match_vector_line = re_vector_line.match(line)

        if line == '':
            # Ignoring empty line
            pass
        elif line[0] == '#':
            # Ignoring comment line
            pass

        # Commands...
        elif line.lower() == 'reset':
            reset()
        elif line.lower() == 'quit':
            break
        elif line.lower() == 'calibration':
            # Prints the calibration vectors
            for name in State.CALIBRATION_NAMES:
                print "{0}:\t{1}".format(
                    name,
                    repr(getattr(state, name))
                )

        # Calibration coordinates
        elif line.lower() in State.CALIBRATION_NAMES:
            value = raw_input().strip()
            setattr(state, line.lower(), vector(value))

        # Reading a coordinate
        elif match_vector_line:
            print repr(vector(line))

        # Fallback for unknown lines
        else:
            print "Unrecognized line {0}: {1}".format(line_number, line)


if __name__ == "__main__":
    main()
