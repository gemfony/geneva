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
GObject::GObject()
	:rnrGenerationMode_(Gem::Util::RNRFACTORY)
{
	gr.setRNRFactoryMode();
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
GObject::GObject(const GObject& cp)
	:rnrGenerationMode_(cp.rnrGenerationMode_)
{
    switch(rnrGenerationMode_) {
    case Gem::Util::RNRFACTORY:
    	gr.setRNRFactoryMode();
    	break;

    case Gem::Util::RNRLOCAL:
    	gr.setRNRLocalMode();
    	break;
    };
}

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

	if(checkForInequality("GObject", rnrGenerationMode_, cp.rnrGenerationMode_,"rnrGenerationMode_", "cp.rnrGenerationMode_", expected)) return false;

	else return true;
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

	if(checkForDissimilarity("GObject", rnrGenerationMode_, cp.rnrGenerationMode_,limit, "rnrGenerationMode_", "cp.rnrGenerationMode_", expected)) return false;

	else return true;
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
 * Determines whether production of random numbers should happen remotely (RNRFACTORY) or locally (RNRLOCAL)
 *
 * @param rnrGenMode A parameter which indicates where random numbers should be produced
 */
void GObject::setRnrGenerationMode(const Gem::Util::rnrGenerationMode& rnrGenMode) {
    switch(rnrGenerationMode_) {
    case Gem::Util::RNRFACTORY:
    	gr.setRNRFactoryMode();
    	break;

    case Gem::Util::RNRLOCAL:
    	gr.setRNRLocalMode();
    	break;
    };

    // Store the value locally
    rnrGenerationMode_ = rnrGenMode;
}

/**************************************************************************************************/
/**
 * Retrieves the current value of the random number generation mode.
 *
 * @return The current value of the random number generation mode
 */
Gem::Util::rnrGenerationMode GObject::getRnrGenerationMode() const {
	return rnrGenerationMode_;
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
 		} // note: explicit scope here is essential so the ia-destructor gets called

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


	// Adjust the production place, if necessary
	if(rnrGenerationMode_ != cp->rnrGenerationMode_) {
		rnrGenerationMode_ = cp->rnrGenerationMode_;

		switch(rnrGenerationMode_) {
		case Gem::Util::RNRFACTORY:
			gr.setRNRFactoryMode();
			break;

		case Gem::Util::RNRLOCAL:
			gr.setRNRLocalMode();
			break;
		};
	}
}

/**************************************************************************************************/

} /* namespace GenEvA */
} /* namespace Gem */
