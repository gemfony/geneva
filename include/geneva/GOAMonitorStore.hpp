/**
 * @file GOAMonitorStore.hpp
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

// Standard header files go here

// Boost header files go here
#include <boost/shared_ptr.hpp>

#ifndef GOAMONITORSTORE_HPP_
#define GOAMONITORSTORE_HPP_

// For Microsoft-compatible compilers
#if defined(_MSC_VER)  &&  (_MSC_VER >= 1020)
#pragma once
#endif

// Geneva headers go here
#include "common/GGlobalOptionsT.hpp"
#include "geneva/GOptimizationAlgorithmT.hpp"
#include "geneva/GParameterSet.hpp"

// A global store for optimization algorithm monitors. The idea is that algorithms adding an
// algorithm with a nickname of e.g. "ea" can check in this store whether any specific monitors have
// been registered.
typedef Gem::Geneva::GOptimizationAlgorithmT<Gem::Geneva::GParameterSet>::GOptimizationMonitorT goam_factory;
typedef Gem::Common::GSingletonT<Gem::Common::GGlobalOptionsT<boost::shared_ptr<goam_factory> > > GOAMStore;
#define GOAMonitorStore GOAMStore::Instance(0)

#endif /* GOAMONITORSTORE_HPP_ */
