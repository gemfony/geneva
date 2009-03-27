/**
 * @file GBoolean.hpp
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

// Boost headers go here

#include <boost/version.hpp>
#include "GGlobalDefines.hpp"
#if BOOST_VERSION < ALLOWED_BOOST_VERSION
#error "Error: Boost has incorrect version !"
#endif /* BOOST_VERSION */

#include <boost/cstdint.hpp>

#ifndef GBOOLEAN_HPP_
#define GBOOLEAN_HPP_

// GenEvA headers go here

#include "GParameterT.hpp"
#include "GEnums.hpp"
#include "GLogger.hpp"
#include "GenevaExceptions.hpp"

namespace Gem {
namespace GenEvA {

/************************************************************************/
/**
 * This class encapsulates a single bit, represented as a bool. This might appear heavy weight,
 * and indeed for most applications this is not the recommended solution -
 * use the GBooleanCollection instead.
 *
 * Bits are mutated by the GBooleanAdaptor in GenEvA. It incorporates a mutable bit-flip probability.
 * The reason for this class is that there might be applications where one might want different flip
 * probabilities for different bits. Where this is the case, separate GBooleanAdaptors must be
 * assigned to each bit value, which cannot be done with the GBooleanCollection. Plus, having
 * a separate bit class adds some consistency to GenEvA, as other values (most
 * notably doubles) have their own class as well (GDouble).
 */
class GBoolean
	:public GParameterT<bool> {
	///////////////////////////////////////////////////////////////////////
	friend class boost::serialization::access;

	template<typename Archive>
	void serialize(Archive & ar, const unsigned int version) {
		using boost::serialization::make_nvp;
		ar & make_nvp("GParameterT",	boost::serialization::base_object<GParameterT<bool> >(*this));
	}
	///////////////////////////////////////////////////////////////////////

public:
	/** @brief The standard constructor */
	GBoolean(void);
	/** @brief Initialization with a boolean */
	explicit GBoolean(const bool&);
	/** @brief The standard copy constructor */
	GBoolean(const GBoolean&);
	/** @brief The standard destructor */
	virtual ~GBoolean();

	/** @brief The standard assignment operator */
	const GBoolean& operator=(const GBoolean&);

	/** @brief Loads another GBoolean object, camouflaged as a GObject */
	virtual void load(const GObject * gb);
	/** @brief Creates a deep copy of this object */
	virtual GObject *clone(void);
};

/************************************************************************/

} /* namespace GenEvA */
} /* namespace Gem */

#endif /* GBOOLEAN_HPP_ */
