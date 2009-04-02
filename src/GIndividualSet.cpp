/**
 * @file GIndividualSet.cpp
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

#include "GIndividualSet.hpp"

// Included here so no conflicts occur. See explanation at
// http://www.boost.org/libs/serialization/doc/special.html#derivedpointers
#include <boost/serialization/export.hpp>
BOOST_CLASS_EXPORT(Gem::GenEvA::GIndividualSet)

namespace Gem
{
	namespace GenEvA
	{
		/******************************************************************/
		/**
		 * The default constructor
		 */
		GIndividualSet::GIndividualSet() :GMutableSetT<Gem::GenEvA::GIndividual>()
		{ /* nothing */	}

		/******************************************************************/
		/**
		 * The copy constructor
		 *
		 * @param cp A constant reference to another GIndividualSet object
		 */
		GIndividualSet::GIndividualSet(const GIndividualSet& cp)
		  :GMutableSetT<Gem::GenEvA::GIndividual>(cp)
		{ /* nothing */ }

		/******************************************************************/
		/**
		 * The destructor
		 */
		GIndividualSet::~GIndividualSet()
		{ /* nothing */ }

		/******************************************************************/
		/**
		 * Checks for equality with another GIndividualSet object
		 *
		 * @param  cp A constant reference to another GIndividualSet object
		 * @return A boolean indicating whether both objects are equal
		 */
		bool GIndividualSet::operator==(const GIndividualSet& cp) const {
			return GIndividualSet::isEqualTo(cp);
		}

		/******************************************************************/
		/**
		 * Checks for inequality with another GIndividualSet object
		 *
		 * @param  cp A constant reference to another GIndividualSet object
		 * @return A boolean indicating whether both objects are inequal
		 */
		bool GIndividualSet::operator!=(const GIndividualSet& cp) const {
			return !GIndividualSet::isEqualTo(cp);
		}

		/******************************************************************/
		/**
		 * Checks for equality with another GIndividualSet object. As we have no
		 * local data, we just check for equality of the parent class-
		 *
		 * @param  cp A constant reference to another GIndividualSet object
		 * @return A boolean indicating whether both objects are equal
		 */
		bool GIndividualSet::isEqualTo(const GIndividualSet& cp) const {
			if(!GMutableSetT<Gem::GenEvA::GIndividual>::isEqualTo(cp)) return  false;
			return true;
		}

		/******************************************************************/
		/**
		 * Checks for similarity with another GIndividualSet object. As we have
		 * no local data, we just check for similarity of the parent class.
		 *
		 * @param  cp A constant reference to another GIndividualSet object
		 * @param limit A double value specifying the acceptable level of differences of floating point values
		 * @return A boolean indicating whether both objects are similar to each other
		 */
		bool GIndividualSet::isSimilarTo(const GIndividualSet& cp, const double& limit) const {
			if(!GMutableSetT<Gem::GenEvA::GIndividual>::isSimilarTo(cp, limit)) return  false;
			return true;
		}

		/******************************************************************/
		/**
		 * Loads the data of another GObject
		 *
		 * @param cp Another GIndividualSet object, camouflaged as a GObject
		 */
		void GIndividualSet::load(const GObject* cp)
		{
			const GIndividualSet *gis_load = static_cast<const GIndividualSet *> (cp);

			// Check that this object is not accidentally assigned to itself.
			if (gis_load == this) {
				std::ostringstream error;
				error << "In GIndividualSet::load(): Error!" << std::endl
					  << "Tried to assign an object to itself." << std::endl;

				throw geneva_error_condition(error.str());
			}

			// Load the parent class'es data
			GMutableSetT<Gem::GenEvA::GIndividual>::load(cp);
		}

		/******************************************************************/

	} /* namespace GenEvA */
} /* namespace Gem */
