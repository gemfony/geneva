/**
 * @file GBit.hpp
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

#include "GBit.hpp"

// Included here so no conflicts occur. See explanation at
// http://www.boost.org/libs/serialization/doc/special.html#derivedpointers
#include <boost/serialization/export.hpp>
BOOST_CLASS_EXPORT(Gem::GenEvA::GBit)

namespace Gem
{
	namespace GenEvA
	{

		/************************************************************************/
		/**
		 * The standard constructor. No local data, hence all work is done
		 * by the parent class. We initialize with a random bit value. As we
		 * cannot rely on the random number generator already having been
		 * initialized in the parent class'es destructor's arguments, we need
		 * to set the value again in the body of this functon.
		 */
		GBit::GBit(void)
			:GParameterT<Gem::GenEvA::bit>(Gem::GenEvA::G_TRUE)
		{
		   this->setValue(gr.bitRandom());
		}

		/************************************************************************/
		/**
		 * Initialization with a boolean value
		 *
		 * @param val The value to assign (in converted form) to this object
		 */
		GBit::GBit(bool val)
			:GParameterT<Gem::GenEvA::bit>(val?Gem::GenEvA::G_TRUE:Gem::GenEvA::G_FALSE)
		{ /* nothing */ }

		/************************************************************************/
		/**
		 * A constructor that assigns an initialisation value to the bit.
		 *
		 * @param val the desired initial value of the GBit object
		 */
		GBit::GBit(const Gem::GenEvA::bit& val)
			:GParameterT<Gem::GenEvA::bit>(val)
		{ /* nothing */}

		/************************************************************************/
		/**
		 * A standard copy constructor. We have no local data. Hence all work is
		 * done by the parent class.
		 *
		 * @param cp A copy of another GBit object
		 */
		GBit::GBit(const GBit& cp)
			:GParameterT<Gem::GenEvA::bit>(cp)
		{ /* nothing */}

		/************************************************************************/
		/**
		 * The standard destructor. No local data, hence nothing to do.
		 */
		GBit::~GBit()
		{ /* nothing */}

		/************************************************************************/
		/**
		 * A standard assignment operator for GBit object.
		 *
		 * @param cp A copy of another GBit object
		 * @return A copy of this object
		 */
		const GBit& GBit::operator=(const GBit& cp) {
			GBit::load(&cp);
			return *this;
		}

		/************************************************************************/
		/**
		 * Loads the data of another GBit Object.
		 *
		 * @param gb A pointer to another GBit object, camouflaged as a GObject
		 */
		void GBit::load(const GObject * cp) {
			// Check that this object is not accidently assigned to itself.
			// As we do not actually do any calls with this pointer, we
			// can use the faster static_cast<>
			if(static_cast<const GBit *>(cp) == this) {
				std::ostringstream error;
				error << "In GBit::load() : Error!" << std::endl
					  << "Tried to assign an object to itself." << std::endl;

				LOGGER.log(error.str(), Gem::GLogFramework::CRITICAL);

				throw geneva_error_condition() << error_string(error.str());
			}

			// We can rely on the parent class, as we
			// have no local data ourselves.
			GParameterT<Gem::GenEvA::bit>::load(cp);
		}

		/************************************************************************/
		/**
		 * Creates a deep copy of this object
		 *
		 * @return A deep copy of this object
		 */
		GObject *GBit::clone(void) {
			return new GBit(*this);
		}

		/************************************************************************/

	} /* namespace GenEvA */
} /* namespace Gem */
