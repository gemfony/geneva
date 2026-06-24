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

SET(DEPENDENCIES
		GSimpleOptimizer
		GParaboloid2D
		GParameterObjectUsagePatterns
		GMultiCriterionParabola
		GDirectEA
		GDirectSA
		GDirectSwarm
		GFunctionMinimizer
		GPostProcessing
		GNeuralNetwork
		GMetaOptimizer
		GExternalEvaluator
		GPluggableOptimizationMonitors
		GDependentConstraints
		GAlgorithmChaining)

IF(GENEVA_BUILD_WITH_MPI_CONSUMER)
	SET(DEPENDENCIES
			${DEPENDENCIES}
			GMPIConsumer
			GMPISubClients)
ENDIF()

IF(GENEVA_BUILD_WITH_GPU_CONSUMER)
	SET(DEPENDENCIES
		${DEPENDENCIES}
		GCUDAWorker)
ENDIF()

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
# 10_GStarter is intentionally NOT built in-tree: it ships as the canonical
# standalone downstream template that consumes an installed Geneva via
# find_package(Geneva). See examples/geneva/10_GStarter/README.
ADD_SUBDIRECTORY( 11_GMetaOptimizer )
ADD_SUBDIRECTORY( 12_GExternalEvaluator )
ADD_SUBDIRECTORY( 13_GPluggableOptimizationMonitors )
ADD_SUBDIRECTORY( 14_GDependentConstraints )

# Example 15 (Mona-Lisa) drives the courtier GPU consumer, so it is built only when that consumer is
# (GENEVA_BUILD_WITH_GPU_CONSUMER -- the opt-in add-on folded into gemfony-courtier). Its kernel is
# runtime-compiled (NVRTC), so it needs no CUDA language at build time -- it builds with an
# ordinary C++ toolchain and selects CPU/CUDA at run time. It also requires libpng (its own
# CMakeLists skips the example if libpng is absent).
IF(GENEVA_BUILD_WITH_GPU_CONSUMER)
	ADD_SUBDIRECTORY(15_GCUDAWorker)
ENDIF()

IF(GENEVA_BUILD_WITH_MPI_CONSUMER)
	ADD_SUBDIRECTORY( 16_GMPIConsumer )
	ADD_SUBDIRECTORY( 17_GMPISubClients )
ENDIF()

ADD_SUBDIRECTORY( 18_GAlgorithmChaining )

GENEVA_AGGREGATE_TARGET( "examples-geneva" )