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
CMAKE_MINIMUM_REQUIRED(VERSION 2.8)

IF (NOT GENEVA_FULL_TREE_BUILD)

	PROJECT(Geneva_Examples_Geneva)

	# For building the examples independently, we still assume that the
	# 'CMakeModules' folder can be found at the same place as the current
	# source folder... This avoids having to copy the same content over
	# and over again in all the examples.
	SET(CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/CMakeModules")

	MESSAGE("\nPerforming an independent build of ${PROJECT_NAME}")
	INCLUDE(CommonGenevaBuild)

ENDIF ()

ADD_CUSTOM_TARGET( "examples-geneva"
	# Add all the available example's targets (EXECUTABLENAMEs) here
	DEPENDS GSimpleOptimizer
		GParaboloid2D
		GParameterObjectUsagePatterns
		GMultiCriterionParabola
		GDirectEA
        GDirectSA
		GDirectSwarm
		GFunctionMinimizer
        GPostProcessing
		GNeuralNetwork
		GStarter
		GMetaOptimizer
		GExternalEvaluator
		GPluggableOptimizationMonitors
		GDependentConstraints
	COMMENT "Building all the examples for the Optimization library."
)

ADD_SUBDIRECTORY( 01_GSimpleOptimizer )
ADD_SUBDIRECTORY( 02_GParaboloid2D )
ADD_SUBDIRECTORY( 03_GParameterObjectUsagePatterns )
ADD_SUBDIRECTORY( 04_GMultiCriterionParabola )
ADD_SUBDIRECTORY( 06_GDirectEA )
ADD_SUBDIRECTORY( 06_GDirectSA )
ADD_SUBDIRECTORY( 06_GDirectSwarm )
ADD_SUBDIRECTORY( 07_GFunctionMinimizer )
ADD_SUBDIRECTORY( 08_GPostProcessing )
ADD_SUBDIRECTORY( 09_GNeuralNetwork )
ADD_SUBDIRECTORY( 10_GStarter )
ADD_SUBDIRECTORY( 11_GMetaOptimizer )
ADD_SUBDIRECTORY( 12_GExternalEvaluator )
ADD_SUBDIRECTORY( 13_GPluggableOptimizationMonitors )
ADD_SUBDIRECTORY( 14_GDependentConstraints )

IF(GENEVA_BUILD_WITH_OPENCL_EXAMPLES AND OpenCL_FOUND)
    ADD_SUBDIRECTORY( 15_GOpenCLConsumer )
ENDIF()
