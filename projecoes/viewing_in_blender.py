#!/usr/bin/env python
# -*- coding: utf-8 -*-
# vi:ts=4 sw=4 et

import bpy
import Blender
import sys
import numpy


# ./generate_sphere_vectors.py -c 0 > 3d.txt
# ./convert_coordinates.py < 3d.txt > 2d.txt
#
# Then run this script in blender.


# Global vars

# How many frames per each iteration?
FPS = 2

# 2D point
ScreenPoint = Blender.Object.Get("ScreenPoint")
# Scaling factor for 2D coordinates
screen_scaling = 100.0

# Screen background, currently useless for this script
Screen = Blender.Object.Get("Screen")

# 3D point
Sensor = Blender.Object.Get("Sensor")


def create_IPO_curves(obj, three_d=True):
    name = obj.getName()
    # Creating an empty IPO curve
    ipo = Blender.Ipo.New("Object", name+"_Ipo")

    if three_d:
        curve_names = ("LocX", "LocY", "LocZ")
    else:
        curve_names = ("LocX", "LocY")

    for curvename in curve_names:
        ipocurve = ipo.addCurve(curvename)
        ipocurve.extend = Blender.IpoCurve.ExtendTypes["CONST"]
        ipocurve.interpolation = Blender.IpoCurve.InterpTypes["BEZIER"]

    obj.setIpo(ipo)


def new_bezier_point(x, y, knot_len=1):
    # Note: the knot_len is completely ignored when handleTypes are set to AUTO,
    # i.e., Blender will calculate new positions for the handles automatically.
    b = Blender.BezTriple.New( (
        x-knot_len, y, 0,
        x, y, 0,
        x+knot_len, y, 0,
    ) )
    # Bezier handle types:
    # http://wiki.blender.org/index.php/Doc:Manual/Modelling/Curves#B.C3.A9ziers
    b.handleTypes = [
        Blender.BezTriple.HandleTypes["AUTO"],
        Blender.BezTriple.HandleTypes["AUTO"],
    ]
    return b


def main():
    Blender.Window.EditMode(0)

    # Reading 3D coordinates from the sensor
    create_IPO_curves(Sensor, three_d=True)
    curvex = Sensor.getIpo()[Blender.Ipo.OB_LOCX]
    curvey = Sensor.getIpo()[Blender.Ipo.OB_LOCY]
    curvez = Sensor.getIpo()[Blender.Ipo.OB_LOCZ]
    obj_for_prev_line = None
    ipo_x = 0
    f = open("3d.txt", "r")
    for line in f:
        line = line.strip()

        # Support for calibration corners
        if line in ['topleft', 'topright', 'bottomright', 'bottomleft']:
            obj_for_prev_line = Blender.Object.Get('Corner_' + line)

        try:
            x, y, z = [float(i) for i in line.split()]
        except:
            continue

        if obj_for_prev_line is None:
            # Add coordinate to Sensor object
            curvex.append(new_bezier_point(1 + ipo_x*FPS, x))
            curvey.append(new_bezier_point(1 + ipo_x*FPS, y))
            curvez.append(new_bezier_point(1 + ipo_x*FPS, z))

            ipo_x += 1
        else:
            # Moving the corner
            obj_for_prev_line.LocX = x
            obj_for_prev_line.LocY = y
            obj_for_prev_line.LocZ = z
            obj_for_prev_line = None

    f.close()


    # Reading 2D coordinates for the screen
    create_IPO_curves(ScreenPoint, three_d=False)
    curvex = ScreenPoint.getIpo()[Blender.Ipo.OB_LOCX]
    curvey = ScreenPoint.getIpo()[Blender.Ipo.OB_LOCY]
    ipo_x = 0
    f = open("2d.txt", "r")
    for line in f:
        line = line.strip()

        if line == 'discarded':
            ipo_x += 1
        else:
            try:
                x, y = [float(i) for i in line.split()]
            except:
                continue

            # Add coordinate to ScreenPoint object
            curvex.append(new_bezier_point(1 + ipo_x*FPS, x*screen_scaling))
            curvey.append(new_bezier_point(1 + ipo_x*FPS, y*screen_scaling))

            ipo_x += 1

    f.close()

    Blender.Redraw()


if __name__ == "__main__":
    main()
