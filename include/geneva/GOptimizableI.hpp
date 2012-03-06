/**
 * @file GOptimizableI.hpp
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
#include <vector>

// Boost header files go here
#include <boost/cstdint.hpp>

#ifndef GOPTIMIZATIONALGORITHMI_HPP_
#define GOPTIMIZATIONALGORITHMI_HPP_

// For Microsoft-compatible compilers
#if defined(_MSC_VER)  &&  (_MSC_VER >= 1020)
#pragma once
#endif

// Geneva headers go here
#include "geneva/GIndividual.hpp"

namespace Gem {
namespace Geneva {

/**************************************************************************************/
/**
 * This class specifies the interface that needs to be implemented by optimization
 * algorithms.
 */
class GOptimizableI
{
	///////////////////////////////////////////////////////////////////////
	friend class boost::serialization::access;

	template<typename Archive>
	void serialize(Archive &, const unsigned int){
	  using boost::serialization::make_nvp;
	  /* nothing */
	}
	///////////////////////////////////////////////////////////////////////

public:
	/** @brief The default constructor */
	GOptimizableI();
	/** @brief The destructor */
	virtual ~GOptimizableI();

	/** @brief Perform the actual optimization cycle, starting to count iterations at a given offset */
	virtual void optimize(const boost::uint32_t& offset) = 0;

	/** @brief A simple wrapper function that forces the class to start with offset 0 */
	virtual void optimize();

	/** @brief Retrieves the current iteration of this object */
	virtual boost::uint32_t getIteration() const = 0;

	/**************************************************************************************/
	/**
	 * Starts the optimization cycle and returns the best individual found, converted to
	 * the desired target type.
	 *
	 * @return The best individual found during the optimization process, converted to the desired type
	 */
	template <typename individual_type>
	boost::shared_ptr<individual_type> optimize() {
		this->optimize(0);
		return this->getBestIndividual<individual_type>();
	}

	/**************************************************************************************/
	/**
	 * Starts the optimization cycle and returns the best individual found, converted to
	 * the desired target type. This function uses a configurable offset for the iteration
	 * counter
	 *
	 * @param offset An offset for the iteration counter
	 * @return The best individual found during the optimization process, converted to the desired type
	 */
	template <typename individual_type>
	boost::shared_ptr<individual_type> optimize(
			const boost::uint32_t& offset
	) {
		this->optimize(offset);
		return this->getBestIndividual<individual_type>();
	}

	/**********************************************************************************/
	/**
	 * Retrieves the best individual and converts it to a given target type. Note that
	 * this function will not allow you to modify the best individual itself as it will
	 * return a copy to you.
	 *
	 * @return A copy of the best individual found in the optimization run
	 */
	template <typename individual_type>
	boost::shared_ptr<individual_type> getBestIndividual(
		typename boost::enable_if<boost::is_base_of<GIndividual, individual_type> >::type* dummy = 0
	) {
		return getBestIndividual()->clone<individual_type>();
	}

	/**********************************************************************************/
	/**
	 * Retrieves a list of the best individuals and converts them to a given target type.
	 * Note that this function will not allow you to modify the best individuals themselves
	 * as it will return a copies to you.
	 *
	 * @return A list of copies of the best individuals found in the optimization run
	 */
	template <typename individual_type>
	std::vector<boost::shared_ptr<individual_type> > getBestIndividuals(
		typename boost::enable_if<boost::is_base_of<GIndividual, individual_type> >::type* dummy = 0
	) {
		std::vector<boost::shared_ptr<individual_type> > bestIndividuals;

		std::vector<boost::shared_ptr<GIndividual> >::iterator it;
		std::vector<boost::shared_ptr<GIndividual> > bestBaseIndividuals = this->getBestIndividuals();

		// Cross check that we indeed got a valid set of individuals
		if(bestBaseIndividuals.empty()) {
			raiseException(
				"In GOptimizableI::getBestIndividuals(): Error!" << std::endl
				<< "Received empty collection of best individuals." << std::endl
			);
		}

		for(it=bestBaseIndividuals.begin(); it!=bestBaseIndividuals.end(); ++it) {
			bestIndividuals.push_back((*it)->clone<individual_type>());
		}

		return bestIndividuals;
	}

	/** @brief Returns information about the type of optimization algorithm. */
	virtual personality_oa getOptimizationAlgorithm() const;

	/** @brief Returns a name assigned to this algorithm */
	virtual std::string getAlgorithmName() const = 0;
	/** @brief Checks whether a given algorithm type likes to communicate via the broker */
	virtual bool usesBroker() const;

protected:
	/**********************************************************************************/
	/** @brief Retrieves the best individual found */
	virtual boost::shared_ptr<GIndividual> getBestIndividual() = 0;
	/** @brief Retrieves a list of the best individuals found */
	virtual std::vector<boost::shared_ptr<GIndividual> > getBestIndividuals() = 0;
};

/**************************************************************************************/

} /* namespace Geneva */
} /* namespace Gem */

/**************************************************************************************************/
/**
 * @brief Needed for Boost.Serialization
 */
BOOST_SERIALIZATION_ASSUME_ABSTRACT(Gem::Geneva::GOptimizableI)

#endif /* GOPTIMIZATIONALGORITHMI_HPP_ */
