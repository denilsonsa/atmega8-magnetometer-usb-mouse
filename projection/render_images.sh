#!/bin/bash

# Hey, wanna render the images without having a window popping up for every
# image? Run:
#   xvfb-run ./render_images.sh

# phi and theta offsets
p=0
t=0

# PHI and THETA apertures
#P=80
#T=80

# White dot size
DOT_SIZE=4

# Destination directory
IMAGE_DIR="images"


mkdir -p "${IMAGE_DIR}"

#for abertura in {10..90..5} ; do
for abertura in 15 30 45 60 75 80 85 90 ; do
	P=${abertura}
	T=${abertura}

	# C program
	#./generate_sphere_vectors.py -P ${P} -T ${T} -p ${p} -t ${t} \
	#| ./linear_eq_conversion \
	#| ./draw_points.py -p -s ${DOT_SIZE} -q -o "${IMAGE_DIR}/P${P}T${T}p${p}t${t}_cleq.png"

	# Python program
	#for a in {1..13} ; do
	for a in {7..13} ; do
		./generate_sphere_vectors.py -P ${P} -T ${T} -p ${p} -t ${t} \
		| ./convert_coordinates.py -a ${a} \
		| ./draw_points.py -p -s ${DOT_SIZE} -w 640 640 -q -o "${IMAGE_DIR}/P${P}T${T}p${p}t${t}_a${a}.png"
	done
done

echo "If you want to save space, also run this command:"
echo "optipng -o7 images/*"
