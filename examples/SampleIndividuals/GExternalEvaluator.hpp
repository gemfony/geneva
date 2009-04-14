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
	void serialize(Archive & ar, const unsigned int version) {
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
	 * @param fileName The filename (including path) of the external program that should be executed
	 * @param arguments Additional user-defined arguments to be handed to the external program
	 */
	GExternalEvaluator(const std::string& fileName, const std::string& arguments, bool random=false)
		:fileName_(fileName),
		 arguments_(arguments),
		 nEvaluations_(1),
		 exchangeMode_(Gem::GenEvA::BINARYEXCHANGE),
		 maximize_(false),
		 parameterFile_("./parameterData")
	{
		// Tell the external program to send us a template with the structure of the individual
		std::string commandLine = fileName + "-t " +(random?std::string("-R"):std::string("")) + " -p " + parameterFile;
		if(!arguments.empty() && arguments != "empty") commandLine += arguments;


		// todo: get template from external program.
	}

	/********************************************************************************************/
	/**
	 * A standard copy constructor
	 */
	GExternalEvaluator(const GExternalEvaluator& cp)
		:GParameterSet(cp),
		 fileName_(cp.fileName_),
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
	 * 	@param fileName The filename (including path) of the external program that should be executed
	 * @param arguments Additional user-defined arguments to be handed to the external program
	 */
	static void initialize(const std::string& fileName, const std::string& arguments) {
		if(fileName.empty() || fileName == "empty") {
			std::ostringstream error;
			error << "In GExternalEvaluator::initialize : received bad filename" << std::endl;
			throw (Gem::GenEvA::geneva_error_condition(error.str()));
		}

		std::string commandLine = fileName + "-i ";
		if(!arguments.empty() && arguments != "empty") commandLine += arguments;

		system(commandLine.c_str());
	}

	/********************************************************************************************/
	/**
	 * Asks the external program to perform any necessary finalization work. It has been
	 * made a static member in order to centralize all external communication in this class.
	 *
	 * 	@param fileName The filename (including path) of the external program that should be executed
	 * @param arguments Additional user-defined arguments to be handed to the external program
	 */
	static void finalize(const std::string& fileName, const std::string& arguments) {
		if(fileName.empty() || fileName == "empty") {
			std::ostringstream error;
			error << "In GExternalEvaluator::initialize : received bad filename" << std::endl;
			throw (Gem::GenEvA::geneva_error_condition(error.str()));
		}

		std::string commandLine = fileName + "-f ";
		if(!arguments.empty() && arguments != "empty") commandLine += arguments;

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
		fileName_ = gei_load->fileName_;
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
		if(fileName_ != gev_load->fileName_) return false;
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
		if(fileName_ != gev_load->fileName_) return false;
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

		// Retrieve a pointer to the GDoubleCollection
		boost::shared_ptr<GDoubleCollection> gdc_load = pc_at<GDoubleCollection>(0);

		// Make the parameters known externally
		std::string resultFile = "bestParameterSet";
		std::ofstream parameters(resultFile.c_str());

		// First emit information about the number of double values
		std::size_t nDParm = gdc_load->size();
		parameters << nDParm << std::endl;

		// Then write out the actual parameter value
		GDoubleCollection::iterator it;
		for(it=gdc_load->begin(); it!=gdc_load->end(); ++it) {
			double current = *it;
			parameters << current << std::endl;
		}

		// Finally close the file
		parameters.close();

		// Check that we have a valid fileName_ ...
		if(fileName_ == "unknown" || fileName_.empty()) {
			std::ostringstream error;
			error << "In GExternalEvaluator::printResult(): Error!" << std::endl
				  << "Invalid file name \"" << fileName_ << "\"" << std::endl;

			throw geneva_error_condition(error.str());
		}

		// Assemble command line and run the external program
		if(arguments_ == "empty")
			commandLine = fileName_ + " -r -p " + resultFile;
		else
			commandLine = fileName_ + " " + arguments_ + " -r -p " + resultFile;

		// Initiate the result calculation
		system(commandLine.c_str()); // It is not clear whether this is thread-safe
	}

	/********************************************************************************************/

protected:
	/********************************************************************************************/
	/**
	 * The actual fitness calculation takes place here
	 *
	 * @return The value of this object
	 */
	virtual double fitnessCalculation(){
		double result = 0;
		std::string commandLine;

		// Retrieve a pointer to the GDoubleCollection. Compile in DEBUG mode in order to check this conversion
		boost::shared_ptr<GDoubleCollection> gdc_load = pc_at<GDoubleCollection>(0);

		// Make the parameters known externally
		std::ostringstream parFile;
		parFile << "parFile_" << this->getPopulationPosition();
		std::ofstream parameters(parFile.str().c_str());

		// First emit information about the number of double values
		std::size_t nDParm = gdc_load->size();
		parameters << nDParm << std::endl;

		// Then write out the actual parameter value
		GDoubleCollection::iterator it;
		for(it=gdc_load->begin(); it!=gdc_load->end(); ++it) {
			double current = *it;
			parameters << current << std::endl;
		}

		// Finally close the file
		parameters.close();

		// Check that we have a valid fileName_ ...
		if(fileName_ == "unknown" || fileName_.empty()) {
			std::ostringstream error;
			error << "In GExternalEvaluator::fitnessCalculation(): Error!" << std::endl
				  << "Invalid file name \"" << fileName_ << "\"" << std::endl;

			throw geneva_error_condition(error.str());
		}

		// Assemble command line and run the external program
		if(arguments_ == "empty")
			commandLine = fileName_ + " -p " + parFile.str();
		else
			commandLine = fileName_ + " " + arguments_ + " -p " + parFile.str();

		system(commandLine.c_str()); // It is not clear whether this is thread-safe

	    // ... then retrieve the output.
	    std::ifstream resultStream(parFile.str().c_str());
	    resultStream >> result;

	    // Finally close the file
	    resultStream.close();

	    // Let the audience know
		return result;
	}

private:
	/********************************************************************************************/
	/**
	 * The default constructor. Only needed for serialization purposes
	 */
	GExternalEvaluator()
		:fileName_("unknown"),
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
	 */
	void readFromFile(const std::string& fileName) {
		// Read in the data
		if(exchangeMode_ == BINARYEXCHANGE)	gde.readFromFile(fileName,  true);
		else gde.readFromFile(fileName, false); // TEXTEXCHANGE

		// Switch to the best data set in the collection
		if(maximize_) gde.switchToBestDataSet(false); // descending order
		else gde.switchToBestDataSet(true); // ascending order

		// Retrieve our current collection items, so we can load the exchange data back
		boost::shared_ptr<GBoundedDoubleCollection> gbdc = pc_at<GBoundedDoubleCollection>(0);
		// Get the sizes of both containers
		std::size_t exchangeSize = gde.size<double>();
		std::size_t collectionSize = gbdc->size();

		// Copy the data over. As we can not rely on the fact that
		// local and exchange-container have similar sizes, we need
		// to do some more work
		GBoundedDoubleCollection::iterator gbdc_it;
		if(exchangeSize == collectionSize) { // The most frequent case, likely
			// No need to check the upper bound of pos, as we already know that exchangeSize == collectionSize
			for(std::size_t pos=0, gbdc_it=gbdc->begin(); gbdc_it!=gbdc.end(); ++pos, ++gbdc_it) {
				boost::shared_ptr<GDoubleParameter> gdp_ptr = gdi.parameterSet_at<double>(pos);
				boost::shared_ptr<GBoundedDouble> p;
				if(gdp_ptr->hasBoundaries()) {
					p = boost::shared_ptr<GBoundedDouble>(new GBoundedDouble(gdp_ptr->value(), gdp_ptr->getLowerBoundary(), gdp_ptr->getUpperBoundary()));
				}
				else {

				}
			}
		}
		else if(exchangeSize > collectionSize) {

		}
		else if(exchangeSize < collectionSize) {

		}

		boost::shared_ptr<GBoundedInt32Collection> gbic = pc_at<GBoundedInt32Collection>(1);
		boost::shared_ptr<GBooleanCollection> gbc = pc_at<GBooleanCollection>(2);
		boost::shared_ptr<GCharObjectCollection> gcoc = pc_at<GCharObjectCollection>(3);

		// Finally assign the new value to this object and make sure it no longer appears to be "dirty"
	}

	/********************************************************************************************/

	std::string fileName_; ///< The name of the external program to be executed
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
