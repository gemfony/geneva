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
#include <cmath>
#include <typeinfo>
#include <map>
#include <vector>

// Boost header files go here

#include <boost/version.hpp>
#include "GGlobalDefines.hpp"
#if BOOST_VERSION < ALLOWED_BOOST_VERSION
#error "Error: Boost has incorrect version !"
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
    void serialize(Archive & ar, const unsigned int){
      using boost::serialization::make_nvp;
      ar & make_nvp("name_",GObject::name_);
    }
    ///////////////////////////////////////////////////////////////////////

public:
	/** @brief The default constructor */
	GObject() ;
	/** @brief Initialization by name */
	explicit GObject(const std::string&) ;
	/** @brief The copy constructor */
	GObject(const GObject& cp) ;
	/** @brief The destructor */
	virtual ~GObject();

	/** @brief A standard assignment operator */
	const GObject& operator=(const GObject&);

	/** @brief Checks for equality with another GObject object */
	bool operator==(const GObject&) const;
	/** @brief Checks inquality with another GObject object */
	bool operator!=(const GObject&) const;
	/** @brief Checks for equality with another GObject object */
	virtual bool isEqualTo(const GObject&) const;
	/** @brief Checks for inequality with another GObject object (or a derivative) */
	virtual bool isNotEqualTo(const GObject&) const;
	/** @brief Checks for similarity with another GObject object */
	virtual bool isSimilarTo(const GObject&, const double&) const;
	/** @brief Checks for disimilarity with another GObject object (or a derivative) */
	virtual bool isNotSimilarTo(const GObject&, const double&) const;

	/** @brief Convert class to a serial representation, using a user-specified serialization mode */
	std::string toString(const serializationMode& serMod);
	/** @brief Convert class to a serial representation, using a specific serialization mode */
	void fromString(const std::string&, const serializationMode& serMod);

	/** @brief Creates a deep clone of this object */
	virtual GObject* clone() const = 0;
	/** @brief Loads the data of another GObject */
	virtual void load(const GObject*);

	/** @brief Returns an XML description of the derivative it is called for */
	std::string report();

	/** @brief Retrieve the name of this class */
	std::string name() const ;
	/** @brief Give the class a name */
	void setName(const std::string&) ;

	/**************************************************************************************************/
	/**
	 * The function creates a clone of the GObject pointer and converts it to a pointer to a derived class.
	 * This work and the corresponding error checks are centralized in this function. Conversion will be faster
	 * if we do not compile in DEBUG mode.
	 *
	 * @return A converted clone of this object
	 */
	template <typename clone_type>
	clone_type* clone_ptr_cast() const {
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
	boost::shared_ptr<clone_type> clone_bptr_cast() const {
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
     * its own instance. Note that the actual calculation is done in
     * a random number server. Also note that the GRandom
     * object will not be serialized.
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
	const load_type* conversion_cast(const GObject *load_ptr, const load_type* This) const {
#ifdef DEBUG
		const load_type *result = dynamic_cast<const load_type *> (load_ptr);

		// dynamic_cast will emit a NULL pointer, if the conversion failed
		if (!result) {
			std::ostringstream error;
			error << "In GObject::conversion_cast<T>() : Conversion error!" << std::endl;

			// throw an exception. Add some information so that if the exception
			// is caught through a base object, no information is lost.
			throw geneva_error_condition(error.str());
		}

		// Check that this object is not accidentally assigned to itself.
		if (load_ptr == This) {
			std::ostringstream error;
			error << "In GObject::conversion_cast<T>() : Error!" << std::endl
					<< "Tried to assign an object to itself." << std::endl;

			throw geneva_error_condition(error.str());
		}

		return result;
#else
		return static_cast<const load_type *>(load_ptr);
#endif
	}

	/**************************************************************************************************/
	/**
	 * Checks for inequality of the two arguments, which are assumed to be basic types. This is
	 * needed by the isEqualTo function, so we have a standardized way of emitting information
	 * on deviations. Note: If you want specific behavior for a particular type then you can always
	 * create a specialization of this function.
	 *
	 * @param className The name of the calling class
	 * @param x The first parameter to be compared
	 * @param y The second parameter to be compared
	 * @param x_name The name of the first parameter
	 * @param y_name The name of the second parameter
	 * @return A boolean indicating whether any differences were found
	 */
	template <typename basic_type>
	bool checkForInequality(const std::string& className,
										          const basic_type& x,
										          const basic_type& y,
										          const std::string& x_name,
										          const std::string& y_name) const
	{
		if(x==y) return false;
		else {
#ifdef GENEVATESTING
			std::cout << "//-----------------------------------------------------------------" << std::endl
				            << "Found inequality in object of type \"" << className << "\":" << std::endl
				            << x_name << " (type " << typeid(x).name() << ") = " << x << std::endl
				            << y_name << " (type " << typeid(y).name() << ") = " << y << std::endl;
#endif /* GENEVATESTING */
			return true;
		}
	}

	/**************************************************************************************************/
	/**
	 * Checks for inequality of the two arguments, which are assumed to be vectors of basic types.
	 * This is needed by the isEqualTo function, so we have a standardized way of emitting information
	 * on deviations. Note: If you want specific behavior for a particular type then you can always
	 * create a specialization of this function.
	 *
	 * @param className The name of the calling class
	 * @param x The first parameter to be compared
	 * @param y The second parameter to be compared
	 * @param x_name The name of the first parameter
	 * @param y_name The name of the second parameter
	 * @return A boolean indicating whether any differences were found
	 */
	template <typename basic_type>
	bool checkForInequality(const std::string& className,
										           const std::vector<basic_type>& x,
										           const std::vector<basic_type>& y,
										           const std::string& x_name,
										           const std::string& y_name) const
	{
		if(x==y) return false;
		else {
#ifdef GENEVATESTING
			// Check sizes
			if(x.size() != y.size()) {
				std::cout << "//-----------------------------------------------------------------" << std::endl
					            << "Found inequality in object of type \"" << className << "\":" << std::endl
					            << x_name << " (type std::vector<" << typeid(basic_type).name() <<">): Size = " << x.size() << std::endl
					            << y_name << " (type std::vector<" << typeid(basic_type).name() <<">): Size = " << y.size() << std::endl;

				return true;
			}

			// Loop over all entries and find out which is wrong
			for(std::size_t i=0; i<x.size(); i++) {
				if(x.at(i) != y.at(i)) {
					std::cout << "//-----------------------------------------------------------------" << std::endl
						            << "Found inequality in object of type \"" << className << "\":" << std::endl
						            << x_name << "[" << i << "] (type std::vector<" << typeid(basic_type).name() <<">) " << x << std::endl
						            << y_name << "[" << i << "] (type std::vector<" << typeid(basic_type).name() <<">) " << y << std::endl;

					return true;
				}
			}

#endif /* GENEVATESTING */
			return true;
		}
	}

	/**************************************************************************************************/
	/**
	 * Checks for inequality of the two arguments, which are assumed to be vectors of boost::share_ptr
	 * objects of complex. This is needed by the isEqualTo function, so we have a standardized way of
	 * emitting information on deviations. Note: If you want specific behavior for a particular type then you
	 * can always create a specialization of this function.
	 *
	 * @param className The name of the calling class
	 * @param x The first parameter to be compared
	 * @param y The second parameter to be compared
	 * @param x_name The name of the first parameter
	 * @param y_name The name of the second parameter
	 * @return A boolean indicating whether any differences were found
	 */
	template <typename complex_type>
	bool checkForInequality(const std::string& className,
										           const std::vector<boost::shared_ptr<complex_type> >& x,
										           const std::vector<boost::shared_ptr<complex_type> >& y,
										           const std::string& x_name,
										           const std::string& y_name) const
	{
		// Check sizes
		if(x.size() != y.size()) {
#ifdef GENEVATESTING
			std::cout << "//-----------------------------------------------------------------" << std::endl
							<< "Found inequality in object of type \"" << className << "\":" << std::endl
							<< x_name << " (type std::vector<boost::shared_ptr<" << typeid(complex_type).name() <<"> >): Size = " << x.size() << std::endl
							<< y_name << " (type std::vector<boost::shared_ptr<" << typeid(complex_type).name() <<"> >): Size = " << y.size() << std::endl;
#endif /* GENEVATESTING */
			return true;
		}

		// Loop over all entries and find out which is wrong
		typename std::vector<boost::shared_ptr<complex_type> >::const_iterator cit_x;
		typename std::vector<boost::shared_ptr<complex_type> >::const_iterator cit_y;
		std::size_t i=0;
		for(cit_x=x.begin(), cit_y=y.begin(); cit_x != x.end(), cit_y != y.end(); ++cit_x, ++cit_y) {
			if(!(*cit_x)->isEqualTo(**cit_y)) {
#ifdef GENEVATESTING
				std::cout << "//-----------------------------------------------------------------" << std::endl
								<< "Found inequality in object of type \"" << className << "\":" << std::endl
								<< x_name << "[" << i << "] (type std::vector<boost::shared_ptr<" << typeid(complex_type).name() <<"> >) " << std::endl
								<< y_name << "[" << i << "] (type std::vector<boost:.shared_ptr<" << typeid(complex_type).name() <<"> >) " << std::endl;
#endif /* GENEVATESTING */

				i++;
				return true;
			}
		}

		return false;
	}

	/**************************************************************************************************/
	/**
	 * Checks for similarity of the two arguments, which are assumed to be basic types. This is
	 * needed by the isSimilarTo function, so we have a standardized way of emitting information on
	 * deviations.  By default all types are just checked for equality. A specialization exists for
	 * typeof(basic_type) == typeof(double). Note that: If you want specific behaviour for any other
	 * type then you can always create a specialization of this function.
	 *
	 * @param className The name of the calling class
	 * @param x The first parameter to be compared
	 * @param y The second parameter to be compared
	 * @param limit The acceptable deviation of x and y
	 * @param x_name The name of the first parameter
	 * @param y_name The name of the second parameter
	 * @return A boolean indicating whether any differences were found
	 */
	template <typename basic_type>
	bool checkForDissimilarity(const std::string& className,
													  const basic_type& x,
													  const basic_type& y,
													  const double& limit,
													  const std::string& x_name,
													  const std::string& y_name) const
	{
		if(x==y) return false;
		else {
#ifdef GENEVATESTING
			std::cout << "//-----------------------------------------------------------------" << std::endl
				            << "Found dissimilarity in object of type \"" << className << "\":" << std::endl
				            << x_name << " (type " << typeid(x).name() << ") = " << x << std::endl
				            << y_name << " (type " << typeid(y).name() << ") = " << y << std::endl;
#endif /* GENEVATESTING */
			return true;
		}
	}

	/**************************************************************************************************/
	/**
	 * Checks for dissimilarity of the two arguments, which are assumed to be vectors of basic types.
	 * This is needed by the isSimilarTo function, so we have a standardized way of emitting information
	 * on deviations. Note: If you want specific behavior for a particular type then you can always
	 * create a specialization of this function. One such specialization exists for
	 * typef(basic_type) == typeof(double).
	 *
	 * @param className The name of the calling class
	 * @param x The first parameter to be compared
	 * @param y The second parameter to be compared
	 * @param x_name The name of the first parameter
	 * @param y_name The name of the second parameter
	 * @return A boolean indicating whether any differences were found
	 */
	template <typename basic_type>
	bool checkForDissimilarity(const std::string& className,
										           const std::vector<basic_type>& x,
										           const std::vector<basic_type>& y,
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
					            << x_name << " (type std::vector<" << typeid(basic_type).name() <<">): Size = " << x.size() << std::endl
					            << y_name << " (type std::vector<" << typeid(basic_type).name() <<">): Size = " << y.size() << std::endl;
			}

			// Loop over all entries and find out which is wrong
			for(std::size_t i=0; i<x.size(); i++) {
				if(x.at(i) != y.at(i)) {
					std::cout << "//-----------------------------------------------------------------" << std::endl
						            << "Found dissimilarity in object of type \"" << className << "\":" << std::endl
						            << x_name << "[" << i << "] (type std::vector<" << typeid(basic_type).name() <<">) " << x << std::endl
						            << y_name << "[" << i << "] (type std::vector<" << typeid(basic_type).name() <<">) " << y << std::endl;

					return true;
				}
			}

#endif /* GENEVATESTING */
			return true;
		}
	}

	/**************************************************************************************************/
	/**
	 * Checks for dissimilarity of the two arguments, which are assumed to be vectors of boost::share_ptr
	 * objects of complex. This is needed by the isEqualTo function, so we have a standardized way of
	 * emitting information on deviations. Note: If you want specific behavior for a particular type then you
	 * can always create a specialization of this function.
	 *
	 * @param className The name of the calling class
	 * @param x The first parameter to be compared
	 * @param y The second parameter to be compared
	 * @param limit Acceptable limits for deviation of the two parameters
	 * @param x_name The name of the first parameter
	 * @param y_name The name of the second parameter
	 * @return A boolean indicating whether any differences were found
	 */
	template <typename complex_type>
	bool checkForDissimilarity(const std::string& className,
										           const std::vector<boost::shared_ptr<complex_type> >& x,
										           const std::vector<boost::shared_ptr<complex_type> >& y,
										           const double& limit,
										           const std::string& x_name,
										           const std::string& y_name) const
	{
		// Check sizes
		if(x.size() != y.size()) {
#ifdef GENEVATESTING
			std::cout << "//-----------------------------------------------------------------" << std::endl
							<< "Found inequality in object of type \"" << className << "\":" << std::endl
							<< x_name << " (type std::vector<boost::shared_ptr<" << typeid(complex_type).name() <<"> >): Size = " << x.size() << std::endl
							<< y_name << " (type std::vector<boost::shared_ptr<" << typeid(complex_type).name() <<"> >): Size = " << y.size() << std::endl;
#endif /* GENEVATESTING */
			return true;
		}

		// Loop over all entries and find out which is wrong
		typename std::vector<boost::shared_ptr<complex_type> >::const_iterator cit_x;
		typename std::vector<boost::shared_ptr<complex_type> >::const_iterator cit_y;
		std::size_t i=0;
		for(cit_x=x.begin(), cit_y=y.begin(); cit_x != x.end(), cit_y != y.end(); ++cit_x, ++cit_y) {
			if(!(*cit_x)->isSimilarTo(**cit_y, limit)) {
#ifdef GENEVATESTING
				std::cout << "//-----------------------------------------------------------------" << std::endl
								<< "Found inequality in object of type \"" << className << "\":" << std::endl
								<< x_name << "[" << i << "] (type std::vector<boost::shared_ptr<" << typeid(complex_type).name() <<"> >) " << std::endl
								<< y_name << "[" << i << "] (type std::vector<boost:.shared_ptr<" << typeid(complex_type).name() <<"> >) " << std::endl;
#endif /* GENEVATESTING */

				i++;
				return true;
			}
		}

		return false;
	}

	/**************************************************************************************************/

private:
	std::string name_; ///< Allows to assign a name to this object
};

/*
 * Specializations of some template functions
 */
template <> bool GObject::checkForDissimilarity<double>(const std::string&, const double&,	const double&, const double&, const std::string&, const std::string&) const;
template <> bool GObject::checkForDissimilarity<std::map<std::string, std::string> >(const std::string&, const std::map<std::string, std::string>&, const std::map<std::string, std::string>&, const double&, const std::string&, const std::string&) const;
template <> bool GObject::checkForInequality<std::map<std::string, std::string> >(const std::string&, const std::map<std::string, std::string>&,	const std::map<std::string, std::string>&, const std::string&, const std::string&) const;
template <> bool GObject::checkForDissimilarity<double>(const std::string&,  const std::vector<double>&,  const std::vector<double>&,  const double&,  const std::string&, const std::string&) const;

} /* namespace GenEvA */
} /* namespace Gem */

/**************************************************************************************************/
/**
 * @brief Needed for Boost.Serialization
 */
BOOST_SERIALIZATION_ASSUME_ABSTRACT(Gem::GenEvA::GObject)

/**************************************************************************************************/

#endif /* GOBJECT_HPP_ */
