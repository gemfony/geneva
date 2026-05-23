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

IF (NOT GENEVA_FULL_TREE_BUILD)

	PROJECT(Geneva_Benchmarks_All)

	# For building the examples independently, we still assume that the
	# 'CMakeModules' folder can be found at the same place as the current
	# source folder... This avoids having to copy the same content over
	# and over again in all the examples.
	SET(CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/../CMakeModules")

	MESSAGE("\nPerforming an independent build of ${PROJECT_NAME}")
	INCLUDE(CommonGenevaBuild)

	# In a full-tree build the top-level CMakeLists.txt probes for CUDA.
	# In a standalone build we must do it here so that CMAKE_CUDA_COMPILER_LOADED
	# is set before benchmarks/geneva/CMakeLists.txt evaluates its guard.
	if(NOT GENEVA_SKIP_CUDA)
		# Derive CUDAToolkit_ROOT from CMAKE_CUDA_COMPILER when only the compiler
		# path was supplied (standard NVIDIA layout: <root>/bin/nvcc).
		if(DEFINED CMAKE_CUDA_COMPILER AND NOT DEFINED CUDAToolkit_ROOT)
			get_filename_component(_geneva_cuda_bindir "${CMAKE_CUDA_COMPILER}" DIRECTORY)
			get_filename_component(_geneva_cuda_root   "${_geneva_cuda_bindir}"  DIRECTORY)
			set(CUDAToolkit_ROOT "${_geneva_cuda_root}"
				CACHE PATH "CUDA toolkit root (derived from CMAKE_CUDA_COMPILER)")
			unset(_geneva_cuda_bindir)
			unset(_geneva_cuda_root)
		endif()

		include(CheckLanguage)
		check_language(CUDA)
		if(CMAKE_CUDA_COMPILER)
			enable_language(CUDA)
		endif()
	endif()

ENDIF ()

ADD_SUBDIRECTORY ( common )
ADD_SUBDIRECTORY ( courtier )
ADD_SUBDIRECTORY ( geneva )

GENEVA_AGGREGATE_TARGET( "benchmarks" )
