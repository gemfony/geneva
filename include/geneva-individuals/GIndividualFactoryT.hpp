/**
 * @file GIndividualFactoryT.hpp
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

// Standard header files go here
#include <iostream>
#include <sstream>
#include <string>

// Boost header files go here
#include <boost/shared_ptr.hpp>
#include <boost/utility.hpp>

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
 * A factory class that returns GParameterSet-derivatives of type ind_type . These are
 * constructed according to specifications read from a configuration file. The actual work
 * needs to be done in functions that are implemented in derived classes for each individual
 * independently.
 */
template <typename ind_type>
class GIndividualFactoryT
	:private boost::noncopyable
{
public:
	/***************************************************************************************/
	/**
	 * The standard constructor
	 *
	 * @param configFile The name of a configuration file holding information about individuals of type ind_type
	 */
	GIndividualFactoryT(const std::string& configFile)
		: configFile_(configFile)
		, id_(std::size_t(0))
		, initialized_(false)
		, finalized_(false)
		, gpb()
	{ /* nothing */ }

	/***************************************************************************************/
	/**
	 * The destructor
	 */
	virtual ~GIndividualFactoryT()
	{ /* nothing */ }

	/***************************************************************************************/
	/**
	 * Triggers the creation of objects of the desired type
	 *
	 * @return An individual of the desired type
	 */
	boost::shared_ptr<ind_type> operator()() {
#ifdef DEBUG
		// It is an error if this function is called on a finalized object
		if(finalized_) {
			raiseException(
					"In GIndividualFactoryT<ind_type>::operator()(): Error!" << std::endl
					<< "Tried to retrieve individual when object has already been finalized!"
			);
		}
#endif /* DEBUG */

		this->init(); // This function will do nothing when called more than once
		return this->getIndividual_(id_++); // Retrieve the actual individual
	}

	/***************************************************************************************/
	/**
	 * Performs necessary initialization work. This function will do nothing when
	 * called more than once.
	 */
	virtual void init() {
		if(!initialized_) {
			// It is an error if this function is called on a finalized object
			if(finalized_) {
				raiseException(
						"In GIndividualFactoryT<ind_type>::init(): Error!" << std::endl
						<< "Tried to initialize object which has already been finalized"
				);
			}

			// Execute the user-defined configuration specifications
			this->describeConfigurationOptions_();

			// Read the configuration parameters from file
			if(!gpb.parseConfigFile(configFile_)) {
				raiseException(
						"In GIndividualFactoryT<ind_type>::init(): Error!" << std::endl
						<< "Could not parse configuration file " << configFile_
				);
			}

			// Perform the user-defined initialization work
			this->init_();

			initialized_ = true;
		}
	}

	/***************************************************************************************/
	/**
	 * Performs any required finalization work. This function does nothing when called more
	 * than once.
	 */
	virtual void finalize(){
		// The object should always have been initialized before the finalize() function is called
		if(!initialized_) {
			raiseException(
					"In GIndividualFactoryT<ind_type>::finalize(): Error!" << std::endl
					<< "Function called on un-initialized object"
			);
		}

		if(!finalized_) {
			this->finalize_();
			finalized_ = true;
		}
	}

protected:
	/***************************************************************************************/
	/** @brief Performs necessary initialization work */
	virtual void init_() {}
	/** @brief Performs any required finalization work */
	virtual void finalize_(){}
	/** @brief Allows to describe configuration options in derived classes */
	virtual void describeConfigurationOptions_() = 0;
	/** @brief Creates individuals of the desired type */
	virtual boost::shared_ptr<ind_type> getIndividual_(const std::size_t&) = 0;

private:
	/***************************************************************************************/
	GIndividualFactoryT(); ///< The default constructor. Intentionally private and undefined

	std::string configFile_; ///< The name of the configuration file
	std::size_t id_; ///< The id/number of the individual currently being created
	bool initialized_; ///< Indicates whether the configuration file has already been parsed
	bool finalized_;

protected:
	/***************************************************************************************/
	Gem::Common::GParserBuilder gpb; ///< The parser who reads data from the configuration file
};

/*******************************************************************************************/

} /* namespace Geneva */
} /* namespace Gem */

#endif /* GINDIVIDUALFACTORYT_HPP_ */
