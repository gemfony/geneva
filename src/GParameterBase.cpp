/**
 * @file
 */

/* GParameterBase.cpp
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

#include "GParameterBase.hpp"

// Included here so no conflicts occur. See explanation at
// http://www.boost.org/libs/serialization/doc/special.html#derivedpointers
#include <boost/serialization/export.hpp>
BOOST_CLASS_EXPORT(Gem::GenEvA::GParameterBase)

namespace Gem {
namespace GenEvA {

/**********************************************************************************/
/**
 * The default constructor. No local data, hence nothing to do.
 */
GParameterBase::GParameterBase() throw() :GMutableI(), GObject()
{ /* nothing */ }

/**********************************************************************************/
/**
 * The standard copy constructor. No local data, hence nothing to do.
 *
 * @param cp A copy of another GParameterBase object
 */
GParameterBase::GParameterBase(const GParameterBase& cp) :GMutableI(cp), GObject(cp)
{ /* nothing */ }

/**********************************************************************************/
/**
 * The standard destructor. No local data, hence nothing to do.
 */
GParameterBase::~GParameterBase()
{ /* nothing */ }

/**********************************************************************************/
/**
 * Resets the class to its initial state
 */
void GParameterBase::reset(){
	// GMutableI doesn't have a reset function
	GObject::reset();
}

/**********************************************************************************/
/**
 * Loads the data of another GObject
 *
 * @param cp A copy of another GParameterBase object, camouflaged as a GObject
 */
void GParameterBase::load(const GObject* cp){
    const GParameterBase *gpb_load = dynamic_cast<const GParameterBase *>(cp);

    // dynamic_cast will emit a NULL pointer, if the conversion failed
    if(!gpb_load){
    	std::ostringstream error;
		error << "In GParameterBase::load(): Conversion error!" << std::endl;

    	LOGGER.log(error.str(), Gem::GLogFramework::CRITICAL);

		// throw an exception. Add some information so that if the exception
		// is caught through a base object, no information is lost.
		throw geneva_dynamic_cast_conversion_error() << error_string(error.str());
    }

    // Check that this object is not accidentally assigned to itself.
    if(gpb_load == this){
    	std::ostringstream error;
		error << "In GParameterBase::load(): Error!" << std::endl
			  << "Tried to assign an object to itself." << std::endl;

		LOGGER.log(error.str(), Gem::GLogFramework::CRITICAL);
		throw geneva_dynamic_cast_conversion_error() << error_string(error.str());
    }

	// Load the parent class'es data
	GObject::load(cp);
}

/**********************************************************************************/

}} /* namespace Gem::GenEvA */

