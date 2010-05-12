/**
 * @file GMutableI.hpp
 */

/* Copyright (C) Dr. Ruediger Berlich and Karlsruhe Institute of Technology
 * (University of the State of Baden-Wuerttemberg and National Laboratory
 * of the Helmholtz Association)
 *
 * Contact: info [at] gemfony (dot) com
 *
 * This file is part of the Geneva library, Gemfony scientific's optimization
 * library.
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
 *
 * For further information on Gemfony scientific and Geneva, visit
 * http://www.gemfony.com .
 */

// Standard header files go here
#include <iostream>
#include <sstream>

// Includes check for correct Boost version(s)
#include "GGlobalDefines.hpp"

// Boost header files go here
#if BOOST_VERSION < 104200
#include <boost/exception.hpp> // Deprecated as of Boost 1.42
#else
#include <boost/exception/all.hpp>
#endif

#ifndef GMUTABLEI_HPP_
#define GMUTABLEI_HPP_

// For Microsoft-compatible compilers
#if defined(_MSC_VER)  &&  (_MSC_VER >= 1020)
#pragma once
#endif


// Geneva header files go here

namespace Gem {
namespace GenEvA {

/**************************************************************************************************/
/**
 * This is a simple interface class for mutable objects.
 */
class GMutableI {
public:
	/** @brief The standard destructor */
	virtual ~GMutableI(){ /* nothing */ }

	/** @brief Allows derivatives to be adapted */
	virtual void adapt() = 0;

	/*********************************************************************************************/
	/**
	 * A version of the adaption functionality that also checks for
	 * exceptions. To be used when adapt() is to become the main
	 * function to be called by a thread.
	 */
	void checkedAdaption(){
#ifdef DEBUG
		try{
			this->adapt();
		}
		catch(std::exception& e){
			std::ostringstream error;
			error << "In GMutableI::checkedAdaption(): Caught std::exception with message" << std::endl
				  << e.what() << std::endl;
			std::cerr << error.str();
			std::terminate();
		}
		catch(boost::exception& e){
			std::ostringstream error;
			error << "In GMutableI::checkedAdaption(): Caught boost::exception" << std::endl;
			std::cerr << error.str();
			std::terminate();
		}
		catch(...){
			std::ostringstream error;
			error << "In GMutableI::checkedAdaption(): Caught unknown exception" << std::endl;
			std::cerr << error.str();
			std::terminate();
		}
#else
	this->adapt();
#endif
	}
};

} /* namespace GenEvA */
} /* namespace Gem */

/**************************************************************************************************/

#endif /* GMUTABLEI_HPP_ */
