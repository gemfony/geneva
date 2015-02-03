/**
 * @file GOAFactoryStore.hpp
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

// For Microsoft-compatible compilers
#include "common/GWindowsDefines.hpp"

// Standard header files go here

// Boost header files go here
#include <boost/shared_ptr.hpp>

#ifndef GOAFACTORYSTORE_HPP_
#define GOAFACTORYSTORE_HPP_

// Geneva headers go here
#include "common/GGlobalOptionsT.hpp"
#include "geneva/GOptimizationAlgorithmFactoryT.hpp"
#include "geneva/GParameterSet.hpp"

// A global store for optimization algorithm factories
typedef Gem::Geneva::GOptimizationAlgorithmFactoryT<Gem::Geneva::GOptimizationAlgorithmT<Gem::Geneva::GParameterSet> > goa_factory;
typedef Gem::Common::GSingletonT<Gem::Common::GGlobalOptionsT<boost::shared_ptr<goa_factory> > > GOAStore;
#define GOAFactoryStore GOAStore::Instance(0)

#endif /* GOAFACTORYSTORE_HPP_ */
