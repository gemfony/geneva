/**
 * @file GObject.cpp
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
	name_("GObject")
{ /* nothing */ }

/**************************************************************************************************/
/**
 * If a particular name is desired for an object of this class, it can be
 * set upon initialization with this function.
 *
 * @param geneva_object_name The name which is assigned to a Geneva object
 */
GObject::GObject(const std::string& geneva_object_name) throw() :
	name_(geneva_object_name)
{ /* nothing */ }

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
	name_(cp.name_)
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
 * Returns an XML description of the derivative it is called for
 *
 * @return An XML description of the GObject-derivative the function is called for
 */
std::string GObject::report() {
	return toString(XMLSERIALIZATION);
}

/**************************************************************************************************/
/**
 * Converts the class to a text representation, using the currently set serialization mode for this
 * class. Note that you will have to take care yourself that serialization and de-serialization
 * happens in the same mode.
 *
 * @param serMod The desired serialization mode
 * @return A text-representation of this class (or its derivative)
 */
std::string GObject::toString(const serializationMode& serMod) {
    std::ostringstream oarchive_stream;
    GObject *local = this; // Serialization should happen through a base pointer

    switch(serMod)
    {
    case TEXTSERIALIZATION:
		{
			boost::archive::text_oarchive oa(oarchive_stream);
			oa << boost::serialization::make_nvp("classhierarchyFromGObject", local);
		} // note: explicit scope here is essential so the oa-destructor gets called

		break;

    case XMLSERIALIZATION:
		{
			boost::archive::xml_oarchive oa(oarchive_stream);
			oa << boost::serialization::make_nvp("classhierarchyFromGObject", local);
		} // note: explicit scope here is essential so the oa-destructor gets called

		break;

    case BINARYSERIALIZATION:
		{
			boost::archive::binary_oarchive oa(oarchive_stream);
			oa << boost::serialization::make_nvp("classhierarchyFromGObject", local);
		} // note: explicit scope here is essential so the oa-destructor gets called

		break;
    }

    return oarchive_stream.str();
}

/**************************************************************************************************/
/**
 * Initializes the object from its string representation, using the currently set serialization mode.
 * Note that the string will likely describe a derivative of GObject, as GObject cannot be instantiated.
 * Note also that you will have to take care yourself that serialization and de-serialization happens
 * in the same mode.
 *
 * @param descr A text representation of a GObject-derivative
 * @param serMod The desired serialization mode
 */
void GObject::fromString(const std::string& descr, const serializationMode& serMod) {
    std::istringstream istr(descr);
    // De-serialization and serialization should happen through a pointer to the same type.
    GObject *local = (GObject *)NULL;

    switch(serMod)
     {
     case TEXTSERIALIZATION:
 		{
		    boost::archive::text_iarchive ia(istr);
		    ia >> boost::serialization::make_nvp("classhierarchyFromGObject", local);
 		} // note: explicit scope here is essential so the ia-destructor gets called

 		break;

     case XMLSERIALIZATION:
		{
		    boost::archive::xml_iarchive ia(istr);
		    ia >> boost::serialization::make_nvp("classhierarchyFromGObject", local);
		} // note: explicit scope here is essential so the ia-destructor gets called

		break;

     case BINARYSERIALIZATION:
 		{
		    boost::archive::binary_iarchive ia(istr);
		    ia >> boost::serialization::make_nvp("classhierarchyFromGObject", local);
 		}

 		break;
     }

    this->load(local);
    if(local) delete local;
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

} /* namespace GenEvA */
} /* namespace Gem */
