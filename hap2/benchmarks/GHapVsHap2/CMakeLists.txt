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


################################################################################
#
# This file is part of the Geneva library collection. Apache License 2.0.
# See the NOTICE file in the top-level directory for contributors/copyright.
#
################################################################################
#
# GHapVsHap2 — side-by-side benchmark linking BOTH gemfony-hap and
# gemfony-hap2 into one binary.
#
################################################################################

FIND_PACKAGE ( Threads REQUIRED )

ADD_EXECUTABLE ( GHapVsHap2 GHapVsHap2.cpp )

SET_TARGET_PROPERTIES(
	GHapVsHap2
	PROPERTIES
	CXX_STANDARD 20
	CXX_STANDARD_REQUIRED ON
	CXX_EXTENSIONS OFF
)

# Links both RNG libraries for the direct comparison. Include dirs for
# <hap/...> and <hap2/...> are already on the global INCLUDE_DIRECTORIES.
TARGET_LINK_LIBRARIES (
	GHapVsHap2
	PRIVATE
	${HAP_LIBNAME}
	${HAP2_LIBNAME}
	Threads::Threads
)
