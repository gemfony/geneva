/**
 * @file GExternalSetterIndividual.hpp
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
#include <iostream>
#include <vector>

// Boost header files go here

#ifndef GEXTERNALSETTERINDIVIDUAL_HPP_
#define GEXTERNALSETTERINDIVIDUAL_HPP_

// For Microsoft-compatible compilers
#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

// Geneva header files go here
#include "geneva/GParameterSet.hpp"

#ifdef GEM_TESTING
#include "geneva/GDoubleObject.hpp"
#include "geneva/GDoubleGaussAdaptor.hpp"
#endif /* GEM_TESTING */

namespace Gem {
namespace Geneva {

/******************************************************************************/
/**
 * This is a very simple individual that allows external entities to set the fitness value,
 * thereby clearing the dirty flag. It can be used if the fitness calculation should take place
 * outside of the individual. The fitnessCalculation() function will throw by default. If you
 * want different behavior (e.g. in order to benchmark external against internal calculation),
 * you need to overload and re-implement the function in derived classes.
 */
class GExternalSetterIndividual
	:public Gem::Geneva::GParameterSet
{
	///////////////////////////////////////////////////////////////////////
	friend class boost::serialization::access;

	template<typename Archive>
	void serialize(Archive & ar, const unsigned int) {
		using boost::serialization::make_nvp;

		ar & BOOST_SERIALIZATION_BASE_OBJECT_NVP(GParameterSet);
	}
	///////////////////////////////////////////////////////////////////////

public:
	/** @brief The default constructor */
	GExternalSetterIndividual();
	/** @brief The copy constructor */
	GExternalSetterIndividual(const GExternalSetterIndividual&);
	/** @brief The standard destructor */
	virtual ~GExternalSetterIndividual();

	/** @brief A standard assignment operator */
	const GExternalSetterIndividual& operator=(const GExternalSetterIndividual&);

	/** @brief Checks for equality with another GExternalSetterIndividual object */
	bool operator==(const GExternalSetterIndividual& cp) const;
	/** @brief Checks for inequality with another GExternalSetterIndividual object */
	bool operator!=(const GExternalSetterIndividual& cp) const;

	/** @brief Adds local configuration options to a GParserBuilder object */
	virtual void addConfigurationOptions(Gem::Common::GParserBuilder&, const bool&);

	/** @brief Checks whether a given expectation for the relationship between this object and another object is fulfilled */
	virtual boost::optional<std::string> checkRelationshipWith(
			const GObject&,
			const Gem::Common::expectation&,
			const double&,
			const std::string&,
			const std::string&,
			const bool&
	) const;

	/** @brief Sets the fitness to a given set of values and clears the dirty flag */
	void setFitness(const double&, const std::vector<double>&);

protected:
	/** @brief Loads the data of another GExternalSetterIndividual */
	virtual void load_(const GObject*);
	/** @brief Creates a deep clone of this object */
	virtual GObject* clone_() const;

	/** @brief The actual fitness calculation takes place here. */
	virtual double fitnessCalculation();

	/***************************************************************************/
#ifdef GEM_TESTING
public:
	/** @brief Applies modifications to this object. */
	virtual bool modify_GUnitTests();
	/** @brief Performs self tests that are expected to succeed. */
	virtual void specificTestsNoFailureExpected_GUnitTests();
	/** @brief Performs self tests that are expected to fail. */
	virtual void specificTestsFailuresExpected_GUnitTests();

#endif /* GEM_TESTING */
}; // End of class declaration

/******************************************************************************/

} /* namespace Geneva */
} /* namespace Gem */

BOOST_CLASS_EXPORT_KEY(Gem::Geneva::GExternalSetterIndividual)

#endif /* GEXTERNALSETTERINDIVIDUAL_HPP_ */
