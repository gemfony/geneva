/**
 * @file GNeuralNetworkIndividual.hpp
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


// Standard header files go here
#include <iostream>
#include <cmath>
#include <sstream>
#include <vector>
#include <fstream>

// Includes check for correct Boost version(s)
#include "GGlobalDefines.hpp"

// Boost header files go here
#include <boost/shared_ptr.hpp>
#include <boost/archive/xml_oarchive.hpp>
#include <boost/archive/xml_iarchive.hpp>
#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>
#include <boost/archive/binary_oarchive.hpp>
#include <boost/archive/binary_iarchive.hpp>
#include <boost/serialization/nvp.hpp>
#include <boost/serialization/vector.hpp>
#include <boost/serialization/shared_ptr.hpp>
#include <boost/serialization/base_object.hpp>
#include <boost/serialization/utility.hpp>
#include <boost/serialization/tracking.hpp>
#include <boost/serialization/split_member.hpp>
#include <boost/serialization/export.hpp>
#include <boost/serialization/version.hpp>
#include <boost/serialization/split_member.hpp>
#include <boost/function.hpp>
#include <boost/bind.hpp>
#include <boost/ref.hpp>
#include <boost/optional.hpp>

#ifndef GNEURALNETWORKINDIVIDUAL_HPP_
#define GNEURALNETWORKINDIVIDUAL_HPP_

// For Microsoft-compatible compilers
#if defined(_MSC_VER)  &&  (_MSC_VER >= 1020)
#pragma once
#endif

// GenEvA header files go here
#include "GRandom.hpp"
#include "GDoubleCollection.hpp"
#include "GParameterSet.hpp"
#include "GDoubleGaussAdaptor.hpp"
#include "GenevaExceptions.hpp"
#include "GEqualityPrinter.hpp"

namespace Gem
{
namespace GenEvA
{

/************************************************************************************************/
/**
 * Allows to specify whether we want to use a sigmoidal transfer function or a radial basis function
 */
enum transferMode {SIGMOID=0, RBF=1};

/************************************************************************************************/
/**
 * A single data set holding the training data of a single training iteration
 */
struct trainingSet
{
	///////////////////////////////////////////////////////////////////////
	friend class boost::serialization::access;

	template<class Archive>
	void serialize(Archive & ar, const unsigned int) {
		using boost::serialization::make_nvp;

		ar & BOOST_SERIALIZATION_NVP(Input)
		   & BOOST_SERIALIZATION_NVP(Output);
	}
	///////////////////////////////////////////////////////////////////////

	/********************************************************************************************/
	/** The destructor -- empty, as there is no local, dynamically
	 * allocated data. A virtual destructor is needed due to a
	 * serialization problem in Boost 1.41*/
	virtual ~trainingSet() { /* nothing */ }

	/********************************************************************************************/
	/**
	 * Checks for equality with another trainingSet object
	 *
	 * @param  cp A constant reference to another trainingSet object
	 * @return A boolean indicating whether both objects are equal
	 */
	bool operator==(const trainingSet& cp) const {
		using namespace Gem::Util;
		// Means: The expectation of equality was fulfilled, if no error text was emitted (which converts to "true")
		return !checkRelationshipWith(cp, CE_EQUALITY, 0.,"trainingSet::operator==","cp", CE_SILENT);
	}

	/********************************************************************************************/
	/**
	 * Checks for inequality with another trainingSet object
	 *
	 * @param  cp A constant reference to another trainingSet object
	 * @return A boolean indicating whether both objects are inequal
	 */
	bool operator!=(const trainingSet& cp) const {
		using namespace Gem::Util;
		// Means: The expectation of inequality was fulfilled, if no error text was emitted (which converts to "true")
		return !checkRelationshipWith(cp, CE_INEQUALITY, 0.,"trainingSet::operator!=","cp", CE_SILENT);
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
	boost::optional<std::string> checkRelationshipWith(const trainingSet& cp,
			const Gem::Util::expectation& e,
			const double& limit,
			const std::string& caller,
			const std::string& y_name,
			const bool& withMessages) const
	{
	    using namespace Gem::Util;
	    using namespace Gem::Util::POD;

		// Will hold possible deviations from the expectation, including explanations
	    std::vector<boost::optional<std::string> > deviations;

		// Check local data
		deviations.push_back(checkExpectation(withMessages, "trainingSet", Input, cp.Input, "Input", "cp.Input", e , limit));
		deviations.push_back(checkExpectation(withMessages, "trainingSet", Output, cp.Output, "Output", "cp.Output", e , limit));

		return evaluateDiscrepancies("trainingSet", caller, deviations, e);
	}

	/********************************************************************************************/
	// Local data
	std::vector<double> Input; ///< Holds the input data
	std::vector<double> Output; ///< Holds the output data
};

/************************************************************************************************/
/**
 * This struct holds all necessary information for the training of the neural network individual.
 * We simply use serialized data generated from the struct using the Boost.Serialization library.
 */
struct trainingData
{
	///////////////////////////////////////////////////////////////////////
	friend class boost::serialization::access;

	template<class Archive>
	void serialize(Archive & ar, const unsigned int version) {
		using boost::serialization::make_nvp;

		ar & BOOST_SERIALIZATION_NVP(data);
	}
	///////////////////////////////////////////////////////////////////////

	/********************************************************************************************/
	/**
	 * A standard destructor. "Virtual" needed due to aserialization problem in Boost 1.41.
	 */
	virtual ~trainingData() { /* nothing */ }

	/********************************************************************************************/
	/**
	 * Checks for equality with another trainingData object
	 *
	 * @param  cp A constant reference to another trainingData object
	 * @return A boolean indicating whether both objects are equal
	 */
	bool operator==(const trainingData& cp) const {
		using namespace Gem::Util;
		// Means: The expectation of equality was fulfilled, if no error text was emitted (which converts to "true")
		return !checkRelationshipWith(cp, CE_EQUALITY, 0.,"trainingData::operator==","cp", CE_SILENT);
	}

	/********************************************************************************************/
	/**
	 * Checks for inequality with another trainingData object
	 *
	 * @param  cp A constant reference to another trainingData object
	 * @return A boolean indicating whether both objects are inequal
	 */
	bool operator!=(const trainingData& cp) const {
		using namespace Gem::Util;
		// Means: The expectation of inequality was fulfilled, if no error text was emitted (which converts to "true")
		return !checkRelationshipWith(cp, CE_INEQUALITY, 0.,"trainingData::operator!=","cp", CE_SILENT);
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
	boost::optional<std::string> checkRelationshipWith(const trainingData& cp,
			const Gem::Util::expectation& e,
			const double& limit,
			const std::string& caller,
			const std::string& y_name,
			const bool& withMessages) const
	{
	    using namespace Gem::Util;
	    using namespace Gem::Util::POD;

		// Will hold possible deviations from the expectation, including explanations
	    std::vector<boost::optional<std::string> > deviations;

	    // Check vector sizes
	    if(data.size() != cp.data.size()) {
	    	std::ostringstream error;
	    	error << "Vector sizes did not match in trainingData::checkRelationshipWith(): " << data.size() << " / " << cp.data.size();
	    	deviations.push_back(boost::optional<std::string>(error.str()));
	    }
	    else {
	    	// Check local data
	    	std::vector<boost::shared_ptr<trainingSet> >::iterator it;
	    	std::vector<boost::shared_ptr<trainingSet> >::const_iterator cit;
	    	for(it=data.begin(), cit=cp.data.begin(); it!=data.end(); ++it, ++cit) {
	    		deviations.push_back((*it)->checkRelationshipWith(**cit, e, limit, y_name, withMessages));
	    	}
	    }

		return evaluateDiscrepancies("trainingData", caller, deviations, e);
	}

	/********************************************************************************************/
	/** Holds the individual data sets */
	std::vector<boost::shared_ptr<trainingSet> > data;
};

/************************************************************************************************/
/**
 * With this individual you can use evolutionary strategies instead of the standard
 * back-propagation algorithm to train feed-forward neural networks.
 */
class GNeuralNetworkIndividual
	:public GParameterSet
{
	///////////////////////////////////////////////////////////////////////
	friend class boost::serialization::access;

	template<typename Archive>
	void serialize(Archive & ar, const unsigned int) {
		using boost::serialization::make_nvp;

		ar & BOOST_SERIALIZATION_BASE_OBJECT_NVP(GParameterSet)
		   & BOOST_SERIALIZATION_NVP(architecture_)
		   & BOOST_SERIALIZATION_NVP(tD_)
		   & BOOST_SERIALIZATION_NVP(transferMode_);
	}

	///////////////////////////////////////////////////////////////////////

public:
	/********************************************************************************************/
	/**
	 * A constructor which initializes the individual with a suitable set of network layers. It
	 * also loads the training data from file.
	 *
	 * @param trainingDataFile The name of a file holding the training data
	 * @param architecture Holds the number of nodes in the input layer, hidden(1/2) layer and output layer
	 * @param min The minimum value of random numbers used for initialization of the network layers
	 * @param max The maximum value of random numbers used for initialization of the network layers
	 */
	GNeuralNetworkIndividual(std::string trainingDataFile,
				             const std::vector<std::size_t>& architecture,
				             double min, double max)
		: architecture_(architecture)
		, transferMode_(SIGMOID)
	{
		// Check the architecture we've been given and create the layers
		std::size_t nLayers = architecture.size();

		if(nLayers < 2){ // Two layers are required at the minimum (3 and 4 layers are useful)
			std::ostringstream error;
			error << "In GNeuralNetworkIndividual::GNeuralNetworkIndividual([...]) : Error!" << std::endl
				  << "Invalid number of layers supplied" << std::endl;

			throw geneva_error_condition(error.str());
		}

		std::vector<std::size_t>::iterator layerIterator;
		std::size_t layerNumber=0;
		std::size_t nNodes=0;
		std::size_t nNodesPrevious=0;

		for(layerIterator=architecture_.begin(); layerIterator!=architecture_.end(); ++layerIterator){
			if(*layerIterator){ // Add the next network layer to this class, if possible
				nNodes = *layerIterator;

				// Set up a GDoubleCollection
				boost::shared_ptr<GDoubleCollection> gdc(new GDoubleCollection());
				// Set up and register an adaptor for the collection, so it
				// knows how to be mutated. We want a sigma dependent on the value of max, sigma-adaption of 0.001,
				// a minimum sigma of 0.0001, and a maximum sigma dependent on the value of max
				boost::shared_ptr<GDoubleGaussAdaptor> gdga(new GDoubleGaussAdaptor(fabs(max), 0.001, 0.002, fabs(max)));
				gdc->addAdaptor(gdga);

				// The input layer needs 2*nNodes double values
				if(layerNumber==0) gdc->addRandomData(2*nNodes, min, max);
				// We need nNodes * (nNodesPrevious + 1) double values
				else gdc->addRandomData(nNodes*(nNodesPrevious+1), min, max);

				// Make the parameter collection known to this individual
				this->data.push_back(gdc);

				nNodesPrevious=nNodes;
				layerNumber++;
			}
			else {
				std::ostringstream error;
				error << "In GNeuralNetworkIndividual::GNeuralNetworkIndividual([...]) : Error!" << std::endl
					  << "Found invalid number of nodes in layer: " << *layerIterator << std::endl;

				throw geneva_error_condition(error.str());
			}
		}

		// Load the training data from file
		std::ifstream trDat(trainingDataFile.c_str());

		if(!trDat){
			std::ostringstream error;
			error << "In GNeuralNetworkIndividual::GNeuralNetworkIndividual(const std::string&) : Error!" << std::endl
				  << "Data file " << trainingDataFile << " could not be opened for reading." << std::endl;

			throw geneva_error_condition(error.str());
		}

		// Load the data, using the Boost.Serialization library
		{
			boost::archive::xml_iarchive ia(trDat);
			ia >> boost::serialization::make_nvp("projectionData", tD_);
		} // Explicit scope at this point is essential so that ia's destructor is called

		trDat.close();
	}

	/********************************************************************************************/
	/**
	 * A constructor that accepts a trainingData struct as argument, instead of loading the data
	 * from file.
	 *
	 * @param tD A trainingData struct, holding the required training data
	 * @param architecture Holds the number of nodes in the input layer, hidden(1/2) layer and output layer
	 * @param min The minimum value of random numbers used for initialization of the network layers
	 * @param max The maximum value of random numbers used for initialization of the network layers
	 */
	GNeuralNetworkIndividual(boost::shared_ptr<trainingData> tD,
							 const std::vector<std::size_t>& architecture,
							 double min, double max)
		: architecture_(architecture)
		, tD_(tD)
		, transferMode_(SIGMOID)
	{
		// Check the architecture we've been given and create the layers
		std::size_t nLayers = architecture.size();

		if(nLayers < 2){ // Two layers are required at the minimum (3 and 4 layers are useful)
			std::ostringstream error;
			error << "In GNeuralNetworkIndividual::GNeuralNetworkIndividual([...]) : Error!" << std::endl
				  << "Invalid number of layers supplied" << std::endl;

			throw geneva_error_condition(error.str());
		}

		std::vector<std::size_t>::iterator layerIterator;
		std::size_t layerNumber=0;
		std::size_t nNodes=0;
		std::size_t nNodesPrevious=0;

		for(layerIterator=architecture_.begin(); layerIterator!=architecture_.end(); ++layerIterator){
			if(*layerIterator){ // Add the next network layer to this class, if possible
				nNodes = *layerIterator;

				// Set up a GDoubleCollection
				boost::shared_ptr<GDoubleCollection> gdc(new GDoubleCollection());
				// Set up and register an adaptor for the collection, so it
				// knows how to be mutated. We want a sigma dependent on the value of max, sigma-adaption of 0.001,
				// a minimum sigma of 0.0001, and a maximum sigma dependent on the value of max
				boost::shared_ptr<GDoubleGaussAdaptor> gdga(new GDoubleGaussAdaptor(fabs(max), 0.001, 0.0001,fabs(max)));
				gdc->addAdaptor(gdga);

				// The input layer needs 2*nNodes double values
				if(layerNumber==0) gdc->addRandomData(2*nNodes, min, max);
				// We need nNodes * (nNodesPrevious + 1) double values
				else gdc->addRandomData(nNodes*(nNodesPrevious+1), min, max);

				// Make the parameter collection known to this individual
				this->data.push_back(gdc);

				nNodesPrevious=nNodes;
				layerNumber++;
			}
			else {
				std::ostringstream error;
				error << "In GNeuralNetworkIndividual::GNeuralNetworkIndividual([...]) : Error!" << std::endl
					  << "Found invalid number of nodes in layer: " << *layerIterator << std::endl;

				throw geneva_error_condition(error.str());
			}
		}
	}

	/********************************************************************************************/
	/**
	 * A standard copy constructor
	 *
	 * @param cp A copy of another GNeuralNetworkIndividual object
	 */
	GNeuralNetworkIndividual(const GNeuralNetworkIndividual& cp)
		:GParameterSet(cp)
	{
		tD_ = boost::shared_ptr<trainingData>(new trainingData());

		// We need to copy the training data over manually
		std::vector<boost::shared_ptr<trainingSet> >::const_iterator cit;
		for(cit=cp.tD_->data.begin(); cit!=cp.tD_->data.end(); ++cit){
			boost::shared_ptr<trainingSet> p(new trainingSet);

			p->Input = (*cit)->Input;
			p->Output = (*cit)->Output;

			tD_->data.push_back(p);
		}

		architecture_ = cp.architecture_;
		transferMode_ = cp.tansferMode_;
	}

	/********************************************************************************************/
	/**
	 * The standard destructor
	 */
	virtual ~GNeuralNetworkIndividual()
	{ /* nothing */	}

	/********************************************************************************************/
	/**
	 * A standard assignment operator
	 *
	 * @param cp A copy of another GNeuralNetworkIndividual object
	 * @return A reference to this object
	 */
	const GNeuralNetworkIndividual& operator=(const GNeuralNetworkIndividual& cp){
		GNeuralNetworkIndividual::load(&cp);
		return *this;
	}

	/********************************************************************************************/
	/**
	 * Loads the data of another GNeuralNetworkIndividual, camouflaged as a GObject
	 *
	 * @param cp A copy of another GNeuralNetworkIndividual, camouflaged as a GObject
	 */
	virtual void load(const GObject* cp){
		const GNeuralNetworkIndividual *p_load = conversion_cast(cp, this);

		// Load the parent class'es data
		GParameterSet::load(cp);

		// Load our local data.


		// tD_ is a shared_ptr, hence we need to copy the data itself. We do not do
		// this, if we already have the data present. This happens as we assume that
		// the training data doesn't change.
		if(!tD_){
			tD_ = boost::shared_ptr<trainingData>(new trainingData());

			// We need to copy the training data over manually
			std::vector<boost::shared_ptr<trainingSet> >::const_iterator cit;
			for(cit=p_load->tD_->data.begin(); cit!=p_load->tD_->data.end(); ++cit){
				boost::shared_ptr<trainingSet> p(new trainingSet);

				p->Input = (*cit)->Input;
				p->Output = (*cit)->Output;

				tD_->data.push_back(p);
			}
		}

		// The architecture of the hidden layers could actually be changed
		// in later versions, hence we copy it over.
		architecture_ = p_load->architecture_;

		// Copy and set the transfer mode
		transferMode_ = p_load->transferMode_;
	}

	/********************************************************************************************/
	/**
	 * Checks for equality with another GNeuralNetworkIndividual object
	 *
	 * @param  cp A constant reference to another GNeuralNetworkIndividual object
	 * @return A boolean indicating whether both objects are equal
	 */
	bool operator==(const GNeuralNetworkIndividual& cp) const {
		using namespace Gem::Util;
		// Means: The expectation of equality was fulfilled, if no error text was emitted (which converts to "true")
		return !checkRelationshipWith(cp, CE_EQUALITY, 0.,"GNeuralNetworkIndividual::operator==","cp", CE_SILENT);
	}

	/********************************************************************************************/
	/**
	 * Checks for inequality with another GNeuralNetworkIndividual object
	 *
	 * @param  cp A constant reference to another GNeuralNetworkIndividual object
	 * @return A boolean indicating whether both objects are inequal
	 */
	bool operator!=(const GNeuralNetworkIndividual& cp) const {
		using namespace Gem::Util;
		// Means: The expectation of inequality was fulfilled, if no error text was emitted (which converts to "true")
		return !checkRelationshipWith(cp, CE_INEQUALITY, 0.,"GNeuralNetworkIndividual::operator!=","cp", CE_SILENT);
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
			const Gem::Util::expectation& e,
			const double& limit,
			const std::string& caller,
			const std::string& y_name,
			const bool& withMessages) const
	{
	    using namespace Gem::Util;
	    using namespace Gem::Util::POD;

		// Check that we are indeed dealing with a GParamterBase reference
		const GNeuralNetworkIndividual *p_load = GObject::conversion_cast(&cp,  this);

		// Will hold possible deviations from the expectation, including explanations
	    std::vector<boost::optional<std::string> > deviations;

	    // Check our parent class ....
	    deviations.push_back(GParameterSet::checkRelationshipWith(cp, e, limit, "GNeuralNetworkIndividual", y_name, withMessages));

	    // ... and then our local data
	    deviations.push_back(tD_.checkRelationshipWith(cp.tD_, e, limit, y_name, withMessages));
	    deviations.push_back(checkExpectation(withMessages, "GNeuralNetworkIndividual", architecture_, p_load->architecture_, "architecture_", "p_load->architecture_", e , limit));
	    deviations.push_back(checkExpectation(withMessages, "GNeuralNetworkIndividual", transferMode_, p_load->transferMode_, "transferMode_", "p_load->transferMode_", e , limit));

		return evaluateDiscrepancies("GNeuralNetworkIndividual", caller, deviations, e);
	}


	/********************************************************************************************/
	/**
	 * Sets the transfer mode of the neural network either to Sigmoid (the default) or Radial Basis.
	 */
	void setTransferMode(const transferMode& tm){
		transferMode_=tm;
	}

	/********************************************************************************************/
	/**
	 * Retrieves the transfer mode of this neural network
	 *
	 * @return The current transfer mode
	 */
	transferMode getTransferMode() const {
		return transferMode_;
	}

	/********************************************************************************************/
	/**
	 * This static function can be called in main() in order to create a suitable set of training
	 * data for this class. It is added here as this individual is for demonstration purposes only
	 * and as the creation of a separate helper program could be avoided in this way. We use a simple
	 * hyper-cube, ranging from [-edgelength/2,edgelength/2[ in each dimension. Areas outside of the
	 * cube get an output value of 0.99, areas inside of the cube get an output value of 0.01. The
	 * training data is initialized in the range [-edgelength:edgelength[.
	 *
	 * You can call this function in the following way:
	 *
	 * GNeuralNetworkInvididual::createHyerCubeTrainingData("someFileName.xml", [other arguments]);
	 *
	 * @param fileName of the file the result should be written to
	 * @param nData The number of training sets to create
	 * @param nDim The number of dimensions of the hyper cube
	 * @param edgelength The desired edge length of the cube
	 * @return A copy of the trainingData struct that has been created, wrapped in a shared_ptr
	 */
	static boost::shared_ptr<trainingData> createHyperCubeTrainingData(const std::string& fileName,
				                                                       std::size_t nData,
				                                                       std::size_t nDim,
				                                                       double edgelength)
	{
		// Create a local random number generator. We cannot access the
		// class'es generator, as this function is static.
		Gem::Util::GRandom l_gr;

		// Create the required data.
		boost::shared_ptr<trainingData> tD(new trainingData());
		bool outside=false;
		for(std::size_t datCounter=0; datCounter<nData; datCounter++){
			outside=false;
			boost::shared_ptr<trainingSet> tS(new trainingSet());

			for(std::size_t i=0; i<nDim; i++){
				double oneDimRnd = l_gr.evenRandom(-edgelength,edgelength);

				// Need to find at least one dimension outside of the perimeter
				// in order to set the outside flag to true.
				if(oneDimRnd < -edgelength/2. || oneDimRnd > edgelength/2.) outside=true;

				tS->Input.push_back(oneDimRnd);
			}

			if(outside) tS->Output.push_back(0.99);
			else tS->Output.push_back(0.01);

			tD->data.push_back(tS);
		}


		if(fileName != ""){ // Serialize and write to a file, if requested.
			std::ofstream fileStream(fileName.c_str());
			if(!fileStream){
				std::ostringstream error;
				error << "In GNeuralNetworkIndividual::createHyperCubeTrainingData([...]) : Error!" << std::endl
					  << "Data file " << fileName << " could not be opened for writing." << std::endl;

				throw geneva_error_condition(error.str());
			}
			else {
				boost::archive::xml_oarchive oa(fileStream);
				oa << boost::serialization::make_nvp("trainingData", tD);
			}

			fileStream.close();
		}

		return tD;
	}

	/********************************************************************************************/
	/**
	 * This static function can be called in main() in order to create a suitable input file for
	 * this class. It is added here as this individual is for demonstration purposes only and as the
	 * creation of a separate helper program could be avoided in this way. We create a sphere of
	 * radius "radius". See http://en.wikipedia.org/wiki/Hypersphere for a description of the
	 * formulae used.  Areas outside of the sphere get an output value of 0.99, areas inside of the
	 * cube get an output value of 0.01. The training data is initialized with a radius of 2*radius.
	 *
	 * You can call this function in the following way:
	 *
	 * GNeuralNetworkInvididual::::createHyperSphereTrainingData("someFileName.xml", [other arguments]);
	 *
	 * If filename is an empty string ("") , then no serialization takes place and the data is returned as
	 * a struct only.
	 *
	 * @param fileName Name of the file the result should be written to
	 * @param nData The number of training sets to create
	 * @param nDim The number of dimensions of the hypersphere
	 * @param radius The desired radius of the sphere
	 * @return A copy of the trainingData struct that has been created, wrapped in a shared_ptr
	 */
	static boost::shared_ptr<trainingData> createHyperSphereTrainingData(const std::string& fileName,
															             std::size_t nData,
															             std::size_t nDim,
															             double radius)
	{
		// Create a local random number generator. We cannot access the
		// class'es generator, as this function is static.
		Gem::Util::GRandom l_gr;

		// Create the required data.
		boost::shared_ptr<trainingData> tD(new trainingData());
		double local_radius=1.;

		for(std::size_t datCounter=0; datCounter<nData; datCounter++)
		{
			boost::shared_ptr<trainingSet> tS(new trainingSet());

			local_radius = l_gr.evenRandom(0,2*radius);
			if(local_radius > radius) tS->Output.push_back(0.99);
			else tS->Output.push_back(0.01);

			//////////////////////////////////////////////////////////////////
			// Calculate random cartesian coordinates for hyper sphere

			// Special cases
			switch(nDim)
			{
			case 1:
				tS->Input.push_back(local_radius);
				break;

			case 2:
				{
					double phi = l_gr.evenRandom(0,2*M_PI);
					tS->Input.push_back(local_radius*sin(phi)); // x
					tS->Input.push_back(local_radius*cos(phi)); // y
				}
				break;

			default: // dimensions 3 ... inf
				{
					//////////////////////////////////////////////////////////////////
					// Create the required random numbers in spherical coordinates.
					// nDim will be at least 3 here.
					std::size_t nAngles = nDim - 1;
					std::vector<double> angle_collection(nAngles);
					for(std::size_t i=0; i<(nAngles-1); i++){ // Angles in range [0,Pi[
						angle_collection[i]=l_gr.evenRandom(0., M_PI);
					}
					angle_collection[nAngles-1]=l_gr.evenRandom(0., 2.*M_PI); // Range of last angle is [0,2*Pi[

					//////////////////////////////////////////////////////////////////
					// Now we can fill the source-vector itself
					std::vector<double> cartCoord(nDim);

					for(std::size_t i=0; i<nDim; i++) cartCoord[i]=local_radius; // They all have that

					cartCoord[0] *= cos(angle_collection[0]); // x_1 / cartCoord[0]

					for(std::size_t i=1; i<nDim-1; i++){ // x_2 ... x_(n-1) / cartCoord[1] .... cartCoord[n-2]
						for(std::size_t j=0; j<i; j++){
							cartCoord[i] *= sin(angle_collection[j]);
						}
						cartCoord[i] *= cos(angle_collection[i]);
					}

					for(std::size_t j=0; j<nAngles; j++){ // x_n / cartCoord[n-1]
						cartCoord[nDim-1] *= sin(angle_collection[j]);
					}

					// Transfer the results
					tS->Input=cartCoord;
				}
				break;
			}

			tD->data.push_back(tS);
		}

		if(fileName != ""){ // Serialize and write to a file, if requested.
			std::ofstream fileStream(fileName.c_str());
			if(!fileStream){
				std::ostringstream error;
				error << "In GNeuralNetworkIndividual::createHyperSphereTrainingData([...]) : Error!" << std::endl
					  << "Data file " << fileName << " could not be opened for writing." << std::endl;

				throw geneva_error_condition(error.str());
			}
			else {
				boost::archive::xml_oarchive oa(fileStream);
				oa << boost::serialization::make_nvp("trainingData", tD);
			}

			fileStream.close();
		}

		return tD;
	}

	/********************************************************************************************/
	/**
	 * Creates a C++ output file for the trained network, suitable for usage in
	 * other projects. If you just want to retrieve the C++ description of the network,
	 * call this function with an empty string "" .
	 *
	 * @param headerFile The name of the header file the network should be saved in
	 * @return A string holding the data that was just saved to file
	 */
	std::string writeTrainedNetwork(const std::string& headerFile, const std::string& testProgram) {
		std::ostringstream header;
		std::ostringstream testprogram;

		// The following only makes sense if the input dimension is 2
		if(architecture_[0] == 2){
			testprogram << "/**" << std::endl
			            << " * @file " << testProgram << std::endl
			            << " *" << std::endl
			            << " * This program allows to visualize the output of the training example." << std::endl
			            << " * It has been auto-generated by the GNeuralNetworkIndividual class of" << std::endl
			            << " * the GenEvA library" << std::endl
			            << " */" << std::endl
			            << std::endl
			            << "/* Copyright (C) 2004-2008 Dr. Ruediger Berlich" << std::endl
						<< " * Copyright (C) 2007-2008 Forschungszentrum Karlsruhe GmbH" << std::endl
						<< " *" << std::endl
						<< " * This file is part of Geneva, Gemfony scientific's optimization library." << std::endl
						<< " *" << std::endl
						<< " * Geneva is free software: you can redistribute it and/or modify" << std::endl
						<< " * it under the terms of version 3 of the GNU Affero General Public License" << std::endl
						<< " * as published by the Free Software Foundation." << std::endl
						<< " *" << std::endl
						<< " * Geneva is distributed in the hope that it will be useful," << std::endl
						<< " * but WITHOUT ANY WARRANTY; without even the implied warranty of" << std::endl
						<< " * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the" << std::endl
						<< " * GNU Affero General Public License for more details." << std::endl
						<< " *" << std::endl
						<< " * You should have received a copy of the GNU Affero General Public License" << std::endl
						<< " * along with the Geneva library.  If not, see <http://www.gnu.org/licenses/>." << std::endl
						<< " */" << std::endl
						<< std::endl
						<< "/*" << std::endl
						<< " * Can be compiled with the command" << std::endl
						<< " * g++ -g -o testNetwork -I/opt/boost136/include/boost-1_36/ " << testProgram << std::endl
						<< " * on OpenSUSE 11 (assuming that Boost in installed under /opt in your" << std::endl
						<< " * system." << std::endl
						<< " */" << std::endl
						<< std::endl
						<< "#include <iostream>" << std::endl
						<< "#include <sstream>" << std::endl
						<< "#include <fstream>" << std::endl
						<< "#include <vector>" << std::endl
						<< std::endl
						<< "#include <boost/cstdint.hpp>" << std::endl
						<< "#include <boost/random.hpp>" << std::endl
						<< std::endl
						<< "#include \"trainingResult.hpp\"" << std::endl
						<< std::endl
						<< "const boost::uint32_t MAXPOINTS=10000;" << std::endl
						<< std::endl
						<< "using namespace Gem::NeuralNetwork;" << std::endl
						<< std::endl
						<< "main(){" << std::endl
						<< "  boost::lagged_fibonacci607 lf(123);" << std::endl
						<< std::endl
						<< "  double x=0., y=0., result=0;" << std::endl
						<< "  std::vector<double> in;" << std::endl
						<< "  std::vector<double> out;" << std::endl
						<< std::endl
						<< "  std::vector<double> x_inside, y_inside;" << std::endl
						<< "  std::vector<double> x_outside, y_outside;" << std::endl
						<< std::endl
						<< "  // Create random numbers and check the output" << std::endl
						<< "  for(boost::uint32_t i=0; i<MAXPOINTS; i++){" << std::endl
						<< "    x=-1. + 2.*lf();" << std::endl
						<< "    y=-1. + 2.*lf();" << std::endl
						<< std::endl
						<< "    in.clear();" << std::endl
						<< "    out.clear();" << std::endl
						<< std::endl
						<< "    in.push_back(x);" << std::endl
						<< "    in.push_back(y);" << std::endl
						<< std::endl
						<< "    if(!network(in,out) || out.size()==0){" << std::endl
						<< "      std::cout << \"Error in calculation of network output\" << std::endl;" << std::endl
						<< "      exit(1);" << std::endl
						<< "    }" << std::endl
						<< std::endl
						<< "    double output = out[0];" << std::endl
						<< std::endl
						<< "    if(output < 0.5) {" << std::endl
						<< "      x_inside.push_back(x);" << std::endl
						<< "      y_inside.push_back(y);" << std::endl
						<< "    }" << std::endl
						<< "    else{" << std::endl
						<< "      x_outside.push_back(x);" << std::endl
						<< "      y_outside.push_back(y);" << std::endl
						<< "    }" << std::endl
						<< "  }" << std::endl
						<< std::endl
						<< "  // Write test results" << std::endl
						<< "  std::ostringstream results;" << std::endl
						<< "  results << \"{\" << std::endl" << std::endl
						<< "          << \"  double x_inside[\" << x_inside.size() << \"];\" << std::endl" << std::endl
						<< "          << \"  double y_inside[\" << y_inside.size() << \"];\" << std::endl" << std::endl
						<< "          << \"  double x_outside[\" << x_outside.size() << \"];\" << std::endl" << std::endl
						<< "          << \"  double y_outside[\" << y_outside.size() << \"];\" << std::endl" << std::endl
						<< "          << std::endl;" << std::endl
						<< std::endl
						<< "  for(std::size_t i=0; i<x_inside.size(); i++){" << std::endl
						<< "    results << \"  x_inside[\" << i << \"] = \" << x_inside[i] << \";\" << std::endl" << std::endl
						<< "            << \"  y_inside[\" << i << \"] = \" << y_inside[i] << \";\" << std::endl;" << std::endl
						<< "  }" << std::endl
						<< std::endl
						<< "  for(std::size_t i=0; i<x_outside.size(); i++){" << std::endl
						<< "    results << \"  x_outside[\" << i << \"] = \" << x_outside[i] << \";\" << std::endl" << std::endl
						<< "            << \"  y_outside[\" << i << \"] = \" << y_outside[i] << \";\" << std::endl;" << std::endl
						<< "  }" << std::endl
						<< std::endl
						<< "  results << std::endl" << std::endl
						<< "          << \"  TGraph *inside = new TGraph(\" << x_inside.size() << \", x_inside, y_inside);\" << std::endl" << std::endl
						<< "          << \"  TGraph *outside = new TGraph(\" << x_outside.size() << \", x_outside, y_outside);\" << std::endl" << std::endl
						<< "          << std::endl" << std::endl
						<< "          << \"  inside->SetMarkerStyle(21);\" << std::endl" << std::endl
						<< "          << \"  inside->SetMarkerSize(0.2);\" << std::endl" << std::endl
						<< "          << \"  inside->SetMarkerColor(12);\" << std::endl" << std::endl
						<< "          << \"  outside->SetMarkerStyle(21);\" << std::endl" << std::endl
						<< "          << \"  outside->SetMarkerSize(0.35);\" << std::endl" << std::endl
						<< "          << \"  outside->SetMarkerColor(17);\" << std::endl" << std::endl
						<< "          << std::endl" << std::endl
						<< "          << \"  inside->Draw(\\\"AP\\\");\" << std::endl" << std::endl
						<< "          << \"  outside->Draw(\\\"P\\\");\" << std::endl" << std::endl
						<< "          << \"}\" << std::endl;" << std::endl
						<< std::endl
						<< "  std::cout << \"Writing test results into file testResults.C\" << std::endl" << std::endl
						<< "            << \"Test with the command \\\"root -l testResults.C\\\"\" << std::endl;" << std::endl
						<< "  std::ofstream fstr(\"testResults.C\");" << std::endl
						<< "  fstr << results.str();" << std::endl
						<< "  fstr.close();" << std::endl
						<< "}" << std::endl;
		}

		header << "/**" << std::endl
				<< " * @file " << headerFile << std::endl
				<< " *" << std::endl
				<< " * This file represents the results of a feedforward neural network trained" << std::endl
				<< " * using the GenEvA library. It has been auto-generated by the GNeuralNetworkIndividual" << std::endl
				<< " * class." << std::endl
				<< " */" << std::endl
				<< std::endl
				<< "/* Copyright (C) 2004-2008 Dr. Ruediger Berlich" << std::endl
				<< " * Copyright (C) 2007-2008 Forschungszentrum Karlsruhe GmbH" << std::endl
				<< " *" << std::endl
				<< " * This file is part of Geneva, Gemfony scientific's optimization library." << std::endl
				<< " *" << std::endl
				<< " * Geneva is free software: you can redistribute it and/or modify" << std::endl
				<< " * it under the terms of version 3 of the GNU Affero General Public License" << std::endl
				<< " * as published by the Free Software Foundation." << std::endl
				<< " *" << std::endl
				<< " * Geneva is distributed in the hope that it will be useful," << std::endl
				<< " * but WITHOUT ANY WARRANTY; without even the implied warranty of" << std::endl
				<< " * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the" << std::endl
				<< " * GNU Affero General Public License for more details." << std::endl
				<< " *" << std::endl
				<< " * You should have received a copy of the GNU Affero General Public License" << std::endl
				<< " * along with the Geneva library.  If not, see <http://www.gnu.org/licenses/>." << std::endl
				<< " */" << std::endl
				<< std::endl
				<< "#include <cmath>" << std::endl
				<< "#include <vector>" << std::endl
				<< std::endl
				<< "#ifndef GENEVANEURALNETHEADER_HPP_" << std::endl
				<< "#define GENEVANEURALNETHEADER_HPP_" << std::endl
				<< std::endl
				<< "namespace Gem" << std::endl
				<< "{" << std::endl
				<< "  namespace NeuralNetwork" << std::endl
				<< "  {" << std::endl
				<< "    double transfer(double value) {" << std::endl;

		switch(transferMode_) {
			case SIGMOID:
				header << "      return 1./(1.+exp(-value));" << std::endl;
				break;
			case RBF:
				header << "      return exp(-value*value);" << std::endl;
				break;
		}

		header << "    }" << std::endl
				<< std::endl
				<< "    bool network(const std::vector<double>& in, std::vector<double>& out){" << std::endl
				<< "      double nodeResult=0.;" << std::endl
				<< std::endl
				<< "      register std::size_t nodeCounter = 0;" << std::endl
				<< "      register std::size_t prevNodeCounter = 0;" << std::endl
				<< std::endl
				<< "      const std::size_t nLayers = " << this->data.size() <<";" << std::endl
				<< "      const std::size_t architecture[nLayers] = {" << std::endl;

		for(std::size_t i=0; i<architecture_.size(); i++) {
			header << "        " << architecture_.at(i);
			if(i==architecture_.size() - 1)
				header << std::endl;
			else
				header << "," << std::endl;
		}

		std::size_t weightOffset=0;

		header << "      };" << std::endl
				<< "      const std::size_t weightOffset[nLayers] = {" << std::endl
				<< "        " << weightOffset << "," << std::endl;

		weightOffset += 2*architecture_[0];
		header << "       " << weightOffset << "," << std::endl;

		for(std::size_t i=1; i<architecture_.size()-1; i++) {
			weightOffset += architecture_[i]*(architecture_[i-1]+1);
			header << "        " << weightOffset;

			if(i==architecture_.size() - 1)
				header << std::endl;
			else
				header << "," << std::endl;
		}

		header << "      };" << std::endl;

		std::size_t nWeights = 2*architecture_[0];
		for(std::size_t i=1; i<architecture_.size(); i++) {
			nWeights += architecture_[i]*(architecture_[i-1]+1);
		}

		header << "      const std::size_t nWeights = " << nWeights << ";" << std::endl
				<< "      const double weights[nWeights] = {" << std::endl;

		for(std::size_t i=0; i<architecture_.size(); i++) {
			boost::shared_ptr<GDoubleCollection> currentLayer = pc_at<GDoubleCollection>(i);

			for(std::size_t j=0; j<currentLayer->size(); j++) {
				header << "        " << currentLayer->at(j);

				if(i==(architecture_.size()-1) && j==(currentLayer->size()-1)) header << std::endl;
				else header << "," << std::endl;
			}
		}

		header << "      };" << std::endl
				<< std::endl
				<< "      // Rudimentary error check" << std::endl
				<< "      if(in.size() != architecture[0]) return false;" << std::endl
				<< std::endl
				<< "      // Clear the result vector" << std::endl
				<< "      out.clear();" << std::endl
				<< std::endl
				<< "      // The input layer" << std::endl
				<< "      std::vector<double> prevResults;" << std::endl
				<< "      std::size_t nLayerNodes = architecture[0];" << std::endl
				<< "      std::size_t nPrevLayerNodes = 0;" << std::endl
				<< std::endl
				<< "      for(nodeCounter=0; nodeCounter<nLayerNodes; nodeCounter++){" << std::endl
				<< "        nodeResult=in[nodeCounter] * weights[2*nodeCounter] - weights[2*nodeCounter+1];" << std::endl
				<< "        nodeResult=transfer(nodeResult);" << std::endl
				<< "        prevResults.push_back(nodeResult);" << std::endl
				<< "      }" << std::endl
				<< std::endl
				<< "      // All other layers" << std::endl
				<< "      for(register std::size_t layerCounter=1; layerCounter<nLayers; layerCounter++){" << std::endl
				<< "        std::vector<double> currentResults;" << std::endl
				<< "        nLayerNodes=architecture[layerCounter];" << std::endl
				<< "        nPrevLayerNodes=architecture[layerCounter-1];" << std::endl
				<< std::endl
				<< "        // For each node" << std::endl
				<< "        for(nodeCounter=0; nodeCounter<nLayerNodes; nodeCounter++){" << std::endl
				<< "          nodeResult=0.;" << std::endl
				<< "          // Loop over all nodes of the previous layer" << std::endl
				<< "          for(prevNodeCounter=0; prevNodeCounter<nPrevLayerNodes; prevNodeCounter++){" << std::endl
				<< "            nodeResult += prevResults[prevNodeCounter]*weights[weightOffset[layerCounter] + nodeCounter*(nPrevLayerNodes+1)+prevNodeCounter];" << std::endl
				<< "          }" << std::endl
				<< "          nodeResult -= weights[weightOffset[layerCounter] + nodeCounter*(nPrevLayerNodes+1)+nPrevLayerNodes];" << std::endl
				<< "          nodeResult = transfer(nodeResult);" << std::endl
				<< "          currentResults.push_back(nodeResult);" << std::endl
				<< "        }" << std::endl
				<< std::endl
				<< "        prevResults=currentResults;" << std::endl
				<< "      }" << std::endl
				<< std::endl
				<< "      // At this point prevResults should contain the output values of the output layer" << std::endl
				<< "      out=prevResults;" << std::endl
				<< std::endl
				<< "      return true;" << std::endl
				<< "    }" << std::endl
				<< std::endl
				<< "  } /* namespace NeuralNetwork */" << std::endl
				<< "} /* namespace Gem */" << std::endl
				<< std::endl
				<< "#endif /* GENEVANEURALNETHEADER_HPP_ */" << std::endl;

		// Write header to file, if requested
		if(headerFile != "") {
			std::ofstream fstr(headerFile.c_str());
			if(fstr) {
				fstr << header.str();
				fstr.close();
			}
			else {
				std::ostringstream error;
				error << "In GNeuralNetworkIndividual::writeTrainedNetwork(const std::string&) :" << std::endl
				      << "Error writing output file!" << std::endl;

				throw geneva_error_condition(error.str());
			}
		}

		if(architecture_[0] == 2){
			// Write testprogram to file, if requested and useful
			if(testProgram != "") {
				std::ofstream fstr(testProgram.c_str());
				if(fstr) {
					fstr << testprogram.str();
					fstr.close();
				}
				else {
					std::ostringstream error;
					error << "In GNeuralNetworkIndividual::writeTrainedNetwork(const std::string&) :" << std::endl
						  << "Error writing output file!" << std::endl;

					throw geneva_error_condition(error.str());
				}
			}
		}

		return header.str();
	}

protected:
	/********************************************************************************************/
	/**
	 * Creates a deep clone of this object
	 *
	 * @return A deep clone of this object, camouflaged as a GObject
	 */
	virtual GObject* clone_() const{
		return new GNeuralNetworkIndividual(*this);
	}

	/********************************************************************************************/
	/**
	 * The actual fitness calculation (i.e. the error calculation) takes place here. In the
	 * case of a feed-forward network this fitness is equivalent to the error a network makes
	 * for a given weight-set when trying to categorize a training set with known network output.
	 * Minimizing this error means training the network.
	 *
	 * The error is implemented using the formula
	 *
	 * \f[
	 * E(weights)=\sum_{\nu=1}^{p}\sum_{k}(y_{k}^{\nu}-s_{k}(x^{\nu}))^{2}
	 * \f]
	 *
	 * where \f$p\f$ is the number of training patters (pairs of input/output
	 * values), \f$k\f$ is the number of output nodes, \f$y_{k}^{\nu}\f$ is
	 * the desired output value of output node \f$k\f$ for input pattern
	 * \f$x^{\nu}\f$ and \f$s_{k}(x^{\nu})\f$ is the real output of output
	 * node \f$k\f$ for input pattern \f$x^{\nu}\f$.
	 *
	 * The function "transfer()" used in this function can be either radial basis
	 * or sigmoid.
	 *
	 * A small demonstration of the technique of storing a reference to a vector
	 * in another vector is shown in the file refWrapper.cpp in the Geneva test cases.
	 *
	 * @return The fitness of this object
	 */
	virtual double fitnessCalculation() {
		double result=0;

		// Now loop over all data sets
		std::vector<boost::shared_ptr<trainingSet> >::iterator d_it;
		for(d_it=tD_->data.begin(); d_it!=tD_->data.end(); ++d_it){
			boost::shared_ptr<trainingSet> tS = *d_it;

			// The input layer
			std::vector<double> prevResults;
			std::size_t nLayerNodes = architecture_.at(0);
			double nodeResult=0;
			boost::shared_ptr<GDoubleCollection> inputLayer = pc_at<GDoubleCollection>(0);
			for(std::size_t nodeCounter=0; nodeCounter<nLayerNodes; nodeCounter++){
				nodeResult=tS->Input.at(nodeCounter) * inputLayer->at(2*nodeCounter) - inputLayer->at(2*nodeCounter+1);
				nodeResult=transfer(nodeResult);
				prevResults.push_back(nodeResult);
			}

			// All other layers
			for(std::size_t layerCounter=1; layerCounter<this->data.size(); layerCounter++){
				std::vector<double> currentResults;
				nLayerNodes=architecture_.at(layerCounter);
				std::size_t nPrevLayerNodes=architecture_.at(layerCounter-1);
				boost::shared_ptr<GDoubleCollection> currentLayer = pc_at<GDoubleCollection>(layerCounter);

				for(std::size_t nodeCounter=0; nodeCounter<nLayerNodes; nodeCounter++){
					// Loop over all nodes of the previous layer
					nodeResult=0.;
					for(std::size_t prevNodeCounter=0; prevNodeCounter<nPrevLayerNodes; prevNodeCounter++){
						nodeResult += prevResults.at(prevNodeCounter) * currentLayer->at(nodeCounter*(nPrevLayerNodes+1)+prevNodeCounter);
					}
					nodeResult -= currentLayer->at(nodeCounter*(nPrevLayerNodes+1)+nPrevLayerNodes);
					nodeResult=transfer(nodeResult);
					currentResults.push_back(nodeResult);
				}

				prevResults=currentResults;
			}

			// At this point prevResults should contain the output values of the output layer

			// Calculate the error made and add it to the result
			for(std::size_t nodeCounter = 0; nodeCounter<prevResults.size(); nodeCounter++){
				result += GSQUARED(prevResults.at(nodeCounter) - tS->Output.at(nodeCounter));
			}
		}

		return result;
	}

	/********************************************************************************************/

private:
	GNeuralNetworkIndividual()	{ /* nothing */ } ///< Default constructor intentionally private and empty

	/********************************************************************************************/
	/**
	 * The actual transfer function, which chooses from a number of available backends, depending
	 * on the transfer mode.
	 */
	inline double transfer(const double& value) {
		switch(tansferMode_) {
		case SIGMOID:
			return sigmoid(value);
			break;
		case RBF:
			return rbf(value);
			break;
		}
	}

	/********************************************************************************************/
	/**
	 * A sigmoidal transfer function. Declared static so we avoid storing the local "this" pointer
	 * in a boost::function object.
	 *
	 * @param value The value to which the sigmoid function should be applied
	 */
	inline double sigmoid(const double& value) {
		return 1./(1.+exp(-value));
	}

	/********************************************************************************************/
	/**
	 * A radial basis transfer function. Declared static so we avoid storing the local "this" pointer
	 * in a boost::function object.
	 *
	 * @param value The value to which the rbf function should be applied
	 */
	inline double rbf(const double& value) {
		return exp(-GSQUARED(value));
	}

	/********************************************************************************************/
	// Local variables

	std::vector<std::size_t> architecture_; ///< Holds the network's architecture data
	boost::shared_ptr<trainingData> tD_; ///< Holds the training data

	transferMode transferMode_;
};

} /* namespace GenEvA */
} /* namespace Gem */

/************************************************************************************************/
#include <boost/serialization/export.hpp>
BOOST_CLASS_EXPORT(Gem::GenEvA::GNeuralNetworkIndividual)

#endif /* GNEURALNETWORKINDIVIDUAL_HPP_ */
