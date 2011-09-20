#!/usr/bin/env python
# -*- coding: utf-8 -*-
# vi:ts=4 sw=4 et

import numpy
import re
import sys
import time

from numpy import matrix, array, cross, dot, rad2deg, deg2rad
from numpy.linalg import norm


def vector(param):
    '''Constructor for a 3D vector using numpy.array'''
    return numpy.array(param, dtype='f')

def vector_from_string(s):
    try:
        x,y,z = [float(i) for i in s.split()]
        return vector([x,y,z])
    except:
        return None


cos_between_vectors = lambda A,B: dot(A,B)/norm(A)/norm(B)


class State(object):
    CALIBRATION_NAMES = ['topleft', 'topright', 'bottomright', 'bottomleft']

    def __init__(self):
        self.DEBUG = False
        for name in self.CALIBRATION_NAMES:
            setattr(self, name, vector([0,0,0]))


    def single_edge_interpolation(self, A, B, C):
        # A and B are one of the corners.
        # C is the currently pointed value.
        #
        # Returns None in case of errors.
        #
        # Returns None if C is at the opposite side of N, when looking at the
        # plane AOB. In other words, if the angle between N and C is greater
        # than 90 degrees.
        #
        # Else, returns a value between 0.0 and 1.0, which means how close to A
        # is C, inside the segment AB.
        #
        #
        # N ^
        #   |   _
        #   |  .'| B (pointing inside this drawing)
        #   |.'
        # O *--------> A
        #
        # N = A x B

        # N is normal to the plane of A and B
        N = cross(A, B)
        # Converting to unit vector
        # N /= norm(N)

        # Checking the side of C, in relation to N and plane AB
        if dot(N, C) < 0:
            return None

        # Clinha is the projection of C onto AB plane.
        # I don't care about the size of this vector, only about the direction.
        # http://www.euclideanspace.com/maths/geometry/elements/plane/lineOnPlane/index.htm
        Clinha = cross(N, cross(C,N))

        # Comparing the cossines...
        # I could compare the angles, but that would need arccos() function,
        # while I can calculate the cossines directly by dot-product and
        # division.
        # Bah... nevermind... I'm going to compare the angles until I get a
        # better solution
        cos_AB = cos_between_vectors(A, B)
        cos_AC = cos_between_vectors(A, Clinha)
        cos_BC = cos_between_vectors(B, Clinha)
        if self.DEBUG:
            print "cos_AB", cos_AB
            print "cos_AC", cos_AC
            print "cos_BC", cos_BC

        # Check if inside the bounds of AB
        #if cos_AC < cos_AB or cos_BC < cos_AB:
        #    return None

        if not (
            -1 <= cos_AB <= 1 and
            -1 <= cos_AC <= 1 and
            -1 <= cos_BC <= 1
        ):
            return None
        ang_AB = numpy.arccos(cos_AB)
        ang_AC = numpy.arccos(cos_AC)
        ang_BC = numpy.arccos(cos_BC)
        if self.DEBUG:
            print "ang_AB", ang_AB
            print "ang_AC", ang_AC
            print "ang_BC", ang_BC

        # Return the proportion...
        return ang_AC / ang_AB

    def interpolation_using_2_edges(self, pointer):
        # This is a very bad approximation
        x = self.single_edge_interpolation(self.topleft   , self.topright, pointer)
        y = self.single_edge_interpolation(self.bottomleft, self.topleft , pointer)
        if None in [x, y]:
            return (None, None)

        y = 1 - y

        return (x, y)

    def interpolation_using_4_edges(self, pointer):
        # Let:
        #  A = topleft
        #  B = topright
        #  C = bottomright
        #  D = bottomleft
        # as 3D vector coordinates
        #
        # And let:
        #  AB = pointer projection at AB edge
        #  BC = pointer projection at BC edge
        #  DC = pointer projection at DC edge
        #  AD = pointer projection at AD edge
        # as 1D coordinates, already normalized between 0 and 1
        #
        # Let's trace a line joining AB and DC, and another joining AD and BC.
        # The intersection of these two lines should be at the 2D screen
        # coordinate pointed by the user.
        #
        # y(x) = x * (BC - AD) + AD
        #   y(0) = AD
        #   y(1) = BC
        #
        # x(y) = y * (DC - AB) + AB
        #   x(0) = AB
        #   x(1) = DC
        #
        # Some math later:
        # x = (AD * (DC - AB) + AB) / (1 - (BC - AD) * (DC - AB))

        AB = self.single_edge_interpolation(self.topleft    , self.topright   , pointer)
        BC = self.single_edge_interpolation(self.topright   , self.bottomright, pointer)
        DC = self.single_edge_interpolation(self.bottomright, self.bottomleft , pointer)
        AD = self.single_edge_interpolation(self.bottomleft , self.topleft    , pointer)

        if None in [AB, BC, DC, AD]:
            return (None, None)

        DC = 1 - DC
        AD = 1 - AD

        x = (AD * (DC - AB) + AB) / (1 - (BC - AD) * (DC - AB))
        y = x * (BC - AD) + AD

        return (x, y)


def reset():
    global state
    state = State()


def parse_args():
    import argparse

    parser = argparse.ArgumentParser(
        description='Converts 3D vector coordinates to 2D screen coordinates',
        formatter_class=argparse.ArgumentDefaultsHelpFormatter
    )

    parser.add_argument(
        '-s', '--sleep',
        action='store',
        type=float,
        default=0.0,
        metavar='MS',
        dest='sleep_ms',
        help='Sleep MS milliseconds after each printed coordinate'
    )
    parser.add_argument(
        '-f', '--flush',
        action='store_true',
        dest='force_flush',
        help='Force stdout flush after each printed coordinate (automatically enabled if --sleep is set)'
    )

    args = parser.parse_args()
    return args


def main():
    re_vector_line = re.compile(r'^\s*([-\d]+)\s*([-\d]+)\s*([-\d]+)')

    global options
    options = parse_args()

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
        elif line.lower() == 'debug':
            state.DEBUG = not state.DEBUG
            print "Debug is now {0}".format("ON" if state.DEBUG else "OFF")
            sys.stdout.flush()
        elif line.lower() == 'calibration':
            # Prints the calibration vectors
            for name in State.CALIBRATION_NAMES:
                print "{0}:\t{1}".format(
                    name,
                    repr(getattr(state, name))
                )
            sys.stdout.flush()

        # Calibration coordinates
        elif line.lower() in State.CALIBRATION_NAMES:
            value = raw_input().strip()
            pointer = vector_from_string(value)
            if pointer is not None:
                setattr(state, line.lower(), pointer)

        # Reading a coordinate
        elif match_vector_line:
            pointer = vector_from_string(line)
            if pointer is None:
                continue

            if state.DEBUG:
                print repr(pointer)

            x, y = state.interpolation_using_4_edges(pointer)

            if state.DEBUG:
                print "x,y", x, y

            if x is not None and y is not None:
                #x *= 640
                #y *= 480
                print "{0} {1}".format(x, y)

                if options.sleep_ms > 0 or options.force_flush:
                    sys.stdout.flush()

                if options.sleep_ms > 0:
                    #time.sleep(0.015625)  # 2**-6
                    #time.sleep(2**-8)
                    time.sleep(options.sleep_ms/1000.0)
            else:
                print "discarded"

        # Fallback for unknown lines
        else:
            print "Unrecognized line {0}: {1}".format(line_number, line)
            sys.stdout.flush()


if __name__ == "__main__":
    main()
