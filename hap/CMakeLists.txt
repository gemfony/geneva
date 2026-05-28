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
# Orchestrator for the self-contained Hap library subtree.
#
# Layout (per the 2026-05-28 repo-layout reorganisation):
#   hap/include/hap/*.hpp   public headers (install layout 1:1)
#   hap/src/*.cpp *.cu      implementation + the gemfony-hap target
#                           (CUDA random-number source under USECUDARNG)
#   hap/tests/              UnitTests + ManualTests + DocuPrograms
#   hap/examples/           05_GHapUsagePatterns
#   (no benchmarks subtree for hap)
#
# Tests / examples are gated on the top-level options so a minimal build
# only descends into include and src. The "hap-all" aggregate is picked up
# by gemfony-build-all at the top level.
################################################################################

ADD_SUBDIRECTORY ( include/hap )
ADD_SUBDIRECTORY ( src )

IF ( GENEVA_BUILD_TESTS )
	ADD_SUBDIRECTORY ( tests )
ENDIF ()

IF ( GENEVA_BUILD_EXAMPLES )
	ADD_SUBDIRECTORY ( examples )
ENDIF ()

GENEVA_AGGREGATE_TARGET( "hap-all" )
