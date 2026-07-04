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
		GAlgorithmChaining
		GGenericOptimizer)

IF(GENEVA_BUILD_WITH_MPI_CONSUMER)
	SET(DEPENDENCIES
			${DEPENDENCIES}
			GMPIConsumer
			GMPISubClients)
ENDIF()

IF(GENEVA_GPU_CONSUMER_ENABLED)
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
IF(GENEVA_GPU_CONSUMER_ENABLED)
	ADD_SUBDIRECTORY(15_GCUDAWorker)
ENDIF()

IF(GENEVA_BUILD_WITH_MPI_CONSUMER)
	ADD_SUBDIRECTORY( 16_GMPIConsumer )
	ADD_SUBDIRECTORY( 17_GMPISubClients )
ENDIF()

ADD_SUBDIRECTORY( 18_GAlgorithmChaining )
ADD_SUBDIRECTORY( 19_GLoadableIndividual )

# ---------------------------------------------------------------------------------------------------
# Optional: refresh every shipped example config in place via the Go2 --update-configs switch.
#
# `make geneva_update_configs` runs each eligible example binary with --update-configs, which rewrites
# the configs in that binary's BUILD directory in canonical form (drop keys no registered parameter
# consumes, keep existing values, add newly-registered keys with their defaults). It touches ONLY the
# build tree; copying the refreshed configs back into the source tree is a deliberate, separate step
# (Development Invariant 17 -- a build never writes into the checkout).
#
# Only standard Go2 examples that (a) construct Go2 and reach optimize() and (b) build their individual
# without an external tool/service or a device are eligible. Deliberately NOT included, each for a
# STRUCTURAL reason (verified, not a convenience drop):
#   03_GParameterObjectUsagePatterns -- a parameter-object demo; it never constructs Go2, so the switch
#                                       never engages (its lone config is unaffected either way);
#   06_GDirect{EA,SA,Swarm}          -- drive an optimization algorithm directly, without Go2;
#   10_GStarter                      -- not built in-tree (standalone "consumes an installed Geneva");
#   12_GExternalEvaluator            -- building its individual EXECUTES an external evaluator program;
#   15_GCUDAWorker                   -- needs a CUDA device;
#   16_GMPIConsumer / 17_GMPISubClients -- need mpirun;
#   19_GLoadableIndividual           -- a plugin loader (needs --individual <plugin.so>).
# Those are refreshed by hand (or via their own run) when needed.
OPTION(GENEVA_UPDATE_CONFIGS
	"Add a 'geneva_update_configs' target that refreshes eligible example configs in place (build tree only) via Go2 --update-configs"
	OFF)

IF(GENEVA_UPDATE_CONFIGS)
	SET(GENEVA_UPDATE_CONFIG_TARGETS
			GSimpleOptimizer               # 01
			GParaboloid2D                  # 02
			GMultiCriterionParabola        # 04
			GFunctionMinimizer             # 07
			GPostProcessing                # 08
			GNeuralNetwork                 # 09
			GMetaOptimizer                 # 11
			GPluggableOptimizationMonitors # 13
			GDependentConstraints          # 14
			GAlgorithmChaining)            # 18
	ADD_CUSTOM_TARGET(geneva_update_configs
			COMMENT "Refreshing eligible example configs in place via --update-configs (build tree only)")
	FOREACH(_gpb_target IN LISTS GENEVA_UPDATE_CONFIG_TARGETS)
		IF(TARGET ${_gpb_target})
			# Run the built binary in its own build directory (where its config/ was copied), so it
			# rewrites ./config/*.json there.
			ADD_CUSTOM_COMMAND(TARGET geneva_update_configs POST_BUILD
					COMMAND ${_gpb_target} --update-configs
					WORKING_DIRECTORY $<TARGET_FILE_DIR:${_gpb_target}>
					COMMENT "  ${_gpb_target} --update-configs"
					VERBATIM)
			ADD_DEPENDENCIES(geneva_update_configs ${_gpb_target})
		ENDIF()
	ENDFOREACH()
ENDIF()

GENEVA_AGGREGATE_TARGET( "examples-geneva" )