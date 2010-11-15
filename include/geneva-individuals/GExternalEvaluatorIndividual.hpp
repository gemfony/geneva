/**
 * @file GExternalEvaluatorIndividual.hpp
 */

/*
 * Copyright (C) Authors of the Geneva library collection and Karlsruhe
 * Institute of Technology (University of the State of Baden-Wuerttemberg
 * and National Laboratory of the Helmholtz Association).
 *
 * See the AUTHORS file in the top-level directory for a list of authors.
 *
 * Contact: info [at] gemfony (dot) com
 *
 * This file is part of the Geneva library collection
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

/**
 * NOTE: It is not at present clear whether this individual can be used in
 * a multi-threaded environment. Use with care.
 */

// Standard header files go here
#include <iostream>
#include <cmath>
#include <sstream>
#include <fstream>
#include <cstdlib>
#include <vector>
#include <iostream>
#include <string>

// Boost header files go here
#include <boost/shared_ptr.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/filesystem.hpp>
#include <boost/optional.hpp>


#ifndef GEXTERNALEVALUATORINDIVIDUAL_HPP_
#define GEXTERNALEVALUATORINDIVIDUAL_HPP_

// For Microsoft-compatible compilers
#if defined(_MSC_VER)  &&  (_MSC_VER >= 1020)
#pragma once
#endif

// Geneva header files go here
#include <common/GCommonEnums.hpp>
#include <common/GHelperFunctions.hpp>
#include <common/GParserBuilder.hpp>
#include <dataexchange/GDataExchangeEnums.hpp>
#include <dataexchange/GBoolParameter.hpp>
#include <dataexchange/GDataExchange.hpp>
#include <dataexchange/GDoubleParameter.hpp>
#include <dataexchange/GLongParameter.hpp>
#include <geneva/GBooleanAdaptor.hpp>
#include <geneva/GBooleanCollection.hpp>
#include <geneva/GConstrainedDoubleObjectCollection.hpp>
#include <geneva/GConstrainedInt32ObjectCollection.hpp>
#include <geneva/GDoubleCollection.hpp>
#include <geneva/GDoubleGaussAdaptor.hpp>
#include <geneva/GInt32FlipAdaptor.hpp>
#include <geneva/GParameterSet.hpp>


namespace bf = boost::filesystem; // alias for ease of use

namespace Gem
{
namespace Geneva
{

/************************************************************************************************/
/**
 * This individual calls an external program to evaluate a given set of parameters.
 * Data exchange happens through the GDataExchange class. The
 * structure of the individual is determined from information given by the external
 * program. Currently double-, bool- and boost::int32_t values are allowed.
 *
 * External programs should understand the following command line arguments
 * -i / --initialize : gives the external program the opportunity to do any needed
 *                          preliminary work (e.g. downloading files, setting up directories, ...)
 * -f / -- finilize :   Allows the external program to clean up after work. Compare the -i switch.
 * -p / --paramfile <filename>: The name of the file through which data is exchanged
 *     This switch is needed for the following options:
 *     -t / --template: asks the external program to write a description of the individual
 *                               into paramfile (e.g. program -t -p <filename> ).
 *            -t also allows the additional option -R (randomly initialize parameters)
 *     -r / --result  Asks the external program to emit a result file in a user-defined format.
 *                         It will not be read in by this individual, but allows the user to examine
 *                         the results. The input data needed to create the result file is contained
 *                         in the parameter file. Calls will be done like this: "program -r -p <filename>"
 * If the "-p <filename>" switch is used without any additional switches, the external program
 * is expected to perform a value calculation, based on the data in the parameter file and
 * to emit the result into the same file.
 * The following switch affects the desired transfer mode between external program
 * and this individual.
 * -m / --transferMode=<number>
 *     0 means binary mode (the default, if --transferMode is not used)
 *     1 means text mode
 * During evaluations or when being asked to print a result, the external program may
 * also be passed an additional identifying string (such as the current generation)
 * -g <string> switch. This can e.g. be used to create different result file names for different
 * generations. Usage of this string by the external program is optional.
 */
class GExternalEvaluatorIndividual
:public GParameterSet
 {
	///////////////////////////////////////////////////////////////////////
	friend class boost::serialization::access;

	template<class Archive>
	void serialize(Archive & ar, const unsigned int) {
		using boost::serialization::make_nvp;

		ar & BOOST_SERIALIZATION_BASE_OBJECT_NVP(GParameterSet)
		   & BOOST_SERIALIZATION_NVP(program_)
		   & BOOST_SERIALIZATION_NVP(arguments_)
		   & BOOST_SERIALIZATION_NVP(exchangeMode_)
		   & BOOST_SERIALIZATION_NVP(parameterFile_)
		   & BOOST_SERIALIZATION_NVP(gdbl_ptr_)
		   & BOOST_SERIALIZATION_NVP(glong_ptr_);
	}
	///////////////////////////////////////////////////////////////////////

 public:
	/** @brief A constructor which initializes the individual with the name of the external program that should be executed. */
	GExternalEvaluatorIndividual(
			const std::string& program
			, const std::string& ="empty"
			, const bool& =false
			, const Gem::Dataexchange::dataExchangeMode& = Gem::Dataexchange::BINARYEXCHANGE
			, boost::shared_ptr<GAdaptorT<double> > = boost::shared_ptr<GAdaptorT<double> >((GAdaptorT<double> *)NULL)
			, boost::shared_ptr<GAdaptorT<boost::int32_t> > = boost::shared_ptr<GAdaptorT<boost::int32_t> >((GAdaptorT<boost::int32_t> *)NULL)
			, boost::shared_ptr<GAdaptorT<bool> > = boost::shared_ptr<GAdaptorT<bool> >((GAdaptorT<bool> *)NULL)
	);

	/** @brief A standard copy constructor */
	GExternalEvaluatorIndividual(const GExternalEvaluatorIndividual&);
	/** @brief The standard destructor */
	~GExternalEvaluatorIndividual();

	/** @brief A standard assignment operator */
	const GExternalEvaluatorIndividual& operator=(const GExternalEvaluatorIndividual&);

	/** @brief Checks for equality with another GExternalEvaluatorIndividual object */
	bool operator==(const GExternalEvaluatorIndividual& cp) const;
	/** @brief Checks for inequality with another GExternalEvaluatorIndividual object */
	bool operator!=(const GExternalEvaluatorIndividual& cp) const;

	/** @brief Checks whether a given expectation for the relationship between this object and another object is fulfilled. */
	virtual boost::optional<std::string> checkRelationshipWith(
			const GObject&,
			const Gem::Common::expectation&,
			const double&,
			const std::string&,
			const std::string&,
			const bool&) const;

	/** @brief Sets the exchange mode between this individual and the external program. */
	Gem::Dataexchange::dataExchangeMode setDataExchangeMode(const Gem::Dataexchange::dataExchangeMode&);
	/** @brief Retrieves the current value of the exchangeMode_ variable */
	Gem::Dataexchange::dataExchangeMode getDataExchangeMode() const;

	/** @brief Sets the base name of the data exchange file */
	void setExchangeFileName(const std::string&);
	/** @brief Retrieves the current value of the parameterFile_ variable */
	std::string getExchangeFileName() const;

	/** @brief Initiates the printing of the best individual */
	void printResult(const std::string& = "empty");

	/********************************************************************************************/
	/**
	 * Asks the external program to perform any necessary initialization work. To be called
	 * from outside this class. It has been made a static member in order to centralize all
	 * external communication in this class.
	 *
	 * 	@param program The filename (including path) of the external program that should be executed
	 * @param arguments Additional user-defined arguments to be handed to the external program
	 */
	static void initialize(const std::string& program, const std::string& arguments) {
		GExternalEvaluatorIndividual::checkStringIsValid(program); // Do some error checking

		std::string commandLine = program + " -i ";
		if(!arguments.empty() && arguments != "empty") commandLine += (" " + arguments);

		Gem::Common::runExternalCommand(commandLine);
	}

	/********************************************************************************************/
	/**
	 * Asks the external program to perform any necessary finalization work. It has been
	 * made a static member in order to centralize all external communication in this class.
	 *
	 * 	@param program The filename (including path) of the external program that should be executed
	 * @param arguments Additional user-defined arguments to be handed to the external program
	 */
	static void finalize(const std::string& program, const std::string& arguments) {
		GExternalEvaluatorIndividual::checkStringIsValid(program); // Do some error checking

		std::string commandLine = program + " -f ";
		if(!arguments.empty() && arguments != "empty") commandLine += (" " + arguments);

		Gem::Common::runExternalCommand(commandLine);
	}

	/********************************************************************************************/
	/**
	 * A factory function that returns a GExternalEvaluatorIndividual, setting some values according
	 * to the specifications of a configuration file.
	 *
	 * @return A GExternalEvaluatorIndividual object, equipped according to the settings in a configuration file
	 */
	static boost::shared_ptr<GParameterSet> getExternalEvaluatorIndividual(const std::string& cf = "GExternalEvaluatorIndividual.cfg") {
		using namespace Gem::Common;

		// The variables that should be read from the configuration file
		boost::uint32_t processingCycles;
		std::string program;
		std::string externalArguments;
		boost::uint32_t adaptionThreshold;
		double sigma;
		double sigmaSigma;
		double minSigma;
		double maxSigma;
		Gem::Dataexchange::dataExchangeMode exchangeMode;
		bool randomFill;

		// Register a number of parameters
		GParserBuilder gpb(cf.c_str());
		gpb.registerParameter("processingCycles", processingCycles, boost::uint32_t(1));
		gpb.registerParameter("program", program, std::string("./evaluator/evaluator"));
		gpb.registerParameter("externalArguments", externalArguments, std::string("empty"));
		gpb.registerParameter("adaptionThreshold", adaptionThreshold, boost::uint32_t(1));
		gpb.registerParameter("sigma", sigma, double(0.5));
		gpb.registerParameter("sigmaSigma", sigmaSigma, double(0.8));
		gpb.registerParameter("minSigma", minSigma, double(0.));
		gpb.registerParameter("maxSigma", maxSigma, double(2.));
		gpb.registerParameter("exchangeMode", exchangeMode, Gem::Dataexchange::dataExchangeMode(Gem::Dataexchange::BINARYEXCHANGE));
		gpb.registerParameter("randomFill", randomFill, true);

		// Read the parameters from the configuration file
		if(!gpb.parse()) {
			std::ostringstream error;
			error << "In GFunctionIndividual::getExternalEvaluatorIndividual(const std::string&): Error!" << std::endl
				  << "Could not parse configuration file " << cf << std::endl;
			throw(Gem::Common::gemfony_error_condition(error.str()));
		}


	}

 protected:
	/********************************************************************************************/
	/** @brief Loads the data of another GExternalEvaluatorIndividual */
	virtual void load_(const GObject*);
	/** @brief Creates a deep clone of this object */
	virtual GObject* clone_() const;

	/** @brief The actual fitness calculation is triggered from here */
	virtual double fitnessCalculation();

 private:
	/** @brief The default constructor. Only needed for serialization purposes */
	GExternalEvaluatorIndividual();

	/** @brief Writes the class'es data to a file. */
	void writeParametersToFile(const std::string&);
	/** @brief Reads the class'es data from a file */
	double readParametersFromFile(const std::string&, bool&, const bool& = true);

	/********************************************************************************************/
	/**
	 * A helper function that checks whether a string is not empty. The function will throw if
	 * an empty string was found.
	 *
	 * @param s The string to be checked
	 */
	static void checkStringIsValid(const std::string& s) {
		if(s.empty() || s == "empty" || s == "unknown") {
			std::ostringstream error;
			error << "In GExternalEvaluatorIndividual::checkStringIsValid(): Error!" << std::endl
				  << "Received bad string \"" << s << "\"." << std::endl;
			throw (Gem::Common::gemfony_error_condition(error.str()));
		}
	}

	/********************************************************************************************/

	std::string program_; ///< The name of the external program to be executed
	std::string arguments_; ///< Any additional arguments to be handed to the external program
	Gem::Dataexchange::dataExchangeMode exchangeMode_; ///< The desired method of data exchange
	std::string parameterFile_;

	boost::shared_ptr<GConstrainedDoubleObject> gdbl_ptr_; ///< A template for GConstrainedDoubleObject objects
	boost::shared_ptr<GConstrainedInt32Object> glong_ptr_; ///< A template for GConstrainedInt32Object objects

	Gem::Dataexchange::GDataExchange gde_; ///< takes care of the data exchange with external programs
 };

} /* namespace Geneva */
} /* namespace Gem */



// Needed for testing purposes
/*************************************************************************************************/
///////////////////////////////////////////////////////////////////////////////////////////////////
/*************************************************************************************************/

#ifdef GENEVATESTING

/**
 * As the Gem::Geneva::Gem::Geneva::GExternalEvaluatorIndividual has a private default constructor, we need to provide a
 * specialization of the factory function that creates GStartProjectIndividual objects
 */
template <> boost::shared_ptr<Gem::Geneva::GExternalEvaluatorIndividual> TFactory_GUnitTests<Gem::Geneva::GExternalEvaluatorIndividual>();

#endif /* GENEVATESTING */

/*************************************************************************************************/
///////////////////////////////////////////////////////////////////////////////////////////////////
/*************************************************************************************************/

#endif /* GEXTERNALEVALUATORINDIVIDUAL_HPP_ */
