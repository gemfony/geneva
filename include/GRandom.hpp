/**
 * @file GRandom.hpp
 */

/* Copyright (C) Dr. Ruediger Berlich and Karlsruhe Institute of Technology
 * (University of the State of Baden-Wuerttemberg and National Laboratory
 * of the Helmholtz Association)
 *
 * Contact: info [at] gemfony (dot) com
 *
 * This file is part of the Geneva library, Gemfony scientific's optimization
 * library.
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

#include <cstdlib>
#include <iomanip>
#include <ctime>
#include <cmath>
#include <iostream>
#include <sstream>
#include <cassert>

// Includes check for correct Boost version(s)
#include "GGlobalDefines.hpp"

// Boost headers go here

#include <boost/cstdint.hpp>
#include <boost/random.hpp>
#include <boost/date_time.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/thread/thread.hpp>
#include <boost/bind.hpp>
#include <boost/utility.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/shared_array.hpp>
#include <boost/pool/detail/singleton.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/exception.hpp>
#include <boost/cstdint.hpp>
#include <boost/cast.hpp>
#include <boost/function.hpp>
#include <boost/random/linear_congruential.hpp>

/**
 * Check that we have support for threads. This collection of classes is useless
 * without this.
 */
#ifndef BOOST_HAS_THREADS
#error "Error: No support for Boost.threads available."
#endif

#ifndef GRANDOM_HPP_
#define GRANDOM_HPP_

// For Microsoft-compatible compilers
#if defined(_MSC_VER)  &&  (_MSC_VER >= 1020)
#pragma once
#endif


// GenEvA headers go here

#include "GObject.hpp"
#include "GBoundedBufferT.hpp"
#include "GEnums.hpp"
#include "GThreadGroup.hpp"
#include "GRandomFactory.hpp"
#include "GenevaExceptions.hpp"

/****************************************************************************/

namespace Gem {
namespace Util {

// const double rnr_max = static_cast<double>(std::numeric_limits<boost::uint32_t>::max());
const double rnr_max = static_cast<double>(std::numeric_limits<boost::int32_t>::max()); // The return type of boost::rand48

/****************************************************************************/
/**
 * This class gives objects access to random numbers. It internally handles
 * retrieval of random numbers from the GRandomFactory class as needed, or
 * produces them locally. Random distributions are calculated on the fly from
 * these numbers. Usage is thus transparent to the user, when random numbers
 * are retrieved from the factory.
 */
class GRandom
	:public Gem::GenEvA::GObject
{
    ///////////////////////////////////////////////////////////////////////
    friend class boost::serialization::access;

    template<typename Archive>
    void save(Archive & ar, const unsigned int) const {
      using boost::serialization::make_nvp;
      ar & make_nvp("rnrGenerationMode_", rnrGenerationMode_);
      ar & make_nvp("initialSeed_", initialSeed_);
    }

    template<typename Archive>
    void load(Archive & ar, const unsigned int){
        using boost::serialization::make_nvp;
        ar & make_nvp("rnrGenerationMode_", rnrGenerationMode_);

        switch(rnrGenerationMode_) {
        case Gem::Util::RNRFACTORY:
        	// Make sure we have a local pointer to the factory
        	grf_ = GRANDOMFACTORY;
        	break;

        case Gem::Util::RNRLOCAL:
        	// Reset all other random number generation modes
        	p01_.reset();
        	grf_.reset();
        	break;
        };

        ar & make_nvp("initialSeed_", initialSeed_);

        // Make sure we use the correct seed
        linCongr_.seed(boost::numeric_cast<boost::uint64_t>(initialSeed_));
    }

    BOOST_SERIALIZATION_SPLIT_MEMBER()
    ///////////////////////////////////////////////////////////////////////

public:
	/** @brief The standard constructor */
	GRandom();
	/** @brief Initialization with the random number generation mode */
	explicit GRandom(const Gem::Util::rnrGenerationMode&);
	/** @brief A copy constructor */
	GRandom(const GRandom&);
	/** @brief A standard destructor */
	~GRandom();

	/** @brief A standard assignment operator */
	GRandom& operator=(const GRandom&);

	/** @brief Checks for equality with another GRandom object */
	bool operator==(const GRandom&) const;
	/** @brief Checks inequality with another GRandom object */
	bool operator!=(const GRandom&) const;
	/** @brief Checks for equality with another GRandom object, camouflaged as a GObject */
	virtual bool isEqualTo(const Gem::GenEvA::GObject&, const boost::logic::tribool& expected = boost::logic::indeterminate) const;
	/** @brief Checks for similarity with another GRandom object, camouflaged as a GObject */
	virtual bool isSimilarTo(const Gem::GenEvA::GObject&, const double&, const boost::logic::tribool& expected = boost::logic::indeterminate) const;

	/** @brief Creates a deep clone of this object */
	virtual Gem::GenEvA::GObject* clone() const;
	/** @brief Loads the data of another GRandom object, camouflaged as a GObject */
	virtual void load(const Gem::GenEvA::GObject*);

	/** @brief Emits evenly distributed random numbers in the range [0,1[ */
	double evenRandom();
	/** @brief Emits evenly distributed random numbers in the range [0,1[ */
	double evenRandomFromFactory();
	/** @brief Emits evenly distributed random numbers in the range [0,1[ */
	double evenRandomLocalProduction();
	/** @brief Emits evenly distributed random numbers in the range [0,max[ */
	double evenRandom(const double&);
	/** @brief Produces evenly distributed random numbers in the range [min,max[ */
	double evenRandom(const double&, const double&);

	/** @brief Produces gaussian-distributed random numbers */
	double gaussRandom(const double&, const double&);
	/** @brief Produces two gaussians with defined distance */
	double doubleGaussRandom(const double&, const double&, const double&);

	/** @brief Produces integer random numbers in the range of [0, max[  */
	template <typename int_type> int_type discreteRandom(const int_type&);
	/** @brief Produces integer random numbers in the range of [min, max[ . */
	template <typename int_type> int_type discreteRandom(const int_type&, const int_type&);

	/** @brief Produces bool values with a 50% likelihood each for true and false. */
	bool boolRandom();
	/** @brief Returns true with a given probability, otherwise false. */
	bool boolRandom(const double&);
	/** @brief Produces random ASCII characters */
	char charRandom(const bool& printable = true);

	/** @brief Sets the random number generation mode */
	void setRnrGenerationMode(const Gem::Util::rnrGenerationMode&);
	/** @brief Retrieves the current random number generation mode */
	Gem::Util::rnrGenerationMode getRnrGenerationMode () const;
	/** @brief Specifies a rng-proxy to be used and empties the p01_ array */
	void setRNRFactoryMode();
	/** @brief Switches to local production mode, using GRandomFactory::getStartSeed() for seeding */
	void setRNRLocalMode();
	/** @brief Switches to local production mode, using the supplied seed value */
	void setRNRLocalMode(const boost::uint32_t&);

	/** @brief Allows to store a user-defined seed for local random number generation */
	void setSeed(const boost::uint32_t&);
	/** @brief Retrieves the current seed value */
	boost::uint32_t getSeed();

private:
	/** @brief Fills a random container if none could be retrieved from the factory */
	void fillContainer01();
	/** @brief (Re-)Initialization of p01_ */
	void getNewP01();

	rnrGenerationMode rnrGenerationMode_; ///< The current random number generation mode. Size 4 Byte on a 64bit system

	// The following options are used when random numbers are taken from the factory
	std::size_t currentPackageSize_; ///< The package size, as obtained from the factory.  Size is 8 byte on a 64 bit system
	boost::shared_array<double> p01_; ///< Holds the container of [0,1[ random numbers  Size is 16 bytes on a 64 bit system
	std::size_t current01_; ///< The current position in p01_.  Size is 8 byte on a 64 bit system
	double *p_raw_; ///< A pointer to the content of p01_ for faster access.  Size is 8 byte on a 64 bit system
	boost::shared_ptr<Gem::Util::GRandomFactory> grf_; ///< A local copy of the global GRandomFactory.  Size is 16 byte on a 64 bit system (?)

	/** @brief Used in conjunction with the local generation of random numbers */
	boost::uint32_t initialSeed_; ///< Used as a start value for the local random number generator.  Size is 4 byte on a 64 bit system
	/** @brief Used as a fall-back when the factory could not return a package, or for local random number generation */
	boost::rand48 linCongr_;
};

/****************************************************************************/
/**
 * This function produces integer random numbers in the range of [0, max[ .
 *
 * @param max The maximum (excluded) value of the range
 * @return Discrete random numbers evenly distributed in the range [0,max[
 */
template <typename int_type>
int_type GRandom::discreteRandom(const int_type& max) {
#ifdef DEBUG
	int_type result = boost::numeric_cast<int_type> (this->evenRandom(boost::numeric_cast<double> (max)));
	assert(result<max);
	return result;
#else
	return static_cast<int_type>(this->evenRandom(static_cast<double>(max)));
#endif
}

/*************************************************************************/
/**
 * This function produces integer random numbers in the range of [min, max[ .
 * Note that max may also be < 0. .
 *
 * @param min The minimum value of the range
 * @param max The maximum (excluded) value of the range
 * @return Discrete random numbers evenly distributed in the range [min,max[
 */
template <typename int_type>
int_type GRandom::discreteRandom(const int_type& min, const int_type& max) {
#ifdef DEBUG
	assert(min < max);
	int_type result = boost::numeric_cast<int_type>(discreteRandom(max - min) + min);
	assert(result>=min && result<max);
	return result;
#else
	return discreteRandom(max - min) + min;
#endif
}

/*************************************************************************/

template <> double GRandom::discreteRandom(const double&); ///< Specialization for double
template <>	double GRandom::discreteRandom(const double&, const double&); ///< Specialization for double
template <> float GRandom::discreteRandom(const float&); ///< Specialization for float
template <>	float GRandom::discreteRandom(const float&, const float&); ///< Specialization for float
template <> bool GRandom::discreteRandom(const bool&); ///< Specialization for bool
template <>	bool GRandom::discreteRandom(const bool&, const bool&); ///< Specialization for bool

} /* namespace Util */
} /* namespace Gem */

#endif /* GRANDOM_HPP_ */
