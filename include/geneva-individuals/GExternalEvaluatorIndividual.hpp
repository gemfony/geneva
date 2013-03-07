/**
 * @file GExternalEvaluatorIndividual.hpp
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
#include "common/GCommonEnums.hpp"
#include "common/GHelperFunctions.hpp"
#include "common/GNumericParameterT.hpp"
#include "geneva/GBooleanAdaptor.hpp"
#include "geneva/GBooleanCollection.hpp"
#include "geneva/GConstrainedDoubleObjectCollection.hpp"
#include "geneva/GConstrainedInt32ObjectCollection.hpp"
#include "geneva/GDoubleCollection.hpp"
#include "geneva/GDoubleGaussAdaptor.hpp"
#include "geneva/GInt32FlipAdaptor.hpp"
#include "geneva/GParameterSet.hpp"

// aliases for ease of use
namespace bf = boost::filesystem;
namespace pt = boost::property_tree;

namespace Gem {
namespace Geneva {

/************************************************************************************************/
/**
 * This individual calls an external program to evaluate a given set of parameters.
 * Data exchange happens partially through the GNumericParameterT class. The
 * structure of the individual is determined from information given by the external
 * program. Currently double-, bool- and boost::int32_t values are supported.
 *
 * External programs should understand the following command line arguments with
 * obvious meanings
 * --init                                              # Called once before the optimization starts
 * --finalize                                          # Called once after the optimization ends
 * --setup="setupFile.txt"                             # Asks the external program to emit setup information to the file setupFile.txt
 * --evaluate="parameters.xml" --result="outFile.xml"  # Asks the external program to perform evaluation of the parameters in file parameters.xml and to store the results in the file outfile.xml
 * --evaluate="parameters.xml" --archive               # Asks the external program to perform evaluation of the parameters in file parameters.xml and to perform any archiving work needed
 */
class GExternalEvaluatorIndividual :public GParameterSet
 {
	///////////////////////////////////////////////////////////////////////

   friend class boost::serialization::access;

	template<class Archive>
	void serialize(Archive & ar, const unsigned int) {
		using boost::serialization::make_nvp;

		ar
		& make_nvp("GParameterSet", boost::serialization::base_object<GParameterSet>(*this))
		& make_nvp("programName_", programName_)
		& make_nvp("parameterFileBaseName_", parameterFileBaseName_);
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
	 */
	GExternalEvaluatorIndividual(
	      const std::string& programName
   )
	  : programName_(programName)
	  , parameterFileBaseName_("./parameterData")
	  {
		//-----------------------------------------------------------------------------------------------
		// Create the required, empty collections.
		boost::shared_ptr<GBoundedDoubleCollection> gbdc_ptr(new GBoundedDoubleCollection());
		boost::shared_ptr<GBoundedInt32Collection> gbic_ptr(new GBoundedInt32Collection());
		boost::shared_ptr<GBooleanCollection> gbc_ptr(new GBooleanCollection());

		// Set up the local adaptor templates and collection items
		//-----------------------------------------------------------------------------------------------
		gdbl_ptr_=boost::shared_ptr<GBoundedDouble>(new GBoundedDouble());
		if(gdbl_ad_ptr) {
			gdbl_ptr_->addAdaptor(gdbl_ad_ptr->GObject::clone<GAdaptorT<double> >());
		} else {
			gdbl_ptr_->addAdaptor(boost::shared_ptr<GAdaptorT<double> >(new GDoubleGaussAdaptor())); // uses default values
		}

		//-----------------------------------------------------------------------------------------------
		glong_ptr_=boost::shared_ptr<GBoundedInt32>(new GBoundedInt32());
		if(glong_ad_ptr) {
			glong_ptr_->addAdaptor(glong_ad_ptr->GObject::clone<GAdaptorT<boost::int32_t> >());
		} else {
			glong_ptr_->addAdaptor(boost::shared_ptr<GAdaptorT<boost::int32_t> >(new GInt32FlipAdaptor())); // uses default values
		}

		//-----------------------------------------------------------------------------------------------
		// GBooleanCollection is special in that it always directly contains adaptors
		if(gbool_ad_ptr) {
			gbc_ptr->addAdaptor(gbool_ad_ptr->GObject::clone<GAdaptorT<bool> >());
		} else {
			gbc_ptr->addAdaptor(boost::shared_ptr<GAdaptorT<bool> >(new GBooleanAdaptor())); // uses default values
		}

		//-----------------------------------------------------------------------------------------------

		// Add the collections to the class
		this->push_back(gbdc_ptr);
		this->push_back(gbic_ptr);
		this->push_back(gbc_ptr);

		//-----------------------------------------------------------------------------------------------------------------------------
		// Tell the external program to send us a template with the structure of the individual

		if(programName_.empty() || programName_ == "empty" || programName_ == "unknown") {
			std::ostringstream error;
			error << "In GExternalEvaluatorIndividual::GExternalEvaluatorIndividual(...) : received bad program name \"" << programName_ << "\"." << std::endl;
			throw (Gem::Common::gemfony_error_condition(error.str()));
		}

		std::string commandLine;
		if(exchangeMode_ == Gem::Geneva::BINARYEXCHANGE) {
			commandLine = programName_ + " -m 0 -t " +(random?std::string(" -R"):std::string("")) + " -p " + parameterFileBaseName_;
		}
		else {
			commandLine = programName_ + " -m 1 -t " +(random?std::string(" -R"):std::string("")) + " -p " + parameterFileBaseName_;
		}

		if(!arguments.empty() && arguments != "empty") commandLine += (" " + arguments_);

#ifdef PRINTCOMMANDLINE
		std::cout << "Requesting template with commandLine = \"" << commandLine << "\" ...";
#endif /* PRINTCOMMANDLINE */
		Gem::Common::runExternalCommand(commandLine);
#ifdef PRINTCOMMANDLINE
		std::cout << "... done." << std::endl;
#endif /* PRINTCOMMANDLINE */


		//-----------------------------------------------------------------------------------------------------------------------------
		// Finally fill this class up with the external template data. Make sure the data doesn't get sorted
		bool hasValue;
		this->readParametersFromFile(parameterFileBaseName_, hasValue, false);

		// Erase the parameter file - not needed anymore.
		bf::path p(parameterFileBaseName_.c_str());
#ifdef DEBUG
		if(!bf::exists(p)) {
			std::ostringstream error;
			error << "In GExternalEvaluatorIndividual constructor: Error!" << std::endl
					<< "Tried to erase non-existant parameter file " << parameterFileBaseName_ << std::endl;
			throw(Gem::Common::gemfony_error_condition(error.str()));
		}
#endif /* DEBUG */
		bf::remove(p);
	  }

	/********************************************************************************************/
	/**
	 * A standard copy constructor.
	 */
	GExternalEvaluatorIndividual(const GExternalEvaluatorIndividual& cp)
	: GParameterSet(cp) // copies all local collections
	  , programName_(cp.programName_)
	  , arguments_(cp.arguments_)
	  , nEvaluations_(cp.nEvaluations_)
	  , exchangeMode_(cp.exchangeMode_)
	  , maximize_(cp.maximize_)
	  , parameterFileBaseName_(cp.parameterFileBaseName_)
	  {
		gdbl_ptr_ = cp.gdbl_ptr_->GObject::clone<GBoundedDouble>();
		glong_ptr_ = cp.glong_ptr_->GObject::clone<GBoundedInt32>();
	  }

	/********************************************************************************************/
	/**
	 * The standard destructor
	 */
	~GExternalEvaluatorIndividual()
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
			error << "In GExternalEvaluatorIndividual::initialize : received bad program name \"" << program << "\"." << std::endl;
			throw (Gem::Common::gemfony_error_condition(error.str()));
		}

		std::string commandLine = program + " -i ";
		if(!arguments.empty() && arguments != "empty") commandLine += (" " + arguments);

#ifdef PRINTCOMMANDLINE
		std::cout << "Initializing with command line = \"" << commandLine << "\" ...";
#endif /* PRINTCOMMANDLINE */
		Gem::Common::runExternalCommand(commandLine);
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
			error << "In GExternalEvaluatorIndividual::initialize : received bad program name \"" << program << "\"." << std::endl;
			throw (Gem::Common::gemfony_error_condition(error.str()));
		}

		std::string commandLine = program + " -f ";
		if(!arguments.empty() && arguments != "empty") commandLine += (" " + arguments);

#ifdef PRINTCOMMANDLINE
		std::cout << "Finalizing with command line = \"" << commandLine << "\" ...";
#endif /* PRINTCOMMANDLINE */
		Gem::Common::runExternalCommand(commandLine);
#ifdef PRINTCOMMANDLINE
		std::cout << " ... done." << std::endl;
#endif /* PRINTCOMMANDLINE */
	}

	/********************************************************************************************/
	/**
	 * A standard assignment operator
	 */
	const GExternalEvaluatorIndividual& operator=(const GExternalEvaluatorIndividual& cp){
		GExternalEvaluatorIndividual::load_(&cp);
		return *this;
	}

	/********************************************************************************************/
	/**
	 * Checks for equality with another GExternalEvaluatorIndividual object
	 *
	 * @param  cp A constant reference to another GExternalEvaluatorIndividual object
	 * @return A boolean indicating whether both objects are equal
	 */
	bool operator==(const GExternalEvaluatorIndividual& cp) const {
		using namespace Gem::Common;
		// Means: The expectation of equality was fulfilled, if no error text was emitted (which converts to "true")
		return !checkRelationshipWith(cp, CE_EQUALITY, 0.,"GExternalEvaluatorIndividual::operator==","cp", CE_SILENT);
	}

	/********************************************************************************************/
	/**
	 * Checks for inequality with another GExternalEvaluatorIndividual object
	 *
	 * @param  cp A constant reference to another GExternalEvaluatorIndividual object
	 * @return A boolean indicating whether both objects are inequal
	 */
	bool operator!=(const GExternalEvaluatorIndividual& cp) const {
		using namespace Gem::Common;
		// Means: The expectation of inequality was fulfilled, if no error text was emitted (which converts to "true")
		return !checkRelationshipWith(cp, CE_INEQUALITY, 0.,"GExternalEvaluatorIndividual::operator!=","cp", CE_SILENT);
	}

	/********************************************************************************************/
	/**
	 * Checks whether a given expectation for the relationship between this object and another object
	 * is fulfilled.
	 *
	 * @param cp A constant reference to another object, camouflaged as a GObject
	 * @param e The expected outcome of the comparison
	 * @param limit The maximum deviation for floating point values (important for similarity checks)
	 * @param caller An identifier for the calling entity
	 * @param y_name An identifier for the object that should be compared to this one
	 * @param withMessages Whether or not information should be emitted in case of deviations from the expected outcome
	 * @return A boost::optional<std::string> object that holds a descriptive string if expectations were not met
	 */
	boost::optional<std::string> checkRelationshipWith(const GObject& cp,
			const Gem::Common::expectation& e,
			const double& limit,
			const std::string& caller,
			const std::string& y_name,
			const bool& withMessages) const
			{
		using namespace Gem::Common;

		// Check that we are indeed dealing with a GParamterBase reference
		const GExternalEvaluatorIndividual *p_load = conversion_cast<GExternalEvaluatorIndividual>(&cp);

		// Will hold possible deviations from the expectation, including explanations
		std::vector<boost::optional<std::string> > deviations;

		// Check our parent class'es data ...
		deviations.push_back(GParameterSet::checkRelationshipWith(cp, e, limit, "GExternalEvaluatorIndividual", y_name, withMessages));

		// ... and then our local data
		deviations.push_back(checkExpectation(withMessages, "GExternalEvaluatorIndividual", programName_, p_load->programName_, "programName_", "p_load->programName_", e , limit));
		deviations.push_back(checkExpectation(withMessages, "GExternalEvaluatorIndividual", parameterFileBaseName_, p_load->parameterFileBaseName_, "parameterFileBaseName_", "p_load->parameterFileBaseName_", e , limit));

		return evaluateDiscrepancies("GExternalEvaluatorIndividual", caller, deviations, e);
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
			error << "In GExternalEvaluatorIndividual::setExchangeFileName(): Error!" << std::endl
					<< "Invalid file name \"" << parameterFile << "\"" << std::endl;
			throw Gem::Common::gemfony_error_condition(error.str());
		}

		parameterFileBaseName_ = parameterFile;
	}

	/********************************************************************************************/
	/**
	 * Retrieves the current value of the parameterFileBaseName_ variable.
	 *
	 * @return The current base name of the exchange file
	 */
	std::string getExchangeFileName() const {
		return parameterFileBaseName_;
	}

 protected:
	/********************************************************************************************/
	/**
	 * Loads the data of another GExternalEvaluatorIndividual, camouflaged as a GObject
	 *
	 * @param cp A copy of another GExternalEvaluatorIndividual, camouflaged as a GObject
	 */
	virtual void load_(const GObject* cp){
		// Convert to a local representation
		const GExternalEvaluatorIndividual *p_load = conversion_cast<GExternalEvaluatorIndividual>(cp);

		// First load the data of our parent class ...
		GParameterSet::load_(cp);

		// ... and then our own
		programName_ = p_load->programName_;
		parameterFileBaseName_ = p_load->parameterFileBaseName_;
	}

	/********************************************************************************************/
	/**
	 * Creates a deep clone of this object
	 *
	 * @return A deep clone of this object, camouflaged as a GObject
	 */
	virtual GObject* clone_() const{
		return new GExternalEvaluatorIndividual(*this);
	}

	/********************************************************************************************/
	/**
	 * The actual fitness calculation takes place in an external program. Here we just
	 * write a file with the required parameters to disk and execute the program.
	 *
	 * @return The value of this object
	 */
	virtual double fitnessCalculation() {
		// Check that we have a valid programName_ name ...
		if(programName_.empty() || programName_ == "empty" || programName_ == "unknown") {
			std::ostringstream error;
			error << "In GExternalEvaluatorIndividual::fitnessCalculation(): Error!" << std::endl
					<< "Invalid program name \"" << programName_ << "\"" << std::endl;

			throw Gem::Common::gemfony_error_condition(error.str());
		}

		// Make the parameters known externally
		std::string parFile = parameterFileBaseName_ + "_" + boost::lexical_cast<std::string>(getParentAlgIteration());

		if(this->getPersonality() == EA) parFile +=  ("_" +  boost::lexical_cast<std::string>( this->getEAPersonalityTraits()->getPopulationPosition()));

		// Write out the required data
		this->writeParametersToFile(parFile);

		// Assemble command line and run the external program
		std::string commandLine;

		if(exchangeMode_ == Gem::Geneva::BINARYEXCHANGE)
			commandLine = programName_ + " -m 0  -p " + parFile;
		else
			commandLine = programName_ + " -m 1  -p " + parFile;;


		if(arguments_ != "empty" && !arguments_.empty())
			commandLine += (" " + arguments_);

#ifdef PRINTCOMMANDLINE
		std::cout << "Calculating result with command line = \"" << commandLine << "\" ...";
#endif /* PRINTCOMMANDLINE */
		Gem::Common::runExternalCommand(commandLine); // It is not clear whether this is thread-safe
#ifdef PRINTCOMMANDLINE
		std::cout << " ... done." << std::endl;
#endif /* PRINTCOMMANDLINE */

		bool hasValue = false;
		double result = this->readParametersFromFile(parFile, hasValue, true);

		// Check whether a result value was received
		if(!hasValue) {
			std::ostringstream error;
			error << "In GExternalEvaluatorIndividual::fitnessCalculation(): Error!" << std::endl
					<< "Received no value from the external calculation" << std::endl;

			throw Gem::Common::gemfony_error_condition(error.str());
		}

		// Erase the parameter file - not needed anymore.
		bf::path p(parFile.c_str());
#ifdef DEBUG
		if(!bf::exists(p)) {
			std::ostringstream error;
			error << "In GExternalEvaluatorIndividual::fitnessCalculation(): Error!" << std::endl
					<< "Tried to erase non-existent parameter file " << parFile << std::endl;
			throw(Gem::Common::gemfony_error_condition(error.str()));
		}
#endif /* DEBUG */
		bf::remove(p);

		// Let the audience know
		return result;
	}

 private:
	/********************************************************************************************/
	/**
	 * The default constructor. Only needed for serialization purposes
	 */
	GExternalEvaluatorIndividual()
	   : programName_("unknown")
	   , parameterFileBaseName_("empty")
	 { /* nothing */ }

	/********************************************************************************************/
	/**
	 * Writes the class'es data to a file. Note that, if the nEvaluations_ variable is set to a
	 * value higher than 1, this function will create multiple, adapted copies of this
	 * individual and add them to the output file. The goal is to allow external programs
	 * to perform more than one evaluation in sequence, so the overhead incurred through
	 * the frequent disc i/o is reduced.
	 *
	 * The structure of this individual is:
	 * GBoundedDoubleCollection
	 * GBoundedInt32Collection
	 * GBooleanCollection
	 *
	 * @param fileName The name of the file to write to
	 */
	void writeParametersToFile(const std::string& fileName) {
		// Make sure we are dealing with a clean exchange module
		gde_.resetAll();

		// Create nEvaluations_ data sets from this object
		for(boost::uint32_t i=0; i<nEvaluations_; i++) {
			boost::shared_ptr<GExternalEvaluatorIndividual> p; // the additional object will vanish at the end of the loop

			// More than one data set requested for evaluation ?
			if(i>0) {
				// Switch to a new page in the GDataExchange module
				gde_.newDataSet();

				// Create a copy of this object and adapt it
				p = this->clone<GExternalEvaluatorIndividual>();
				p->setAllowLazyEvaluation(true); // Prevent evaluation upon adaption
				p->adapt(); // Make sure we do not evaluate the same parameter set
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
			  boost::shared_ptr<Gem::Dataexchange::GDoubleParameter> dpar(new Gem::Dataxchange::GDoubleParameter((*gbdc_it)->value(), (*gbdc_it)->getLowerBoundary(), (*gbdc_it)->getUpperBoundary()));
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
				boost::shared_ptr<Gem::Dataexchange::GLongParameter> ipar(new Gem::Dataexchange::GLongParameter((*gbic_it)->value(), (*gbic_it)->getLowerBoundary(), (*gbic_it)->getUpperBoundary()));
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
				boost::shared_ptr<Gem::Dataexchange::GBoolParameter> bpar(new Gem::Dataexchange::GBoolParameter(*gbc_it)); // no boundaries for booleans
				gde_.append(bpar);
			}
		}

		// At this point all necessary data should have been stored in the GDataExchange module. We can now write it to file.
		if(exchangeMode_ == Gem::Geneva::BINARYEXCHANGE)	gde_.writeToFile(fileName, true);
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
			boost::shared_ptr<Gem::Dataexchange::GDoubleParameter> gdp_ptr = gde_.parameterSet_at<double>(pos);
			(*gbdc_it)->resetBoundaries();
			**gbdc_it = gdp_ptr->value();

			if(gdp_ptr->hasBoundaries()) {
				(*gbdc_it)->setBoundaries(gdp_ptr->getLowerBoundary(), gdp_ptr->getUpperBoundary());
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
			boost::shared_ptr<Gem::Dataexchange::GLongParameter> glp_ptr = gde_.parameterSet_at<boost::int32_t>(pos);
			(*gbic_it)->resetBoundaries();
			**gbic_it = glp_ptr->value();

			if(glp_ptr->hasBoundaries()) {
				(*gbic_it)->setBoundaries(glp_ptr->getLowerBoundary(), glp_ptr->getUpperBoundary());
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
			boost::shared_ptr<Gem::Dataexchange::GBoolParameter> gbp_ptr = gde_.parameterSet_at<bool>(pos);
			*gbc_it = gbp_ptr->value();
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

	std::string programName_; ///< The name of the external program to be executed
	std::string parameterFileBaseName_; ///< The base name to be assigned to the parameterFile
 };

} /* namespace Geneva */
} /* namespace Gem */

BOOST_CLASS_EXPORT_KEY(Gem::Geneva::GExternalEvaluatorIndividual)

// Needed for testing purposes
/*************************************************************************************************/
///////////////////////////////////////////////////////////////////////////////////////////////////
/*************************************************************************************************/

#ifdef GENEVATESTING

/**
 * As the Gem::Geneva::Gem::GExternalEvaluatorIndividual has a private default constructor, we need to provide a
 * specialization of the factory function that creates GStartProjectIndividual objects
 */
template <>
boost::shared_ptr<Gem::Geneva::GExternalEvaluatorIndividual> TFactory_GUnitTests<Gem::Geneva::GExternalEvaluatorIndividual>() {
	return boost::shared_ptr<Gem::Geneva::GExternalEvaluatorIndividual>(new Gem::Geneva::GExternalEvaluatorIndividual("../../SampleEvaluator/sampleEvaluator"));
}

#endif /* GENEVATESTING */

/*************************************************************************************************/
///////////////////////////////////////////////////////////////////////////////////////////////////
/*************************************************************************************************/

#endif /* GEXTERNALEVALUATORINDIVIDUAL_HPP_ */
