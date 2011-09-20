#!/usr/bin/env python
# -*- coding: utf-8 -*-
# vi:ts=4 sw=4 et

# How to use:
# ./something_that_generates_points | ./draw_points.py
# ./draw_points.py < some_data.txt
#
# Once the window is opened, it can be resized, or it can be closed by pressing
# Esc or Q, or by just closing it.
#
# Upon reading EOF, it will stop reading from stdin, but the window will remain
# open until you close it.


from __future__ import division
from __future__ import print_function

import select
import sys
from itertools import izip

import pygame
from pygame.locals import *


class DrawPoints(object):

    def init(self, args):
        pygame.init()

        if args.persist:
            self.FADE_POINTS = False
            self.MAX_POINTS = 0
            self.colors = []
            self.points = []
        else:
            self.FADE_POINTS = True
            self.MAX_POINTS = 256
            self.colors = [
                Color(i,i,i) for i in range(256)
            ]
            self.points = [ (None, None) ] * self.MAX_POINTS

        self.BLACK = Color(0, 0, 0)
        self.WHITE = Color(255, 255, 255)

        self.resolution = args.window_size
        self.thickness = args.size

        self.poll = select.poll()
        self.poll.register(sys.stdin, select.POLLIN)

        self.save_as = args.output


    def run(self):
        self.screen = pygame.display.set_mode(self.resolution, RESIZABLE)
        pygame.display.set_caption("Drawing points from stdin")

        # Inject a USEREVENT every 10ms
        pygame.time.set_timer(USEREVENT, 10)

        while True:
            self.redraw()

            for event in [pygame.event.wait(),]+pygame.event.get():
                if event.type == QUIT:
                    self.quit()
                elif event.type == KEYDOWN and event.key in [K_ESCAPE, K_q]:
                    self.quit()

                elif event.type == VIDEORESIZE:
                    self.resolution = event.size
                    self.screen = pygame.display.set_mode(self.resolution, RESIZABLE)

                elif event.type == USEREVENT:
                    # poll() returns a list of file-descriptors that are "ready"
                    while self.poll.poll(0):
                        line = sys.stdin.readline()
                        if line == "":
                            # EOF
                            self.poll.unregister(sys.stdin)
                            pygame.time.set_timer(USEREVENT, 0)
                        try:
                            x,y = [float(i) for i in line.strip().split()]
                        except:
                            continue
                        self.add_point(x,y)


    def add_point(self, x, y):
        if self.FADE_POINTS:
            self.points.append( (x,y) )
            if len(self.points) > self.MAX_POINTS:
                self.points.pop(0)
        else:
            self.draw_point(x, y, self.WHITE)

    def draw_point(self, x, y, color):
        x *= self.resolution[0]
        y *= self.resolution[1]
        rect = Rect(
            x - self.thickness, y - self.thickness,
            1 + 2 * self.thickness, 1 + 2 * self.thickness
        )
        self.screen.fill(color, rect=rect)

    def redraw(self):
        if self.FADE_POINTS:
            self.screen.fill(self.BLACK)
            for point, color in izip(self.points, self.colors):
                if point[0] is None or point[1] is None:
                    continue
                self.draw_point(point[0], point[1], color)
        else:
            pass

        pygame.display.flip()


    def quit(self, status=0):
        if self.save_as:
            print("Saving image to '{0}'".format(self.save_as))
            pygame.image.save(self.screen, self.save_as)

        pygame.quit()
        sys.exit(status)


def parse_args():
    import argparse

    parser = argparse.ArgumentParser(
        description="Draws 2D points based on coordinates between 0.0 and 1.0",
        formatter_class=argparse.ArgumentDefaultsHelpFormatter
    )
    parser.add_argument(
        '-p', '--persist',
        action='store_true',
        help='Persist the drawing, instead of fading the older points.'
    )
    parser.add_argument(
        '-s', '--size',
        action='store',
        type=int,
        default=2,
        help='The thickness of each "dot". The actual size is "1 + 2*SIZE".'
    )
    parser.add_argument(
        '-w', '--window',
        action='store',
        type=int,
        nargs=2,
        default=(640, 480),
        dest='window_size',
        help='The size of the window'
    )
    parser.add_argument(
        '-o', '--output',
        metavar='FILE',
        action='store',
        type=str,
        help='Save the final drawing to a file.'
    )
    args = parser.parse_args()
    return args

if __name__ == "__main__":
    program = DrawPoints()
    program.init(parse_args())
    program.run()  # This function should never return
