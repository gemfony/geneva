/**
 * @file GDelayIndividual.hpp
 */

/*
 * Copyright (C) Authors of the Geneva library collection and Karlsruhe
 * Institute of Technology (University of the State of Baden-Wuerttemberg
 * and National Laboratory of the Helmholtz Association).
 *
 * See the AUTHORS file in the top-level directory for a list of authors.
 *
 * Contact: info [at] gemfony (dot) com
 *
 * This file is part of the Geneva library collection
 *
 * Geneva is free software: you can redistribute it and/or modify
 * it under the terms of version 3 of the GNU Affero General Public License
 * as published by the Free Software Foundation.
 *
 * Geneva is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with the Geneva library.  If not, see <http://www.gnu.org/licenses/>.
 *
 * For further information on Gemfony scientific and Geneva, visit
 * http://www.gemfony.com .
 */


// Standard header files go here
#include <iostream>
#include <cmath>
#include <sstream>
#include <vector>

// Boost header files go here
#include <boost/shared_ptr.hpp>
#include <boost/serialization/nvp.hpp>
#include <boost/date_time.hpp>
#include <boost/date_time/posix_time/time_serialize.hpp>
#include <boost/date_time/gregorian/greg_serialize.hpp>
#include <boost/thread.hpp>

#ifndef GDELAYINDIVIDUAL_HPP_
#define GDELAYINDIVIDUAL_HPP_

// For Microsoft-compatible compilers
#if defined(_MSC_VER)  &&  (_MSC_VER >= 1020)
#pragma once
#endif

// Geneva header files go here
#include "geneva/GDoubleCollection.hpp"
#include "geneva/GParameterSet.hpp"
#include "geneva/GDoubleGaussAdaptor.hpp"

namespace Gem
{
namespace Tests
{

using namespace Gem::Geneva;

/************************************************************************************************/
/**
 * This individual waits for a predefined amount time before returning the result of the evaluation
 * (which is always the same). Its purpose is to measure the overhead of the parallelization, compared
 * to the serial execution.
 */
class GDelayIndividual: public GParameterSet
{
	///////////////////////////////////////////////////////////////////////
	friend class boost::serialization::access;

    template<class Archive>
    void serialize(Archive & ar, const unsigned int version)
    {
		using boost::serialization::make_nvp;

		ar & BOOST_SERIALIZATION_BASE_OBJECT_NVP(GParameterSet)
		   & BOOST_SERIALIZATION_NVP(sleepTime_);
    }
	///////////////////////////////////////////////////////////////////////

public:
    /** @brief Initialization with the amount of time the fitness evaluation should sleep before continuing */
    GDelayIndividual(const boost::posix_time::time_duration&);
	/** @brief A standard copy constructor */
	GDelayIndividual(const GDelayIndividual&);
	/** @brief The standard destructor */
	~GDelayIndividual();

	/** @brief A standard assignment operator */
	const GDelayIndividual& operator=(const GDelayIndividual&);

	/** @brief Checks for equality with another GDelayIndividual object */
	bool operator==(const GDelayIndividual&) const;
	/** @brief Checks for inequality with another GDelayIndividual object */
	bool operator!=(const GDelayIndividual&) const;

	/** @brief Checks whether a given expectation for the relationship between this object and another object is fulfilled */
	virtual boost::optional<std::string> checkRelationshipWith(
			const GObject&,
			const Gem::Common::expectation&,
			const double&,
			const std::string&,
			const std::string&,
			const bool&) const;

protected:
	/** @brief Loads the data of another GDelayIndividual, camouflaged as a GObject */
	virtual void load_(const GObject*);
	/** @brief Creates a deep clone of this object */
	virtual GObject* clone_() const;

	/** @brief The actual adaption operations */
	virtual void customAdaptions();
	/** @brief The actual fitness calculation takes place here */
	virtual double fitnessCalculation();

private:
	/** The default constructor. Intentionally private */
	GDelayIndividual();

	boost::posix_time::time_duration sleepTime_; ///< The amount of time the evaluation function should sleep before continuing
};

/************************************************************************************************/

} /* namespace Tests */
} /* namespace Gem */

#endif /* GDELAYINDIVIDUAL_HPP_ */
