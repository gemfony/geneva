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
 	echo "Setting variable defaults, as no Geneva config"
 	echo "file was provided. See the Geneva directory for"
 	echo "an example (genevaConfig.gcfg)."
 	
 	CMAKE=/usr/bin/cmake                      # Where the cmake executable is located
	BOOSTROOT="/opt/boost142"                 # Where Boost is installed
	BOOSTLIBS="${BOOSTROOT}/lib"              # Where the Boost libraries are
	BOOSTINCL="${BOOSTROOT}/include/boost"    # Where the Boost headers are
	BUILDMODE="Release"                       # Release or Debug
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
	echo "Using configuration file $1"
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
	echo "Error: This does not seem to be a complete"
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

####################################################################
# Find out where this script is located and whether there is a 
# CMakeLists.txt file in the same directory. We then assume that
# this is the project root, as it should be.
PROJECTROOT=`dirname $0`
if [ ! -e ${PROJECTROOT}/CMakeLists.txt ]; then
	echo "Error: You should call this script relative to the current directory,"
	echo "from within the project root."
	exit
fi

####################################################################
# Do the actual call to cmake

${CMAKE} \
	-DBOOST_ROOT=${BOOSTROOT} \
	-DBOOST_INCLUDEDIR=${BOOSTINCL} \
	-DBOOST_LIBRARYDIR=${BOOSTLIBS} \
	-DCMAKE_BUILD_TYPE=${BUILDMODE} \
	-DCMAKE_INSTALL_PREFIX=${INSTALLDIR} \
	${PROJECTROOT}
