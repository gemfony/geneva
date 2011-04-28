/**
 * @file GEvaluator.hpp
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

#ifndef GEVALUATOR_HPP_
#define GEVALUATOR_HPP_

// For Microsoft-compatible compilers
#if defined(_MSC_VER)  &&  (_MSC_VER >= 1020)
#pragma once
#endif

// Geneva headers go here
#include "common/GExceptions.hpp"
#include "geneva/GMutableSetT.hpp"
#include "geneva/GObject.hpp"

namespace Gem {
namespace Geneva {

class GParameterSet; ///< Forward declaration

/**************************************************************************************************/
/**
 * This class allows to implement evaluators for GParameterSet objects. Any number of evaluators is
 * possible. This feature is particularly useful in conjunction with multi-criterion optimization.
 * Note that the first registered evaluator plays a special role in optimization algorithms that
 * are not capable of dealing with multiple evaluation criteria.
 */
class GEvaluator :public GObject
{
	///////////////////////////////////////////////////////////////////////
	friend class boost::serialization::access;

	template<typename Archive>
	void serialize(Archive & ar, const unsigned int){
	  using boost::serialization::make_nvp;

	  ar & BOOST_SERIALIZATION_BASE_OBJECT_NVP(GObject)
	     & BOOST_SERIALIZATION_NVP(eval_);
	}
	///////////////////////////////////////////////////////////////////////

public:
	/** @brief The default constructor */
	GEvaluator();
	/** @brief The copy constructor */
	GEvaluator(const GEvaluator&);
	/** @brief The destructor */
	virtual ~GEvaluator();

	/** @brief Checks for equality with another GEvaluator object */
	bool operator==(const GEvaluator&) const;
	/** @brief Checks for inequality with another GEvaluator object */
	bool operator!=(const GEvaluator&) const;

	/** @brief Checks whether this object fulfills a given expectation in relation to another object */
	virtual boost::optional<std::string> checkRelationshipWith(
			const GObject&
			, const Gem::Common::expectation&
			, const double&, const std::string&
			, const std::string&
			, const bool&
	) const;

	/** @brief Triggers the fitness calculation, stores and returns the result */
	double fitness(GParameterSet * const);
	/** @brief Returns the cached (i.e. last known) result of the fitness calculation */
	double getCachedFitness() const;

protected:
	/** @brief Loads the data of another GEvaluator */
	virtual void load_(const GObject*);
	/** @brief Creates a deep clone of this object */
	virtual GObject* clone_() const;

	/** @brief The actual fitness calculation */
	virtual double fitnessCalculation(GParameterSet * const) const;

private:
	double eval_; ///< Holds the last known fitness value

#ifdef GENEVATESTING
public:
	/**********************************************************************************************/
	/** @brief Applies modifications to this object. This is needed for testing purposes */
	virtual bool modify_GUnitTests();
	/** @brief Performs self tests that are expected to succeed. This is needed for testing purposes */
	virtual void specificTestsNoFailureExpected_GUnitTests();
	/** @brief Performs self tests that are expected to fail. This is needed for testing purposes */
	virtual void specificTestsFailuresExpected_GUnitTests();

	/**********************************************************************************************/
#endif /* GENEVATESTING */
};

/**************************************************************************************************/
////////////////////////////////////////////////////////////////////////////////////////////////////
/**************************************************************************************************/

} /* namespace Geneva */
} /* namespace Gem */

/*************************************************************************************************/
/**
 * @brief Needed for Boost.Serialization
 */
BOOST_CLASS_EXPORT_KEY(Gem::Geneva::GEvaluator)

/*************************************************************************************************/

#endif /* GEVALUATOR_HPP_ */
