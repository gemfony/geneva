/**
 * @file GBooleanAdaptor.hpp
 */

/* Copyright (C) 2004-2008 Dr. Ruediger Berlich
 * Copyright (C) 2007-2008 Forschungszentrum Karlsruhe GmbH
 *
 * This file is part of Geneva, Gemfony scientific's optimization library.
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
 */

// Standard headers go here
#include <sstream>

// Boost headers go here

#include <boost/version.hpp>

#if BOOST_VERSION < 103600
#error "Error: Boost should at least have version 1.36 !"
#endif /* BOOST_VERSION */

#include <boost/cstdint.hpp>

#ifndef GBOOLEANADAPTOR_HPP_
#define GBOOLEANADAPTOR_HPP_

// GenEvA headers go here

#include "GAdaptorT.hpp"
#include "GObject.hpp"
#include "GBoundedDouble.hpp"
#include "GEnums.hpp"
#include "GLogger.hpp"
#include "GenevaExceptions.hpp"

namespace Gem {
namespace GenEvA {

const double SGM = 0.01;
const double SGMSGM = 0.001;
const double MINSGM = 0.00001;
const double MAXSGM = 0.01;

const double DEFAULTMUTPROB = 0.05; // 5 percent mutation probability
const std::string GBOOLEANADAPTORSTANDARDNAME="GBooleanAdaptor";

/***********************************************************************************/
/**
 * This class is designed to allow mutations of bit values (represented as booleans).
 * Bits can be flipped with a probability that can be mutated along with the bit value.
 * Hence the adaptor can adapt itself to varying conditions, if desired. Note that
 * this makes the allegedly simple application of flipping a bit a rather complicated
 * procedure. Hence it is recommended to limit usage of this adaptor to bit collections
 * rather than single bits.
 */
class GBooleanAdaptor
	: public GAdaptorT<bool>
{
	///////////////////////////////////////////////////////////////////////
	friend class boost::serialization::access;

	template<typename Archive>
	void serialize(Archive & ar, const unsigned int version) {
		using boost::serialization::make_nvp;
		ar & make_nvp("GAdaptorT", boost::serialization::base_object<GAdaptorT<bool> >(*this));
		ar & make_nvp("mutProb_", mutProb_);
	}
	///////////////////////////////////////////////////////////////////////

public:
	/** @brief Standard constructor. Every adaptor needs a name */
	GBooleanAdaptor();
	/** @brief Constructor sets the mutation probability to a given value */
	explicit GBooleanAdaptor(const double&);
	/** @brief Standard copy constructor */
	GBooleanAdaptor(const GBooleanAdaptor&);
	/** @brief Standard destructor */
	virtual ~GBooleanAdaptor();

	/** @brief A standard assignment operator */
	const GBooleanAdaptor& operator=(const GBooleanAdaptor&);

	/** @brief Loads the content of another GBooleanAdaptor */
	virtual void load(const GObject *);
	/** @brief Creates a deep copy of this object */
	virtual GObject *clone();

	/** @brief Retrieves the current mutation probability */
	double getMutationProbability();
	/** @brief Sets the mutation probability to a given value */
	void setMutationProbability(const double&);

	/** @brief Sets the mutation parameters of the internal GDouble */
	void setMutationParameters(const double&, const double&, const double&, const double&);

	/**********************************************************************/
	/**
	 * Returns the standard name of a GBooleanAdaptor
	 *
	 * @return The name assigned to adaptors of this type
	 */
	static std::string adaptorName() throw() {
		return GBOOLEANADAPTORSTANDARDNAME;
	}

	/**********************************************************************/

protected:
	/** @brief Initializes a new mutation run */
	virtual void adaptMutation();

	/** @brief The actual mutation of the bit value */
	virtual void customMutations(bool&);

private:
	/** @brief Simple flip of a bit value */
	void flip(bool&) const throw();

	GBoundedDouble mutProb_; ///< internal representation of the mutation probability
};

/***********************************************************************************/

} /* namespace GenEvA */
} /* namespace Gem */

#endif /* GBOOLEANADAPTOR_HPP_ */
