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
#include <sstream>

// Boost header files go here

#include <boost/version.hpp>

#if BOOST_VERSION < 103600
#error "Error: Boost should at least have version 1.36 !"
#endif /* BOOST_VERSION */

#include <boost/serialization/nvp.hpp>
#include <boost/serialization/vector.hpp>
#include <boost/serialization/shared_ptr.hpp>
#include <boost/serialization/base_object.hpp>
#include <boost/serialization/utility.hpp>
#include <boost/serialization/tracking.hpp>
#include <boost/serialization/split_member.hpp>
#include <boost/serialization/export.hpp>
#include <boost/serialization/is_abstract.hpp>

#ifndef GMUTABLEI_HPP_
#define GMUTABLEI_HPP_

// Geneva header files go here
#include "GLogger.hpp"

namespace Gem {
namespace GenEvA {

/**************************************************************************************************/
/**
 * This is a simple interface class for mutable objects.
 */
class GMutableI {
	///////////////////////////////////////////////////////////////////////
	friend class boost::serialization::access;

	template<typename Archive>
	void serialize(Archive & ar, const unsigned int version){
	  using boost::serialization::make_nvp;
	  /* nothing - this is a base class */
	}
	///////////////////////////////////////////////////////////////////////

public:
	/** @brief The standard destructor */
	virtual ~GMutableI(){ /* nothing */ }

	/** @brief Allows derivatives to be mutated */
	virtual void mutate(void) = 0;

	/*********************************************************************************************/
	/**
	 * A version of the mutation functionality that also checks for
	 * exceptions. To be used when mutate() is to become the main
	 * function to be called by a thread.
	 */
	void checkedMutate(void){
		try{
			this->mutate();
		}
		catch(std::exception& e){
			std::ostringstream error;
			error << "In GMutableI::checkedMutate(): Caught std::exception with message" << std::endl
				  << e.what() << std::endl;
			LOGGER.log(error.str(), Gem::GLogFramework::CRITICAL);

			std::terminate();
		}
		catch(boost::exception& e){
			std::ostringstream error;
			error << "In GMutableI::checkedMutate(): Caught boost::exception with message" << std::endl
				  << e.diagnostic_information() << std::endl;
			LOGGER.log(error.str(), Gem::GLogFramework::CRITICAL);

			std::terminate();
		}
		catch(...){
			std::ostringstream error;
			error << "In GMutableI::checkedMutate(): Caught unknown exception" << std::endl;
			LOGGER.log(error.str(), Gem::GLogFramework::CRITICAL);

			std::terminate();
		}
	}
};

} /* namespace GenEvA */
} /* namespace Gem */

/**************************************************************************************************/
/**
 * @brief Needed for Boost.Serialization
 */
BOOST_SERIALIZATION_ASSUME_ABSTRACT(Gem::GenEvA::GMutableI)

/**************************************************************************************************/

#endif /* GMUTABLEI_HPP_ */
