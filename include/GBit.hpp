/**
 * @file GBit.hpp
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

#if BOOST_VERSION < 103600
#error "Error: Boost should at least have version 1.36 !"
#endif /* BOOST_VERSION */

#include <boost/cstdint.hpp>

#ifndef GBIT_HPP_
#define GBIT_HPP_

// GenEvA headers go here

#include "GParameterT.hpp"
#include "GEnums.hpp"
#include "GLogger.hpp"
#include "GenevaExceptions.hpp"

namespace Gem {
namespace GenEvA {

/************************************************************************/
/**
 * This class encapsulates a single bit. This might appear heavy weight,
 * and indeed for most applications this is not the recommended solution -
 * use the GBitCollection instead. Bits are mutated by the GBitFlipAdaptor in
 * GenEvA. It incorporates a mutable bit-flip probability. The reason for this class
 * is that there might be applications where one might want different flip probabilities
 * for different bits. Where this is the case, separate GBitFlipAdaptors must be assigned
 * to each bit value, which cannot be done with the GBitCollection. Plus, having
 * a separate bit class adds some consistency to GenEvA, as other values (most
 * notably doubles) have their own class as well (GDouble).
 */
class GBit
	:public GParameterT<Gem::GenEvA::bit> {
	///////////////////////////////////////////////////////////////////////
	friend class boost::serialization::access;

	template<class Archive>
	void serialize(Archive & ar, const unsigned int version) {
		using boost::serialization::make_nvp;
		ar & make_nvp("GParameterT",	boost::serialization::base_object<GParameterT<Gem::GenEvA::bit> >(*this));
	}
	///////////////////////////////////////////////////////////////////////

public:
	/** @brief The standard constructor */
	GBit(void);
	/** @brief Initialization with a boolean */
	explicit GBit(bool);
	/** @brief Initialization with a value */
	explicit GBit(const Gem::GenEvA::bit&);
	/** @brief The standard copy constructor */
	GBit(const GBit&);
	/** @brief The standard destructor */
	virtual ~GBit();

	/** @brief The standard assignment operator */
	const GBit& operator=(const GBit&);

	/** @brief Loads another GBit object, camouflaged as a GObject */
	virtual void load(const GObject * gb);
	/** @brief Creates a deep copy of this object */
	virtual GObject *clone(void);
};

/************************************************************************/

} /* namespace GenEvA */
} /* namespace Gem */

#endif /*GBIT_HPP_*/
