/**
 * @file GPluggableOptimizationMonitors.cpp
 */

/*
 * Copyright (C) Gemfony scientific UG (haftungsbeschraenkt)
 *
 * See the AUTHORS file in the top-level directory for a list of authors.
 *
 * Contact: contact [at] gemfony (dot) eu
 *
 * This file is part of the Geneva library collection.
 *
 * Geneva was developed with kind support from Karlsruhe Institute of
 * Technology (KIT) and Steinbuch Centre for Computing (SCC). Further
 * information about KIT and SCC can be found at http://www.kit.edu/english
 * and http://scc.kit.edu .
 *
 * Geneva is free software: you can redistribute and/or modify it under
 * the terms of version 3 of the GNU Affero General Public License
 * as published by the Free Software Foundation.
 *
 * Geneva is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with the Geneva library. If not, see <http://www.gnu.org/licenses/>.
 *
 * For further information on Gemfony scientific and Geneva, visit
 * http://www.gemfony.eu .
 */

#include "geneva/GPluggableOptimizationMonitors.hpp"

/******************************************************************************/
// Exports of classes

BOOST_CLASS_EXPORT_IMPLEMENT(Gem::Geneva::GStandardMonitor)
BOOST_CLASS_EXPORT_IMPLEMENT(Gem::Geneva::GFitnessMonitor)
BOOST_CLASS_EXPORT_IMPLEMENT(Gem::Geneva::GCollectiveMonitor)
BOOST_CLASS_EXPORT_IMPLEMENT(Gem::Geneva::GProgressPlotter)
BOOST_CLASS_EXPORT_IMPLEMENT(Gem::Geneva::GAllSolutionFileLogger)
BOOST_CLASS_EXPORT_IMPLEMENT(Gem::Geneva::GIterationResultsFileLogger)
BOOST_CLASS_EXPORT_IMPLEMENT(Gem::Geneva::GNAdpationsLogger)
BOOST_CLASS_EXPORT_IMPLEMENT(Gem::Geneva::GAdaptorPropertyLogger<double>)
BOOST_CLASS_EXPORT_IMPLEMENT(Gem::Geneva::GAdaptorPropertyLogger<std::int32_t>)
BOOST_CLASS_EXPORT_IMPLEMENT(Gem::Geneva::GAdaptorPropertyLogger<bool>)
BOOST_CLASS_EXPORT_IMPLEMENT(Gem::Geneva::GProcessingTimesLogger)
