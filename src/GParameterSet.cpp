/**
 * @file
 */

/* GParameterSet.cpp
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

#include "GParameterSet.hpp"

// Included here so no conflicts occur. See explanation at
// http://www.boost.org/libs/serialization/doc/special.html#derivedpointers
#include <boost/serialization/export.hpp>
BOOST_CLASS_EXPORT(Gem::GenEvA::GParameterSet)

namespace Gem
{
	namespace GenEvA
	{
		/**********************************************************************************/
		/**
		 * The default constructor.
		 */
		GParameterSet::GParameterSet()
			:GMutableSetT<Gem::GenEvA::GParameterBase>()
		{ /* nothing */ }

		/**********************************************************************************/
		/**
		 * The copy constructor. Note that we cannot rely on the operator=() of the vector
		 * here, as we do not know the actual type of T objects.
		 *
		 * @param cp A copy of another GParameterSet object
		 */
		GParameterSet::GParameterSet(const GParameterSet& cp)
			:GMutableSetT<Gem::GenEvA::GParameterBase>(cp),
			 eval_(cp.eval_)
		{ /* nothing */ }

		/**********************************************************************************/
		/**
		 * The destructor
		 */
		GParameterSet::~GParameterSet()
		{ /* nothing */ }

		/**********************************************************************************/
		/**
		 * A Standard assignment operator
		 *
		 * @param cp A copy of another GParameterSet object
		 * @return A constant reference to this object
		 */
		const GParameterSet& GParameterSet::operator=(const GParameterSet& cp){
			GParameterSet::load(&cp);
			return *this;
		}

		/**********************************************************************************/
		/**
		 * Creates a deep clone of this object.
		 *
		 * @return A deep clone of this object
		 */
		GObject * GParameterSet::clone(){
			return new GParameterSet(*this);
		}

		/**********************************************************************************/
		/**
		 * Loads the data of another GParameterSet object, camouflaged as a GObject.
		 *
		 * @param cp A copy of another GParameterSet object, camouflaged as a GObject
		 */
		void GParameterSet::load(const GObject* cp){
			// Convert to local format
			const GParameterSet *gps_load = this->checkedConversion(cp, this);

			// Load the parent class'es data
			GMutableSetT<Gem::GenEvA::GParameterBase>::load(cp);

			// Then load our local data.
			if(eval_) {// Only necessary if there is a local GEvaluator object
				// Note that we cannot do this by
				// simply assigning the two smart pointers. For once, we want separate
				// evaluation function objects (with their own, modifiable local data)
				// for each object. And then we only have a base pointer.
				GObject *gev_tmp = ((gps_load->eval_).get())->clone();

				GEvaluator *gev = dynamic_cast<GEvaluator *>(gev_tmp);
				if(!gev){
					std::ostringstream error;
					error << "In GParameterSet::load(): Conversion error (2)!" << std::endl;

					LOGGER.log(error.str(), Gem::GLogFramework::CRITICAL);

					// throw an exception. Add some information so that if the exception
					// is caught through a base object, no information is lost.
					throw geneva_dynamic_cast_conversion_error() << error_string(error.str());
				}

				shared_ptr<GEvaluator> p(gev);
				eval_ = p;
			}
		}

		/**********************************************************************************/
		/**
		 * Registers a GEvaluator object with this class. Note that the class will take
		 * charge of the object. It may not be deleted individually by the user.
		 */
		void GParameterSet::registerEvaluator(GEvaluator *eval){
			if(!eval){ // empty pointer ?
				std::ostringstream error;
				error << "In GParameterSet::registerEvaluator(): Error" << std::endl
					  << "Received empty pointer" << std::endl;

				LOGGER.log(error.str(), Gem::GLogFramework::CRITICAL);

				throw geneva_empty_evaluation_function() << error_string(error.str());
			}

			shared_ptr<GEvaluator> p(eval);
			eval_ = p;
		}

		/**********************************************************************************/
		/**
		 * The actual fitness calculation takes place here. Note that you need
		 * to overload this function if you do not want to use the GEvaluator
		 * mechanism.
		 *
		 * @return The newly calculated fitness of this object
		 */
		double GParameterSet::fitnessCalculation(){
			if(!eval_){ // Has an evaluator been stored in this class ?
				std::ostringstream error;
				error << "In GParameterSet::fitnessCalculation(): Error" << std::endl
					  << "No evaluation function present" << std::endl;

				LOGGER.log(error.str(), Gem::GLogFramework::CRITICAL);

				throw geneva_evaluation_function_not_present() << error_string(error.str());
			}

			// Trigger the actual calculation
			return eval_->eval(*this);
		}
		/**********************************************************************************/

	} /* namespace GenEvA */
} /* namespace Gem */
