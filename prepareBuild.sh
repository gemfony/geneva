####################################################################
# Copyright (C) Dr. Ruediger Berlich
# and Karlsruhe Institute of Technology (University of the State of
# Baden-Wuerttemberg and National Laboratory of the Helmholtz Association)
#
# Contact: info [at] gemfony (dot) com
#
# This file is part of the Geneva library, Gemfony scientific's optimization
# library.
#
# Geneva is free software: you can redistribute it and/or modify
# it under the terms of version 3 of the GNU Affero General Public License
# as published by the Free Software Foundation.
#
# Geneva is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU Affero General Public License for more details.
#
# You should have received a copy of the GNU Affero General Public License
# along with the Geneva library.  If not, see <http://www.gnu.org/licenses/>.
#
# For further information on Gemfony scientific and Geneva, visit
# http://www.gemfony.com .
#
####################################################################
# This script is a wrapper around the rather complicated cmake call,
# allowing you to permanently specify some options you'd otherwise
# have to provide repeatedly on the command line. Call the script
# with a file setting the variables below. See the Geneva root dir
# for an example (genevaConfig.gcfg). Config files need to end with
# .gcfg
####################################################################
#!/bin/bash

####################################################################
# Check variables, set variable defaults if no config file was given
if [ $# -eq 0 ]; then
	echo -e "\nSetting variable defaults, as no Geneva config"
	echo -e "file was provided. See the Geneva directory for"
	echo -e "an example (genevaConfig.gcfg).\n"

	CMAKE=/usr/bin/cmake                      # Where the cmake executable is located
	BOOSTROOT="/opt/boost"                    # Where Boost is installed
	BOOSTLIBS="${BOOSTROOT}/lib"              # Where the Boost libraries are
	BOOSTINCL="${BOOSTROOT}/include/boost"    # Where the Boost headers are
	BUILDMODE="Release"                       # Release or Debug
	BUILDTESTCODE="0"                         # Whether to build Geneva with testing code
	BUILDPCH="0"                              # Whether to use pre-compiled headers if possible
	VERBOSEMAKEFILE="0"                       # Whether compilation information should be emitted
	INSTALLDIR="/opt/geneva"                  # Where the Geneva library shall go
elif [ $# -eq 1 ]; then
	# Check that the command file has the expected form (ends with .gcfg)
	testfile=`basename $1 .gcfg`.gcfg
	if [ ! `basename $1` = "$testfile" ]; then
		echo "File $1 does not seem to be a Geneva config file, as it does not"
		echo "end in .gcfg as expected. Tested with ${testfile}. Leaving."
		exit
	fi

	# Check that the file exists
	if [ ! -e $1 ]; then
		echo "Error: File $1 does not seem to exist. Leaving"
		exit
	fi

	# Source the config file
	echo -e "\nUsing configuration file $1"
	. $1
else
	echo "You should provide exactly one Geneva config file as command"
	echo "line argument, ending in .gcfg . Got $# arguments instead."
	echo "See the Geneva directory for an example. Leaving."
	exit
fi

####################################################################
# Some checks
if [ ! -x ${CMAKE} ]; then
	echo "Error: Could not find cmake executable ${CMAKE}"
	echo "Please provide the correct path"
	exit
fi

if [ ! -e ${BOOSTINCL}/version.hpp ]; then
	echo "Error: There does not seem to be a complete"
	echo "Boost installation. Got variables"
	echo "BOOSTROOT = ${BOOSTROOT}"
	echo "BOOSTLIBS = ${BOOSTLIBS}"
	echo "BOOSTINCL = ${BOOSTINCL}"
	echo "Leaving"
	exit
fi

if [ ! "${BUILDMODE}" = "Release" -a ! "${BUILDMODE}" = "Debug" ]; then
	echo "Error: Invalid build mode ${BUILDMODE} provided. Leaving"
	exit
fi

if [ ! "${BUILDTESTCODE}" = "0" -a ! "${BUILDTESTCODE}" = "1" ]; then
	echo "Error: Variable BUILDTESTCODE must be 0 or 1. Got ${BUILDTESTCODE}"
	echo "Leaving"
	exit
fi

if [ ! "${BUILDPCH}" = "0" -a ! "${BUILDPCH}" = "1" ]; then
	echo "Error: Variable BUILDPCH must be 0 or 1. Got ${BUILDPCH}"
	echo "Leaving"
	exit
fi

if [ ! "${VERBOSEMAKEFILE}" = "0" -a ! "${VERBOSEMAKEFILE}" = "1" ]; then
	echo "Error: Variable VERBOSEMAKEFILE must be 0 or 1. Got ${VERBOSEMAKEFILE}"
	echo "Leaving"
	exit
fi

####################################################################
# Find out where this script is located and whether there is a
# CMakeLists.txt file in the same directory. We then assume that
# this is the project root, as it should be.
PROJECTROOT=`dirname $0`
if [ ! -e ${PROJECTROOT}/CMakeLists.txt ]; then
	echo "Error: the script should reside in the project root."
	echo "Leaving."
	exit
fi

####################################################################
# Do the actual call to cmake
CONFIGURE="${CMAKE} -DBoost_NO_SYSTEM_PATHS="1" -DBOOST_ROOT=${BOOSTROOT} -DBOOST_LIBRARYDIR=${BOOSTLIBS} -DBOOST_INCLUDEDIR=${BOOSTINCL} -DGENEVA_BUILD_TYPE=${BUILDMODE} -DBUILDTESTCODE=${BUILDTESTCODE} -DBUILDPCH=${BUILDPCH} -DCMAKE_VERBOSE_MAKEFILE=${VERBOSEMAKEFILE} -DCMAKE_INSTALL_PREFIX=${INSTALLDIR} ${PROJECTROOT}"

echo -e "\nConfiguring with command: \"${CONFIGURE}\"\n"
${CONFIGURE}

####################################################################
# Finish by telling the user how to continue :-)
echo -e "\n\n---------------------------------------------------------------------"
echo -e "\nYou may now build and install Geneva in the usual way:"
echo -e "\tmake\t\t\t# Use '-j2' if in a dual-core machine"
echo -e "\tmake install\n\n"

####################################################################
