/**
 * @file GExternalEvaluator.hpp
 */

/* Copyright (C) 2009 Dr. Ruediger Berlich
 * Copyright (C) 2009 Forschungszentrum Karlsruhe GmbH
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
		ar & make_nvp("fileName_", fileName_);
		ar & make_nvp("arguments_", arguments_);
		ar & make_nvp("nEvaluations_", nEvaluations_);
		ar & make_nvp("exchangeMode_", exchangeMode_);
		ar & make_nvp("maximize_", maximize_);
		ar & make_nvp("parameterFile_", parameterFile_);
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
			                             bool random=false,
			                             const boost::shared_ptr<GAdaptorT<double> >& gdbl_ad_ptr = boost::shared_ptr<GAdaptorT<double> >((GAdaptorT<double> *)NULL),
			                             const boost::shared_ptr<GAdaptorT<boost::int32_t> >& glong_ad_ptr = boost::shared_ptr<GAdaptorT<boost::int32_t> >((GAdaptorT<boost::int32_t> *)NULL),
			                             const boost::shared_ptr<GAdaptorT<bool> >& gbool_ad_ptr = boost::shared_ptr<GAdaptorT<bool> >((GAdaptorT<bool> *)NULL),
			                             const boost::shared_ptr<GAdaptorT<char> >& gchar_ad_ptr = boost::shared_ptr<GAdaptorT<char> >((GAdaptorT<char> *)NULL)
		)
		:program_(program),
		 arguments_(arguments),
		 nEvaluations_(1),
		 exchangeMode_(Gem::GenEvA::BINARYEXCHANGE),
		 maximize_(false),
		 parameterFile_("./parameterData")
	{
		//-----------------------------------------------------------------------------------------------------------------------------
		// Fill the class with the required collections. The need to contain at least one item, to be used
		// as a template for the others.
		boost::shared_ptr<GBoundedDoubleCollection> gbdc_ptr(new GBoundedDoubleCollection());
		boost::shared_ptr<GBoundedDouble> gbd_ptr(new GBoundedDouble());
		if(gdbl_ad_ptr) gbd_ptr->push_back(gdbl_ad_ptr);
		else gbd_ptr->push_back(boost::shared_ptr<GDoubleGaussAdaptor>(new GDoubleGaussAdaptor())); // uses default values
		gbdc_ptr->push_back(gbd);
		this->push_back(gbdc_ptr);

		boost::shared_ptr<GBoundedInt32Collection> gbic_ptr(new GBoundedInt32Collection());
		boost::shared_ptr<GBoundedInt32> gbi_ptr(new GBoundedInt32());
		if(glong_ad_ptr) gbi_ptr->push_back(glong_ad_ptr);
		else gbi_ptr->push_back(boost::shared_ptr<GInt32FlipAdaptor>(new GInt32FlipAdaptor())); // uses default values
		gbic_ptr->push_back(gbi);
		this->push_back(gbic_ptr);

		boost::shared_ptr<GBooleanCollection> gbc_ptr(new GBooleanCollection());
		if(gbool_ad_ptr) gbc_ptr->push_back(gdbl_ad_ptr);
		else gbc_ptr->push_back(boost::shared_ptr<GBooleanAdaptor>(new GBooleanAdaptor())); // uses default values
		this->push_back(gbc_ptr);

		boost::shared_ptr<GCharObjectCollection> gcoc_ptr(new GCharObjectCollection());
		boost::shared_ptr<GChar> gc_ptr(new GChar());
		if(gchar_ad_ptr) gc_ptr->push_back(gchar_ad_ptr);
		else gc_ptr->push_back(boost::shared_ptr<GCharFlipAdaptor>(new GCharFlipAdaptor())); // uses default values
		gcoc_ptr->push_back(gc_ptr);
		this->push_back(gcoc_ptr);

		//-----------------------------------------------------------------------------------------------------------------------------
		// Tell the external program to send us a template with the structure of the individual
		std::string commandLine = fileName + " -t " +(random?std::string(" -R"):std::string("")) + " -p " + parameterFile_;
		if(!arguments.empty() && arguments != "empty") commandLine += (" " + arguments_);
		system(commandLine.c_str());

		//-----------------------------------------------------------------------------------------------------------------------------
		// Finally fill this class up with the external template data
		 this->readFromFile(parameterFile_);
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
	{ /* nothing */	}

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
		if(program.empty() || program == "empty") {
			std::ostringstream error;
			error << "In GExternalEvaluator::initialize : received bad program name \"" << program << "\"." << std::endl;
			throw (Gem::GenEvA::geneva_error_condition(error.str()));
		}

		std::string commandLine = program + " -i ";
		if(!arguments.empty() && arguments != "empty") commandLine += (" " + arguments);

		system(commandLine.c_str());
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
		if(program.empty() || program == "empty") {
			std::ostringstream error;
			error << "In GExternalEvaluator::initialize : received bad program name \"" << program << "\"." << std::endl;
			throw (Gem::GenEvA::geneva_error_condition(error.str()));
		}

		std::string commandLine = program + " -f ";
		if(!arguments.empty() && arguments != "empty") commandLine += (" " + arguments);

		system(commandLine.c_str());
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
		const GExternalEvaluator *gei_load = conversion_cast(cp, this);

		// First load the data of our parent class ...
		GParameterSet::load(cp);

		// ... and then our own
		program_ = gei_load->program_;
		arguments_ = gei_load->arguments_;
		nEvaluations_ = gei_load->nEvaluations_;
		exchangeMode_ = gei_load->exchangeMode_;
		maximize_ = gei_load->maximize_;
		parameterFile_ = gei_load->parameterFile_;
	}

	/********************************************************************************************/
	/**
	 * Checks for equality with another GExternalEvaluator object
	 *
	 * @param  cp A constant reference to another GExternalEvaluator object
	 * @return A boolean indicating whether both objects are equal
	 */
	bool operator==(const GExternalEvaluator& cp) const {
		return GExternalEvaluator::isEqualTo(cp);
	}

	/********************************************************************************************/
	/**
	 * Checks for inequality with another GExternalEvaluator object
	 *
	 * @param  cp A constant reference to another GExternalEvaluator object
	 * @return A boolean indicating whether both objects are inequal
	 */
	bool operator!=(const GExternalEvaluator& cp) const {
		return !GExternalEvaluator::isEqualTo(cp);
	}

	/********************************************************************************************/
	/**
	 * Checks for equality with another GExternalEvaluator object..
	 *
	 * @param  cp A constant reference to another GExternalEvaluator object
	 * @return A boolean indicating whether both objects are equal
	 */
	virtual bool isEqualTo(const GObject& cp) const {
		// Check that we are indeed dealing with a GNumCollectionT reference
		const GExternalEvaluator *gev_load = GObject::conversion_cast(&cp,  this);

		// Check equality of the parent class
		if(GParameterSet.isNotEqualTo(*gev_load)) return false;

		// Then check our local data
		if(program_ != gev_load->program_) return false;
		if(arguments_ != gev_load->arguments_) return false;
		if(nEvaluations_ != gev_load->nEvaluations_) return false;
		if(exchangeMode_ != gev_load->exchangeMode_) return false;
		if(maximize_ != gev_load->maximize_) return false;
		if(parameterFile_ != gev_load->parameterFile_) return false;

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
	virtual bool isSimilarTo(const GObject& cp, const double& limit) const {
		// Check that we are indeed dealing with a GNumCollectionT reference
		const GExternalEvaluator *gev_load = GObject::conversion_cast(&cp,  this);

		// Check similarity of the parent class
		if(GParameterSet.isNotSimilarTo(*gev_load, limit)) return false;

		// Then check our local data
		if(program_ != gev_load->program_) return false;
		if(arguments_ != gev_load->arguments_) return false;
		if(nEvaluations_ != gev_load->nEvaluations_) return false;
		if(exchangeMode_ != gev_load->exchangeMode_) return false;
		if(maximize_ != gev_load->maximize_) return false;
		if(parameterFile_ != gev_load->parameterFile_) return false;

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
	void setMaximize(const bool& maximize=true)	{
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
	void printResult() {
		std::string commandLine;

		// Make the parameters known externally
		std::string resultFile = "bestParameterSet";

		// Emit our data
		this->writeToFile(resultFile);

		// Check that we have a valid program_ ...
		if(program_ == "empty" || program_.empty()) {
			std::ostringstream error;
			error << "In GExternalEvaluator::printResult(): Error!" << std::endl
				      << "Invalid program name \"" << program_ << "\"" << std::endl;

			throw geneva_error_condition(error.str());
		}

		// Assemble command line and run the external program
		if(arguments_ == "empty" || arguments_.empty())
			commandLine = program_ + " -r -p " + resultFile;
		else
			commandLine = program_ + " -r -p " + resultFile + " " + arguments_;

		// Initiate the result calculation
		system(commandLine.c_str());

		// Clean up - remove the result file
		system(("rm -f " + resultFile).c_str());
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
		// Make the parameters known externally
		std::string parFile = parameterFile_ + boost::lexical_cast<std::string>( this->getPopulationPosition());

		// Write out the required data
		this->writeToFile(parFile);

		// Check that we have a valid program_ name ...
		if(program_ == "empty" || program_.empty()) {
			std::ostringstream error;
			error << "In GExternalEvaluator::fitnessCalculation(): Error!" << std::endl
				     << "Invalid program name \"" << program_ << "\"" << std::endl;

			throw geneva_error_condition(error.str());
		}

		// Assemble command line and run the external program
		std::string commandLine;
		if(arguments_ == "empty" || arguments_.empty())
			commandLine = program_ + " -p " + parFile.str();
		else
			commandLine = program_ + " " + arguments_ + " -p " + parFile.str();

		system(commandLine.c_str()); // It is not clear whether this is thread-safe

		bool hasValue = false;
		double result = this->readFromFile(parFile, hasValue);

		// Check whether a result value was received
		if(!hasValue) {
			std::ostringstream error;
			error << "In GExternalEvaluator::fitnessCalculation(): Error!" << std::endl
				     << "Received no value from the external calculation" << std::endl;

			throw geneva_error_condition(error.str());
		}

		// Clean up - remove the parameter file
		system(("rm -f " + parFile).c_str());

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
		 maximize_(true)
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
	void writeToFile(const std::string& fileName) {
		// Make sure we are dealing with a clean exchange module
		gde.resetAll();

		// Create nEvaluations_ data sets from this object
		for(boost::uint32_t i=0; i<nEvaluations_; i++) {
			boost::shared_ptr<GExternalEvaluator> p; // the additional object will vanish at the end of the loop

			// More than one data set requested for evaluation ?
			if(i>0) {
				// Switch to a new page in the GDataExchange module
				gde.newDataSet();

				// Create a copy of this object and mutate it
				p = boost::shared_ptr<GExternalEvaluation>(this->clone());
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
				boost::shared_ptr<GDoubleParameter> dpar(new GDoubleParameter((*gbdc_it)->value(), (*gbdc_it)->getLowerBoundary(), (*gbdc_it)->getUpperBoundary()));
				gde.append(dpar);
			}


			boost::shared_ptr<GBoundedInt32Collection> gbic;
			if(i==0) {
				gbic = pc_at<GBoundedInt32Collection>(1);
			}
			else {
				gbdc = p->pc_at<GBoundedInt32Collection>(1);
			}
			GBoundedInt32Collection::iterator gbic_it;
			for(gbic_it=gbic->begin(); gbic_it!=gbic->end(); ++gbic_it) {
				boost::shared_ptr<GLongParameter> ipar(new GLongParameter((*gbic_it)->value(), (*gbic_it)->getLowerBoundary(), (*gbic_it)->getUpperBoundary()));
				gde.append(ipar);
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
				boost::shared_ptr<GBoolParameter> bpar(new GBoolParameter(*gbc_it)); // no boundaries for booleans
				gde.append(bpar);
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
				boost::shared_ptr<GCharParameter> cpar(new GCharParameter((*gcoc_it)->value())); // no boundaries for characters for now
				gde.append(cpar);
			}
		}

		// At this point all necessary data should have been stored in the GDataExchange module. We can now write it to file.
		if(exchangeMode_ == BINARYEXCHANGE)	gde.writeToFile(fileName, true);
		else gde.writeToFile(fileName, false); // TEXTEXCHANGE
	}

	/********************************************************************************************/
	/**
	 * Reads the class'es data from a file and loads the best data set into the local
	 * structures.
	 *
	 * @param fileName The name of the file to read from
	 * @return The value of the data set in the file (if available), or 0.
	 */
	double readFromFile(const std::string& fileName, bool& hasValue=false) {
		hasValue = false;

		// Make sure gde is empty
		gde.resetAll();

		// Read in the data
		if(exchangeMode_ == BINARYEXCHANGE)	gde.readFromFile(fileName,  true);
		else gde.readFromFile(fileName, false); // TEXTEXCHANGE

		// Switch to the best data set in the collection
		if(maximize_) gde.switchToBestDataSet(false); // descending order
		else gde.switchToBestDataSet(true); // ascending order

		//--------------------------------------------------------------------------------------------------------------------------------------
		// Retrieve our "double" collection items
		boost::shared_ptr<GBoundedDoubleCollection> gbdc = pc_at<GBoundedDoubleCollection>(0);

		// Make sure we have (template-)items in the local collection. This should have been
		// taken care of in the main constructor
		if(gbdc->empty()) {
			std::ostringstream error;
			error << "In GExternalEvaluator::readFromFile(): Error!" << std::endl
					 << "GBoundedDoubleCollection is empty," << std::endl;

			throw geneva_error_condition(error.str());
		}

		// Get the size of the  "foreign" container ...
		std::size_t exchangeSize = gde.size<double>();

		// ... and adjust the population size, as needed . This will erase items
		// or add copies of the second argument, as needed.
		gbdc->resize(exchangeSize, gbdc->at(0));

		// Now copy the items over
		GBoundedDoubleCollection::iterator gbdc_it;
		for(std::size_t pos=0, gbdc_it=gbdc->begin(); gbdc_it!=gbdc->end(); ++pos, ++gbdc_it) {
			if(!(*gbdc_it)->hasAdaptors()) {
				std::ostringstream error;
				error << "In GExternalEvaluator::readFromFile(): Error!" << std::endl
						 << "GBoundedDouble item " << pos << " has no adaptor" << std::endl;

				throw geneva_error_condition(error.str());
			}

			boost::shared_ptr<GDoubleParameter> gdp_ptr = gde.parameterSet_at<double>(pos);
			if(gdp_ptr->hasBoundaries()) {
				(*gbdc_it)->setExternalValue(gdp_ptr->value());
				(*gbdc_it)->setBoundaries(gdp_ptr->getLowerBoundary(), gdp_ptr->getUpperBoundary());
			}
			else {
				(*gbdc_it)->setExternalValue(gdp_ptr->value());
				(*gbdc_it)->resetBoundaries();
			}
		}

		//--------------------------------------------------------------------------------------------------------------------------------------
		// Retrieve our "long" collection items
		boost::shared_ptr<GBoundedInt32Collection> gbic = pc_at<GBoundedInt32Collection>(1);

		// Make sure we have (template-)items in the local collection. This should have been
		// taken care of in the main constructor
		if(gbic->empty()) {
			std::ostringstream error;
			error << "In GExternalEvaluator::readFromFile(): Error!" << std::endl
					 << "GBoundedInt32Collection is empty," << std::endl;

			throw geneva_error_condition(error.str());
		}

		// Make sure we have (template-)items in the local collection
		if(gbic->empty()) {
			boost::shared_ptr<GBoundedInt32> gbi_templ(new GBoundedInt32());
			boost::shared_ptr<GInt32FlipAdaptor> gifa_templ(new GInt32FlipAdaptor());
			gbi_templ->addAdaptor(gifa_templ);
			gbic->push_back(gbi_templ);
		}

		// Get the size of the "foreign" container ...
		exchangeSize = gde.size<boost::int32_t>();

		// ... and adjust the population size, as needed . This will erase items
		// or add copies of the second argument, as needed.
		gbic->resize(exchangeSize, gbic->at(0));

		// Now copy the items over
		GBoundedInt32Collection::iterator gbic_it;
		for(std::size_t pos=0, gbic_it=gbic->begin(); gbic_it!=gbic->end(); ++pos, ++gbic_it) {
			if(!(*gbic_it)->hasAdaptors()) {
				std::ostringstream error;
				error << "In GExternalEvaluator::readFromFile(): Error!" << std::endl
						 << "GBoundedInt32 item " << pos << " has no adaptor" << std::endl;

				throw geneva_error_condition(error.str());
			}

			boost::shared_ptr<GLongParameter> glp_ptr = gde.parameterSet_at<boost::int32_t>(pos);
			if(glp_ptr->hasBoundaries()) {
				(*gbic_it)->setExternalValue(glp_ptr->value());
				(*gbic_it)->setBoundaries(glp_ptr->getLowerBoundary(), glp_ptr->getUpperBoundary());
			}
			else {
				(*gbic_it)->setExternalValue(glp_ptr->value());
				(*gbic_it)->resetBoundaries();
			}
		}

		//--------------------------------------------------------------------------------------------------------------------------------------
		// Retrieve our "bool" collection items
		boost::shared_ptr<GBooleanCollection> gbc = pc_at<GBooleanCollection>(2);

		if(gbc->empty() || !gbc->hasAdaptors()) {
			std::ostringstream error;
			error << "In GExternalEvaluator::readFromFile(): Error!" << std::endl
					  << "GBooleanCollection is empty or has no adaptor" << std::endl
					  << gbc->size() << " " << gbc->numberOfAdaptors() << std::endl;

			throw geneva_error_condition(error.str());
		}

		// Get the size of the "foreign" container ...
		exchangeSize = gde.size<bool>();

		// ... and adjust the population size, as needed . This will erase items
		// or add copies of the second argument, as needed.
		gbc->resize(exchangeSize, false);

		// Now copy the items over
		GBooleanCollection::iterator gbc_it;
		for(std::size_t pos=0, gbc_it=gbc->begin(); gbc_it!=gbc->end(); ++pos, ++gbc_it) {
			boost::shared_ptr<GBoolParameter> gbp_ptr = gde.parameterSet_at<bool>(pos);
			*gbc_it = gbp_ptr->value();
		}

		//--------------------------------------------------------------------------------------------------------------------------------------
		// Retrieve our "char" collection items
		boost::shared_ptr<GCharObjectCollection> gcoc = pc_at<GCharObjectCollection>(3);

		// Make sure we have (template-)items in the local collection. This should have been
		// taken care of in the main constructor
		if(gcoc->empty()) {
			std::ostringstream error;
			error << "In GExternalEvaluator::readFromFile(): Error!" << std::endl
					  << "GCharObjectCollection is empty," << std::endl;

			throw geneva_error_condition(error.str());
		}

		// Get the size of the "foreign" container ...
		exchangeSize = gde.size<char>();

		// ... and adjust the population size, as needed . This will erase items
		// or add copies of the second argument, as needed.
		gcoc->resize(exchangeSize, gcoc->at(0));

		// Now copy the items over
		GCharCollection::iterator gcoc_it;
		for(std::size_t pos=0, gcoc_it=gcoc->begin(); gcoc_it!=gcoc->end(); ++pos, ++gcoc_it) {
			if(!(*gcoc_it)->hasAdaptors()) {
				std::ostringstream error;
				error << "In GExternalEvaluator::readFromFile(): Error!" << std::endl
						 << "GCharObject" << pos << " has no adaptor" << std::endl;

				throw geneva_error_condition(error.str());
			}

			boost::shared_ptr<GCharParameter> gcp_ptr = gde.parameterSet_at<char>(pos);
			*gcoc_it = gcp_ptr->value();
		}

		//--------------------------------------------------------------------------------------------------------------------------------------

		// Finally return the value of this data set (if any), or 0.
		if(gde.hasValue()) {
			hasValue = true;
			return gde.value();
		}
		return 0.;
	}

	/********************************************************************************************/

	std::string program_; ///< The name of the external program to be executed
	std::string arguments_; ///< Any additional arguments to be handed to the external program
	boost::uint32_t nEvaluations_; ///< The number of data sets to be handed to the external program in one go
	dataExchangeMode exchangeMode_; ///< The desired method of data exchange
	bool maximize_; ///< indicates whether small values of this individual are better than large values
	std::string parameterFile_;

	Gem::Util::GDataExchange gde; ///< takes care of the data exchange with external programs
};

} /* namespace GenEvA */
} /* namespace Gem */

#include <boost/serialization/export.hpp>
BOOST_CLASS_EXPORT(Gem::GenEvA::GExternalEvaluator)

#endif /* GEXTERNALEVALUATOR_HPP_ */
