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

# Small, self-contained demos of the courtier interfaces, each a single .cpp linking only the
# Geneva libraries. They double as the manual's source for the corresponding listings (snippet
# markers, see source/snippets.map in the manual repository), so each one is also registered as a
# test: a documentation example that stops compiling, or stops doing what the text says, has to
# turn a gate red rather than rot quietly.

SET ( _COURTIER_EXAMPLES
	minimal_consumer  # the consumer contract: implement dispatch_(), submit through processBatch()
)

FOREACH ( _ex ${_COURTIER_EXAMPLES} )
	ADD_EXECUTABLE ( courtier_${_ex} ${_ex}.cpp )
	# courtier's own dependency closure only (courtier <- hap <- common <- weft, in link order) --
	# an example of this library must not need the libraries that sit above it.
	TARGET_LINK_LIBRARIES ( courtier_${_ex}
		${COURTIER_LIBNAME} ${HAP_LIBNAME} ${COMMON_LIBNAME} ${WEFT_LIBNAME} ${Boost_LIBRARIES} )
	ADD_TEST ( NAME courtier_${_ex} COMMAND courtier_${_ex} )
	SET_TESTS_PROPERTIES ( courtier_${_ex} PROPERTIES LABELS "courtier;example" TIMEOUT 120 )
	GENEVA_TEST_SIZE ( courtier_${_ex} S )
	INSTALL ( TARGETS courtier_${_ex}
		DESTINATION ${INSTALL_PREFIX_DATA}/examples/courtier )
ENDFOREACH ()

GENEVA_AGGREGATE_TARGET( "examples-courtier" )
