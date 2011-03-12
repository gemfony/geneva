/**
 * @file GThreadWrapper.cpp
 */

/*
 * Copyright (C) Gemfony scientific UG (haftungsbeschraenkt)
 *
 * See the AUTHORS file in the top-level directory for a list of authors.
 *
 * Contact: contact [at] gemfony (dot) com
 *
 * This file is part of the Geneva library collection.
 *
 * Geneva was developed with kind support from Karlsruhe Institute of
 * Technology (KIT) and Steinbuch Centre for Computing (SCC). Further
 * information about KIT and SCC can be found at http://www.kit.edu/english
 * and http://scc.kit.edu .
 *
 * Geneva is free software: you can redistribute and/or modify it under
 * the terms of version 3 of the GNU Affero General Public License
 * as published by the Free Software Foundation.
 *
 * Geneva is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with the Geneva library. If not, see <http://www.gnu.org/licenses/>.
 *
 * For further information on Gemfony scientific and Geneva, visit
 * http://www.gemfony.com .
 */

#include "common/GThreadWrapper.hpp"

namespace Gem {
namespace Common {

/**********************************************************************************************/
/**
 * The standard constructor for this struct
 *
 * @param f The function to be executed by the thread
 */
GThreadWrapper::GThreadWrapper(boost::function<void()> f)
	:f_(f)
{ /* nothing */	}

/**********************************************************************************************/
/**
 * This is the main function that will be executed by the thread
 */
void GThreadWrapper::operator()() {
#ifdef DEBUG
	try{
		// Execute the actual worker task
		f_();
	}
	catch(Gem::Common::gemfony_error_condition& e) {
		std::ostringstream error;
		error << "In GThreadWrapper::operator(): Caught Gem::Common::gemfony_error_condition with message" << std::endl
		      << e.what() << std::endl;
		std::cerr << error.str();
		std::terminate();
	}
	catch(std::exception& e){
		std::ostringstream error;
		error << "In GThreadWrapper::operator(): Caught std::exception with message" << std::endl
		      << e.what() << std::endl;
		std::cerr << error.str();
		std::terminate();
	}
	catch(boost::exception& e){
		std::ostringstream error;
		error << "In GThreadWrapper::operator(): Caught boost::exception with message" << std::endl;
		std::cerr << error.str();
		std::terminate();
	}
	catch(...){
		std::ostringstream error;
		error << "GThreadWrapper::operator(): Caught unknown exception" << std::endl;
		std::cerr << error.str();
		std::terminate();
	}
#else /* DEBUG */
	f_(); // Simply execute the worker task without catching any exceptions
#endif /* DEBUG */
}

/**********************************************************************************************/

} /* namespace Common */
} /* namespace Gem */
