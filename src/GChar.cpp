/**
 * @file GChar.cpp
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

#include "GChar.hpp"

// Included here so no conflicts occur. See explanation at
// http://www.boost.org/libs/serialization/doc/special.html#derivedpointers
#include <boost/serialization/export.hpp>
BOOST_CLASS_EXPORT(Gem::GenEvA::GChar)

namespace Gem
{
	namespace GenEvA
	{

		/************************************************************************/
		/**
		 * The standard constructor. No local data, hence all work is done
		 * by the parent class. We initialize with a random character.
		 */
		GChar::GChar(void)
			:GParameterT<char>(gr.charRandom())
		{ /* nothing */ }

		/************************************************************************/
		/**
		 * Initialization with a char value
		 *
		 * @param val The value to assign (in converted form) to this object
		 */
		GChar::GChar(const char& val)
			:GParameterT<char>(val)
		{ /* nothing */ }

		/************************************************************************/
		/**
		 * A standard copy constructor. We have no local data. Hence all work is
		 * done by the parent class.
		 *
		 * @param cp A copy of another GChar object
		 */
		GChar::GChar(const GChar& cp)
			:GParameterT<char>(cp)
		{ /* nothing */ }

		/************************************************************************/
		/**
		 * The standard destructor. No local data, hence nothing to do.
		 */
		GChar::~GChar()
		{ /* nothing */ }

		/************************************************************************/
		/**
		 * A standard assignment operator for GChar object.
		 *
		 * @param cp A copy of another GChar object
		 * @return A copy of this object
		 */
		const GChar& GChar::operator=(const GChar& cp) {
			GChar::load(&cp);
			return *this;
		}

		/************************************************************************/
		/**
		 * Checks equality of this object with another.
		 *
		 * @param cp A constant reference to another GChar object
		 * @return A boolean indicating whether both objects are equal
		 */
		bool GChar::operator==(const GChar& cp) const {
			return GChar::isEqualTo(cp);
		}

		/******************************************************************************/
		/**
		 * Checks inequality of this object with another.
		 *
		 * @param cp A constant reference to another GChar object
		 * @return A boolean indicating whether both objects are inequal
		 */
		bool GChar::operator!=(const GChar& cp) const {
			return !GChar::isEqualTo(cp);
		}

		/******************************************************************************/
		/**
		 * Checks equality of this object with another.
		 *
		 * @param cp A constant reference to another GChar object
		 * @return A boolean indicating whether both objects are equal
		 */
		bool GChar::isEqualTo(const GChar& cp) const {
			// Check the parent class'es equality
			if(!GParameterT<char>::isEqualTo(cp)) return false;
			return true;
		}

		/******************************************************************************/
		/**
		 * Checks similarity of this object with another.
		 *
		 * @param cp A constant reference to another GChar object
		 * @param limit The acceptable different between two doubles
		 * @return A boolean indicating whether both objects are similar to each other
		 */
		bool GChar::isSimilarTo(const GChar& cp, const double& limit) const {
			// Check the parent class'es similarity
			if(!GParameterT<char>::isSimilarTo(cp, limit)) return false;
			return true;
		}


		/************************************************************************/
		/**
		 * Loads the data of another GChar Object.
		 *
		 * @param gb A pointer to another GChar object, camouflaged as a GObject
		 */
		void GChar::load(const GObject * cp) {
			// Check that this object is not accidently assigned to itself.
			// As we do not actually do any calls with this pointer, we
			// can use the faster static_cast<>
			if(static_cast<const GChar *>(cp) == this) {
				std::ostringstream error;
				error << "In GChar::load() : Error!" << std::endl
					  << "Tried to assign an object to itself." << std::endl;

				throw geneva_error_condition(error.str());
			}

			// We can rely on the parent class, as we
			// have no local data ourselves.
			GParameterT<char>::load(cp);
		}

		/************************************************************************/
		/**
		 * Creates a deep copy of this object
		 *
		 * @return A deep copy of this object
		 */
		GObject *GChar::clone(void) {
			return new GChar(*this);
		}

		/************************************************************************/

	} /* namespace GenEvA */
} /* namespace Gem */
