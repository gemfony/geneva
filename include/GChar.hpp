/**
 * @file GChar.hpp
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

#ifndef GCHAR_HPP_
#define GCHAR_HPP_

// GenEvA headers go here

#include "GParameterT.hpp"
#include "GEnums.hpp"
#include "GLogger.hpp"
#include "GenevaExceptions.hpp"

namespace Gem {
namespace GenEvA {

/************************************************************************/
/**
 * This class encapsulates a char type. This might appear heavy weight,
 * and indeed for most applications this is not the recommended solution -
 * use the GString class instead. char types are mutated by the GCharAdaptor
 * in GenEvA.
 */
class GChar
	:public GParameterT<char> {
	///////////////////////////////////////////////////////////////////////
	friend class boost::serialization::access;

	template<typename Archive>
	void serialize(Archive & ar, const unsigned int version) {
		using boost::serialization::make_nvp;
		ar & make_nvp("GParameterT", boost::serialization::base_object<GParameterT<char> >(*this));
	}
	///////////////////////////////////////////////////////////////////////

public:
	/** @brief The standard constructor */
	GChar(void);
	/** @brief Initialization with a char */
	explicit GChar(const char&);
	/** @brief The standard copy constructor */
	GChar(const GChar&);
	/** @brief The standard destructor */
	virtual ~GChar();

	/** @brief The standard assignment operator */
	const GChar& operator=(const GChar&);

    /** @brief Checks equality */
    bool operator==(const GChar&) const;
    /** @brief Checks inequality */
    bool operator!=(const GChar&) const;
    /** @brief Checks equality */
    bool isEqualTo(const GChar&) const;
    /** @brief Checks similarity */
    bool isSimilarTo(const GChar&, const double& limit = 0.) const;

	/** @brief Loads another GChar object, camouflaged as a GObject */
	virtual void load(const GObject * gb);
	/** @brief Creates a deep copy of this object */
	virtual GObject *clone(void);
};

/************************************************************************/

} /* namespace GenEvA */
} /* namespace Gem */

#endif /* GCHAR_HPP_ */
