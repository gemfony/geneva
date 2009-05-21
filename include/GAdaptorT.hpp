/**
 * @file GAdaptorT.hpp
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

// Standard headers go here
#include <iostream>
#include <string>
#include <sstream>
#include <vector>

// Includes check for correct Boost version(s)
#include "GGlobalDefines.hpp"

// Boost headers go here

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

#ifndef GADAPTORT_HPP_
#define GADAPTORT_HPP_

// Geneva headers go here

#include "GObject.hpp"
#include "GRandom.hpp"
#include "GEnums.hpp"

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
 * Classes derived from GParameterBaseWithAdaptorsT<T> can additionally store
 * "adaptors". These are templatized function objects that can act
 * on the items of a collection of user-defined types. Predefined
 * adaptors exist for standard types (with the most prominent
 * examples being bits and double values).
 *
 * The GAdaptorT class mostly acts as an interface for these
 * adaptors, but also implements some functionality of its own. E.g., it is possible
 * to specify a function that shall be called every adaptionThreshold_ calls of the
 * mutate() function.
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
template<typename T>
class GAdaptorT:
	public GObject
{
	///////////////////////////////////////////////////////////////////////
	friend class boost::serialization::access;

	template<typename Archive>
	void serialize(Archive & ar, const unsigned int) {
		using boost::serialization::make_nvp;
		ar & make_nvp("GObject", boost::serialization::base_object<GObject>(*this));
		ar & make_nvp("gr",gr);
		ar & make_nvp("adaptionCounter_", adaptionCounter_);
		ar & make_nvp("adaptionThreshold_", adaptionThreshold_);
	}
	///////////////////////////////////////////////////////////////////////

public:
	/***********************************************************************************/
	/**
	 * The default constructor.
	 */
	GAdaptorT():
		GObject(),
		adaptionCounter_(0),
		adaptionThreshold_(0)
	{
		gr.setRnrGenerationMode(Gem::Util::RNRFACTORY);
	}

	/***********************************************************************************/
	/**
	 * A standard copy constructor.
	 *
	 * @param cp A copy of another GAdaptorT<T>
	 */
	GAdaptorT(const GAdaptorT<T>& cp):
		GObject(cp),
		gr(cp.gr),
		adaptionCounter_(cp.adaptionCounter_),
		adaptionThreshold_(cp.adaptionThreshold_)
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
	 * Loads the contents of another GAdaptorT<T>. The function
	 * is similar to a copy constructor (but with a pointer as
	 * argument). As this function might be called in an environment
	 * where we do not know the exact type of the class, the
	 * GAdaptorT<T> is camouflaged as a GObject . This implies the
	 * need for dynamic conversion.
	 *
	 * @param gb A pointer to another GAdaptorT<T>, camouflaged as a GObject
	 */
	virtual void load(const GObject *cp) {
		// Convert cp into local format
		const GAdaptorT<T> *gat = this->conversion_cast(cp, this);

		// Load the parent class'es data
		GObject::load(cp);

		// Then our own data
		gr.load(&(gat->gr));
		adaptionCounter_ = gat->adaptionCounter_;
		adaptionThreshold_ = gat->adaptionThreshold_;
	}

	/***********************************************************************************/
	/** @brief Creates a deep copy of this object */
	virtual GObject *clone(void) const =0;

	/***********************************************************************************/
	/**
	 * Checks for equality with another GAdaptorT<T> object
	 *
	 * @param  cp A constant reference to another GAdaptorT<T> object
	 * @return A boolean indicating whether both objects are equal
	 */
	bool operator==(const GAdaptorT<T>& cp) const {
		return GAdaptorT<T>::isEqualTo(cp, boost::logic::indeterminate);
	}

	/***********************************************************************************/
	/**
	 * Checks for inequality with another GAdaptorT<T> object
	 *
	 * @param  cp A constant reference to another GAdaptorT<T> object
	 * @return A boolean indicating whether both objects are inequal
	 */
	bool operator!=(const GAdaptorT<T>& cp) const {
		return !GAdaptorT<T>::isEqualTo(cp, boost::logic::indeterminate);
	}

	/***********************************************************************************/
	/**
	 * Checks for equality with another GAdaptorT<T> object
	 *
	 * @param  cp A constant reference to another GAdaptorT<T> object
	 * @return A boolean indicating whether both objects are equal
	 */
	virtual bool isEqualTo(const GObject& cp, const boost::logic::tribool& expected = boost::logic::indeterminate) const {
		using namespace Gem::Util;

		// Check that we are indeed dealing with a GAdaptorT reference
		const GAdaptorT<T> *gat_load = GObject::conversion_cast(&cp,  this);

		// First check our parent class for equality
		if(!GObject::isEqualTo(*gat_load, expected)) return false;

		// then our local data
		if(!gr.isEqualTo(gat_load->gr, expected)) return false;
		if(checkForInequality("GAdaptorT", adaptionCounter_, gat_load->adaptionCounter_,"adaptionCounter_", "gat_load->adaptionCounter_", expected)) return false;
		if(checkForInequality("GAdaptorT", adaptionThreshold_, gat_load->adaptionThreshold_,"adaptionThreshold_", "gat_load->adaptionThreshold_", expected)) return false;

		return true;
	}

	/***********************************************************************************/
	/**
	 * Checks for similarity with another GAdaptorT<T> object
	 *
	 * @param  cp A constant reference to another GAdaptorT<T> object
	 * @param limit A double value specifying the acceptable level of differences of floating point values
	 * @return A boolean indicating whether both objects are similar to each other
	 */
	virtual bool isSimilarTo(const GObject& cp, const double& limit, const boost::logic::tribool& expected = boost::logic::indeterminate) const {
		using namespace Gem::Util;

		// Check that we are indeed dealing with a GAdaptorT reference
		const GAdaptorT<T> *gat_load = GObject::conversion_cast(&cp,  this);

		// First check our parent class for dissimilarity
		if(!GObject::isSimilarTo(*gat_load, limit, expected)) return false;

		// Then our local data
		if(!gr.isSimilarTo(gat_load->gr, limit, expected)) return false;
		if(checkForDissimilarity("GAdaptorT", adaptionCounter_, gat_load->adaptionCounter_, limit, "adaptionCounter_", "gat_load->adaptionCounter_", expected)) return false;
		if(checkForDissimilarity("GAdaptorT", adaptionThreshold_, gat_load->adaptionThreshold_, limit, "adaptionThreshold_", "gat_load->adaptionThreshold_", expected)) return false;

		return true;
	}

	/***********************************************************************************/
	/**
	 * Retrieves the id of the adaptor. Purely virtual, must be implemented by the
	 * actual adaptors.
	 *
	 * @return The id of the adaptor
	 */
	virtual Gem::GenEvA::adaptorId getAdaptorId() const = 0;

	/***********************************************************************************/
	/**
	 * Determines whether production of random numbers should happen remotely
	 * (RNRFACTORY) or locally (RNRLOCAL) in the local random number generator.
	 *
	 * @param rnrGenMode A parameter which indicates where random numbers should be produced
	 */
	virtual void setRnrGenerationMode(const Gem::Util::rnrGenerationMode& rnrGenMode) {
		gr.setRnrGenerationMode(rnrGenMode);
	}

	/***********************************************************************************/
	/**
	 * Retrieves the random number generators current generation mode.
	 *
	 * @return The current random number generation mode of the local generator
	 */
	Gem::Util::rnrGenerationMode getRnrGenerationMode() const {
		return gr.getRnrGenerationMode();
	}

	/***********************************************************************************/
	/**
	 * Common interface for all adaptors to the mutation
	 * functionality. The user specifies this functionality in the
	 * customMutations() function.
	 *
	 * @param val The value that needs to be mutated
	 */
	inline void mutate(T& val)  {
		if(adaptionThreshold_ && adaptionCounter_++ >= adaptionThreshold_){
			adaptionCounter_ = 0;
			adaptMutation();
		}

		this->customMutations(val);
	}

	/***********************************************************************************/
	/**
	 * Retrieves the current value of the adaptionCounter_ variable.
	 *
	 * @return The value of the adaptionCounter_ variable
	 */
	virtual boost::uint32_t getAdaptionCounter() const  {
		return adaptionCounter_;
	}

	/***********************************************************************************/
	/**
	 * Sets the value of adaptionThreshold_. If set to 0, no adaption of the optimization
	 * parameters will take place
	 *
	 * @param adaptionCounter The value that should be assigned to the adaptionCounter_ variable
	 */
	void setAdaptionThreshold(const boost::uint32_t& adaptionThreshold)  {
		adaptionThreshold_ = adaptionThreshold;
	}

	/***********************************************************************************/
	/**
	 * Retrieves the value of the adaptionThreshold_ variable.
	 *
	 * @return The value of the adaptionThreshold_ variable
	 */
	boost::uint32_t getAdaptionThreshold() const  {
		return adaptionThreshold_;
	}

protected:
	/***********************************************************************************/
    /**
     * A random number generator. Note that the actual calculation is possibly
     * done in a random number server. GRandom also has a local generator
     * in case the factory is unreachable, or local storage of random
     * number containers requires too much memory.
     */
	Gem::Util::GRandom gr;

	/***********************************************************************************/
	/**
	 *  This function is re-implemented by derived classes, if they wish to
	 *  implement special behavior upon a new mutation run. E.g., an internal
	 *  variable could be set to a new value. This is used in GDoubleGaussAdaptor
	 *  to modify the sigma of the gaussian, if desired. The function will be
	 *  called every adaptionThreshold_ calls of the mutate function, unless the
	 *  threshold is set to 0 . It is not purely virtual, as we do not force
	 *  derived classes to re-implement this function.
	 */
	virtual void adaptMutation() { /* nothing */ }

	/***********************************************************************************/
	/** @brief Mutation of values as specified by the user */
	virtual void customMutations(T& val)=0;

private:
	/***********************************************************************************/
	boost::uint32_t adaptionCounter_; ///< A local counter
	boost::uint32_t adaptionThreshold_; ///< Specifies after how many mutations the mutation itself should be adapted
};

/******************************************************************************************/

} /* namespace GenEvA */
} /* namespace Gem */

/********************************************************************************************/
// The content of BOOST_SERIALIZATION_ASSUME_ABSTRACT(T)

namespace boost {
	namespace serialization {
		template<typename T>
		struct is_abstract<Gem::GenEvA::GAdaptorT<T> > : public boost::true_type {};
		template<typename T>
		struct is_abstract< const Gem::GenEvA::GAdaptorT<T> > : public boost::true_type {};
	}
}

/********************************************************************************************/

#endif /* GADAPTORT_HPP_ */
