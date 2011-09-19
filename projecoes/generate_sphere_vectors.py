#!/usr/bin/env python
# -*- coding: utf-8 -*-
# vi:ts=4 sw=4 et

from math import sin, cos, radians, degrees

def main(args):
    radius = 200
    for theta in range(75, -75, -5):
        for phi in range(0, 360, 5):
            # http://en.wikipedia.org/wiki/Spherical_coordinates
            x = radius * cos(radians(theta)) * cos(radians(phi))
            y = radius * cos(radians(theta)) * sin(radians(phi))
            z = radius * sin(radians(theta))
            print '{0}\t{1}\t{2}'.format(
                int(round(x)),
                int(round(y)),
                int(round(z))
            )


if __name__ == "__main__":
    import sys
    main(sys.argv[1:])
