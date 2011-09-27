#!/bin/bash

# phi and theta offsets
p=0
t=0

# PHI and THETA apertures
P=90
T=90

# Algorithm
a=9

./generate_sphere_vectors.py -P ${P} -T ${T} -p ${p} -t ${t} \
| tee 3d.txt \
| ./convert_coordinates.py -a ${a} > 2d.txt
