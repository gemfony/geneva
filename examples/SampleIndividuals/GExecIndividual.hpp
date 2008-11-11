/**
 * @file GExecIndividual.hpp
 */

/* Copyright (C) 2004-2008 Dr. Ruediger Berlich
 * Copyright (C) 2007-2008 Forschungszentrum Karlsruhe GmbH
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

// Standard header files go here
#include <iostream>
#include <cmath>
#include <sstream>
#include <fstream>
#include <cstdlib>
#include <vector>

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

		ar & make_nvp("ParameterSet", boost::serialization::base_object<GParameterSet>(*this));
		ar & make_nvp("fileName_", fileName_);
	}
	///////////////////////////////////////////////////////////////////////

public:
	/********************************************************************************************/
	/**
	 * A constructor which initializes the individual with a suitable set of random double values.
	 *
	 * @param sz The desired size of the double collection
	 * @param min The minimum value of the random numbers to fill the collection
	 * @param max The maximum value of the random numbers to fill the collection
	 * @param as The number of calls to GDoubleGaussAdaptor::mutate after which mutation should be adapted
	 * @param fileName The filename (including path) of the external program that should be executed
	 */
	GExecIndividual(std::size_t sz, double min, double max, boost::uint32_t as, std::string fileName)
		:fileName_(fileName)
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
		 fileName(cp.fileName_)
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
	virtual GObject* clone(){
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

		// Retrieve a pointer to the double vector. Compile in DEBUG mode in order to check this conversion
		boost::shared_ptr<GDoubleCollection> gdc_load = parameterbase_cast<GDoubleCollection>(0);

		// Make the parameters known externally
		std::ostringstream parFile;
		parFile << "parFile_" << this->getPopulationPosition();
		std::ofstream parameters(parFile.str().c_str(), ios_base::out | ios_base::binary | ios_base::trunc);

		// First emit information about the number of double values
		std::size_t nDParm = gdc_load->size();
		parameters.write((char *)&nDParm, sizeof(std::size_t));

		// Then write out the actual parameter value
		GDoubleCollection::iterator it;
		for(it=gdc_load->begin(); it!=gdc_load->end(); ++it) {
			double current = *it;
			parameters.write((char *)&current, sizeof(double));
		}

		// Finally close the file
		parameters.close();

		// Assemble command line and run the external program
		commandLine = fileName_ + " " + parFile.str();

		// Check that we have a valid fileName_
		if(fileName_ == "empty" || fileName_ == "") {
			std::ostringstream error;
			error << "In GExecIndividual::fitnessCalculation(): Error!" << std::endl
				  << "Invalid file name " << fileName_ << std::endl;

			throw geneva_error_condition(error.str());
		}

		system(commandLine.c_str());

	    // then retrieve the output.
	    std::ifstream resultFile(parFile.str().c_str(), ios_base::in | ios_base::binary);
	    resultFile.read((char *)&result, sizeof(double));

	    // Finally close the file
	    resultFile.close();

	    // Let the audience know
		return result;
	}

private:
	/********************************************************************************************/
	/**
	 * The default constructor. Only needed for serialization purposes
	 */
	GExecIndividual()
		:fileName("empty")
	{ /* nothing */ }

	/********************************************************************************************/

	std::string fileName_;
};

} /* namespace GenEvA */
} /* namespace Gem */

#include <boost/serialization/export.hpp>
BOOST_CLASS_EXPORT(Gem::GenEvA::GExecIndividual)

#endif /* GEXECINDIVIDUAL_HPP_ */
