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

// Standard headers go here
#include <cstdlib>
#include <fstream>
#include <sstream>
#include <iostream>
#include <vector>
#include <string>

// Includes check for correct Boost version(s)
#include "GGlobalDefines.hpp"

// Boost headers go here
#include <boost/algorithm/string.hpp>
#include <boost/array.hpp>
#include <boost/filesystem.hpp>
#include <boost/thread/thread.hpp>
#include <boost/utility.hpp>
#include <boost/program_options.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/date_time.hpp>
#include <boost/tokenizer.hpp>
#include <boost/function.hpp>
#include <boost/bind.hpp>
#include <boost/foreach.hpp>
#include <boost/thread/mutex.hpp>

/**
 * Check that we have support for threads. This collection of classes is useless
 * without this.
 */
#ifndef BOOST_HAS_THREADS
#error "Error: No support for Boost.threads available"
#endif

#ifndef GPARSERBUILDER_HPP_
#define GPARSERBUILDER_HPP_

// For Microsoft-compatible compilers
#if defined(_MSC_VER)  &&  (_MSC_VER >= 1020)
#pragma once
#endif

// Geneva header files go here
#include <common/GExceptions.hpp>
#include <common/GLogger.hpp>
#include <common/GCommonEnums.hpp>
#include <common/GDefaultValueT.hpp>

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
 * Gives access to a reference to parm_ a single time. When this has happened,
 * only an explicit reset allows to gain access to a parameter-reference again.
 * It is however possible to explicitly set the parameter.
 */
template <typename T>
class GOneTimeRefParameterT
   :private boost::noncopyable
{
public:
   /***************************************************************************/
   /**
    * The standard constructor
    */
   explicit GOneTimeRefParameterT(const T& def = T(NULL))
      : parm_(def)
      , parmDummy_(def)
      , parmSet_(false)
   { /* nothing */ }

   /***************************************************************************/
   /**
    * Returns a reference to the parameter, if it hasn't been set. Otherwise
    * it will return a reference to the dummy parameter.
    */
   T& reference() {
      if(parmSet_) {
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
   void setValue(const T& parm) {
      parm_ = parm;
      parmSet_ = true;
   }

   /***************************************************************************/
   /**
    * Explicit assignment of a T value
    */
   void operator=(const T& parm) {
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
 * This class specifies the interface of parsable parameters, to
 * which a call-back function has been assigned. It also stores some
 * information common to all parameter types.
 */
class GParsableI
   :boost::noncopyable
{
public:
	/** @brief A constructor for individual items */
	GParsableI(
		const std::string&
		, const std::string&
	);

	/** @brief A constructor for vectors */
	GParsableI(
		const std::vector<std::string>&
		, const std::vector<std::string>&
	);

	/** @brief The destructor */
	virtual ~GParsableI();

	/** @brief Retrieves the option name at a given position */
	std::string optionName(std::size_t = 0) const;
	/** @brief Retrieves the comment that was assigned to this variable at a given position */
	std::string comment(std::size_t = 0) const;

   /***************************************************************************/
   /**
    * Create a std::vector<T> from a single element
    */
   template <typename T>
   static std::vector<T> makeVector(const T& item) {
      std::vector<T> result;
      result.push_back(item);
      return result;
   }

   /***************************************************************************/
   /**
    * Create a std::vector<T> from two elements
    */
   template <typename T>
   static std::vector<T> makeVector(const T& item1, const T& item2) {
      std::vector<T> result;
      result.push_back(item1);
      result.push_back(item2);
      return result;
   }

private:
	/***************************************************************************/
	/** @brief The default constructor. Intentionally private and undefined */
	GParsableI();

	std::vector<std::string> optionName_; ///< The name of this parameter
	std::vector<std::string> comment_; ///< A comment assigned to this parameter
};

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/
/**
 * This class specifies the interface of parsable file parameters, to
 * which a call-back function has been assigned. Note that this class cannot
 * be copied, as the parent class is derived from boost::noncopyable.
 */
class GFileParsableI
   : public GParsableI
{
   // We want GParserBuilder to be able to call our private load- and save functions
   friend class GParserBuilder;

public:
   /** @brief A constructor for individual items */
   GFileParsableI(
      const std::string&
      , const std::string&
      , const bool&
   );
   /** @brief A constructor for vectors */
   GFileParsableI(
      const std::vector<std::string>&
      , const std::vector<std::string>&
      , const bool&
   );
   /** @brief The destructor */
   virtual ~GFileParsableI();

   /** @brief Executes a stored call-back function */
   virtual void executeCallBackFunction() = 0;

   /** @brief Checks whether this is an essential variable at a given position */
   bool isEssential() const;

protected:
   /***************************************************************************/
   /** @brief Loads data from a property_tree object */
   virtual void load(const boost::property_tree::ptree&) = 0;
   /** @brief Saves data to a property tree object */
   virtual void save(boost::property_tree::ptree&) const = 0;

private:
   /***************************************************************************/
   /** @brief The default constructor. Intentionally private and undefined */
   GFileParsableI();

   bool isEssential_; ///< Indicates whether this is an essential variable
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
   : public GParsableI
{
   // We want GParserBuilder to be able to call our private load- and save functions
   friend class GParserBuilder;

public:
   /** @brief A constructor for individual items */
   GCLParsableI(
      const std::string&
      , const std::string&
   );
   /** @brief A constructor for vectors */
   GCLParsableI(
      const std::vector<std::string>&
      , const std::vector<std::string>&
   );
   /** @brief The destructor */
   virtual ~GCLParsableI();

protected:
   /** @brief Saves data to a property tree object */
   virtual void save(boost::program_options::options_description&) const = 0;
   /** @brief Returns the content of this object as a std::string */
   virtual std::string content() const = 0;

private:
   /***************************************************************************/
   /** @brief The default constructor. Intentionally private and undefined */
   GCLParsableI();
};

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/
/**
 * This class wraps individual parsable file parameters, to which a callback
 * function has been assigned.
 */
template <typename parameter_type>
struct GFileSingleParsableParameterT
   : public GFileParsableI
{
	// We want GParserBuilder to be able to call our load- and save functions
	friend class GParserBuilder;

public:
	/***************************************************************************/
	/**
	 * Initializes the parameter and sets values in the parent class
	 */
	explicit GFileSingleParsableParameterT(
		const std::string& optionNameVar
		, const std::string& commentVar
		, const bool& isEssentialVar
		, const parameter_type& defVal
	)
		: GFileParsableI(
			optionNameVar
			, commentVar
			, isEssentialVar
		  )
		, par_(defVal)
		, def_val_(defVal)
	{ /* nothing */ }

	/***************************************************************************/
	/**
	 * The destructor
	 */
	virtual ~GFileSingleParsableParameterT()
	{ /* nothing */ }

	/***************************************************************************/
	/**
	 * Executes a stored call-back function
	 */
	virtual void executeCallBackFunction() {
		if(!callBack_) {
		   glogger
		   << "In GSingleParsableParameter::executeCallBackFunction(): Error" << std::endl
         << "Tried to execute call-back function without a stored function" << std::endl
         << GEXCEPTION;
		}

		// Execute the function
		callBack_(par_);
	}

	/***************************************************************************/
	/**
	 * Allows to register a call-back function with this object
	 *
	 * @param callBack The function to be executed
	 */
	void registerCallBackFunction(boost::function<void(parameter_type)> callBack) {
		if(!callBack) {
		   glogger
		   << "In GSingleParsableParameter::registerCallBackFunction(): Error" << std::endl
         << "Tried to register an empty call-back function" << std::endl
         << GEXCEPTION;
		}

		callBack_ = callBack;
	}

protected:
	/***************************************************************************/
	/**
	 * Loads data from a property_tree object
	 *
	 * @param pt The object from which data should be loaded
	 */
	virtual void load(const boost::property_tree::ptree& pt) {
		par_ = pt.get((optionName(0) + ".value").c_str(), def_val_);
	}

	/***************************************************************************/
	/**
	 * Saves data to a property tree object, including comments.
	 *
	 * @param pt The object to which data should be saved
	 */
	virtual void save(boost::property_tree::ptree& pt) const {
		// Needed for the separation of comment strings
		typedef boost::tokenizer<boost::char_separator<char> > tokenizer;
		boost::char_separator<char> semicolon_sep(";");
		std::string comment = this->comment(0);

		if(comment != "") { // Only output useful comments
			// Break the comment into individual lines after each semicolon
			tokenizer commentTokenizer(comment, semicolon_sep);
			for(tokenizer::iterator c=commentTokenizer.begin(); c!=commentTokenizer.end(); ++c) {
				pt.add((optionName(0) + ".comment").c_str(), (*c).c_str());
			}
		}

		pt.put((optionName(0) + ".default").c_str(), def_val_);
		pt.put((optionName(0) + ".value").c_str(), par_);
	}

private:
	/***************************************************************************/
	GFileSingleParsableParameterT(); ///< The default constructor. Intentionally private and undefined

	parameter_type par_; ///< Holds the individual parameter
	parameter_type def_val_; ///< Holds the parameter's default value

	boost::function<void(parameter_type)> callBack_; ///< Holds the call-back function
};

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/
/**
 * This class wraps combined parsable file parameters, to which a callback
 * function has been assigned.
 */
template <typename par_type0, typename par_type1>
struct GFileCombinedParsableParameterT
   : public GFileParsableI
{
	// We want GParserBuilder to be able to call our load- and save functions
	friend class GParserBuilder;

public:
	/***************************************************************************/
	/**
	 * Initializes the parameters
	 */
	GFileCombinedParsableParameterT(
		const std::string& optionNameVar0
		, const std::string& commentVar0
		, const par_type0& defVal0
		, const std::string& optionNameVar1
		, const std::string& commentVar1
		, const par_type1& defVal1
		, const bool& isEssentialVar
		, const std::string& combined_label
	)
		: GFileParsableI(
			GFileParsableI::makeVector(optionNameVar0, optionNameVar1)
			, GFileParsableI::makeVector(commentVar0, commentVar1)
			, isEssentialVar
		  )
		, par0_(defVal0)
		, par1_(defVal1)
		, def_val0_(defVal0)
		, def_val1_(defVal1)
		, combined_label_(combined_label)
	{ /* nothing */ }

	/***************************************************************************/
	/**
	 * The destructor
	 */
	virtual ~GFileCombinedParsableParameterT()
	{ /* nothing */ }

	/***************************************************************************/
	/**
	 * Executes a stored call-back function
	 */
	virtual void executeCallBackFunction() {
		if(!callBack_) {
		   glogger
		   << "In GCombinedParsableParameter::executeCallBackFunction(): Error" << std::endl
         << "Tried to execute call-back function without a stored function" << std::endl
         << GEXCEPTION;
		}

		// Execute the function
		callBack_(par0_, par1_);
	}

	/***************************************************************************/
	/**
	 * Allows to register a call-back function with this object
	 *
	 * @param callBack The function to be executed
	 */
	void registerCallBackFunction(boost::function<void(par_type0, par_type1)> callBack) {
		if(!callBack) {
		   glogger
		   << "In GCombinedParsableParameter::registerCallBackFunction(): Error" << std::endl
         << "Tried to register an empty call-back function" << std::endl
         << GEXCEPTION;
		}

		callBack_ = callBack;
	}

protected:
	/***************************************************************************/
	/**
	 * Loads data from a property_tree object
	 *
	 * @param pt The object from which data should be loaded
	 */
	virtual void load(const boost::property_tree::ptree& pt) {
		par0_ = pt.get((combined_label_ + "." + optionName(0) + ".value").c_str(), def_val0_);
		par1_ = pt.get((combined_label_ + "." + optionName(1) + ".value").c_str(), def_val1_);
	}

	/***************************************************************************/
	/**
	 * Saves data to a property tree object, including comments.
	 *
	 * @param pt The object to which data should be saved
	 */
	virtual void save(boost::property_tree::ptree& pt) const {
		// Needed for the separation of comment strings
		typedef boost::tokenizer<boost::char_separator<char> > tokenizer;
		boost::char_separator<char> semicolon_sep(";");
		std::string comment0 = this->comment(0);
		std::string comment1 = this->comment(1);

		if(comment0 != "") { // Only output useful comments
			// Break the comment into individual lines after each semicolon
			tokenizer commentTokenizer(comment0, semicolon_sep);
			for(tokenizer::iterator c=commentTokenizer.begin(); c!=commentTokenizer.end(); ++c) {
				pt.add((combined_label_ + "." + optionName(0) + ".comment").c_str(), (*c).c_str());
			}
		}
		pt.put((combined_label_ + "." + optionName(0) + ".default").c_str(), def_val0_);
		pt.put((combined_label_ + "." + optionName(0) + ".value").c_str(), par0_);

		if(comment1 != "") { // Only output useful comments
			// Break the comment into individual lines after each semicolon
			tokenizer commentTokenizer(comment1, semicolon_sep);
			for(tokenizer::iterator c=commentTokenizer.begin(); c!=commentTokenizer.end(); ++c) {
				pt.add((combined_label_ + "." + optionName(1) + ".comment").c_str(), (*c).c_str());
			}
		}
		pt.put((combined_label_ + "." + optionName(1) + ".default").c_str(), def_val1_);
		pt.put((combined_label_ + "." + optionName(1) + ".value").c_str(), par1_);
	}

private:
	/***************************************************************************/
	GFileCombinedParsableParameterT(); ///< The default constructor. Intentionally private and undefined

	par_type0 par0_; ///< Holds the first parameter
	par_type1 par1_; ///< Holds the second parameter

	par_type0 def_val0_; ///< Holds the default value of the first parameter
	par_type1 def_val1_; ///< Holds the default value of the second parameter

	std::string combined_label_; ///< Holds a path label for the combined JSON path

	boost::function<void(par_type0, par_type1)> callBack_; ///< Holds the call-back function
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
template <typename parameter_type>
class GFileVectorParsableParameterT
   : public GFileParsableI
{
	// We want GParserBuilder to be able to call our load- and save functions
	friend class GParserBuilder;

public:
	/***************************************************************************/
	/**
	 * Initializes the parameters
	 */
	GFileVectorParsableParameterT(
		const std::string& optionNameVar
		, const std::string& commentVar
		, const std::vector<parameter_type>& def_val
		, const bool& isEssentialVar
	)
		: GFileParsableI(
		      GFileParsableI::makeVector(optionNameVar)
			, GFileParsableI::makeVector(commentVar)
			, isEssentialVar
		  )
	    , def_val_(def_val)
		, par_() // empty
	{ /* nothing */ }

	/***************************************************************************/
	/**
	 * The destructor
	 */
	virtual ~GFileVectorParsableParameterT()
	{ /* nothing */ }

	/***************************************************************************/
	/**
	 * Executes a stored call-back function
	 */
	virtual void executeCallBackFunction() {
		if(!callBack_) {
		   glogger
		   << "In GVectorParsableParameter::executeCallBackFunction(): Error" << std::endl
         << "Tried to execute call-back function without a stored function" << std::endl
         << GEXCEPTION;
		}

		// Execute the function
		callBack_(par_);
	}

	/***************************************************************************/
	/**
	 * Allows to register a call-back function with this object
	 *
	 * @param callBack The function to be executed
	 */
	void registerCallBackFunction(boost::function<void(std::vector<parameter_type>)> callBack) {
		if(!callBack) {
		   glogger
		   << "In GVectorParsableParameter::registerCallBackFunction(): Error" << std::endl
         << "Tried to register an empty call-back function" << std::endl
         << GEXCEPTION;
		}

		callBack_ = callBack;
	}

protected:
	/***************************************************************************/
	/**
	 * Loads data from a property_tree object
	 *
	 * @param pt The object from which data should be loaded
	 */
	virtual void load(const boost::property_tree::ptree& pt) {
		using namespace boost::property_tree;

		// Make sure the recipient vector is empty
		par_.clear();

		std::string ppath = optionName(0) + ".value";
	    BOOST_FOREACH(ptree::value_type const &v, pt.get_child(ppath.c_str())) {
		   par_.push_back(boost::lexical_cast<parameter_type>(v.second.data()));
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
	virtual void save(boost::property_tree::ptree& pt) const {
		// Needed for the separation of comment strings
		typedef boost::tokenizer<boost::char_separator<char> > tokenizer;
		boost::char_separator<char> semicolon_sep(";");
		std::string comment = this->comment(0);

		// Do some error checking
		if(def_val_.empty()) {
		   glogger
		   << "In GVectorParsableParameter::save(): Error!" << std::endl
         << "You need to provide at least one default value" << std::endl
         << GEXCEPTION;
		}

		// Add the comments
		if(comment != "") { // Only output useful comments
			// Break the comment into individual lines after each semicolon
			tokenizer commentTokenizer(comment, semicolon_sep);
			for(tokenizer::iterator c=commentTokenizer.begin(); c!=commentTokenizer.end(); ++c) {
				pt.add((optionName(0) + ".comment").c_str(), (*c).c_str());
			}
		}

		// Add the value and default items
		typename std::vector<parameter_type>::const_iterator it;
		for(it=def_val_.begin(); it!=def_val_.end(); ++it) {
			pt.add((optionName(0) + ".default.item").c_str(), *it);
			pt.add((optionName(0) + ".value.item").c_str(), *it);
		}
	}

private:
	/***************************************************************************/

	std::vector<parameter_type> def_val_; ///< Holds default values
	std::vector<parameter_type> par_; ///< Holds the parsed parameters

	boost::function<void(std::vector<parameter_type>)> callBack_; ///< Holds the call-back function
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
template <typename parameter_type>
class GFileVectorReferenceParsableParameterT :public GFileParsableI
{
	// We want GParserBuilder to be able to call our load- and save functions
	friend class GParserBuilder;

public:
	/***************************************************************************/
	/**
	 * Initializes the parameters
	 */
	GFileVectorReferenceParsableParameterT(
		std::vector<parameter_type>& stored_reference
		, const std::string& optionNameVar
		, const std::string& commentVar
		, const std::vector<parameter_type>& def_val
		, const bool& isEssentialVar
	)
		: GFileParsableI(
		      GFileParsableI::makeVector(optionNameVar)
			, GFileParsableI::makeVector(commentVar)
			, isEssentialVar
		  )
	    , stored_reference_(stored_reference)
		, def_val_(def_val)
		, par_()
	{ /* nothing */ }

	/***************************************************************************/
	/**
	 * The destructor
	 */
	virtual ~GFileVectorReferenceParsableParameterT()
	{ /* nothing */ }

	/***************************************************************************/
	/**
	 * Assigns the parsed parameters to the reference vector
	 */
	virtual void executeCallBackFunction() {
		stored_reference_ = par_;
	}

protected:
	/***************************************************************************/
	/**
	 * Loads data from a property_tree object
	 *
	 * @param pt The object from which data should be loaded
	 */
	virtual void load(const boost::property_tree::ptree& pt) {
		using namespace boost::property_tree;

		// Make sure the recipient vector is empty
		par_.clear();

		std::string ppath = optionName(0) + ".value";
	    BOOST_FOREACH(ptree::value_type const &v, pt.get_child(ppath.c_str()))
			par_.push_back(boost::lexical_cast<parameter_type>(v.second.data()));
	}

	/***************************************************************************/
	/**
	 * Saves data to a property tree object, including comments. Default
	 * values are taken from the def_val_ vector. Note that there needs
	 * to be at least a single default value in it.
	 *
	 * @param pt The object to which data should be saved
	 */
	virtual void save(boost::property_tree::ptree& pt) const {
		// Needed for the separation of comment strings
		typedef boost::tokenizer<boost::char_separator<char> > tokenizer;
		boost::char_separator<char> semicolon_sep(";");
		std::string comment = this->comment(0);

		// Do some error checking
		if(def_val_.empty()) {
		   glogger
		   << "In GVectorReferenceParsableParameter::save(): Error!" << std::endl
         << "You need to provide at least one default value" << std::endl
         << GEXCEPTION;
		}

		// Add the comments
		if(comment != "") { // Only output useful comments
			// Break the comment into individual lines after each semicolon
			tokenizer commentTokenizer(comment, semicolon_sep);
			for(tokenizer::iterator c=commentTokenizer.begin(); c!=commentTokenizer.end(); ++c) {
				pt.add((optionName(0) + ".comment").c_str(), (*c).c_str());
			}
		}

		// Add the value and default items
		typename std::vector<parameter_type>::const_iterator it;
		for(it=def_val_.begin(); it!=def_val_.end(); ++it) {
			pt.add((optionName(0) + ".default.item").c_str(), *it);
			pt.add((optionName(0) + ".value.item").c_str(), *it);
		}
	}

private:
	/***************************************************************************/

	std::vector<parameter_type>& stored_reference_; ///< Holds a reference to the target vector
	std::vector<parameter_type> def_val_; ///< Holds default values
	std::vector<parameter_type> par_; ///< Holds the parsed parameters
};

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/
/**
 * This class wraps a boost::array of values (obviously of identical type).
 * This class enforces a fixed number of items in the array.
 */
template <typename parameter_type, std::size_t N>
class GFileArrayParsableParameterT
   : public GFileParsableI
{
	// We want GParserBuilder to be able to call our load- and save functions
	friend class GParserBuilder;

public:
	/***************************************************************************/
	/**
	 * Initializes the parameters
	 */
	GFileArrayParsableParameterT(
		const std::string& optionNameVar
		, const std::string& commentVar
		, const boost::array<parameter_type,N>& def_val
		, const bool& isEssentialVar
	)
		: GFileParsableI(
		   GFileParsableI::makeVector(optionNameVar)
			, GFileParsableI::makeVector(commentVar)
			, isEssentialVar
		  )
	    , def_val_(def_val)
		, par_(def_val) // Same size as def_val
	{ /* nothing */ }

	/***************************************************************************/
	/**
	 * The destructor
	 */
	virtual ~GFileArrayParsableParameterT()
	{ /* nothing */ }

	/***************************************************************************/
	/**
	 * Executes a stored call-back function
	 */
	virtual void executeCallBackFunction() {
		if(!callBack_) {
		   glogger
		   << "In GArrayParsableParameter::executeCallBackFunction(): Error" << std::endl
         << "Tried to execute call-back function without a stored function" << std::endl
         << GEXCEPTION;
		}

		// Execute the function
		callBack_(par_);
	}

	/***************************************************************************/
	/**
	 * Allows to register a call-back function with this object
	 *
	 * @param callBack The function to be executed
	 */
	void registerCallBackFunction(boost::function<void(boost::array<parameter_type,N>)> callBack) {
		if(!callBack) {
		   glogger
		   << "In GArrayParsableParameter::registerCallBackFunction(): Error" << std::endl
         << "Tried to register an empty call-back function" << std::endl
         << GEXCEPTION;
		}

		callBack_ = callBack;
	}

protected:
	/***************************************************************************/
	/**
	 * Loads data from a property_tree object
	 *
	 * @param pt The object from which data should be loaded
	 */
	virtual void load(const boost::property_tree::ptree& pt) {
		using namespace boost::property_tree;

		for(std::size_t i=0; i<par_.size(); i++) {
			par_.at(i) = pt.get((optionName(0) + "." + boost::lexical_cast<std::string>(i) + ".value").c_str(), def_val_.at(i));
		}
	}

	/***************************************************************************/
	/**
	 * Saves data to a property tree object, including comments. Default
	 * values are taken from the def_val_ vector.
	 *
	 * @param pt The object to which data should be saved
	 */
	virtual void save(boost::property_tree::ptree& pt) const {
		// Needed for the separation of comment strings
		typedef boost::tokenizer<boost::char_separator<char> > tokenizer;
		boost::char_separator<char> semicolon_sep(";");
		std::string comment = this->comment(0);

		// Do some error checking
		if(def_val_.empty()) {
		   glogger
		   << "In GArrayParsableParameter::save(): Error!" << std::endl
         << "You need to provide at least one default value" << std::endl
         << GEXCEPTION;
		}

		// Add the comments
		if(comment != "") { // Only output useful comments
			// Break the comment into individual lines after each semicolon
			tokenizer commentTokenizer(comment, semicolon_sep);
			for(tokenizer::iterator c=commentTokenizer.begin(); c!=commentTokenizer.end(); ++c) {
				pt.add((optionName(0) + ".comment").c_str(), (*c).c_str());
			}
		}

		// Add the value and default items
		for(std::size_t i=0; i<def_val_.size(); i++) {
			pt.add((optionName(0) + "." + boost::lexical_cast<std::string>(i) + ".default").c_str(), def_val_.at(i));
			pt.add((optionName(0) + "." + boost::lexical_cast<std::string>(i) + ".value").c_str(), par_.at(i));
		}
	}

private:
	/***************************************************************************/

	boost::array<parameter_type,N> def_val_; ///< Holds default values
	boost::array<parameter_type,N> par_; ///< Holds the parsed parameters

	boost::function<void(boost::array<parameter_type,N>)> callBack_; ///< Holds the call-back function
};

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/
/**
 * This class wraps a reference to a boost::array of values (obviously of
 * identical type). This class enforces a fixed number of items in the array.
 */
template <typename parameter_type, std::size_t N>
class GFileArrayReferenceParsableParameterT
   : public GFileParsableI
{
	// We want GParserBuilder to be able to call our load- and save functions
	friend class GParserBuilder;

public:
	/***************************************************************************/
	/**
	 * Initializes the parameters
	 */
	GFileArrayReferenceParsableParameterT(
		boost::array<parameter_type,N>& stored_reference
		, const std::string& optionNameVar
		, const std::string& commentVar
		, const boost::array<parameter_type,N>& def_val
		, const bool& isEssentialVar
	)
		: GFileParsableI(
		   GFileParsableI::makeVector(optionNameVar)
			, GFileParsableI::makeVector(commentVar)
			, isEssentialVar
		  )
	    , stored_reference_(stored_reference)
		, def_val_(def_val)
		, par_(def_val)
	{ /* nothing */ }

	/***************************************************************************/
	/**
	 * The destructor
	 */
	virtual ~GFileArrayReferenceParsableParameterT()
	{ /* nothing */ }

	/***************************************************************************/
	/**
	 * Assigns the parsed parameters to the reference vector
	 */
	virtual void executeCallBackFunction() {
		stored_reference_ = par_;
	}

protected:
	/***************************************************************************/
	/**
	 * Loads data from a property_tree object
	 *
	 * @param pt The object from which data should be loaded
	 */
	virtual void load(const boost::property_tree::ptree& pt) {
		using namespace boost::property_tree;

		for(std::size_t i=0; i<par_.size(); i++) {
			par_.at(i) = pt.get((optionName(0) + "." + boost::lexical_cast<std::string>(i) + ".value").c_str(), def_val_.at(i));
		}
	}

	/***************************************************************************/
	/**
	 * Saves data to a property tree object, including comments. Default
	 * values are taken from the def_val_ vector.
	 *
	 * @param pt The object to which data should be saved
	 */
	virtual void save(boost::property_tree::ptree& pt) const {
		// Needed for the separation of comment strings
		typedef boost::tokenizer<boost::char_separator<char> > tokenizer;
		boost::char_separator<char> semicolon_sep(";");
		std::string comment = this->comment(0);

		// Do some error checking
		if(def_val_.empty()) {
		   glogger
		   << "In GArrayReferenceParsableParameter::save(): Error!" << std::endl
         << "You need to provide at least one default value" << std::endl
         << GEXCEPTION;
		}

		// Add the comments
		if(comment != "") { // Only output useful comments
			// Break the comment into individual lines after each semicolon
			tokenizer commentTokenizer(comment, semicolon_sep);
			for(tokenizer::iterator c=commentTokenizer.begin(); c!=commentTokenizer.end(); ++c) {
				pt.add((optionName(0) + ".comment").c_str(), (*c).c_str());
			}
		}

		// Add the value and default items
		for(std::size_t i=0; i<def_val_.size(); i++) {
			pt.add((optionName(0) + "." + boost::lexical_cast<std::string>(i) + ".default").c_str(), def_val_.at(i));
			pt.add((optionName(0) + "." + boost::lexical_cast<std::string>(i) + ".value").c_str(), par_.at(i));
		}
	}

private:
	/***************************************************************************/

	boost::array<parameter_type,N>& stored_reference_; ///< Holds a reference to the target vector
	boost::array<parameter_type,N> def_val_; ///< Holds default values
	boost::array<parameter_type,N> par_; ///< Holds the parsed parameters
};

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/
/**
 * This class wraps a reference to individual parameters. Instead of
 * executing a stored call-back function, executeCallBackFunction will assign
 * the parsed value to the reference.
 */
template <typename parameter_type>
class GFileReferenceParsableParameterT
   : public GFileParsableI
{
	// We want GParserBuilder to be able to call our load- and save functions
	friend class GParserBuilder;

public:
	/***************************************************************************/
	/**
	 * A constructor that initializes the internal reference
	 *
	 * @param storedReference A reference to a variable in which parsed values should be stored
	 * @param defVal The default value of this variable
	 */
	GFileReferenceParsableParameterT(
		parameter_type& storedReference
		, const std::string& optionNameVar
		, const std::string& commentVar
		, const bool& isEssentialVar
		, parameter_type defVal
	)
		: GFileParsableI(
		   GFileParsableI::makeVector(optionNameVar)
			, GFileParsableI::makeVector(commentVar)
			, isEssentialVar
		  )
		, storedReference_(storedReference)
		, par_(defVal)
		, def_val_(defVal)
	{ /* nothing */ }

	/***************************************************************************/
	/**
	 * Assigns the stored parameter to the reference
	 */
	virtual void executeCallBackFunction() {
		storedReference_ = par_;
	}

protected:
	/***************************************************************************/
	/**
	 * Loads data from a property_tree object
	 *
	 * @param pt The object from which data should be loaded
	 */
	virtual void load(const boost::property_tree::ptree& pt) {
		par_ = pt.get((optionName(0) + ".value").c_str(), def_val_);
	}

	/***************************************************************************/
	/**
	 * Saves data to a property tree object, including comments.
	 *
	 * @param pt The object to which data should be saved
	 */
	virtual void save(boost::property_tree::ptree& pt) const {
		// Needed for the separation of comment strings
		typedef boost::tokenizer<boost::char_separator<char> > tokenizer;
		boost::char_separator<char> semicolon_sep(";");
		std::string comment = this->comment(0);

		if(comment != "") { // Only output useful comments
			// Break the comment into individual lines after each semicolon
			tokenizer commentTokenizer(comment, semicolon_sep);
			for(tokenizer::iterator c=commentTokenizer.begin(); c!=commentTokenizer.end(); ++c) {
				pt.add((optionName(0) + ".comment").c_str(), (*c).c_str());
			}
		}

		pt.put((optionName(0) + ".default").c_str(), def_val_);
		pt.put((optionName(0) + ".value").c_str(), par_);
	}

private:
	/***************************************************************************/
	GFileReferenceParsableParameterT(); ///< The default constructor. Intentionally private and undefined

	parameter_type& storedReference_; ///< Holds the reference to which the parsed value will be assigned
	parameter_type par_; ///< Holds the individual parameter
	parameter_type def_val_; ///< Holds the default value if par_
};

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/
/**
 * This class wraps a reference to individual command line parameters.
 */
template <typename parameter_type>
class GCLReferenceParsableParameterT
   : public GCLParsableI
{
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
      parameter_type& storedReference
      , std::string optionNameVar
      , std::string commentVar
      , parameter_type defVal
      , bool implicitAllowed
      , parameter_type implVal
   )
      : GCLParsableI(
         GCLParsableI::makeVector(optionNameVar)
         , GCLParsableI::makeVector(commentVar)
        )
      , storedReference_(storedReference)
      , def_val_(defVal)
      , implicitAllowed_(implicitAllowed)
      , impl_val_(implVal)
   { /* nothing */ }

protected:
   /***************************************************************************/
   /**
    * Saves data to a property tree object
    */
   virtual void save(boost::program_options::options_description& desc) const {
      namespace po = boost::program_options;
      if(GCL_IMPLICIT_ALLOWED == implicitAllowed_) {
         desc.add_options() (
            (this->optionName()).c_str()
            , po::value<parameter_type>(&storedReference_)->implicit_value(impl_val_)->default_value(def_val_)
            , (this->comment()).c_str()
         );
      } else { // GCL_IMPLICIT_NOT_ALLOWED
         desc.add_options() (
            (this->optionName()).c_str()
            , po::value<parameter_type>(&storedReference_)->default_value(def_val_)
            , (this->comment()).c_str()
         );
      }
   }

   /***************************************************************************/
   /**
    * Returns the content of this object as a std::string
    */
   virtual std::string content() const {
      std::ostringstream result;
      result << this->optionName() << " :\t" << storedReference_ << "\t" << ((storedReference_!=def_val_)?"default: " + boost::lexical_cast<std::string>(def_val_):std::string(""));
      return result.str();
   }

private:
   /***************************************************************************/
   GCLReferenceParsableParameterT(); ///< The default constructor. Intentionally private and undefined

   parameter_type& storedReference_; ///< Holds the reference to which the parsed value will be assigned
   parameter_type def_val_; ///< Holds the default value
   bool implicitAllowed_; ///< Indicates, whether implicit values (e.g. --server=true vs. --server) are allowed
   parameter_type impl_val_; ///< Holds an implicit value used if only the option name is given
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
class GParserBuilder :boost::noncopyable
{
public:
	/** @brief The default constructor */
	GParserBuilder();

	/** @brief The destructor */
	virtual ~GParserBuilder();

	/** @brief Tries to parse a given configuration file for a set of options */
	bool parseConfigFile(const std::string&);
	/** @brief Writes out a configuration file */
	void writeConfigFile(const std::string& = "", const std::string& = "", bool = true) const;
	/** @brief Provides information on the number of file configuration options stored in this class */
	std::size_t numberOfFileOptions() const;

	/** @brief Parses the commandline for options */
	bool parseCommandLine(int, char **,bool=false);
   /** @brief Provides information on the number of command line configuration options stored in this class */
   std::size_t numberOfCLOptions() const;

	/***************************************************************************/
	/**
	 * Adds a single parameter of configurable type to the collection. When
	 * this parameter has been read using parseConfigFile, a call-back
	 * function is executed.
	 */
	template <typename parameter_type>
	void registerFileParameter(
		std::string optionName
		, parameter_type def_val
		, boost::function<void(parameter_type)> callBack
		, bool isEssential = true
		, std::string comment = ""
	) {
		boost::shared_ptr<GFileSingleParsableParameterT<parameter_type> >
			singleParm_ptr(new GFileSingleParsableParameterT<parameter_type>(
					optionName
					, comment
					, isEssential
					, def_val
				)
			);

		singleParm_ptr->registerCallBackFunction(callBack);

		// Store for later usage
		file_parameter_proxies_.push_back(singleParm_ptr);
	}

	/***************************************************************************/
	/**
	 * Adds two parameters of configurable types to the collection. When
	 * these parameters have been read using parseConfigFile, a call-back
	 * function will be executed.
	 */
	template <typename par_type1, typename par_type2>
	void registerFileParameter(
		std::string optionName1
		, std::string optionName2
		, par_type1 def_val1
		, par_type2 def_val2
		, boost::function<void(par_type1, par_type2)> callBack
		, std::string combined_label
		, bool isEssential = true
		, std::string comment1 = ""
		, std::string comment2 = ""
	) {
		boost::shared_ptr<GFileCombinedParsableParameterT<par_type1, par_type2> >
			combParm_ptr(new GFileCombinedParsableParameterT<par_type1, par_type2>(
					optionName1
					, comment1
					, def_val1
					, optionName2
					, comment2
					, def_val2
					, isEssential
					, combined_label
				)
			);

		combParm_ptr->registerCallBackFunction(callBack);

		// Store one and two for later usage
		file_parameter_proxies_.push_back(combParm_ptr);
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
	template <typename parameter_type>
	void registerFileParameter(
		std::string optionName
		, parameter_type& parameter
		, parameter_type def_val
		, bool isEssential = true
		, std::string comment = ""
	) {
		boost::shared_ptr<GFileReferenceParsableParameterT<parameter_type> >
			refParm_ptr(new GFileReferenceParsableParameterT<parameter_type>(
					parameter
					, optionName
					, comment
					, isEssential
					, def_val
				)
			);

		// Store for later usage
		file_parameter_proxies_.push_back(refParm_ptr);
	}

	/***************************************************************************/
	/**
	 * Adds a vector of configurable type to the collection, using a
	 * call-back function
	 */
	template <typename parameter_type>
	void registerFileParameter(
		const std::string& optionName
		, const std::vector<parameter_type>& def_val
		, boost::function<void(std::vector<parameter_type>)> callBack
		, const bool& isEssential = true
		, const std::string& comment = ""
	) {
		boost::shared_ptr<GFileVectorParsableParameterT<parameter_type> >
			vecParm_ptr(new GFileVectorParsableParameterT<parameter_type> (
					optionName
					, comment
					, def_val
					, isEssential
				)
			);

		vecParm_ptr->registerCallBackFunction(callBack);

		// Store for later usage
		file_parameter_proxies_.push_back(vecParm_ptr);
	}

	/***************************************************************************/
	/**
	 * Adds a reference to a vector of configurable type to the collection
	 */
	template <typename parameter_type>
	void registerFileParameter(
		const std::string& optionName
		, std::vector<parameter_type>& stored_reference
		, const std::vector<parameter_type>& def_val
		, const bool& isEssential = true
		, const std::string& comment = ""
	) {
		boost::shared_ptr<GFileVectorReferenceParsableParameterT<parameter_type> >
			vecRefParm_ptr(new GFileVectorReferenceParsableParameterT<parameter_type> (
					stored_reference
					, optionName
					, comment
					, def_val
					, isEssential
				)
			);

		// Store for later usage
		file_parameter_proxies_.push_back(vecRefParm_ptr);
	}

	/***************************************************************************/
	/**
	 * Adds an array of configurable type but fixed size to the collection.
	 * This allows to make sure that a given amount of configuration options
	 * must be available.
	 */
	template <typename parameter_type, std::size_t N>
	void registerFileParameter(
		const std::string& optionName
		, const boost::array<parameter_type,N>& def_val
		, boost::function<void(boost::array<parameter_type,N>)> callBack
		, const bool& isEssential = true
		, const std::string& comment = ""
	) {
		boost::shared_ptr<GFileArrayParsableParameterT<parameter_type,N> >
			arrayParm_ptr(new GFileArrayParsableParameterT<parameter_type,N> (
					optionName
					, comment
					, def_val
					, isEssential
				)
			);

		// Register the call back function
		arrayParm_ptr->registerCallBackFunction(callBack);

		// Store for later usage
		file_parameter_proxies_.push_back(arrayParm_ptr);
	}

	/***************************************************************************/
	/**
	 * Adds a reference to an array of configurable type but fixed size
	 * to the file parameter collection
	 */
	template <typename parameter_type, std::size_t N>
	void registerFileParameter(
		const std::string& optionName
		, boost::array<parameter_type,N>& stored_reference
		, const boost::array<parameter_type,N>& def_val
		, const bool& isEssential = true
		, const std::string& comment = ""
	) {
		boost::shared_ptr<GFileArrayReferenceParsableParameterT<parameter_type,N> >
			arrayRefParm_ptr(new GFileArrayReferenceParsableParameterT<parameter_type,N> (
					stored_reference
					, optionName
					, comment
					, def_val
					, isEssential
				)
			);

		// Store for later usage
		file_parameter_proxies_.push_back(arrayRefParm_ptr);
	}

	/***************************************************************************/
   /**
    * Adds a reference to a configurable type to the command line parameters.
    *
    * @param optionName The name of the option
    * @param parameter The parameter into which the value will be written
    * @param def_val A default value to be used if the corresponding parameter was not found in the configuration file
    * @param comment A comment to be associated with the parameter in configuration files
    */
   template <typename parameter_type>
   void registerCLParameter(
      std::string optionName
      , parameter_type& parameter
      , parameter_type def_val
      , std::string comment = ""
      , bool implicitAllowed=GCL_IMPLICIT_NOT_ALLOWED
      , parameter_type impl_val = Gem::Common::GDefaultValueT<parameter_type>()
   ) {
      boost::shared_ptr<GCLReferenceParsableParameterT<parameter_type> >
         refParm_ptr(new GCLReferenceParsableParameterT<parameter_type>(
               parameter
               , optionName
               , comment
               , def_val
               , implicitAllowed
               , impl_val
            )
         );

      // Store for later usage
      cl_parameter_proxies_.push_back(refParm_ptr);
   }


private:
	/***************************************************************************/

	std::vector<boost::shared_ptr<GFileParsableI> > file_parameter_proxies_; ///< Holds file parameter proxies
	std::vector<boost::shared_ptr<GCLParsableI> >   cl_parameter_proxies_;   ///< Holds command line parameter proxies

	std::string configFileBaseName_;

   static boost::mutex configFileParser_mutex_; ///< Synchronization of access to configuration files (may only happen serially)
};

/******************************************************************************/

} /* namespace Common */
} /* namespace Gem */

#endif /* GPARSERBUILDER_HPP_ */
