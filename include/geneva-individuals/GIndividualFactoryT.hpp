/**
 * @file GIndividualFactoryT.hpp
 */

/*
 * Copyright (C) Authors of the Geneva library collection and Karlsruhe
 * Institute of Technology (University of the State of Baden-Wuerttemberg
 * and National Laboratory of the Helmholtz Association).
 *
 * See the AUTHORS file in the top-level directory for a list of authors.
 *
 * Contact: info [at] gemfony (dot) com
 *
 * This file is part of the Geneva library collection
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
#include <string>

// Boost header files go here
#include <boost/shared_ptr.hpp>

#ifndef GINDIVIDUALFACTORYT_HPP_
#define GINDIVIDUALFACTORYT_HPP_

// For Microsoft-compatible compilers
#if defined(_MSC_VER)  &&  (_MSC_VER >= 1020)
#pragma once
#endif

// Geneva header files go here
#include <geneva/GParameterSet.hpp>
#include <common/GParserBuilder.hpp>
#include <common/GExceptions.hpp>

namespace Gem
{
namespace Geneva
{

/*******************************************************************************************/
/**
 * A factory function that returns GParameterSet-derivatives of type ind_type . These are
 * constructed according to specifications read from a configuration file. The actual work
 * needs to be done in functions that are written independently for each individual separately.
 * This function is a trap to catch cases, where no independent factory function was created.
 *
 * @param cf The name of a configuration file holding information about individuals of type ind_type
 * @return An individual of type ind_type, constructed according to the information read from cf
 */
template <typename ind_type>
boost::shared_ptr<GParameterSet> GIndividualFactory(const std::string& cf)
{
	std::ostringstream error;
	error << "In GIndividualFactory<ind_type>(const std::string&): Error!" << std::endl
		  << "No specialization provided." << std::endl;
	throw(Gem::Common::gemfony_error_condition(error.str()));
}

} /* namespace Geneva */
} /* namespace Gem */

#endif /* GINDIVIDUALFACTORYT_HPP_ */
