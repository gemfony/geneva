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
#include <boost/thread/thread.hpp>
#include <boost/utility.hpp>
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
 * This class specifies the interface of parseable parameters, to
 * which a callback function has been assigned
 */
struct GParsableI
{
public:
	virtual ~GParsableI() { /* nothing */ }

	/** @brief Executes a stored call-back function */
	virtual void executeCallBackFunction() = 0;
};

/************************************************************************/
/**
 * This class wraps individual parsable parameters, to which a callback
 * function has been assigned.
 */
template <typename parameter_type>
struct GSingleParsableParameter
	: public GParsableI
	, boost::noncopyable
{
public:
	/********************************************************************/
	/**
	 * The default constructor
	 */
	GSingleParsableParameter()
		: par_(parameter_type(0))
	{ /* nothing */ }

	/********************************************************************/
	/**
	 * Initializes the parameter
	 */
	explicit GSingleParsableParameter(const parameter_type& par)
		: par_(par)
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

private:
	/********************************************************************/
	parameter_type par_; ///< Holds the individual parameter

	boost::function<void(parameter_type)> callBack_; ///< Holds the call-back function
};

/************************************************************************/
/**
 * This class wraps combined parsable parameters, to which a callback
 * function has been assigned.
 */
template <typename par_type1, typename par_type2>
struct GCombinedParsableParameter
	: public GParsableI
	, boost::noncopyable
{
public:
	/********************************************************************/
	/**
	 * The default constructor
	 */
	GCombinedParsableParameter()
		: par1_(par_type1(0))
		, par2_(par_type2(0))
	{ /* nothing */ }

	/********************************************************************/
	/**
	 * Initializes the parameters
	 */
	GCombinedParsableParameter(const par_type1& par1, const par_type2& par2)
		: par1_(par1)
		, par2_(par2)
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

private:
	/********************************************************************/
	par_type1 par1_; ///< Holds the first parameter
	par_type2 par2_; ///< Holds the second parameter

	boost::function<void(par_type1, par_type2)> callBack_; ///< Holds the call-back function
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

		config_.add_options()
				(optionName.c_str(), po::value<parameter_type>(&parameter)->default_value(def_val), comment.c_str())
		;

		variables_.push_back(optionName);
		defaultValues_.push_back(boost::lexical_cast<std::string>(def_val));
		isEssential_.push_back(isEssential);
		comments_.push_back(comment);
	}

	/********************************************************************/
	/**
	 * Adds a single parameter of configurable types to the collection. When
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
			singleParm_ptr(new GSingleParsableParameter<parameter_type>(def_val));

		singleParm_ptr->registerCallBackFunction(callBack);

		// Add the variable to our configuration list
		config_.add_options()
				(optionName.c_str(), po::value<parameter_type>(singleParm_ptr->getParameter())->default_value(def_val), comment.c_str())
		;

		// Store one and two for later usage
		many_.push_back(singleParm_ptr);
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
			combParm_ptr(new GCombinedParsableParameter<par_type1, par_type2>(def_val1, def_val2));

		combParm_ptr->registerCallBackFunction(callBack);

		// Add the variables to our configuration list
		config_.add_options()
				(optionName1.c_str(), po::value<par_type1>(combParm_ptr->getParameter1())->default_value(def_val1), comment1.c_str())
				(optionName2.c_str(), po::value<par_type2>(combParm_ptr->getParameter2())->default_value(def_val2), comment2.c_str())
		;

		// Store one and two for later usage
		many_.push_back(combParm_ptr);
	}

private:
	/********************************************************************/

	std::string configurationFile_; ///< The name and path of the configuration file to be parsed
	boost::program_options::options_description config_; ///< Will hold a description of the program options
	std::vector<std::string> variables_; ///< The names of the variables to be parsed
	std::vector<std::string> defaultValues_; ///< The default values associated with each variable
	std::vector<bool> isEssential_; ///< Indicates whether the corresponding variable is essential or secondary
	std::vector<std::string> comments_; ///< Comments to be associated with each variable

	std::vector<boost::shared_ptr<GParsableI> > many_; ///< Ensures the survival of the parameters
};

} /* namespace Common */
} /* namespace Gem */

#endif /* GPARSERBUILDER_HPP_ */
