/**
 * @file GSwarmAlgorithmFactory.hpp
 */

/*
 * Copyright (C) Gemfony scientific UG (haftungsbeschraenkt)
 *
 * See the AUTHORS file in the top-level directory for a list of authors.
 *
 * Contact: contact [at] gemfony (dot) com
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
 * http://www.gemfony.com .
 */

// Standard header files go here
#include <string>

// Boost header files go here
#include <boost/date_time.hpp>
#include <boost/filesystem.hpp>

#ifndef GSWARMALGORITHMFACTORY_HPP_
#define GSWARMALGORITHMFACTORY_HPP_

// For Microsoft-compatible compilers
#if defined(_MSC_VER)  &&  (_MSC_VER >= 1020)
#pragma once
#endif

// Geneva headers go here
#include "courtier/GCourtierEnums.hpp"
#include "geneva/GOptimizationAlgorithmFactoryT.hpp"
#include "geneva/GBaseSwarm.hpp"
#include "geneva/GSerialSwarm.hpp"
#include "geneva/GMultiThreadedSwarm.hpp"
#include "geneva/GBrokerSwarm.hpp"

namespace Gem
{
namespace Geneva
{

/******************************************************************************/
/**
 * This class is a specialization of the GOptimizationAlgorithmFactoryT<> class
 * for swarm algorithms.
 */
class GSwarmAlgorithmFactory
	: public GOptimizationAlgorithmFactoryT<GBaseSwarm>
{
public:
	/** @brief The standard constructor */
	GSwarmAlgorithmFactory(const std::string&, const parMode&);
	/** @brief The destructor */
	virtual ~GSwarmAlgorithmFactory();

protected:
	/** @brief Creates individuals of this type */
	virtual boost::shared_ptr<GBaseSwarm> getObject_(Gem::Common::GParserBuilder&, const std::size_t&);
	/** @brief Allows to act on the configuration options received from the configuration file */
	virtual void postProcess_(boost::shared_ptr<GBaseSwarm>&);

private:
	/** @brief The default constructor. Intentionally private and undefined */
	GSwarmAlgorithmFactory();
};
/******************************************************************************/

} /* namespace Geneva */
} /* namespace Gem */

#endif /* GSWARMALGORITHMFACTORY_HPP_ */
