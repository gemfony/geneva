/**
 * @file
 */

/* GRateableI.hpp
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

// Standard header files go here
#include <sstream>

// Boost header files go here

#include <boost/version.hpp>

#if BOOST_VERSION < 103600
#error "Error: Boost should at least have version 1.36 !"
#endif /* BOOST_VERSION */

#ifndef GRATEABLEI_HPP_
#define GRATEABLEI_HPP_

// Geneva header files go here

namespace Gem {
namespace GenEvA {

/**
 * A simple interface class for objects that can be evaluated.
 */
class GRateableI {
public:
	/** @brief The standard destructor */
	virtual ~GRateableI(){ /* nothing */ }

	/** @brief Retrieve a value for this class */
	virtual double fitness(void) = 0;

	/*********************************************************************************************/
	/**
	 * A version of the fitness framework that also checks for
	 * exceptions. To be used when fitness() is to become the main
	 * function to be called by a thread.
	 */
	double checkedFitness(void){
		try{
			return this->fitness();
		}
		catch(std::exception& e){
			std::ostringstream error;
			error << "In GRateableI::checkedFitness(): Caught std::exception with message" << std::endl
				  << e.what() << std::endl;
			LOGGER.log(error.str(), Gem::GLogFramework::CRITICAL);

			std::terminate();
		}
		catch(boost::exception& e){
			std::ostringstream error;
			error << "In GRateableI::checkedFitness(): Caught boost::exception with message" << std::endl
				  << e.diagnostic_information() << std::endl;
			LOGGER.log(error.str(), Gem::GLogFramework::CRITICAL);

			std::terminate();
		}
		catch(...){
			std::ostringstream error;
			error << "In GRateableI::checkedFitness(): Caught unknown exception" << std::endl;
			LOGGER.log(error.str(), Gem::GLogFramework::CRITICAL);

			std::terminate();
		}
	}
};

} /* namespace GenEvA */
} /* namespace Gem */

#endif /* GRATEABLEI_HPP_ */
