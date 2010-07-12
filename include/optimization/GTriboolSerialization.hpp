/**
 * @file GTriboolSerialization.hpp
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

// Includes check for correct Boost version(s)
#include "GGlobalDefines.hpp"

// Boost headers go here
#include <boost/logic/tribool.hpp>
#include <boost/serialization/split_free.hpp>
#include <boost/cstdint.hpp>

#ifndef GTRIBOOLSERIALIZATION_HPP_
#define GTRIBOOLSERIALIZATION_HPP_

// For Microsoft-compatible compilers
#if defined(_MSC_VER)  &&  (_MSC_VER >= 1020)
#pragma once
#endif

// Geneva headers go here
#include "GOptimizationEnums.hpp"

namespace boost {
namespace serialization {

/*
 * These helper functions allow to serialize boost::logic::tribool variables
 */

/***************************************************************************************/
/**
 * Saves a tribool variable to an archive
 */
template <class Archive>
void save(Archive& ar,
          const boost::logic::tribool& val,
          unsigned int)
{
	Gem::Util::triboolStates tbs = Gem::Util::FALSE;
	if(val == true)
		tbs = Gem::Util::TRUE;
	else if(boost::logic::indeterminate(val))
		tbs = Gem::Util::INDETERMINATE;

	ar & make_nvp("tbs", tbs);
}

/***************************************************************************************/
/**
 * Loads a tribool variable from an archive
 */
template <class Archive>
void load(Archive& ar,
          boost::logic::tribool& val,
          unsigned int)
{
	Gem::Util::triboolStates tbs = Gem::Util::FALSE;
	ar & make_nvp("tbs", tbs);

	switch(tbs) {
	case Gem::Util::FALSE:
		val=false;
		break;

	case Gem::Util::TRUE:
		val=true;
		break;

	case Gem::Util::INDETERMINATE:
		val=boost::logic::indeterminate;
		break;
	};
}

/***************************************************************************************/

} /* namespace serialization */
} /* namespace boost */

/***************************************************************************************/
/**
 * Needed so Boost.Serialization does not search for a serialize function
 */
BOOST_SERIALIZATION_SPLIT_FREE(boost::logic::tribool)

/***************************************************************************************/

#endif /* GTRIBOOLSERIALIZATION_HPP_ */
