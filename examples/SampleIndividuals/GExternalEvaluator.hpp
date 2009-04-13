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
 * External programs should understand the following arguments:
 * -i / --initialize : gives the external program the opportunity to do any needed
 *                          preliminary work (e.g. downloading files, setting up directories, ...)
 * -f / -- finilize :   Allows the external program to clean up after work. Compare the -i switch.
 * -p / --paramfile : The name of the file through which data is exchanged
 *     This switch is needed for the following options:
 *     -t / --template: asks the external program to write a description of the individual
 *                               into paramfile (e.g. program -t -p <filename> ).
 *            -t also allows the additional option -R (randomly initialize parameters)
 *     -r / --result  Asks the external program to emit a result file in a user-defined format.
 *                         It will not be read in by this individual, but allows the user to examine
 *                         the results. The input data needed to create the result file is contained
 *                         in the parameter file. Calls will be done like this: "program -r -p <filename>"
 * If the "-p <filename>" switch is used without any further arguments, the external program
 * is expected to perform a value calculation, based on the data in the parameterfile and
 * to emit the result into the same file.
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
	GExternalEvaluator(const std::string& fileName, const std::string& arguments)
		:fileName_(fileName),
		 arguments_(arguments),
		 nEvaluations_(1)
	{
		// todo: get data from external program.
	}

	/********************************************************************************************/
	/**
	 * A standard copy constructor
	 */
	GExternalEvaluator(const GExternalEvaluator& cp)
		:GParameterSet(cp),
		 fileName_(cp.fileName_),
		 arguments_(cp.arguments_),
		 nEvaluations_(cp.nEvaluations_)
	{ /* nothing */	}

	/********************************************************************************************/
	/**
	 * The standard destructor
	 */
	~GExternalEvaluator()
	{ /* nothing */	}

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
		 nEvaluations_(1)
	{ /* nothing */ }

	/********************************************************************************************/

	std::string fileName_; ///< The name of the external program to be executed
	std::string arguments_; ///< Any additional arguments to be handed to the external program
	boost::uint32_t nEvaluations_; ///< The number of data sets to be handed to the external program in one go
};

} /* namespace GenEvA */
} /* namespace Gem */

#include <boost/serialization/export.hpp>
BOOST_CLASS_EXPORT(Gem::GenEvA::GExecternalEvaluator)

#endif /* GEXTERNALEVALUATOR_HPP_ */
