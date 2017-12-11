/**
 * @file GParserBuilder.hpp
 */

/*
 * Copyright (C) Gemfony scientific UG (haftungsbeschraenkt)
 *
 * See the AUTHORS file in the top-level directory for a list of authors.
 *
 * Contact: contact [at] gemfony (dot) eu
 *
 * This file is part of the Geneva library collection.
 *
 * Geneva was developed with kind support from Karlsruhe Institute of
 * Technology (KIT) and Steinbuch Centre for Computing (SCC). Further
 * information about KIT and SCC can be found at http://www.kit.edu/english
 * and http://scc.kit.edu .
 *
 * Geneva is free software: you can redistribute and/or modify it under
 * the terms of version 3 of the GNU Affero General Public License
 * as published by the Free Software Foundation.
 *
 * Geneva is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with the Geneva library. If not, see <http://www.gnu.org/licenses/>.
 *
 * For further information on Gemfony scientific and Geneva, visit
 * http://www.gemfony.eu .
 */

// Global checks, defines and includes needed for all of Geneva
#include "common/GGlobalDefines.hpp"

// Standard headers go here
#include <cstdlib>
#include <fstream>
#include <sstream>
#include <iostream>
#include <vector>
#include <string>
#include <functional>
#include <mutex>
#include <array>

// Boost headers go here
#include <boost/algorithm/string.hpp>
#include <boost/filesystem.hpp>
#include <boost/filesystem/fstream.hpp>
#include <boost/utility.hpp>
#include <boost/program_options.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/tokenizer.hpp>
#include <boost/archive/xml_oarchive.hpp>
#include <boost/archive/xml_iarchive.hpp>
#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>
#include <boost/archive/binary_oarchive.hpp>
#include <boost/archive/binary_iarchive.hpp>
#include <boost/serialization/nvp.hpp>
#include <boost/serialization/map.hpp>
#include <boost/serialization/variant.hpp>
#include <boost/serialization/vector.hpp>
#include <boost/serialization/shared_ptr.hpp>
#include <boost/serialization/base_object.hpp>
#include <boost/serialization/utility.hpp>
#include <boost/serialization/tracking.hpp>
#include <boost/serialization/split_member.hpp>
#include <boost/serialization/export.hpp>

/******************************************************************************/

#ifndef GPARSERBUILDER_HPP_
#define GPARSERBUILDER_HPP_

// Geneva header files go here
#include "common/GExceptions.hpp"
#include "common/GLogger.hpp"
#include "common/GErrorStreamer.hpp"
#include "common/GCommonEnums.hpp"
#include "common/GDefaultValueT.hpp"
#include "common/GCommonHelperFunctions.hpp"

namespace Gem {
namespace Common {

// Forward declaration
class GParserBuilder;

/******************************************************************************/
// Indicates whether help was requested using the -h or --help switch on the command line
const bool GCL_HELP_REQUESTED = true;
const bool GCL_NO_HELP_REQUESTED = false;

// Indicates whether implicit values are allowed (such as in --server vs. --server=true)
const bool GCL_IMPLICIT_ALLOWED = true;
const bool GCL_IMPLICIT_NOT_ALLOWED = false;

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/
/**
 * Gives write access to a reference to parm_ a single time. When this has happened,
 * only an explicit reset allows to gain access to a parameter-reference again.
 * It is however possible to explicitly set the parameter.
 */
template<typename T>
class GOneTimeRefParameterT {
	 ///////////////////////////////////////////////////////////////////////
	 friend class boost::serialization::access;

	 template<typename Archive>
	 void serialize(Archive &ar, const unsigned int) {
		 using boost::serialization::make_nvp;

		 ar
		 & BOOST_SERIALIZATION_NVP(parm_)
		 & BOOST_SERIALIZATION_NVP(parmDummy_)
		 & BOOST_SERIALIZATION_NVP(parmSet_);
	 }
	 ///////////////////////////////////////////////////////////////////////

public:
	 /***************************************************************************/
	 /**
	  * The standard constructor
	  */
	 explicit GOneTimeRefParameterT(const T &def = T(0))
		 : parm_(def)
		 , parmDummy_(def)
		 , parmSet_(false)
	 { /* nothing */ }

	 /***************************************************************************/
	 /**
	  * The copy constructor
	  */
	 GOneTimeRefParameterT(const GOneTimeRefParameterT<T> &cp)
		 : parm_(cp.parm_)
		 , parmDummy_(cp.parmDummy_)
		 , parmSet_(cp.parmSet_)
	 { /* nothing */ }

	 /***************************************************************************/
	 /**
	  * Assignment of another object of this type
	  */
	 GOneTimeRefParameterT<T> &operator=(const GOneTimeRefParameterT<T> &cp) {
		 parm_ = cp.parm_;
		 parmDummy_ = cp.parmDummy_;
		 parmSet_ = cp.parmSet_;

		 return *this;
	 }

	 /***************************************************************************/
	 /**
	  * Returns a reference to the parameter, if it hasn't been set. Otherwise
	  * it will return a reference to the dummy parameter.
	  */
	 T &reference() {
		 if (parmSet_) {
			 return parmDummy_;
		 } else {
			 parmSet_ = true;
			 return parm_;
		 }
	 }

	 /***************************************************************************/
	 /**
	  * Allows to check whether a parameter has already been set
	  */
	 bool parmSet() const {
		 return parmSet_;
	 }

	 /***************************************************************************/
	 /**
	  * Explicit reset of the "dirty" flag
	  */
	 void reset() {
		 parmSet_ = false;
	 }

	 /***************************************************************************/
	 /**
	  * Returns the parameter value
	  */
	 T value() const {
		 return parm_;
	 }

	 /***************************************************************************/
	 /**
	  * Allows to explicitly set the value of the parameter
	  */
	 void setValue(const T &parm) {
		 parm_ = parm;
		 parmSet_ = true;
	 }

	 /***************************************************************************/
	 /**
	  * Explicit assignment of a T value
	  */
	 void operator=(const T &parm) {
		 this->setValue(parm);
	 }

	 /***************************************************************************/
	 /**
	  * Automatic conversion
	  */
	 operator T() {
		 return parm_;
	 }

	 /***************************************************************************/
	 /**
	  * Automatic conversion for constant callers
	  */
	 operator T() const {
		 return parm_;
	 }

private:
	 /***************************************************************************/

	 T parm_; ///< Stores the actual setting
	 T parmDummy_; ///< Returned instead of parm_ if the latter has already been set
	 bool parmSet_; ///< Set to true if the parameter has been set already
};

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/
/**
 * A manipulator object that allows to identify the id of the comment to be
 * added
 */
class commentLevel {
public:
	 /** @brief The standard constructor */
	 explicit G_API_COMMON commentLevel(const std::size_t);

	 /** @brief Retrieves the current commentLevel */
	 G_API_COMMON std::size_t getCommentLevel() const;

private:
	 commentLevel() = delete; ///< The default constructor -- intentionally private and undefined
	 std::size_t commentLevel_; ///< The id of the comment inside of GParsableI
};

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/
/**
 * A manipulator object that increments the comment level
 */
class nextComment {
public:
	 /** @brief The default constructor */
	 G_API_COMMON nextComment() { /* nothing */ }
};

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/
/**
 * This class specifies the interface of parsable parameters, to
 * which a call-back function has been assigned. It also stores some
 * information common to all parameter types.
 */
class GParsableI {
public:
	 /** @brief A constructor for individual items */
	 G_API_COMMON GParsableI(
		 const std::string &
		 , const std::string &
	 );

	 /** @brief A constructor for vectors */
	 G_API_COMMON GParsableI(
		 const std::vector<std::string> &
		 , const std::vector<std::string> &
	 );

	 /** @brief The destructor */
	 virtual G_API_COMMON ~GParsableI();

	 /** @brief Retrieves the option name at a given position */
	 G_API_COMMON std::string optionName(std::size_t = 0) const;
	 /** @brief Retrieves the comment that was assigned to this variable at a given position */
	 G_API_COMMON std::string comment(std::size_t = 0) const;
	 /** @brief Checks whether comments have indeed been registered */
	 G_API_COMMON bool hasComments() const;
	 /** @brief Retrieves the number of comments available */
	 G_API_COMMON std::size_t numberOfComments() const;

	 /***************************************************************************/
	 /**
	  * Create a std::vector<T> from a single element
	  */
	 template<typename T>
	 static std::vector<T> makeVector(const T &item) {
		 std::vector<T> result;
		 result.push_back(item);
		 return result;
	 }

	 /***************************************************************************/
	 /**
	  * Create a std::vector<T> from two elements
	  */
	 template<typename T>
	 static std::vector<T> makeVector(const T &item1, const T &item2) {
		 std::vector<T> result;
		 result.push_back(item1);
		 result.push_back(item2);
		 return result;
	 }

	 /***************************************************************************/
	 /**
	  * This function will forward all arguments to a newly created ostringstream
	  * and will then be added to the current comment_ entry.
	  */
	 template<typename T>
	 GParsableI &operator<<(const T &t) {
		 std::ostringstream oss;
		 oss << t;
		 m_comment.at(m_cl) += oss.str();
		 return *this;
	 }

	 /***************************************************************************/
	 /** @brief Needed for std::ostringstream */
	 G_API_COMMON GParsableI &operator<<(std::ostream &( *val )(std::ostream &));
	 /** @brief Needed for std::ostringstream */
	 G_API_COMMON GParsableI &operator<<(std::ios &( *val )(std::ios &));
	 /** @brief Needed for std::ostringstream */
	 G_API_COMMON GParsableI &operator<<(std::ios_base &( *val )(std::ios_base &));
	 /** @brief Allows to indicate the current comment level */
	 G_API_COMMON GParsableI &operator<<(const commentLevel &);
	 /** @brief Allows to switch to the next comment level */
	 G_API_COMMON GParsableI &operator<<(const nextComment &);

protected:
	 /***************************************************************************/
	 /** @brief Splits a comment into sub-tokens */
	 G_API_COMMON std::vector<std::string> splitComment(const std::string &) const;

private:
	 /***************************************************************************/
	 // Prevent copying, moving and default construction
	 GParsableI() = delete;
	 GParsableI(const GParsableI&) = delete;
	 GParsableI(const GParsableI&&) = delete;
	  GParsableI& operator=(const GParsableI&) = delete;
	  GParsableI& operator=(const GParsableI&&) = delete;

	 std::vector<std::string> m_option_name; ///< The name of this parameter
	 std::vector<std::string> m_comment; ///< A comment assigned to this parameter

	 std::size_t m_cl; ///< The id of the current comment inside of the comment_ vector
};

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/
/**
 * This class specifies the interface of parsable file parameters, to
 * which a call-back function has been assigned. Note that this class cannot
 * be copied, as the parent class is derived from boost::noncopyable.
 */
class GFileParsableI : public GParsableI
{
	 // We want GParserBuilder to be able to call our private load- and save functions
	 friend class GParserBuilder;

public:
	 /** @brief A constructor for individual items */
	 G_API_COMMON GFileParsableI(
		 const std::string &
		 , const std::string &
		 , const bool &
	 );
	 /** @brief A constructor for vectors */
	 G_API_COMMON GFileParsableI(
		 const std::vector<std::string> &
		 , const std::vector<std::string> &
		 , const bool &
	 );

	 /** @brief The destructor */
	 virtual G_API_COMMON ~GFileParsableI();

	 /** @brief Executes a stored call-back function */
	 virtual G_API_COMMON void executeCallBackFunction() = 0;

	 /** @brief Checks whether this is an essential variable at a given position */
	 G_API_COMMON bool isEssential() const;

protected:
	 /***************************************************************************/
	 /** @brief Loads data from a property_tree object */
	 virtual void load(const boost::property_tree::ptree &) = 0;

	 /** @brief Saves data to a property tree object */
	 virtual void save(boost::property_tree::ptree &) const = 0;

private:
	 /***************************************************************************/
	 // Prevent copying, moving and default construction
	 GFileParsableI() = delete;
	 GFileParsableI(const GFileParsableI&) = delete;
	 GFileParsableI(const GFileParsableI&&) = delete;
	  GFileParsableI& operator=(const GFileParsableI&) = delete;
	  GFileParsableI& operator=(const GFileParsableI&&) = delete;

	 bool m_is_essential; ///< Indicates whether this is an essential variable
};

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/
/**
 * This class specifies the interface of parsable command line parameters. Note
 * that this class cannot be copied, as the parent class is derived from
 * boost::noncopyable.
 */
class GCLParsableI
	: public GParsableI {
	 // We want GParserBuilder to be able to call our private load- and save functions
	 friend class GParserBuilder;

public:
	 /** @brief A constructor for individual items */
	 G_API_COMMON GCLParsableI(
		 const std::string &
		 , const std::string &
	 );
	 /** @brief A constructor for vectors */
	 G_API_COMMON GCLParsableI(
		 const std::vector<std::string> &
		 , const std::vector<std::string> &
	 );

	 /** @brief The destructor */
	 virtual G_API_COMMON ~GCLParsableI();

protected:
	 /** @brief Saves data to a property tree object */
	 virtual void save(boost::program_options::options_description &) const = 0;

	 /** @brief Returns the content of this object as a std::string */
	 virtual std::string content() const = 0;

private:
	 /***************************************************************************/
	 // Prevent copying, moving and default construction
	 GCLParsableI() = delete;
	 GCLParsableI(const GCLParsableI&) = delete;
	 GCLParsableI(const GCLParsableI&&) = delete;
	  GCLParsableI& operator=(const GCLParsableI&) = delete;
	  GCLParsableI& operator=(const GCLParsableI&&) = delete;
};

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/
/**
 * A base class for single parameters. This class was introduced so we can
 * reset the default values in a central location rather than having to
 * convert to different target class. This makes user-code easier.
 */
template<typename parameter_type>
struct GSingleParmT
	: public GFileParsableI {
	 // We want GParserBuilder to be able to call the reset function
	 friend class GParserBuilder;

public:
	 /***************************************************************************/
	 /**
	  * Initializes the parameter and sets values in the parent class
	  */
	 GSingleParmT(
		 const std::string &optionNameVar, const std::string &commentVar, const bool &isEssentialVar,
		 const parameter_type &def_val
	 )
		 : GFileParsableI(
		 optionNameVar, commentVar, isEssentialVar
	 ), m_def_val(def_val), m_par(def_val) { /* nothing */ }

	 /***************************************************************************/
	 /**
	  * The destructor
	  */
	 virtual ~GSingleParmT() { /* nothing */ }

protected:
	 /***************************************************************************/
	 /**
	  * Allows derived classes to reset the default value. The function assumes
	  * that no important data is stored in par_, as its value will be overwritten
	  * as well. The reason is that configuration files will otherwise contain
	  * the "old" par_-value.
	  */
	 void resetDefault(const parameter_type &def_val) {
		 m_def_val = def_val;
		 m_par = def_val;
	 }

	 /***************************************************************************/
	 parameter_type m_def_val; ///< Holds the parameter's default value
	 parameter_type m_par; ///< Holds the individual parameter

private:
	 /***************************************************************************/
	 // Prevent copying, moving and default construction
	 GSingleParmT() = delete;
	 GSingleParmT(const GSingleParmT<parameter_type>&) = delete;
	 GSingleParmT(const GSingleParmT<parameter_type>&&) = delete;
	  GSingleParmT<parameter_type>& operator=(const GSingleParmT<parameter_type>&) = delete;
	  GSingleParmT<parameter_type>& operator=(const GSingleParmT<parameter_type>&&) = delete;
};

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/
/**
 * This class wraps individual parsable file parameters, to which a callback
 * function has been assigned.
 */
template<typename parameter_type>
struct GFileSingleParsableParameterT
	: public GSingleParmT<parameter_type> {
	 // We want GParserBuilder to be able to call our load- and save functions
	 friend class GParserBuilder;

public:
	 /***************************************************************************/
	 /**
	  * Initializes the parameter and sets values in the parent class
	  */
	 GFileSingleParsableParameterT(
		 const std::string &optionNameVar
		 , const std::string &commentVar
		 , const bool &isEssentialVar
		 , const parameter_type &def_val
	 )
		 : GSingleParmT<parameter_type>(
		 optionNameVar, commentVar, isEssentialVar, def_val
	 ) { /* nothing */ }

	 /***************************************************************************/
	 /**
	  * Initializes the parameter and sets values in the parent class, except
	  * for comments.
	  */
	 GFileSingleParsableParameterT(
		 const std::string &optionNameVar
		 , const parameter_type &def_val
	 )
		 : GSingleParmT<parameter_type>(optionNameVar, std::string(""), Gem::Common::VAR_IS_ESSENTIAL, def_val)
	 { /* nothing */ }

	 /***************************************************************************/
	 /**
	  * The destructor
	  */
	 virtual ~GFileSingleParsableParameterT() { /* nothing */ }

	 /***************************************************************************/
	 /**
	  * Executes a stored call-back function
	  */
	 virtual void executeCallBackFunction() {
		 if (!m_call_back_func) {
			 throw gemfony_exception(
				 g_error_streamer(DO_LOG, time_and_place)
					 << "In GSingleParsableParameter::executeCallBackFunction(): Error" << std::endl
					 << "Tried to execute call-back function without a stored function" << std::endl
			 );
		 }

		 // Execute the function
		 m_call_back_func(GSingleParmT<parameter_type>::m_par);
	 }

	 /***************************************************************************/
	 /**
	  * Allows to register a call-back function with this object
	  *
	  * @param callBack The function to be executed
	  */
	 void registerCallBackFunction(std::function<void(parameter_type)> callBack) {
		 if (!callBack) {
			 throw gemfony_exception(
				 g_error_streamer(DO_LOG, time_and_place)
					 << "In GSingleParsableParameter::registerCallBackFunction(): Error" << std::endl
					 << "Tried to register an empty call-back function" << std::endl
			 );
		 }

		 m_call_back_func = callBack;
	 }

protected:
	 /***************************************************************************/
	 /**
	  * Loads data from a property_tree object
	  *
	  * @param pt The object from which data should be loaded
	  */
	 virtual void load(const boost::property_tree::ptree &pt) {
		 GSingleParmT<parameter_type>::m_par = pt.get((GParsableI::optionName(0) + ".value").c_str(),
			 GSingleParmT<parameter_type>::m_def_val);
	 }

	 /***************************************************************************/
	 /**
	  * Saves data to a property tree object, including comments.
	  *
	  * @param pt The object to which data should be saved
	  */
	 virtual void save(boost::property_tree::ptree &pt) const {
		 // Check that we have the right number of comments
		 if (this->hasComments()) {
			 if (this->numberOfComments() != 1) {
				 throw gemfony_exception(
					 g_error_streamer(DO_LOG, time_and_place)
						 << "In GFileSingleParsableParameterT<>::save(): Error!" << std::endl
						 << "Expected 0 or 1 comment but got " << this->numberOfComments() << std::endl
				 );
			 }

			 // Retrieve a list of sub-comments
			 std::vector<std::string> comments = GParsableI::splitComment(this->comment(0));
			 std::vector<std::string>::iterator c;
			 if (!comments.empty()) {
				 for (c = comments.begin(); c != comments.end(); ++c) {
					 pt.add((GParsableI::optionName(0) + ".comment").c_str(), (*c).c_str());
				 }
			 }
		 }

		 pt.put((GParsableI::optionName(0) + ".default").c_str(), GSingleParmT<parameter_type>::m_def_val);
		 pt.put((GParsableI::optionName(0) + ".value").c_str(), GSingleParmT<parameter_type>::m_par);
	 }

private:
	 /***************************************************************************/
	 // Prevent copying, moving and default construction
	 GFileSingleParsableParameterT() = delete;
	 GFileSingleParsableParameterT(const GFileSingleParsableParameterT<parameter_type>&) = delete;
	 GFileSingleParsableParameterT(const GFileSingleParsableParameterT<parameter_type>&&) = delete;
	  GFileSingleParsableParameterT<parameter_type>& operator=(const GFileSingleParsableParameterT<parameter_type>&) = delete;
	  GFileSingleParsableParameterT<parameter_type>& operator=(const GFileSingleParsableParameterT<parameter_type>&&) = delete;

	 std::function<void(parameter_type)> m_call_back_func; ///< Holds the call-back function
};

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/
/**
 * This class wraps a reference to individual parameters. Instead of
 * executing a stored call-back function, executeCallBackFunction will assign
 * the parsed value to the reference.
 */
template<typename parameter_type>
class GFileReferenceParsableParameterT
	: public GSingleParmT<parameter_type> {
	 // We want GParserBuilder to be able to call our load- and save functions
	 friend class GParserBuilder;

public:
	 /***************************************************************************/
	 /**
	  * Initializes the parameter and sets values in the parent class
	  */
	 GFileReferenceParsableParameterT(
		 parameter_type &storedReference, const std::string &optionNameVar, const std::string &commentVar,
		 const bool &isEssentialVar, const parameter_type &def_val
	 )
		 : GSingleParmT<parameter_type>(
		 optionNameVar, commentVar, isEssentialVar, def_val
	 ), m_stored_reference(storedReference) { /* nothing */ }

	 /***************************************************************************/
	 /**
	  * Initializes the parameter and sets values in the parent class, except
	  * for comments.
	  */
	 GFileReferenceParsableParameterT(
		 parameter_type &storedReference, const std::string &optionNameVar, const parameter_type &def_val
	 )
		 : GSingleParmT<parameter_type>(
		 optionNameVar, std::string(""), Gem::Common::VAR_IS_ESSENTIAL, def_val
	 ), m_stored_reference(storedReference) { /* nothing */ }

	 /***************************************************************************/
	 /**
	  * The destructor
	  */
	 virtual ~GFileReferenceParsableParameterT() { /* nothing */ }

	 /***************************************************************************/
	 /**
	  * Assigns the stored parameter to the reference
	  */
	 virtual void executeCallBackFunction() {
		 m_stored_reference = GSingleParmT<parameter_type>::m_par;
	 }

protected:
	 /***************************************************************************/
	 /**
	  * Loads data from a property_tree object
	  *
	  * @param pt The object from which data should be loaded
	  */
	 virtual void load(const boost::property_tree::ptree &pt) {
		 GSingleParmT<parameter_type>::m_par = pt.get((GParsableI::optionName(0) + ".value").c_str(),
			 GSingleParmT<parameter_type>::m_def_val);
	 }

	 /***************************************************************************/
	 /**
	  * Saves data to a property tree object, including comments.
	  *
	  * @param pt The object to which data should be saved
	  */
	 virtual void save(boost::property_tree::ptree &pt) const {
		 // Check that we have the right number of comments
		 if (this->hasComments()) {
			 if (this->numberOfComments() != 1) {
				 throw gemfony_exception(
					 g_error_streamer(DO_LOG, time_and_place)
						 << "In GFileReferenceParsableParameterT<>::save(): Error!" << std::endl
						 << "Expected 0 or 1 comment but got " << this->numberOfComments() << std::endl
				 );
			 }

			 // Retrieve a list of sub-comments
			 std::vector<std::string> comments = GParsableI::splitComment(this->comment(0));
			 std::vector<std::string>::iterator c;
			 if (!comments.empty()) {
				 for (c = comments.begin(); c != comments.end(); ++c) {
					 pt.add((GParsableI::optionName(0) + ".comment").c_str(), (*c).c_str());
				 }
			 }
		 }

		 pt.put((GParsableI::optionName(0) + ".default").c_str(), GSingleParmT<parameter_type>::m_def_val);
		 pt.put((GParsableI::optionName(0) + ".value").c_str(), GSingleParmT<parameter_type>::m_par);
	 }

private:
	 /***************************************************************************/
	 // Prevent copying, moving and default construction
	 GFileReferenceParsableParameterT() = delete;
	 GFileReferenceParsableParameterT(const GFileReferenceParsableParameterT<parameter_type>&) = delete;
	 GFileReferenceParsableParameterT(const GFileReferenceParsableParameterT<parameter_type>&&) = delete;
	  GFileReferenceParsableParameterT<parameter_type>& operator=(const GFileReferenceParsableParameterT<parameter_type>&) = delete;
	  GFileReferenceParsableParameterT<parameter_type>& operator=(const GFileReferenceParsableParameterT<parameter_type>&&) = delete;

	 parameter_type &m_stored_reference; ///< Holds the reference to which the parsed value will be assigned
};

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/
/**
 * A base class for combined parameters. This class was introduced so we can
 * reset the default values in a central location rather than having to
 * convert to different target class. This makes user-code easier.
 */
template<typename par_type0, typename par_type1>
struct GCombinedParT
	: public GFileParsableI {
	 // We want GParserBuilder to be able to call the reset function
	 friend class GParserBuilder;

public:


	 /***************************************************************************/
	 /**
	  * Initializes the parameter and sets values in the parent class
	  */
	 GCombinedParT(
		 const std::string &optionNameVar0, const std::string &commentVar0, const par_type0 &def_val0,
		 const std::string &optionNameVar1, const std::string &commentVar1, const par_type1 &def_val1,
		 const bool &isEssentialVar, const std::string &combined_label
	 )
		 : GFileParsableI(
		 GFileParsableI::makeVector(optionNameVar0, optionNameVar1), GFileParsableI::makeVector(commentVar0, commentVar1),
		 isEssentialVar
	 ), m_par0(def_val0), m_def_val0(def_val0), m_par1(def_val1), m_def_val1(def_val1),
		 combined_label_(combined_label) { /* nothing */ }

	 /***************************************************************************/
	 /**
	  * The destructor
	  */
	 virtual ~GCombinedParT() { /* nothing */ }

protected:
	 /***************************************************************************/
	 /**
	  * Allows derived classes to reset the default value. The function assumes
	  * that no important data is stored in par1_ and par_2, as their values will
	  * be overwritten as well. The reason is that configuration files will otherwise
	  * contain the "old" par_-value.
	  */
	 void resetDefault(const par_type0 &def_val0, const par_type1 &def_val1) {
		 m_def_val0 = def_val0;
		 m_def_val1 = def_val1;
		 m_par0 = def_val0;
		 m_par1 = def_val1;
	 }

	 /***************************************************************************/
	 par_type0 m_par0, m_def_val0; ///< Holds the individual parameters and default values 0
	 par_type1 m_par1, m_def_val1; ///< Holds the individual parameters and default values 1

	 std::string combined_label_; ///< Holds a path label for the combined JSON path

private:
	 /***************************************************************************/
	 // Prevent copying, moving and default construction
	 GCombinedParT() = delete;
	 GCombinedParT(const GCombinedParT<par_type0, par_type1>&) = delete;
	 GCombinedParT(const GCombinedParT<par_type0, par_type1>&&) = delete;
	  GCombinedParT<par_type0, par_type1>& operator=(const GCombinedParT<par_type0, par_type1>&) = delete;
	  GCombinedParT<par_type0, par_type1>& operator=(const GCombinedParT<par_type0, par_type1>&&) = delete;
};

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/
/**
 * This class wraps combined parsable file parameters, to which a callback
 * function has been assigned.
 */
template<typename par_type0, typename par_type1>
struct GFileCombinedParsableParameterT
	: public GCombinedParT<par_type0, par_type1> {
	 // We want GParserBuilder to be able to call our load- and save functions
	 friend class GParserBuilder;

public:
	 /***************************************************************************/
	 /**
	  * Initializes the parameters
	  */
	 GFileCombinedParsableParameterT(
		 const std::string &optionNameVar0, const std::string &commentVar0, const par_type0 &defVal0,
		 const std::string &optionNameVar1, const std::string &commentVar1, const par_type1 &defVal1,
		 const bool &isEssentialVar, const std::string &combined_label
	 )
		 : GCombinedParT<par_type0, par_type1>(
		 optionNameVar0, commentVar0, defVal0, optionNameVar1, commentVar1, defVal1, isEssentialVar, combined_label
	 ) { /* nothing */ }

	 /***************************************************************************/
	 /**
	  * Initializes the parameters
	  */
	 GFileCombinedParsableParameterT(
		 const std::string &optionNameVar0, const par_type0 &defVal0, const std::string &optionNameVar1,
		 const par_type1 &defVal1, const std::string &combined_label
	 )
		 : GCombinedParT<par_type0, par_type1>(
		 optionNameVar0, std::string(""), defVal0, optionNameVar1, std::string(""), defVal1, Gem::Common::VAR_IS_ESSENTIAL,
		 combined_label
	 ) { /* nothing */ }

	 /***************************************************************************/
	 /**
	  * The destructor
	  */
	 virtual ~GFileCombinedParsableParameterT() { /* nothing */ }

	 /***************************************************************************/
	 /**
	  * Executes a stored call-back function
	  */
	 virtual void executeCallBackFunction() {
		 if (!m_call_back_func) {
			 throw gemfony_exception(
				 g_error_streamer(DO_LOG, time_and_place)
					 << "In GFileCombinedParsableParameterT::executeCallBackFunction(): Error" << std::endl
					 << "Tried to execute call-back function without a stored function" << std::endl
			 );
		 }

		 // Execute the function
		 m_call_back_func(GCombinedParT<par_type0, par_type1>::m_par0, GCombinedParT<par_type0, par_type1>::m_par1);
	 }

	 /***************************************************************************/
	 /**
	  * Allows to register a call-back function with this object
	  *
	  * @param callBack The function to be executed
	  */
	 void registerCallBackFunction(std::function<void(par_type0, par_type1)> callBack) {
		 if (!callBack) {
			 throw gemfony_exception(
				 g_error_streamer(DO_LOG, time_and_place)
					 << "In GFileCombinedParsableParameterT::registerCallBackFunction(): Error" << std::endl
					 << "Tried to register an empty call-back function" << std::endl
			 );
		 }

		 m_call_back_func = callBack;
	 }

protected:
	 /***************************************************************************/
	 /**
	  * Loads data from a property_tree object
	  *
	  * @param pt The object from which data should be loaded
	  */
	 virtual void load(const boost::property_tree::ptree &pt) {
		 GCombinedParT<par_type0, par_type1>::m_par0 = pt.get(
			 (GCombinedParT<par_type0, par_type1>::combined_label_ + "." + GParsableI::optionName(0) + ".value").c_str(),
			 GCombinedParT<par_type0, par_type1>::m_def_val0);
		 GCombinedParT<par_type0, par_type1>::m_par1 = pt.get(
			 (GCombinedParT<par_type0, par_type1>::combined_label_ + "." + GParsableI::optionName(1) + ".value").c_str(),
			 GCombinedParT<par_type0, par_type1>::m_def_val1);
	 }

	 /***************************************************************************/
	 /**
	  * Saves data to a property tree object, including comments.
	  *
	  * @param pt The object to which data should be saved
	  */
	 virtual void save(boost::property_tree::ptree &pt) const {
		 // Check that we have the right number of comments
		 if (this->hasComments()) {
			 if (this->numberOfComments() != 2) {
				 throw gemfony_exception(
					 g_error_streamer(DO_LOG, time_and_place)
						 << "In GFileCombinedParsableParameterT<>::save(): Error!" << std::endl
						 << "Expected 0 or 2 comments but got " << this->numberOfComments() << std::endl
				 );
			 }

			 // Retrieve a list of sub-comments
			 std::vector<std::string> comments0 = GParsableI::splitComment(this->comment(0));
			 std::vector<std::string>::iterator c;
			 if (!comments0.empty()) {
				 for (c = comments0.begin(); c != comments0.end(); ++c) {
					 pt.add((GCombinedParT<par_type0, par_type1>::combined_label_ + "." + GParsableI::optionName(0) +
								".comment").c_str(), (*c).c_str());
				 }
			 }
		 }
		 pt.put(
			 (GCombinedParT<par_type0, par_type1>::combined_label_ + "." + GParsableI::optionName(0) + ".default").c_str(),
			 GCombinedParT<par_type0, par_type1>::m_def_val0);
		 pt.put(
			 (GCombinedParT<par_type0, par_type1>::combined_label_ + "." + GParsableI::optionName(0) + ".value").c_str(),
			 GCombinedParT<par_type0, par_type1>::m_par0);

		 if (this->hasComments()) {
			 std::vector<std::string> comments1 = GParsableI::splitComment(this->comment(1));
			 std::vector<std::string>::iterator c;
			 if (!comments1.empty()) {
				 for (c = comments1.begin(); c != comments1.end(); ++c) {
					 pt.add((GCombinedParT<par_type0, par_type1>::combined_label_ + "." + GParsableI::optionName(1) +
								".comment").c_str(), (*c).c_str());
				 }
			 }
		 }
		 pt.put(
			 (GCombinedParT<par_type0, par_type1>::combined_label_ + "." + GParsableI::optionName(1) + ".default").c_str(),
			 GCombinedParT<par_type0, par_type1>::m_def_val1);
		 pt.put(
			 (GCombinedParT<par_type0, par_type1>::combined_label_ + "." + GParsableI::optionName(1) + ".value").c_str(),
			 GCombinedParT<par_type0, par_type1>::m_par1);
	 }

private:
	 /***************************************************************************/
	 // Prevent copying, moving and default construction
	 GFileCombinedParsableParameterT() = delete;
	 GFileCombinedParsableParameterT(const GFileCombinedParsableParameterT<par_type0, par_type1>&) = delete;
	 GFileCombinedParsableParameterT(const GFileCombinedParsableParameterT<par_type0, par_type1>&&) = delete;
	  GFileCombinedParsableParameterT<par_type0, par_type1>& operator=(const GFileCombinedParsableParameterT<par_type0, par_type1>&) = delete;
	  GFileCombinedParsableParameterT<par_type0, par_type1>& operator=(const GFileCombinedParsableParameterT<par_type0, par_type1>&&) = delete;

	 std::function<void(par_type0, par_type1)> m_call_back_func; ///< Holds the call-back function
};

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/
/**
 * A base class for vector parameters. This class was introduced so we can
 * reset the default values in a central location rather than having to
 * convert to different target class. This makes user-code easier.
 */
template<typename parameter_type>
struct GVectorParT
	: public GFileParsableI {
	 // We want GParserBuilder to be able to call the reset function
	 friend class GParserBuilder;

public:
	 /***************************************************************************/
	 /**
	  * Initializes the parameter and sets values in the parent class
	  */
	 GVectorParT(
		 const std::string &optionNameVar, const std::string &commentVar, const std::vector<parameter_type> &def_val,
		 const bool &isEssentialVar
	 )
		 : GFileParsableI(
		 optionNameVar, commentVar, isEssentialVar
	 ), m_def_val(def_val), m_par() { /* nothing */ }

	 /***************************************************************************/
	 /**
	  * The destructor
	  */
	 virtual ~GVectorParT() { /* nothing */ }

protected:
	 /***************************************************************************/
	 /**
	  * Allows derived classes to reset the default value.
	  */
	 void resetDefault(const std::vector<parameter_type> &def_val) {
		 m_def_val = def_val;
	 }

	 /***************************************************************************/
	 std::vector<parameter_type> m_def_val; ///< Holds default values
	 std::vector<parameter_type> m_par; ///< Holds the parsed parameters

private:
	 /***************************************************************************/
	 // Prevent copying, moving and default construction
	 GVectorParT() = delete;
	 GVectorParT(const GVectorParT<parameter_type>&) = delete;
	 GVectorParT(const GVectorParT<parameter_type>&&) = delete;
	  GVectorParT<parameter_type>& operator=(const GVectorParT<parameter_type>&) = delete;
	  GVectorParT<parameter_type>& operator=(const GVectorParT<parameter_type>&&) = delete;
};

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/
/**
 * This class wraps a std::vector of values (obviously of identical type).
 * Note that this class does not enforce a given amount of parameters. However,
 * there needs to be at least one default value in the def_val vector, if
 * you plan to write out a parameter file.
 */
template<typename parameter_type>
class GFileVectorParsableParameterT
	: public GVectorParT<parameter_type> {
	 // We want GParserBuilder to be able to call our load- and save functions
	 friend class GParserBuilder;

public:
	 /***************************************************************************/
	 /**
	  * Initializes the parameters
	  */
	 GFileVectorParsableParameterT(
		 const std::string &optionNameVar, const std::string &commentVar, const std::vector<parameter_type> &def_val,
		 const bool &isEssentialVar
	 )
		 : GVectorParT<parameter_type>(
		 optionNameVar, commentVar, def_val, isEssentialVar
	 ) { /* nothing */ }

	 /***************************************************************************/
	 /**
	  * Initializes the parameters, except for comments
	  */
	 GFileVectorParsableParameterT(
		 const std::string &optionNameVar, const std::vector<parameter_type> &def_val
	 )
		 : GVectorParT<parameter_type>(
		 optionNameVar, std::string(""), def_val, Gem::Common::VAR_IS_ESSENTIAL
	 ) { /* nothing */ }

	 /***************************************************************************/
	 /**
	  * The destructor
	  */
	 virtual ~GFileVectorParsableParameterT() { /* nothing */ }

	 /***************************************************************************/
	 /**
	  * Executes a stored call-back function
	  */
	 virtual void executeCallBackFunction() {
		 if (!m_call_back_func) {
			 throw gemfony_exception(
				 g_error_streamer(DO_LOG, time_and_place)
					 << "In GFileVectorParsableParameterT::executeCallBackFunction(): Error" << std::endl
					 << "Tried to execute call-back function without a stored function" << std::endl
			 );
		 }

		 // Execute the function
		 m_call_back_func(GVectorParT<parameter_type>::m_par);
	 }

	 /***************************************************************************/
	 /**
	  * Allows to register a call-back function with this object
	  *
	  * @param callBack The function to be executed
	  */
	 void registerCallBackFunction(std::function<void(std::vector<parameter_type>)> callBack) {
		 if (!callBack) {
			 throw gemfony_exception(
				 g_error_streamer(DO_LOG, time_and_place)
					 << "In GFileVectorParsableParameterT::registerCallBackFunction(): Error" << std::endl
					 << "Tried to register an empty call-back function" << std::endl
			 );
		 }

		 m_call_back_func = callBack;
	 }

protected:
	 /***************************************************************************/
	 /**
	  * Loads data from a property_tree object
	  *
	  * @param pt The object from which data should be loaded
	  */
	 virtual void load(const boost::property_tree::ptree &pt) {
		 using namespace boost::property_tree;

		 // Make sure the recipient vector is empty
		 GVectorParT<parameter_type>::m_par.clear();

		 std::string ppath = GParsableI::optionName(0) + ".value";
		 for(const auto& v: pt.get_child(ppath.c_str())) {
			 GVectorParT<parameter_type>::m_par.push_back(boost::lexical_cast<parameter_type>(v.second.data()));
		 }
	 }

	 /***************************************************************************/
	 /**
	  * Saves data to a property tree object, including comments. Default
	  * values are taken from the def_val_ vector. Note that there needs
	  * to be at least a single default value in it.
	  *
	  * @param pt The object to which data should be saved
	  */
	 virtual void save(boost::property_tree::ptree &pt) const {
		 // Check that we have the right number of comments
		 if (this->hasComments()) {
			 if (this->numberOfComments() != 1) {
				 throw gemfony_exception(
					 g_error_streamer(DO_LOG, time_and_place)
						 << "In GFileVectorParsableParameterT<>::save(): Error!" << std::endl
						 << "Expected 0 or 1 comment but got " << this->numberOfComments() << std::endl
				 );
			 }

			 // Retrieve a list of sub-comments
			 std::vector<std::string> comments = GParsableI::splitComment(this->comment(0));
			 std::vector<std::string>::iterator c;
			 if (!comments.empty()) {
				 for (c = comments.begin(); c != comments.end(); ++c) {
					 pt.add((GParsableI::optionName(0) + ".comment").c_str(), (*c).c_str());
				 }
			 }
		 }

		 // Do some error checking
		 if (GVectorParT<parameter_type>::m_def_val.empty()) {
			 throw gemfony_exception(
				 g_error_streamer(DO_LOG, time_and_place)
					 << "In GVectorParsableParameter::save(): Error!" << std::endl
					 << "You need to provide at least one default value" << std::endl
			 );
		 }

		 // Add the value and default items
		 typename std::vector<parameter_type>::const_iterator it;
		 for (it = GVectorParT<parameter_type>::m_def_val.begin();
				it != GVectorParT<parameter_type>::m_def_val.end(); ++it) {
			 pt.add((GParsableI::optionName(0) + ".default.item").c_str(), *it);
			 pt.add((GParsableI::optionName(0) + ".value.item").c_str(), *it);
		 }
	 }

private:
	 /***************************************************************************/
	 // Prevent copying, moving and default construction
	 GFileVectorParsableParameterT() = delete;
	 GFileVectorParsableParameterT(const GFileVectorParsableParameterT<parameter_type>&) = delete;
	 GFileVectorParsableParameterT(const GFileVectorParsableParameterT<parameter_type>&&) = delete;
	  GFileVectorParsableParameterT<parameter_type>& operator=(const GFileVectorParsableParameterT<parameter_type>&) = delete;
	  GFileVectorParsableParameterT<parameter_type>& operator=(const GFileVectorParsableParameterT<parameter_type>&&) = delete;

	 std::function<void(std::vector<parameter_type>)> m_call_back_func; ///< Holds the call-back function
};

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/
/**
 * This class wraps a reference std::vector of values (obviously of identical type).
 * Note that this class does not enforce a given amount of parameters. However,
 * there needs to be at least one default value in the def_val vector, if
 * you plan to write out a parameter file.
 */
template<typename parameter_type>
class GFileVectorReferenceParsableParameterT
	: public GVectorParT<parameter_type> {
	 // We want GParserBuilder to be able to call our load- and save functions
	 friend class GParserBuilder;

public:
	 /***************************************************************************/
	 /**
	  * Initializes the parameters
	  */
	 GFileVectorReferenceParsableParameterT(
		 std::vector<parameter_type> &stored_reference, const std::string &optionNameVar, const std::string &commentVar,
		 const std::vector<parameter_type> &def_val, const bool &isEssentialVar
	 )
		 : GVectorParT<parameter_type>(
		 optionNameVar, commentVar, def_val, isEssentialVar
	 ), m_stored_reference(stored_reference) { /* nothing */ }

	 /***************************************************************************/
	 /**
	  * Initializes the parameters, except for comments
	  */
	 GFileVectorReferenceParsableParameterT(
		 std::vector<parameter_type> &stored_reference, const std::string &optionNameVar,
		 const std::vector<parameter_type> &def_val
	 )
		 : GVectorParT<parameter_type>(
		 optionNameVar, std::string(""), def_val, Gem::Common::VAR_IS_ESSENTIAL
	 ), m_stored_reference(stored_reference) { /* nothing */ }


	 /***************************************************************************/
	 /**
	  * The destructor
	  */
	 virtual ~GFileVectorReferenceParsableParameterT() { /* nothing */ }

	 /***************************************************************************/
	 /**
	  * Assigns the parsed parameters to the reference vector
	  */
	 virtual void executeCallBackFunction() {
		 m_stored_reference = GVectorParT<parameter_type>::m_par;
	 }

protected:
	 /***************************************************************************/
	 /**
	  * Loads data from a property_tree object
	  *
	  * @param pt The object from which data should be loaded
	  */
	 virtual void load(const boost::property_tree::ptree &pt) {
		 using namespace boost::property_tree;

		 // Make sure the recipient vector is empty
		 GVectorParT<parameter_type>::m_par.clear();

		 std::string ppath = GParsableI::optionName(0) + ".value";
		 for(const auto& v: pt.get_child(ppath.c_str())) {
			 GVectorParT<parameter_type>::m_par.push_back(boost::lexical_cast<parameter_type>(v.second.data()));
		 }
	 }

	 /***************************************************************************/
	 /**
	  * Saves data to a property tree object, including comments. Default
	  * values are taken from the def_val_ vector. Note that there needs
	  * to be at least a single default value in it.
	  *
	  * @param pt The object to which data should be saved
	  */
	 virtual void save(boost::property_tree::ptree &pt) const {
		 // Check that we have the right number of comments
		 if (this->hasComments()) {
			 if (this->numberOfComments() != 1) {
				 throw gemfony_exception(
					 g_error_streamer(DO_LOG, time_and_place)
						 << "In GFileVectorReferenceParsableParameterT<>::save(): Error!" << std::endl
						 << "Expected 0 or 1 comment but got " << this->numberOfComments() << std::endl
				 );
			 }

			 // Retrieve a list of sub-comments
			 std::vector<std::string> comments = GParsableI::splitComment(this->comment(0));
			 std::vector<std::string>::iterator c;
			 if (!comments.empty()) {
				 for (c = comments.begin(); c != comments.end(); ++c) {
					 pt.add((GParsableI::optionName(0) + ".comment").c_str(), (*c).c_str());
				 }
			 }
		 }

		 // Do some error checking
		 if (GVectorParT<parameter_type>::m_def_val.empty()) {
			 throw gemfony_exception(
				 g_error_streamer(DO_LOG, time_and_place)
					 << "In GFileVectorReferenceParsableParameterT::save(): Error!" << std::endl
					 << "You need to provide at least one default value" << std::endl
			 );
		 }

		 // Add the value and default items
		 typename std::vector<parameter_type>::const_iterator it;
		 for (it = GVectorParT<parameter_type>::m_def_val.begin();
				it != GVectorParT<parameter_type>::m_def_val.end(); ++it) {
			 pt.add((GParsableI::optionName(0) + ".default.item").c_str(), *it);
			 pt.add((GParsableI::optionName(0) + ".value.item").c_str(), *it);
		 }
	 }

private:
	 /***************************************************************************/
	 // Prevent copying, moving and default construction
	 GFileVectorReferenceParsableParameterT() = delete;
	 GFileVectorReferenceParsableParameterT(const GFileVectorReferenceParsableParameterT<parameter_type>&) = delete;
	 GFileVectorReferenceParsableParameterT(const GFileVectorReferenceParsableParameterT<parameter_type>&&) = delete;
	  GFileVectorReferenceParsableParameterT<parameter_type>& operator=(const GFileVectorReferenceParsableParameterT<parameter_type>&) = delete;
	  GFileVectorReferenceParsableParameterT<parameter_type>& operator=(const GFileVectorReferenceParsableParameterT<parameter_type>&&) = delete;

	 std::vector<parameter_type> &m_stored_reference; ///< Holds a reference to the target vector
};

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/
/**
 * A base class for array parameters. This class was introduced so we can
 * reset the default values in a central location rather than having to
 * convert to different target class. This makes user-code easier.
 */
template<typename parameter_type, std::size_t N>
struct GArrayParT
	: public GFileParsableI {
	 // We want GParserBuilder to be able to call the reset function
	 friend class GParserBuilder;

public:
	 /***************************************************************************/
	 /**
	  * Initializes the parameter and sets values in the parent class
	  */
	 GArrayParT(
		 const std::string &optionNameVar, const std::string &commentVar, const std::array<parameter_type, N> &def_val,
		 const bool &isEssentialVar
	 )
		 : GFileParsableI(
		 optionNameVar, commentVar, isEssentialVar
	 ), m_def_val(def_val), m_par(def_val) { /* nothing */ }

	 /***************************************************************************/
	 /**
	  * The destructor
	  */
	 virtual ~GArrayParT() { /* nothing */ }

protected:
	 /***************************************************************************/
	 /**
	  * Allows derived classes to reset the default value. The function assumes
	  * that no important data is stored in par_, as its value will be overwritten
	  * as well. The reason is that configuration files will otherwise contain
	  * the "old" par_-value.
	  */
	 void resetDefault(const std::array<parameter_type, N> &def_val) {
		 m_def_val = def_val;
		 m_par = def_val;
	 }

	 /***************************************************************************/
	 std::array<parameter_type, N> m_def_val; ///< Holds default values
	 std::array<parameter_type, N> m_par; ///< Holds the parsed parameters

private:
	 /***************************************************************************/
	 // Prevent copying, moving and default construction
	 GArrayParT() = delete;
	 GArrayParT(const GArrayParT<parameter_type, N>&) = delete;
	 GArrayParT(const GArrayParT<parameter_type, N>&&) = delete;
	  GArrayParT<parameter_type, N>& operator=(const GArrayParT<parameter_type, N>&) = delete;
	  GArrayParT<parameter_type, N>& operator=(const GArrayParT<parameter_type, N>&&) = delete;
};

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/
/**
 * This class wraps a std::array of values (obviously of identical type).
 * This class enforces a fixed number of items in the array.
 */
template<typename parameter_type, std::size_t N>
class GFileArrayParsableParameterT
	: public GArrayParT<parameter_type, N> {
	 // We want GParserBuilder to be able to call our load- and save functions
	 friend class GParserBuilder;

public:
	 /***************************************************************************/
	 /**
	  * Initializes the parameters
	  */
	 GFileArrayParsableParameterT(
		 const std::string &optionNameVar, const std::string &commentVar, const std::array<parameter_type, N> &def_val,
		 const bool &isEssentialVar
	 )
		 : GArrayParT<parameter_type, N>(
		 optionNameVar, commentVar, def_val, isEssentialVar
	 ) { /* nothing */ }

	 /***************************************************************************/
	 /**
	  * Initializes the parameters, except for comments
	  */
	 GFileArrayParsableParameterT(
		 const std::string &optionNameVar, const std::array<parameter_type, N> &def_val
	 )
		 : GArrayParT<parameter_type, N>(
		 optionNameVar, std::string(""), def_val, Gem::Common::VAR_IS_ESSENTIAL
	 ) { /* nothing */ }

	 /***************************************************************************/
	 /**
	  * The destructor
	  */
	 virtual ~GFileArrayParsableParameterT() { /* nothing */ }

	 /***************************************************************************/
	 /**
	  * Executes a stored call-back function
	  */
	 virtual void executeCallBackFunction() {
		 if (!m_call_back_func) {
			 throw gemfony_exception(
				 g_error_streamer(DO_LOG, time_and_place)
					 << "In GFileArrayParsableParameterT::executeCallBackFunction(): Error" << std::endl
					 << "Tried to execute call-back function without a stored function" << std::endl
			 );
		 }

		 // Execute the function
		 m_call_back_func(GArrayParT<parameter_type, N>::m_par);
	 }

	 /***************************************************************************/
	 /**
	  * Allows to register a call-back function with this object
	  *
	  * @param callBack The function to be executed
	  */
	 void registerCallBackFunction(std::function<void(std::array<parameter_type, N>)> callBack) {
		 if (!callBack) {
			 throw gemfony_exception(
				 g_error_streamer(DO_LOG, time_and_place)
					 << "In GFileArrayParsableParameterT::registerCallBackFunction(): Error" << std::endl
					 << "Tried to register an empty call-back function" << std::endl
			 );
		 }

		 m_call_back_func = callBack;
	 }

protected:
	 /***************************************************************************/
	 /**
	  * Loads data from a property_tree object
	  *
	  * @param pt The object from which data should be loaded
	  */
	 virtual void load(const boost::property_tree::ptree &pt) {
		 using namespace boost::property_tree;

		 for (std::size_t i = 0; i < GArrayParT<parameter_type, N>::m_par.size(); i++) {
			 GArrayParT<parameter_type, N>::m_par.at(i) = pt.get(
				 (GParsableI::optionName(0) + "." + boost::lexical_cast<std::string>(i) + ".value").c_str(),
				 GArrayParT<parameter_type, N>::m_def_val.at(i));
		 }
	 }

	 /***************************************************************************/
	 /**
	  * Saves data to a property tree object, including comments. Default
	  * values are taken from the def_val_ vector.
	  *
	  * @param pt The object to which data should be saved
	  */
	 virtual void save(boost::property_tree::ptree &pt) const {
		 // Check that we have the right number of comments
		 if (this->hasComments()) {
			 if (this->numberOfComments() != 1) {
				 throw gemfony_exception(
					 g_error_streamer(DO_LOG, time_and_place)
						 << "In GFileArrayParsableParameterT<>::save(): Error!" << std::endl
						 << "Expected 0 or 1 comment but got " << this->numberOfComments() << std::endl
				 );
			 }

			 // Retrieve a list of sub-comments
			 std::vector<std::string> comments = GParsableI::splitComment(this->comment(0));
			 std::vector<std::string>::iterator c;
			 if (!comments.empty()) {
				 for (c = comments.begin(); c != comments.end(); ++c) {
					 pt.add((GParsableI::optionName(0) + ".comment").c_str(), (*c).c_str());
				 }
			 }
		 }

		 // Do some error checking
		 if (GArrayParT<parameter_type, N>::m_def_val.empty()) {
			 throw gemfony_exception(
				 g_error_streamer(DO_LOG, time_and_place)
					 << "In GFileArrayParsableParameterT::save(): Error!" << std::endl
					 << "You need to provide at least one default value" << std::endl
			 );
		 }

		 // Add the value and default items
		 for (std::size_t i = 0; i < GArrayParT<parameter_type, N>::m_def_val.size(); i++) {
			 pt.add((GParsableI::optionName(0) + "." + boost::lexical_cast<std::string>(i) + ".default").c_str(),
				 GArrayParT<parameter_type, N>::m_def_val.at(i));
			 pt.add((GParsableI::optionName(0) + "." + boost::lexical_cast<std::string>(i) + ".value").c_str(),
				 GArrayParT<parameter_type, N>::m_par.at(i));
		 }
	 }

private:
	 /***************************************************************************/
	 // Prevent copying, moving and default construction
	 GFileArrayParsableParameterT() = delete;
	 GFileArrayParsableParameterT(const GFileArrayParsableParameterT<parameter_type, N>&) = delete;
	 GFileArrayParsableParameterT(const GFileArrayParsableParameterT<parameter_type, N>&&) = delete;
	  GFileArrayParsableParameterT<parameter_type, N>& operator=(const GFileArrayParsableParameterT<parameter_type, N>&) = delete;
	  GFileArrayParsableParameterT<parameter_type, N>& operator=(const GFileArrayParsableParameterT<parameter_type, N>&&) = delete;

	 std::function<void(std::array<parameter_type, N>)> m_call_back_func; ///< Holds the call-back function
};

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/
/**
 * This class wraps a reference to a std::array of values (obviously of
 * identical type). This class enforces a fixed number of items in the array.
 */
template<typename parameter_type, std::size_t N>
class GFileArrayReferenceParsableParameterT
	: public GArrayParT<parameter_type, N> {
	 // We want GParserBuilder to be able to call our load- and save functions
	 friend class GParserBuilder;

public:
	 /***************************************************************************/
	 /**
	  * Initializes the parameters
	  */
	 GFileArrayReferenceParsableParameterT(
		 std::array<parameter_type, N> &stored_reference, const std::string &optionNameVar,
		 const std::string &commentVar, const std::array<parameter_type, N> &def_val, const bool &isEssentialVar
	 )
		 : GArrayParT<parameter_type, N>(
		 optionNameVar, commentVar, def_val, isEssentialVar
	 ), m_stored_reference(stored_reference) { /* nothing */ }

	 /***************************************************************************/
	 /**
	  * Initializes the parameters, except for comments
	  */
	 GFileArrayReferenceParsableParameterT(
		 std::array<parameter_type, N> &stored_reference, const std::string &optionNameVar,
		 const std::array<parameter_type, N> &def_val
	 )
		 : GArrayParT<parameter_type, N>(
		 optionNameVar, std::string(""), def_val, Gem::Common::VAR_IS_ESSENTIAL
	 ), m_stored_reference(stored_reference) { /* nothing */ }

	 /***************************************************************************/
	 /**
	  * The destructor
	  */
	 virtual ~GFileArrayReferenceParsableParameterT() { /* nothing */ }

	 /***************************************************************************/
	 /**
	  * Assigns the parsed parameters to the reference vector
	  */
	 virtual void executeCallBackFunction() {
		 m_stored_reference = GArrayParT<parameter_type, N>::m_par;
	 }

protected:
	 /***************************************************************************/
	 /**
	  * Loads data from a property_tree object
	  *
	  * @param pt The object from which data should be loaded
	  */
	 virtual void load(const boost::property_tree::ptree &pt) {
		 using namespace boost::property_tree;

		 for (std::size_t i = 0; i < GArrayParT<parameter_type, N>::m_par.size(); i++) {
			 GArrayParT<parameter_type, N>::m_par.at(i) = pt.get(
				 (GParsableI::optionName(0) + "." + boost::lexical_cast<std::string>(i) + ".value").c_str(),
				 GArrayParT<parameter_type, N>::m_def_val.at(i));
		 }
	 }

	 /***************************************************************************/
	 /**
	  * Saves data to a property tree object, including comments. Default
	  * values are taken from the def_val_ vector.
	  *
	  * @param pt The object to which data should be saved
	  */
	 virtual void save(boost::property_tree::ptree &pt) const {
		 // Check that we have the right number of comments
		 if (this->hasComments()) {
			 if (this->numberOfComments() != 1) {
				 throw gemfony_exception(
					 g_error_streamer(DO_LOG, time_and_place)
						 << "In GFileArrayReferenceParsableParameterT<>::save(): Error!" << std::endl
						 << "Expected 0 or 1 comment but got " << this->numberOfComments() << std::endl
				 );
			 }

			 // Retrieve a list of sub-comments
			 std::vector<std::string> comments = GParsableI::splitComment(this->comment(0));
			 std::vector<std::string>::iterator c;
			 if (!comments.empty()) {
				 for (c = comments.begin(); c != comments.end(); ++c) {
					 pt.add((GParsableI::optionName(0) + ".comment").c_str(), (*c).c_str());
				 }
			 }
		 }

		 // Do some error checking
		 if (GArrayParT<parameter_type, N>::m_def_val.empty()) {
			 throw gemfony_exception(
				 g_error_streamer(DO_LOG, time_and_place)
					 << "In GFileArrayReferenceParsableParameterT::save(): Error!" << std::endl
					 << "You need to provide at least one default value" << std::endl
			 );
		 }

		 // Add the value and default items
		 for (std::size_t i = 0; i < GArrayParT<parameter_type, N>::m_def_val.size(); i++) {
			 pt.add((GParsableI::optionName(0) + "." + boost::lexical_cast<std::string>(i) + ".default").c_str(),
				 GArrayParT<parameter_type, N>::m_def_val.at(i));
			 pt.add((GParsableI::optionName(0) + "." + boost::lexical_cast<std::string>(i) + ".value").c_str(),
				 GArrayParT<parameter_type, N>::m_par.at(i));
		 }
	 }

private:
	 /***************************************************************************/
	 // Prevent copying, moving and default construction
	 GFileArrayReferenceParsableParameterT() = delete;
	 GFileArrayReferenceParsableParameterT(const GFileArrayReferenceParsableParameterT<parameter_type, N>&) = delete;
	 GFileArrayReferenceParsableParameterT(const GFileArrayReferenceParsableParameterT<parameter_type, N>&&) = delete;
	  GFileArrayReferenceParsableParameterT<parameter_type, N>& operator=(const GFileArrayReferenceParsableParameterT<parameter_type, N>&) = delete;
	  GFileArrayReferenceParsableParameterT<parameter_type, N>& operator=(const GFileArrayReferenceParsableParameterT<parameter_type, N>&&) = delete;

	 std::array<parameter_type, N> &m_stored_reference; ///< Holds a reference to the target vector
};

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/
/**
 * This class wraps a reference to individual command line parameters.
 */
template<typename parameter_type>
class GCLReferenceParsableParameterT
	: public GCLParsableI {
	 // We want GParserBuilder to be able to call our private functions
	 friend class GParserBuilder;

public:
	 /***************************************************************************/
	 /**
	  * A constructor that initializes the internal reference
	  *
	  * @param storedReference A reference to a variable in which parsed values should be stored
	  * @param defVal The default value of this variable
	  */
	 GCLReferenceParsableParameterT(
		 parameter_type &storedReference, const std::string &optionNameVar, const std::string &commentVar,
		 parameter_type defVal, bool implicitAllowed, parameter_type implVal
	 )
		 : GCLParsableI(
		 GCLParsableI::makeVector(optionNameVar), GCLParsableI::makeVector(commentVar)
	 ), m_stored_reference(storedReference), m_def_val(defVal), m_implicit_allowed(implicitAllowed),
		 m_impl_val(implVal) { /* nothing */ }

	 /***************************************************************************/
	 /**
	  * A constructor that initializes the internal variiables, except for comments
	  *
	  * @param storedReference A reference to a variable in which parsed values should be stored
	  * @param defVal The default value of this variable
	  */
	 GCLReferenceParsableParameterT(
		 parameter_type &storedReference, const std::string &optionNameVar, parameter_type defVal, bool implicitAllowed,
		 parameter_type implVal
	 )
		 : GCLParsableI(
		 GCLParsableI::makeVector(optionNameVar), GCLParsableI::makeVector(std::string(""))
	 ), m_stored_reference(storedReference), m_def_val(defVal), m_implicit_allowed(implicitAllowed),
		 m_impl_val(implVal) { /* nothing */ }

protected:
	 /***************************************************************************/
	 /**
	  * Saves data to a property tree object
	  */
	 virtual void save(boost::program_options::options_description &desc) const {
		 namespace po = boost::program_options;
		 if (GCL_IMPLICIT_ALLOWED == m_implicit_allowed) {
			 desc.add_options()(
				 (this->optionName()).c_str(),
				 po::value<parameter_type>(&m_stored_reference)->implicit_value(m_impl_val)->default_value(m_def_val),
				 (this->comment()).c_str()
			 );
		 } else { // GCL_IMPLICIT_NOT_ALLOWED
			 desc.add_options()(
				 (this->optionName()).c_str(), po::value<parameter_type>(&m_stored_reference)->default_value(m_def_val),
				 (this->comment()).c_str()
			 );
		 }
	 }

	 /***************************************************************************/
	 /**
	  * Returns the content of this object as a std::string
	  */
	 virtual std::string content() const {
		 std::ostringstream result;
		 result << this->optionName() << " :\t" << m_stored_reference << "\t" <<
				  ((m_stored_reference != m_def_val) ? "default: " + boost::lexical_cast<std::string>(m_def_val) : std::string(""));
		 return result.str();
	 }

private:
	 /***************************************************************************/
	 // Prevent copying, moving and default construction
	 GCLReferenceParsableParameterT() = delete;
	 GCLReferenceParsableParameterT(const GCLReferenceParsableParameterT<parameter_type>&) = delete;
	 GCLReferenceParsableParameterT(const GCLReferenceParsableParameterT<parameter_type>&&) = delete;
	  GCLReferenceParsableParameterT<parameter_type>& operator=(const GCLReferenceParsableParameterT<parameter_type>&) = delete;
	  GCLReferenceParsableParameterT<parameter_type>& operator=(const GCLReferenceParsableParameterT<parameter_type>&&) = delete;

	 parameter_type &m_stored_reference; ///< Holds the reference to which the parsed value will be assigned
	 parameter_type m_def_val; ///< Holds the default value
	 bool m_implicit_allowed; ///< Indicates, whether implicit values (e.g. --server=true vs. --server) are allowed
	 parameter_type m_impl_val; ///< Holds an implicit value used if only the option name is given
};


/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/
/**
 * This class implements a "parser builder", that allows to easily specify
 * the options that the parser should search for in a configuration file.
 * Results of the parsing process will be written directly into the supplied
 * variables. If not found, a default value will be used. Note that this
 * class assumes that the parameter_type can be streamed using operator<< or
 * operator>>
 */
class GParserBuilder {
public:
	 /** @brief The default constructor */
	 G_API_COMMON GParserBuilder();

	 /** @brief The destructor */
	 virtual G_API_COMMON ~GParserBuilder();

	 /** @brief Tries to parse a given configuration file for a set of options */
	 G_API_COMMON bool parseConfigFile(const std::string &);
	 /** @brief Writes out a configuration file */
	 G_API_COMMON void writeConfigFile(const std::string & = std::string(), const std::string & = "", bool = true) const;
	 /** @brief Provides information on the number of file configuration options stored in this class */
	 G_API_COMMON std::size_t numberOfFileOptions() const;

	 /** @brief Parses the commandline for options */
	 G_API_COMMON bool parseCommandLine(int, char **, const bool & = false);
	 /** @brief Provides information on the number of command line configuration options stored in this class */
	 G_API_COMMON std::size_t numberOfCLOptions() const;

	 /***************************************************************************/
	 /**
	  * Allows to retrieve a GFileParsableI-derivative by name and to convert it to
	  * the derived type. This allows us to selectively change properties of these
	  * objects.
	  */
	 template<typename fileParsableDerivative>
	 std::shared_ptr <fileParsableDerivative> file_at(const std::string &optionName) {
		 // Check whether the option already exists. If not, complain
		 std::vector<std::shared_ptr < GFileParsableI>> ::iterator
			 it;
		 if ((it = std::find_if(m_file_parameter_proxies.begin(), m_file_parameter_proxies.end(),
			 findFileProxyByName(optionName))) == m_file_parameter_proxies.end()) {
			 return std::shared_ptr<fileParsableDerivative>();
		 }

		 return std::dynamic_pointer_cast<fileParsableDerivative>(*it);
	 }

	 /***************************************************************************/
	 /**
	  * Allows to retrieve a GCLParsableI-derivative by name and to convert it to
	  * the derived type. This allows us to selectively change properties of these
	  * objects.
	  */
	 template<typename clParsableDerivative>
	 std::shared_ptr <clParsableDerivative> cl_at(const std::string &optionName) {
		 // Check whether the option already exists. If not, complain
		 std::vector<std::shared_ptr < GCLParsableI>> ::iterator
			 it;
		 if ((it = std::find_if(m_cl_parameter_proxies.begin(), m_cl_parameter_proxies.end(),
			 findCLProxyByName(optionName))) == m_cl_parameter_proxies.end()) {
			 return std::shared_ptr<clParsableDerivative>();
		 }

		 return std::dynamic_pointer_cast<clParsableDerivative>(*it);
	 }

	 /////////////////////////////////////////////////////////////////////////////
	 /***************************************************************************/
	 /**
	  * Adds a single parameter of configurable type to the collection. When
	  * this parameter has been read using parseConfigFile, a call-back
	  * function is executed.
	  */
	 template<typename parameter_type>
	 GParsableI &registerFileParameter(
		 const std::string &optionName
		 , parameter_type def_val
		 , std::function<void(parameter_type)> callBack
		 , const bool &isEssential = Gem::Common::VAR_IS_ESSENTIAL
		 , const std::string &comment = std::string()
	 ) {
#ifdef DEBUG
		 // Check whether the option already exists
		 std::vector<std::shared_ptr<GFileParsableI>>::iterator it;
		 if((it=std::find_if(m_file_parameter_proxies.begin(), m_file_parameter_proxies.end(), findFileProxyByName(optionName))) != m_file_parameter_proxies.end()) {
			 throw gemfony_exception(
				 g_error_streamer(DO_LOG, time_and_place)
					 << "In GParserBuilder::registerFileParameter(singleParm_ptr): Error!" << std::endl
					 << "Parameter " << optionName << " has already been registered" << std::endl
			 );
		 }
#endif /* DEBUG */

		 std::shared_ptr <GFileSingleParsableParameterT<parameter_type>> singleParm_ptr;

		 if (comment.empty() || comment == "") {
			 singleParm_ptr = std::shared_ptr<GFileSingleParsableParameterT<parameter_type>>(
				 new GFileSingleParsableParameterT<parameter_type>(
					 optionName, def_val
				 )
			 );
		 } else {
			 singleParm_ptr = std::shared_ptr<GFileSingleParsableParameterT<parameter_type>>(
				 new GFileSingleParsableParameterT<parameter_type>(
					 optionName, comment, isEssential, def_val
				 )
			 );
		 }

		 singleParm_ptr->registerCallBackFunction(callBack);

		 // Add to the proxy store
		 m_file_parameter_proxies.push_back(singleParm_ptr);
		 return *singleParm_ptr;
	 }

	 /***************************************************************************/
	 /**
	  * Adds a parameter with a configurable type to the collection.
	  *
	  * @param optionName The name of the option
	  * @param parameter The parameter into which the value will be written
	  * @param def_val A default value to be used if the corresponding parameter was not found in the configuration file
	  * @param isEssential A boolean which indicates whether this is an essential or a secondary parameter
	  * @param comment A comment to be associated with the parameter in configuration files
	  */
	 template<typename parameter_type>
	 GParsableI &registerFileParameter(
		 const std::string &optionName
		 , parameter_type &parameter
		 , parameter_type def_val
		 , const bool &isEssential = Gem::Common::VAR_IS_ESSENTIAL
		 , const std::string &comment = std::string()
	 ) {
#ifdef DEBUG
		 // Check whether the option already exists
		 std::vector<std::shared_ptr<GFileParsableI>>::iterator it;
		 if((it=std::find_if(m_file_parameter_proxies.begin(), m_file_parameter_proxies.end(), findFileProxyByName(optionName))) != m_file_parameter_proxies.end()) {
			 throw gemfony_exception(
				 g_error_streamer(DO_LOG, time_and_place)
					 << "In GParserBuilder::registerFileParameter(refParm_ptr): Error!" << std::endl
					 << "Parameter " << optionName << " has already been registered" << std::endl
			 );
		 }
#endif /* DEBUG */

		 std::shared_ptr <GFileReferenceParsableParameterT<parameter_type>> refParm_ptr;

		 if (comment.empty() || comment == "") {
			 refParm_ptr = std::shared_ptr<GFileReferenceParsableParameterT<parameter_type>>(
				 new GFileReferenceParsableParameterT<parameter_type>(
					 parameter, optionName, def_val
				 )
			 );
		 } else {
			 refParm_ptr = std::shared_ptr<GFileReferenceParsableParameterT<parameter_type>>(
				 new GFileReferenceParsableParameterT<parameter_type>(
					 parameter, optionName, comment, isEssential, def_val
				 )
			 );
		 }

		 // Add to the proxy store
		 m_file_parameter_proxies.push_back(refParm_ptr);
		 return *refParm_ptr;
	 }

	 /***************************************************************************/
	 /**
	  * Allows to reset default values. This is useful, if a derived class needs
	  * a different default value in configuration files. This function is meant
	  * to be called before any parsing takes place, as the par_-value will be
	  * overwritten as well.
	  */
	 template<typename parameter_type>
	 void resetFileParameterDefaults(
		 const std::string &optionName, parameter_type def_val
	 ) {
		 // Retrieve the parameter object with this name
		 std::shared_ptr <GSingleParmT<parameter_type>> parmObject
			 = file_at<GSingleParmT<parameter_type>>(optionName);

		 // Check that we have indeed received an item
		 if (!parmObject) {
			 throw gemfony_exception(
				 g_error_streamer(DO_LOG, time_and_place)
					 << "In GParameterObject::resetFileParameterDefaults(GSingleParmT): Error!" << std::endl
					 << "Parameter object couldn't be found" << std::endl
			 );
		 }

		 // Reset the default value
		 parmObject->resetDefault(def_val);
	 }

	 /////////////////////////////////////////////////////////////////////////////
	 /***************************************************************************/
	 /**
	  * Adds two parameters of configurable types to the collection. When
	  * these parameters have been read using parseConfigFile, a call-back
	  * function will be executed.
	  */
	 template<typename par_type1, typename par_type2>
	 GParsableI &registerFileParameter(
		 const std::string &optionName1, const std::string &optionName2, par_type1 def_val1, par_type2 def_val2,
		 std::function<void(par_type1, par_type2)> callBack, const std::string &combined_label,
		 bool isEssential = Gem::Common::VAR_IS_ESSENTIAL, const std::string &comment1 = std::string(),
		 const std::string &comment2 = std::string()
	 ) {
#ifdef DEBUG
		 // Check whether the option already exists
		 std::vector<std::shared_ptr<GFileParsableI>>::iterator it;
		 if((it=std::find_if(m_file_parameter_proxies.begin(), m_file_parameter_proxies.end(), findFileProxyByName(optionName1))) != m_file_parameter_proxies.end()) {
			 throw gemfony_exception(
				 g_error_streamer(DO_LOG, time_and_place)
					 << "In GParserBuilder::registerFileParameter(combParm_ptr): Error!" << std::endl
					 << "Parameter " << optionName1 << " has already been registered" << std::endl
			 );
		 }
#endif /* DEBUG */

		 std::shared_ptr <GFileCombinedParsableParameterT<par_type1, par_type2>> combParm_ptr;

		 if ((comment1.empty() || comment1 == "") && (comment2.empty() || comment2 == "")) {
			 combParm_ptr = std::shared_ptr<GFileCombinedParsableParameterT<par_type1, par_type2>>(
				 new GFileCombinedParsableParameterT<par_type1, par_type2>(
					 optionName1, def_val1, optionName2, def_val2, combined_label
				 )
			 );
		 } else {
			 combParm_ptr = std::shared_ptr<GFileCombinedParsableParameterT<par_type1, par_type2>>(
				 new GFileCombinedParsableParameterT<par_type1, par_type2>(
					 optionName1, comment1, def_val1, optionName2, comment2, def_val2, isEssential, combined_label
				 )
			 );
		 }

		 combParm_ptr->registerCallBackFunction(callBack);

		 // Add to the proxy store
		 m_file_parameter_proxies.push_back(combParm_ptr);
		 return *combParm_ptr;
	 }

	 /***************************************************************************/
	 /**
	  * Allows to reset default values. This is useful, if a derived class needs
	  * a different default value in configuration files. Note that we only need
	  * the first option name here, but two default values. This function is meant
	  * to be called before any parsing takes place, as the par_-value will be
	  * overwritten as well.
	  */

	 template<typename par_type1, typename par_type2>
	 void resetFileParameterDefaults(
		 const std::string &optionName1, par_type1 def_val1, par_type2 def_val2
	 ) {
		 // Retrieve the parameter object with this name
		 std::shared_ptr <GCombinedParT<par_type1, par_type2>> parmObject
			 = file_at<GCombinedParT<par_type1, par_type2>>(optionName1);

		 // Check that we have indeed received an item
		 if (!parmObject) {
			 throw gemfony_exception(
				 g_error_streamer(DO_LOG, time_and_place)
					 << "In GParameterObject::resetFileParameterDefaults(GCombinedParT): Error!" << std::endl
					 << "Parameter object couldn't be found" << std::endl
			 );
		 }

		 // Reset the default value
		 parmObject->resetDefault(def_val1, def_val2);
	 }

	 /////////////////////////////////////////////////////////////////////////////
	 /***************************************************************************/
	 /**
	  * Adds a vector of configurable type to the collection, using a
	  * call-back function
	  */
	 template<typename parameter_type>
	 GParsableI &registerFileParameter(
		 const std::string &optionName, const std::vector<parameter_type> &def_val,
		 std::function<void(std::vector<parameter_type>)> callBack,
		 const bool &isEssential = Gem::Common::VAR_IS_ESSENTIAL, const std::string &comment = std::string()
	 ) {
#ifdef DEBUG
		 // Check whether the option already exists
		 std::vector<std::shared_ptr<GFileParsableI>>::iterator it;
		 if((it=std::find_if(m_file_parameter_proxies.begin(), m_file_parameter_proxies.end(), findFileProxyByName(optionName))) != m_file_parameter_proxies.end()) {
			 throw gemfony_exception(
				 g_error_streamer(DO_LOG, time_and_place)
					 << "In GParserBuilder::registerFileParameter(vecParm_ptr): Error!" << std::endl
					 << "Parameter " << optionName << " has already been registered" << std::endl
			 );
		 }
#endif /* DEBUG */

		 std::shared_ptr <GFileVectorParsableParameterT<parameter_type>> vecParm_ptr;

		 if (comment.empty() || comment == "") {
			 vecParm_ptr = std::shared_ptr<GFileVectorParsableParameterT<parameter_type>>(
				 new GFileVectorParsableParameterT<parameter_type>(
					 optionName, def_val
				 )
			 );
		 } else {
			 vecParm_ptr = std::shared_ptr<GFileVectorParsableParameterT<parameter_type>>(
				 new GFileVectorParsableParameterT<parameter_type>(
					 optionName, comment, def_val, isEssential
				 )
			 );
		 }

		 vecParm_ptr->registerCallBackFunction(callBack);

		 // Add to the proxy store
		 m_file_parameter_proxies.push_back(vecParm_ptr);
		 return *vecParm_ptr;
	 }

	 /***************************************************************************/
	 /**
	  * Adds a reference to a vector of configurable type to the collection
	  */
	 template<typename parameter_type>
	 GParsableI &registerFileParameter(
		 const std::string &optionName, std::vector<parameter_type> &stored_reference,
		 const std::vector<parameter_type> &def_val, const bool &isEssential = Gem::Common::VAR_IS_ESSENTIAL,
		 const std::string &comment = std::string()
	 ) {
#ifdef DEBUG
		 // Check whether the option already exists
		 std::vector<std::shared_ptr<GFileParsableI>>::iterator it;
		 if((it=std::find_if(m_file_parameter_proxies.begin(), m_file_parameter_proxies.end(), findFileProxyByName(optionName))) != m_file_parameter_proxies.end()) {
			 throw gemfony_exception(
				 g_error_streamer(DO_LOG, time_and_place)
					 << "In GParserBuilder::registerFileParameter(vecRefParm_ptr): Error!" << std::endl
					 << "Parameter " << optionName << " has already been registered" << std::endl
			 );
		 }
#endif /* DEBUG */


		 std::shared_ptr <GFileVectorReferenceParsableParameterT<parameter_type>> vecRefParm_ptr;

		 if (comment.empty() || comment == "") {
			 vecRefParm_ptr = std::shared_ptr<GFileVectorReferenceParsableParameterT<parameter_type>>(
				 new GFileVectorReferenceParsableParameterT<parameter_type>(
					 stored_reference, optionName, def_val
				 )
			 );
		 } else {
			 vecRefParm_ptr = std::shared_ptr<GFileVectorReferenceParsableParameterT<parameter_type>>(
				 new GFileVectorReferenceParsableParameterT<parameter_type>(
					 stored_reference, optionName, comment, def_val, isEssential
				 )
			 );
		 }

		 // Add to the proxy store
		 m_file_parameter_proxies.push_back(vecRefParm_ptr);
		 return *vecRefParm_ptr;
	 }

	 /***************************************************************************/
	 /**
	  * Allows to reset default values. This is useful, if a derived class needs
	  * a different default value in configuration files. Note that we only need
	  * the first option name here, but two default values. This function is meant
	  * to be called before any parsing takes place, as the par_-value will be
	  * overwritten as well.
	  */
	 template<typename parameter_type>
	 void resetFileParameterDefaults(
		 const std::string &optionName, const std::vector<parameter_type> &def_val
	 ) {
		 // Retrieve the parameter object with this name
		 std::shared_ptr <GVectorParT<parameter_type>> parmObject
			 = file_at<GVectorParT<parameter_type>>(optionName);

		 // Check that we have indeed received an item
		 if (!parmObject) {
			 throw gemfony_exception(
				 g_error_streamer(DO_LOG, time_and_place)
					 << "In GParameterObject::resetFileParameterDefaults(GVectorParT): Error!" << std::endl
					 << "Parameter object couldn't be found" << std::endl
			 );
		 }

		 // Reset the default value
		 parmObject->resetDefault(def_val);
	 }

	 /////////////////////////////////////////////////////////////////////////////
	 /***************************************************************************/
	 /**
	  * Adds an array of configurable type but fixed size to the collection.
	  * This allows to make sure that a given amount of configuration options
	  * must be available.
	  */
	 template<typename parameter_type, std::size_t N>
	 GParsableI &registerFileParameter(
		 const std::string &optionName, const std::array<parameter_type, N> &def_val,
		 std::function<void(std::array<parameter_type, N>)> callBack,
		 const bool &isEssential = Gem::Common::VAR_IS_ESSENTIAL, const std::string &comment = std::string()
	 ) {
#ifdef DEBUG
		 // Check whether the option already exists
		 std::vector<std::shared_ptr<GFileParsableI>>::iterator it;
		 if((it=std::find_if(m_file_parameter_proxies.begin(), m_file_parameter_proxies.end(), findFileProxyByName(optionName))) != m_file_parameter_proxies.end()) {
			 throw gemfony_exception(
				 g_error_streamer(DO_LOG, time_and_place)
					 << "In GParserBuilder::registerFileParameter(arrayParm_ptr): Error!" << std::endl
					 << "Parameter " << optionName << " has already been registered" << std::endl
			 );
		 }
#endif /* DEBUG */

		 std::shared_ptr <GFileArrayParsableParameterT<parameter_type, N>> arrayParm_ptr;

		 if (comment.empty() || comment == "") {
			 arrayParm_ptr = std::shared_ptr<GFileArrayParsableParameterT<parameter_type, N>>(
				 new GFileArrayParsableParameterT<parameter_type, N>(
					 optionName, def_val
				 )
			 );
		 } else {
			 arrayParm_ptr = std::shared_ptr<GFileArrayParsableParameterT<parameter_type, N>>(
				 new GFileArrayParsableParameterT<parameter_type, N>(
					 optionName, comment, def_val, isEssential
				 )
			 );
		 }

		 // Register the call back function
		 arrayParm_ptr->registerCallBackFunction(callBack);

		 // Add to the proxy store
		 m_file_parameter_proxies.push_back(arrayParm_ptr);
		 return *arrayParm_ptr;
	 }

	 /***************************************************************************/
	 /**
	  * Adds a reference to an array of configurable type but fixed size
	  * to the file parameter collection
	  */
	 template<typename parameter_type, std::size_t N>
	 GParsableI &registerFileParameter(
		 const std::string &optionName, std::array<parameter_type, N> &stored_reference,
		 const std::array<parameter_type, N> &def_val, const bool &isEssential = Gem::Common::VAR_IS_ESSENTIAL,
		 const std::string &comment = std::string()
	 ) {
#ifdef DEBUG
		 // Check whether the option already exists
		 std::vector<std::shared_ptr<GFileParsableI>>::iterator it;
		 if((it=std::find_if(m_file_parameter_proxies.begin(), m_file_parameter_proxies.end(), findFileProxyByName(optionName))) != m_file_parameter_proxies.end()) {
			 throw gemfony_exception(
				 g_error_streamer(DO_LOG, time_and_place)
					 << "In GParserBuilder::registerFileParameter(arrayRefParm_ptr): Error!" << std::endl
					 << "Parameter " << optionName << " has already been registered" << std::endl
			 );
		 }
#endif /* DEBUG */

		 std::shared_ptr <GFileArrayReferenceParsableParameterT<parameter_type, N>> arrayRefParm_ptr;
		 if (comment.empty() || comment == "") {
			 arrayRefParm_ptr = std::shared_ptr<GFileArrayReferenceParsableParameterT<parameter_type, N>>(
				 new GFileArrayReferenceParsableParameterT<parameter_type, N>(
					 stored_reference, optionName, def_val
				 )
			 );
		 } else {
			 arrayRefParm_ptr = std::shared_ptr<GFileArrayReferenceParsableParameterT<parameter_type, N>>(
				 new GFileArrayReferenceParsableParameterT<parameter_type, N>(
					 stored_reference, optionName, comment, def_val, isEssential
				 )
			 );
		 }

		 // Add to the proxy store
		 m_file_parameter_proxies.push_back(arrayRefParm_ptr);
		 return *arrayRefParm_ptr;
	 }

	 /***************************************************************************/
	 /**
	  * Allows to reset default values. This is useful, if a derived class needs
	  * a different default value in configuration files. Note that we only need
	  * the first option name here, but two default values. This function is meant
	  * to be called before any parsing takes place, as the par_-value will be
	  * overwritten as well.
	  */

	 template<typename parameter_type, std::size_t N>
	 void resetFileParameterDefaults(
		 const std::string &optionName, const std::array<parameter_type, N> &def_val
	 ) {
		 // Retrieve the parameter object with this name
		 std::shared_ptr <GArrayParT<parameter_type, N>> parmObject
			 = file_at<GArrayParT<parameter_type, N>>(optionName);

		 // Check that we have indeed received an item
		 if (!parmObject) {
			 throw gemfony_exception(
				 g_error_streamer(DO_LOG, time_and_place)
					 << "In GParameterObject::resetFileParameterDefaults(GArrayParT): Error!" << std::endl
					 << "Parameter object couldn't be found" << std::endl
			 );
		 }

		 // Reset the default value
		 parmObject->resetDefault(def_val);
	 }

	 /////////////////////////////////////////////////////////////////////////////
	 /***************************************************************************/
	 /**
	  * Adds a reference to a configurable type to the command line parameters.
	  *
	  * @param optionName The name of the option
	  * @param parameter The parameter into which the value will be written
	  * @param def_val A default value to be used if the corresponding parameter was not found in the configuration file
	  * @param comment A comment to be associated with the parameter in configuration files
	  */
	 template<typename parameter_type>
	 GParsableI &registerCLParameter(
		 const std::string &optionName, parameter_type &parameter, const parameter_type &def_val,
		 const std::string &comment = std::string(), bool implicitAllowed = GCL_IMPLICIT_NOT_ALLOWED,
		 parameter_type impl_val = GDefaultValueT<parameter_type>::value()
	 ) {
#ifdef DEBUG
		 // Check whether the option already exists
		 std::vector<std::shared_ptr<GCLParsableI>>::iterator it;
		 if((it=std::find_if(m_cl_parameter_proxies.begin(), m_cl_parameter_proxies.end(), findCLProxyByName(optionName))) != m_cl_parameter_proxies.end()) {
			 throw gemfony_exception(
				 g_error_streamer(DO_LOG, time_and_place)
					 << "In GParserBuilder::registerCLParameter(refParm_ptr): Error!" << std::endl
					 << "Parameter " << optionName << " has already been registered" << std::endl
			 );
		 }
#endif /* DEBUG */

		 std::shared_ptr <GCLReferenceParsableParameterT<parameter_type>> refParm_ptr;

		 if (comment.empty() || comment == "") {
			 refParm_ptr = std::shared_ptr<GCLReferenceParsableParameterT<parameter_type>>(
				 new GCLReferenceParsableParameterT<parameter_type>(
					 parameter, optionName, def_val, implicitAllowed, impl_val
				 )
			 );
		 } else {
			 refParm_ptr = std::shared_ptr<GCLReferenceParsableParameterT<parameter_type>>(
				 new GCLReferenceParsableParameterT<parameter_type>(
					 parameter, optionName, comment, def_val, implicitAllowed, impl_val
				 )
			 );
		 }

		 // Add to the proxy store
		 m_cl_parameter_proxies.push_back(refParm_ptr);
		 return *refParm_ptr;
	 }

private:
	 /***************************************************************************/
	 /**
	  * Allows to search for file parameter proxies with a given name
	  */
	 struct findFileProxyByName {
		  explicit findFileProxyByName(const std::string &proxyName)
			  : m_proxy_name(proxyName) { /* nothing */ }


		  bool operator()(std::shared_ptr <GFileParsableI> candidate_ptr) const {
			  if (candidate_ptr->GParsableI::optionName(0) == m_proxy_name) {
				  return true;
			  } else {
				  return false;
			  }
		  }

	 private:
		  // Intentionally private and undefined
		  findFileProxyByName() = delete;

		  // The name of the proxy
		  std::string m_proxy_name;
	 };

	 /***************************************************************************/
	 /**
	  * Allows to search for command line parameter proxies with a given name
	  */
	 struct findCLProxyByName {
		  explicit findCLProxyByName(const std::string &proxyName)
			  : m_proxy_name(proxyName) { /* nothing */ }


		  bool operator()(std::shared_ptr <GCLParsableI> candidate_ptr) const {
			  if (candidate_ptr->GParsableI::optionName(0) == m_proxy_name) {
				  return true;
			  } else {
				  return false;
			  }
		  }

	 private:
		  // Intentionally private and undefined
		  findCLProxyByName() = delete;

		  // The name of the proxy
		  std::string m_proxy_name;
	 };

	 /***************************************************************************/
	 // Prevent copying and moving
	 GParserBuilder(const GParserBuilder&) = delete;
	 GParserBuilder(const GParserBuilder&&) = delete;
	  GParserBuilder& operator=(const GParserBuilder&) = delete;
	  GParserBuilder& operator=(const GParserBuilder&&) = delete;


	 std::vector<std::shared_ptr<GFileParsableI>> m_file_parameter_proxies; ///< Holds file parameter proxies
	 std::vector<std::shared_ptr<GCLParsableI>> m_cl_parameter_proxies;   ///< Holds command line parameter proxies

	 std::string m_configfile_Base_name;

	 static std::mutex m_configfile_parser_mutex; ///< Synchronization of access to configuration files (may only happen serially)
};

/******************************************************************************/
/**
 * A helper function that lets users configure a given object from a file.
 * The function assumes that the target object has a suitable addConfigurationOptions
 * function. The function will automatically generate the configuration file using
 * the mechanisms implenented in GParserBuilder, should the file not exist.
 *
 * @param path Name and path to a configuration file
 */
template <typename conf_object_type>
void configureFromFile(
	conf_object_type& target_object
	, const std::string& conf_file
) {
	// Create a parser builder object. It will be destroyed at
	// the end of this scope and thus cannot cause trouble
	// due to registered call-backs and references
	Gem::Common::GParserBuilder gpb;

	// Add configuration options from the target object
	target_object.addConfigurationOptions(gpb);

	//----------------------------------------------------------------------------
	// Some error checking

	boost::filesystem::path conf_file_path(conf_file);

	// Check whether path is a directory name rather than
	// a file. It is a severe error if this is the case.
	if(boost::filesystem::is_directory(conf_file_path)) {
		throw gemfony_exception(
			g_error_streamer(DO_LOG, time_and_place)
				<< "In configureFromFile(" << conf_file << "): Error!" << std::endl
				<< "Target is a directory rather than a file." << std::endl
		);
	}

	// Check whether the target directory exists. It is a
	// severe error if this is not the case.
	if(!boost::filesystem::exists(conf_file_path.parent_path())) {
		throw gemfony_exception(
			g_error_streamer(DO_LOG, time_and_place)
				<< "In configureFromFile(" << conf_file << "): Error!" << std::endl
				<< "Target has invalid parent path" << std::endl
		);
	}

	//----------------------------------------------------------------------------
	// Do the actual parsing
	gpb.parseConfigFile(conf_file);

	//----------------------------------------------------------------------------
}

/******************************************************************************/

} /* namespace Common */
} /* namespace Gem */

#endif /* GPARSERBUILDER_HPP_ */
