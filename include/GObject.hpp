/**
 * @file
 */

/* GObject.hpp
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
#include <sstream>
#include <cstdlib>

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
#include <boost/serialization/nvp.hpp>
#include <boost/serialization/vector.hpp>
#include <boost/serialization/shared_ptr.hpp>
#include <boost/serialization/base_object.hpp>
#include <boost/serialization/utility.hpp>
#include <boost/serialization/tracking.hpp>
#include <boost/serialization/split_member.hpp>
#include <boost/serialization/export.hpp>
#include <boost/serialization/is_abstract.hpp>
#include <boost/strong_typedef.hpp>

#ifndef GOBJECT_HPP_
#define GOBJECT_HPP_

// Geneva header files go here

#include "GSerializableI.hpp"
#include "GenevaExceptions.hpp"
#include "GRandom.hpp"

namespace Gem {
namespace GenEvA {

/**
 * The serialization modes that are currently allowed
 */
enum serializationMode {TEXTSERIALIZATION,XMLSERIALIZATION};

/**************************************************************************************************/
/**
 * GObject is the parent class for the majority of GenEvA classes. Essentially, GObject gives a
 * GenEvA class the ability to carry a name and defines a number of interface functions.
 * The GObject::reset(void) , GObject::load(const GObject *) and  GObject::clone(void)
 * member functions should be re-implemented for each derived class. Unfortunately,
 * there is no way to enforce this in C++. Further common functionality of many GenEvA classes will
 * be implemented here over time.
 */
class GObject :public GSerializableI
{
    ///////////////////////////////////////////////////////////////////////
    friend class boost::serialization::access;

    template<class Archive>
    void serialize(Archive & ar, const unsigned int version){
      using boost::serialization::make_nvp;
      ar & make_nvp("name_",GObject::name_);
      ar & make_nvp("serializationMode_",serializationMode_);
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

	/** @brief An assignment operator that is also suitable for derived classes */
	virtual const GObject& operator=(const GObject&);

	/** @brief Convert class to a text representation */
	std::string toString();
	/** @brief Initialize class from text representation */
	void fromString(const std::string&);

	/** @brief Creates a deep clone of this object */
	virtual GObject* clone() = 0;
	/** @brief Resets the class to its initial state */
	virtual void reset();
	/** @brief Loads the data of another GObject */
	virtual void load(const GObject*);

	/** @brief Returns an XML description of the derivative it is called for */
	std::string report();

	/** @brief Retrieve the name of this class */
	std::string name(void) const throw();
	/** @brief Give the class a name */
	void setName(const std::string&) throw();

	/** @brief Retrieves the current serialization mode */
	serializationMode getSerializationMode(void) const throw();
	/** @brief Sets the serialization mode */
	void setSerializationMode(const serializationMode&);

protected:
    /**
     * @brief A random number generator. Each GenEvA object has
     * its own instance with a separate seed. Note that the actual
     * calculation is done in a random number server. Note that the
     * GRandom object will not be serialized. This means that objects
     * created from serialization data will re-initialize the random
     * number generator.
     */
	GRandom gr;

private:
	std::string name_;
	serializationMode serializationMode_;
};

}} /* namespace Gem::GenEvA */


/**************************************************************************************************/
/**
 * @brief Needed for Boost.Serialization
 */
BOOST_SERIALIZATION_ASSUME_ABSTRACT(Gem::GenEvA::GObject)

/**************************************************************************************************/

#endif /* GOBJECT_HPP_ */
