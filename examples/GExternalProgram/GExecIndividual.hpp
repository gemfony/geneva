/**
 * @file GExecIndividual.hpp
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

#ifndef GEXECINDIVIDUAL_HPP_
#define GEXECINDIVIDUAL_HPP_

// GenEvA header files go here
#include "GDoubleCollection.hpp"
#include "GParameterSet.hpp"
#include "GDoubleGaussAdaptor.hpp"

namespace Gem
{
namespace GenEvA
{

/************************************************************************************************/
/**
 * This individual calls an external program to evaluate a given set of double values.
 */
class GExecIndividual
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
	}
	///////////////////////////////////////////////////////////////////////

public:
	/********************************************************************************************/
	/**
	 * A constructor which initializes the individual with the name of the external program
	 * that should be executed.
	 *
	 * @param fileName The filename (including path) of the external program that should be executed
	 * @param arguments Additional user-defined arguments to be handed to the external program
	 */
	GExecIndividual(const std::string& fileName, const std::string& arguments)
		:fileName_(fileName),
		 arguments_(arguments)
	{ /* nothing */ }

	/********************************************************************************************/
	/**
	 * A constructor which initializes the individual with a suitable set of random double values.
	 * This constructor is meant as a quick and dirty start point. Usually one would rather add
	 * GDoubleCollection objects to the individual as required, using the first constructor.
	 *
	 * @param sz The desired size of the double collection
	 * @param min The minimum value of the random numbers to fill the collection
	 * @param max The maximum value of the random numbers to fill the collection
	 * @param as The number of calls to GDoubleGaussAdaptor::mutate after which mutation should be adapted
	 * @param fileName The filename (including path) of the external program that should be executed
	 * @param arguments Additional user-defined arguments to be handed to the external program
	 */
	GExecIndividual(std::size_t sz,
				       double min,
				       double max,
				       boost::uint32_t as,
				       const std::string& fileName,
				       const std::string& arguments)
		:fileName_(fileName),
		 arguments_(arguments)
	{
		// Set up a GDoubleCollection with sz values, each initialized
		// with a random number in the range [min,max[
		boost::shared_ptr<GDoubleCollection> gdc(new GDoubleCollection(sz,min,max));

		// Set up and register an adaptor for the collection, so it
		// knows how to be mutated. We want a sigma of 1, a sigma-adaption of 0.001, a
		// minimum sigma of 0.000001 and a maximum sigma of 5.
		boost::shared_ptr<GDoubleGaussAdaptor> gdga(new GDoubleGaussAdaptor(1.,0.001,0.000001,5));
		gdga->setAdaptionThreshold(as);

		gdc->addAdaptor(gdga);

		// Make the parameter collection known to this individual
		this->data.push_back(gdc);
	}

	/********************************************************************************************/
	/**
	 * A standard copy constructor
	 */
	GExecIndividual(const GExecIndividual& cp)
		:GParameterSet(cp),
		 fileName_(cp.fileName_),
		 arguments_(cp.arguments_)
	{ /* nothing */	}

	/********************************************************************************************/
	/**
	 * The standard destructor
	 */
	~GExecIndividual()
	{ /* nothing */	}

	/********************************************************************************************/
	/**
	 * A standard assignment operator
	 */
	const GExecIndividual& operator=(const GExecIndividual& cp){
		GExecIndividual::load(&cp);
		return *this;
	}

	/********************************************************************************************/
	/**
	 * Creates a deep clone of this object
	 *
	 * @return A deep clone of this object, camouflaged as a GObject
	 */
	virtual GObject* clone() const{
		return new GExecIndividual(*this);
	}

	/********************************************************************************************/
	/**
	 * Loads the data of another GExecIndividual, camouflaged as a GObject
	 *
	 * @param cp A copy of another GExecIndividual, camouflaged as a GObject
	 */
	virtual void load(const GObject* cp){
		const GExecIndividual *gei_load = conversion_cast(cp, this);

		// First load the data of our parent class ...
		GParameterSet::load(cp);

		// ... and then our own
		fileName_ = gei_load->fileName_;
		arguments_ = gei_load->arguments_;
	}

	/********************************************************************************************/
	/**
	 * Initiates the printing of the best individual
	 */
	void printResult() {
		std::string commandLine;

		// Retrieve a pointer to the GDoubleCollection
		boost::shared_ptr<GDoubleCollection> gdc_load = parameterbase_cast<GDoubleCollection>(0);

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
			error << "In GExecIndividual::printResult(): Error!" << std::endl
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
		boost::shared_ptr<GDoubleCollection> gdc_load = parameterbase_cast<GDoubleCollection>(0);

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
			error << "In GExecIndividual::fitnessCalculation(): Error!" << std::endl
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
	GExecIndividual()
		:fileName_("unknown"),
		 arguments_("empty")
	{ /* nothing */ }

	/********************************************************************************************/

	std::string fileName_; ///< The name of the external program to be executed
	std::string arguments_; ///< Any additional arguments to be handed to the external program
};

} /* namespace GenEvA */
} /* namespace Gem */

#include <boost/serialization/export.hpp>
BOOST_CLASS_EXPORT(Gem::GenEvA::GExecIndividual)

#endif /* GEXECINDIVIDUAL_HPP_ */
