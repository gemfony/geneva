/**
 * @file GMultiThreadedGD.hpp
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

// Standard headers go here
#include <string>
#include <fstream>
#include <sstream>
#include <cmath>
#include <cfloat>
#include <vector>
#include <algorithm>

// Includes check for correct Boost version(s)
#include "common/GGlobalDefines.hpp"

// Boost headers go here

#include <boost/cstdint.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/bind.hpp>
#include <boost/function.hpp>
#include <boost/cast.hpp>
#include <boost/serialization/nvp.hpp>
#include <boost/date_time.hpp>
#include <boost/date_time/gregorian/greg_serialize.hpp>
#include <boost/date_time/posix_time/time_serialize.hpp>
#include <boost/cast.hpp>
#include <boost/filesystem.hpp>
#include <boost/utility/enable_if.hpp>
#include <boost/type_traits.hpp>
#include <boost/thread/thread.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/thread/condition.hpp>
#include <boost/thread/xtime.hpp>
#include <common/thirdparty/boost/threadpool.hpp>
/**
 * Check that we have support for threads
 */
#ifndef BOOST_HAS_THREADS
#error "Error: Support for multi-threading does not seem to be available."
#endif

#ifndef GMULTITHREADEDGD_HPP_
#define GMULTITHREADEDGD_HPP_

// For Microsoft-compatible compilers
#if defined(_MSC_VER)  &&  (_MSC_VER >= 1020)
#pragma once
#endif


// Geneva headers go here
#include "common/GExceptions.hpp"
#include "geneva/GGradientDescent.hpp"
#include "common/GHelperFunctions.hpp"
#include "common/GThreadWrapper.hpp"
#include "geneva/GIndividual.hpp"
#include "geneva/GObject.hpp"

#ifdef GENEVATESTING
#include "GTestIndividual1.hpp"
#endif /* GENEVATESTING */

namespace Gem {
namespace Geneva {

/** @brief The default number of threads for parallelization with boost */
const boost::uint16_t DEFAULTBOOSTTHREADSGD = 2;

/*********************************************************************************/
/**
 * A multi-threaded version of the GMultiThreadedGD class
 */
class GMultiThreadedGD
	:public GGradientDescent
{
	///////////////////////////////////////////////////////////////////////
	friend class boost::serialization::access;

	template<typename Archive>
	void serialize(Archive & ar, const unsigned int) {
		using boost::serialization::make_nvp;

		ar & BOOST_SERIALIZATION_BASE_OBJECT_NVP(GGradientDescent)
		   & BOOST_SERIALIZATION_NVP(nThreads_);
	}

	///////////////////////////////////////////////////////////////////////

public:
	/** @brief The default constructor */
	GMultiThreadedGD();
	/** @brief Initialization with the number of starting points and the size of the finite step */
	GMultiThreadedGD(const std::size_t&, const float&, const float&);
	/** @brief A standard copy constructor */
	GMultiThreadedGD(const GMultiThreadedGD&);
	/** @brief The destructor */
	virtual ~GMultiThreadedGD();

	/** @brief A standard assignment operator */
	const GMultiThreadedGD& operator=(const GMultiThreadedGD&);

	/** @brief Checks for equality with another GMultiThreadedGD object */
	bool operator==(const GMultiThreadedGD&) const;
	/** @brief Checks for inequality with another GMultiThreadedGD object */
	bool operator!=(const GMultiThreadedGD&) const;

	/** @brief Checks whether this object fulfills a given expectation in relation to another object */
	virtual boost::optional<std::string> checkRelationshipWith(
			const GObject&
			, const Gem::Common::expectation&
			, const double&
			, const std::string&
			, const std::string&
			, const bool&
	) const;

	/** @brief Sets the maximum number of threads */
	void setNThreads(const boost::uint8_t&);
	/** @brief Retrieves the maximum number of threads */
	boost::uint8_t getNThreads() const ;

protected:
	/** @brief Loads the data of another population */
	virtual void load_(const GObject *);
	/** @brief Creates a deep clone of this object */
	virtual GObject *clone_() const;

	virtual void init();
	/** @brief Does any necessary finalization work */
	virtual void finalize();

	/** @brief Triggers fitness calculation of a number of individuals */
	virtual double doFitnessCalculation(const std::size_t&);

private:
	boost::uint8_t nThreads_; ///< The number of threads
	boost::threadpool::pool tp_; ///< A thread pool

	std::vector<bool> sm_value_; ///< Internal storage for server mode flags

#ifdef GENEVATESTING
public:
	/**************************************************************************************************/
	/** @brief Applies modifications to this object. This is needed for testing purposes */
	virtual bool modify_GUnitTests();
	/** @brief Performs self tests that are expected to succeed. This is needed for testing purposes */
	virtual void specificTestsNoFailureExpected_GUnitTests();
	/** @brief Performs self tests that are expected to fail. This is needed for testing purposes */
	virtual void specificTestsFailuresExpected_GUnitTests();
#endif /* GENEVATESTING */
};

} /* namespace Geneva */
} /* namespace Gem */

#ifdef GENEVATESTING
// Tests of this class (and parent classes)
/*************************************************************************************************/
///////////////////////////////////////////////////////////////////////////////////////////////////
/*************************************************************************************************/
/** @brief We need to provide a specialization of the factory function that creates objects of this type. */
template <> boost::shared_ptr<Gem::Geneva::GMultiThreadedGD> TFactory_GUnitTests<Gem::Geneva::GMultiThreadedGD>();

/*************************************************************************************************/
///////////////////////////////////////////////////////////////////////////////////////////////////
/*************************************************************************************************/
#endif /* GENEVATESTING */

#endif /* GMULTITHREADEDGD_HPP_ */
