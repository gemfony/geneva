/**
 * @file
 */

/* GAdaptorT.hpp
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

#include <iostream>
#include <string>
#include <sstream>
#include <vector>

#include <boost/version.hpp>

#if BOOST_VERSION < 103600
#error "Error: Boost should at least have version 1.36 !"
#endif /* BOOST_VERSION */

#include <boost/cstdint.hpp>
#include <boost/archive/xml_oarchive.hpp>
#include <boost/archive/xml_iarchive.hpp>
#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>
#include <boost/serialization/nvp.hpp>
#include <boost/serialization/vector.hpp>
#include <boost/serialization/shared_ptr.hpp>
#include <boost/serialization/base_object.hpp>
#include <boost/serialization/utility.hpp>

#ifndef GADAPTORT_H_
#define GADAPTORT_H_

#include "GObject.hpp"
#include "GLogger.hpp"

namespace Gem {
namespace GenEvA {

/******************************************************************************************/
/**
 * In GenEvA, two mechanisms exist that let the user specify the
 * type of mutation he wants to have executed on collections of
 * items (basic types or any other types).  The most basic
 * possibility is for the user to overload the GIndividual::customMutations()
 * function and manually specify the types of mutations (s)he
 * wants. This allows great flexibility, but is not very practicable
 * for standard mutations.
 *
 * Classes derived from GMutable<T> can additionally store
 * "adaptors". These are templatized function objects that can act
 * on the items of a collection of user-defined types. Predefined
 * adaptors exist for standard types (with the most prominent
 * examples being bits and double values).
 *
 * The GAdaptorT class mostly acts as an interface for these
 * adaptors, but also implements some functionality of its own.
 *
 * Adaptors can be applied to single items T or collections of items
 * vector<T>. In collections, the initialization function
 * initNewRun() can be called either for each invocation of the
 * adaptor, or for sequences, indicated by the variable alwaysInit_,
 * as set by the caller.
 *
 * In order to use this class, the user must derive a class from
 * GAdaptorT<T> and specify the type of mutation he wishes to
 * have applied to items, by overloading of
 * GAdaptorT<T>::customMutations(T&) .  T will often be
 * represented by a basic value (double, long, bool, ...). Where
 * this is not the case, the adaptor will only be able to access
 * public functions of T, unless T declares the adaptor as a friend.
 *
 * As a derivative of GObject, this class follows similar rules as
 * the other GenEvA classes.
 */
template<class T>
class GAdaptorT:
	public GObject
{
	///////////////////////////////////////////////////////////////////////
	friend class boost::serialization::access;

	template<class Archive>
	void serialize(Archive & ar, const unsigned int version) {
		using boost::serialization::make_nvp;
		ar & make_nvp("GObject", boost::serialization::base_object<GObject>(*this));
		ar & make_nvp("alwaysInit_", alwaysInit_);
	}
	///////////////////////////////////////////////////////////////////////

public:
	/***********************************************************************************/
	/**
	 * Every adaptor is required to have a name. It is set in this
	 * constructor and the name is passed to the underlying GObject
	 * class.  No default constructor exists, as we want to enforce
	 * names for adaptors. By default we want to call the
	 * initialization function of the adaptor for every item of a
	 * collection.
	 *
	 * @param name The name assigned to the adaptor
	 */
	explicit GAdaptorT(std::string name) throw() :
		GObject(name),
		alwaysInit_(true)
	{ /* nothing */ }

	/***********************************************************************************/
	/**
	 * A standard copy constructor.
	 *
	 * @param cp A copy of another GAdaptorT<T>
	 */
	GAdaptorT(const GAdaptorT<T>& cp) throw() :
		GObject(cp),
		alwaysInit_(cp.alwaysInit_)
	{ /* nothing */ }

	/***********************************************************************************/
	/**
	 * The standard destructor. We have no local, dynamically allocated data, so the body of
	 * this function is empty.
	 */
	virtual ~GAdaptorT() { /* nothing */ }

	/***********************************************************************************/
	/**
	 * A standard assignment operator for GAdaptorT<T> objects,
	 *
	 * @param cp A copy of another GAdaptorT<T> object
	 */
	const GAdaptorT<T>& operator=(const GAdaptorT<T>& cp) {
		GAdaptorT<T>::load(&cp);
		return *this;
	}

	/***********************************************************************************/
	/**
	 * This function resets the GAdaptorT<T> to its initial state.
	 */
	virtual void reset(void) {
		alwaysInit_ = true;
		GObject::reset();
	}

	/***********************************************************************************/
	/**
	 * Loads the content of another GAdaptorT<T>. The function
	 * is similar to a copy constructor (but with a pointer as
	 * argument). As this function might be called in an environment
	 * where we do not know the exact type of the class, the
	 * GAdaptorT<T> is camouflaged as a GObject . This implies the
	 * need for dynamic conversion.
	 *
	 * @param gb A pointer to another GAdaptorT<T>, camouflaged as a GObject
	 */
	virtual void load(const GObject *cp) {
		const GAdaptorT<T> *gat = GObject::checkConversion(cp, this);

		/*
		const GAdaptorT<T> *gat = dynamic_cast<const GAdaptorT<T> *> (cp);

		// dynamic_cast will emit a NULL pointer, if the conversion failed
		if (!gat) {
			std::ostringstream error;
			error << "In GAdaptorT::load(): Conversion error!" << std::endl;

			LOGGER.log(error.str(), Gem::GLogFramework::CRITICAL);

			// throw an exception. Add some information so that if the exception
			// is caught through a base object, no information is lost.
			throw geneva_dynamic_cast_conversion_error() << error_string(error.str());
		}

		// Check that this object is not accidently assigned to itself.
		if (gat == this) {
			std::ostringstream error;
			error << "In GAdaptorT::load(): Error!" << std::endl
				  << "Tried to assign an object to itself." << std::endl;

			LOGGER.log(error.str(), Gem::GLogFramework::CRITICAL);
			throw geneva_object_assigned_to_itself() << error_string(error.str());
		}
	*/

		// Load the parent class'es data
		GObject::load(cp);

		// Then our own data
		alwaysInit_ = gat->alwaysInit_;
	}

	/***********************************************************************************/
	/** @brief Creates a deep copy of this object */
	virtual GObject *clone(void)=0;

	/***********************************************************************************/
	/**
	 * Common interface for all adaptors to the mutation
	 * functionality. The user specifies this functionality in the
	 * customMutations() function.
	 *
	 * @param val The value that needs to be mutated
	 */
	inline void mutate(T& val) throw() {
		this->customMutations(val);
	}

	/***********************************************************************************/
	/**
	 * Mutation of sequences of values, stored in an STL vector. This function also calls an
	 * initialization function for the mutation function, either for each value, or only once
	 * per sequence (depending on the value of alwaysInit_).
	 *
	 * @param collection A vector of values that need to be mutated
	 */
	inline virtual void mutate(std::vector<T>& collection) {
		typename std::vector<T>::iterator it;
		for (it = collection.begin(); it != collection.end(); ++it) {
			if (alwaysInit_ || it == collection.begin()) initNewRun();
			this->mutate(*it);
		}
	}

	/***********************************************************************************/
	/**
	 * Retrieves the value of the alwaysInit_ variable.
	 *
	 * @return The value of the alwaysInit_ variable
	 */
	bool alwaysInit(void) const throw() {
		return alwaysInit_;
	}

	/***********************************************************************************/
	/**
	 * Sets the value of alwaysInit_. If set to true, mutations will be
	 * initialized for each item of a sequence. If set to false, initialization will only
	 * happen for the first item.
	 *
	 * @param val The value that should be assigned to the alwaysInit_ variable
	 */
	void setAlwaysInit(bool val) throw() {
		alwaysInit_ = val;
	}

protected:
	/***********************************************************************************/
	/**
	 *  This function is re-implemented by derived classes, if they wish to
	 *  implement special behavior upon a new mutation run. E.g., an internal
	 *  variable could be set to a new value. This is used in GDoubleGaussAdaptor
	 *  to modify the sigma of the gaussian, if desired. The function will be
	 *  called for each item of a sequence, if alwaysInit_ is set to true, otherwise
	 *  it will be called only for the first item. It is not purely virtual, as
	 *  we do not force derived classes to re-implement this function.
	 */
	virtual void initNewRun(void) { /* nothing */ }

	/***********************************************************************************/
	/** @brief Mutation of values as specified by the user */
	virtual void customMutations(T& val)=0;

private:
	/***********************************************************************************/
	/** @brief Default constructor. Private as we want every adaptor to have a name.
	 Intentionally left undefined. */
	GAdaptorT(void);

	bool alwaysInit_;
};

/******************************************************************************************/

}} /* namespace Gem::GenEvA */

/********************************************************************************************/
// The content of BOOST_SERIALIZATION_ASSUME_ABSTRACT(T)

namespace boost {
	namespace serialization {
		template<class T>
		struct is_abstract<Gem::GenEvA::GAdaptorT<T> > : boost::true_type {};
		template<class T>
		struct is_abstract< const Gem::GenEvA::GAdaptorT<T> > : boost::true_type {};
	}
}

/********************************************************************************************/

#endif /*GADAPTORT_H_*/
