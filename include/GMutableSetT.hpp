/**
 * @file GMutableSetT.hpp
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
#include <sstream>
#include <vector>
#include <typeinfo>

// Includes check for correct Boost version(s)
#include "GGlobalDefines.hpp"

// Boost header files go here

#include <boost/cstdint.hpp>
#include <boost/shared_ptr.hpp>

#ifndef GMUTABLESETT_HPP_
#define GMUTABLESETT_HPP_

// For Microsoft-compatible compilers
#if defined(_MSC_VER)  &&  (_MSC_VER >= 1020)
#pragma once
#endif


#include "GIndividual.hpp"
#include "GParameterBase.hpp"
#include "GObject.hpp"
#include "GHelperFunctionsT.hpp"
#include "GStdPtrVectorInterfaceT.hpp"

namespace Gem {
namespace GenEvA {

/******************************************************************************************/
/**
 * This class forms the basis for many user-defined individuals. It acts as a collection
 * of different parameter sets. User individuals can thus contain a mix of parameters of
 * different types, such as double, GenEvA::bit, long, ... . Derived classes must implement
 * a useful operator=(). It is also assumed that template arguments have the GObject and the
 * GMutableI interfaces, in particular the load(), clone() and mutate() functions.
 */
template <typename T>
class GMutableSetT:
	public GIndividual,
	public GStdPtrVectorInterfaceT<T>
{
    ///////////////////////////////////////////////////////////////////////
    friend class boost::serialization::access;

    template<typename Archive>
    void serialize(Archive & ar, const unsigned int){
      using boost::serialization::make_nvp;

      ar & BOOST_SERIALIZATION_BASE_OBJECT_NVP(GIndividual)
         & make_nvp("GStdPtrVectorInterfaceT_T", boost::serialization::base_object<GStdPtrVectorInterfaceT<T> >(*this));
    }
    ///////////////////////////////////////////////////////////////////////

public:
	/**********************************************************************************/
	/**
	 * The default constructor. No local data, hence nothing to do.
	 */
	GMutableSetT()
		: GIndividual()
		, GStdPtrVectorInterfaceT<T>()
	{ /* nothing */	}

	/**********************************************************************************/
	/**
	 * The copy constructor. Note that we cannot rely on the operator=() of the vector
	 * here, as we do not know the actual type of T objects.
	 *
	 * @param cp A copy of another GMutableSetT<T> object
	 */
	GMutableSetT(const GMutableSetT<T>& cp)
		: GIndividual(cp)
		, GStdPtrVectorInterfaceT<T>(cp)
	{ /* nothing */ }

	/**********************************************************************************/
	/**
	 * The destructor. As we use smart pointers, we do not need to take care of the
	 * data in the vector ourselves.
	 */
	virtual ~GMutableSetT()
	{ /* nothing */ }

	/**********************************************************************************/
	/**
	 * Loads the data of another GParameterBase object, camouflaged as a GObject
	 *
	 * @param cp A copy of another GMutableSetT object, camouflaged as a GObject
	 */
	virtual void load(const GObject* cp){
		// Convert cp into local format
	    const GMutableSetT<T> *p_load = this->conversion_cast(cp, this);

	    // No local data - load the parent class'es data
	    GIndividual::load(cp);
		GStdPtrVectorInterfaceT<T>::operator=(*p_load);
	}

	/**********************************************************************************/
	/**
	 * Checks for equality with another GNumCollectionT<T> object
	 *
	 * @param  cp A constant reference to another GMutableSetT<T> object
	 * @return A boolean indicating whether both objects are equal
	 */
	bool operator==(const GMutableSetT<T>& cp) const {
		using namespace Gem::Util;
		// Means: The expectation of equality was fulfilled, if no error text was emitted (which converts to "true")
		return !checkRelationshipWith(cp, CE_EQUALITY, 0.,"GMutableSetT<T>::operator==","cp", CE_SILENT);
	}

	/**********************************************************************************/
	/**
	 * Checks for inequality with another GMutableSetT<T> object
	 *
	 * @param  cp A constant reference to another GMutableSetT<T> object
	 * @return A boolean indicating whether both objects are inequal
	 */
	bool operator!=(const GMutableSetT<T>& cp) const {
		using namespace Gem::Util;
		// Means: The expectation of inequality was fulfilled, if no error text was emitted (which converts to "true")
		return !checkRelationshipWith(cp, CE_INEQUALITY, 0.,"GMutableSetT<T>::operator!=","cp", CE_SILENT);
	}

	/**********************************************************************************/
	/**
	 * Checks whether a given expectation for the relationship between this object and another object
	 * is fulfilled.
	 *
	 * @param cp A constant reference to another object, camouflaged as a GObject
	 * @param e The expected outcome of the comparison
	 * @param limit The maximum deviation for floating point values (important for similarity checks)
	 * @param caller An identifier for the calling entity
	 * @param y_name An identifier for the object that should be compared to this one
	 * @param withMessages Whether or not information should be emitted in case of deviations from the expected outcome
	 * @return A boost::optional<std::string> object that holds a descriptive string if expectations were not met
	 */
	boost::optional<std::string> checkRelationshipWith(const GObject& cp,
			const Gem::Util::expectation& e,
			const double& limit,
			const std::string& caller,
			const std::string& y_name,
			const bool& withMessages) const
	{
	    using namespace Gem::Util;
	    using namespace Gem::Util::POD;

		// Check that we are indeed dealing with a GParamterBase reference
		const GMutableSetT<T> *p_load = GObject::conversion_cast(&cp,  this);

		// Will hold possible deviations from the expectation, including explanations
	    std::vector<boost::optional<std::string> > deviations;

		// Check our parent class'es data ...
		deviations.push_back(GIndividual::checkRelationshipWith(cp, e, limit, "GMutableSetT<T>", y_name, withMessages));
		deviations.push_back(GStdPtrVectorInterfaceT<T>::checkRelationshipWith(*p_load, e, limit, "GMutableSetT<T>", y_name, withMessages));

		// no local data ...

		return evaluateDiscrepancies("GMutableSetT<T>", caller, deviations, e);
	}

	/**********************************************************************************/
	/**
	 * Swap another object's vector with ours
	 */
	inline void swap(GMutableSetT<T>& cp) { GStdPtrVectorInterfaceT<T>::swap(cp.data); }

	/**********************************************************************************/
	/**
	 * Swap another vector with ours
	 */
	inline void swap(std::vector<boost::shared_ptr<T> >& cp_data) { GStdPtrVectorInterfaceT<T>::swap(cp_data); }


protected:
	/**********************************************************************************/
	/** @brief Creates a deep clone of this object. Purely virtual, so this class cannot be instantiated */
	virtual GObject* clone_() const = 0;

	/**********************************************************************************/
	/** @brief The actual fitness calculation takes place here */
	virtual double fitnessCalculation() = 0;

	/**********************************************************************************/
	/**
	 * The actual mutation operations. Easy, as we know that all GParameterBase objects
	 * in this object must implement the mutate() function.
	 */
	virtual void customMutations(){
		typename GMutableSetT<T>::iterator it;
		for(it=this->begin(); it!=this->end(); ++it) (*it)->mutate();
	}

	/**********************************************************************************/
	/**
	 * Re-implementation of a corresponding function in GStdPtrVectorInterface.
	 * Make the vector wrapper purely virtual allows the compiler to perform
	 * further optimizations.
	 */
	virtual void dummyFunction() { /* nothing */ }

	/**********************************************************************************/
};

} /* namespace GenEvA */
} /* namespace Gem */

/**************************************************************************************************/
/**
 * @brief The content of the BOOST_SERIALIZATION_ASSUME_ABSTRACT(T) macro. Needed for Boost.Serialization
 */
namespace boost {
  namespace serialization {
    template<typename T>
    struct is_abstract<Gem::GenEvA::GMutableSetT<T> > : public boost::true_type {};
    template<typename T>
    struct is_abstract< const Gem::GenEvA::GMutableSetT<T> > : public boost::true_type {};
  }
}

/**************************************************************************************************/

#endif /* GMUTABLESETT_HPP_ */
