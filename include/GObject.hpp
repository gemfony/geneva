/**
 * @file GObject.hpp
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

// Standard header files go here

#include <string>
#include <sstream>
#include <cstdlib>
#include <typeinfo>

// Boost header files go here

#include <boost/version.hpp>

#if BOOST_VERSION < 103600
#error "Error: Boost should at least have version 1.36 !"
#endif /* BOOST_VERSION */

#include <boost/cstdint.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/exception.hpp>
#include <boost/archive/xml_oarchive.hpp>
#include <boost/archive/xml_iarchive.hpp>
#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>
#include <boost/archive/binary_oarchive.hpp>
#include <boost/archive/binary_iarchive.hpp>
#include <boost/serialization/nvp.hpp>
#include <boost/serialization/vector.hpp>
#include <boost/serialization/shared_ptr.hpp>
#include <boost/serialization/base_object.hpp>
#include <boost/serialization/utility.hpp>
#include <boost/serialization/tracking.hpp>
#include <boost/serialization/split_member.hpp>
#include <boost/serialization/export.hpp>
// #include <boost/serialization/is_abstract.hpp>

#ifndef GOBJECT_HPP_
#define GOBJECT_HPP_

// Geneva header files go here

#include "GSerializableI.hpp"
#include "GenevaExceptions.hpp"
#include "GRandom.hpp"
#include "GEnums.hpp"
#include "GLogger.hpp"

namespace Gem {
namespace GenEvA {

/**************************************************************************************************/
/**
 * GObject is the parent class for the majority of GenEvA classes. Essentially, GObject gives a
 * GenEvA class the ability to carry a name and defines a number of interface functions.
 * The GObject::load(const GObject *) and  GObject::clone() member functions should be
 * re-implemented for each derived class. Unfortunately, there is no way to enforce this in C++.
 * Further common functionality of many GenEvA classes will be implemented here over time.
 */
class GObject
	:public GSerializableI
{
    ///////////////////////////////////////////////////////////////////////
    friend class boost::serialization::access;

    template<typename Archive>
    void serialize(Archive & ar, const unsigned int version){
      using boost::serialization::make_nvp;
      ar & make_nvp("GSerializableI", boost::serialization::base_object<GSerializableI>(*this));
      ar & make_nvp("name_",GObject::name_);
    }
    ///////////////////////////////////////////////////////////////////////

public:
	/** @brief The default constructor */
	GObject() throw();
	/** @brief Initialization by name */
	explicit GObject(const std::string&) throw();
	/** @brief The copy constructor */
	GObject(const GObject& cp) throw();
	/** @brief The destructor */
	virtual ~GObject();

	/** @brief A standard assignment operator */
	const GObject& operator=(const GObject&);

	/** @brief Convert class to a serial representation, using a user-specified serialization mode */
	std::string toString(const serializationMode& serMod);
	/** @brief Convert class to a serial representation, using a specific serialization mode */
	void fromString(const std::string&, const serializationMode& serMod);

	/** @brief Creates a deep clone of this object */
	virtual GObject* clone() = 0;
	/** @brief Loads the data of another GObject */
	virtual void load(const GObject*);

	/** @brief Returns an XML description of the derivative it is called for */
	std::string report();

	/** @brief Retrieve the name of this class */
	std::string name() const throw();
	/** @brief Give the class a name */
	void setName(const std::string&) throw();

	/**************************************************************************************************/
	/**
	 * The function creates a clone of the GObject pointer and converts it to a pointer to a derived class.
	 * This work and the corresponding error checks are centralized in this function. Conversion will be faster
	 * if we do not compile in DEBUG mode.
	 *
	 * @return A converted clone of this object
	 */
	template <typename clone_type>
	inline clone_type* clone_ptr_cast(){
#ifdef DEBUG
		clone_type *result = dynamic_cast<clone_type *> (this->clone());

		// dynamic_cast will emit a NULL pointer, if the conversion failed
		if (!result) {
			std::ostringstream error;
			error << "In GObject::clone_ptr_cast<T>() : Conversion error!" << std::endl;

			// throw an exception. Add some information so that if the exception
			// is caught through a base object, no information is lost.
			throw geneva_error_condition(error.str());
		}

		return result;
#else
		return static_cast<clone_type *>(this->clone());
#endif
	}

	/**************************************************************************************************/
	/**
	 * The function creates a clone of the GObject pointer, converts it to a pointer to a derived class
	 * and emits it as a boost::shared_ptr<> . This work and the corresponding error checks are centralized
	 * in this function. Conversion will be faster if we do not compile in DEBUG mode.
	 *
	 * @return A converted clone of this object, wrapped into a boost::shared_ptr
	 */
	template <typename clone_type>
	inline boost::shared_ptr<clone_type> clone_bptr_cast(){
		// Get a clone of this object and wrap it in a boost::shared_ptr<GObject>
		boost::shared_ptr<GObject> p_base(this->clone());

#ifdef DEBUG
		// Convert to the desired target type
		boost::shared_ptr<clone_type> p_load = boost::dynamic_pointer_cast<clone_type>(p_base);

		// Check that the conversion worked. dynamic_cast emits an empty pointer,
		// if this was not the case.
		if(!p_load){
			std::ostringstream error;
			error << "In GObject::clone_bptr_cast<clone_type>() : Conversion error!" << std::endl;

			// throw an exception. Add some information so that if the exception
			// is caught through a base object, no information is lost.
			throw geneva_error_condition(error.str());
		}

		return p_load;
#else
		return boost::static_pointer_cast<clone_type>(p_base);
#endif
	}

	/**************************************************************************************************/

protected:
    /**
     * A random number generator. Each GenEvA object has
     * its own instance with a separate seed. Note that the actual
     * calculation is done in a random number server. Note that the
     * GRandom object will not be serialized. This means that objects
     * created from serialization data will re-initialize the random
     * number generator.
     */
	Gem::Util::GRandom gr;

	/**************************************************************************************************/
	/**
	 * The load function takes a GObject pointer and converts it to a pointer to a derived class. This
	 * work and the corresponding error checks are centralized in this function. Conversion will be fast
	 * if we do not compile in DEBUG mode.
	 *
	 * @param load_ptr A pointer to another T-object, camouflaged as a GObject
	 */
	template <typename load_type>
	inline const load_type* conversion_cast(const GObject *load_ptr, const load_type* This){
#ifdef DEBUG
		const load_type *result = dynamic_cast<const load_type *> (load_ptr);

		// dynamic_cast will emit a NULL pointer, if the conversion failed
		if (!result) {
			std::ostringstream error;
			error << "In GObject::checkConversion<T>() : Conversion error!" << std::endl;

			// throw an exception. Add some information so that if the exception
			// is caught through a base object, no information is lost.
			throw geneva_error_condition(error.str());
		}

		// Check that this object is not accidentally assigned to itself.
		if (load_ptr == This) {
			std::ostringstream error;
			error << "In GObject::checkConversion<T>() : Error!" << std::endl
					<< "Tried to assign an object to itself." << std::endl;

			throw geneva_error_condition(error.str());
		}

		return result;
#else
		return static_cast<const load_type *>(load_ptr);
#endif
	}

	/**************************************************************************************************/

private:
	std::string name_; ///< Allows to assign a name to this object
};

} /* namespace GenEvA */
} /* namespace Gem */


/**************************************************************************************************/
/**
 * @brief Needed for Boost.Serialization
 */
BOOST_SERIALIZATION_ASSUME_ABSTRACT(Gem::GenEvA::GObject)

/**************************************************************************************************/

#endif /* GOBJECT_HPP_ */
