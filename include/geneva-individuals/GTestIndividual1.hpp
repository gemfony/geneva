/**
 * @file GTestIndividual1.hpp
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
#include <cmath>
#include <cstdlib>
#include <sstream>
#include <vector>
#include <list>
#include <algorithm> // for std::sort
#include <utility> // For std::pair

// Boost header files go here
#include <boost/shared_ptr.hpp>
#include <boost/bind.hpp>
#include <boost/filesystem/operations.hpp>
#include <boost/algorithm/string/trim.hpp>
#include <boost/cast.hpp>

#ifndef GTESTINDIVIDUAL1_HPP_
#define GTESTINDIVIDUAL1_HPP_

// For Microsoft-compatible compilers
#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

// Geneva header files go here
#include "geneva/GParameterSet.hpp"
#include "geneva/GDoubleCollection.hpp"
#include "geneva/GDoubleObject.hpp"
#include "geneva/GDoubleGaussAdaptor.hpp"
#include "geneva/GConstrainedDoubleObjectCollection.hpp"
#include "geneva/GConstrainedDoubleObject.hpp"
#include "geneva/GOptimizationEnums.hpp"
#include "common/GExceptions.hpp"
#include "common/GGlobalOptionsT.hpp"
#include "common/GCommonEnums.hpp"

namespace Gem
{
namespace Tests
{
/************************************************************************************************/
/**
 * This individual serves as the basis for unit tests of the individual hierarchy. At the time
 * of writing, it was included in order to be able to set the individual's personality without
 * weakening data protection.
 */
class GTestIndividual1 :public Gem::Geneva::GParameterSet
{
	///////////////////////////////////////////////////////////////////////
	friend class boost::serialization::access;

	template<typename Archive>
	void serialize(Archive & ar, const unsigned int) {
		using boost::serialization::make_nvp;

		ar & BOOST_SERIALIZATION_BASE_OBJECT_NVP(GParameterSet)
		   & BOOST_SERIALIZATION_NVP(fakeUpdateOnStall_);
	}
	///////////////////////////////////////////////////////////////////////

public:
	/** @brief The default constructor */
	GTestIndividual1();
	/** @brief The copy constructor */
	GTestIndividual1(const GTestIndividual1&);
	/** @brief The standard destructor */
	virtual ~GTestIndividual1();

	/** @brief A standard assignment operator */
	const GTestIndividual1& operator=(const GTestIndividual1&);

	/** @brief Checks for equality with another GTestIndividual1 object */
	bool operator==(const GTestIndividual1& cp) const;
	/** @brief Checks for inequality with another GTestIndividual1 object */
	bool operator!=(const GTestIndividual1& cp) const;

	/** @brief Checks whether a given expectation for the relationship between this object and another object is fulfilled */
	virtual boost::optional<std::string> checkRelationshipWith(
			const GObject&,
			const Gem::Common::expectation&,
			const double&,
			const std::string&,
			const std::string&,
			const bool&
	) const;

	/** @brief Sets the fakeUpdateOnStall_ variable */
	void setFakeCustomUpdateOnStall(const bool&);
	/** @brief Retrieves the current value of the fakeUpdateOnStall_ flag */
	bool getFakeCustomUpdateOnStall() const;
	/** @brief An overload of GIndividual::customUpdateOnStall() that can fake updates */
	bool customUpdateOnStall();

protected:
	/** @brief Loads the data of another GTestIndividual1 */
	virtual void load_(const GObject*);
	/** @brief Creates a deep clone of this object */
	virtual GObject* clone_() const;

	/** @brief The actual fitness calculation takes place here. */
	virtual double fitnessCalculation();

private:
	bool fakeUpdateOnStall_;

#ifdef GENEVATESTING
public:
	// Note: The following functions are, in the context of GTestIndividual1,
	// designed to mainly test parent classes

	/** @brief Applies modifications to this object. */
	virtual bool modify_GUnitTests();
	/** @brief Adds a number of GDoubleObject objects to the individual */
	void addGDoubleObjects(const std::size_t&);
	/** @brief Performs self tests that are expected to succeed. */
	virtual void specificTestsNoFailureExpected_GUnitTests();
	/** @brief Performs self tests that are expected to fail. */
	virtual void specificTestsFailuresExpected_GUnitTests();

#endif /* GENEVATESTING */
};

} /* namespace Tests */
} /* namespace Gem */

BOOST_CLASS_EXPORT_KEY(Gem::Tests::GTestIndividual1)

#endif /* GTESTINDIVIDUAL1_HPP_ */
