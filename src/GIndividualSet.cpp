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

				LOGGER.log(error.str(), Gem::GLogFramework::CRITICAL);
				throw geneva_error_condition() << error_string(error.str());
			}

			// Load the parent class'es data
			GMutableSetT<Gem::GenEvA::GIndividual>::load(cp);
		}

		/******************************************************************/

	} /* namespace GenEvA */
} /* namespace Gem */
