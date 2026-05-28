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
# Orchestrator for the self-contained Geneva library subtree (the largest
# of the four; absorbed the former geneva-individuals library in 1.12.0).
#
# Layout (per the 2026-05-28 repo-layout reorganisation):
#   geneva/include/geneva/             public headers (install layout 1:1)
#     *.hpp                              top-level (Go2.hpp, GSigHupHandler.hpp,
#                                        GMultiConstraintT.hpp,
#                                        GPluggableOptimizationMonitors.hpp, ...)
#     par/*.hpp                          parameter objects, adaptors, traits
#     oa/*.hpp                           optimization algorithms (EA, SA,
#                                        Swarm, GD, CGD, NelderMead, PS)
#     individuals/*.hpp                  reusable problem definitions
#     Interface/*.hpp                    abstract interfaces
#   geneva/src/*.cpp                   implementation (flat, mirrors the
#                                        top-level header set); 73 sources
#   geneva/tests/                      UnitTests + ManualTests
#   geneva/examples/{01_..18_}/        all numbered examples
#   geneva/benchmarks/                 6 benchmark binaries
#
# Tests / examples / benchmarks gated on the top-level options. The
# "geneva-all" aggregate is picked up by gemfony-build-all at top level.
################################################################################

ADD_SUBDIRECTORY ( include/geneva )
ADD_SUBDIRECTORY ( src )

IF ( GENEVA_BUILD_TESTS )
	ADD_SUBDIRECTORY ( tests )
ENDIF ()

IF ( GENEVA_BUILD_EXAMPLES )
	ADD_SUBDIRECTORY ( examples )
ENDIF ()

IF ( GENEVA_BUILD_BENCHMARKS )
	ADD_SUBDIRECTORY ( benchmarks )
ENDIF ()

# Per-library Doxygen target (doc-geneva).
ADD_SUBDIRECTORY ( doc )

GENEVA_AGGREGATE_TARGET( "geneva-all" )
