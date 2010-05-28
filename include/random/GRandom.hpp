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
#include <boost/cstdint.hpp>
#include <boost/cast.hpp>
#include <boost/function.hpp>
#include <boost/random/linear_congruential.hpp>
#include <boost/utility/enable_if.hpp>
#include <boost/type_traits.hpp>
#include <boost/archive/xml_oarchive.hpp>
#include <boost/archive/xml_iarchive.hpp>
#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>
#include <boost/archive/binary_oarchive.hpp>
#include <boost/archive/binary_iarchive.hpp>
#include <boost/serialization/nvp.hpp>
#include <boost/serialization/vector.hpp>
#include <boost/serialization/shared_ptr.hpp>
#include <boost/serialization/base_object.hpp>
#include <boost/serialization/utility.hpp>
#include <boost/serialization/tracking.hpp>
#include <boost/serialization/split_member.hpp>
#include <boost/serialization/export.hpp>

#ifndef GRANDOM_HPP_
#define GRANDOM_HPP_

// For Microsoft-compatible compilers
#if defined(_MSC_VER)  &&  (_MSC_VER >= 1020)
#pragma once
#endif


// GenEvA headers go here

#include "GBoundedBufferT.hpp"
#include "GThreadGroup.hpp"
#include "GRandomFactory.hpp"
#include "GExceptions.hpp"

/****************************************************************************/

namespace Gem {
namespace Util {

/****************************************************************************/
/**
 * Random number generation can happen in two modes
 */
enum rnrGenerationMode {
	  RNRFACTORY
	, RNRLOCAL
};

/****************************************************************************/
/**
 * The default random number generation mode
 */
const rnrGenerationMode DEFAULTRNRGENMODE=RNRLOCAL;

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
{
    ///////////////////////////////////////////////////////////////////////
    friend class boost::serialization::access;

    template<typename Archive>
    void save(Archive & ar, const unsigned int) const {
      using boost::serialization::make_nvp;

      ar & BOOST_SERIALIZATION_NVP(rnrGenerationMode_)
         & BOOST_SERIALIZATION_NVP(initialSeed_)
         & BOOST_SERIALIZATION_NVP(gaussCache_)
         & BOOST_SERIALIZATION_NVP(gaussCacheAvailable_);
    }

    template<typename Archive>
    void load(Archive & ar, const unsigned int){
        using boost::serialization::make_nvp;

        ar & BOOST_SERIALIZATION_NVP(rnrGenerationMode_);

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

        ar & BOOST_SERIALIZATION_NVP(initialSeed_);

        // Make sure we use the correct seed
        linCongr_.seed(boost::numeric_cast<boost::uint64_t>(initialSeed_));

        ar & BOOST_SERIALIZATION_NVP(gaussCache_)
           & BOOST_SERIALIZATION_NVP(gaussCacheAvailable_);
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
	virtual ~GRandom();

	/** @brief A standard assignment operator */
	GRandom& operator=(const GRandom&);
	/** @brief Loads the data of another GRandom object */
	void load(const GRandom&);


	/** @brief Emits evenly distributed random numbers in the range [0,1[ */
	// inline double evenRandom();
	/*************************************************************************/
	/**
	 * This function emits evenly distributed random numbers in the range [0,1[ .
	 * These are either taken from the random number factory or are created
	 * locally.
	 *
	 * @return Random numbers evenly distributed in the range [0,1[
	 */
	inline double evenRandom() {
		switch(rnrGenerationMode_) {
		case RNRFACTORY:
			{
				// If the object has been newly created, p01_ will be empty
				if (!p01_ || (current01_ == currentPackageSize_+1)) {
					getNewP01();
					current01_ = 1; // Position 0 is the array size
				}

				return p_raw_[current01_++];
			}
			break;

		/* Produces random numbers locally, following the linear congruential method.
		 * See e.g. http://en.wikipedia.org/wiki/Linear_congruential_generator for further information. */
		case RNRLOCAL:
			return evenRandomLocalProduction();
			break;
		}; // no default necessary, as this switch is based on an enum

		return 0.; // make the compiler happy
	}
	/*************************************************************************/
	/**
	 * Produces even random numbers locally, using the linear congruential generator.
	 * See e.g. http://en.wikipedia.org/wiki/Linear_congruential_generator for further
	 * information.
	 *
	 * @return A random number evently distributed in [0,1[
	 */
	inline double evenRandomLocalProduction() {
#ifdef DEBUG
			double value =  boost::numeric_cast<double>(linCongr_())/rnr_max; // Will be slower ...
			assert(value>=0. && value<1.);
			return value;
#else
			return static_cast<double>(static_cast<double>(linCongr_()))/rnr_max;
#endif
	}

	/*************************************************************************/

	/** @brief Emits evenly distributed random numbers in the range [0,max[ */
	double evenRandom(const double&);
	/** @brief Produces evenly distributed random numbers in the range [min,max[ */
	double evenRandom(const double&, const double&);

	/** @brief Produces gaussian-distributed random numbers */
	double gaussRandom(const double&, const double&);
	/** @brief Produces two gaussians with defined distance */
	double doubleGaussRandom(const double&, const double&, const double&);

	/****************************************************************************/
	/**
	 * This function produces integer random numbers in the range of [0, max[ .
	 *
	 * @param max The maximum (excluded) value of the range
	 * @return Discrete random numbers evenly distributed in the range [0,max[
	 */
	template <typename int_type>
	int_type discreteRandom(
			  const int_type& max
			, typename boost::enable_if<boost::is_integral<int_type> >::type* dummy = 0
	) {
	#ifdef DEBUG
		int_type result = boost::numeric_cast<int_type> (evenRandom(boost::numeric_cast<double> (max)));
		assert(result<max);
		return result;
	#else
		return static_cast<int_type>(evenRandom(static_cast<double>(max)));
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
	int_type discreteRandom(
			  const int_type& min
			, const int_type& max
			, typename boost::enable_if<boost::is_integral<int_type> >::type* dummy = 0
	) {
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

	/** @brief Two gaussian random numbers are produced in one go. One number can be cached here */
	double gaussCache_;
	/** @brief Specifies whether a valid cached gaussian is available */
	bool gaussCacheAvailable_;
};

/****************************************************************************/
// Some helper functions

/** @brief Puts a Gem::Util::rnrGenerationMode into a stream. Needed also for boost::lexical_cast<> */
std::ostream& operator<<(std::ostream&, const Gem::Util::rnrGenerationMode&);

/** @brief Reads a Gem::Util::rnrGenerationMode item from a stream. Needed also for boost::lexical_cast<> */
std::istream& operator>>(std::istream&, Gem::Util::rnrGenerationMode&);

/****************************************************************************/

} /* namespace Util */
} /* namespace Gem */

#endif /* GRANDOM_HPP_ */
