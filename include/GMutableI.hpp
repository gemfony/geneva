/**
 * @file GMutableI.hpp
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

// Standard header files go here
#include <iostream>
#include <sstream>

// Boost header files go here

#include <boost/version.hpp>
#include "GGlobalDefines.hpp"
#if BOOST_VERSION < ALLOWED_BOOST_VERSION
#error "Error: Boost has incorrect version !"
#endif /* BOOST_VERSION */

#include <boost/exception.hpp>

#ifndef GMUTABLEI_HPP_
#define GMUTABLEI_HPP_

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

	/** @brief Allows derivatives to be mutated */
	virtual void mutate() = 0;

	/*********************************************************************************************/
	/**
	 * A version of the mutation functionality that also checks for
	 * exceptions. To be used when mutate() is to become the main
	 * function to be called by a thread.
	 */
	void checkedMutate(){
#ifdef DEBUG
		try{
			this->mutate();
		}
		catch(std::exception& e){
			std::ostringstream error;
			error << "In GMutableI::checkedMutate(): Caught std::exception with message" << std::endl
				  << e.what() << std::endl;
			std::cerr << error.str();
			std::terminate();
		}
		catch(boost::exception& e){
			std::ostringstream error;
			error << "In GMutableI::checkedMutate(): Caught boost::exception" << std::endl;
			std::cerr << error.str();
			std::terminate();
		}
		catch(...){
			std::ostringstream error;
			error << "In GMutableI::checkedMutate(): Caught unknown exception" << std::endl;
			std::cerr << error.str();
			std::terminate();
		}
#else
	this->mutate();
#endif
	}
};

} /* namespace GenEvA */
} /* namespace Gem */

/**************************************************************************************************/

#endif /* GMUTABLEI_HPP_ */
