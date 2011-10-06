#!/bin/bash

# WARNING! THIS SCRIPT MAY BE DANGEROUS!
#
# If you are not careful enough, you may lose some files, and this script may
# not work.
#
# INSTRUCTIONS:
#
# 1. Make a second copy of the repository. How? Like this:
#    $ hg clone .. repo_clone
#    This will create a new directory called "repo_clone" that will contain a
#    new, independent copy of the repository.
#    This is important because this script will work inside that copy.
#
# 2. Make sure the variables below are sane.
#    You probably don't even need to change them.
#
# 3. Once you are sure you want to run this script, export this env var:
#    YES_I_HAVE_READ_THE_DOCUMENTATION=1


# Directory relative to ${SCRIPT_DIR}
REPO_DIR="repo_clone"

# Directory relative to ${REPO_DIR}
FIRMWARE_DIR="firmware"

# Path relative to ${SCRIPT_DIR}
DATABASE_FILE="size_vs_commit.txt"

# Will be auto-detected by this script.
# Will contain an absolute path equal to $PWD
SCRIPT_DIR=""

# END OF CONFIGURATION
############################################################
# START OF CODE

# Init
if [ -z "${YES_I_HAVE_READ_THE_DOCUMENTATION}" ] ; then
	echo "You should read this script source before running it."
	exit 1
fi

# Using absolute paths:
SCRIPT_DIR=`pwd`
REPO_DIR="${SCRIPT_DIR}/${REPO_DIR}"
FIRMWARE_DIR="${REPO_DIR}/${FIRMWARE_DIR}"
DATABASE_FILE="${SCRIPT_DIR}/${DATABASE_FILE}"

# Sanity check
if [ ! -d "${REPO_DIR}" ] ; then
	echo "Repository dir was not found: ${REPO_DIR}"
	exit 1
fi

# Auto-creating the file
if [ ! -f "${DATABASE_FILE}" ] ; then
	{
		echo $'# The values are separated by tab'
		echo $'# size1 = make all'
		echo $'# size2 = make combine'
		echo $'# rev\thash\tsize1\tsize2\tdate\tdescription'
	} > "${DATABASE_FILE}"
fi

# Finding the latest revision + 1
REV_BEGIN=`awk 'BEGIN {FS="\t"; a=0} /^[0-9]+\t/{if($1 > a) a=$1} END {print a+1}' < "${DATABASE_FILE}"`

# Finding the tip revision
cd "${REPO_DIR}"
REV_END=`hg log -r tip --template '{rev}'`

# Iterating over all revisions
for REV in `seq ${REV_BEGIN} ${REV_END}` ; do
	echo ">> Trying revision ${REV}"

	cd "${REPO_DIR}"
	hg up -C -r "${REV}"

	# Check if there is a firmware
	if [ -d "${FIRMWARE_DIR}" ] ; then
		# Try compiling the firmware, this may fail
		cd "${FIRMWARE_DIR}"
		SIZE1=`make all | sed -n 's/^ROM: \([0-9]\+\) bytes.*/\1/p'`
		make clean

		SIZE2=`make combine | sed -n 's/^ROM: \([0-9]\+\) bytes.*/\1/p'`
		make clean

		# If it fails, then the size is zero
		[ -z "${SIZE1}" ] && SIZE1=0
		[ -z "${SIZE2}" ] && SIZE2=0
	else
		SIZE1=0
		SIZE2=0
	fi

	# Writing to the database file
	hg log -r . --template "{rev}\\t{node|short}\\t${SIZE1}\\t${SIZE2}\\t{date|isodate}\\t{desc|firstline}\\n" >> "${DATABASE_FILE}"
done
