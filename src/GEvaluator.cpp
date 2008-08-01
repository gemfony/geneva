/**
 * @file
 */

/* GEvaluator.cpp
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

#include "GEvaluator.hpp"

// Included here so no conflicts occur. See explanation at
// http://www.boost.org/libs/serialization/doc/special.html#derivedpointers
#include <boost/serialization/export.hpp>
BOOST_CLASS_EXPORT(Gem::GenEvA::GEvaluator)

// break circular dependency
#include "GParameterSet.hpp"

namespace Gem
{
	namespace GenEvA
	{
		/******************************************************************/
		/**
		 * The default constructor
		 */
		GEvaluator::GEvaluator() :GObject()
		{ /* nothing */	}

		/******************************************************************/
		/**
		 * The copy constructor
		 */
		GEvaluator::GEvaluator(const GEvaluator& cp)
			:GObject(cp),
			 eval_(cp.eval_)
		{ /* nothing */ }

		/******************************************************************/
		/**
		 * Standard destructor
		 */
		GEvaluator::~GEvaluator()
		{
			eval_.clear();
		}

		/******************************************************************/
		/**
		 * A standard assignment operator
		 *
		 * @param cp A copy of another GEvaluator object
		 * @return A constant reference of this object
		 */
		const GEvaluator& GEvaluator::operator=(const GEvaluator& cp){
			this->load(&cp);
			return *this;
		}

		/******************************************************************/
		/**
		 * Creates a deep clone of this object
		 */
		GObject* GEvaluator::clone()
		{
			return new GEvaluator(*this);
		}

		/******************************************************************/
		/**
		 * Resets the class to its initial state
		 */
		void GEvaluator::reset(){
			// Erase our local data
			eval_.clear();

			// Then erase our parent object's data
			GObject::reset();
		}

		/******************************************************************/
		/** @brief Loads the data of another GEvaluator, camouflaged as a GObject */
		void GEvaluator::load(const GObject* cp){
			// Convert to local format
			const GEvaluator *ge_load = this->checkedConversion(cp, this);

			// Load the parent class'es data
			GObject::load(cp);

			// Then load our local data
			eval_ = ge_load->eval_;
		}

		/******************************************************************/
		/** @brief Overload this function to allow evaluation of GParameterSet objects */
		double GEvaluator::eval(const GParameterSet& gps){
			if(!eval_){ // Has an evaluation function been registered ?
				std::ostringstream error;
				error << "In GEvaluator::eval(): Error!" << std::endl
					  << "No evaluation function has been registered." << std::endl;

				// Let the audience know
				LOGGER.log(error.str(), Gem::GLogFramework::CRITICAL);
				// And throw an exception
				throw geneva_evaluation_function_not_present() << error_string(error.str());
			}

			return eval_(gps);
		}

		/******************************************************************/
		/**
		 * Allows to register an evaluation function with this class. Note that information
		 * contained in a function object stored in eval_ can not be serialized. Using this
		 * facility in a networked environment is thus not useful. The facility can be used
		 * for quick, local testing of an optimization framework, though.
		 *
		 * @param evfunc A Boost.function object for the evaluation of this object
		 */
		void GEvaluator::registerEvalFunction(const boost::function<double (const GParameterSet&)> evfunc){
			eval_ = evfunc;
		}
		/******************************************************************/

	} /* namespace GenEvA */
} /* namespace Gem */

