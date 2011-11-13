#!/usr/bin/env python
# -*- coding: utf-8 -*-
# vi:ts=4 sw=4 et

from __future__ import print_function

import sys
import time

from math import sin, cos, radians, degrees


# argparse is beautiful!
# This var will be written by parse_args()
options = None


def parse_args(args=None):
    global options

    import argparse

    class HackishFormatterClass(argparse.ArgumentDefaultsHelpFormatter, argparse.RawDescriptionHelpFormatter):
        # This is a hack to join two formatter classes into one.
        # Why is this a hack? Because of the following comment:
        #
        # (Also note that HelpFormatter and RawDescriptionHelpFormatter are only
        # considered public as object names -- the API of the formatter objects is
        # still considered an implementation detail.)
        # http://hg.python.org/cpython/file/2.7/Lib/argparse.py#l59

        pass

    parser = argparse.ArgumentParser(
        description='Generates 3D vectors (x,y,z) around a sphere',
        epilog=
        'When printing the calibration vectors, the "aperture angles" define how far\n'
        'apart are each corner of the pyramid, while the "offset angles" define the\n'
        'orientation of the pyramid.\n'
        '\n'
        'Remember: pitch=theta=vertical [-90, 90]; yaw=phi=horizontal [-180, 180]',
        # If the next line breaks, try one of the others:
        formatter_class=HackishFormatterClass
        #formatter_class=argparse.ArgumentDefaultsHelpFormatter
        #formatter_class=argparse.RawDescriptionHelpFormatter
    )

    parser.add_argument(
        '-r', '--radius',
        action='store',
        type=int,
        default=200,
        help='The radius of the sphere, i.e. the size (norm) of each vector'
    )
    parser.add_argument(
        '--phi-range', '--h-range',
        action='store',
        type=int,
        nargs=3,
        default=(90, -91, -1),
        metavar=('START', 'STOP', 'STEP'),
        dest='phi_range',
        help='Phi (horizontal) range when printing the vectors, passed directly to Python\'s range() function'
    )
    parser.add_argument(
        '--theta-range', '--v-range',
        action='store',
        type=int,
        nargs=3,
        default=(45, -45, -2),
        metavar=('START', 'STOP', 'STEP'),
        dest='theta_range',
        help='Theta (vertical) range when printing the vectors, passed directly to Python\'s range() function'
    )

    parser.add_argument(
        '-P', '--PHI', '--h-aperture',
        action='store',
        type=int,
        default=45,
        metavar='ANGLE',
        dest='phi_aperture',
        help='Aperture angle (horizontal - phi) for the calibration pyramid'
    )
    parser.add_argument(
        '-T', '--THETA', '--v-aperture',
        action='store',
        type=int,
        default=45,
        metavar='ANGLE',
        dest='theta_aperture',
        help='Aperture angle (vertical - theta) for the calibration pyramid'
    )
    parser.add_argument(
        '-p', '--phi', '--h-offset',
        action='store',
        type=int,
        default=0,
        metavar='ANGLE',
        dest='phi_offset',
        help='Offset angle (horizontal - phi) for the calibration pyramid'
    )
    parser.add_argument(
        '-t', '--theta', '--v-offset',
        action='store',
        type=int,
        default=0,
        metavar='ANGLE',
        dest='theta_offset',
        help='Offset angle (vertical - theta) for the calibration pyramid'
    )
    parser.add_argument(
        '-C', '--no-calibration',
        action='store_true',
        dest='omit_calibration',
        help='Don\'t print the calibration vectors at the start'
    )
    parser.add_argument(
        '-s', '--sleep',
        action='store',
        type=float,
        default=0.0,
        metavar='MS',
        dest='sleep_ms',
        help='Sleep MS milliseconds after each theta value'
    )

    options = parser.parse_args(args)


def print_calibration():
    global options

    pitch = options.theta_aperture/2
    yaw = options.phi_aperture/2

    offsets = [
        #(delta_theta, delta_phi, name)
        (+pitch, +yaw, 'topleft'),
        (+pitch, -yaw, 'topright'),
        (-pitch, -yaw, 'bottomright'),
        (-pitch, +yaw, 'bottomleft'),
    ]
    for delta_theta, delta_phi, name in offsets:
        x, y, z = spherical_to_cartesian(
            options.theta_offset + delta_theta,
            options.phi_offset + delta_phi
        )
        print(name)
        print('{0}\t{1}\t{2}'.format(x, y, z))

def spherical_to_cartesian(theta, phi):
    global options

    # These are not exactly the same equations as in:
    # http://en.wikipedia.org/wiki/Spherical_coordinates

    x = options.radius * cos(radians(theta)) * cos(radians(phi))
    y = options.radius * cos(radians(theta)) * sin(radians(phi))
    z = options.radius * sin(radians(theta))

    x = int(round(x))
    y = int(round(y))
    z = int(round(z))

    return x, y, z

def main():
    global options

    parse_args()

    if not options.omit_calibration:
        print_calibration()

    for theta in xrange(*options.theta_range):
    #for theta in xrange(45, -45, -2):
    #for theta in xrange(75, -75, -5):
        for phi in xrange(*options.phi_range):
        #for phi in xrange(-90, 90, 1):
        #for phi in xrange(0, 360, 5):
            x, y, z = spherical_to_cartesian(theta, phi)
            print('{0}\t{1}\t{2}'.format(x, y, z))

        if options.sleep_ms > 0:
            # Flushing the output
            sys.stdout.flush()

            # Printing the current theta value
            #sys.stderr.write('theta={0}\n'.format(theta))
            #sys.stderr.flush()

            time.sleep(options.sleep_ms/1000.0)


if __name__ == "__main__":
    main()
