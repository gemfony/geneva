/**
 * @file GEvolutionaryAlgorithm.hpp
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

// Boost header files go here

#ifndef GEVOLUTIONARYALGORITHM_HPP_
#define GEVOLUTIONARYALGORITHM_HPP_

// For Microsoft-compatible compilers
#if defined(_MSC_VER)  &&  (_MSC_VER >= 1020)
#pragma once
#endif

// Geneva headers go here
#include "common/GExceptions.hpp"
#include "hap/GRandomT.hpp"
#include "geneva/GMutableSetT.hpp"
#include "geneva/GOptimizationEnums.hpp"
#include "geneva/GParameterSet.hpp"
#include "geneva/GSerialEA.hpp"
#include "geneva/GMultiThreadedEA.hpp"
#include "geneva/GBrokerEA.hpp"
#include "geneva/GenevaHelperFunctionsT.hpp"
#include "geneva/GOptimizableI.hpp"

namespace Gem {
namespace Geneva {

/**************************************************************************************/
/**
 * This class acts as a wrapper arount the different modes of evolutionary algorithms
 * in Geneva (serial, multi-threaded or broker-based)
 */
class GEvolutionaryAlgorithm
	: public GMutableSetT<GIndividual>
	, public GOptimizableI
{
	///////////////////////////////////////////////////////////////////////
	friend class boost::serialization::access;

	template<typename Archive>
	void serialize(Archive & ar, const unsigned int){
	  using boost::serialization::make_nvp;

	  ar & make_nvp("GMutableSetT_GParameterSet", boost::serialization::base_object<GMutableSetT<GIndividual> >(*this));
	}
	///////////////////////////////////////////////////////////////////////

public:
	/** @brief The default constructor */
	GEvolutionaryAlgorithm();
	/** @brief A copy constructor */
	GEvolutionaryAlgorithm(const GEvolutionaryAlgorithm&);
	/** @brief The destructor */
	virtual ~GEvolutionaryAlgorithm();

	/** @brief Standard assignment operator */
	const GEvolutionaryAlgorithm& operator=(const GEvolutionaryAlgorithm&);

	/** @brief Checks for equality with another Go object */
	bool operator==(const GEvolutionaryAlgorithm&) const;
	/** @brief Checks for inequality with another Go object */
	bool operator!=(const GEvolutionaryAlgorithm&) const;

	/** @brief Checks whether this object fulfills a given expectation in relation to another object */
	virtual boost::optional<std::string> checkRelationshipWith(
			const GObject&
			, const Gem::Common::expectation&
			, const double&
			, const std::string&
			, const std::string&
			, const bool&
	) const;

	/** @brief Perform the actual optimization cycle, starting to count iterations at a given offset */
	virtual void optimize(const boost::uint32_t& offset);

	/** @brief Allows to specify an optimization monitor to be used with evolutionary algorithms */
	void registerOptimizationMonitor(boost::shared_ptr<GSerialEA::GEAOptimizationMonitor>);

protected:
	/** @brief Retrieves the best individual found */
	virtual boost::shared_ptr<GIndividual> getBestIndividual();

private:
};

/**************************************************************************************/

} /* namespace Geneva */
} /* namespace Gem */

BOOST_CLASS_EXPORT_KEY(Gem::Geneva::GEvolutionaryAlgorithm)

#endif /* GEVOLUTIONARYALGORITHM_HPP_ */
