/**
 * @file GBoolean.cpp
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

#include "GBoolean.hpp"

// Included here so no conflicts occur. See explanation at
// http://www.boost.org/libs/serialization/doc/special.html#derivedpointers
#include <boost/serialization/export.hpp>
BOOST_CLASS_EXPORT(Gem::GenEvA::GBoolean)

namespace Gem
{
	namespace GenEvA
	{

		/************************************************************************/
		/**
		 * The standard constructor. No local data, hence all work is done
		 * by the parent class. We initialize with a random bit value.
		 */
		GBoolean::GBoolean(void)
			:GParameterT<bool>(gr.boolRandom())
		{ /* nothing */ }

		/************************************************************************/
		/**
		 * Initialization with a boolean value
		 *
		 * @param val The value to assign (in converted form) to this object
		 */
		GBoolean::GBoolean(const bool& val)
			:GParameterT<bool>(val)
		{ /* nothing */ }

		/************************************************************************/
		/**
		 * A standard copy constructor. We have no local data. Hence all work is
		 * done by the parent class.
		 *
		 * @param cp A copy of another GBoolean object
		 */
		GBoolean::GBoolean(const GBoolean& cp)
			:GParameterT<bool>(cp)
		{ /* nothing */ }

		/************************************************************************/
		/**
		 * The standard destructor. No local data, hence nothing to do.
		 */
		GBoolean::~GBoolean()
		{ /* nothing */ }

		/************************************************************************/
		/**
		 * A standard assignment operator for GBoolean object.
		 *
		 * @param cp A copy of another GBoolean object
		 * @return A copy of this object
		 */
		const GBoolean& GBoolean::operator=(const GBoolean& cp) {
			GBoolean::load(&cp);
			return *this;
		}

		/************************************************************************/
	    /**
	     * Checks equality of this object with another.
	     *
	     * @param cp A constant reference to another GBoolean object
	     * @return A boolean indicating whether both objects are equal
	     */
	    bool GBoolean::operator==(const GBoolean& cp) const {
	    	return GBoolean::isEqualTo(cp);
	    }

		/******************************************************************************/
	    /**
	     * Checks inequality of this object with another.
	     *
	     * @param cp A constant reference to another GBoolean object
	     * @return A boolean indicating whether both objects are inequal
	     */
	    bool GBoolean::operator!=(const GBoolean& cp) const {
	    	return !GBoolean::isEqualTo(cp);
	    }

		/******************************************************************************/
	    /**
	     * Checks equality of this object with another.
	     *
	     * @param cp A constant reference to another GBoolean object
	     * @return A boolean indicating whether both objects are equal
	     */
	    bool GBoolean::isEqualTo(const GBoolean& cp) const {
	    	// Check the parent class'es equality
	    	if(!GParameterT<bool>::isEqualTo(cp)) return false;
	    	return true;
	    }

		/******************************************************************************/
	    /**
	     * Checks similarity of this object with another.
	     *
	     * @param cp A constant reference to another GBoolean object
	     * @param limit The acceptable different between two doubles
	     * @return A boolean indicating whether both objects are similar to each other
	     */
	    bool GBoolean::isSimilarTo(const GBoolean& cp, const double& limit) const {
	    	// Check the parent class'es similarity
	    	if(!GParameterT<bool>::isSimilarTo(cp, limit)) return false;
			return true;
	    }

		/******************************************************************************/

		/**
		 * Loads the data of another GBoolean Object.
		 *
		 * @param gb A pointer to another GBoolean object, camouflaged as a GObject
		 */
		void GBoolean::load(const GObject * cp) {
			// Check that this object is not accidently assigned to itself.
			// As we do not actually do any calls with this pointer, we
			// can use the faster static_cast<>
			if(static_cast<const GBoolean *>(cp) == this) {
				std::ostringstream error;
				error << "In GBoolean::load() : Error!" << std::endl
					  << "Tried to assign an object to itself." << std::endl;

				throw geneva_error_condition(error.str());
			}

			// We can rely on the parent class, as we
			// have no local data ourselves.
			GParameterT<bool>::load(cp);
		}

		/************************************************************************/
		/**
		 * Creates a deep copy of this object
		 *
		 * @return A deep copy of this object
		 */
		GObject *GBoolean::clone(void) {
			return new GBoolean(*this);
		}

		/************************************************************************/

	} /* namespace GenEvA */
} /* namespace Gem */
