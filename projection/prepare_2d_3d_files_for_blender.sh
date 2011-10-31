#!/bin/bash

# phi and theta offsets
p=0
t=0

# PHI and THETA apertures
P=45
T=45

# Algorithm
a=6

./generate_sphere_vectors.py -P ${P} -T ${T} -p ${p} -t ${t} \
| tee 3d.txt \
| ./convert_coordinates.py -a ${a} > 2d.txt
