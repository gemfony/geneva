/**
 * @file GenevaHelperFunctionsT.hpp
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
#include <vector>

// Includes check for correct Boost version(s)
#include "common/GGlobalDefines.hpp"

// Boost headers go here
#include <boost/shared_ptr.hpp>
#include <boost/utility/enable_if.hpp>
#include <boost/type_traits.hpp>

#ifndef GENEVAHELPERFUNCTIONST_HPP_
#define GENEVAHELPERFUNCTIONST_HPP_

// For Microsoft-compatible compilers
#if defined(_MSC_VER)  &&  (_MSC_VER >= 1020)
#pragma once
#endif

// Our own headers go here
#include "geneva/GObject.hpp"

namespace Gem
{
namespace Geneva
{

/**************************************************************************************************/
/**
 * This function takes two smart Geneva pointers and copies their contents (if any) with the
 * load / clone functions. boost::enable_if makes sure that this function can only be called
 * for GObject-derivatives (which are then required to have to load/clone interface.
 *
 * @param from The source smart pointer
 * @param to The target smart pointer
 */
template <typename T>
void copyGenevaSmartPointer (
	const boost::shared_ptr<T>& from
	, boost::shared_ptr<T>& to
	, typename boost::enable_if<boost::is_base_of<Gem::Geneva::GObject, T> >::type* dummy = 0
) {
	// Make sure to is empty when from is empty
	if(!from) {
		to.reset();
	} else {
		if(!to) {
			to = from->GObject::clone<T>();
		} else {
			to->load(from);
		}
	}
}

/**************************************************************************************************/

} /* namespace Geneva */
} /* namespace Gem */

#endif /* GENEVAHELPERFUNCTIONST_HPP_ */
