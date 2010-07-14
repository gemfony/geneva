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
	load_(&cp);
	return *this;
}

/**************************************************************************************************/
/**
 * Checks whether a given expectation for the relationship between this object and another object
 * is fulfilled.
 *
 * @param cp A constant reference to another GObject object
 * @param e The expected outcome of the comparison
 * @param limit The maximum deviation for floating point values (important for similarity checks)
 * @param caller An identifier for the calling entity
 * @param y_name An identifier for the object that should be compared to this one
 * @param withMessages Whether or not information should be emitted in case of deviations from the expected outcome
 * @return A boost::optional<std::string> object that holds a descriptive string if expectations were not met
 */
boost::optional<std::string> GObject::checkRelationshipWith(const GObject& cp,
					                                        const Gem::Common::expectation& e,
					                                        const double& limit,
					                                        const std::string& caller,
					                                        const std::string& y_name,
					                                        const bool& withMessages) const
{
	// no local data, hence nothing to do
	return boost::optional<std::string>(); // default constructor equates to "false"
}

/**************************************************************************************************/
/**
 * Returns an XML description of the derivative it is called for
 *
 * @return An XML description of the GObject-derivative the function is called for
 */
std::string GObject::report() {
	return toString(Gem::Common::SERIALIZATIONMODE_XML);
}

/**************************************************************************************************/
/**
 * Converts class to a serial representation that is then written to a stream.
 *
 * @param oarchive_stream The output stream the object should be written to
 * @param serMod The desired serialization mode
 */
void GObject::toStream(std::ostream& oarchive_stream, const Gem::Common::serializationMode& serMod) {
    GObject *local = this; // Serialization should happen through a base pointer

    switch(serMod)
    {
    case Gem::Common::SERIALIZATIONMODE_TEXT:
		{
			boost::archive::text_oarchive oa(oarchive_stream);
			oa << boost::serialization::make_nvp("classhierarchyFromGObject", local);
		} // note: explicit scope here is essential so the oa-destructor gets called

		break;

    case Gem::Common::SERIALIZATIONMODE_XML:
		{
			boost::archive::xml_oarchive oa(oarchive_stream);
			oa << boost::serialization::make_nvp("classhierarchyFromGObject", local);
		} // note: explicit scope here is essential so the oa-destructor gets called

		break;

    case Gem::Common::SERIALIZATIONMODE_BINARY:
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
void GObject::fromStream(std::istream& istr, const Gem::Common::serializationMode& serMod) {
    // De-serialization and serialization should happen through a pointer to the same type.
    GObject *local = (GObject *)NULL;

    switch(serMod)
     {
     case Gem::Common::SERIALIZATIONMODE_TEXT:
 		{
		    boost::archive::text_iarchive ia(istr);
		    ia >> boost::serialization::make_nvp("classhierarchyFromGObject", local);
 		} // note: explicit scope here is essential so the ia-destructor gets called

 		break;

     case Gem::Common::SERIALIZATIONMODE_XML:
		{
		    boost::archive::xml_iarchive ia(istr);
		    ia >> boost::serialization::make_nvp("classhierarchyFromGObject", local);
		} // note: explicit scope here is essential so the ia-destructor gets called

		break;

     case Gem::Common::SERIALIZATIONMODE_BINARY:
 		{
		    boost::archive::binary_iarchive ia(istr);
		    ia >> boost::serialization::make_nvp("classhierarchyFromGObject", local);
 		} // note: explicit scope here is essential so the ia-destructor gets called

 		break;
     }

    load_(local);
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
std::string GObject::toString(const Gem::Common::serializationMode& serMod) {
	std::ostringstream oarchive_stream;
	toStream(oarchive_stream, serMod);
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
void GObject::fromString(const std::string& descr, const Gem::Common::serializationMode& serMod) {
    std::istringstream istr(descr);
    fromStream(istr, serMod);
}

/**************************************************************************************************/
/**
 * Writes a serial representation of this object to a file. Can be used for check-pointing.
 *
 * @param fileName The name of the file the object should be saved to.
 * @param serMod The desired serialization mode
 *
 * TODO: Error check whether the file is accessible / state of the stream
 */
void GObject::toFile(const std::string& fileName, const Gem::Common::serializationMode& serMod) {
	std::ofstream checkpointStream(fileName.c_str());
	toStream(checkpointStream, serMod);
	checkpointStream.close();
}

/**************************************************************************************************/
/**
 * Loads a serial representation of this object from file. Can be used for check-pointing.
 *
 * @param fileName The name of the file the object should be loaded from
 * @param serMod The desired serialization mode
 *
 * TODO: Error check whether the file is accessible / state of the stream
 */
void GObject::fromFile(const std::string& fileName, const Gem::Common::serializationMode& serMod) {
	std::ifstream checkpointStream(fileName.c_str());
	fromStream(checkpointStream, serMod);
}

/**************************************************************************************************/
/**
 * Loads the data of another GObject
 *
 * @param cp A pointer to another GObject object
 */
void GObject::load_(const GObject *cp) {
	// Checks whether we are accidently assigning the object to itself
	selfAssignmentCheck<GObject>(cp);

	// No local data, hence nothing to do
}

/**************************************************************************************************/
/**
 * A specialization of the general clone for cases where no conversion takes place at all
 *
 * @return A boost::shared_ptr<GObject> to a clone of the derived object
 */
template <> boost::shared_ptr<GObject> GObject::clone<GObject>(
		boost::enable_if<boost::is_base_of<Gem::GenEvA::GObject, GObject> >::type* dummy
) const {
	return boost::shared_ptr<GObject>(clone_());
}

/**************************************************************************************************/
/**
 * Creates a deep clone of this object, storing it in a boost::shared_ptr<GObject>
 *
 * @return A boost::shared_ptr<GObject> to a clone of the derived object
 */
boost::shared_ptr<GObject> GObject::clone() const {
	return boost::shared_ptr<GObject>(clone_());
}

/**************************************************************************************************/

} /* namespace GenEvA */
} /* namespace Gem */
