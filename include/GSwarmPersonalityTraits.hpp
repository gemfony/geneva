/**
 * @file GSwarmPersonalityTraits.hpp
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
#include <string>

// Includes check for correct Boost version(s)
#include "GGlobalDefines.hpp"

// Boost headers go here
#include <boost/cstdint.hpp>
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


#ifndef GSWARMPERSONALITYTRAITS_HPP_
#define GSWARMPERSONALITYTRAITS_HPP_

// For Microsoft-compatible compilers
#if defined(_MSC_VER)  &&  (_MSC_VER >= 1020)
#pragma once
#endif


// GenEvA headers go here
#include "GPersonalityTraits.hpp"

namespace Gem {
namespace GenEvA {

/*********************************************************************************/
// Forward declaration needed as GSwarmPersonalityTraits.hpp is
// included in GIndividual.hpp
class GIndividual;

/*********************************************************************************/
/**
 * This class adds variables and functions to GPersonalityTraits that are specific
 * to evolutionary algorithms.
 */
class GSwarmPersonalityTraits :public GPersonalityTraits
{
	///////////////////////////////////////////////////////////////////////
	friend class boost::serialization::access;

	template<typename Archive>
	void serialize(Archive & ar, const unsigned int){
	  using boost::serialization::make_nvp;

	  ar & BOOST_SERIALIZATION_BASE_OBJECT_NVP(GPersonalityTraits)
	     & BOOST_SERIALIZATION_NVP(popPos_)
	     & BOOST_SERIALIZATION_NVP(command_)
	     & BOOST_SERIALIZATION_NVP(c_local_)
	     & BOOST_SERIALIZATION_NVP(c_local_range_)
	     & BOOST_SERIALIZATION_NVP(c_global_)
	     & BOOST_SERIALIZATION_NVP(c_global_range_)
	     & BOOST_SERIALIZATION_NVP(c_delta_)
	     & BOOST_SERIALIZATION_NVP(c_delta_range_);
	}
	///////////////////////////////////////////////////////////////////////

public:
	/** @brief The default constructor */
	GSwarmPersonalityTraits();
	/** @brief The copy contructor */
	GSwarmPersonalityTraits(const GSwarmPersonalityTraits&);
	/** @brief The standard destructor */
	virtual ~GSwarmPersonalityTraits();

	/** @brief Checks for equality with another GSwarmPersonalityTraits object */
	bool operator==(const GSwarmPersonalityTraits&) const;
	/** @brief Checks for inequality with another GSwarmPersonalityTraits object */
	bool operator!=(const GSwarmPersonalityTraits&) const;

	/** @brief Checks whether this object fulfills a given expectation in relation to another object */
	virtual boost::optional<std::string> checkRelationshipWith(const GObject&, const Gem::Util::expectation&, const double&, const std::string&, const std::string&, const bool&) const;

	/** @brief Sets a command to be performed by a remote client. */
	virtual void setCommand(const std::string&);
	/** @brief Retrieves the command to be performed by a remote client. */
	virtual std::string getCommand() const;

	/** @brief Sets the position of the individual in the population */
	void setPopulationPosition(std::size_t) ;
	/** @brief Retrieves the position of the individual in the population */
	std::size_t getPopulationPosition(void) const;

	/** @brief Allows to set a static multiplier for local distances */
	void setCLocal(const double&);
	/** @brief Allows to set the lower and upper boundary for random multiplier range for local distances */
	void setCLocal(const double&, const double&);

	/** @brief Allows to retrieve the static multiplier for local distances or the lower boundary of a random range */
	double getCLocal() const;
	/** @brief Allows to retrieve the random multiplier range for local distances (-1 if unset) */
	double getCLocalRange() const;

	/** @brief Allows to set a static multiplier for global distances */
	void setCGlobal(const double&);
	/** @brief Allows to set the lower and upper boundary for random multiplier range for global distances */
	void setCGlobal(const double&, const double&);

	/** @brief Allows to retrieve the static multiplier for local distances or the lower boundary of a random range */
	double getCGlobal() const;
	/** @brief Allows to retrieve the random multiplier range for local distances (-1 if unset) */
	double getCGlobalRange() const;

	/** @brief Allows to set a static multiplier for deltas */
	void setCDelta(const double&);
	/** @brief Allows to set the lower and upper boundary for random multiplier range for deltas */
	void setCDelta(const double&, const double&);

	/** @brief Allows to retrieve the static multiplier for deltas or the lower boundary of a random range */
	double getCDelta() const;
	/** @brief Allows to retrieve the random multiplier range for deltas (-1 if unset) */
	double getCDeltaRange() const;

	/** @brief Makes the globally best individual known to this object */
	void registerGlobalBest(boost::shared_ptr<Gem::GenEvA::GIndividual>);
	/** @brief Makes the locally best individual known to this object */
	void registerLocalBest(boost::shared_ptr<Gem::GenEvA::GIndividual>);

	/** @brief Applies modifications to this object. This is needed for testing purposes */
	virtual bool modify_GUnitTests();
	/** @brief Performs self tests that are expected to succeed. This is needed for testing purposes */
	virtual void specificTestsNoFailureExpected_GUnitTests();
	/** @brief Performs self tests that are expected to fail. This is needed for testing purposes */
	virtual void specificTestsFailuresExpected_GUnitTests();

protected:
	/** @brief Loads the data of another GSwarmPersonalityTraits object */
	virtual void load_(const GObject*);
	/** @brief Creates a deep clone of this object */
	virtual GObject* clone_() const;

private:
	/** @brief Stores the current position in the population */
	std::size_t popPos_;
	/** @brief The command to be performed by remote clients */
	std::string command_;

	/** @brief A factor for multiplication of local bests, or lower end of a random range */
	double c_local_;
	/** @brief A possible range for random multiplication of local bests. -1 if disabled */
	double c_local_range_;

	/** @brief A factor for multiplication of global bests, or lower end of a random range */
	double c_global_;
	/** @brief A possible range for random multiplication of global l bests. -1 if disabled */
	double c_global_range_;

	/** @brief A factor for multiplication of deltas, or lower end of a random range */
	double c_delta_;
	/** @brief A possible range for random multiplication of velocities. -1 if disabled */
	double c_delta_range_;

	/** @brief A pointer to the locally best individual. It will not be serialized or copied */
	boost::shared_ptr<Gem::GenEvA::GIndividual> local_best_;
	/** @brief A pointer to the globally best individual. It will not be serialized or copied */
	boost::shared_ptr<Gem::GenEvA::GIndividual> global_best_;
};
} /* namespace GenEvA */
} /* namespace Gem */


#endif /* GSWARMPERSONALITYTRAITS_HPP_ */

