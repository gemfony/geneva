/**
 * @file GDataExchange.hpp
 */

/* Copyright (C) 2009 Dr. Ruediger Berlich
 *
 * This file is part of Geneva, Gemfony scientific's optimization library.
 *
 * This file is free software: you can redistribute it and/or modify
 * it under the terms of version 3 of the GNU Affero General Public License,
 * or, at your option, under the terms of version 2 of the GNU General Public
 * License, as published by the Free Software Foundation.
 *
 * NOTE THAT THIS FORM OF DUAL-LICENSING DOES NOT APPLY TO ANY OTHER FILES
 * OF THE GENEVA LIBRARY, UNLESS THIS IS EXPLICITLY STATED IN THE CORRSPONDING FILE.
 *
 * Geneva is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of version 2 of the GNU General Public License
 * and of version 3 of the GNU Affero General Public License along with the Geneva
 * library. If not, see <http://www.gnu.org/licenses/>.
 */

// Standard headers go here
#include <vector>
#include <algorithm>
#include <string>
#include <fstream>
#include <sstream>
#include <iostream>

// Boost headers go here
#include <boost/version.hpp>
#include "GGlobalDefines.hpp"
#if BOOST_VERSION < ALLOWED_BOOST_VERSION
#error "Error: Boost has incorrect version !"
#endif /* BOOST_VERSION */

#include <boost/cstdint.hpp>
#include <boost/lexical_cast.hpp>
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
#include <boost/shared_ptr.hpp>
#include <boost/bind.hpp>

#ifndef GDATAEXCHANGE_HPP_
#define GDATAEXCHANGE_HPP_

#include "GNumericParameterT.hpp"

#include "GBoolParameter.hpp"
#include "GCharParameter.hpp"
#include "GDoubleParameter.hpp"
#include "GLongParameter.hpp"
#include "GHelperFunctionsT.hpp"

namespace Gem
{
namespace Util
{

/*******************************************************************************/
/**
 * This class serves as an exchange vehicle between external programs and
 * the Geneva library. It allows to store and load parameters particular to a given
 * individual. Particular storage formats can be re-defined in derived classes
 * in order to accommodate "foreign" exchange formats. The class itself only
 * implements a very simple format, where all data is stored as ASCII data
 * consecutively in a file.
 */
class GDataExchange {
    ///////////////////////////////////////////////////////////////////////
    friend class boost::serialization::access;

    template<typename Archive>
	void save(Archive & ar, const unsigned int version) const {
        using boost::serialization::make_nvp;
        ar & make_nvp("parameterValueSet_", parameterValueSet_);

        std::size_t currentPosition = current_ - parameterValueSet_.begin();
        ar & make_nvp("currentPosition_", currentPosition);
    }

    template<typename Archive>
    void load(Archive & ar, const unsigned int version){
    	using boost::serialization::make_nvp;
    	ar & make_nvp("parameterValueSet_", parameterValueSet_);

    	std::size_t currentPosition;
    	ar & make_nvp("currentPosition_", currentPosition);
    	current_ = parameterValueSet_.begin() + currentPosition;
    }

    BOOST_SERIALIZATION_SPLIT_MEMBER()
    ///////////////////////////////////////////////////////////////////////

public:
	/** @brief The default constructor */
	GDataExchange();
	/** @brief A standard copy constructor */
	GDataExchange(const GDataExchange&);
	/** @brief The destructor */
	virtual ~GDataExchange();

	/** @brief A standard assignment operator */
	const GDataExchange& operator=(const GDataExchange&);

	/** @brief Resets the current parameter set */
	void reset();
	/** @brief Resets all parameter sets in sequence */
	void resetAll();

	/** @brief Sorts the data sets according to their values */
	void sort(const bool& ascending=true);

	/** @brief Set the precision of ASCII IO of FP numbers */
	void setPrecision(const std::streamsize&);

	/** @brief Assign a value to the current data set */
	void setValue(double);
	/** @brief Retrieve value of the current data set */
	double value();
	/** @brief Check whether the current data set has a value */
	bool hasValue();

	/** @brief Goes to to the start of the list */
	void gotoStart();
	/** @brief Switches to the next available data set */
	bool nextDataSet();
	/** @brief Adds a new, empty data set and adjusts the counter */
	void newDataSet();

	/** @brief Retrieves the number of data sets in the collection */
	std::size_t nDataSets();

	/**************************************************************************/
	/**
	 * Gives access to a full data set of a particular type, including its
	 * boundaries. This is a trap. The specializations should be used instead.
	 *
	 * @param pos The position of the data access is sought for
	 * @return The data access was sought for
	 */
	template <typename T>
	boost::shared_ptr<GNumericParameterT<T> > parameterSet_at(std::size_t pos) {
		std::cout << "In GDataExchange::parameterSet_at<T>(std::size_t): Error!" << std::endl
			      << "The function seems to have been called with a type" << std::endl
			      << "it was not designed for. Leaving ..." << std::endl;
		exit(1);

		// Make the compiler happy
		return boost::shared_ptr<GNumericParameterT<T> >(new GNumericParameterT<T>());
	}

	/**************************************************************************/
	/**
	 * Gives access to the data of a particular type.
	 * This is a trap. The specializations should be used instead.
	 *
	 * @param pos The position of the data access is sought for
	 * @return The data access was sought for
	 */
	template <typename T>
	T at(std::size_t pos) {
		std::cout << "In GDataExchange::at<T>(std::size_t): Error!" << std::endl
			      << "The function seems to have been called with a type" << std::endl
			      << "it was not designed for. Leaving ..." << std::endl;
		exit(1);

		// Make the compiler happy
		return 0.;
	}

	/**************************************************************************/
	/**
	 * Gives access to the size of a vector of a particular type.
	 * This is a trap. The specializations should be used instead.
	 *
	 * @return The size of the vector of a particular type
	 */
	template <typename T>
	std::size_t size() {
		std::cout << "In GDataExchange::size<T>(): Error!" << std::endl
			      << "The function seems to have been called with a type" << std::endl
			      << "it was not designed for. Leaving ..." << std::endl;
		exit(1);

		// Make the compiler happy
		return (std::size_t)NULL;
	}

	/**************************************************************************/
	/**
	 * Allows to append data of a given type to the corresponding vector.
	 * This is a trap. The specializations should be used instead.
	 */
	template <typename T>
	void append(const T&) {
		std::cout << "In GDataExchange::append<T>(): Error!" << std::endl
			      << "The function seems to have been called with a type" << std::endl
			      << "it was not designed for. Leaving ..." << std::endl;
		exit(1);
	}

	/**************************************************************************/
	/**
	 * Allows to append data of a given type to the corresponding vector.
	 * This is a trap. The specializations should be used instead.
	 */
	template <typename T>
	void append(const T&, const T&, const T&) {
		std::cout << "In GDataExchange::append<T>(): Error!" << std::endl
			      << "The function seems to have been called with a type" << std::endl
			      << "it was not designed for. Leaving ..." << std::endl;
		exit(1);
	}

	/**************************************************************************/
	/** @brief Adds a boost::shared_ptr<GDoubleParameter> object to the corresponding vector */
	void append(const boost::shared_ptr<GDoubleParameter>&);
	/** @brief Adds a boost::shared_ptr<GLongParameter> object to the corresponding vector */
	void append(const boost::shared_ptr<GLongParameter>&);
	/** @brief Adds a boost::shared_ptr<GBoolParameter> object to the corresponding vector */
	void append(const boost::shared_ptr<GBoolParameter>&);
	/** @brief Adds a boost::shared_ptr<GCharParameter> object to the corresponding vector */
	void append(const boost::shared_ptr<GCharParameter>&);

	/**************************************************************************/

	/** @brief Writes the class'es data to a stream */
	void writeToStream(std::ostream&) const;
	/** @brief Reads the class'es data from a stream */
	void readFromStream(std::istream&);

private:
	/**********************************************************************************/
	////////////////////////////////////////////////////////////////////////////////////
	/**********************************************************************************/
	/**
	 * An internal struct used to store a single parameter-value pair
	 */
	struct parameterValuePair {
	public:
	    ///////////////////////////////////////////////////////////////////////
	    friend class boost::serialization::access;

	    template<typename Archive>
	    void serialize(Archive & ar, const unsigned int version){
	      using boost::serialization::make_nvp;

	      ar & make_nvp("dArray_", dArray_);
	      ar & make_nvp("lArray_", lArray_);
	      ar & make_nvp("bArray_", bArray_);
	      ar & make_nvp("cArray_", cArray_);
	      ar & make_nvp("value_", value_);
	      ar & make_nvp("hasValue_", hasValue_);
	    }
	    ///////////////////////////////////////////////////////////////////////

	    /******************************************************************************/
	    /**
	     * The standard constructor
	     */
		parameterValuePair()
			:value_(0.),
			 hasValue_(false)
		{ /* nothing */ }

		/******************************************************************************/
		/**
		 * A standard copy constructor
		 */
		parameterValuePair(const parameterValuePair& cp) {
			// Copy the double vector's content
			std::vector<boost::shared_ptr<GDoubleParameter> >::const_iterator dcit;
			for(dcit=cp.dArray_.begin(); dcit!=cp.dArray_.end(); ++dcit) {
				boost::shared_ptr<GDoubleParameter> p(new GDoubleParameter(*(dcit->get())));
				dArray_.push_back(p);
			}

			// Copy the long vector's content
			std::vector<boost::shared_ptr<GLongParameter> >::const_iterator lcit;
			for(lcit=cp.lArray_.begin(); lcit!=cp.lArray_.end(); ++lcit) {
				boost::shared_ptr<GLongParameter> p(new GLongParameter(*(lcit->get())));
				lArray_.push_back(p);
			}

			// Copy the bool vector's content
			std::vector<boost::shared_ptr<GBoolParameter> >::const_iterator bcit;
			for(bcit=cp.bArray_.begin(); bcit!=cp.bArray_.end(); ++bcit) {
				boost::shared_ptr<GBoolParameter> p(new GBoolParameter(*(bcit->get())));
				bArray_.push_back(p);
			}

			// Copy the char vector's content
			std::vector<boost::shared_ptr<GCharParameter> >::const_iterator ccit;
			for(ccit=cp.cArray_.begin(); ccit!=cp.cArray_.end(); ++ccit) {
				boost::shared_ptr<GCharParameter> p(new GCharParameter(*(ccit->get())));
				cArray_.push_back(p);
			}

			value_ = cp.value_;
			hasValue_ = cp.hasValue_;
		}

		/******************************************************************************/
		/**
		 * The destructor
		 */
		~parameterValuePair(){
			dArray_.clear();
			lArray_.clear();
			bArray_.clear();
			cArray_.clear();
		}

		/******************************************************************************/
		/**
		 * A standard assignment operator. As it needs to take care of differing
		 * vector sizes, it is far more complicated than the copy constructor. We
		 * thus use an external helper function to carry out the procedure.
		 */
		const parameterValuePair& operator=(const parameterValuePair& cp) {
			// Copy the double vector's content
			copySmartPointerVector<GDoubleParameter>(cp.dArray_, dArray_);

			// Copy the long vector's content
			copySmartPointerVector<GLongParameter>(cp.lArray_, lArray_);

			// Copy the bool vector's content
			copySmartPointerVector<GBoolParameter>(cp.bArray_, bArray_);

			// Copy the char vector's content
			copySmartPointerVector<GCharParameter>(cp.cArray_, cArray_);

			value_ = cp.value_;
			hasValue_ = cp.hasValue_;
		}

		/******************************************************************************/
		/**
		 * Resets the structure to its initial state
		 */
		void reset() {
			dArray_.clear();
			lArray_.clear();
			bArray_.clear();
			cArray_.clear();

			value_ = 0.;
			hasValue_ = false;
		}

		/**************************************************************************/
		/**
		 * Gives access to the object's value
		 *
		 * @return The value of the object
		 */
		double value() {
			return value_;
		}

		/**************************************************************************/
		/**
		 * Sets the precision of FP IO in ASCII mode
		 *
		 * @param The desired precision for FP IO
		 */
		void setPrecision(const std::streamsize& precision) {
			std::vector<boost::shared_ptr<GDoubleParameter> >::iterator it;
			for(it=dArray_.begin(); it!=dArray_.end(); ++it) (*it)->setPrecision(precision);
		}

		/**************************************************************************/
		/**
		 * Writes the class'es data to a stream
		 *
		 * @param stream The external output stream to write to
		 */
		void writeToStream(std::ostream& stream) const {
			std::size_t dArraySize = dArray_.size();
			stream << dArraySize;
			if(dArraySize) {
				std::vector<boost::shared_ptr<GDoubleParameter> >::const_iterator dit;
				for(dit=dArray_.begin(); dit!=dArray_.end(); ++dit) stream << **dit;
			}

			std::size_t lArraySize = lArray_.size();
			stream << lArraySize;
			if(lArraySize) {
				std::vector<boost::shared_ptr<GLongParameter> >::const_iterator lit;
				for(lit=lArray_.begin(); lit!=lArray_.end(); ++lit) stream << **lit;
			}

			std::size_t bArraySize = bArray_.size();
			stream << bArraySize;
			if(bArraySize) {
				std::vector<boost::shared_ptr<GBoolParameter> >::const_iterator bit;
				for(bit=bArray_.begin(); bit!=bArray_.end(); ++bit) stream << **bit;
			}

			std::size_t cArraySize = cArray_.size();
			stream << cArraySize;
			if(cArraySize) {
				std::vector<boost::shared_ptr<GCharParameter> >::const_iterator cit;
				for(cit=cArray_.begin(); cit!=cArray_.end(); ++cit) stream << **cit;
			}
		}

		/**************************************************************************/
		/**
		 * Reads the class'es data from a stream
		 *
		 * @param stream The external input stream to read from
		 */
		void readFromStream(std::istream& stream) {
			std::size_t dArraySize;
			stream >> dArraySize;
			dArray_.clear();
			if(dArraySize) {
				for(std::size_t i=0; i<dArraySize; i++){
					boost::shared_ptr<GDoubleParameter> p(new GDoubleParameter());
					stream >> *p;
					dArray_.push_back(p);
				}
			}

			std::size_t lArraySize;
			stream >> lArraySize;
			lArray_.clear();
			if(lArraySize) {
				for(std::size_t i=0; i<lArraySize; i++){
					boost::shared_ptr<GLongParameter> p(new GLongParameter());
					stream >> *p;
					lArray_.push_back(p);
				}
			}

			std::size_t bArraySize;
			stream >> bArraySize;
			bArray_.clear();
			if(bArraySize) {
				for(std::size_t i=0; i<bArraySize; i++){
					boost::shared_ptr<GBoolParameter> p(new GBoolParameter());
					stream >> *p;
					bArray_.push_back(p);
				}
			}

			std::size_t cArraySize;
			stream >> cArraySize;
			cArray_.clear();
			if(cArraySize) {
				for(std::size_t i=0; i<cArraySize; i++){
					boost::shared_ptr<GCharParameter> p(new GCharParameter());
					stream >> *p;
					cArray_.push_back(p);
				}
			}
		}

		/******************************************************************************/

		std::vector<boost::shared_ptr<GDoubleParameter> > dArray_; ///< vector holding double parameter sets
		std::vector<boost::shared_ptr<GLongParameter> > lArray_; ///< vector holding long parameter sets
		std::vector<boost::shared_ptr<GBoolParameter> > bArray_; ///< vector holding boolean parameter sets
		std::vector<boost::shared_ptr<GCharParameter> > cArray_; ///< vector holding character parameter sets

		double value_; ///< The value of this particular data set, if it has already been assigned
		bool hasValue_; ///< Indicates whether a value has been assigned to the data set
	};
	/**********************************************************************************/
	////////////////////////////////////////////////////////////////////////////////////
	/**********************************************************************************/

	/** @brief This vector holds the actual data */
	std::vector<boost::shared_ptr<parameterValuePair> > parameterValueSet_;
	/** @brief An iterator indicating the current position in the vector */
	std::vector<boost::shared_ptr<parameterValuePair> >::iterator current_;
};

/**************************************************************************/
// Specializations for the usual cases

/** @brief Gives access to a full data set containing doubles */
template <> boost::shared_ptr<GDoubleParameter> GDataExchange::parameterSet_at<double>(std::size_t);
/** @brief Gives access to a full data set containing boost::int32_ts */
template <> boost::shared_ptr<GLongParameter> GDataExchange::parameterSet_at<boost::int32_t>(std::size_t);
/** @brief Gives access to a full data set containing bools */
template <> boost::shared_ptr<GBoolParameter> GDataExchange::parameterSet_at<bool>(std::size_t);
/** @brief Gives access to a full data set containing chars */
template <> boost::shared_ptr<GCharParameter> GDataExchange::parameterSet_at<char>(std::size_t);

/** @brief Gives access to a parameter of type double */
template <> double GDataExchange::at<double>(std::size_t);
/** @brief Gives access to a parameter of type boost::int32_t */
template <> boost::int32_t GDataExchange::at<boost::int32_t>(std::size_t);
/** @brief Gives access to a parameter of type bool */
template <> bool GDataExchange::at<bool>(std::size_t);
/** @brief Gives access to a parameter of type char */
template <> char GDataExchange::at<char>(std::size_t);

/** @brief Gives access to the size of the dArray_ vector */
template <> std::size_t GDataExchange::size<double>();
/** @brief Gives access to the size of the lArray_ vector */
template <> std::size_t GDataExchange::size<boost::int32_t>();
/** @brief Gives access to the size of the bArray_ vector */
template <> std::size_t GDataExchange::size<bool>();
/** @brief Gives access to the size of the cArray_ vector */
template <> std::size_t GDataExchange::size<char>();

/** @brief Appends a boost::shared_ptr<GDoubleParameter> object without boundaries to the corresponding vector. */
template <> void GDataExchange::append<double>(const double&);
/** @brief Appends a boost::shared_ptr<GLongParameter> object without boundaries to the corresponding vector. */
template <> void GDataExchange::append<boost::int32_t>(const boost::int32_t&);
/** @brief Appends a boost::shared_ptr<GBoolParameter> object without boundaries to the corresponding vector. */
template <> void GDataExchange::append<bool>(const bool&);
/** @brief Appends a boost::shared_ptr<GCharParameter> object without boundaries to the corresponding vector. */
template <> void GDataExchange::append<char>(const char&);

/** @brief  Adds a boost::shared_ptr<GDoubleParameter> object with boundaries to the corresponding array */
template <> void GDataExchange::append<double>(const double&, const double&, const double&);
/** @brief  Adds a boost::shared_ptr<GLongParameter> object with boundaries to the corresponding array */
template <> void GDataExchange::append<boost::int32_t>(const boost::int32_t&, const boost::int32_t&, const boost::int32_t&);
/** @brief  Adds a boost::shared_ptr<GCharParameter> object with boundaries to the corresponding array */
template <> void GDataExchange::append<bool>(const bool&, const bool&, const bool&);
/** @brief  Adds a boost::shared_ptr<GCharParameter> object with boundaries to the corresponding array */
template <> void GDataExchange::append<char>(const char&, const char&, const char&);

/*************************************************************************/
// IO helper functions

/** @brief Helper function to aid IO  of this data set */
std::ostream& operator<<(std::ostream&, const GDataExchange&);
/** @brief Helper function to aid IO  of this data set */
std::istream& operator>>(std::istream&, GDataExchange&);

/*************************************************************************/

} /* namespace Util */
} /* namespace Gem */

#endif /* GDATAEXCHANGE_HPP_ */

