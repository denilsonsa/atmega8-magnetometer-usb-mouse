#!/bin/bash

# phi and theta offsets
p=0
t=0

# PHI and THETA apertures
#P=80
#T=80

# White dot size
DOT_SIZE=2

for abertura in {10..90..5} ; do
	P=${abertura}
	T=${abertura}

	for a in {1..8} ; do
		./generate_sphere_vectors.py -P ${P} -T ${T} -p ${p} -t ${t} \
		| ./convert_coordinates.py -a ${a} \
		| ./draw_points.py -p -s ${DOT_SIZE} -q -o "P${P}T${T}p${p}t${t}_a${a}.png"
	done
done
