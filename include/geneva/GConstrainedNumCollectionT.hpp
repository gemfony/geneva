/**
 * @file GConstrainedNumCollectionT.hpp
 */

/*
 * Copyright (C) Gemfony scientific UG (haftungsbeschraenkt)
 *
 * See the AUTHORS file in the top-level directory for a list of authors.
 *
 * Contact: contact [at] gemfony (dot) eu
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
 * http://www.gemfony.eu .
 */

// Global checks, defines and includes needed for all of Geneva
#include "common/GGlobalDefines.hpp"

// Standard header files go here
#include <string>
#include <sstream>
#include <vector>

// Boost header files go here

#ifndef GCONSTRAINEDNUMCOLLECTIONT_HPP_
#define GCONSTRAINEDNUMCOLLECTIONT_HPP_

// Geneva header files go here
#include "common/GExceptions.hpp"
#include "common/GTypeToStringT.hpp"
#include "geneva/GDoubleGaussAdaptor.hpp"
#include "geneva/GObject.hpp"
#include "geneva/GParameterCollectionT.hpp"
#include "geneva/GConstrainedValueLimitT.hpp"

namespace Gem {
namespace Geneva {

/******************************************************************************/
/**
 * This class represents a collection of numeric values with common
 * boundaries, all modified using the same algorithm. The most likely types to
 * be stored in this class are double and std::int32_t . Note: If you want
 * to access or set the transformed value, use the value() and setValue()
 * functions. Using the subscript operator or at() function, or the
 * native iterator, will give you the "raw" data only.
 */
template <typename num_type>
class GConstrainedNumCollectionT
	: public GParameterCollectionT<num_type>
{
	///////////////////////////////////////////////////////////////////////
	friend class boost::serialization::access;

	template<typename Archive>
	void serialize(Archive & ar, const unsigned int) {
		using boost::serialization::make_nvp;
		ar
		& make_nvp("GParameterCollectionT",	boost::serialization::base_object<GParameterCollectionT<num_type>>(*this))
		& BOOST_SERIALIZATION_NVP(lowerBoundary_)
		& BOOST_SERIALIZATION_NVP(upperBoundary_);
	}
	///////////////////////////////////////////////////////////////////////

public:
	/** @brief Specifies the type of parameters stored in this collection */
	using collection_type = num_type;

	/***************************************************************************/
	/**
	 * Initialize the lower and upper boundaries for data members of this class
	 *
	 * @param size The desired size of the vector
	 * @param lowerBoundary The lower boundary of the value range
	 * @param upperBoundary The upper boundary of the value range
	 */
	GConstrainedNumCollectionT(
		const std::size_t size
		, const num_type& lowerBoundary
		, const num_type& upperBoundary
	)
		: GParameterCollectionT<num_type> (size, lowerBoundary)
		, lowerBoundary_(lowerBoundary)
		, upperBoundary_(upperBoundary)
	{
		// Naturally the upper boundary should be >= the lower boundary
		if(lowerBoundary_ > upperBoundary_) {
			glogger
			<< "In GConstrainedNumCollectionT<num_type>::GConstrainedNumCollectionT(size, lower,upper):" << std::endl
			<< "lowerBoundary_ = " << lowerBoundary_ << "is larger than" << std::endl
			<< "upperBoundary_ = " << upperBoundary_ << std::endl
			<< GTERMINATION;
		}

		// We might have constraints regarding the allowed boundaries. Cross-check
		if(lowerBoundary < GConstrainedValueLimitT<num_type>::lowest() || upperBoundary > GConstrainedValueLimitT<num_type>::highest()) {
			glogger
			<< "In GConstrainedNumCollectionT<num_type>::GConstrainedNumCollectionT(size, lower,upper):" << std::endl
			<< "lower and/or upper limit outside of allowed value range:" << std::endl
			<< "lowerBoundary = " << lowerBoundary << std::endl
			<< "upperBoundary = " << upperBoundary << std::endl
			<< "GConstrainedValueLimit<num_type>::lowest() = " << GConstrainedValueLimitT<num_type>::lowest() << std::endl
			<< "GConstrainedValueLimit<num_type>::highest() = " << GConstrainedValueLimitT<num_type>::highest()
			<< GTERMINATION;
		}
	}

	/***************************************************************************/
	/**
	 * Initialize the lower and upper boundaries for data members of this class
	 * and assign a fixed value to each position
	 *
	 * @param size The desired size of the vector
	 * @param val The value to be assigned to each position
	 * @param lowerBoundary The lower boundary of the value range
	 * @param upperBoundary The upper boundary of the value range
	 */
	GConstrainedNumCollectionT(
		const std::size_t size
		, const num_type& val
		, const num_type& lowerBoundary
		, const num_type& upperBoundary
	)
		: GParameterCollectionT<num_type> (size, val)
		, lowerBoundary_(lowerBoundary)
		, upperBoundary_(upperBoundary)
	{
		// Naturally the upper boundary should be > the lower boundary
		if(lowerBoundary_ > upperBoundary_) {
			glogger
			<< "In GConstrainedNumCollectionT<num_type>::GConstrainedNumCollectionT(size, val, lower,upper):" << std::endl
			<< "lowerBoundary_ = " << lowerBoundary_ << "is larger than" << std::endl
			<< "upperBoundary_ = " << upperBoundary_ << std::endl
			<< GTERMINATION;
		}

		// We might have constraints regarding the allowed boundaries. Cross-check
		if(lowerBoundary < GConstrainedValueLimitT<num_type>::lowest() || upperBoundary > GConstrainedValueLimitT<num_type>::highest()) {
			glogger
			<< "In GConstrainedNumCollectionT<num_type>::GConstrainedNumCollectionT(size, val, lower,upper):" << std::endl
			<< "lower and/or upper limit outside of allowed value range:" << std::endl
			<< "lowerBoundary = " << lowerBoundary << std::endl
			<< "upperBoundary = " << upperBoundary << std::endl
			<< "GConstrainedValueLimit<num_type>::lowest() = " << GConstrainedValueLimitT<num_type>::lowest() << std::endl
			<< "GConstrainedValueLimit<num_type>::highest() = " << GConstrainedValueLimitT<num_type>::highest() << std::endl
			<< GTERMINATION;
		}

		// Check that assigned value is in the allowed range
		if(val<lowerBoundary || val > upperBoundary) {
			glogger
			<< "In GConstrainedNumCollectionT<num_type>::GConstrainedNumCollectionT(size, val, lower,upper):" << std::endl
			<< "Assigned value is outside of allowed value range:" << std::endl
			<< "val = " << val << std::endl
			<< "lowerBoundary = " << lowerBoundary << std::endl
			<< "upperBoundary = " << upperBoundary << std::endl
			<< GTERMINATION;
		}
	}

	/***************************************************************************/
	/**
	 * The standard copy constructor. We assume that the boundaries have
	 * "legal" values. Thus we do not make any error checks here.
	 */
	GConstrainedNumCollectionT(const GConstrainedNumCollectionT<num_type>& cp)
		: GParameterCollectionT<num_type> (cp)
		, lowerBoundary_(cp.lowerBoundary_)
		, upperBoundary_(cp.upperBoundary_)
	{ /* nothing */ }

	/***************************************************************************/
	/**
	 * The standard destructor
	 */
	virtual ~GConstrainedNumCollectionT()
	{ /* nothing */ }

	/***************************************************************************/
	/**
	 * The standard assignment operator
	 */
	const GConstrainedNumCollectionT<num_type>& operator=(const GConstrainedNumCollectionT<num_type>& cp) {
		this->load_(&cp);
		return *this;
	}

	/***************************************************************************/
	/**
	 * Checks for equality with another GConstrainedNumCollectionT<num_type> object
	 *
	 * @param  cp A constant reference to another GConstrainedNumCollectionT<num_type> object
	 * @return A boolean indicating whether both objects are equal
	 */
	bool operator==(const GConstrainedNumCollectionT<num_type>& cp) const {
		using namespace Gem::Common;
		try {
			this->compare(cp, Gem::Common::expectation::CE_EQUALITY, CE_DEF_SIMILARITY_DIFFERENCE);
			return true;
		} catch(g_expectation_violation&) {
			return false;
		}
	}

	/***************************************************************************/
	/**
	 * Checks for inequality with another GConstrainedNumCollectionT<num_type> object
	 *
	 * @param  cp A constant reference to another GConstrainedNumCollectionT<num_type> object
	 * @return A boolean indicating whether both objects are inequal
	 */
	bool operator!=(const GConstrainedNumCollectionT<num_type>& cp) const {
		using namespace Gem::Common;
		try {
			this->compare(cp, Gem::Common::expectation::CE_INEQUALITY, CE_DEF_SIMILARITY_DIFFERENCE);
			return true;
		} catch(g_expectation_violation&) {
			return false;
		}
	}

	/***************************************************************************/
	/**
	 * Searches for compliance with expectations with respect to another object
	 * of the same type
	 *
	 * @param cp A constant reference to another GObject object
	 * @param e The expected outcome of the comparison
	 * @param limit The maximum deviation for floating point values (important for similarity checks)
	 */
	virtual void compare(
		const GObject& cp
		, const Gem::Common::expectation& e
		, const double& limit
	) const override {
		using namespace Gem::Common;

		// Check that we are dealing with a GConstrainedNumCollectionT<num_type> reference independent of this object and convert the pointer
		const GConstrainedNumCollectionT<num_type> *p_load = Gem::Common::g_convert_and_compare<GObject, GConstrainedNumCollectionT<num_type>>(cp, this);

		GToken token("GConstrainedNumCollectionT<num_type>", e);

		// Compare our parent data ...
		Gem::Common::compare_base<GParameterCollectionT<num_type>>(IDENTITY(*this, *p_load), token);

		// ... and then the local data
		compare_t(IDENTITY(lowerBoundary_, p_load->lowerBoundary_), token);
		compare_t(IDENTITY(upperBoundary_, p_load->upperBoundary_), token);

		// React on deviations from the expectation
		token.evaluate();
	}

	/***************************************************************************/
	/**
	  * Retrieves the lower boundary
	  *
	  * @return The value of the lower boundary
	  */
	num_type getLowerBoundary() const {
		return lowerBoundary_;
	}

	/***************************************************************************/
	/**
	  * Retrieves the upper boundary
	  *
	  * @return The value of the upper boundary
	  */
	num_type getUpperBoundary() const {
		return upperBoundary_;
	}

	/***************************************************************************/
	/**
	 * Resets the boundaries to the maximum allowed value.
	 */
	void resetBoundaries() {
		this->setBoundaries(GConstrainedValueLimitT<num_type>::lowest(), GConstrainedValueLimitT<num_type>::highest());
	}

	/***************************************************************************/
	/**
	 * Sets the boundaries of this object and does corresponding
	 * error checks. If the current value is below or above the new
	 * boundaries, this function will throw. Set the external value to
	 * a new value between the new boundaries before calling this
	 * function, or use the corresponding "setValue()" overload, which
	 * also allows setting of boundaries.
	 *
	 * @param lower The new lower boundary for this object
	 * @param upper The new upper boundary for this object
	 */
	virtual void setBoundaries(const num_type& lower, const num_type& upper) {
		std::vector<num_type> currentValues;
		for(std::size_t pos=0; pos<this->size(); pos++) {
			currentValues.push_back(GParameterCollectionT<num_type>::value(pos));

			// Check that the value is inside the allowed range
			if(currentValues[pos] < lower || currentValues[pos] > upper){
				throw gemfony_exception(
					g_error_streamer(DO_LOG, time_and_place)
						<< "In GConstrainedNumT<num_type>::setBoundaries(const T&, const T&) :" << std::endl
						<< "with typeid(num_type).name() = " << typeid(num_type).name() << std::endl
						<< "Attempt to set new boundaries [" << lower << ":" << upper << "]" << std::endl
						<< "with existing value  " << currentValues[pos] << " at position " << pos << " outside of this range." << std::endl
				);
			}
		}

		// Check that the boundaries make sense
		if(lower > upper) {
			throw gemfony_exception(
				g_error_streamer(DO_LOG, time_and_place)
					<< "In GConstrainedNumT<num_type>::setBoundaries(const num_type&, const num_type&)" << std::endl
					<< "with typeid(num_type).name() = " << typeid(num_type).name() << " :" << std::endl
					<< "Lower and/or upper boundary has invalid value : " << lower << " " << upper << std::endl
			);
		}

		lowerBoundary_ = lower;
		upperBoundary_ = upper;

		// Re-set the internal representation of the values -- we might be in a different
		// region of the transformation internally, and the mapping will likely depend on
		// the boundaries.
		for(std::size_t pos=0; pos<this->size(); pos++) {
			GParameterCollectionT<num_type>::setValue(pos, currentValues.at(pos));
		}
	}

	/***************************************************************************/
	/**
	 * Allows to set the value in a given position. This function will throw if val is
	 * not in the currently assigned value range. Use the corresponding overload if
	 * you want to set the value together with its boundaries instead.
	 *
	 * @param pos The position of the parameter to be set
	 * @param val The new num_type value stored in this class
	 */
	virtual void setValue(const std::size_t& pos, const num_type& val) override {
		// Do some error checking
		if(val < lowerBoundary_ || val > upperBoundary_) {
			throw gemfony_exception(
				g_error_streamer(DO_LOG, time_and_place)
					<< "In GConstrainedNumCollectionT<num_type>::setValue(pos, val):" << std::endl
					<< "In position " << pos << ":" << std::endl
					<< "Assigned value " << val << " is outside of its allowed boundaries: " << std::endl
					<< "lowerBoundary_ = " << lowerBoundary_ << std::endl
					<< "upperBoundary_ = " << upperBoundary_ << std::endl
			);
		}

		// O.k., assign value
		GParameterCollectionT<num_type>::setValue(pos, val);
	}

	/***************************************************************************/
	/**
	 * Retrieval of the value at a given position. This is an overloaded version
	 * of the original GParameterCollectionT<num_type>::value(pos) function which
	 * applies a transformation, to be defined in derived classes.
	 *
	 * @param pos The position for which the transformed value needs to be returned
	 * @return The transformed value of val_
	 */
	virtual num_type value(const std::size_t& pos) override {
		num_type mapping = transfer(GParameterCollectionT<num_type>::value(pos));

		// Reset internal value
		GParameterCollectionT<num_type>::setValue(pos, mapping);

		return mapping;
	}

	/***************************************************************************/
	/** @brief The transfer function needed to calculate the externally visible
	 * value. Declared public so we can do tests of the value transformation. */
	virtual num_type transfer(const num_type&) const = 0;

	/***************************************************************************/
	/**
	 * Converts the local data to a boost::property_tree node
	 *
	 * @param ptr The boost::property_tree object the data should be saved to
	 * @param id The id assigned to this object
	 */
	virtual void toPropertyTree(
		pt::ptree& ptr
		, const std::string& baseName
	) const override {
#ifdef DEBUG
      // Check that the object isn't empty
      if(this->empty()) {
			throw gemfony_exception(
				g_error_streamer(DO_LOG, time_and_place)
					<< "In GConstrainedNumCollectionT<num_type>::toPropertyTree(): Error!" << std::endl
					<< "Object is empty!" << std::endl
			);
      }
#endif /* DEBUG */

		ptr.put(baseName + ".name", this->getParameterName());
		ptr.put(baseName + ".type", this->name());
		ptr.put(baseName + ".baseType", Gem::Common::GTypeToStringT<num_type>::value());
		ptr.put(baseName + ".isLeaf", this->isLeaf());
		ptr.put(baseName + ".nVals", this->size());

		typename GConstrainedNumCollectionT<num_type>::const_iterator cit;
		std::size_t pos;
		for(cit=this->begin(); cit!=this->end(); ++cit) {
			pos = cit - this->begin();
			ptr.put(baseName + "values.value" + boost::lexical_cast<std::string>(pos), *cit);
		}
		ptr.put(baseName + ".lowerBoundary", this->getLowerBoundary());
		ptr.put(baseName + ".upperBoundary", this->getUpperBoundary());
		ptr.put(baseName + ".initRandom", false); // Unused for the creation of a property tree
		ptr.put(baseName + ".adaptionsActive", this->adaptionsActive());
	}

	/***************************************************************************/
	/**
	 * Emits a name for this class / object
	 */
	virtual std::string name() const override {
		return std::string("GConstrainedNumCollectionT");
	}

protected:
	/***************************************************************************/
	/**
	 * Loads the data of another GConstrainedNumCollectionT<num_type> object,
	 * camouflaged as a GObject. We have no local data, so
	 * all we need to do is to the standard identity check,
	 * preventing that an object is assigned to itself.
	 *
	 * @param cp A copy of another GConstrainedNumCollectionT<num_type> object, camouflaged as a GObject
	 */
	virtual void load_(const GObject *cp) override {
		// Check that we are dealing with a GConstrainedNumCollectionT<num_type> reference independent of this object and convert the pointer
		const GConstrainedNumCollectionT<num_type> *p_load = Gem::Common::g_convert_and_compare<GObject, GConstrainedNumCollectionT<num_type>>(cp, this);

		// Load our parent class'es data ...
		GParameterCollectionT<num_type>::load_(cp);

		// ... and then our local data
		lowerBoundary_ = p_load->lowerBoundary_;
		upperBoundary_ = p_load->upperBoundary_;
	}

	/***************************************************************************/
	/**
	 * Returns a "comparative range". This is e.g. used to make Gauss-adaption
	 * independent of a parameters value range
	 */
	virtual num_type range() const override {
		return upperBoundary_ - lowerBoundary_;
	}

	/***************************************************************************/
	/**
	 * Creates a deep copy of this object. Purely virtual as this class
	 * should not be instantiable.
	 *
	 * @return A pointer to a deep clone of this object
	 */
	virtual GObject *clone_() const override = 0;

	/***************************************************************************/
	/** @brief Triggers random initialization of the parameter collection */
	virtual bool randomInit_(
		const activityMode&
		, Gem::Hap::GRandomBase& gr
	) override = 0;

	/***************************************************************************/
	/**
	 * The default constructor. Intentionally protected -- only needed
	 * for de-serialization and as the basis for derived class'es default
	 * constructors
	 */
	GConstrainedNumCollectionT()
		: GParameterCollectionT<num_type> ()
		, lowerBoundary_(num_type(0))
		, upperBoundary_(num_type(1))
	{ /* nothing */ }

private:
	/***************************************************************************/
	num_type lowerBoundary_; ///< The lower allowed boundary for our value
	num_type upperBoundary_; ///< The upper allowed boundary for our value

public:
	/***************************************************************************/
	/**
	 * Applies modifications to this object. This is needed for testing purposes
	 *
	 * @return A boolean which indicates whether modifications were made
	 */
	virtual bool modify_GUnitTests() override {
#ifdef GEM_TESTING
      bool result = false;

		// Call the parent classes' functions
		if(GParameterCollectionT<num_type>::modify_GUnitTests()) result = true;

		return result;

#else /* GEM_TESTING */  // If this function is called when GEM_TESTING isn't set, throw
		Gem::Common::condnotset("GConstrainedNumCollectionT<>::modify_GUnitTests", "GEM_TESTING");
		return false;
#endif /* GEM_TESTING */
	}

	/***************************************************************************/
	/**
	 * Performs self tests that are expected to succeed. This is needed for testing purposes
	 */
	virtual void specificTestsNoFailureExpected_GUnitTests() override {
#ifdef GEM_TESTING
		// Call the parent classes' functions
		GParameterCollectionT<num_type>::specificTestsNoFailureExpected_GUnitTests();
#else /* GEM_TESTING */ // If this function is called when GEM_TESTING isn't set, throw
		Gem::Common::condnotset("GConstrainedNumCollectionT<>::specificTestsNoFailureExpected_GUnitTests", "GEM_TESTING");
#endif /* GEM_TESTING */
	}

	/***************************************************************************/
	/**
	 * Performs self tests that are expected to fail. This is needed for testing purposes
	 */
	virtual void specificTestsFailuresExpected_GUnitTests() override {
#ifdef GEM_TESTING
		// Call the parent classes' functions
		GParameterCollectionT<num_type>::specificTestsFailuresExpected_GUnitTests();
#else /* GEM_TESTING */ // If this function is called when GEM_TESTING isn't set, throw
		Gem::Common::condnotset("GConstrainedNumCollectionT<>::specificTestsFailuresExpected_GUnitTests", "GEM_TESTING");
#endif /* GEM_TESTING */
	}

	/***************************************************************************/
};

/******************************************************************************/
/**
 * Returns a "comparative range". Specialization for T==bool;
 */
template<>
inline bool GConstrainedNumCollectionT<bool>::range() const {
	return true;
}

/******************************************************************************/

} /* namespace Geneva */
} /* namespace Gem */

/******************************************************************************/
// The content of BOOST_SERIALIZATION_ASSUME_ABSTRACT(T)

namespace boost {
namespace serialization {
template<typename num_type>
struct is_abstract<Gem::Geneva::GConstrainedNumCollectionT<num_type>> : public boost::true_type {};
template<typename num_type>
struct is_abstract< const Gem::Geneva::GConstrainedNumCollectionT<num_type>> : public boost::true_type {};
}
}
/******************************************************************************/

#endif /* GCONSTRAINEDNUMCOLLECTIONT_HPP_ */
