/**
 * @file GEvolutionaryAlgorithmFactory.hpp
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

#ifndef GEVOLUTIONARYALGORITHMFACTORY_HPP_
#define GEVOLUTIONARYALGORITHMFACTORY_HPP_

// For Microsoft-compatible compilers
#if defined(_MSC_VER)  &&  (_MSC_VER >= 1020)
#pragma once
#endif

// Geneva headers go here
#include "common/GFactoryT.hpp"
#include "courtier/GCourtierEnums.hpp"
#include "geneva/GBaseEA.hpp"
#include "geneva/GSerialEA.hpp"
#include "geneva/GMultiThreadedEA.hpp"
#include "geneva/GBrokerEA.hpp"

namespace Gem
{
namespace Geneva
{

/*******************************************************************************************/
// Default settings
const boost::uint16_t FACT_DEF_NEVALUATIONTHREADS=0;

/*******************************************************************************************/
/**
 * This class is a specialization of the GFactoryT<> class for evolutionary algorithms.
 */
class GEvolutionaryAlgorithmFactory
	: public Gem::Common::GFactoryT<GBaseEA>
{
public:
	/** @brief The standard constructor */
	GEvolutionaryAlgorithmFactory(const std::string&, const parMode&);
	/** @brief The destructor */
	virtual ~GEvolutionaryAlgorithmFactory();

protected:
	/** @brief Allows to describe configuration options */
	virtual void describeLocalOptions_(Gem::Common::GParserBuilder&);
	/** @brief Creates individuals of this type */
	virtual boost::shared_ptr<GBaseEA> getObject_(Gem::Common::GParserBuilder&, const std::size_t&);
	/** @brief Allows to act on the configuration options received from the configuration file */
	virtual void postProcess_(boost::shared_ptr<GBaseEA>&);

private:
	/** @brief The default constructor. Intentionally private and undefined */
	GEvolutionaryAlgorithmFactory();

	parMode pm_; ///< Holds information about the desired parallelization mode

	boost::uint16_t nEvaluationThreads_; ///< The number of threads used for evaluations in multithreaded execution

    boost::posix_time::time_duration firstTimeOut_; ///< Maximum time frame for first individual
	double minWaitFactor_; ///< The minimum allowed wait factor
	double maxWaitFactor_; ///< The maximum allowed wait factor
	bool doLogging_; ///< Specifies whether arrival times of individuals should be logged
	bool boundlessWait_; ///< Indicates whether the retrieveItem call should wait for an unlimited amount of time
	double waitFactorIncrement_; ///< The amount by which the waitFactor_ may be incremented or decremented
};

/*******************************************************************************************/

} /* namespace Geneva */
} /* namespace Gem */

#endif /* GEVOLUTIONARYALGORITHMFACTORY_HPP_ */
