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
CLEAN=0
GENERATE_PRESET=0
CONFIGFILE=""

for arg in "$@"; do
	case "$arg" in
		--help|-h)
			echo -e "\nUsage: $(basename "$0") [<config.gcfg>] [--clean] [--dryrun] [--generate-preset] [--help|-h]"
			echo -e "\nOptions:"
			echo -e "  <config.gcfg>       Optional Geneva configuration file (must end in .gcfg)."
			echo -e "                      If omitted, built-in defaults are used."
			echo -e "  --clean             Remove all files from the build directory except .gcfg"
			echo -e "                      files, then reconfigure. Required when calling the script"
			echo -e "                      a second time in the same directory."
			echo -e "  --dryrun            Print the full cmake command that would be executed,"
			echo -e "                      including all -D options derived from the config file,"
			echo -e "                      without actually running cmake. Useful for copying the"
			echo -e "                      command into an IDE such as JetBrains CLion."
			echo -e "  --generate-preset   Write a CMakeUserPresets.json to the project root so"
			echo -e "                      that JetBrains CLion (including via Gateway) picks up"
			echo -e "                      the same CMake configuration automatically."
			echo -e "  --help, -h          Show this help message.\n"
			exit 0
			;;
		--clean)
			CLEAN=1
			;;
		--dryrun)
			DRYRUN=1
			;;
		--generate-preset)
			GENERATE_PRESET=1
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

if [ "${CLEAN}" = "1" ] && [ "${DRYRUN}" = "1" ]; then
	echo -e "\nError: --clean and --dryrun are mutually exclusive."
	echo -e "--clean deletes files; --dryrun must not change anything. Leaving...\n"
	exit 1
fi

####################################################################
# Auto-detect a .gcfg file in the call directory if none was given
if [ -z "${CONFIGFILE}" ]; then
	gcfg_files=( "${GENEVA_BUILDROOT}"/*.gcfg )
	if [ -e "${gcfg_files[0]}" ]; then
		if [ "${#gcfg_files[@]}" -eq 1 ]; then
			CONFIGFILE="${gcfg_files[0]}"
			_AUTODETECTED=1
		else
			echo -e "\nError: multiple .gcfg files found in '${GENEVA_BUILDROOT}'."
			echo -e "Please specify the configuration file explicitly. Leaving...\n"
			exit 1
		fi
	fi
fi

####################################################################
# Set built-in defaults, then override with the config file if provided.
CMAKE="/usr/bin/cmake"
BUILDMODE="Debug"
BUILDTESTCODE="1"
BUILDEXAMPLES="1"
BUILDBENCHMARKS="1"
BUILDSTATIC="0"
VERBOSEMAKEFILE="0"
INSTALLDIR="/opt/geneva"
MPIROOT=""
BUILDMPICONSUMER="0"
BUILDCUDAEXAMPLES="0"
USECUDARNG="0"

if [ -n "${CONFIGFILE}" ]; then
	testfile=$(basename "${CONFIGFILE}" .gcfg).gcfg
	if [ ! "$(basename "${CONFIGFILE}")" = "$testfile" ]; then
		echo -e "\nFile '${CONFIGFILE}' does not seem to be a Geneva config file, as it"
		echo -e "does not end in '.gcfg' as expected. Leaving...\n"
		exit 1
	fi
	if [ ! -e "${CONFIGFILE}" ]; then
		echo -e "\nError: File '${CONFIGFILE}' does not seem to exist.\nLeaving...\n"
		exit 1
	fi
	if [ "${_AUTODETECTED}" = "1" ]; then
		echo -e "\nUsing auto-detected configuration file '${CONFIGFILE}'"
	else
		echo -e "\nUsing configuration file '${CONFIGFILE}'"
	fi
	# shellcheck source=/dev/null
	. "${CONFIGFILE}"
else
	echo -e "\nNo Geneva config file provided — using built-in defaults."
	echo -e "See the Geneva 'scripts' directory for an example (genevaConfig.gcfg).\n"
fi

####################################################################
# Validate variables

_check_bool() {
	if [ ! "${2}" = "0" ] && [ ! "${2}" = "1" ]; then
		echo -e "\nError: Variable ${1} must be 0 or 1. Got '${2}'\nLeaving...\n"
		exit 1
	fi
}

if [ ! -x "${CMAKE}" ]; then
	echo -e "\nError: Could not find cmake executable '${CMAKE}'"
	echo -e "Please provide the correct path in the configuration file."
	echo -e "Leaving...\n"
	exit 1
fi

# Check if the BOOST* variables were set correctly: either ROOT or LIB+INCLUDE
# must be set, or none at all
if [ -z "${BOOSTROOT}" ] && [ -z "${BOOSTLIBS}" ] && [ -z "${BOOSTINCL}" ]; then
	echo "Variable BOOSTROOT wasn't set. Letting CMake search for a system Boost installation."
elif [ -z "${BOOSTLIBS}" ] && [ -n "${BOOSTINCL}" ]; then
	_BOOST_VAR_ERROR="true"
elif [ -n "${BOOSTLIBS}" ] && [ -z "${BOOSTINCL}" ]; then
	_BOOST_VAR_ERROR="true"
elif [ -n "${BOOSTROOT}" ] && [ -n "${BOOSTLIBS}" ] && [ -n "${BOOSTINCL}" ]; then
	_BOOST_VAR_ERROR="true"
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

case "${BUILDMODE}" in
	Release|Debug|RelWithDebInfo|MinSizeRel|Sanitize) ;;
	*) echo -e "\nError: Invalid build mode '${BUILDMODE}'. Leaving...\n"; exit 1 ;;
esac

_check_bool BUILDTESTCODE    "${BUILDTESTCODE}"
_check_bool BUILDEXAMPLES    "${BUILDEXAMPLES}"
_check_bool BUILDBENCHMARKS  "${BUILDBENCHMARKS}"
_check_bool BUILDSTATIC      "${BUILDSTATIC}"
_check_bool VERBOSEMAKEFILE  "${VERBOSEMAKEFILE}"
_check_bool BUILDMPICONSUMER   "${BUILDMPICONSUMER}"
_check_bool BUILDCUDAEXAMPLES  "${BUILDCUDAEXAMPLES}"
_check_bool USECUDARNG         "${USECUDARNG}"

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
# Guard against reconfiguring an already-configured build directory.

_confirm_and_clean() {
	echo -e "\nBuild directory '${GENEVA_BUILDROOT}' is already configured."
	echo -e "Warning: ALL files (except *.gcfg) will be permanently deleted."
	printf "Proceed? [y/N] "
	read -r _confirm
	case "${_confirm}" in
		[yY]|[yY][eE][sS])
			echo -en "Cleaning build directory '${GENEVA_BUILDROOT}' ..."
			find "${GENEVA_BUILDROOT}" -mindepth 1 -maxdepth 1 ! -name "*.gcfg" -exec rm -rf {} +
			echo -e " done\n"
			;;
		*)
			echo -e "Aborted. Leaving...\n"
			exit 1
			;;
	esac
}

if [ -e "${GENEVA_BUILDROOT}/CMakeCache.txt" ]; then
	if [ "${CLEAN}" = "1" ] || [ "${DRYRUN}" = "0" ]; then
		_confirm_and_clean
	fi
fi

####################################################################
# Do the actual call to cmake.
if [ -n "${BOOSTROOT}" ]; then
	BOOSTLOCATIONPATHS="-DBOOST_ROOT=${BOOSTROOT}"
elif [ -n "${BOOSTLIBS}" ]; then
	BOOSTLOCATIONPATHS="-DBOOST_LIBRARYDIR=${BOOSTLIBS} -DBOOST_INCLUDEDIR=${BOOSTINCL}"
fi

CONFIGURE="${CMAKE} $BOOSTLOCATIONPATHS \
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

####################################################################
# Optionally generate CMakeUserPresets.json for CLion integration.
if [ "${GENERATE_PRESET}" = "1" ]; then
	_preset_add() {
		[ -n "${_PRESET_VARS}" ] && _PRESET_VARS="${_PRESET_VARS},"
		_PRESET_VARS="${_PRESET_VARS}
        \"${1}\": { \"type\": \"${2}\", \"value\": \"${3}\" }"
	}

	_PRESET_VARS=""
	_preset_add "CMAKE_INSTALL_PREFIX"            "PATH"   "${INSTALLDIR}"
	_preset_add "CMAKE_VERBOSE_MAKEFILE"          "BOOL"   "${VERBOSEMAKEFILE}"
	_preset_add "GENEVA_BUILD_TYPE"               "STRING" "${BUILDMODE}"
	_preset_add "GENEVA_BUILD_TESTS"              "BOOL"   "${BUILDTESTCODE}"
	_preset_add "GENEVA_BUILD_EXAMPLES"           "BOOL"   "${BUILDEXAMPLES}"
	_preset_add "GENEVA_BUILD_BENCHMARKS"         "BOOL"   "${BUILDBENCHMARKS}"
	_preset_add "GENEVA_STATIC"                   "BOOL"   "${BUILDSTATIC}"
	_preset_add "GENEVA_BUILD_WITH_MPI_CONSUMER"  "BOOL"   "${BUILDMPICONSUMER}"
	_preset_add "GENEVA_BUILD_WITH_CUDA_EXAMPLES" "BOOL"   "${BUILDCUDAEXAMPLES}"
	_preset_add "GENEVA_USE_CUDA_RNG"             "BOOL"   "${USECUDARNG}"

	if [ -n "${BOOSTROOT}" ]; then
		_preset_add "BOOST_ROOT"       "PATH" "${BOOSTROOT}"
	elif [ -n "${BOOSTLIBS}" ]; then
		_preset_add "BOOST_LIBRARYDIR" "PATH" "${BOOSTLIBS}"
		_preset_add "BOOST_INCLUDEDIR" "PATH" "${BOOSTINCL}"
	fi

	[ "$MPIROOT" != "" ]          && _preset_add "MPI_HOME"               "PATH"   "${MPIROOT}"
	[ "$CXXEXTRAFLAGS" != "" ]    && _preset_add "CMAKE_CXX_FLAGS"        "STRING" "${CXXEXTRAFLAGS}"
	[ "$LINKEREXTRAFLAGS" != "" ] && _preset_add "CMAKE_EXE_LINKER_FLAGS" "STRING" "${LINKEREXTRAFLAGS}"

	PRESET_FILE="${PROJECTROOT}/CMakeUserPresets.json"
	cat > "${PRESET_FILE}" <<ENDOFPRESET
{
  "version": 3,
  "cmakeMinimumRequired": { "major": 3, "minor": 27, "patch": 0 },
  "configurePresets": [
    {
      "name": "geneva-from-script",
      "displayName": "Geneva (prepareBuild config)",
      "description": "Auto-generated by prepareBuild.sh -- do not edit manually",
      "generator": "Unix Makefiles",
      "binaryDir": "${GENEVA_BUILDROOT}",
      "cacheVariables": {${_PRESET_VARS}
      }
    }
  ]
}
ENDOFPRESET

	if [ "$CMAKEEXTRAFLAGS" != "" ]; then
		echo -e "Note: CMAKEEXTRAFLAGS='${CMAKEEXTRAFLAGS}' was not written to ${PRESET_FILE}."
		echo -e "      Add these flags manually to the preset's 'cacheVariables' if needed.\n"
	fi
	echo -e "Generated '${PRESET_FILE}' for CLion integration.\n"
fi

####################################################################
echo -e "\nConfiguring with command: \"${CONFIGURE} ${PROJECTROOT}\"\n"
echo -e "---------------------------------------------------------------------\n"

if [ "${DRYRUN}" = "1" ]; then
	echo -e "Dry run: cmake was NOT executed. Copy the command above into your"
	echo -e "IDE (e.g. JetBrains CLion) as the CMake options / command line.\n"
else
	# Word-splitting of CONFIGURE is intentional here — it holds cmake flags
	# shellcheck disable=SC2086
	if eval ${CONFIGURE} ${PROJECTROOT}; then
		echo -e "\n---------------------------------------------------------------------"
		echo -e "\nYou may now build and install Geneva in the usual way:"
		echo -e "make\t\t# Use '-jn', where 'n' is the number of cores in your system"
		echo -e "make install\n"
	fi
fi

####################################################################
# Done
