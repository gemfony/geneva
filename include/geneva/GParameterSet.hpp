/**
 * @file GParameterSet.hpp
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

// Boost header files go here

#ifndef GPARAMETERSET_HPP_
#define GPARAMETERSET_HPP_

// For Microsoft-compatible compilers
#if defined(_MSC_VER)  &&  (_MSC_VER >= 1020)
#pragma once
#endif


// Geneva headers go here
#include "common/GExceptions.hpp"
#include "common/GLogger.hpp"
#include "geneva/GObject.hpp"
#include "geneva/GMutableSetT.hpp"
#include "geneva/GParameterBase.hpp"

// TODO: Remove this dependency
#include "geneva/GEAPersonalityTraits.hpp"
#include "geneva/GSAPersonalityTraits.hpp"

#ifdef GEM_TESTING
#include "geneva/GBooleanObject.hpp"
#include "geneva/GConstrainedInt32Object.hpp"
#include "geneva/GConstrainedDoubleObject.hpp"
#include "geneva/GConstrainedDoubleObjectCollection.hpp"
#include "geneva/GDoubleCollection.hpp"
#include "geneva/GDoubleObject.hpp"
#include "geneva/GInt32Collection.hpp"
#include "common/GUnitTestFrameworkT.hpp"
#include "hap/GRandomT.hpp"
#endif /* GEM_TESTING */

namespace Gem {

namespace Tests {
class GTestIndividual1; // forward declaration, needed for testing purposes
} /* namespace Tests */

namespace Geneva {

/******************************************************************************/
/**
 * This class implements a collection of GParameterBase objects. It
 * will form the basis of many user-defined individuals.
 */
class GParameterSet:
	public GMutableSetT<Gem::Geneva::GParameterBase>
{
	friend class Gem::Tests::GTestIndividual1; ///< Needed for testing purposes

	///////////////////////////////////////////////////////////////////////
	friend class boost::serialization::access;

	template<typename Archive>
	void serialize(Archive & ar, const unsigned int){
	  using boost::serialization::make_nvp;
	  ar & make_nvp("GMutableSetT_GParameterBase",
			  boost::serialization::base_object<GMutableSetT<Gem::Geneva::GParameterBase> >(*this));
	}
	///////////////////////////////////////////////////////////////////////

public:
	/** @brief The default constructor */
	GParameterSet();
	/** @brief The copy constructor */
	GParameterSet(const GParameterSet&);
	/** @brief The destructor */
	virtual ~GParameterSet();

	/** @brief Standard assignment operator */
	const GParameterSet& operator=(const GParameterSet&);

	/** @brief Checks for equality with another GParameterSet object */
	bool operator==(const GParameterSet&) const;
	/** @brief Checks for inequality with another GParameterSet object */
	bool operator!=(const GParameterSet&) const;

	/** @brief Checks whether this object fulfills a given expectation in relation to another object */
	virtual boost::optional<std::string> checkRelationshipWith(
			const GObject&
			, const Gem::Common::expectation&
			, const double&
			, const std::string&
			, const std::string&
			, const bool&
	) const;

	/** @brief Triggers updates when the optimization process has stalled */
	virtual bool updateOnStall();

	/** @brief Allows to randomly initialize parameter members */
	virtual void randomInit();

	/** @brief Performs a cross-over with another GParameterSet object */
	void crossOver(GParameterSet&, const double&);

	/** @brief Specify whether we want to work in maximization (true) or minimization (false) mode */
	void setMaxMode(const bool&);

	/** @brief Multiplies with a random floating point number in a given range */
	void fpMultiplyByRandom(const float&, const float&);
	/** @brief Multiplies with a random floating point number in the range [0, 1[ */
	void fpMultiplyByRandom();

	/** @brief Multiplies floating-point parameters with a given value */
	void fpMultiplyBy(const float& val);

	/** @brief Initializes floating-point parameters with a given value */
	void fpFixedValueInit(const float&);

	/** @brief Adds the floating point parameters of another GParameterSet object to this one */
	void fpAdd(boost::shared_ptr<GParameterSet>);
	/** @brief Subtracts the floating point parameters of another GParameterSet object from this one */
	void fpSubtract(boost::shared_ptr<GParameterSet>);

	/** @brief Emits a GParameterSet object that only has the GParameterBase objects attached to it */
	boost::shared_ptr<GParameterSet> parameter_clone() const;

	/** @brief Updates the random number generators contained in this object's GParameterBase-derivatives */
	virtual void updateRNGs();
	/** @brief Restores the local random number generators contained in this object's GParameterBase-derivatives */
	virtual void restoreRNGs();
	/** @brief Checks whether all GParameterBase derivatives use local random number generators */
	virtual bool localRNGsUsed() const;
	/** @brief Checks whether all GParameterBase derivatives use the assigned random number generator */
	virtual bool assignedRNGUsed() const;

	/** @brief Adds local configuration options to a GParserBuilder object */
	virtual void addConfigurationOptions(Gem::Common::GParserBuilder&, const bool&);

	/** @brief Allows to assign a name to the role of this individual(-derivative) */
	virtual std::string getIndividualCharacteristic() const;

	/** @brief Provides access to all data stored in the individual in a user defined selection */
	virtual void custom_streamline(std::vector<boost::any>&);

	/** @brief Transformation of the individual's parameter objects into a boost::property_tree object */
	void toPropertyTree(pt::ptree&, const std::string& = "parameterset") const;
	/** @brief Transformation of the individual's parameter objects into a list of comma-separated values */
	std::string toCSV() const;

	/** @brief Emits a name for this class / object */
	virtual std::string name() const;

	/** @brief Prevent shadowing of std::vector<GParameterBase>::at() */
	boost::shared_ptr<Gem::Geneva::GParameterBase> at(const std::size_t& pos);

	/***************************************************************************/
	/**
	 * This function returns a parameter set at a given position of the data set.
	 * Note that this function will only be accessible to the compiler if par_type
	 * is a derivative of GParameterBase, thanks to the magic of Boost's enable_if and
	 * Type Traits libraries.
	 *
	 * @param pos The position in our data array that shall be converted
	 * @return A converted version of the GParameterBase object, as required by the user
	 */
	template <typename par_type>
	inline const boost::shared_ptr<par_type> at(
			  const std::size_t& pos
			, typename boost::enable_if<boost::is_base_of<GParameterBase, par_type> >::type* dummy = 0
	)  const {
      // Does error checks on the conversion internally
      return Gem::Common::convertSmartPointer<GParameterBase, par_type>(data.at(pos));
	}

	/* ----------------------------------------------------------------------------------
	 * So far untested. See also the second version of the at() function.
	 * ----------------------------------------------------------------------------------
	 */

	/***************************************************************************/
	/**
	 * Retrieve information about the total number of parameters of type
	 * par_type in the individual. Note that the GParameterBase-template
	 * function will throw if this function is called for an unsupported type.
	 */
	template <typename par_type>
	std::size_t countParameters() const {
		std::size_t result = 0;

		// Loop over all GParameterBase objects. Each object
		// will contribute the amount of its parameters of this type
		// to the result.
		GParameterSet::const_iterator cit;
		for(cit=this->begin(); cit!=this->end(); ++cit) {
			result += (*cit)->countParameters<par_type>();
		}

		return result;
	}

	/* ----------------------------------------------------------------------------------
	 * So far untested.
	 * ----------------------------------------------------------------------------------
	 */

	/***************************************************************************/
	/**
	 * Loops over all GParameterBase objects. Each object will add the
	 * values of its parameters to the vector, if they comply with the
	 * type of the parameters to be stored in the vector.
	 *
	 * @param parVec The vector to which the parameters will be added
	 */
	template <typename par_type>
	void streamline(std::vector<par_type>& parVec) const {
		// Make sure the vector is clean
		parVec.clear();

		// Loop over all GParameterBase objects.
		GParameterSet::const_iterator cit;
		for(cit=this->begin(); cit!=this->end(); ++cit) {
			(*cit)->streamline<par_type>(parVec);
		}
	}

	/* ----------------------------------------------------------------------------------
	 * So far untested.
	 * ----------------------------------------------------------------------------------
	 */

	/***************************************************************************/
	/**
	 * Loops over all GParameterBase objects. Each object will add the
	 * lower and upper boundaries of its parameters to the vector, if
	 * they comply with the type of the parameters to be stored in the
	 * vector.
	 *
	 * @param lBndVec The vector to which the lower boundaries will be added
	 * @param uBndVec The vector to which the upper boundaries will be added
	 */
	template <typename par_type>
	void boundaries(
			std::vector<par_type>& lBndVec
			, std::vector<par_type>& uBndVec
	) const {
		// Make sure the vectors are clean
		lBndVec.clear();
		uBndVec.clear();

		// Loop over all GParameterBase objects.
		GParameterSet::const_iterator cit;
		for(cit=this->begin(); cit!=this->end(); ++cit) {
			(*cit)->boundaries<par_type>(lBndVec, uBndVec);
		}
	}

	/* ----------------------------------------------------------------------------------
	 * So far untested.
	 * ----------------------------------------------------------------------------------
	 */

	/***************************************************************************/
	/**
	 * Assigns values from a std::vector to the parameters in the collection
	 *
	 * @param parVec A vector of values, to be assigned to be added to GParameterBase derivatives
	 */
	template <typename par_type>
	void assignValueVector(const std::vector<par_type>& parVec) {
#ifdef DEBUG
		if(countParameters<par_type>() != parVec.size()) {
		   glogger
		   << "In GParameterSet::assignValueVector(const std::vector<pat_type>&):" << std::endl
         << "Sizes don't match: " <<  countParameters<par_type>() << " / " << parVec.size() << std::endl
         << GEXCEPTION;
		}
#endif /* DEBUG */

		// Start assignment at the beginning of parVec
		std::size_t pos = 0;

		// Loop over all GParameterBase objects. Each object will extract the relevant
		// parameters and increment the position counter as required.
		GParameterSet::const_iterator cit;
		for(cit=this->begin(); cit!=this->end(); ++cit) {
			(*cit)->assignValueVector<par_type>(parVec, pos);
		}

		// As we have modified our internal data sets, make sure the dirty flag is set
		GIndividual::setDirtyFlag();
	}

protected:
	/***************************************************************************/
	/** @brief Loads the data of another GObject */
	virtual void load_(const GObject*);
	/** @brief Creates a deep clone of this object */
	virtual GObject* clone_() const;

	/** @brief The actual fitness calculation takes place here */
	virtual double fitnessCalculation();
	/* @brief The actual adaption operations. */
	virtual void customAdaptions();

private:
	explicit GParameterSet(const float&); ///< Intentionally private and undefined

	/***************************************************************************/
public:
	/** @brief Applies modifications to this object. This is needed for testing purposes */
	virtual bool modify_GUnitTests();
	/** @brief Performs self tests that are expected to succeed. This is needed for testing purposes */
	virtual void specificTestsNoFailureExpected_GUnitTests();
	/** @brief Performs self tests that are expected to fail. This is needed for testing purposes */
	virtual void specificTestsFailuresExpected_GUnitTests();
	/***************************************************************************/
};

} /* namespace Geneva */
} /* namespace Gem */

/******************************************************************************/
/**
 * @brief Needed for Boost.Serialization
 */
BOOST_CLASS_EXPORT_KEY(Gem::Geneva::GParameterSet)

/******************************************************************************/

#endif /* GPARAMETERSET_HPP_ */
