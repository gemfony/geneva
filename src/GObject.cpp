/**
 * @file GObject.cpp
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

#include "GObject.hpp"

namespace Gem {
namespace GenEvA {

/**************************************************************************************************/
/**
 * The default constructor initializes the internal values of this class.
 * In particular, it sets the name of the Geneva object to "GObject"
 */
GObject::GObject()
{ /* nothing */ }

/**************************************************************************************************/
/**
 * The copy constructor
 *
 * @param cp A copy of another GObject object
 */
GObject::GObject(const GObject& cp)
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
 * A standard assignment operator
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
 * Checks for equality with another GObject object
 *
 * @param  cp A constant reference to another GObject object
 * @return A boolean indicating whether both objects are equal
 */
bool  GObject::operator==(const GObject& cp) const {
	return GObject::isEqualTo(cp, boost::logic::indeterminate);
}

/**************************************************************************************************/
/**
 * Checks inequality with another GObject object
 *
 * @param  cp A constant reference to another GObject object
 * @return A boolean indicating whether both objects are inequal
 */
bool  GObject::operator!=(const GObject& cp) const {
	return !GObject::isEqualTo(cp, boost::logic::indeterminate);
}

/**************************************************************************************************/
/**
 * Checks for equality with another GObject object. Equality means equality of all local data
 * plus the parent class'es data.
 *
 * @param  cp A constant reference to another GObject object
 * @param expected Indicates whether we expect the condition to be true or false, or whether any reporting should be suppressed ("ignore")
 * @return A boolean indicating whether both objects are equal
 */
bool  GObject::isEqualTo(const GObject& cp,  const boost::logic::tribool& expected) const {
    using namespace Gem::Util;

    // No local data, hence nothing to do

    return true;
}

/**************************************************************************************************/
/**
 * Checks for inequality with another GObject object (or a derivative thereof).
 * Equality means equality of all local data plus the parent class'es data. While this
 * function may appear as syntactic sugar, the statement !isSimilarTo is very easy
 * to read wrong.
 *
 * @param  cp A constant reference to another GObject object
 * @param expected Indicates whether we expect the condition to be true or false, or whether any reporting should be suppressed ("ignore")
 * @return A boolean indicating whether both objects are equal
 */
bool  GObject::isNotEqualTo(const GObject& cp,  const boost::logic::tribool& expected) const {
	return !this->isEqualTo(cp, expected);
}

/**************************************************************************************************/
/**
 * Checks for similarity  with another GObject object. This only plays a role for objects that
 * also contain double values. This feature is mainly used for testing purposes, particularly
 * when text i/o is done. Text i/o usually implies a loss in precision for floating point numbers.
 *
 * @param  cp A constant reference to another GObject object
 * @param limit A double indicating the level of acceptable deviation of two double values
 * @param expected Indicates whether we expect the condition to be true or false, or whether any reporting should be suppressed ("ignore")
 * @return A boolean indicating whether both objects are similar or not
 */
bool  GObject::isSimilarTo(const GObject& cp, const double& limit,  const boost::logic::tribool& expected) const {
    using namespace Gem::Util;

    // No local data, hence nothing to do

	return true;
}

/**************************************************************************************************/
/**
 * Checks for dissimilarity  with another GObject object. This only plays a role for objects that
 * also contain double values. This feature is mainly used for testing purposes, particularly
 * when text i/o is done. Text i/o usually implies a loss in precision for floating point numbers.
 * While this function may appear as syntactic sugar, the statement !isSimilarTo is very easy
 * to read wrong.
 *
 * NOTE: Something is fishy with this function. Changing the !GObject::isSimilarTo() to
 * GObject::isNotSimilarTo in GAdaptorT leads to a memory access violation. Infinite recursion ???
 *
 * @param  cp A constant reference to another GObject object
 * @param limit A double indicating the level of acceptable deviation of two double values
 * @param expected Indicates whether we expect the condition to be true or false, or whether any reporting should be suppressed ("ignore")
 * @return A boolean indicating whether both objects are similar or not
 */
bool  GObject::isNotSimilarTo(const GObject& cp, const double& limit,  const boost::logic::tribool& expected) const {
	return !this->isSimilarTo(cp, limit, expected);
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
 * Converts class to a serial representation that is then written to a stream.
 *
 * @param oarchive_stream The output stream the object should be written to
 * @param serMod The desired serialization mode
 */
void GObject::toStream(std::ostream& oarchive_stream, const serializationMode& serMod) {
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
}

/**************************************************************************************************/
/**
 * Loads the object from a stream.
 *
 * @param istr The stream from which the object should be loaded
 * @param serMod The desired serialization mode
 *
 */
void GObject::fromStream(std::istream& istr, const serializationMode& serMod) {
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
 		} // note: explicit scope here is essential so the ia-destructor gets called

 		break;
     }

    this->load(local);
    if(local) delete local;
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
	this->toStream(oarchive_stream, serMod);
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

 */
void GObject::fromString(const std::string& descr, const serializationMode& serMod) {
    std::istringstream istr(descr);
    this->fromStream(istr, serMod);
}

/**************************************************************************************************/
/**
 * Writes a serial representation of this object to a file. Can be used for check-pointing.
 *
 * @param fileName The name of the file the object should be saved to.
 * @param serMod The desired serialization mode
 */
void GObject::toFile(const std::string& fileName, const serializationMode& serMod) {
	std::ofstream checkpointStream(fileName.c_str());
	this->toStream(checkpointStream, serMod);
	checkpointStream.close();
}

/**************************************************************************************************/
/**
 * Loads a serial representation of this object from file. Can be used for check-pointing.
 *
 * @param fileName The name of the file the object should be loaded from
 * @param serMod The desired serialization mode
 */
void GObject::fromFile(const std::string& fileName, const serializationMode& serMod) {
	std::ifstream checkpointStream(fileName.c_str());
	this->fromStream(checkpointStream, serMod);
}

/**************************************************************************************************/
/**
 * Loads the data of another GObject
 *
 * @param cp A pointer to another GObject object
 */
void GObject::load(const GObject *cp) {
#ifdef DEBUG
	// Check that this object is not accidentally assigned to itself.
	if (cp == this) {
		// Compose an error message
		std::ostringstream str;
		str << "In GObject::load: Error! Object was assigned to itself";

		// throw an exception. Add some information so that if the exception
		// is caught through a base object, no information is lost
		throw geneva_error_condition(str.str());
	}
#endif /* DEBUG */

	// No local data, hence nothing to do
}

/**************************************************************************************************/

} /* namespace GenEvA */
} /* namespace Gem */
