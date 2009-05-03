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
GObject::GObject()  :
	name_("GObject")
{ /* nothing */ }

/**************************************************************************************************/
/**
 * If a particular name is desired for an object of this class, it can be
 * set upon initialization with this function.
 *
 * @param geneva_object_name The name which is assigned to a Geneva object
 */
GObject::GObject(const std::string& geneva_object_name)  :
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
GObject::GObject(const GObject& cp)  :
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
	return GObject::isEqualTo(cp);
}

/**************************************************************************************************/
/**
 * Checks inequality with another GObject object
 *
 * @param  cp A constant reference to another GObject object
 * @return A boolean indicating whether both objects are inequal
 */
bool  GObject::operator!=(const GObject& cp) const {
	return !GObject::isEqualTo(cp);
}

/**************************************************************************************************/
/**
 * Checks for equality with another GObject object. Equality means equality of all local data
 * plus the parent class'es data.
 *
 * @param  cp A constant reference to another GObject object
 * @return A boolean indicating whether both objects are equal
 */
bool  GObject::isEqualTo(const GObject& cp) const {
	if(GObject::checkForInequality("GObject", name_, cp.name_,"name_", "cp.name_")) return false;
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
 * @return A boolean indicating whether both objects are equal
 */
bool  GObject::isNotEqualTo(const GObject& cp) const {
	return !this->isEqualTo(cp);
}

/**************************************************************************************************/
/**
 * Checks for similarity  with another GObject object. This only plays a role for objects that
 * also contain double values. This feature is mainly used for testing purposes, particularly
 * when text i/o is done. Text i/o usually implies a loss in precision for floating point numbers.
 *
 * @param  cp A constant reference to another GObject object
 * @param limit A double indicating the level of acceptable deviation of two double values
 * @return A boolean indicating whether both objects are similar or not
 */
bool  GObject::isSimilarTo(const GObject& cp, const double& limit) const {
	if(GObject::checkForDissimilarity("GObject", name_, cp.name_,limit, "name_", "cp.name_")) return false;
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
 * @return A boolean indicating whether both objects are similar or not
 */
bool  GObject::isNotSimilarTo(const GObject& cp, const double& limit) const {
	return !this->isSimilarTo(cp, limit);
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
	// Check that this object is not accidentally assigned to itself.
	if (cp == this) {
		// Compose an error message
		std::ostringstream str;
		str << "In GObject::load: Error! Object was assigned to itself";

		// throw an exception. Add some information so that if the exception
		// is caught through a base object, no information is lost
		throw geneva_error_condition(str.str());
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
std::string GObject::name() const  {
	return name_;
}

/**************************************************************************************************/
/**
 * Allows to set a name for this class.
 *
 * @param geneva_object_name The name of this class
 */
void GObject::setName(const std::string& geneva_object_name)  {
	name_ = geneva_object_name;
}

/**************************************************************************************************/
/**
 * Specialization for typeof(basic_type) == typeof(double).
 *
 * @param className The name of the calling class
 * @param x The first parameter to be compared
 * @param y The second parameter to be compared
 * @param limit The acceptable deviation of x and y
 * @param x_name The name of the first parameter
 * @param y_name The name of the second parameter
 * @return A boolean indicating whether any differences were found
 */
template <>
bool GObject::checkForDissimilarity<double>(const std::string& className,
															                       const double& x,
															                       const double& y,
															                       const double& limit,
															                       const std::string& x_name,
															                       const std::string& y_name) const
{
	if(fabs(x-y) <= fabs(limit)) return false;
	else {
#ifdef GENEVATESTING
		std::cout << "//-----------------------------------------------------------------" << std::endl
			            << "Found dissimilarity in object of type \"" << className << "\":" << std::endl
			            << x_name << " (type " << typeid(x).name() << ") = " << x << std::endl
			            << y_name << " (type " << typeid(y).name() << ") = " << y << std::endl
			            << "limit = " << limit << std::endl;
#endif /* GENEVATESTING */
		return true;
	}
}

/**************************************************************************************************/
/**
 * Specialization for typeof(basic_type) == typeof(std::map<std::string, std::string>).
 *
 * @param className The name of the calling class
 * @param x The first parameter to be compared
 * @param y The second parameter to be compared
 * @param limit The acceptable deviation of x and y
 * @param x_name The name of the first parameter
 * @param y_name The name of the second parameter
 * @return A boolean indicating whether any differences were found
 */
template <>
bool GObject::checkForDissimilarity<std::map<std::string, std::string> >(const std::string& className,
		                                                                                                                     const std::map<std::string, std::string>& x,
		                                                                                                                     const std::map<std::string, std::string>& y,
		                                                                                                                     const double& limit,
		                                                                                                                     const std::string& x_name,
		                                                                                                                     const std::string& y_name) const
 {
	// Check sizes
	if(x.size() != y.size()) {
#ifdef GENEVATESTING
		std::cout << "//-----------------------------------------------------------------" << std::endl
			            << "Found dissimilarity in object of type \"" << className << "\":" << std::endl
			            << x_name << " (type std::map<std::string, std::string>): Size = " << x.size() << std::endl
			            << y_name << " (type std::map<std::string, std::string>): Size = " << y.size() << std::endl;
#endif /* GENEVATESTING */
		return true;
	}

	// Check individual entries
	std::map<std::string, std::string>::const_iterator cit_x, cit_y;
	for(cit_x=x.begin(), cit_y=y.begin(); cit_x!=x.end(), cit_y!=y.end(); ++cit_x, ++cit_y) {
		if(*cit_x != *cit_y) {
#ifdef GENEVATESTING
		std::cout << "//-----------------------------------------------------------------" << std::endl
			            << "Found dissimilarity in object of type \"" << className << "\":" << std::endl
			            << x_name << " (type std::map<std::string, std::string>): Key = " << cit_x->first << " value = " << cit_x->second << std::endl
			            << y_name << " (type std::map<std::string, std::string>): Key = " << cit_y->first << " value = " << cit_y->second << std::endl;
#endif /* GENEVATESTING */
			return true;
		}
	}

	return false;
 }

/**************************************************************************************************/
/**
 * Specialization for typeof(basic_type) == typeof(std::map<std::string, std::string>).
 *
 * @param className The name of the calling class
 * @param x The first parameter to be compared
 * @param y The second parameter to be compared
 * @param limit The acceptable deviation of x and y
 * @param x_name The name of the first parameter
 * @param y_name The name of the second parameter
 * @return A boolean indicating whether any differences were found
 */
template <>
bool GObject::checkForInequality<std::map<std::string, std::string> >(const std::string& className,
																														const std::map<std::string, std::string>& x,
																														const std::map<std::string, std::string>& y,
																														const std::string& x_name,
																														const std::string& y_name) const
{
	// Check sizes
	if(x.size() != y.size()) {
#ifdef GENEVATESTING
		std::cout << "//-----------------------------------------------------------------" << std::endl
			            << "Found inequality in object of type \"" << className << "\":" << std::endl
			            << x_name << " (type std::map<std::string, std::string>): Size = " << x.size() << std::endl
			            << y_name << " (type std::map<std::string, std::string>): Size = " << y.size() << std::endl;
#endif /* GENEVATESTING */
		return true;
	}

	// Check individual entries
	std::map<std::string, std::string>::const_iterator cit_x, cit_y;
	for(cit_x=x.begin(), cit_y=y.begin(); cit_x!=x.end(), cit_y!=y.end(); ++cit_x, ++cit_y) {
		if(*cit_x != *cit_y) {
#ifdef GENEVATESTING
		std::cout << "//-----------------------------------------------------------------" << std::endl
			            << "Found inequality in object of type \"" << className << "\":" << std::endl
			            << x_name << " (type std::map<std::string, std::string>): Key = " << cit_x->first << " value = " << cit_x->second << std::endl
			            << y_name << " (type std::map<std::string, std::string>): Key = " << cit_y->first << " value = " << cit_y->second << std::endl;
#endif /* GENEVATESTING */
			return true;
		}
	}

	return false;
}

/**************************************************************************************************/
/**
 * Checks for dissimilarity of the two arguments, which are assumed to be vectors of doubles.
 * This is needed by the isSimilarTo function, so we have a standardized way of emitting information
 * on deviations.
 *
 * @param className The name of the calling class
 * @param x The first parameter to be compared
 * @param y The second parameter to be compared
 * @param x_name The name of the first parameter
 * @param y_name The name of the second parameter
 * @return A boolean indicating whether any differences were found
 */
template <>
bool GObject::checkForDissimilarity<double>(const std::string& className,
									           const std::vector<double>& x,
									           const std::vector<double>& y,
									           const double& limit,
									           const std::string& x_name,
									           const std::string& y_name) const
{
	if(x==y) return false;
	else {
#ifdef GENEVATESTING
		// Check sizes
		if(x.size() != y.size()) {
			std::cout << "//-----------------------------------------------------------------" << std::endl
				            << "Found dissimilarity in object of type \"" << className << "\":" << std::endl
				            << x_name << " (type std::vector<double>): Size = " << x.size() << std::endl
				            << y_name << " (type std::vector<double>): Size = " << y.size() << std::endl;

			return true;
		}

		// Loop over all entries and find out which is wrong
		for(std::size_t i=0; i<x.size(); i++) {
			if(fabs(x.at(i) - y.at(i)) > fabs(limit)) {
				std::cout << "//-----------------------------------------------------------------" << std::endl
					            << "Found dissimilarity in object of type \"" << className << "\":" << std::endl
					            << x_name << "[" << i << "] (type std::vector<double>) " << x.at(i) << std::endl
					            << y_name << "[" << i << "] (type std::vector<double>) " << y.at(i) << std::endl
					            << "limit = " << limit << std::endl;

				return true;
			}
		}

#endif /* GENEVATESTING */
		return true;
	}
}

/**************************************************************************************************/
} /* namespace GenEvA */
} /* namespace Gem */
