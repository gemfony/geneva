/**
 * @file GEvaluator.hpp
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
#include <vector>

// Includes check for correct Boost version(s)
#include "common/GGlobalDefines.hpp"

// Boost headrs go here
#include <boost/cstdint.hpp>
#include <boost/shared_ptr.hpp>

#ifndef GEVALUATOR_HPP_
#define GEVALUATOR_HPP_

// For Microsoft-compatible compilers
#if defined(_MSC_VER)  &&  (_MSC_VER >= 1020)
#pragma once
#endif


// Geneva headers go here
#include "geneva/GObject.hpp"
#include "geneva/GEvaluator.hpp"
#include "geneva/GAdaptorT.hpp"
#include "common/GExceptions.hpp"

namespace Gem {
namespace Geneva {

// Forward declaration
class GParameterSet;

/******************************************************************************************/
/**
 * This class can be assigned to a GEvaluatorIndividual object. Its purpose is to perform
 * the definition of parameters and their evaluation in a single place. The user derives a class
 * from GEvaluator, in which he defines the parameter types and possible boundary conditions.
 * The GEvaluatorIndividual then uses that information to set up the appropriate data structures.
 * Upon evaluation, the user is presented with vectors of the value types he has chosen, in
 * the sequence he has defined them.
 */
class GEvaluator
	:public GObject
{
	friend class GParameterSet;

	///////////////////////////////////////////////////////////////////////
	friend class boost::serialization::access;

	template<typename Archive>
	void serialize(Archive & ar, const unsigned int){
	  using boost::serialization::make_nvp;
	  ar & BOOST_SERIALIZATION_BASE_OBJECT_NVP(GObject);
	}
	///////////////////////////////////////////////////////////////////////

public:
	/**************************************************************************************/
	/** @brief The default constructor */
	GEvaluator();
	/** @brief A copy constructor */
	GEvaluator(const GEvaluator&);
	/** @brief The destructor */
	~GEvaluator();

	/** @brief Standard assignment operator */
	const GEvaluator& operator=(const GEvaluator&);

	/** @brief Checks for equality with another GEvaluator object */
	bool operator==(const GEvaluator&) const;
	/** @brief Checks for inequality with another GEvaluator object */
	bool operator!=(const GEvaluator&) const;

	/** @brief Checks whether this object fulfills a given expectation in relation to another object */
	virtual boost::optional<std::string> checkRelationshipWith(
			const GObject&
			, const Gem::Common::expectation&
			, const double&
			, const std::string&
			, const std::string&
			, const bool&
	) const;

protected:
	/**************************************************************************************/
	/** @brief Loads the data of another GObject */
	virtual void load_(const GObject*);
	/** @brief Creates a deep clone of this object */
	virtual GObject* clone_() const;

	/**************************************************************************************/
	/** @brief User-supplied set up of parameters and adaptors */
	virtual void parameterSetup() = 0;
	/** @brief User-supplied fitness calculation */
	virtual double fitnessCalculation() = 0;

	/**************************************************************************************/
	/**
	 * Adding a single, randomly initialized floating point variable. This function is a
	 * trap -- see the specializations for the actual implementation.
	 */
	template <typename fp_type>
	void registerFPParameter() {
		std::ostringstream error;
		error << "In GEvaluator::registerFPParameter(): Function called with unknown type." << std::endl;
		throw(Gem::Common::gemfony_error_condition(error.str()));
	}

	/**************************************************************************************/
	/**
	 * Adding nPars randomly initialized floating point variable. This function is a
	 * trap -- see the specializations for the actual implementation.
	 */
	template <typename fp_type>
	void registerFPParameter(const std::size_t& nPars) {
		std::ostringstream error;
		error << "In GEvaluator::registerFPParameter(): Function called with unknown type." << std::endl;
		throw(Gem::Common::gemfony_error_condition(error.str()));
	}

	/**************************************************************************************/
	/**
	 * Adding nPars floating point variable with defined start value. This function is a
	 * trap -- see the specializations for the actual implementation.
	 */
	template <typename fp_type>
	void registerFPParameter(
			const fp_type& val
			, const std::size_t& nPars
	)  {
		std::ostringstream error;
		error << "In GEvaluator::registerFPParameter(): Function called with unknown type." << std::endl;
		throw(Gem::Common::gemfony_error_condition(error.str()));
	}

	/**************************************************************************************/
	/**
	 * Adding nPars randomly initialized floating point variable with boundaries. This
	 * function is a trap -- see the specializations for the actual implementation.
	 */
	template <typename fp_type>
	void registerFPParameter(
			const fp_type& lowerBoundary
			, const fp_type& upperBoundary
			, const std::size_t& nPars
	) {
		std::ostringstream error;
		error << "In GEvaluator::registerFPParameter(): Function called with unknown type." << std::endl;
		throw(Gem::Common::gemfony_error_condition(error.str()));
	}

	/**************************************************************************************/
	/**
	 * Adding nPars randomly initialized floating point variable with boundaries and
	 * defined start value. This function is a trap -- see the specializations for the
	 * actual implementation.
	 */
	template <typename fp_type>
	void registerFPParameter(
			const fp_type& val
			, const fp_type& lowerBoundary
			, const fp_type& upperBoundary
			, const std::size_t& nPars
	) {
		std::ostringstream error;
		error << "In GEvaluator::registerFPParameter(): Function called with unknown type." << std::endl;
		throw(Gem::Common::gemfony_error_condition(error.str()));
	}

	/**************************************************************************************/
	/**
	 * Adding a single, randomly initialized integer variable. This function is a trap --
	 * see the specializations for the actual implementation.
	 */
	template <typename int_type>
	void registerIntParameter() {
		std::ostringstream error;
		error << "In GEvaluator::registerIntParameter(): Function called with unknown type." << std::endl;
		throw(Gem::Common::gemfony_error_condition(error.str()));
	}

	/**************************************************************************************/
	/**
	 * Adding nPars randomly initialized integer variable. This function is a trap --
	 * see the specializations for the actual implementation.
	 */
	template <typename int_type>
	void registerIntParameter(const std::size_t& nPars) {
		std::ostringstream error;
		error << "In GEvaluator::registerIntParameter(): Function called with unknown type." << std::endl;
		throw(Gem::Common::gemfony_error_condition(error.str()));
	}

	/**************************************************************************************/
	/**
	 * Adding nPars integer variable with defined start value. This function is a trap --
	 * see the specializations for the actual implementation.
	 */
	template <typename int_type>
	void registerIntParameter(
			int_type& val
			, const std::size_t& nPars
	) {
		std::ostringstream error;
		error << "In GEvaluator::registerIntParameter(): Function called with unknown type." << std::endl;
		throw(Gem::Common::gemfony_error_condition(error.str()));
	}

	/**************************************************************************************/
	/**
	 * Adding nPars randomly initialized integer variable with boundaries. This function
	 * is a trap -- see the specializations for the actual implementation.
	 */
	template <typename int_type>
	void registerIntParameter(
			const int_type& lowerBoundary
			, const int_type& upperBoundary
			, const std::size_t& nPars
	) {
		std::ostringstream error;
		error << "In GEvaluator::registerIntParameter(): Function called with unknown type." << std::endl;
		throw(Gem::Common::gemfony_error_condition(error.str()));
	}

	/**************************************************************************************/
	/**
	 * Adding nPars randomly initialized integer variable with boundaries and
	 * defined start value. This function is a trap -- see the specializations for
	 * the actual implementation.
	 */
	template <typename int_type>
	void registerIntParameter(
			const int_type& val
			, const int_type& lowerBoundary
			, const int_type& upperBoundary
			, const std::size_t& nPars
	) {
		std::ostringstream error;
		error << "In GEvaluator::registerIntParameter(): Function called with unknown type." << std::endl;
		throw(Gem::Common::gemfony_error_condition(error.str()));
	}

	/**************************************************************************************/
	/**
	 * Sets the lower and upper boundaries for random numbers in random initialization.
	 * This function is a trap -- see the specializations for the actual implementation.
	 */
	template <typename num_type>
	void setInitBoundaries (
			const num_type& lower
			, const num_type& upper
	) {
		std::ostringstream error;
		error << "In GEvaluator::setInitBoundaries(): Function called with unknown type." << std::endl;
		throw(Gem::Common::gemfony_error_condition(error.str()));
	}

	/**************************************************************************************/
	/**
	 * Retrieves the lower boundary for random numbers of a particular type.
	 * This function is a trap -- see the specializations for the actual implementation.
	 */
	template <typename num_type>
	num_type getLowerInitBoundary() const {
		std::ostringstream error;
		error << "In GEvaluator::getLowerInitBoundary(): Function called with unknown type." << std::endl;
		throw(Gem::Common::gemfony_error_condition(error.str()));
	}

	/**************************************************************************************/
	/**
	 * Retrieves the upper boundary for random numbers of a particular type.
	 * This function is a trap -- see the specializations for the actual implementation.
	 */
	template <typename num_type>
	num_type getUpperInitBoundary() const {
		std::ostringstream error;
		error << "In GEvaluator::getUpperInitBoundary(): Function called with unknown type." << std::endl;
		throw(Gem::Common::gemfony_error_condition(error.str()));
	}

	/**************************************************************************************/
	/** @brief Adding a single, randomly initialized boolean variable. */
	void registerBooleanParameter();
	/** @brief Adding nPars randomly initialized boolean variable. */
	void registerBooleanParameter(const std::size_t&);
	/** @brief Adding nPars boolean variable with defined start value */
	void registerBooleanParameter(const bool&, const std::size_t&);

	/**************************************************************************************/
	// Parameter values, to be updated prior to the fitness calculation
	std::vector<double> double_values; ///< Holds double values ready for consumption by the fitness function
	std::vector<boost::int32_t> int32_values; ///< Holds boost::int32_t values ready for consumption by the fitness function
	std::vector<bool> boolean_values; ///< Holds boolean values ready for consumption by the fitness function

private:
	/**************************************************************************************/
	/** @brief Addition of parameters to the individual */
	void parameterSetup_(boost::shared_ptr<GParameterSet>&);

	/**************************************************************************************/
	/**
	 * An enumeration type used to store the parameter type added to the individual
	 */
	enum evaluator_parameter_type {
		GBOOLEANOBJECT = 0
		, GINT32OBJECT = 1
		, GDOUBLEOBJECT = 2
		, GCONSTRAINEDINT32OBJECT = 3
		, GCONSTRAINEDDOUBLEOBJECT = 4
		, GBOOLEANOBJECTCOLLECTION = 5
		, GINT32OBJECTCOLLECTION = 6
		, GDOUBLEOBJECTCOLLECTION = 7
		, GCONSTRAINEDINT32OBJECTCOLLECTION = 8
		, GCONSTRAINEDDOUBLEOBJECTCOLLECTION = 9
		, GBOOLEANCOLLECTION = 10
		, GINT32COLLECTION = 11
		, GDOUBLECOLLECTION = 12
		, GCONSTRAINEDINT32COLLECTION = 13
		, GCONSTRAINEDDOUBLECOLLECTION = 14
	};

	/**************************************************************************************/
	/**
	 * A private struct used to store information about parameters, as defined by the user
	 */
	template <typename num_type>
	struct parameter_info {
		num_type value; ///< The value assigned to the parameter (if defined)
		bool value_initialized; ///< Specifies whether "value" should be used for the initialization of the parameter
		num_type lowerBoundary; ///< The lower boundary (if any)
		num_type upperBoundary; ///< The upper boundary (if any)
		bool has_boundaries; ///< Indicates whether boundaries have been specified
		evaluator_parameter_type par_type; ///< Indicates the type of parameter used to store the values
		std::site_t nParameters; ///< The amount of parameters in the parameter type
		std::size_t position; ///< The position in the individual, where the corresponding parameter is stored (relative to the start of parameters of this particular type)
		std::size_t adaptorId; ///< The location of the adaptor in the corresponding array
	};

	/**************************************************************************************/
	// Information about parameters
	std::vector<parameter_info<double> > double_parameter_info; ///< A collection of parameter_info objects for double parameters
	std::vector<parameter_info<boost::int32_t> > int32_parameter_info; ///< A collection of parameter_info objects for boost::int32_t parameters
	std::vector<parameter_info<bool> > boolean_parameter_info; ///< A collection of parameter_info objects for boolean parameters

	/**************************************************************************************/
	// Templates for adaptors (if needed)
	std::vector<boost::shared_ptr<GAdaptorT<double>  > > double_adaptors_;
	std::vector<boost::shared_ptr<GAdaptorT<boost::int32_t>  > > int32_adaptors_;
	std::vector<boost::shared_ptr<GAdaptorT<bool>  > > boolean_adaptors_;

	/**************************************************************************************/
	// Initialization boundaries for some allowed types
	boost::int32_t lowerIntInitBoundary, upperIntInitBoundary;
	double lowerDoubleInitBoundary, upperDoubleInitBoundary;

#ifdef GENEVATESTING
	/**************************************************************************************/
public:
	/** @brief Applies modifications to this object. This is needed for testing purposes */
	virtual bool modify_GUnitTests();
	/** @brief Performs self tests that are expected to succeed. This is needed for testing purposes */
	virtual void specificTestsNoFailureExpected_GUnitTests();
	/** @brief Performs self tests that are expected to fail. This is needed for testing purposes */
	virtual void specificTestsFailuresExpected_GUnitTests();
#endif /* GENEVATESTING */
};

/******************************************************************************************/
// Specializations for particular types

template <>	void GEvaluator::registerFPParameter<double>();
template <>	void GEvaluator::registerFPParameter<double>(const std::size_t&);
template <>	void GEvaluator::registerFPParameter<double>(const std::size_t&, const double&);
template <>	void GEvaluator::registerFPParameter<double>(const std::size_t&, const double&, const double&);
template <>	void GEvaluator::registerFPParameter<double>(const std::size_t&, const double&, const double&, const double&);

template <> void   GEvaluator::setInitBoundaries<double>(const double&, const double&);
template <> double GEvaluator::getLowerInitBoundary<double>() const;
template <> double GEvaluator::getUpperInitBoundary<double>() const;

template <> void GEvaluator::getFPValues<double>(std::vector<fp_type>&) const;

template <>	void GEvaluator::registerIntParameter<boost::int32_t>();
template <>	void GEvaluator::registerIntParameter<boost::int32_t>(const std::size_t&);
template <>	void GEvaluator::registerIntParameter<boost::int32_t>(const std::size_t&, const boost::int32_t&);
template <>	void GEvaluator::registerIntParameter<boost::int32_t>(const std::size_t&, const boost::int32_t&, const boost::int32_t&);
template <>	void GEvaluator::registerIntParameter<boost::int32_t>(const std::size_t&, const boost::int32_t&, const boost::int32_t&, const boost::int32_t&);

template <> void   GEvaluator::setInitBoundaries<boost::int32_t>(const boost::int32_t&, const boost::int32_t&);
template <> boost::int32_t GEvaluator::getLowerInitBoundary<boost::int32_t>() const;
template <> boost::int32_t GEvaluator::getUpperInitBoundary<boost::int32_t>() const;

template <> void GEvaluator::getIntValues<boost::int32_t>(std::vector<int_type>&) const;

/******************************************************************************************/
/**
 * @brief Needed for Boost.Serialization
 */
BOOST_SERIALIZATION_ASSUME_ABSTRACT(Gem::Geneva::GEvaluator)

/******************************************************************************************/

} /* namespace Geneva */
} /* namespace Gem */

#ifdef GENEVATESTING
// Tests of this class (and parent classes)
/*************************************************************************************************/
///////////////////////////////////////////////////////////////////////////////////////////////////
/*************************************************************************************************/
/** @brief We need to provide a specialization of the factory function that creates objects of this type. */
template <> boost::shared_ptr<Gem::Geneva::GEvaluator> TFactory_GUnitTests<Gem::Geneva::GEvaluator>();

/*************************************************************************************************/
///////////////////////////////////////////////////////////////////////////////////////////////////
/*************************************************************************************************/
#endif /* GENEVATESTING */


#endif /* GEVALUATOR_HPP_ */
