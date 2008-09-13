/**
 * @file GDoubleCollection.cpp
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

#include "GDoubleCollection.hpp"

/**
 * Included here so no conflicts occur. See explanation at
 * http://www.boost.org/libs/serialization/doc/special.html#derivedpointers
 */
#include <boost/serialization/export.hpp>
BOOST_CLASS_EXPORT(Gem::GenEvA::GDoubleCollection)

namespace Gem {
namespace GenEvA {

/**********************************************************************/
/**
 * The default constructor.
 */
GDoubleCollection::GDoubleCollection() :
	GParameterCollectionT<double> ()
{ /* nothing */ }

/**********************************************************************/
/**
 * Fills the class with nval random double values
 *
 * @param nval Number of values to put into the vector
 */
GDoubleCollection::GDoubleCollection(std::size_t nval) :
	GParameterCollectionT<double> () {
	for (std::size_t i = 0; i < nval; i++) {
		data.push_back(gr.evenRandom(-DEFINIT, DEFINIT));
	}
}

/**********************************************************************/
/**
 * Fills the class with nval random double values between min and max.
 *
 * @param nval Number of values to put into the vector
 * @param min The lower boundary for random entries
 * @param max The upper boundary for random entries
 */
GDoubleCollection::GDoubleCollection(std::size_t nval, double min, double max) :
	GParameterCollectionT<double> () {
	for (std::size_t i = 0; i < nval; i++) {
		data.push_back(gr.evenRandom(min, max));
	}
}

/**********************************************************************/
/**
 * The standard copy constructor. All work is done by the parent class.
 *
 * @param cp A copy of another GDoubleCollection object
 */
GDoubleCollection::GDoubleCollection(const GDoubleCollection& cp)
	:GParameterCollectionT<double> (cp)
{ /* nothing */ }

/**********************************************************************/
/**
 * The standard destructor. All work will be done by the parent class.
 */
GDoubleCollection::~GDoubleCollection()
{ /* nothing */ }

/**********************************************************************/
/**
 * A standard assignment operator. We have no local data, so all work
 * is done by the parent class.
 *
 * @param cp A copy of another GDoubleCollection object
 */
const GDoubleCollection& GDoubleCollection::operator=(const GDoubleCollection& cp)
{
	GDoubleCollection::load(&cp);
	return *this;
}

/**********************************************************************/
/**
 * Creates a deep copy of this object
 *
 * @return A deep copy of this object, camouflaged as a GObject
 */
GObject *GDoubleCollection::clone()
{
	return new GDoubleCollection(*this);
}

/**********************************************************************/
/**
 * Loads the data of another GDoubleCollection object,
 * camouflaged as a GObject. We have no local data, so
 * all we need to do is to the standard identity check,
 * preventing that an object is assigned to itself.
 *
 * @param cp A copy of another GDoubleCollection object, camouflaged as a GObject
 */
void GDoubleCollection::load(const GObject * cp) {
	// Check that this object is not accidently assigned to itself.
	// As we do not actually do any calls with this pointer, we
	// can use the faster static_cast<>
	if(static_cast<const GDoubleCollection *>(cp) == this) {
		std::ostringstream error;

		error << "In GDoubleCollection::load(): Error!" << std::endl
			  << "Tried to assign an object to itself." << std::endl;

		LOGGER->log(error.str(), Gem::GLogFramework::CRITICAL);
		throw geneva_error_condition() << error_string(error.str());
	}

	GParameterCollectionT<double>::load(cp);
}

/**********************************************************************/
/**
 * Appends nval random values between min and max to this class.
 *
 * @param nval Number of values to put into the vector
 * @param min The lower boundary for random entries
 * @param max The upper boundary for random entries
 */
void GDoubleCollection::addRandomData(std::size_t nval, double min, double max) {
	for(std::size_t i= 0; i<nval; i++) {
		data.push_back(gr.evenRandom(min,max));
	}
}

/************************************************************************/

} /* namespace GenEvA */
} /* namespace Gem */
