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

#
# This module is needed to setup the Geneva build system, and may also be used
# to compile independent Geneva applications.
#

################################################################################

CMAKE_MINIMUM_REQUIRED(VERSION 3.27 FATAL_ERROR)

# Include guard
IF(NOT COMMON_GENEVA_BUILD_INCLUDED)
	SET(COMMON_GENEVA_BUILD_INCLUDED TRUE)

	################################################################################
	# Set some default values for user-settings. Generally, these should be provided
	# through a custom genevaConfig.gcfg, in conjunction with the prepareBuild.sh script.

	IF( NOT DEFINED GENEVA_BUILD_TYPE )
		SET( GENEVA_BUILD_TYPE "Release" )
	ENDIF()

	IF( NOT DEFINED GENEVA_BUILD_TESTS )
		SET( GENEVA_BUILD_TESTS TRUE )
	ENDIF()

	IF( NOT DEFINED CMAKE_VERBOSE_MAKEFILE )
		SET( CMAKE_VERBOSE_MAKEFILE FALSE )
	ENDIF()

	###############################################################################
	# Include host and compiler identification functions

	INCLUDE(IdentifySystemParameters)

	################################################################################
	# Validate the build type chosen by the user. This macro also sets
	# the variable CMAKE_BUILD_TYPE accordingly.

	VALIDATE_BUILD_TYPE()

	################################################################################
	# Identify the operating system

	FIND_HOST_OS (
			"GENEVA_OS_NAME"
			"GENEVA_OS_VERSION"
	)

	###############################################################################
	# Identify unsupported setups as early as possible

	FLAG_UNSUPPORTED_SETUPS(
			${GENEVA_OS_NAME}
			${GENEVA_OS_VERSION}
			${GENEVA_BUILD_TYPE}
	)

	################################################################################
	# Set the C++ standard to be used

	# Geneva requires at least the C++23 Standard. The user may force a NEWER value
	# at his own risk by setting CMAKE_CXX_STANDARD; anything below 23 is rejected.
	IF( NOT DEFINED CMAKE_CXX_STANDARD )
		SET( CMAKE_CXX_STANDARD "23" )
	ELSEIF( CMAKE_CXX_STANDARD LESS 23 )
		MESSAGE(FATAL_ERROR
			"Geneva requires C++23 or newer, but CMAKE_CXX_STANDARD=${CMAKE_CXX_STANDARD} was requested.")
	ENDIF()

	SET(CMAKE_CXX_STANDARD_REQUIRED ON)
	set(CMAKE_CXX_EXTENSIONS OFF)

	# The CUDA device dialect is set centrally here too, and DELIBERATELY trails the host C++ standard:
	# nvcc (CUDA 13.3) does not implement a C++23 device dialect yet, and Geneva's device code (.cu) is
	# intentionally kept at C++20. Setting it centrally keeps every CUDA target off the inherited C++23
	# (which nvcc would reject) without any per-target restatement. Only relevant when CUDA is enabled;
	# harmless as an unused variable otherwise. Raise this in lockstep once nvcc gains C++23 device support.
	IF( NOT DEFINED CMAKE_CUDA_STANDARD )
		SET( CMAKE_CUDA_STANDARD "20" )
	ENDIF()
	SET(CMAKE_CUDA_STANDARD_REQUIRED ON)

	################################################################################
	# Set the compiler and linker flags

	SET_COMPILER_FLAGS (
			${GENEVA_OS_NAME}
			${GENEVA_OS_VERSION}
			${GENEVA_BUILD_TYPE}
	)

	SET_LINKER_FLAGS (
			${GENEVA_OS_NAME}
			${GENEVA_OS_VERSION}
			${GENEVA_BUILD_TYPE}
	)

	################################################################################
	# Set other necessary build flags

	GET_BUILD_FLAGS (
			${GENEVA_OS_NAME}
			${GENEVA_OS_VERSION}
			${GENEVA_BUILD_TYPE}
			"PLATFORM_NEEDS_LIBRARY_LINKING"
	)

	################################################################################
	# Geneva only supports shared libraries

	################################################################################
	# Set the preprocessor definition for enabling testing code

	IF ( GENEVA_BUILD_TESTS )
		ADD_DEFINITIONS("-DGEM_TESTING")
	ENDIF ()

	################################################################################
	# Define the required Boost environment

	SET (Boost_USE_MULTITHREAD ON)
	SET (Boost_ADDITIONAL_VERSIONS
			"1.90.0"
			"1.90"
	)

	SET (Boost_USE_STATIC_LIBS OFF)

	# The minimum Boost version required for building Geneva and Geneva applications
	SET (GENEVA_MIN_BOOST_VERSION 1.91)

	# These are the libraries required for any Geneva build
	SET (
			GENEVA_BOOST_LIBS
			atomic
			filesystem
			json
			regex
			serialization
			program_options
	)

	# Use Boost's own BoostConfig.cmake (available since Boost 1.70) rather than
	# CMake's legacy FindBoost module, which was removed in CMake 4.x (CMP0167).
	if(POLICY CMP0167)
		cmake_policy(SET CMP0167 NEW)
	endif()
	if(POLICY CMP0153)
		cmake_policy(SET CMP0153 NEW)
	endif()

	# Search for the required libraries
	MESSAGE("Searching for Boost...\n")
	FIND_PACKAGE(
			Boost
			${GENEVA_MIN_BOOST_VERSION} REQUIRED
			COMPONENTS ${GENEVA_BOOST_LIBS}
	)
	MESSAGE("")

	IF (GENEVA_BUILD_TESTS)
		MESSAGE("Searching for Catch2...\n")
		FIND_PACKAGE(Catch2 3 REQUIRED)
		MESSAGE("")
	ENDIF()

	INCLUDE_DIRECTORIES(${Boost_INCLUDE_DIRS})

	# Optionally Search for MPI
	IF(GENEVA_BUILD_WITH_MPI_CONSUMER)
		MESSAGE("Searching for MPI...\n")
		FIND_PACKAGE(MPI REQUIRED)
		MESSAGE("")
		IF(MPI_FOUND)
			INCLUDE_DIRECTORIES(${MPI_INCLUDE_PATH})
		ENDIF()
	ENDIF()

	################################################################################
	# The names of the Geneva libraries

	SET ( COMMON_LIBNAME            "gemfony-common" )
	SET ( DIETRICH_LIBNAME          "gemfony-dietrich" )
	SET ( HAP_LIBNAME               "gemfony-hap" )
	SET ( COURTIER_LIBNAME          "gemfony-courtier" )
	# The GPU consumer is folded INTO gemfony-courtier as an opt-in add-on (GENEVA_BUILD_WITH_GPU_CONSUMER),
	# exactly like the MPI consumer: when enabled, courtier dynamically links the CUDA/OpenCL backends; when
	# disabled, no GPU code is compiled. There is no separate GPU library.
	SET ( GENEVA_LIBNAME            "gemfony-geneva" )
	# The geneva-individuals library was dissolved into the geneva library: the sample
	# individuals now live in Gem::Geneva::Individuals under geneva/{include,src}/individuals/,
	# compiled into gemfony-geneva. There is no separate individuals library (and no
	# GENEVA_INDIVIDUAL_LIBNAME variable — it was removed as an unused leftover of the fold-in).

	# The order of the entries is important, as it translates to the linking
	# order in TARGET_LINK_LIBRARIES() later...
	# Dietrich (plotting) is a leaf peer on top of common, used by geneva; it links
	# after hap so geneva -> dietrich -> common resolves left-to-right.
	SET (
			GENEVA_LIBNAMES
			${GENEVA_LIBNAME}
			${COURTIER_LIBNAME}
			${HAP_LIBNAME}
			${DIETRICH_LIBNAME}
			${COMMON_LIBNAME}
	)

	# This variable contains the library names.
	# The function TARGET_LINK_LIBRARIES() can use either variant.
	SET ( GENEVA_LIBRARIES ${GENEVA_LIBNAMES} )

	################################################################################
	# Add a custom target to run a "make clean" and remove temporaries,
	# so the configuration process may start fresh.

	IF (NOT TARGET "clean-cmake")
		ADD_CUSTOM_TARGET(
				"clean-cmake"
				COMMAND ${CMAKE_BUILD_TOOL} clean 2>&1 > /dev/null
				COMMAND ${CMAKE_COMMAND} -DOLD_CMAKE=${OLD_CMAKE} -P ${PROJECT_SOURCE_DIR}/CMakeModules/CleanCmakeTemporaries.cmake
		)
	ENDIF ()

	################################################################################
	# Set the installation locations

	IF (INSTALL_PREFIX_INCLUDES AND INSTALL_PREFIX_LIBS
			AND INSTALL_PREFIX_DOCS AND INSTALL_PREFIX_DATA)
		# All are set, we ignore CMAKE_INSTALL_PREFIX and install
		# each kind of files in its own location
		SET( INFO_INSTALL_PREFIX                       "\n\t\t(libs)\t\t${INSTALL_PREFIX_LIBS}" )
		SET( INFO_INSTALL_PREFIX "${INFO_INSTALL_PREFIX}\n\t\t(headers)\t${INSTALL_PREFIX_INCLUDES}" )
		SET( INFO_INSTALL_PREFIX "${INFO_INSTALL_PREFIX}\n\t\t(docs)\t\t${INSTALL_PREFIX_DOCS}" )
		SET( INFO_INSTALL_PREFIX "${INFO_INSTALL_PREFIX}\n\t\t(other files)\t${INSTALL_PREFIX_DATA}" )
	ELSEIF (NOT INSTALL_PREFIX_INCLUDES AND NOT INSTALL_PREFIX_LIBS
			AND NOT INSTALL_PREFIX_DOCS AND NOT INSTALL_PREFIX_DATA)
		# All unset, Geneva is installed as a standalone tree in CMAKE_INSTALL_PREFIX
		IF (NOT INSTALL_PREFIX_ROOT)
			IF (CMAKE_INSTALL_PREFIX)
				SET( INSTALL_PREFIX_ROOT ${CMAKE_INSTALL_PREFIX} )
			ELSE ()
				# If no value was set, use relative paths
				SET( INSTALL_PREFIX_ROOT "." )
			ENDIF ()
		ENDIF ()

		SET( INSTALL_PREFIX_INCLUDES "${INSTALL_PREFIX_ROOT}/include" )
		SET( INSTALL_PREFIX_LIBS     "${INSTALL_PREFIX_ROOT}/lib" )
		SET( INSTALL_PREFIX_DOCS     "${INSTALL_PREFIX_ROOT}" )
		SET( INSTALL_PREFIX_DATA     "${INSTALL_PREFIX_ROOT}" )
		SET( INFO_INSTALL_PREFIX     "${INSTALL_PREFIX_ROOT}" )
	ELSE ()
		# Inconsistent settings
		MESSAGE (FATAL_ERROR "Please set either all four or none of the"
				" installation prefix values INSTALL_PREFIX_INCLUDES,"
				" INSTALL_PREFIX_LIBS, INSTALL_PREFIX_DOCS, and INSTALL_PREFIX_DATA .")
	ENDIF ()

	################################################################################
	# Install RPATH so an INSTALLED host executable (and every runtime-loadable module) finds the Geneva
	# shared libraries -- and the module directory -- without LD_LIBRARY_PATH. Nothing set an RPATH before,
	# so an installed binary relied on the loader's default search path; once individuals / algorithms /
	# consumers ship as modules alongside the libs this must be self-locating. The Geneva libraries always
	# install into <prefix>/lib (INSTALL_PREFIX_LIBS); executables land at various depths under the prefix
	# (<prefix>/..., <prefix>/examples/<name>/, ...), so the RPATH lists $ORIGIN-relative entries covering
	# those depths (relocatable) plus the absolute lib dir as a backstop. USE_LINK_PATH also records the
	# link-time dependency dirs (e.g. Boost). A user-supplied CMAKE_INSTALL_RPATH is respected.
	IF (NOT CMAKE_INSTALL_RPATH)
		SET (CMAKE_INSTALL_RPATH
			"$ORIGIN/../lib;$ORIGIN/../../lib;$ORIGIN/../../../lib;${INSTALL_PREFIX_LIBS}")
	ENDIF ()
	SET (CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)

	################################################################################
	# Print a summary of the build settings before continuing with the main script

	MESSAGE ("========================================")
	MESSAGE ("")
	MESSAGE ("Building:")
	MESSAGE ("\tthe Geneva library collection")
	MESSAGE ("\tdynamically linked")
	IF (GENEVA_BUILD_TESTS)
		MESSAGE ("\tincluding testing code")
	ELSE ()
		MESSAGE ("\twithout testing code")
	ENDIF ()
	# Don't print the useless build type on multi-config generators
	IF(NOT CMAKE_CONFIGURATION_TYPES)
		MESSAGE ("\tin ${CMAKE_BUILD_TYPE} mode")
		IF ("${GENEVA_BUILD_TYPE}" STREQUAL "Sanitize")
			MESSAGE ("\twith -fsanitize=thread (if available)")
		ENDIF ()
	ENDIF ()

	MESSAGE ("\twith Boost include location:\t ${Boost_INCLUDE_DIRS}")
	MESSAGE ("\twith Boost library location:\t ${Boost_LIBRARY_DIRS}")

	IF(GENEVA_BUILD_WITH_OPENCL_EXAMPLES)
		MESSAGE ("\twith OpenCL include location:\t ${OpenCL_INCLUDE_DIRS}")
		MESSAGE ("\twith OpenCL library location:\t ${OpenCL_LIBRARIES}")
		MESSAGE ("\twith with highest supported OpenCL version ${OpenCL_VERSION_STRING}")
	ENDIF()

	MESSAGE ("\tusing compiler:\t\t\t ${CMAKE_CXX_COMPILER_ID} v${CMAKE_CXX_COMPILER_VERSION}")
	# Don't try to access the build type on multi-config generators
	IF(NOT CMAKE_CONFIGURATION_TYPES)
		STRING (TOUPPER ${CMAKE_BUILD_TYPE} B_MODE)
		SET (CMAKE_CXX_FLAGS_ALL "${CMAKE_CXX_FLAGS} ${CMAKE_CXX_FLAGS_${B_MODE}}")
		STRING (STRIP "${CMAKE_CXX_FLAGS_ALL}" CMAKE_CXX_FLAGS_STRIPPED)
		STRING (REGEX REPLACE "[ \t]+" "\n\t\t\t\t\t " CMAKE_CXX_FLAGS_SEP ${CMAKE_CXX_FLAGS_STRIPPED})
		MESSAGE ("\twith C++ compiler flags:\t ${CMAKE_CXX_FLAGS_SEP}")
	ENDIF ()

	MESSAGE("\tUsing C++ standard ${CMAKE_CXX_STANDARD}")

	# Don't print linker options if empty
	STRING (STRIP "${CMAKE_EXE_LINKER_FLAGS}" CMAKE_L_FLAGS_STRIPPED)
	IF(NOT "${CMAKE_L_FLAGS_STRIPPED}" STREQUAL "")
		STRING (REGEX REPLACE "[ \t]+" "\n\t\t\t\t\t " CMAKE_L_FLAGS_SEP ${CMAKE_L_FLAGS_STRIPPED})
		MESSAGE ("\twith extra linker flags:\t ${CMAKE_L_FLAGS_SEP}")
	ENDIF ()
	IF( CMAKE_VERBOSE_MAKEFILE )
		MESSAGE ("\tproducing verbose CMake output")
	ELSE()
		MESSAGE ("\tproducing sparse CMake output")
	ENDIF()
	MESSAGE ("\tfor operating system:\t\t ${GENEVA_OS_NAME}")
	MESSAGE ("\tto install into prefix:\t\t ${INFO_INSTALL_PREFIX}")
	MESSAGE ("")
	MESSAGE ("========================================\n")

	###############################################################################
	# Drift-proof aggregate-target helper.
	#
	# GENEVA_AGGREGATE_TARGET(<name>) creates a custom target depending on EVERY
	# buildsystem target defined in the current directory and all of its
	# subdirectories (recursively). Call it AFTER the ADD_SUBDIRECTORY() calls of
	# a CMakeLists. This replaces hand-maintained DEPENDS lists -- which have
	# silently drifted in the past (commented-out entries, missing sub-aggregates,
	# new tests/benchmarks/examples never added) -- so that anything added under
	# the directory is picked up automatically.

	FUNCTION(_GENEVA_COLLECT_TARGETS_RECURSIVE _out_var _dir)
		GET_PROPERTY(_subdirs DIRECTORY "${_dir}" PROPERTY SUBDIRECTORIES)
		GET_PROPERTY(_targets DIRECTORY "${_dir}" PROPERTY BUILDSYSTEM_TARGETS)
		SET(_acc "")
		# Skip targets that opt out of the aggregate (GENEVA_EXCLUDE_FROM_AGGREGATE): opt-in maintenance
		# targets (e.g. config-reference, which regenerates a version-controlled tree) must not run as part
		# of a normal build.
		FOREACH(_t ${_targets})
			GET_TARGET_PROPERTY(_excl ${_t} GENEVA_EXCLUDE_FROM_AGGREGATE)
			IF(NOT _excl)
				LIST(APPEND _acc ${_t})
			ENDIF()
		ENDFOREACH()
		FOREACH(_sub ${_subdirs})
			_GENEVA_COLLECT_TARGETS_RECURSIVE(_child "${_sub}")
			LIST(APPEND _acc ${_child})
		ENDFOREACH()
		SET(${_out_var} "${_acc}" PARENT_SCOPE)
	ENDFUNCTION()

	FUNCTION(GENEVA_AGGREGATE_TARGET _name)
		_GENEVA_COLLECT_TARGETS_RECURSIVE(_collected "${CMAKE_CURRENT_SOURCE_DIR}")
		IF(_collected)
			LIST(REMOVE_DUPLICATES _collected)
		ENDIF()
		ADD_CUSTOM_TARGET("${_name}" DEPENDS ${_collected}
			COMMENT "Building all auto-collected targets for \"${_name}\".")
	ENDFUNCTION()

	###############################################################################
	# GENEVA_MATERIALIZE_CONFIGS(<target> <install-config-dest>)
	#
	# Wires a config-owning binary so its configuration directory is MATERIALIZED FROM CODE at build time
	# rather than copied from a hand-maintained set of shipped files. After <target> is built it is run once
	# with --update-configs, which (re)creates its configuration files from the code's registered defaults
	# into <build-dir>/config; that directory's intentional overrides (config/config-overrides.json in the
	# source tree, if present) are then overlaid onto them via the GConfigOverlay helper. The materialized
	# directory is installed to <install-config-dest> (pass an empty string to skip installation).
	#
	# Any extra arguments after <install-dest> are passed to the binary before --update-configs (e.g. a
	# plugin loader's "--individual <plugin.so>"); if such an argument references another target via a
	# $<TARGET_FILE:...> genex, make <target> depend on it so it is built first.
	#
	# This replaces the former per-directory FILE(COPY config) + INSTALL(FILES ...). It MUST be called from
	# the same CMakeLists.txt that defines <target>, because a POST_BUILD command may only be attached to a
	# target in the current directory -- i.e. in place of the former ADD_SUBDIRECTORY(config).
	FUNCTION(GENEVA_MATERIALIZE_CONFIGS _target _install_dest)
		SET(_extra_args ${ARGN})
		SET(_cfg_dir ${CMAKE_CURRENT_BINARY_DIR}/config)
		SET(_overrides ${CMAKE_CURRENT_SOURCE_DIR}/config/config-overrides.json)

		# Ensure the directory exists at configure time so the INSTALL(DIRECTORY) rule below is valid; the
		# POST_BUILD step (re)populates it from code.
		FILE(MAKE_DIRECTORY ${_cfg_dir})

		# The overlay helper must exist before this target's POST_BUILD step runs it.
		IF(TARGET GConfigOverlay)
			ADD_DEPENDENCIES(${_target} GConfigOverlay)
		ENDIF()

		# POST_BUILD: (re)materialize the config directory from the code defaults. Running the freshly built
		# binary with --update-configs creates any missing config from defaults and rewrites it canonically.
		ADD_CUSTOM_COMMAND(TARGET ${_target} POST_BUILD
				COMMAND ${CMAKE_COMMAND} -E rm -rf ${_cfg_dir}
				COMMAND ${CMAKE_COMMAND} -E make_directory ${_cfg_dir}
				COMMAND ${_target} ${_extra_args} --update-configs
				WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
				COMMENT "Materializing ${_target} configuration from code defaults"
				VERBATIM)

		# Overlay this directory's intentional overrides, if it ships a fragment.
		IF(EXISTS ${_overrides})
			ADD_CUSTOM_COMMAND(TARGET ${_target} POST_BUILD
					COMMAND $<TARGET_FILE:GConfigOverlay> ${_cfg_dir} ${_overrides}
					COMMENT "Overlaying ${_target} configuration overrides"
					VERBATIM)
		ENDIF()

		# Install the materialized directory's contents into <install-config-dest>.
		IF(NOT "${_install_dest}" STREQUAL "")
			INSTALL(DIRECTORY ${_cfg_dir}/ DESTINATION ${_install_dest})
		ENDIF()
	ENDFUNCTION()

	###############################################################################
	# GENEVA_ADD_INDIVIDUAL_MODULE(<target> <source> [<source> ...])
	#
	# Builds a runtime-loadable individual (optimization-problem) plugin as a shared MODULE, following the
	# reference pattern of example 19. The .so carries the problem code + its serialization GUID + the
	# GENEVA_INDIVIDUAL_PLUGIN() entry points, and links NONE of the Geneva libraries -- it needs only their
	# HEADERS at compile time and resolves their symbols from the host process at load (the loader uses
	# RTLD_GLOBAL). Linking no Geneva libs also sidesteps the non-PIC static Catch2 that a testing-enabled
	# Geneva pulls in as a usage requirement, which cannot go into a shared object.
	#
	# The module keeps the "lib" prefix (CMake MODULE libraries drop it by default) so the artifact is named
	# lib<target>.so, matching the individual-plugin convention. The caller's own source directory is on the
	# include path so the glue translation unit finds its problem header.
	FUNCTION(GENEVA_ADD_INDIVIDUAL_MODULE _target)
		IF(${ARGC} LESS 2)
			MESSAGE(FATAL_ERROR
				"GENEVA_ADD_INDIVIDUAL_MODULE(${_target}): at least one source file is required")
		ENDIF()
		ADD_LIBRARY(${_target} MODULE ${ARGN})
		# MODULE libraries drop the "lib" prefix by default; keep it so the .so is named lib<target>.so.
		SET_TARGET_PROPERTIES(${_target} PROPERTIES PREFIX "lib")
		TARGET_INCLUDE_DIRECTORIES(${_target} PRIVATE
			${CMAKE_CURRENT_SOURCE_DIR}
			${PROJECT_SOURCE_DIR}/common/include
			${PROJECT_SOURCE_DIR}/hap/include
			${PROJECT_SOURCE_DIR}/courtier/include
			${PROJECT_SOURCE_DIR}/dietrich/include
			${PROJECT_SOURCE_DIR}/geneva/include
			${Boost_INCLUDE_DIRS})
	ENDFUNCTION()

	###############################################################################
	# GENEVA_DECLARE_INDIVIDUAL(<name>
	#     MODE    <load|compile|both>
	#     SOURCES <individual.cpp> [<more.cpp> ...]
	#     [PLUGIN <glue.cpp>]
	#     [CONFIG <path>])
	#
	# The packaging declaration for a Geneva optimization individual. It encodes the maintainer's DA1b model:
	# the SAME individual source can be packaged two ways at once, and the author states the intent at the
	# definition site. It ONLY packages -- it never modifies or generates any C++ (serialization export stays
	# the author's hand-written BOOST_CLASS_EXPORT_KEY/_IMPLEMENT, one per type, in the individual's own hpp/cpp).
	#
	#   MODE compile  -> an OBJECT library <name>-obj built from SOURCES. A binary that COMPILES the individual
	#                    in links this target, so the individual's BOOST_CLASS_EXPORT_IMPLEMENT static
	#                    initializers (serialization GUID registration) are pulled in and never stripped.
	#   MODE load     -> a shared MODULE lib<name>.so built from SOURCES + the PLUGIN glue TU. It links NONE of
	#                    the Geneva libraries (their symbols resolve from the host process at load via
	#                    RTLD_GLOBAL; only their headers are needed) and is loaded at runtime via --individual.
	#   MODE both     -> both of the above; the module reuses the object library's compiled objects
	#                    ($<TARGET_OBJECTS:...>) rather than recompiling SOURCES.
	#
	# The PLUGIN glue TU (the fixed extern "C" geneva_module_manifest() entry point) is compiled into the
	# module .so ONLY, never the object library: the manifest symbol name is fixed, so a compile-in binary that
	# links two individuals must not contain two definitions of it. Hence SOURCES = the individual (class +
	# export, in both targets) and PLUGIN = the glue (module only). The ONE hard rule the author must honour
	# (not enforceable in CMake): a single PROCESS must never both compile-in and load the same individual, or
	# Boost.Serialization throws on the duplicate GUID registration.
	#
	# CONFIG, when given, is recorded on the module target (property GENEVA_INDIVIDUAL_CONFIG) for downstream
	# config materialization; it is the same path the author passes to individualManifest<> in the glue TU.
	FUNCTION(GENEVA_DECLARE_INDIVIDUAL _name)
		CMAKE_PARSE_ARGUMENTS(GDI "" "MODE;PLUGIN;CONFIG" "SOURCES" ${ARGN})

		IF(NOT GDI_MODE)
			MESSAGE(FATAL_ERROR "GENEVA_DECLARE_INDIVIDUAL(${_name}): MODE <load|compile|both> is required")
		ENDIF()
		IF(NOT GDI_MODE MATCHES "^(load|compile|both)$")
			MESSAGE(FATAL_ERROR
				"GENEVA_DECLARE_INDIVIDUAL(${_name}): MODE must be load, compile or both (got '${GDI_MODE}')")
		ENDIF()
		IF(NOT GDI_SOURCES)
			MESSAGE(FATAL_ERROR "GENEVA_DECLARE_INDIVIDUAL(${_name}): SOURCES <individual.cpp> is required")
		ENDIF()

		SET(_want_compile FALSE)
		SET(_want_load FALSE)
		IF(GDI_MODE STREQUAL "compile" OR GDI_MODE STREQUAL "both")
			SET(_want_compile TRUE)
		ENDIF()
		IF(GDI_MODE STREQUAL "load" OR GDI_MODE STREQUAL "both")
			SET(_want_load TRUE)
		ENDIF()

		# The PLUGIN glue TU is the module's manifest entry point: required for a loadable module, and
		# meaningless (and unsafe -- fixed symbol name) outside one.
		IF(_want_load AND NOT GDI_PLUGIN)
			MESSAGE(FATAL_ERROR
				"GENEVA_DECLARE_INDIVIDUAL(${_name}): MODE ${GDI_MODE} needs PLUGIN <glue.cpp> (the module entry point)")
		ENDIF()
		IF(GDI_PLUGIN AND NOT _want_load)
			MESSAGE(FATAL_ERROR
				"GENEVA_DECLARE_INDIVIDUAL(${_name}): PLUGIN is only valid for MODE load or both")
		ENDIF()

		SET(_gdi_incdirs
			${CMAKE_CURRENT_SOURCE_DIR}
			${PROJECT_SOURCE_DIR}/common/include
			${PROJECT_SOURCE_DIR}/hap/include
			${PROJECT_SOURCE_DIR}/courtier/include
			${PROJECT_SOURCE_DIR}/dietrich/include
			${PROJECT_SOURCE_DIR}/geneva/include
			${Boost_INCLUDE_DIRS})

		# (A) compile/both: the OBJECT library of the individual's own sources (class + BOOST_CLASS_EXPORT).
		# PIC so its objects are equally usable in the shared module (both) and in any consumer.
		IF(_want_compile)
			ADD_LIBRARY(${_name}-obj OBJECT ${GDI_SOURCES})
			SET_TARGET_PROPERTIES(${_name}-obj PROPERTIES POSITION_INDEPENDENT_CODE ON)
			TARGET_INCLUDE_DIRECTORIES(${_name}-obj PUBLIC ${_gdi_incdirs})
			# NOTE on link order: a compile-in consumer must list <name>-obj BEFORE the Geneva and Boost
			# libraries. The object library's own objects instantiate Boost.Serialization for the individual's
			# exported types; the linker's default --as-needed drops (shared) libboost_serialization if nothing
			# preceding it needs it, so the object library's objects -- if placed last -- fail to resolve. CMake
			# deduplicates a shared library to its first position, so a usage-requirement dependency here cannot
			# reorder it; the consumer must order the object library first. The module .so is unaffected: it
			# reuses $<TARGET_OBJECTS:...> (object files only) at the front of its own link.
		ENDIF()

		# (B) load/both: the shared MODULE lib<name>.so. It reuses the object library's objects where they
		# exist (both) or compiles SOURCES itself (load-only), plus the PLUGIN glue (module ONLY). It links no
		# Geneva libraries -- exactly the GENEVA_ADD_INDIVIDUAL_MODULE recipe.
		IF(_want_load)
			IF(_want_compile)
				ADD_LIBRARY(${_name} MODULE $<TARGET_OBJECTS:${_name}-obj> ${GDI_PLUGIN})
			ELSE()
				ADD_LIBRARY(${_name} MODULE ${GDI_SOURCES} ${GDI_PLUGIN})
			ENDIF()
			# MODULE libraries drop the "lib" prefix by default; keep it so the .so is named lib<name>.so.
			SET_TARGET_PROPERTIES(${_name} PROPERTIES PREFIX "lib")
			TARGET_INCLUDE_DIRECTORIES(${_name} PRIVATE ${_gdi_incdirs})
			IF(GDI_CONFIG)
				SET_TARGET_PROPERTIES(${_name} PROPERTIES GENEVA_INDIVIDUAL_CONFIG "${GDI_CONFIG}")
			ENDIF()
		ENDIF()
	ENDFUNCTION()

	###############################################################################
	# End of the include-guard

ENDIF(NOT COMMON_GENEVA_BUILD_INCLUDED)

################################################################################
# Done
