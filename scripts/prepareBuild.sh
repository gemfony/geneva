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
# This script is a wrapper around the rather complicated cmake call,
# allowing you to permanently specify some options you'd otherwise
# have to provide repeatedly on the command line. Call the script
# with a file setting the variables below. See the script directory
# in the Geneva root directory for an example (genevaConfig.gcfg).
# Note that configuration files need to end with ".gcfg" .
####################################################################
#!/bin/bash

####################################################################
# Make a note of the build-root-directory
GENEVA_BUILDROOT=${PWD}

####################################################################
# Check variables, set variable defaults if no config file was given
if [ $# -eq 0 ]; then
	echo -e "\nSetting variable defaults, as no Geneva config"
	echo -e "file was provided. See the Geneva 'scripts' directory"
	echo -e "for an example (genevaConfig.gcfg).\n"

	CMAKE=/usr/bin/cmake                      # Where the cmake executable is located
	BOOSTROOT="/opt/boost"                    # Where Boost is installed
	BOOSTLIBS="${BOOSTROOT}/lib"              # Where the Boost libraries are
	BOOSTINCL="${BOOSTROOT}/include/boost"    # Where the Boost headers are
	BUILDMODE="Release"                       # Release or Debug
	BUILDSTD="cxx98"                          # "auto": choose automatically; "cxx98": enforce the C++98 standard; "cxx11": enforce the C++11 standard
	BUILDWITHMPI="0"                          # Whether Geneva should be built with MPI support (experimental)
	BUILDTESTCODE="0"                         # Whether to build Geneva with testing code
	BUILDSTATIC="0"                           # Whether to build static code / libraries (experimental)
	VERBOSEMAKEFILE="0"                       # Whether compilation information should be emitted
	INSTALLDIR="/opt/geneva"                  # Where the Geneva library shall go
	CEXTRAFLAGS=""                            # Further CMake settings you might want to provide
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

	# Check whether all required variables were set
	if [ -z "${CMAKE}" ]; then
		CMAKE=/usr/bin/cmake
		echo "Variable CMAKE wasn't set. Setting to default value ${CMAKE}"
	fi

	if [ -z "${BOOSTROOT}" ]; then
		BOOSTROOT="/opt/boost"
		echo "Variable BOOSTROOT wasn't set. Setting to default value ${BOOSTROOT}"
	fi

	if [ -z "${BOOSTLIBS}" ]; then
		BOOSTLIBS="${BOOSTROOT}/lib"
		echo "Variable BOOSTLIBS wasn't set. Setting to default value ${BOOSTLIBS}"
	fi

	if [ -z "${BOOSTINCL}" ]; then
		BOOSTINCL="${BOOSTROOT}/include/boost"
		echo "Variable BOOSTINCL wasn't set. Setting to default value ${BOOSTINCL}"
	fi

	if [ -z "${BUILDMODE}" ]; then
		BUILDMODE="Release"
		echo "Variable BUILDMODE wasn't set. Setting to default value ${BUILDMODE}"
	fi

	if [ -z "${BUILDSTD}" ]; then
		BUILDSTD="cxx98"
		echo "Variable BUILDSTD wasn't set. Setting to default value ${BUILDSTD}"
	fi

	if [ -z "${BUILDWITHMPI}" ]; then
		BUILDWITHMPI="0"
		echo "Variable BUILDWITHMPI wasn't set. Setting to default value ${BUILDWITHMPI}"
	fi

	if [ -z "${BUILDTESTCODE}" ]; then
		BUILDTESTCODE="0"
		echo "Variable BUILDTESTCODE wasn't set. Setting to default value ${BUILDTESTCODE}"
	fi

	if [ -z "${BUILDSTATIC}" ]; then
		BUILDSTATIC="0"
		echo "Variable BUILDSTATIC wasn't set. Setting to default value ${BUILDSTATIC}"
	fi

	if [ -z "${VERBOSEMAKEFILE}" ]; then
		VERBOSEMAKEFILE="0"
		echo "Variable VERBOSEMAKEFILE wasn't set. Setting to default value ${VERBOSEMAKEFILE}"
	fi

	if [ -z "${INSTALLDIR}" ]; then
		INSTALLDIR="/opt/geneva"
		echo "Variable INSTALLDIR wasn't set. Setting to default value ${INSTALLDIR}"
	fi

	if [ -z "${CEXTRAFLAGS}" ]; then
		CEXTRAFLAGS=""
	fi
else
    echo "Received $# command line arguments, which is an invalid number."
    echo "You can either call this script without arguments, in which case"
	echo "default values will be assumed for all configuration options,"
	echo "or you can provide exactly one Geneva config file as command"
	echo "line argument, ending in .gcfg . Leaving now, as we do not know"
	echo "how to proceed."
	exit
fi

####################################################################
# Some checks
if [ ! -x ${CMAKE} ]; then
	echo "Error: Could not find cmake executable ${CMAKE}"
	echo "Please provide the correct path in the configuration file."
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

if [ ! "${BUILDWITHMPI}" = "0" -a ! "${BUILDWITHMPI}" = "1" ]; then
	echo "Error: Variable BUILDWITHMPI must be 0 or 1. Got ${BUILDWITHMPI}"
	echo "Leaving"
	exit
fi

if [ ! "${BUILDSTATIC}" = "0" -a ! "${BUILDSTATIC}" = "1" ]; then
	echo "Error: Variable BUILDSTATIC must be 0 or 1. Got ${BUILDSTATIC}"
	echo "Leaving"
	exit
fi

if [ ! "${VERBOSEMAKEFILE}" = "0" -a ! "${VERBOSEMAKEFILE}" = "1" ]; then
	echo "Error: Variable VERBOSEMAKEFILE must be 0 or 1. Got ${VERBOSEMAKEFILE}"
	echo "Leaving"
	exit
fi

if [ ! "${BUILDSTD}" = "auto" -a ! "${BUILDSTD}" = "cxx98" -a ! "${BUILDSTD}" = "cxx11" ]; then
	echo "Error: Variable BUILDSTD must be auto, cxx98 or cxx11. Got ${BUILDSTD}"
	echo "Leaving"
	exit
fi

####################################################################
# Find out where this script is located and whether there is a
# CMakeLists.txt file in the same directory. We then assume that
# this is the project root, as it should be.
PROJECTROOT=`dirname $0`/..
if [ ! -e ${PROJECTROOT}/CMakeLists.txt ]; then
	echo "Error: the script should reside in the project root."
	echo "Leaving."
	exit
fi

####################################################################
# If there is a Makefile in the directory, reset the CMake build
# system, as we are about to launch a new configuration run
# clean-all is a build-target defined by the Geneva-build-system.
# If a Makefile exists, we assume that the build environment has been
# set up before
if [ -e ${GENEVA_BUILDROOT}/Makefile ]; then
	cd ${GENEVA_BUILDROOT}
	echo "Cleaning old build-environment ..."
	make clean-cmake 2>&1 > /dev/null
	echo "... done"
fi

####################################################################
# Do the actual call to cmake
CONFIGURE="${CMAKE} \
-DBoost_NO_SYSTEM_PATHS=1 \
-DBOOST_ROOT=${BOOSTROOT} \
-DBOOST_LIBRARYDIR=${BOOSTLIBS} \
-DBOOST_INCLUDEDIR=${BOOSTINCL} \
-DGENEVA_BUILD_TYPE=${BUILDMODE} \
-DGENEVA_BUILD_TESTS=${BUILDTESTCODE} \
-DGENEVA_STATIC=${BUILDSTATIC} \
-DGENEVA_CXX_STD=${BUILDSTD} \
-DGENEVA_WITH_MPI=${BUILDWITHMPI} \
-DCMAKE_VERBOSE_MAKEFILE=${VERBOSEMAKEFILE} \
-DCMAKE_INSTALL_PREFIX=${INSTALLDIR}"

if [ "x$CEXTRAFLAGS" != "x" ]; then
	CONFIGURE="${CONFIGURE} ${CEXTRAFLAGS}"
fi

echo -e "\nConfiguring with command: \"${CONFIGURE} ${PROJECTROOT}\"\n"
echo -e "---------------------------------------------------------------------\n\n"
${CONFIGURE} ${PROJECTROOT}

####################################################################
# Finish by telling the user how to continue :-)
echo -e "\n\n---------------------------------------------------------------------"
echo -e "\nYou may now build and install Geneva in the usual way:"
echo -e "make\t\t# Use '-jn', where 'n' is the number of cores in your system"
echo -e "make install\n\n"

####################################################################
