/**
 * @file
 */

/* GenevaExceptions.hpp
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

#include <string>

// Boost header files go here

#include <boost/version.hpp>

#if BOOST_VERSION < 103600
#error "Error: Boost should at least have version 1.36 !"
#endif /* BOOST_VERSION */

#include <boost/exception.hpp>

#ifndef GENEVAEXCEPTIONS_HPP_
#define GENEVAEXCEPTIONS_HPP_

// Geneva header files go here

namespace Gem {
namespace GenEvA {

/**************************************************************************************************/
// Exceptions and related definitions

/** @brief String container for error messages */
typedef boost::error_info<struct tag_errno, std::string> error_string;

// GBasePopulation
/** @brief Class to be thrown as an error if the popSize_ or nParents_ parameters
 * haven't been set */
class geneva_popsize_not_set : public boost::exception {};
/** @brief Class to be thrown as an error if the popSize_ parameter is too small */
class geneva_popsize_too_small : public boost::exception {};
/** @brief Class to be thrown as an error if a smart pointer was found to be empty in GBasePopulation */
class geneva_empty_smart_pointer : public boost::exception {};
/** @brief Class to be thrown as an error if an invalid recombination mode was found */
class geneva_invalid_recombination_mode : public boost::exception {};
/** @brief Class to be thrown as an error if no recombination took place in the valueRecombine function */
class geneva_no_value_recombination : public boost::exception {};
/** @brief Class to be thrown as an error if too few children were found */
class geneva_too_few_children : public boost::exception {};
/** @brief Class to be thrown as an error if a dirty individual was found after the optimize call in
 * GBasePopulation::fitnessCalculation() */
class geneva_dirty_individual : public boost::exception {};

// GBoundedBuffer
/** @brief Class to be thrown as a message in the case of a time-out in GBuffer */
class geneva_condition_time_out: public boost::exception {};

// GIndividual
/** @brief Class to be thrown as an error if the dirty flag is set while lazy evaluation is not allowed */
class geneva_dirtyflag_set_lazyevaluation_not : public boost::exception {};

// GParameterBaseWithAdaptorsT
/** @brief Class to be thrown as an error if an adaptor with the same name is already present */
class geneva_duplicate_adaptor : public boost::exception {};

// GParameterSet
/** @brief Class to be thrown as an error if an empty evaluation function was supplied */
class geneva_empty_evaluation_function : public boost::exception {};

/** @brief Class to be thrown as an error if no or not the required adaptor was found */
class geneva_no_adaptor_found : public boost::exception {};

// Several classes
/** @brief Class to be thrown as an error by GenEvA's load functions */
class geneva_object_assigned_to_itself :public boost::exception {};
/** @brief Class to be thrown as an error in case of conversion errors */
class geneva_dynamic_cast_conversion_error :public boost::exception {};
/** @brief Class to be thrown as an error if an evaluation is attempted while no evaluation function
 * object has been registered. */
class geneva_evaluation_function_not_present : public boost::exception {};

/**************************************************************************************************/

}} /* namespace Gem::GenEvA */

#endif /* GENEVAEXCEPTIONS_HPP_ */
