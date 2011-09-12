/**
 * @file GParserBuilder.hpp
 */

/*
 * Copyright (C) Gemfony scientific UG (haftungsbeschraenkt)
 *
 * See the AUTHORS file in the top-level directory for a list of authors.
 *
 * Contact: contact [at] gemfony (dot) com
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
 * http://www.gemfony.com .
 */

// Standard headers go here
#include <fstream>
#include <sstream>
#include <iostream>
#include <vector>
#include <string>

// Includes check for correct Boost version(s)
#include "GGlobalDefines.hpp"

// Boost headers go here
#include <boost/filesystem.hpp>
#include <boost/thread/thread.hpp>
#include <boost/utility.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <boost/program_options.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/date_time.hpp>
#include <boost/tokenizer.hpp>
#include <boost/function.hpp>
#include <boost/bind.hpp>

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

namespace Gem {
namespace Common {

/************************************************************************/
/**
 * This class specifies the interface of parsable parameters, to
 * which a call-back function has been assigned. It also stores some
 * information common to all parameter types.
 */
class GParsableI :boost::noncopyable
{
public:
	/** @brief A constructor for individual items */
	GParsableI(
		const std::string&
		, const std::string&
		, const std::string&
		, const bool&
	);

	/** @brief A constructor for vectors */
	GParsableI(
		const std::vector<std::string>&
		, const std::vector<std::string>&
		, const std::vector<std::string>&
		, const bool&
	);

	/** @brief The destructor */
	virtual ~GParsableI();
	/** @brief Executes a stored call-back function */
	virtual void executeCallBackFunction() = 0;

	/** @brief Retrieves the option name at a given position */
	std::string optionName(std::size_t = 0) const;
	/** @brief Retrieves a string-representation of the default value at a given position */
	std::string defaultValue(std::size_t = 0) const;
	/** @brief Retrieves the comment that was assigned to this variable at a given position */
	std::string comment(std::size_t = 0) const;
	/** @brief Checks whether this is an essential variable at a given position */
	bool isEssential() const;

	/** @brief Makes the parameter object output its data */
	virtual std::string configData() const = 0;

	/**
	 * Create a std::vector<T> from a single element
	 */
	template <typename T>
	static std::vector<T> makeVector(const T& item) {
		std::vector<T> result;
		result.push_back(item);
		return result;
	}

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
	/** @brief The default constructor. Intentionally private and undefined */
	GParsableI();

	std::vector<std::string> optionName_; ///< The name of this parameter
	std::vector<std::string> defaultValueStr_; ///< A string representation of the default value
	std::vector<std::string> comment_; ///< A comment assigned to this parameter
	bool isEssential_; ///< Indicates whether this is an essential variable
};

/************************************************************************/
/**
 * This class wraps individual parsable parameters, to which a callback
 * function has been assigned.
 */
template <typename parameter_type>
struct GSingleParsableParameter :public GParsableI
{
public:
	/********************************************************************/
	/**
	 * Initializes the parameter and sets values in the parent class
	 */
	explicit GSingleParsableParameter(
		const std::string& optionNameVar
		, const std::string& commentVar
		, const bool& isEssentialVar
		, const parameter_type& defVal
	)
		: GParsableI(
			optionNameVar
			, boost::lexical_cast<std::string>(defVal)
			, commentVar
			, isEssentialVar
		  )
		, par_(defVal)
	{ /* nothing */ }

	/********************************************************************/
	/**
	 * The destructor
	 */
	virtual ~GSingleParsableParameter()
	{ /* nothing */ }

	/********************************************************************/
	/**
	 * Executes a stored call-back function
	 */
	virtual void executeCallBackFunction() {
		if(!callBack_) {
			raiseException(
				"In GSingleParsableParameter::executeCallBackFunction(): Error" << std::endl
				<< "Tried to execute call-back function without a stored function" << std::endl
			);
		}

		// Execute the function
		callBack_(par_);
	}

	/********************************************************************/
	/**
	 * Allows to register a call-back function with this object
	 *
	 * @param callBack The function to be executed
	 */
	void registerCallBackFunction(boost::function<void(parameter_type)> callBack) {
		if(!callBack) {
			raiseException(
				"In GSingleParsableParameter::registerCallBackFunction(): Error" << std::endl
				<< "Tried to register an empty call-back function" << std::endl
			);
		}

		callBack_ = callBack;
	}

	/********************************************************************/
	/**
	 * Gives access to the parameter
	 *
	 * @return A reference to the stored parameter
	 */
	parameter_type *getParameter() {
		return &par_;
	}

	/********************************************************************/
	/**
	 * Makes the parameter object output its data
	 *
	 * @return A string with the data
	 */
	virtual std::string configData() const {
		std::ostringstream result;
		// Needed for the separation of comment strings
		typedef boost::tokenizer<boost::char_separator<char> > tokenizer;
		boost::char_separator<char> semicolon_sep(";");
		std::string comment = this->comment(0);

		if(comment != "") { // Only output useful comments
			// Break the comment into individual lines after each semicolon
			tokenizer commentTokenizer(comment, semicolon_sep);
			for(tokenizer::iterator c=commentTokenizer.begin(); c!=commentTokenizer.end(); ++c) {
				result << "# " << *c << std::endl;
			}
		}
		result
			<< "# default value: " << this->defaultValue(0) << std::endl
			<< this->optionName(0) << " = " << this->defaultValue(0) << std::endl
			<< std::endl;

		return result.str();
	}

private:
	/********************************************************************/
	GSingleParsableParameter(); ///< The default constructor. Intentionally private and undefined
	parameter_type par_; ///< Holds the individual parameter

	boost::function<void(parameter_type)> callBack_; ///< Holds the call-back function
};

/************************************************************************/
/**
 * This class wraps combined parsable parameters, to which a callback
 * function has been assigned.
 */
template <typename par_type1, typename par_type2>
struct GCombinedParsableParameter :public GParsableI
{
public:
	/********************************************************************/
	/**
	 * Initializes the parameters
	 */
	GCombinedParsableParameter(
		const std::string& optionNameVar1
		, const std::string& commentVar1
		, const par_type1& defVal1
		, const std::string& optionNameVar2
		, const std::string& commentVar2
		, const par_type2& defVal2
		, const bool& isEssentialVar
	)
		: GParsableI(
			GParsableI::makeVector(optionNameVar1, optionNameVar2)
			, GParsableI::makeVector(boost::lexical_cast<std::string>(defVal1), boost::lexical_cast<std::string>(defVal2))
			, GParsableI::makeVector(commentVar1, commentVar2)
			, isEssentialVar
		  )
		, par1_(defVal1)
		, par2_(defVal2)
	{ /* nothing */ }

	/********************************************************************/
	/**
	 * The destructor
	 */
	virtual ~GCombinedParsableParameter()
	{ /* nothing */ }

	/********************************************************************/
	/**
	 * Executes a stored call-back function
	 */
	virtual void executeCallBackFunction() {
		if(!callBack_) {
			raiseException(
				"In GCombinedParsableParameter::executeCallBackFunction(): Error" << std::endl
				<< "Tried to execute call-back function without a stored function" << std::endl
			);
		}

		// Execute the function
		callBack_(par1_, par2_);
	}

	/********************************************************************/
	/**
	 * Allows to register a call-back function with this object
	 *
	 * @param callBack The function to be executed
	 */
	void registerCallBackFunction(boost::function<void(par_type1, par_type2)> callBack) {
		if(!callBack) {
			raiseException(
				"In GCombinedParsableParameter::registerCallBackFunction(): Error" << std::endl
				<< "Tried to register an empty call-back function" << std::endl
			);
		}

		callBack_ = callBack;
	}

	/********************************************************************/
	/**
	 * Gives access to the first parameter
	 *
	 * @return A reference to the first stored parameter
	 */
	par_type1* getParameter1() {
		return &par1_;
	}

	/********************************************************************/
	/**
	 * Gives access to the second parameter
	 *
	 * @return A reference to the second stored parameter
	 */
	par_type2* getParameter2() {
		return &par2_;
	}

	/********************************************************************/
	/**
	 * Makes the parameter object output its data
	 *
	 * @return A string with the data
	 */
	virtual std::string configData() const {
		std::ostringstream result;
		// Needed for the separation of comment strings
		typedef boost::tokenizer<boost::char_separator<char> > tokenizer;
		boost::char_separator<char> semicolon_sep(";");
		std::string comment0 = this->comment(0);
		std::string comment1 = this->comment(1);

		if(comment0 != "") { // Only output useful comments
			// Break the comment into individual lines after each semicolon
			tokenizer commentTokenizer(comment0, semicolon_sep);
			for(tokenizer::iterator c=commentTokenizer.begin(); c!=commentTokenizer.end(); ++c) {
				result << "# " << *c << std::endl;
			}
		}
		result
			<< "# default value: " << this->defaultValue(0) << std::endl
			<< this->optionName(0) << " = " << this->defaultValue(0) << std::endl
			<< std::endl;

		if(comment1 != "") { // Only output useful comments
			// Break the comment into individual lines after each semicolon
			tokenizer commentTokenizer(comment1, semicolon_sep);
			for(tokenizer::iterator c=commentTokenizer.begin(); c!=commentTokenizer.end(); ++c) {
				result << "# " << *c << std::endl;
			}
		}
		result
			<< "# default value: " << this->defaultValue(1) << std::endl
			<< this->optionName(1) << " = " << this->defaultValue(1) << std::endl
			<< std::endl;

		return result.str();
	}

private:
	/********************************************************************/
	GCombinedParsableParameter(); ///< The default constructor. Intentionally private and undefined

	par_type1 par1_; ///< Holds the first parameter
	par_type2 par2_; ///< Holds the second parameter

	boost::function<void(par_type1, par_type2)> callBack_; ///< Holds the call-back function
};

/************************************************************************/
/**
 * This class wraps a reference to individual parameters. Instead of
 * executing a stored call-back function, executeCallBackFunction will assign
 * the parsed value to the reference.
 */
template <typename parameter_type>
class GReferenceParsableParameter :public GParsableI
{
public:
	/********************************************************************/
	/**
	 * A constructor that initializes the internal reference
	 *
	 * @param storedReference A reference to a variable in which parsed values should be stored
	 * @param defVal The default value of this variable
	 */
	GReferenceParsableParameter(
		parameter_type& storedReference
		, const std::string& optionNameVar
		, const std::string& commentVar
		, const bool& isEssentialVar
		, parameter_type defVal
	)
		: GParsableI(
			GParsableI::makeVector(optionNameVar)
			, GParsableI::makeVector(boost::lexical_cast<std::string>(defVal))
			, GParsableI::makeVector(commentVar)
			, isEssentialVar
		  )
		, storedReference_(storedReference)
		, par_(defVal)
	{ /* nothing */ }

	/********************************************************************/
	/**
	 * Assigns the stored parameter to the reference
	 */
	virtual void executeCallBackFunction() {
		storedReference_ = par_;
	}

	/********************************************************************/
	/**
	 * Gives access to the internal parameter-storage (not the reference)
	 *
	 * @return A reference to the stored parameter
	 */
	parameter_type *getParameter() {
		return &par_;
	}

	/********************************************************************/
	/**
	 * Makes the parameter object output its data
	 *
	 * @return A string with the data
	 */
	virtual std::string configData() const {
		std::ostringstream result;
		// Needed for the separation of comment strings
		typedef boost::tokenizer<boost::char_separator<char> > tokenizer;
		boost::char_separator<char> semicolon_sep(";");
		std::string comment = this->comment(0);

		if(comment != "") { // Only output useful comments
			// Break the comment into individual lines after each semicolon
			tokenizer commentTokenizer(comment, semicolon_sep);
			for(tokenizer::iterator c=commentTokenizer.begin(); c!=commentTokenizer.end(); ++c) {
				result << "# " << *c << std::endl;
			}
		}
		result
			<< "# default value: " << this->defaultValue(0) << std::endl
			<< this->optionName(0) << " = " << this->defaultValue(0) << std::endl
			<< std::endl;

		return result.str();
	}

private:
	/********************************************************************/
	GReferenceParsableParameter(); ///< The default constructor. Intentionally private and undefined

	parameter_type& storedReference_; ///< Holds the reference to which the parsed value will be assigned
	parameter_type par_; ///< Holds the individual parameter
};

/************************************************************************/
/**
 * This class implements a "parser builder", that allows to easily specify
 * the options that the parser should search for in a configuration file.
 * Results of the parsing process will be written directly into the supplied
 * variables. If not found, a default value will be used. Note that this
 * class assumes that the parameter_type can be streamed using operator<< or
 * operator>>
 */
class GParserBuilder
	:boost::noncopyable
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

	/** @brief Provides information on the number of configuration options stored in this class */
	std::size_t numberOfOptions() const;

	/********************************************************************/
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
		namespace po = boost::program_options;

		boost::shared_ptr<GSingleParsableParameter<parameter_type> >
			singleParm_ptr(new GSingleParsableParameter<parameter_type>(
					optionName
					, comment
					, isEssential
					, def_val
				)
			);

		singleParm_ptr->registerCallBackFunction(callBack);

		// Add the variable to our configuration list
		config_.add_options()
				(optionName.c_str(), po::value<parameter_type>(
						singleParm_ptr->getParameter())->default_value(def_val), comment.c_str())
		;

		// Store for later usage
		parameter_proxies_.push_back(singleParm_ptr);
	}

	/********************************************************************/
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
		, bool isEssential = true
		, std::string comment1 = ""
		, std::string comment2 = ""
	) {
		namespace po = boost::program_options;

		boost::shared_ptr<GCombinedParsableParameter<par_type1, par_type2> >
			combParm_ptr(new GCombinedParsableParameter<par_type1, par_type2>(
					optionName1
					, comment1
					, def_val1
					, optionName2
					, comment2
					, def_val2
					, isEssential
				)
			);

		combParm_ptr->registerCallBackFunction(callBack);

		// Add the variables to our configuration list
		config_.add_options()
				(optionName1.c_str(), po::value<par_type1>(combParm_ptr->getParameter1())->default_value(def_val1), comment1.c_str())
				(optionName2.c_str(), po::value<par_type2>(combParm_ptr->getParameter2())->default_value(def_val2), comment2.c_str())
		;

		// Store one and two for later usage
		parameter_proxies_.push_back(combParm_ptr);
	}

	/********************************************************************/
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
	    namespace po = boost::program_options;

		boost::shared_ptr<GReferenceParsableParameter<parameter_type> >
			refParm_ptr(new GReferenceParsableParameter<parameter_type>(
					parameter
					, optionName
					, comment
					, isEssential
					, def_val
				)
			);

		config_.add_options()
				(optionName.c_str(), po::value<parameter_type>(&parameter)->default_value(def_val), comment.c_str())
		;

		// Store for later usage
		parameter_proxies_.push_back(refParm_ptr);
	}

private:
	/********************************************************************/

	boost::program_options::options_description config_; ///< Will hold a description of the program options
	std::vector<boost::shared_ptr<GParsableI> > parameter_proxies_; ///< Holds parameter proxies
};

} /* namespace Common */
} /* namespace Gem */

#endif /* GPARSERBUILDER_HPP_ */
