#!/bin/bash

####################################################################
#
# Copyright (C) Gemfony scientific UG (haftungsbeschraenkt)
#
# See the AUTHORS file in the top-level directory for a list of authors.
#
# Contact: contact [at] gemfony (dot) eu
#
# This file is part of the Geneva library collection.
#
# Geneva was developed with kind support from Karlsruhe Institute of
# Technology (KIT) and Steinbuch Centre for Computing (SCC). Further
# information about KIT and SCC can be found at http://www.kit.edu/english
# and http://scc.kit.edu .
#
# Geneva is free software: you can redistribute and/or modify it under
# the terms of version 3 of the GNU Affero General Public License
# as published by the Free Software Foundation.
#
# Geneva is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
# GNU Affero General Public License for more details.
#
# You should have received a copy of the GNU Affero General Public License
# along with the Geneva library. If not, see <http://www.gnu.org/licenses/>.
#
# For further information on Gemfony scientific and Geneva, visit
# http://www.gemfony.eu .
#
####################################################################
# This script may be used to generate or update the configuration
# files of all tests and examples in the Geneva tree. This script
# requires a fully built Geneva tree, and takes the location of
# the source tree and the location of the fully built binary tree
# as parameters.
# NOTE: running any Geneva test or example in an empty folder will
# automatically generate all the necessary configuration files
# where all the parameters have default values, if an empty
# './config' folder is present. This script only automates the
# calling of all test and example executables and the copying of
# the new files into the source tree.
####################################################################

# Check the parameters
if [ ! $# -eq 2 ]; then

	echo -e "\nUsage: `basename $0` <GENEVA_SOURCE_DIR> <GENEVA_BUILD_DIR>\n"
	exit 1

fi

DIR_GENEVA_SRC="$1"
DIR_GENEVA_BUILD="$2"

if [ ! -d "$DIR_GENEVA_SRC" ]; then
	echo -e "\nError: Folder '$DIR_GENEVA_SRC' does not exist.\nAborting...\n"
	exit 2
fi
if [ ! -d "$DIR_GENEVA_BUILD" ]; then
	echo -e "\nError: Folder '$DIR_GENEVA_BUILD' does not exist.\nAborting...\n"
	exit 2
fi

if [ ! -e "$DIR_GENEVA_SRC/CMakeLists.txt" ] \
		|| [ ! -d "$DIR_GENEVA_SRC/examples" ] \
		|| [ ! -d "$DIR_GENEVA_SRC/tests" ]; then
	echo -e "\nError: Folder '$DIR_GENEVA_SRC' does not seem to contain a Geneva source tree.\nAborting...\n"
	exit 2
fi

if [ ! -e "${DIR_GENEVA_BUILD}/CMakeCache.txt" ] \
		|| [ ! -x "${DIR_GENEVA_BUILD}/examples/geneva/01_GSimpleOptimizer/GSimpleOptimizer" ] \
		|| [ ! -x "${DIR_GENEVA_BUILD}/tests/geneva/UnitTests/GenevaStandardTests" ]; then
	echo -e "\nError: Folder '$DIR_GENEVA_BUILD' does not seem to contain a built Geneva build tree.\nAborting...\n"
	exit 2
fi

####################################################################
# Warn the user and ask for confirmation

echo -e "\nWARNING: This script will delete all 'config/*.json' files in the given" \
	"Geneva source tree, and replace them with newly generated files."
read -p "Are you sure you want to continue? (y/N) " ANS

if [ "x$ANS" != "xy" ] && [ "x$ANS" != "xY" ]; then
	echo -e "\nOk, no files were changed...\n"
	exit 0
fi

####################################################################
# Collect the configuration folders and files to update

pushd "$DIR_GENEVA_SRC" > /dev/null

# There are no filenames/paths with spaces in the Geneva sources
CONFIG_DIRS_EX=`find examples -type d -name config | sort`
CONFIG_DIRS_TST=`find tests -type d -name config | sort`

####################################################################
# Cleanup the old configuration files and generate new ones

for d in $CONFIG_DIRS_EX $CONFIG_DIRS_TST ; do
	rm -f "$d"/*.json
done

popd > /dev/null
pushd "$DIR_GENEVA_BUILD" > /dev/null

# The configuration files in the build tree are copied from the
# source tree by CMake at build time, remove them also
for d in $CONFIG_DIRS_EX $CONFIG_DIRS_TST ; do
	rm -f "$d"/*.json
done

echo -e "\n======================================================================================\n"

ALGS="ea,gd,ps,sa,swarm"
OPT_ALGS="--optimizationAlgorithms"

# Run each program from its own subfolder
for d in $CONFIG_DIRS_EX $CONFIG_DIRS_TST ; do

	PRGM_DIR=`dirname $d`
	pushd $PRGM_DIR > /dev/null
	PRGM=`find -maxdepth 1 -type f -executable`

	echo -e "Running  '$PRGM'  to generate default configuration files..."

	# Check if different optimization algotithms are possible...
	# ... Go2.json config files are generated here already, redirect stderr!
	if `"$PRGM" --help 2> /dev/null | grep -q ".$OPT_ALGS"`; then
		# Configuration files are created all together at the beginning
		PARAMS="$OPT_ALGS $ALGS"
	else
		PARAMS=""
	fi
	timeout 5s "$PRGM" $PARAMS > /dev/null 2>&1
	popd > /dev/null

done

popd > /dev/null

echo -e "\n======================================================================================\n"

####################################################################
# Copy the newly generated configuration files to the source tree

echo -en "Copying newly generated files to the source tree... "

for d in $CONFIG_DIRS_EX $CONFIG_DIRS_TST ; do
	cp "${DIR_GENEVA_BUILD}"/$d/*.json "${DIR_GENEVA_SRC}"/$d
done

echo -e "Done!"

####################################################################
# Finish by telling the user how to continue
echo -e "\n---------------------------------------------------------------------\n"
echo -e "If everything did run correctly, you should now have updated configuration" \
	"files in the Geneva source tree. Please consider checking them back into the" \
	"repository!\n\n"
echo -e "\n---------------------------------------------------------------------\n"

####################################################################
# Done
