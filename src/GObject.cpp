/**
 * @file
 */

/*
 * GObject.cpp
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

#include "GObject.hpp"

// Included here so no conflicts occur. See explanation at
// http://www.boost.org/libs/serialization/doc/special.html#derivedpointers
#include <boost/serialization/export.hpp>
BOOST_CLASS_EXPORT(Gem::GenEvA::GObject)

namespace Gem {
namespace GenEvA {

/**************************************************************************************************/
/**
 * The default constructor initializes the internal values of this class.
 * In particular, it sets the name of the Geneva object to "GObject"
 */
GObject::GObject(void) throw() :
	name_("GObject"), serializationMode_(TEXTSERIALIZATION) { /* nothing */
}

/**************************************************************************************************/
/**
 * If a particular name is desired for an object of this class, it can be
 * set upon initialization with this function.
 *
 * @param geneva_object_name The name which is assigned to a Geneva object
 */
GObject::GObject(const std::string& geneva_object_name) throw() :
	name_(geneva_object_name), serializationMode_(TEXTSERIALIZATION) { /* nothing */
}

/**************************************************************************************************/
/**
 * We need a copy constructor, even though this class is purely virtual. It is called by child
 * classes, when their copy constructor is executed. Note that this function is not implemented
 * with the load() function. Reason: A copy constructor needs to call the constructor
 * of its parent class. Using a load function in the copy constructor might replicate some
 * copying. Hence, throughout the Geneva library, we do not use the load function for copying
 * in the copy constructor.
 *
 * @param cp A copy of another GObject object
 */
GObject::GObject(const GObject& cp) throw() :
	name_(cp.name_), serializationMode_(cp.serializationMode_)
{ /* nothing */ }

/**************************************************************************************************/
/**
 * As no memory is dynamically allocated in GObject, no work has to
 * be done by this destructor.
 */
GObject::~GObject()
{ /* nothing */ }

/**************************************************************************************************/
/**
 * An assignment operator that is also suitable for derived classes
 *
 * @param cp A copy of another GObject(-derivative)
 * @return A constant reference to this object
 */
const GObject& GObject::operator=(const GObject& cp){
	load(&cp);
	return *this;
}

/**************************************************************************************************/
/**
 * Converts the class to a text representation.
 *
 * @return A text-representation of this class (or its derivative)
 */
std::string GObject::toString() {
    std::ostringstream oarchive_stream;

    {
      GObject *local = this;
      boost::archive::xml_oarchive oa(oarchive_stream);
      oa & boost::serialization::make_nvp("classhierarchyFromGObject",local);
    } // note: explicit scope here is essential so oa-destructor gets called

    return oarchive_stream.str();
}

/**************************************************************************************************/
/**
 * Initializes the object from its string representation. Note that the string will
 * likely describe a derivative of GObject.
 *
 * @param descr A text representation of a GObject-derivative
 */
void GObject::fromString(const std::string& descr) {
    std::istringstream istr(descr);

    {
      GObject *local = this;
      boost::archive::xml_iarchive ia(istr);
      ia & boost::serialization::make_nvp("classhierarchyFromGObject",local);
    } // note: explicit scope here is essential so ia-destructor gets called
}

/**************************************************************************************************/
/**
 * Resets the class to its initial state
 */
void GObject::reset() {
	name_ = "GObject";
	serializationMode_ = TEXTSERIALIZATION;
}

/**************************************************************************************************/
/**
 * Loads the data of another GObject
 *
 * @param cp A pointer to another GObject object
 */
void GObject::load(const GObject *cp) {
	// Check that this object is not accidentally assigned to itself.
	if (cp == this) {
		// Compose an error message
		std::ostringstream str;
		str << "In GObject::load: Error! Object was assigned to itself";

		// write a log message
		LOGGER.log(str.str(), Gem::GLogFramework::CRITICAL);

		// throw an exception. Add some information so that if the exception
		// is caught through a base object, no information is lost
		throw geneva_object_assigned_to_itself() << error_string(str.str());
	}

	// Load the actual data
	name_ = cp->name_;
	serializationMode_ = cp->serializationMode_;
}

/**************************************************************************************************/
/**
 * Returns an XML description of the derivative it is called for
 *
 * @return An XML description of the GObject-derivative the function is called for
 */
std::string GObject::report() {
	serializationMode tmp = serializationMode_;
	serializationMode_ = XMLSERIALIZATION;
	std::string result = toString();
	serializationMode_ = tmp;
	return result;
}

/**************************************************************************************************/
/**
 * Returns the name of this class.
 *
 * @return The name of this class
 */
std::string GObject::name(void) const throw() {
	return name_;
}

/**************************************************************************************************/
/**
 * Allows to set a name for this class.
 *
 * @param geneva_object_name The name of this class
 */
void GObject::setName(const std::string& geneva_object_name) throw() {
	name_ = geneva_object_name;
}

/**************************************************************************************************/
/**
 * Retrieves the current serialization mode
 *
 * @return The current serialization mode
 */
serializationMode GObject::getSerializationMode(void) const throw() {
	return serializationMode_;
}

/**************************************************************************************************/
/**
 * Sets the serialization mode. The only allowed values of the enum serializationMode are
 * TEXTSERIALIZATION and XMLSERIALIZATION. The compiler does the error-checking for us.
 *
 * @param ser The new serialization mode
 */
void GObject::setSerializationMode(const serializationMode& ser) {
	serializationMode_ = ser;
}

/**************************************************************************************************/
/**
 * The load function takes a GObject pointer and converts it to a pointer to a derived class. This
 * work is centralized in this function.
 *
 * @param load_ptr A pointer to another T-object, camouflaged as a GObject
 */
template <class T>
inline const T* Gem::GenEvA::GObject::checkConversion(const Gem::GenEvA::GObject *load_ptr, const T* This){
	const T *result = std::dynamic_cast<const T *> (load_ptr);

	// dynamic_cast will emit a NULL pointer, if the conversion failed
	if (!result) {
		std::ostringstream error;
		error << "In GObject::checkConversion<T>() : Conversion error!" << std::endl;

		LOGGER.log(error.str(), Gem::GLogFramework::CRITICAL);

		// throw an exception. Add some information so that if the exception
		// is caught through a base object, no information is lost.
		throw geneva_dynamic_cast_conversion_error() << error_string(error.str());
	}

	// Check that this object is not accidentally assigned to itself.
	if (load_ptr == This) {
		std::ostringstream error;
		error << "In GObject::checkConversion<T>() : Error!" << std::endl
				<< "Tried to assign an object to itself." << std::endl;

		LOGGER.log(error.str(), Gem::GLogFramework::CRITICAL);
		throw geneva_object_assigned_to_itself() << error_string(error.str());
	}

	return result;
}

/**************************************************************************************************/

}} /* namespace Gem::GenEvA */

