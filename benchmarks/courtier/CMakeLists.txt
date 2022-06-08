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

ADD_CUSTOM_TARGET( "benchmarks-courtier"
	# Add all the available test targets (EXECUTABLENAMEs) here
	DEPENDS GBufferPortTTest # GConsumerPerformance
	COMMENT "Building the benchmarks for the Courtier library."
)

ADD_SUBDIRECTORY (GBufferPortTTest )
# ADD_SUBDIRECTORY (GConsumerPerformance)

IF(GENEVA_BUILD_WITH_MPI_CONSUMER)
	ADD_SUBDIRECTORY(GNetworkedConsumerBenchmark)
ENDIF()