/**
 * \file
 */

/* GHelperFunctionsT.hpp
 *
 * Copyright (C) 2004-2008 Dr. Ruediger Berlich
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

#include <sstream>

#include <boost/version.hpp>

#if BOOST_VERSION < 103600
#error "Error: Boost should at least have version 1.36 !"
#endif /* BOOST_VERSION */

#include <boost/cstdint.hpp>


#ifndef GHELPFERFUNCTIONST_HPP_
#define GHELPFERFUNCTIONST_HPP_

using namespace std;

#include "GObject.hpp"
#include "GenevaExceptions.hpp"
#include "GLogger.hpp"

namespace Gem
{
namespace GenEvA
{

/**************************************************************************************************/
/**
 * The load function takes a GObject pointer and converts it to a pointer to a derived class. This
 * work is centralized in this function.
 *
 * @param load_ptr A pointer to another T-object, camouflaged as a GObject
 */
template <class T>
const T* checkConversion(const Gem::GenEvA::GObject *load_ptr, const T* This){
	const T *result = dynamic_cast<const T *> (load_ptr);

	// dynamic_cast will emit a NULL pointer, if the conversion failed
	if (!result) {
		std::ostringstream error;
		error << "In GObject::checkConversion<T>() : Conversion error!" << std::endl;

		LOGGER.log(error.str(), Gem::GLogFramework::CRITICAL);

		// throw an exception. Add some information so that if the exception
		// is caught through a base object, no information is lost.
		throw geneva_dynamic_cast_conversion_error() << error_string(error.str());
	}

	// Check that this object is not accidentally assigned to itself.
	if (load_ptr == This) {
		std::ostringstream error;
		error << "In GObject::checkConversion<T>() : Error!" << std::endl
				<< "Tried to assign an object to itself." << std::endl;

		LOGGER.log(error.str(), Gem::GLogFramework::CRITICAL);
		throw geneva_object_assigned_to_itself() << error_string(error.str());
	}

	return result;
}

/**************************************************************************************************/

} /* namespace GenEvA */
} /* namespace Gem */

#endif /* GHELPFERFUNCTIONST_HPP_*/
