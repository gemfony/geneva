#!/bin/bash

################################################################################
#
# This file is part of the Geneva library collection. The following license
# applies to this file:
#
# ------------------------------------------------------------------------------
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#    http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
# ------------------------------------------------------------------------------
#
# Note that other files in the Geneva library collection may use a different
# license. Please see the licensing information in each file.
#
################################################################################
#
# Geneva was started by Dr. Rüdiger Berlich and was later maintained together
# with Dr. Ariel Garcia under the auspices of Gemfony scientific. For further
# information on Gemfony scientific, see http://www.gemfomy.eu .
#
# The majority of files in Geneva was released under the Apache license v2.0
# in February 2020.
#
# See the NOTICE file in the top-level directory of the Geneva library
# collection for a list of contributors and copyright information.
#
################################################################################
# This script is a wrapper around the rather complicated cmake call,
# allowing you to permanently specify some options you'd otherwise
# have to provide repeatedly on the command line. Call the script
# with a file setting the variables below. See the script directory
# in the Geneva root directory for an example (genevaConfig.gcfg).
# Note that configuration files need to end with ".gcfg" .
####################################################################

####################################################################
# Make a note of the build-root-directory
GENEVA_BUILDROOT="${PWD}"

####################################################################
# Parse command line arguments
DRYRUN=0
CONFIGFILE=""

for arg in "$@"; do
	case "$arg" in
		--help|-h)
			echo -e "\nUsage: $(basename "$0") [<config.gcfg>] [--dryrun] [--help|-h]"
			echo -e "\nOptions:"
			echo -e "  <config.gcfg>  Optional Geneva configuration file (must end in .gcfg)."
			echo -e "                 If omitted, built-in defaults are used."
			echo -e "  --dryrun       Print the full cmake command that would be executed,"
			echo -e "                 including all -D options derived from the config file,"
			echo -e "                 without actually running cmake. Useful for copying the"
			echo -e "                 command into an IDE such as JetBrains CLion."
			echo -e "  --help, -h     Show this help message.\n"
			exit 0
			;;
		--dryrun)
			DRYRUN=1
			;;
		-*)
			echo -e "\nUnknown option: '$arg'. Use --help for usage information.\nLeaving...\n"
			exit 1
			;;
		*)
			if [ -n "${CONFIGFILE}" ]; then
				echo -e "\nError: multiple config files specified. Leaving...\n"
				exit 1
			fi
			CONFIGFILE="$arg"
			;;
	esac
done

####################################################################
# Check variables, set variable defaults if no config file was given
if [ -z "${CONFIGFILE}" ]; then
	echo -e "\nSetting variable defaults, as no Geneva config"
	echo -e "file was provided. See the Geneva 'scripts' directory"
	echo -e "for an example (genevaConfig.gcfg).\n"

	CMAKE="/usr/bin/cmake"         # Where the cmake executable is located
	BUILDMODE="Release"            # Release, Debug, RelWithDebInfo, MinSizeRel or Sanitize (experimental, will default to Debug on unsupported platforms)
	BUILDTESTCODE="1"              # Whether to build Geneva with testing code
	BUILDEXAMPLES="1"              # Whether to build Geneva examples
	BUILDBENCHMARKS="1"            # Whether to build Geneva benchmarks
	BUILDSTATIC="0"                # Whether to build static code / libraries (experimental!)
	VERBOSEMAKEFILE="1"            # Whether compilation information should be emitted
	INSTALLDIR="/opt/geneva"       # Where the Geneva library shall go
	MPIROOT=""                     # Root directory of the MPI installation, empty for automatic find through Cmake
	BUILDMPICONSUMER="0"           # Whether to build the MPI-consumer of the courtier library
	BUILDCUDAEXAMPLES="0"          # Whether to build CUDA examples (note: this is an experimental feature)
	USECUDARNG="0"                 # Whether CUDA shall be used to create random numbers in the Hap-library
else
	# Check that the command file has the expected form (ends with .gcfg)
	testfile=$(basename "${CONFIGFILE}" .gcfg).gcfg
	if [ ! "$(basename "${CONFIGFILE}")" = "$testfile" ]; then
		echo -e "\nFile '${CONFIGFILE}' does not seem to be a Geneva config file, as it"
		echo -e "does not end in '.gcfg' as expected. Leaving...\n"
		exit 1
	fi

	# Check that the file exists
	if [ ! -e "${CONFIGFILE}" ]; then
		echo -e "\nError: File '${CONFIGFILE}' does not seem to exist.\nLeaving...\n"
		exit 1
	fi

	# Source the config file
	echo -e "\nUsing configuration file '${CONFIGFILE}'\n"
	# shellcheck source=/dev/null
	. "${CONFIGFILE}"

	# Check whether all required variables were set
	if [ -z "${CMAKE}" ]; then
		CMAKE="/usr/bin/cmake"
		echo "Variable CMAKE wasn't set. Setting to default value '${CMAKE}'"
	fi

	if [ -z "${BUILDMODE}" ]; then
		BUILDMODE="Debug"
		echo "Variable BUILDMODE wasn't set. Setting to default value '${BUILDMODE}'"
	fi

	if [ -z "${BUILDTESTCODE}" ]; then
		BUILDTESTCODE="1"
		echo "Variable BUILDTESTCODE wasn't set. Setting to default value '${BUILDTESTCODE}'"
	fi

	if [ -z "${BUILDEXAMPLES}" ]; then
		BUILDEXAMPLES="1"
		echo "Variable BUILDEXAMPLES wasn't set. Setting to default value '${BUILDEXAMPLES}'"
	fi

	if [ -z "${BUILDBENCHMARKS}" ]; then
		BUILDBENCHMARKS="1"
		echo "Variable BUILDBENCHMARKS wasn't set. Setting to default value '${BUILDBENCHMARKS}'"
	fi

	if [ -z "${BUILDSTATIC}" ]; then
		BUILDSTATIC="0"
		echo "Variable BUILDSTATIC wasn't set. Setting to default value '${BUILDSTATIC}'"
	fi

	if [ -z "${VERBOSEMAKEFILE}" ]; then
		VERBOSEMAKEFILE="0"
		echo "Variable VERBOSEMAKEFILE wasn't set. Setting to default value '${VERBOSEMAKEFILE}'"
	fi

	if [ -z "${INSTALLDIR}" ]; then
		INSTALLDIR="/opt/geneva"
		echo "Variable INSTALLDIR wasn't set. Setting to default value '${INSTALLDIR}'"
	fi

	if [ -z "${BUILDMPICONSUMER}" ]; then
		BUILDMPICONSUMER="0"
		echo "Variable BUILDMPICONSUMER wasn't set. Setting to default value '${BUILDMPICONSUMER}'"
	fi

	if [ -z "${MPIROOT}" ]; then
		MPIROOT=""
		echo "Variable MPIROOT not specified, setting MPIROOT to empty string, indicating automatic find through CMake."
	fi

	if [ -z "${BUILDCUDAEXAMPLES}" ]; then
		BUILDCUDAEXAMPLES="0"
		echo "Variable BUILDCUDAEXAMPLES wasn't set. Setting to default value '${BUILDCUDAEXAMPLES}'"
	fi

	if [ -z "${USECUDARNG}" ]; then
		USECUDARNG="0"
		echo "Variable USECUDARNG wasn't set. Setting to default value '${USECUDARNG}'"
	fi
fi

####################################################################
# Some checks
if [ ! -x "${CMAKE}" ]; then
	echo -e "\nError: Could not find cmake executable '${CMAKE}'"
	echo -e "Please provide the correct path in the configuration file."
	echo -e "Leaving...\n"
	exit 1
fi

# Check if the BOOST* variables were set correctly: either ROOT or LIB+INCLUDE
# must be set, or none at all
if [ -z "${BOOSTROOT}" ] && [ -z "${BOOSTLIBS}" ] && [ -z "${BOOSTINCL}" ]; then
	_BOOST_SYSTEM="true"
	echo "Variable BOOSTROOT wasn't set. Letting CMake search for a system Boost installation."
elif [ -z "${BOOSTLIBS}" ] && [ -n "${BOOSTINCL}" ]; then
	_BOOST_VAR_ERROR="true"
elif [ -n "${BOOSTLIBS}" ] && [ -z "${BOOSTINCL}" ]; then
	_BOOST_VAR_ERROR="true"
elif [ -n "${BOOSTROOT}" ] && [ -n "${BOOSTLIBS}" ] && [ -n "${BOOSTINCL}" ]; then
	_BOOST_VAR_ERROR="true"
elif [ -n "${BOOSTROOT}" ] && [ -z "${BOOSTLIBS}" ] && [ -z "${BOOSTINCL}" ]; then
	# This case is fine, let CMakes' FindBoost find the header and lib locations,
	# but without searching system paths
	_BOOST_SYSTEM="false"
elif [ -z "${BOOSTROOT}" ] && [ -n "${BOOSTLIBS}" ] && [ -n "${BOOSTINCL}" ]; then
	# This case is fine, both the header and lib locations are given explicitely
	# so do not search system paths
	_BOOST_SYSTEM="false"
fi

if [ "${_BOOST_VAR_ERROR}" = "true" ]; then
	echo -e "\nError: inconsistent Boost location variables. Please"
	echo -e "set either BOOSTROOT, or both BOOSTLIBS and BOOSTINCL,"
	echo -e "or none at all. Got variables:"
	echo -e "\tBOOSTROOT = ${BOOSTROOT}"
	echo -e "\tBOOSTLIBS = ${BOOSTLIBS}"
	echo -e "\tBOOSTINCL = ${BOOSTINCL}"
	echo -e "Leaving...\n"
	exit 1
fi

if [ -n "${BOOSTINCL}" ] && [ ! -e "${BOOSTINCL}/boost/version.hpp" ]; then
	echo -e "\nError: there does not seem to be a complete Boost installation."
	echo -e "Expected to find the file 'boost/version.hpp' in \$BOOSTINCL."
	echo -e "Got variables:"
	echo -e "\tBOOSTLIBS = ${BOOSTLIBS}"
	echo -e "\tBOOSTINCL = ${BOOSTINCL}"
	echo -e "Leaving...\n"
	exit 1
fi

if [ ! "${BUILDMODE}" = "Release" ] && [ ! "${BUILDMODE}" = "Debug" ] \
		&& [ ! "${BUILDMODE}" = "RelWithDebInfo" ] && [ ! "${BUILDMODE}" = "MinSizeRel" ] \
		&& [ ! "${BUILDMODE}" = "Sanitize" ]; then
	echo -e "\nError: Invalid build mode ${BUILDMODE} provided. Leaving...\n"
	exit 1
fi

if [ ! "${BUILDTESTCODE}" = "0" ] && [ ! "${BUILDTESTCODE}" = "1" ]; then
	echo -e "\nError: Variable BUILDTESTCODE must be 0 or 1. Got ${BUILDTESTCODE}"
	echo -e "Leaving...\n"
	exit 1
fi

if [ ! "${BUILDEXAMPLES}" = "0" ] && [ ! "${BUILDEXAMPLES}" = "1" ]; then
	echo -e "\nError: Variable BUILDEXAMPLES must be 0 or 1. Got ${BUILDEXAMPLES}"
	echo -e "Leaving...\n"
	exit 1
fi

if [ ! "${BUILDBENCHMARKS}" = "0" ] && [ ! "${BUILDBENCHMARKS}" = "1" ]; then
	echo -e "\nError: Variable BUILDBENCHMARKS must be 0 or 1. Got ${BUILDBENCHMARKS}"
	echo -e "Leaving...\n"
	exit 1
fi

if [ ! "${BUILDSTATIC}" = "0" ] && [ ! "${BUILDSTATIC}" = "1" ]; then
	echo -e "\nError: Variable BUILDSTATIC must be 0 or 1. Got ${BUILDSTATIC}"
	echo -e "Leaving...\n"
	exit 1
fi

if [ ! "${VERBOSEMAKEFILE}" = "0" ] && [ ! "${VERBOSEMAKEFILE}" = "1" ]; then
	echo -e "\nError: Variable VERBOSEMAKEFILE must be 0 or 1. Got ${VERBOSEMAKEFILE}"
	echo -e "Leaving...\n"
	exit 1
fi

if [ ! "${BUILDMPICONSUMER}" = "0" ] && [ ! "${BUILDMPICONSUMER}" = "1" ]; then
	echo -e "\nError: Variable BUILDMPICONSUMER must be 0 or 1. Got ${BUILDMPICONSUMER}"
	echo -e "Leaving...\n"
	exit 1
fi

if [ ! "${BUILDCUDAEXAMPLES}" = "0" ] && [ ! "${BUILDCUDAEXAMPLES}" = "1" ]; then
	echo -e "\nError: Variable BUILDCUDAEXAMPLES must be 0 or 1. Got ${BUILDCUDAEXAMPLES}"
	echo -e "Leaving...\n"
	exit 1
fi

if [ ! "${USECUDARNG}" = "0" ] && [ ! "${USECUDARNG}" = "1" ]; then
	echo -e "\nError: Variable USECUDARNG must be 0 or 1. Got ${USECUDARNG}"
	echo -e "Leaving...\n"
	exit 1
fi

####################################################################
# Find out where this script is located and whether there is a
# CMakeLists.txt file in the same directory. We then assume that
# this is the project root, as it should be.
PROJECTROOT=$(dirname "$0")/..
if [ ! -e "${PROJECTROOT}/CMakeLists.txt" ]; then
	echo -e "Error: the script should reside in the project root."
	echo -e "Leaving...\n"
	exit 1
fi

####################################################################
# If there is a Makefile in the directory, reset the CMake build
# system, as we are about to launch a new configuration run
# clean-all is a build-target defined by the Geneva-build-system.
# If a Makefile exists, we assume that the build environment has been
# set up before
if [ -e "${GENEVA_BUILDROOT}/Makefile" ]; then
	cd "${GENEVA_BUILDROOT}" || exit 1
	echo -en "Cleaning old build-environment ..."
	make clean-cmake > /dev/null 2>&1
	echo -e " done"
fi
# CMake variables get cached and old ones may be used if currently
# missing, which is very misleading if the user tries changing values.
# Force removing the cache, because 'make clean-cmake' may fail if
# a previous cmake run failed.
if [ -e "${GENEVA_BUILDROOT}/CMakeCache.txt" ]; then
	rm -f "${GENEVA_BUILDROOT}/CMakeCache.txt"
fi

####################################################################
# Do the actual call to cmake.
#
# The 'FindBoost' module uses either the BOOST_ROOT or the
# BOOST_LIBRARYDIR and BOOST_INCLUDEDIR variables, not both.
if [ -n "${BOOSTROOT}" ]; then
	BOOSTLOCATIONPATHS="-DBOOST_ROOT=${BOOSTROOT}"
elif [ -n "${BOOSTLIBS}" ]; then
	BOOSTLOCATIONPATHS="-DBOOST_LIBRARYDIR=${BOOSTLIBS} -DBOOST_INCLUDEDIR=${BOOSTINCL}"
fi

if [ "${_BOOST_SYSTEM}" = "false" ]; then
	BOOSTSYSTEMFLAG="-DBoost_NO_SYSTEM_PATHS=1"
fi

CONFIGURE="${CMAKE} $BOOSTLOCATIONPATHS $BOOSTSYSTEMFLAG \
-DGENEVA_BUILD_TYPE=${BUILDMODE} \
-DGENEVA_BUILD_TESTS=${BUILDTESTCODE} \
-DGENEVA_BUILD_EXAMPLES=${BUILDEXAMPLES} \
-DGENEVA_BUILD_BENCHMARKS=${BUILDBENCHMARKS} \
-DGENEVA_STATIC=${BUILDSTATIC} \
-DCMAKE_VERBOSE_MAKEFILE=${VERBOSEMAKEFILE} \
-DCMAKE_INSTALL_PREFIX=${INSTALLDIR} \
-DGENEVA_BUILD_WITH_MPI_CONSUMER=${BUILDMPICONSUMER} \
-DGENEVA_BUILD_WITH_CUDA_EXAMPLES=${BUILDCUDAEXAMPLES} \
-DGENEVA_USE_CUDA_RNG=${USECUDARNG}"

if [ "$MPIROOT" != "" ]; then
	CONFIGURE="${CONFIGURE} -DMPI_HOME='${MPIROOT}'"
fi

if [ "$CXXEXTRAFLAGS" != "" ]; then
	CONFIGURE="${CONFIGURE} -DCMAKE_CXX_FLAGS='${CXXEXTRAFLAGS}'"
fi

if [ "$LINKEREXTRAFLAGS" != "" ]; then
	CONFIGURE="${CONFIGURE} -DCMAKE_EXE_LINKER_FLAGS='${LINKEREXTRAFLAGS}'"
fi

if [ "$CMAKEEXTRAFLAGS" != "" ]; then
	CONFIGURE="${CONFIGURE} ${CMAKEEXTRAFLAGS}"
fi

echo -e "\nConfiguring with command: \"${CONFIGURE} ${PROJECTROOT}\"\n"
echo -e "---------------------------------------------------------------------\n\n"

if [ "${DRYRUN}" = "1" ]; then
	echo -e "Dry run: cmake was NOT executed. Copy the command above into your"
	echo -e "IDE (e.g. JetBrains CLion) as the CMake options / command line.\n"
else
	# Word-splitting of CONFIGURE is intentional here — it holds cmake flags
	# shellcheck disable=SC2086
	if eval ${CONFIGURE} ${PROJECTROOT}; then
		echo -e "\n\n---------------------------------------------------------------------"
		echo -e "\nYou may now build and install Geneva in the usual way:"
		echo -e "make\t\t# Use '-jn', where 'n' is the number of cores in your system"
		echo -e "make install\n\n"
	fi
fi

####################################################################
# Done
