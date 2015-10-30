/**
 * @file GConfigFileCreation.cpp
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

// Standard header files go here
#include <iostream>
#include <string>

// Boost header files go here
#include <boost/lexical_cast.hpp>
#include <boost/array.hpp>

// Geneva header files go here
#include "common/GParserBuilder.hpp"

using namespace Gem::Common;

// Call-back function for a single parameter
int someGlobalInt;
const int SOMEGLOBALINTDEFAULT = 1;
void setGlobalInt(int globalInt) {
	someGlobalInt = globalInt;
}

// The same can be done with a function object
class twoVariableFunctionObject {
public:
	twoVariableFunctionObject():i_(0), d_(0.) {}
	void operator()(int i, double d) {
		i_ = i;
		d_ = d;
	}

	int getInt() const {return i_;}
	double getDouble() const {return d_;}

private:
	int i_;
	double d_;
};

// Call back function for a std::vector, plus global std::vector<double>
std::vector<double> someGlobalDoubleVec;
void setGlobalDoubleVec(std::vector<double> par) {
	someGlobalDoubleVec = par;
}

// Call back function for a boost::array object, plus global array object as target
const std::size_t ARRAYSIZE = 5;
boost::array<int,ARRAYSIZE> someGlobalBoostArray;
void setGlobalBoostArray(boost::array<int,ARRAYSIZE> par) {
	someGlobalBoostArray = par;
}

/************************************************************************
 * This example illustrates the usage options of the GParserBuilder class
 */
int main(int argc, char **argv) {
	int creationSwitcher;
	bool useOperator;
	std::string fileName;

	// Create the parser builder
	Gem::Common::GParserBuilder gpb;

	//----------------------------------------------------------------
	// Register some command line options
	gpb.registerCLParameter<int>(
		"creationSwitcher,c"
		, creationSwitcher
		, int(0)
		, "Allows to switch between configuration file creation (0) and file parsing (1)"
	);

	// Information may be streamed!
	gpb.registerCLParameter<bool>(
		"useOperator,o"
		, useOperator
		, false
	) << "Allows to enforce usage of gpb.registerFileParameter() << comment";

	gpb.registerCLParameter<std::string>(
		"fileName,f"
		, fileName
		, "./config/configFile.json"
		, "The name of the file information should be written to or read from"
	);

	// Parse the command line and leave if the help flag was given
	if(Gem::Common::GCL_HELP_REQUESTED == gpb.parseCommandLine(argc, argv, true /*verbose*/)) {
		return 0;
	}

	//----------------------------------------------------------------
	// Example 1: Registering a call-back function (which in this
	// case sets a globally defined integer variable)
	if(useOperator) {
		gpb.registerFileParameter<int>(
			"iOption2"
			, SOMEGLOBALINTDEFAULT
			, setGlobalInt
		)
		<< "This is a comment for a call-back option" << std::endl;
	} else {
		gpb.registerFileParameter<int>(
			"iOption2"
			, SOMEGLOBALINTDEFAULT
			, setGlobalInt
			, Gem::Common::VAR_IS_SECONDARY // Could also be VAR_IS_ESSENTIAL
			, "This is a comment for a call-back option"
		);
	}

	//----------------------------------------------------------------
	// Example 2: Registering a function or function object as call-back
	// that expects two parameters. This is meant for parameters that
	// only make sense when set together. Example: Lower and upper boundaries
	// of a random number generator.

	twoVariableFunctionObject tvfo;

	const int I3DEFAULT = 3;
	const double D3DEFAULT = 3.;

	if(useOperator) {
		gpb.registerFileParameter<int,double>(
			"iOption3"
			, "dOption1"
			, I3DEFAULT
			, D3DEFAULT
			, tvfo
			, "combinedLabel"
		)
		<< "A comment concerning the first option"
		<< nextComment() // commentLevel(1) would be another option
		<< "A comment concerning the second option;with a second line";
	} else {
		gpb.registerFileParameter<int,double>(
			"iOption3"
			, "dOption1"
			, I3DEFAULT
			, D3DEFAULT
			, tvfo
			, "combinedLabel"
			, Gem::Common::VAR_IS_ESSENTIAL // Could also be VAR_IS_SECONDARY
			, "A comment concerning the first option"
			, "A comment concerning the second option;with a second line"
		);
	}

	//----------------------------------------------------------------
	// Example 3: We can directly set a variable by providing a reference to it.
	int i = 0; const int IDEFAULT = 0;

	if(useOperator) {
		gpb.registerFileParameter<int>(
			"iOption"
			, i
			, IDEFAULT
		)
		<< "This is a comment; This is the second line of the comment";
	} else {
		gpb.registerFileParameter<int>(
			"iOption"
			, i
			, IDEFAULT
			, Gem::Common::VAR_IS_ESSENTIAL // Could also be VAR_IS_SECONDARY
			, "This is a comment; This is the second line of the comment"
		);
	}

	//----------------------------------------------------------------
	// Example 4: Adding a std::vector<> of configurable type to the config
	// file so a set of values can be read in one go
	std::vector<double> defaultDoubleVec4; // The default values
	defaultDoubleVec4.push_back(0.);
	defaultDoubleVec4.push_back(1.);

	if(useOperator) {
		gpb.registerFileParameter<double>(
			"vectorOptionsWithCallback"
			, defaultDoubleVec4
			, setGlobalDoubleVec // The call-back function. See at the beginning of this file
		)
		<< "Yet another comment";
	} else {
		gpb.registerFileParameter<double>(
			"vectorOptionsWithCallback"
			, defaultDoubleVec4
			, setGlobalDoubleVec // The call-back function. See at the beginning of this file
			, Gem::Common::VAR_IS_ESSENTIAL // Could also be VAR_IS_SECONDARY
			, "Yet another comment"
		);
	}

	//----------------------------------------------------------------
	// Example 5: Adding a reference to a vector of configurable type
	// to the collection.

	std::vector<double> targetDoubleVector; // Will hold the read values

	std::vector<double> defaultDoubleVec5; // The default values
	defaultDoubleVec5.push_back(0.);
	defaultDoubleVec5.push_back(1.);

	if(useOperator) {
		gpb.registerFileParameter<double>(
			"vectorOptionsReference"
			, targetDoubleVector
			, defaultDoubleVec5
		)
		<< "And yet another comment";

	} else {
		gpb.registerFileParameter<double>(
			"vectorOptionsReference"
			, targetDoubleVector
			, defaultDoubleVec5
			, Gem::Common::VAR_IS_ESSENTIAL // Could also be VAR_IS_SECONDARY
			, "And yet another comment"
		);
	}

	//----------------------------------------------------------------
	// Example 6: Adding an array of fixed size to the collection

	boost::array<int,ARRAYSIZE> defValArray;
	i=0; // Already declared above
	for(auto &x: defValArray)  { // Set the default values
		x = boost::numeric_cast<int>(i++);
	}

	if(useOperator) {
		gpb.registerFileParameter<int,ARRAYSIZE>(
			"boostArrayWithCallback"
			, defValArray
			, setGlobalBoostArray // The call back function
		)
		<< "A comment regarding arrays with call-back functions";
	} else {
		gpb.registerFileParameter<int,ARRAYSIZE>(
			"boostArrayWithCallback"
			, defValArray
			, setGlobalBoostArray // The call back function
			, Gem::Common::VAR_IS_ESSENTIAL // Could also be VAR_IS_SECONDARY
			, "A comment regarding arrays with call-back functions"
		);
	}

	//----------------------------------------------------------------
	// Example 7: Adding a reference to a boost::array object. We
	// use the same default values as in example 6.

	boost::array<int,ARRAYSIZE> targetArray;

	if(useOperator) {
		gpb.registerFileParameter<int,ARRAYSIZE>(
			"boostArrayReference"
			, targetArray
			, defValArray
		)
		<< "A comment regarding boost::array references";
	} else {
		gpb.registerFileParameter<int,ARRAYSIZE>(
			"boostArrayReference"
			, targetArray
			, defValArray
			, Gem::Common::VAR_IS_ESSENTIAL // Could also be VAR_IS_SECONDARY
			, "A comment regarding boost::array references"
		);
	}

	//----------------------------------------------------------------
	//////////////////////////////////////////////////////////////////
	//----------------------------------------------------------------

	// Check the number of registered options
	std::cout << "Got " << gpb.numberOfFileOptions() << " options." << std::endl;

	// Depending on the command line argument, write or read a configuration file
	switch(creationSwitcher) {
		case 0: // file creation
		{
			std::string header = "This is a not so complicated header;with a second line;and a third line as well";
			bool writeAll = true; // If set to false, only essential (but no secondary variables) are written
			gpb.writeConfigFile(fileName, header, writeAll);
		}
			break;

		case 1: // file parsing
		{
			gpb.parseConfigFile(fileName);
		}
			break;

		default: // Complain
			throw;
	}
}
