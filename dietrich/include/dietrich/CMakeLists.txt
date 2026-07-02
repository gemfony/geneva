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

# The umbrella header re-exports the full plotting stack.
SET ( GENEVADIETRICHINCLUDES
	GPlotDesigner.hpp
)

# The plotting subsystem lives in focused headers under dietrich/plotting/;
# dietrich/GPlotDesigner.hpp remains as an umbrella that includes them.
SET ( GENEVADIETRICHPLOTTINGINCLUDES
	plotting/GPlotEnums.hpp
	plotting/GDecorators.hpp
	plotting/GPlotSpec.hpp
	plotting/GBasePlotter.hpp
	plotting/GPlots.hpp
	plotting/GPlotEmitter.hpp
	plotting/GDataLog.hpp
	plotting/GPlotDesigner.hpp
)

# This is a workaround for a CLion-problem -- see CPP270 in the JetBrains issue tracker
ADD_CUSTOM_TARGET(CLION_ALL_DIETRICH_HEADERS SOURCES
	${GENEVADIETRICHINCLUDES} ${GENEVADIETRICHPLOTTINGINCLUDES})

INSTALL ( FILES ${GENEVADIETRICHINCLUDES} DESTINATION ${INSTALL_PREFIX_INCLUDES}/dietrich )
INSTALL ( FILES ${GENEVADIETRICHPLOTTINGINCLUDES} DESTINATION ${INSTALL_PREFIX_INCLUDES}/dietrich/plotting )
