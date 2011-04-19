/**
 * @file GDelayIndividual.hpp
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
#include <sstream>
#include <vector>

// Boost header files go here
#include <boost/shared_ptr.hpp>
#include <boost/serialization/nvp.hpp>
#include <boost/date_time.hpp>
#include <boost/date_time/posix_time/time_serialize.hpp>
#include <boost/date_time/gregorian/greg_serialize.hpp>
#include <boost/thread.hpp>
#include <boost/tokenizer.hpp>
#include <boost/lexical_cast.hpp>

#ifndef GDELAYINDIVIDUAL_HPP_
#define GDELAYINDIVIDUAL_HPP_

// For Microsoft-compatible compilers
#if defined(_MSC_VER)  &&  (_MSC_VER >= 1020)
#pragma once
#endif

// Geneva header files go here
#include "geneva/GDoubleCollection.hpp"
#include "geneva/GDoubleObject.hpp"
#include "geneva/GDoubleObjectCollection.hpp"
#include "geneva/GParameterSet.hpp"
#include "geneva/GDoubleGaussAdaptor.hpp"
#include "geneva-individuals/GIndividualFactoryT.hpp"

namespace Gem
{
namespace Tests
{

/************************************************************************************************/
/**
 * This individual waits for a predefined amount time before returning the result of the evaluation
 * (which is always the same). Its purpose is to measure the overhead of the parallelization, compared
 * to the serial execution.
 */
class GDelayIndividual: public Gem::Geneva::GParameterSet
{
	///////////////////////////////////////////////////////////////////////
	friend class boost::serialization::access;

    template<class Archive>
    void serialize(Archive & ar, const unsigned int version)
    {
		using boost::serialization::make_nvp;

		ar & BOOST_SERIALIZATION_BASE_OBJECT_NVP(Gem::Geneva::GParameterSet)
		   & BOOST_SERIALIZATION_NVP(sleepTime_);
    }
	///////////////////////////////////////////////////////////////////////

public:
    /** @brief Initialization with the amount of time the fitness evaluation should sleep before continuing */
    GDelayIndividual(const boost::posix_time::time_duration&);
	/** @brief A standard copy constructor */
	GDelayIndividual(const GDelayIndividual&);
	/** @brief The standard destructor */
	virtual ~GDelayIndividual();

	/** @brief A standard assignment operator */
	const GDelayIndividual& operator=(const GDelayIndividual&);

	/** @brief Checks for equality with another GDelayIndividual object */
	bool operator==(const GDelayIndividual&) const;
	/** @brief Checks for inequality with another GDelayIndividual object */
	bool operator!=(const GDelayIndividual&) const;

	/** @brief Checks whether a given expectation for the relationship between this object and another object is fulfilled */
	virtual boost::optional<std::string> checkRelationshipWith(
			const Gem::Geneva::GObject&,
			const Gem::Common::expectation&,
			const double&,
			const std::string&,
			const std::string&,
			const bool&) const;

	/** @brief Manual setting of the sleepTime_ variable */
	void setSleepTime(const boost::posix_time::time_duration&);
	/** @brief Retrieval of the current value of the sleepTime_ variable */
	boost::posix_time::time_duration getSleepTime() const;

protected:
	/** @brief Loads the data of another GDelayIndividual, camouflaged as a GObject */
	virtual void load_(const GObject*);
	/** @brief Creates a deep clone of this object */
	virtual Gem::Geneva::GObject* clone_() const;

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
//////////////////////////////////////////////////////////////////////////////////////////////////
/************************************************************************************************/
/**
 * A factory for GDelayIndividual objects
 */
class GDelayIndividualFactory :public Gem::Geneva::GIndividualFactoryT<GDelayIndividual>
{
public:
	/** @brief The standard constructor for this class */
	GDelayIndividualFactory(const std::string&);
	/** @brief The destructor */
	virtual ~GDelayIndividualFactory();

	/** @brief Allows to retrieve the name of the result file */
	std::string getResultFileName() const;
	/** @brief Allows to retrieve the name of the file holding the short measurement results */
	std::string getShortResultFileName() const;
	/** @brief Allows to retrieve the number of delays requested by the user */
	std::size_t getNDelays() const;
	/** @brief Allows to retrieve the amount of seconds in between two measurements */
	boost::uint32_t getInterMeasurementDelay() const;
	/** @brief Allows to retrieve the number of measurements to be made for each delay */
	boost::uint32_t getNMeasurements() const;

protected:
	/** @brief Necessary initialization work */
	virtual void init_();
	/** @brief Allows to describe configuration options in derived classes */
	virtual void describeConfigurationOptions_();
	/** @brief Creates individuals of the desired type */
	virtual boost::shared_ptr<GDelayIndividual> getIndividual_(const std::size_t&);

private:
	boost::uint32_t processingCycles_;
	std::size_t nVariables_;
	std::string delays_;
	std::vector<long> sleepSeconds_, sleepMilliSeconds_;
	std::string resultFile_;
	std::string shortResultFile_;
	boost::uint32_t interMeasurementDelay_; ///< A delay in between two measurements
	boost::uint32_t nMeasurements_; ///< The number of measurements for each delay
};

/************************************************************************************************/
} /* namespace Tests */
} /* namespace Gem */

BOOST_CLASS_EXPORT_KEY(Gem::Tests::GDelayIndividual)

#endif /* GDELAYINDIVIDUAL_HPP_ */
