/**
 * @file GExternalEvaluator.hpp
 */

/* Copyright (C) Dr. Ruediger Berlich and Karlsruhe Institute of Technology
 * (University of the State of Baden-Wuerttemberg and National Laboratory
 * of the Helmholtz Association)
 *
 * Contact: info [at] gemfony (dot) com
 *
 * This file is part of the Geneva library, Gemfony scientific's optimization
 * library.
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

#ifndef GEXTERNALEVALUATOR_HPP_
#define GEXTERNALEVALUATOR_HPP_

// For Microsoft-compatible compilers
#if defined(_MSC_VER)  &&  (_MSC_VER >= 1020)
#pragma once
#endif

// GenEvA header files go here
#include "GDoubleCollection.hpp"
#include "GParameterSet.hpp"
#include "GBoundedDoubleCollection.hpp"
#include "GBoundedInt32Collection.hpp"
#include "GCharObjectCollection.hpp"
#include "GBooleanCollection.hpp"
#include "GDoubleGaussAdaptor.hpp"
#include "GBooleanAdaptor.hpp"
#include "GInt32FlipAdaptor.hpp"
#include "GCharFlipAdaptor.hpp"
#include "GDataExchange.hpp"
#include "GEnums.hpp"
#include "GDoubleParameter.hpp"
#include "GLongParameter.hpp"
#include "GBoolParameter.hpp"
#include "GCharParameter.hpp"

namespace Gem
{
namespace GenEvA
{

/************************************************************************************************/
/**
 * This individual calls an external program to evaluate a given set of parameters.
 * Data exchange happens through the GDataExchange class. The
 * structure of the individual is determined from information given by the external
 * program. Currently double-, bool-, boost::int32_t- and char-values are allowed.
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
class GExternalEvaluator
	:public GParameterSet
{
	///////////////////////////////////////////////////////////////////////
	friend class boost::serialization::access;

	template<class Archive>
	void serialize(Archive & ar, const unsigned int) {
		using boost::serialization::make_nvp;

		ar & make_nvp("GParameterSet", boost::serialization::base_object<GParameterSet>(*this));
		ar & make_nvp("program_", program_);
		ar & make_nvp("arguments_", arguments_);
		ar & make_nvp("nEvaluations_", nEvaluations_);
		ar & make_nvp("exchangeMode_", exchangeMode_);
		ar & make_nvp("maximize_", maximize_);
		ar & make_nvp("parameterFile_", parameterFile_);
		ar & make_nvp("gdbl_ptr_", gdbl_ptr_);
		ar & make_nvp("glong_ptr_", glong_ptr_);
		ar & make_nvp("gchar_ptr_", gchar_ptr_);
	}
	///////////////////////////////////////////////////////////////////////

public:
	/********************************************************************************************/
	/**
	 * A constructor which initializes the individual with the name of the external program
	 * that should be executed. The external program is asked for the desired structure of
	 * the individual, and the corresponding data sets are added. Only one individual
	 * needs to be constructed using this method. All other individuals of the population
	 * should be created as copies of this first individual.
	 *
	 * @param program The filename (including path) of the external program that should be executed
	 * @param arguments Additional user-defined arguments to be handed to the external program
	 * @param random A boolean indicating whether template data should be filled randomly
	 * @param gdbl_ad_ptr An adaptor for double values
	 * @param glong_ad_ptr An adaptor for boost::int32_t values
	 * @param gbool_ad_ptr An adaptor for bool values
	 * @param gchar_ad_ptr An adaptor for char values
	 */
	GExternalEvaluator(const std::string& program,
			                             const std::string& arguments="empty",
			                             const bool& random=false,
			                             const dataExchangeMode& exchangeMode = Gem::GenEvA::BINARYEXCHANGE,
			                             boost::shared_ptr<GAdaptorT<double> > gdbl_ad_ptr = boost::shared_ptr<GAdaptorT<double> >((GAdaptorT<double> *)NULL),
			                             boost::shared_ptr<GAdaptorT<boost::int32_t> > glong_ad_ptr = boost::shared_ptr<GAdaptorT<boost::int32_t> >((GAdaptorT<boost::int32_t> *)NULL),
			                             boost::shared_ptr<GAdaptorT<bool> > gbool_ad_ptr = boost::shared_ptr<GAdaptorT<bool> >((GAdaptorT<bool> *)NULL),
			                             boost::shared_ptr<GAdaptorT<char> > gchar_ad_ptr = boost::shared_ptr<GAdaptorT<char> >((GAdaptorT<char> *)NULL)	)
		:program_(program),
		 arguments_(arguments),
		 nEvaluations_(1),
		 exchangeMode_(exchangeMode),
		 maximize_(false),
		 parameterFile_("./parameterData")
	{
		// Set up the local adaptor templates and collection items
		//-----------------------------------------------------------------------------------------------
		gdbl_ptr_=boost::shared_ptr<GBoundedDouble>(new GBoundedDouble());
        if(gdbl_ad_ptr) {
        	gdbl_ptr_->addAdaptor(gdbl_ad_ptr->GObject::clone_bptr_cast<GAdaptorT<double> >());
        }
        else {
        	gdbl_ptr_->addAdaptor(boost::shared_ptr<GAdaptorT<double> >(new GDoubleGaussAdaptor())); // uses default values
        }

		//-----------------------------------------------------------------------------------------------
        glong_ptr_=boost::shared_ptr<GBoundedInt32>(new GBoundedInt32());
        if(glong_ad_ptr) {
        	glong_ptr_->addAdaptor(glong_ad_ptr->GObject::clone_bptr_cast<GAdaptorT<boost::int32_t> >());
        }
        else {
        	glong_ptr_->addAdaptor(boost::shared_ptr<GAdaptorT<boost::int32_t> >(new GInt32FlipAdaptor())); // uses default values
        }

		//-----------------------------------------------------------------------------------------------
        gchar_ptr_=boost::shared_ptr<GChar>(new GChar());
        if(gchar_ad_ptr) {
        	gchar_ptr_->addAdaptor(gchar_ad_ptr->GObject::clone_bptr_cast<GAdaptorT<char> >());
        }
        else {
        	gchar_ptr_->addAdaptor(boost::shared_ptr<GAdaptorT<char> >(new GCharFlipAdaptor())); // uses default values
        }

		//-----------------------------------------------------------------------------------------------
		// Fill the class with the required, empty collections.
        boost::shared_ptr<GBoundedDoubleCollection> gbdc_ptr(new GBoundedDoubleCollection());
		this->push_back(gbdc_ptr);

		boost::shared_ptr<GBoundedInt32Collection> gbic_ptr(new GBoundedInt32Collection());
		this->push_back(gbic_ptr);

		// GBooleanCollection is special in that it directly contains adaptors
		boost::shared_ptr<GBooleanCollection> gbc_ptr(new GBooleanCollection());
        if(gbool_ad_ptr) {
        	gbc_ptr->addAdaptor(gbool_ad_ptr->GObject::clone_bptr_cast<GAdaptorT<bool> >());
        }
        else {
        	gbc_ptr->addAdaptor(boost::shared_ptr<GAdaptorT<bool> >(new GBooleanAdaptor())); // uses default values
        }
		this->push_back(gbc_ptr);

		boost::shared_ptr<GCharObjectCollection> gcoc_ptr(new GCharObjectCollection());
		this->push_back(gcoc_ptr);

		//-----------------------------------------------------------------------------------------------------------------------------
		// Tell the external program to send us a template with the structure of the individual

		if(program_.empty() || program_ == "empty" || program_ == "unknown") {
			std::ostringstream error;
			error << "In GExternalEvaluator::GExternalEvaluator(...) : received bad program name \"" << program_ << "\"." << std::endl;
			throw (Gem::GenEvA::geneva_error_condition(error.str()));
		}

		std::string commandLine;
		if(exchangeMode_ == Gem::GenEvA::BINARYEXCHANGE) {
			commandLine = program_ + " -m 0 -t " +(random?std::string(" -R"):std::string("")) + " -p " + parameterFile_;
		}
		else {
			commandLine = program_ + " -m 1 -t " +(random?std::string(" -R"):std::string("")) + " -p " + parameterFile_;
		}

		if(!arguments.empty() && arguments != "empty") commandLine += (" " + arguments_);

#ifdef PRINTCOMMANDLINE
		std::cout << "Requesting template with commandLine = \"" << commandLine << "\" ...";
#endif /* PRINTCOMMANDLINE */
		GExternalEvaluator::runCommand(commandLine);
#ifdef PRINTCOMMANDLINE
		std::cout << "... done." << std::endl;
#endif /* PRINTCOMMANDLINE */


		//-----------------------------------------------------------------------------------------------------------------------------
		// Finally fill this class up with the external template data. Make sure the data doesn't get sorted
		bool hasValue;
		this->readParametersFromFile(parameterFile_, hasValue, false);

		// Erase the parameter file - not needed anymore. Hmmm - this is not portable to Windows
		// See here for a way to do this portably: http://www.boost.org/doc/libs/1_38_0/libs/filesystem/doc/index.htm
		GExternalEvaluator::runCommand("rm -f  " + parameterFile_);
	}

	/********************************************************************************************/
	/**
	 * A standard copy constructor.
	 */
	GExternalEvaluator(const GExternalEvaluator& cp)
		:GParameterSet(cp), // copies all local collections
		 program_(cp.program_),
		 arguments_(cp.arguments_),
		 nEvaluations_(cp.nEvaluations_),
		 exchangeMode_(cp.exchangeMode_),
		 maximize_(cp.maximize_),
		 parameterFile_(cp.parameterFile_)
	{
		 gdbl_ptr_ = cp.gdbl_ptr_->GObject::clone_bptr_cast<GBoundedDouble>();
		 glong_ptr_ = cp.glong_ptr_->GObject::clone_bptr_cast<GBoundedInt32>();
		 gchar_ptr_ = cp.gchar_ptr_->GObject::clone_bptr_cast<GChar>();
	}

	/********************************************************************************************/
	/**
	 * The standard destructor
	 */
	~GExternalEvaluator()
	{ /* nothing */	}

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
		if(program.empty() || program == "empty" || program == "unknown") {
			std::ostringstream error;
			error << "In GExternalEvaluator::initialize : received bad program name \"" << program << "\"." << std::endl;
			throw (Gem::GenEvA::geneva_error_condition(error.str()));
		}

		std::string commandLine = program + " -i ";
		if(!arguments.empty() && arguments != "empty") commandLine += (" " + arguments);

#ifdef PRINTCOMMANDLINE
		std::cout << "Initializing with command line = \"" << commandLine << "\" ...";
#endif /* PRINTCOMMANDLINE */
		GExternalEvaluator::runCommand(commandLine);
#ifdef PRINTCOMMANDLINE
		std::cout << " ... done." << std::endl;
#endif /* PRINTCOMMANDLINE */
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
		if(program.empty() || program == "empty" || program == "unknown") {
			std::ostringstream error;
			error << "In GExternalEvaluator::initialize : received bad program name \"" << program << "\"." << std::endl;
			throw (Gem::GenEvA::geneva_error_condition(error.str()));
		}

		std::string commandLine = program + " -f ";
		if(!arguments.empty() && arguments != "empty") commandLine += (" " + arguments);

#ifdef PRINTCOMMANDLINE
		std::cout << "Finalizing with command line = \"" << commandLine << "\" ...";
#endif /* PRINTCOMMANDLINE */
		GExternalEvaluator::runCommand(commandLine);
#ifdef PRINTCOMMANDLINE
		std::cout << " ... done." << std::endl;
#endif /* PRINTCOMMANDLINE */
	}

	/********************************************************************************************/
	/**
	 * A standard assignment operator
	 */
	const GExternalEvaluator& operator=(const GExternalEvaluator& cp){
		GExternalEvaluator::load(&cp);
		return *this;
	}

	/********************************************************************************************/
	/**
	 * Creates a deep clone of this object
	 *
	 * @return A deep clone of this object, camouflaged as a GObject
	 */
	virtual GObject* clone() const{
		return new GExternalEvaluator(*this);
	}

	/********************************************************************************************/
	/**
	 * Loads the data of another GExternalEvaluator, camouflaged as a GObject
	 *
	 * @param cp A copy of another GExternalEvaluator, camouflaged as a GObject
	 */
	virtual void load(const GObject* cp){
		const GExternalEvaluator *gev_load = conversion_cast(cp, this);

		// First load the data of our parent class ...
		GParameterSet::load(cp);

		// ... and then our own
		program_ = gev_load->program_;
		arguments_ = gev_load->arguments_;
		nEvaluations_ = gev_load->nEvaluations_;
		exchangeMode_ = gev_load->exchangeMode_;
		maximize_ = gev_load->maximize_;
		parameterFile_ = gev_load->parameterFile_;

		gdbl_ptr_ = gev_load->gdbl_ptr_->GObject::clone_bptr_cast<GBoundedDouble>();
		glong_ptr_ = gev_load->glong_ptr_->GObject::clone_bptr_cast<GBoundedInt32>();
		gchar_ptr_ = gev_load->gchar_ptr_->GObject::clone_bptr_cast<GChar>();
	}

	/********************************************************************************************/
	/**
	 * Checks for equality with another GExternalEvaluator object
	 *
	 * @param  cp A constant reference to another GExternalEvaluator object
	 * @return A boolean indicating whether both objects are equal
	 */
	bool operator==(const GExternalEvaluator& cp) const {
		return GExternalEvaluator::isEqualTo(cp, boost::logic::indeterminate);
	}

	/********************************************************************************************/
	/**
	 * Checks for inequality with another GExternalEvaluator object
	 *
	 * @param  cp A constant reference to another GExternalEvaluator object
	 * @return A boolean indicating whether both objects are inequal
	 */
	bool operator!=(const GExternalEvaluator& cp) const {
		return !GExternalEvaluator::isEqualTo(cp, boost::logic::indeterminate);
	}

	/********************************************************************************************/
	/**
	 * Checks for equality with another GExternalEvaluator object..
	 *
	 * @param  cp A constant reference to another GExternalEvaluator object
	 * @return A boolean indicating whether both objects are equal
	 */
	virtual bool isEqualTo(const GObject& cp, const boost::logic::tribool& expected = boost::logic::indeterminate) const {
		using namespace Gem::Util;

		// Check that we are indeed dealing with a GNumCollectionT reference
		const GExternalEvaluator *gev_load = GObject::conversion_cast(&cp,  this);

		//----------------------------------------------------------------------------------------------------
		// Check equality of the parent class
		if(GParameterSet::isNotEqualTo(*gev_load)) return false;

		//----------------------------------------------------------------------------------------------------
		// Then check our local data

		// First basic types
		if(checkForInequality("GExternalEvaluator", program_, gev_load->program_,"program_", "gev_load->program_", expected)) return false;
		if(checkForInequality("GExternalEvaluator", arguments_, gev_load->arguments_,"arguments_", "gev_load->arguments_", expected)) return false;
		if(checkForInequality("GExternalEvaluator", nEvaluations_, gev_load->nEvaluations_,"nEvaluations_", "gev_load->nEvaluations_", expected)) return false;
		if(checkForInequality("GExternalEvaluator", exchangeMode_, gev_load->exchangeMode_,"exchangeMode_", "gev_load->exchangeMode_", expected)) return false;
		if(checkForInequality("GExternalEvaluator", maximize_, gev_load->maximize_,"maximize_", "gev_load->maximize_", expected)) return false;
		if(checkForInequality("GExternalEvaluator", parameterFile_, gev_load->parameterFile_,"parameterFile_", "gev_load->parameterFile_", expected)) return false;

		// Then objects
		if(gdbl_ptr_->isNotEqualTo(*(gev_load->gdbl_ptr_), expected)) return false;
		if(glong_ptr_->isNotEqualTo(*(gev_load->glong_ptr_), expected)) return false;
		if(gchar_ptr_->isNotEqualTo(*(gev_load->gchar_ptr_), expected)) return false;

		return true;
	}

	/********************************************************************************************/
	/**
	 * Checks for similarity with another GExternalEvaluator object.
	 * As we have no local data, we just check for equality of the parent class.
	 *
	 * @param  cp A constant reference to another GExternalEvaluator object
	 * @param limit A double value specifying the acceptable level of differences of floating point values
	 * @return A boolean indicating whether both objects are similar to each other
	 */
	virtual bool isSimilarTo(const GObject& cp, const double& limit, const boost::logic::tribool& expected = boost::logic::indeterminate) const {
		using namespace Gem::Util;

		// Check that we are indeed dealing with a GNumCollectionT reference
		const GExternalEvaluator *gev_load = GObject::conversion_cast(&cp,  this);

		//----------------------------------------------------------------------------------------------------
		// Check similarity of the parent class
		if(GParameterSet::isNotSimilarTo(*gev_load, limit)) return false;

		//----------------------------------------------------------------------------------------------------
		// Then check our local data

		// First the basic types
		if(checkForDissimilarity("GExternalEvaluator", program_, gev_load->program_, limit, "program_", "gev_load->program_", expected)) return false;
		if(checkForDissimilarity("GExternalEvaluator", arguments_, gev_load->arguments_, limit,"arguments_", "gev_load->arguments_", expected)) return false;
		if(checkForDissimilarity("GExternalEvaluator", nEvaluations_, gev_load->nEvaluations_, limit,"nEvaluations_", "gev_load->nEvaluations_", expected)) return false;
		if(checkForDissimilarity("GExternalEvaluator", exchangeMode_, gev_load->exchangeMode_, limit, "exchangeMode_", "gev_load->exchangeMode_", expected)) return false;
		if(checkForDissimilarity("GExternalEvaluator", maximize_, gev_load->maximize_, limit, "maximize_", "gev_load->maximize_", expected)) return false;
		if(checkForDissimilarity("GExternalEvaluator", parameterFile_, gev_load->parameterFile_, limit, "parameterFile_", "gev_load->parameterFile_", expected)) return false;

		// Then objects
		if(gdbl_ptr_->isNotSimilarTo(*(gev_load->gdbl_ptr_), limit, expected)) return false;
		if(glong_ptr_->isNotSimilarTo(*(gev_load->glong_ptr_), limit, expected)) return false;
		if(gchar_ptr_->isNotSimilarTo(*(gev_load->gchar_ptr_), limit, expected)) return false;

		return true;
	}

	/********************************************************************************************/
	/**
	 * Sets the number of evaluations that should be handed to the external program
	 *
	 * @param nEvaluations The number of evaluations to be handed to the external program
	 */
	void setNEvaluations(const boost::uint32_t nEvaluations) {
		nEvaluations_ = nEvaluations;
	}

	/********************************************************************************************/
	/**
	 * Retrieves the number of evaluations an external program will be asked to do
	 *
	 * @return The number of evaluations an external program will be asked to do
	 */
	boost::uint32_t getNEvaluations() {
		return nEvaluations_;
	}

	/********************************************************************************************/
	/**
	 * Sets the exchange mode between this individual and the external program. Allowed
	 * values are defined in the dataExchangeMode enum. As this allows a comile-time check
	 * of supplied arguments, no further error checks are needed.
	 *
	 * @param The current exchange mode for data
	 * @return The previous exchange mode
	 */
	dataExchangeMode setDataExchangeMode(const dataExchangeMode& exchangeMode) {
		dataExchangeMode current = exchangeMode_;
		exchangeMode_ = exchangeMode;
		return current;
	}

	/********************************************************************************************/
	/**
	 * Retrieves the current value of the exchangeMode_ variable.
	 *
	 * @return The current exchange mode
	 */
	dataExchangeMode getDataExchangeMode() {
		return exchangeMode_;
	}

	/********************************************************************************************/
	/**
	 * Allows to specify whether smaller or larger values of this individual count as
	 * better. Affects the sorting of multiple data sets handed to external programs.
	 *
	 * @param A boolean indicating whether we do maximization (true) or minimization (false)
	 */
	void setMaximize(const bool& maximize = true)	{
		maximize_ = maximize;
	}

	/********************************************************************************************/
	/**
	 * Allows to determine whether smaller or larger values of this individual count as
	 * better.
	 *
	 * @return A boolean indicating whether we do maximization (true) or minimization (false)
	 */
	bool getMaximize() {
		return maximize_;
	}

	/********************************************************************************************/
	/**
	 * Sets the base name of the data exchange file. Note that the individual might add additional
	 * characters in order to distinguish between the exchange files of different individuals.
	 *
	 * @param parameterFile The desired new base name of the exchange file
	 */
	void setExchangeFileName(const std::string& parameterFile) {
		if(parameterFile.empty() || parameterFile == "empty") {
			std::ostringstream error;
			error << "In GExternalEvaluator::setExchangeFileName(): Error!" << std::endl
				     << "Invalid file name \"" << parameterFile << "\"" << std::endl;
			throw geneva_error_condition(error.str());
		}

		parameterFile_ = parameterFile;
	}

	/********************************************************************************************/
	/**
	 * Retrieves the current value of the parameterFile_ variable.
	 *
	 * @return The current base name of the exchange file
	 */
	std::string getExchangeFileName() {
		return parameterFile_;
	}

	/********************************************************************************************/
	/**
	 * Initiates the printing of the best individual
	 */
	void printResult(const std::string& identifier = "empty") {
		std::string commandLine;

		// Determine the output file name
		std::string bestParameterSetFile = "bestParameterSet";

		// Emit our data
		this->writeParametersToFile(bestParameterSetFile);

		// Check that we have a valid program_ ...
		if(program_.empty() || program_ == "empty" || program_=="unknown") {
			std::ostringstream error;
			error << "In GExternalEvaluator::printResult(): Error!" << std::endl
				      << "Invalid program name \"" << program_ << "\"" << std::endl;

			throw geneva_error_condition(error.str());
		}

		// Assemble command line and run the external program
		if(exchangeMode_ == Gem::GenEvA::BINARYEXCHANGE)
			commandLine = program_ + " -m 0  -r -p " + bestParameterSetFile;
		else
			commandLine = program_ + " -m 1  -r -p " + bestParameterSetFile;;

	    if(identifier != "empty" && !identifier.empty()) commandLine += (" -g \"" + identifier + "\"");
		if(arguments_ != "empty" && !arguments_.empty()) commandLine +=  (" " + arguments_);

#ifdef PRINTCOMMANDLINE
		std::cout << "Printing result with command line = \"" << commandLine << "\" ...";
#endif /* PRINTCOMMANDLINE */
		// Initiate the result calculation
		GExternalEvaluator::runCommand(commandLine);
#ifdef PRINTCOMMANDLINE
		std::cout << " ... done." << std::endl;
#endif /* PRINTCOMMANDLINE */

		// Clean up - remove the result file
		GExternalEvaluator::runCommand("rm -f " + bestParameterSetFile);
	}

protected:
	/********************************************************************************************/
	/**
	 * The actual fitness calculation takes place in an external program. Here we just
	 * write a file with the required parameters to disk and execute the program.
	 *
	 * @return The value of this object
	 */
	virtual double fitnessCalculation() {
		// Check that we have a valid program_ name ...
		if(program_.empty() || program_ == "empty" || program_ == "unknown") {
			std::ostringstream error;
			error << "In GExternalEvaluator::fitnessCalculation(): Error!" << std::endl
				     << "Invalid program name \"" << program_ << "\"" << std::endl;

			throw geneva_error_condition(error.str());
		}

		// Make the parameters known externally
		std::string parFile = parameterFile_ + "_" + boost::lexical_cast<std::string>(this->getPersonalityTraits()->getParentAlgIteration());

		if(this->getPersonality() == EA) parFile +=  ("_" +  boost::lexical_cast<std::string>( this->getEAPersonalityTraits()->getPopulationPosition()));

		// Write out the required data
		this->writeParametersToFile(parFile);

		// Assemble command line and run the external program
		std::string commandLine;

		if(exchangeMode_ == Gem::GenEvA::BINARYEXCHANGE)
			commandLine = program_ + " -m 0  -p " + parFile;
		else
			commandLine = program_ + " -m 1  -p " + parFile;;


		if(arguments_ != "empty" && !arguments_.empty())
			commandLine += (" " + arguments_);

#ifdef PRINTCOMMANDLINE
		std::cout << "Calculating result with command line = \"" << commandLine << "\" ...";
#endif /* PRINTCOMMANDLINE */
		GExternalEvaluator::runCommand(commandLine); // It is not clear whether this is thread-safe
#ifdef PRINTCOMMANDLINE
		std::cout << " ... done." << std::endl;
#endif /* PRINTCOMMANDLINE */

		bool hasValue = false;
		double result = this->readParametersFromFile(parFile, hasValue, true);

		// Check whether a result value was received
		if(!hasValue) {
			std::ostringstream error;
			error << "In GExternalEvaluator::fitnessCalculation(): Error!" << std::endl
				     << "Received no value from the external calculation" << std::endl;

			throw geneva_error_condition(error.str());
		}

		// Clean up - remove the parameter file
		GExternalEvaluator::runCommand("rm -f " + parFile);

		// Let the audience know
		return result;
	}

private:
	/********************************************************************************************/
	/**
	 * The default constructor. Only needed for serialization purposes
	 */
	GExternalEvaluator()
		:program_("unknown"),
		 arguments_("empty"),
		 nEvaluations_(1),
		 exchangeMode_(Gem::GenEvA::BINARYEXCHANGE),
		 maximize_(false),
		 parameterFile_("empty"),
         gdbl_ptr_(boost::shared_ptr<GBoundedDouble>((GBoundedDouble *)NULL)),
         glong_ptr_(boost::shared_ptr<GBoundedInt32>((GBoundedInt32 *)NULL)),
         gchar_ptr_(boost::shared_ptr<GChar>((GChar *)NULL))
	{ /* nothing */ }

	/********************************************************************************************/
	/**
	 * Writes the class'es data to a file. Note that, if the nEvaluations_ variable is set to a
	 * value higher than 1, this function will create multiple, mutated copies of this
	 * individual and add them to the output file. The goal is to allow external programs
	 * to perform more than one evaluation in sequence, so the overhead incurred through
	 * the frequent disc i/o is reduced.
	 *
	 * The structure of this individual is:
	 * GBoundedDoubleCollection
	 * GBoundedInt32Collection
	 * GBooleanCollection
	 * GCharObjectCollection
	 *
	 * @param fileName The name of the file to write to
	 */
	void writeParametersToFile(const std::string& fileName) {
		// Make sure we are dealing with a clean exchange module
		gde_.resetAll();

		// Create nEvaluations_ data sets from this object
		for(boost::uint32_t i=0; i<nEvaluations_; i++) {
			boost::shared_ptr<GExternalEvaluator> p; // the additional object will vanish at the end of the loop

			// More than one data set requested for evaluation ?
			if(i>0) {
				// Switch to a new page in the GDataExchange module
				gde_.newDataSet();

				// Create a copy of this object and mutate it
				p = this->clone_bptr_cast<GExternalEvaluator>();
				p->setAllowLazyEvaluation(true); // Prevent evaluation upon mutation
				p->mutate(); // Make sure we do not evaluate the same parameter set
			}

			// Retrieve pointers to the four containers and add their data to the GDataExchange module

			 // A GBoundedDoubleCollection can mostly be treated like a std::vector<boost::shared_ptr<GBoundedDouble> >
			boost::shared_ptr<GBoundedDoubleCollection> gbdc;
			if(i==0) {
				gbdc = pc_at<GBoundedDoubleCollection>(0);
			}
			else {
				gbdc = p->pc_at<GBoundedDoubleCollection>(0);
			}
			GBoundedDoubleCollection::iterator gbdc_it;
			for(gbdc_it=gbdc->begin(); gbdc_it!=gbdc->end(); ++gbdc_it) {
				boost::shared_ptr<Gem::Util::GDoubleParameter> dpar(new Gem::Util::GDoubleParameter((*gbdc_it)->value(), (*gbdc_it)->getLowerBoundary(), (*gbdc_it)->getUpperBoundary()));
				gde_.append(dpar);
			}


			boost::shared_ptr<GBoundedInt32Collection> gbic;
			if(i==0) {
				gbic = pc_at<GBoundedInt32Collection>(1);
			}
			else {
				gbic = p->pc_at<GBoundedInt32Collection>(1);
			}
			GBoundedInt32Collection::iterator gbic_it;
			for(gbic_it=gbic->begin(); gbic_it!=gbic->end(); ++gbic_it) {
				boost::shared_ptr<Gem::Util::GLongParameter> ipar(new Gem::Util::GLongParameter((*gbic_it)->value(), (*gbic_it)->getLowerBoundary(), (*gbic_it)->getUpperBoundary()));
				gde_.append(ipar);
			}


			boost::shared_ptr<GBooleanCollection> gbc;
			if(i==0) {
				gbc = pc_at<GBooleanCollection>(2);
			}
			else {
				gbc = p->pc_at<GBooleanCollection>(2);
			}
			GBooleanCollection::iterator gbc_it;
			for(gbc_it=gbc->begin(); gbc_it!=gbc->end(); ++gbc_it) {
				boost::shared_ptr<Gem::Util::GBoolParameter> bpar(new Gem::Util::GBoolParameter(*gbc_it)); // no boundaries for booleans
				gde_.append(bpar);
			}


			boost::shared_ptr<GCharObjectCollection> gcoc;
			if(i==0) {
				gcoc = pc_at<GCharObjectCollection>(3);
			}
			else {
				gcoc = p->pc_at<GCharObjectCollection>(3);
			}
			GCharObjectCollection::iterator gcoc_it;
			for(gcoc_it=gcoc->begin(); gcoc_it!=gcoc->end(); ++gcoc_it) {
				boost::shared_ptr<Gem::Util::GCharParameter> cpar(new Gem::Util::GCharParameter((*gcoc_it)->value())); // no boundaries for characters for now
				gde_.append(cpar);
			}
		}

		// At this point all necessary data should have been stored in the GDataExchange module. We can now write it to file.
		if(exchangeMode_ == Gem::GenEvA::BINARYEXCHANGE)	gde_.writeToFile(fileName, true);
		else gde_.writeToFile(fileName, false); // TEXTEXCHANGE
	}

	/********************************************************************************************/
	/**
	 * Reads the class'es data from a file and loads the best data set into the local
	 * structures.
	 *
	 * @param fileName The name of the file to read from
	 * @return The value of the data set in the file (if available), or 0.
	 */
	double readParametersFromFile(const std::string& fileName, bool& hasValue, const bool& sort=true) {
		hasValue = false;

		// Make sure gde_ is empty
		gde_.resetAll();

		// Read in the data
		if(exchangeMode_ == BINARYEXCHANGE)	gde_.readFromFile(fileName,  true);
		else gde_.readFromFile(fileName, false); // TEXTEXCHANGE

		if(sort) {
			// Switch to the best data set in the collection
			if(maximize_) gde_.switchToBestDataSet(false); // descending order
			else gde_.switchToBestDataSet(true); // ascending order
		}

		//--------------------------------------------------------------------------------------------------------------------------------------
		// Retrieve our "double" collection items
		boost::shared_ptr<GBoundedDoubleCollection> gbdc = pc_at<GBoundedDoubleCollection>(0);

		// Get the size of the  "foreign" container ...
		std::size_t exchangeSize = gde_.size<double>();

		// ... and adjust the collection size, as needed . This will erase items
		// or add copies of the second argument, as needed.
		gbdc->resize(exchangeSize, gdbl_ptr_);

		// Now copy the items over
		GBoundedDoubleCollection::iterator gbdc_it;
		std::size_t pos;
		for(pos=0, gbdc_it=gbdc->begin(); gbdc_it!=gbdc->end(); ++pos, ++gbdc_it) {
			if(!(*gbdc_it)->hasAdaptor()) {
				std::ostringstream error;
				error << "In GExternalEvaluator::readParametersFromFile(): Error!" << std::endl
						 << "GBoundedDouble item " << pos << " has no adaptor" << std::endl;

				throw geneva_error_condition(error.str());
			}

			boost::shared_ptr<Gem::Util::GDoubleParameter> gdp_ptr = gde_.parameterSet_at<double>(pos);
			if(gdp_ptr->hasBoundaries()) {
				(*gbdc_it)->setBoundaries(gdp_ptr->getLowerBoundary(), gdp_ptr->getUpperBoundary());
				**gbdc_it = gdp_ptr->value();
			}
			else {
				(*gbdc_it)->resetBoundaries();
				**gbdc_it = gdp_ptr->value();
			}
		}

		//--------------------------------------------------------------------------------------------------------------------------------------
		// Retrieve our "long" collection items
		boost::shared_ptr<GBoundedInt32Collection> gbic = pc_at<GBoundedInt32Collection>(1);

		// Make sure we have (template-)items in the local collection
		if(gbic->empty()) {
			boost::shared_ptr<GBoundedInt32> gbi_templ(new GBoundedInt32());
			boost::shared_ptr<GInt32FlipAdaptor> gifa_templ(new GInt32FlipAdaptor());
			gbi_templ->addAdaptor(gifa_templ);
			gbic->push_back(gbi_templ);
		}

		// Get the size of the "foreign" container ...
		exchangeSize = gde_.size<boost::int32_t>();

		// ... and adjust the population size, as needed . This will erase items
		// or add copies of the second argument, as needed.
		gbic->resize(exchangeSize, glong_ptr_);

		// Now copy the items over
		GBoundedInt32Collection::iterator gbic_it;
		for(pos=0, gbic_it=gbic->begin(); gbic_it!=gbic->end(); ++pos, ++gbic_it) {
			if(!(*gbic_it)->hasAdaptor()) {
				std::ostringstream error;
				error << "In GExternalEvaluator::readParametersFromFile(): Error!" << std::endl
						 << "GBoundedInt32 item " << pos << " has no adaptor" << std::endl;

				throw geneva_error_condition(error.str());
			}

			boost::shared_ptr<Gem::Util::GLongParameter> glp_ptr = gde_.parameterSet_at<boost::int32_t>(pos);
			if(glp_ptr->hasBoundaries()) {
				(*gbic_it)->setBoundaries(glp_ptr->getLowerBoundary(), glp_ptr->getUpperBoundary());
				**gbic_it = glp_ptr->value();
			}
			else {
				(*gbic_it)->resetBoundaries();
				**gbic_it = glp_ptr->value();
			}
		}

		//--------------------------------------------------------------------------------------------------------------------------------------
		// Retrieve our "bool" collection items
		boost::shared_ptr<GBooleanCollection> gbc = pc_at<GBooleanCollection>(2);

		// Get the size of the "foreign" container ...
		exchangeSize = gde_.size<bool>();

		// ... and adjust the population size, as needed . This will erase items
		// or add copies of the second argument, as needed.
		gbc->resize(exchangeSize, false);

		// Now copy the items over
		GBooleanCollection::iterator gbc_it;
		for(pos=0, gbc_it=gbc->begin(); gbc_it!=gbc->end(); ++pos, ++gbc_it) {
			boost::shared_ptr<Gem::Util::GBoolParameter> gbp_ptr = gde_.parameterSet_at<bool>(pos);
			*gbc_it = gbp_ptr->value();
		}

		//--------------------------------------------------------------------------------------------------------------------------------------
		// Retrieve our "char" collection items
		boost::shared_ptr<GCharObjectCollection> gcoc = pc_at<GCharObjectCollection>(3);

		// Get the size of the "foreign" container ...
		exchangeSize = gde_.size<char>();

		// ... and adjust the population size, as needed . This will erase items
		// or add copies of the second argument, as needed.
		gcoc->resize(exchangeSize, gchar_ptr_);

		// Now copy the items over
		GCharObjectCollection::iterator gcoc_it;
		for(pos=0, gcoc_it=gcoc->begin(); gcoc_it!=gcoc->end(); ++pos, ++gcoc_it) {
			if(!(*gcoc_it)->hasAdaptor()) {
				std::ostringstream error;
				error << "In GExternalEvaluator::readParametersFromFile(): Error!" << std::endl
						 << "GCharObject" << pos << " has no adaptor" << std::endl;

				throw geneva_error_condition(error.str());
			}

			boost::shared_ptr<Gem::Util::GCharParameter> gcp_ptr = gde_.parameterSet_at<char>(pos);
			**gcoc_it = gcp_ptr->value();
		}

		//--------------------------------------------------------------------------------------------------------------------------------------

		// Finally return the value of this data set (if any), or 0.
		if(gde_.hasValue()) {
			hasValue = true;
			return gde_.value();
		}
		return 0.;
	}

	/********************************************************************************************/
	/**
	 * Execute an external command, reacting to possible errors.
	 *
	 * @param command The command to be executed
	 */
	static void runCommand(const std::string& command) {
		int errorCode = system(command.c_str());
		if(errorCode) {
			std::ostringstream error;
			error << "In GExternalEvaluator::runCommand(): Error" << std::endl
				  << "Command: " << command << std::endl
				  << "Error code: " << errorCode << std::endl;
			throw(Gem::GenEvA::geneva_error_condition(error.str()));
		}
	}

	/********************************************************************************************/

	std::string program_; ///< The name of the external program to be executed
	std::string arguments_; ///< Any additional arguments to be handed to the external program
	boost::uint32_t nEvaluations_; ///< The number of data sets to be handed to the external program in one go
	dataExchangeMode exchangeMode_; ///< The desired method of data exchange
	bool maximize_; ///< indicates whether small values of this individual are better than large values
	std::string parameterFile_;

    boost::shared_ptr<GBoundedDouble> gdbl_ptr_; ///< A template for GBoundedDouble objects
    boost::shared_ptr<GBoundedInt32> glong_ptr_; ///< A template for GBoundedInt32 objects
    boost::shared_ptr<GChar> gchar_ptr_; ///< A template for collections of GChar objects

	Gem::Util::GDataExchange gde_; ///< takes care of the data exchange with external programs
};

} /* namespace GenEvA */
} /* namespace Gem */

#include <boost/serialization/export.hpp>
BOOST_CLASS_EXPORT(Gem::GenEvA::GExternalEvaluator)

#endif /* GEXTERNALEVALUATOR_HPP_ */
