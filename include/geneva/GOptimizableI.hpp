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

// Boost header files go here


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
	GOptimizableI() { /* nothing */ }
	/** @brief The destructor */
	virtual ~GOptimizableI() { /* nothing */ }

	/** @brief Perform the actual optimization cycle, starting to count iterations at a given offset */
	virtual void optimize(const boost::uint32_t& offset) = 0;

    /**********************************************************************************/
	/**
	 * This is a simple wrapper function that forces the class to start with offset 0
	 */
	virtual void optimize() {
		optimize(0);
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

protected:
	/**********************************************************************************/
	/** @brief Retrieves the best individual found */
	virtual boost::shared_ptr<GIndividual> getBestIndividual() = 0;
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
