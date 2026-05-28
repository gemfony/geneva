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

IF(CMAKE_CUDA_COMPILER_LOADED)
    SET(_CUDA_BENCHMARK_DEPS GCUDAOptBenchmarkTest GCUDAOptBenchmark)
ELSE()
    SET(_CUDA_BENCHMARK_DEPS)
ENDIF()

ADD_SUBDIRECTORY ( GBrokerOverhead )
ADD_SUBDIRECTORY ( GBrokerSanityChecks )
ADD_SUBDIRECTORY ( GOptimizationBenchmark )
ADD_SUBDIRECTORY ( GParallelisationOverhead )
ADD_SUBDIRECTORY ( GSerializationOverhead )

IF(CMAKE_CUDA_COMPILER_LOADED)
    ADD_SUBDIRECTORY ( GCUDAOptBenchmark )
ENDIF()

GENEVA_AGGREGATE_TARGET( "benchmarks-geneva" )
