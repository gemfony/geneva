/**
 * @file GNeuralNetworkIndividual.hpp
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
#include <vector>
#include <fstream>

// Boost header files go here
#include <boost/version.hpp>

#if BOOST_VERSION < 103600
#error "Error: Boost should at least have version 1.36 !"
#endif /* BOOST_VERSION */

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
#include <boost/serialization/is_abstract.hpp>
#include <boost/function.hpp>
#include <boost/bind.hpp>

#ifndef GNEURALNETWORKINDIVIDUAL_HPP_
#define GNEURALNETWORKINDIVIDUAL_HPP_

// GenEvA header files go here
#include "GRandom.hpp"
#include "GDoubleCollection.hpp"
#include "GParameterSet.hpp"
#include "GDoubleGaussAdaptor.hpp"
#include "GenevaExceptions.hpp"
#include "GLogger.hpp"
#include "GLogTargets.hpp"
#include "GBoostThreadPopulation.hpp"
#include "GRandom.hpp"

namespace Gem
{
namespace GenEvA
{

/************************************************************************************************/
/** @brief Class to be thrown as an error if the supplied dimensions or number of datasets where incorrect */
class geneva_invalid_dimensions : public boost::exception {};
/** @brief Class to be thrown as an error if the data file could not be opened */
class geneva_invalid_datafile : public boost::exception {};
/** @brief Class to be thrown as an error if an invalid network architecture was supplied */
class geneva_invalid_architecture : public boost::exception {};

/************************************************************************************************/
/**
 * Allows to specify whether we want to use a sigmoidal transfer function or a radial basis function
 */
enum transferMode(SIGMOID=0,RBF=1);

/************************************************************************************************/
/**
 * A single data set holding the training data of a single training iteration
 */
struct trainingSet
{
	///////////////////////////////////////////////////////////////////////
	friend class boost::serialization::access;

	template<class Archive>
	void serialize(Archive & ar, const unsigned int version) {
		using boost::serialization::make_nvp;

		ar & make_nvp("Input", Input);
		ar & make_nvp("Output", Output);
	}
	///////////////////////////////////////////////////////////////////////

	std::vector<double> Input;
	std::vector<double> Output;
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

		ar & make_nvp("data", data);
	}
	///////////////////////////////////////////////////////////////////////

	std::vector<boost::shared_ptr<trainingSet> > data;
};

/************************************************************************************************/
/**
 * With this individual you can use evolutionary strategies instead of the standard
 * backpropagation algorithm to train feedforward neural networks.
 */
class GNeuralNetworkIndividual
	:public GParameterSet
{
	///////////////////////////////////////////////////////////////////////
	friend class boost::serialization::access;

	template<class Archive>
	void serialize(Archive & ar, const unsigned int version) {
		using boost::serialization::make_nvp;

		ar & make_nvp("ParameterSet", boost::serialization::base_object<GParameterSet>(*this));
		ar & make_nvp("architecture_",architecture_);
		ar & make_nvp("tD_", tD_);
		ar & make_nvp("transferMode_", transferMode_);
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
		:architecture_(architecture),
		 transferMode_(SIGMOID)
	{
		// Check the architecture we've been given and create the layers
		std::size_t nLayers = architecture.size();

		if(nLayers < 2){ // Two layers are required at the minimum (3 and 4 layers are useful)
			std::ostringstream error;
			error << "In GNeuralNetworkIndividual::GNeuralNetworkIndividual([...]) : Error!" << std::endl
				  << "Invalid number of layers supplied" << std::endl;

			LOGGER.log(error.str(), Gem::GLogFramework::CRITICAL);
			throw geneva_invalid_architecture() << error_string(error.str());
		}

		std::vector<std::size_t>::iterator layerIterator;
		std::size_t layerNumber=0;
		std::size_t nNodes=0;
		std::size_t nNodesPrevious=0;

		for(layerIterator=architecture.begin(); layerIterator!=architecture.end(); ++layerIterator){
			if(*layerIterator){ // Add the next network layer to this class, if possible
				nNodes = *layerIterator;

				// Set up a GDoubleCollection
				boost::shared_ptr<GDoubleCollection> gdc(new GDoubleCollection());
				// Set up and register an adaptor for the collection, so it
				// knows how to be mutated. We want a sigma of 0.5, sigma-adaption of 0.05 and
				// a minimum sigma of 0.02.
				boost::shared_ptr<GDoubleGaussAdaptor> gdga(new GDoubleGaussAdaptor(0.5,0.05,0.02,"gauss_mutation"));
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

				LOGGER.log(error.str(), Gem::GLogFramework::CRITICAL);
				throw geneva_invalid_architecture() << error_string(error.str());
			}
		}

		// Load the training data from file
		std::ifstream trDat(trainingDataFile.c_str());

		if(!trDat){
			std::ostringstream error;
			error << "In GNeuralNetworkIndividual::GNeuralNetworkIndividual(const std::string&) : Error!" << std::endl
				  << "Data file " << trainingDataFile << " could not be opened for reading." << std::endl;

			LOGGER.log(error.str(), Gem::GLogFramework::CRITICAL);
			throw geneva_invalid_datafile() << error_string(error.str());
		}

		// Load the data, using the Boost.Serialization library
		{
			boost::archive::xml_iarchive ia(trDat);
			ia >> boost::serialization::make_nvp("projectionData", tD_);
		} // Explicit scope at this point is essential so that ia's destructor is called

		trDat.close();

		// Set the transfer function to sigmoidal
		transfer_ = boost::bind(GNeuralNetworkIndividual::sigmoid,this,_1);
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
		tD_ = boost::shared_ptr(new trainingData());

		// We need to copy the training data over manually
		std::vector<boost::shared_ptr<trainingSet> >::const_iterator cit;
		for(cit=cp.tD_->data.begin(); cit!=cp.tD_->data.end(); ++cit){
			boost::shared_ptr<trainingSet> p(new trainingSet);

			p->Input = (*cit)->Input;
			p->Output = (*cit)->Output;

			tD_->data.push_back(p);
		}

		architecture_ = cp.architecture_;
		setTransferMode_(cp.transferMode_); // Also assigns the correct function object
	}

	/********************************************************************************************/
	/**
	 * The standard destructor
	 */
	~GNeuralNetworkIndividual()
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
	 * Creates a deep clone of this object
	 *
	 * @return A deep clone of this object, camouflaged as a GObject
	 */
	virtual GObject* clone(){
		return new GNeuralNetworkIndividual(*this);
	}

	/********************************************************************************************/
	/**
	 * Loads the data of another GNeuralNetworkIndividual, camouflaged as a GObject
	 *
	 * @param cp A copy of another GNeuralNetworkIndividual, camouflaged as a GObject
	 */
	virtual void load(const GObject* cp){
		const GNeuralNetworkIndividual *gpi_load = checkedConversion(cp, this);

		// Load the parent class'es data
		GParameterSet::load(cp);

		// Load our local data.


		// tD_ is a shared_ptr, hence we need to copy the data itself. We do not do
		// this, if we already have the data present. This happens as we assume that
		// the training data doesn't change.
		if(!tD_){
			tD_ = boost::shared_ptr(new trainingData());

			// We need to copy the training data over manually
			std::vector<boost::shared_ptr<trainingSet> >::const_iterator cit;
			for(cit=gpi_load->tD_->data.begin(); cit!=gpi_load->tD_->data.end(); ++cit){
				boost::shared_ptr<trainingSet> p(new trainingSet);

				p->Input = (*cit)->Input;
				p->Output = (*cit)->Output;

				tD_->data.push_back(p);
			}
		}

		// The architecture of the hidden layers could actually be changed
		// in later versions, hence we copy it over.
		architecture_ = gpi_load->architecture_;

		// Copy and set the transfer mode
		setTransferMode(gpi_load->transferMode_);
	}

	/********************************************************************************************/
	/**
	 * Sets the transfer mode of the neural network either to Sigmoid (the default) or Radial Basis.
	 */
	void setTransferMode(const transferMode& tm){
		transferMode_=tm;
		switch(transferMode_){
		case SIGMOID:
			transfer_= boost::bind(GNeuralNetworkIndividual::sigmoid,this,_1);
			break;
		case RBF:
			transfer_= boost::bind(GNeuralNetworkIndividual::rbf,this,_1);
			break;
		}
	}

	/********************************************************************************************/
	/**
	 * Retrieves the transfer mode of this neural network
	 *
	 * @return The current transfer mode
	 */
	transferMode getTransferMode() const throw(){
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
	 * @param nDim The number of dimensions of the hypercube
	 * @param edgelength The desired edge length of the cube
	 * @return A copy of the trainingData struct that has been created, wrapped in a shared_ptr
	 */
	static boost::shared_ptr<trainingData> createHyperCubeTrainingData(const std::string& fileName,
				                                                       std::size_t nData,
				                                                       std::site_t nDim,
				                                                       double edgelength)
	{
		// Create a local random number generator. We cannot access the
		// class'es generator, as this function is static.
		Gem::Util::GRandom l_gr;

		// Create the required data.
		boost::shared_ptr<trainingData> tD(new trainingData());
		bool outside=false;
		for(std::site_t datCounter=0; datCounter<nData; datCounter++){
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

			tD.data.push_back(tS);
		}


		if(fileName != ""){ // Serialize and write to a file, if requested.
			std::ofstream fileStream(fileName.c_str());
			if(!fileStream){
				std::ostringstream error;
				error << "In GNeuralNetworkIndividual::createHyperCubeTrainingData([...]) : Error!" << std::endl
					  << "Data file " << fileName << " could not be opened for writing." << std::endl;

				LOGGER.log(error.str(), Gem::GLogFramework::CRITICAL);
				throw geneva_invalid_datafile() << error_string(error.str());
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
	 * cube get an output value of 0.01. The trainining data is initialized with a radius of 2*radius.
	 *
	 * You can call this function in the following way:
	 *
	 * GNeuralNetworkInvididual::::createHyperSphereTrainingData("someFileName.xml", [other arguments]);
	 *
	 * If filename is an empty string ("") , then no serialization takes place and the data is retured as
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
															             std::site_t nDim,
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
			switch(nDimOrig)
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
					// nDimOrig will be at least 3 here.
					std::size_t nAngles = nDimOrig - 1;
					std::vector<double> angle_collection(nAngles);
					for(std::size_t i=0; i<(nAngles-1); i++){ // Angles in range [0,Pi[
						angle_collection[i]=l_gr.evenRandom(0., M_PI);
					}
					angle_collection[nAngles-1]=l_gr.evenRandom(0., 2.*M_PI); // Range of last angle is [0,2*Pi[

					//////////////////////////////////////////////////////////////////
					// Now we can fill the source-vector itself
					std::vector<double> cartCoord(nDimOrig);

					for(std::size_t i=0; i<nDimOrig; i++) cartCoord[i]=local_radius; // They all have that

					cartCoord[0] *= cos(angle_collection[0]); // x_1 / cartCoord[0]

					for(std::size_t i=1; i<nDimOrig-1; i++){ // x_2 ... x_(n-1) / cartCoord[1] .... cartCoord[n-2]
						for(std::size_t j=0; j<i; j++){
							cartCoord[i] *= sin(angle_collection[j]);
						}
						cartCoord[i] *= cos(angle_collection[i]);
					}

					for(std::size_t j=0; j<nAngles; j++){ // x_n / cartCoord[n-1]
						cartCoord[nDimOrig-1] *= sin(angle_collection[j]);
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

				LOGGER.log(error.str(), Gem::GLogFramework::CRITICAL);
				throw geneva_invalid_datafile() << error_string(error.str());
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
	 * @param fileName The name of the file the network should be saved in
	 * @return A string holding the data that was just saved to file
	 */
	std::string writeTrainedNetwork(std::string fileName){

	}

protected:
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
	 * @return The fitness of this object
	 */
	virtual double fitnessCalculation() {
		typedef std::vector<const std::vector<double>& > layers;

		// Get easier access to the layer data stored in the GDoubleCollection objects
		layers layerData;
		for(std::size_t layerCounter=0; layerCounter<this->data.size(); layerCounter++){
			layerData.push_back(getData<GDoubleCollection>(layerCounter)->data);
		}

		double result=0;

		// Now loop over all data sets
		std::vector<boost::shared_ptr<trainingSet> >::iterator d_it;
		for(d_it=tD_->data.begin(); d_it!=tD_->data.end(); ++d_it){
			shared_ptr<trainingSet> tS = *d_it;

			// The input layer
			std::vector<double> prevResults;
			std::size_t nLayerNodes = architecture_.at(0);
			double nodeResult;
			for(std::size_t nodeCounter=0; nodeCounter<nLayerNodes; nodeCounter++){
				nodeResult=tS->Input.at(nodeCounter) * layerData[0]->data.at(2*nodeCounter) - layerData[0]->data.at(2*nodeCounter+1);
				nodeResult=transfer_(nodeResult);
				prevResults.push_back(nodeResult);
			}

			// Loop over all following layers
			layers::iterator l_it;
			for(l_it=layerData.begin()+1, std::size_t layerNumber=1;
				l_it!=layerData.end(), layerNumber != architecture_.size();
				++l_it, ++layerNumber)
			{
				std::vector<double> currentResults;
				nLayerNodes=architecture_.at(layerNumber);
				nPrevLayerNodes=architecture_.at(layerNumber-1);

				for(std::size_t nodeCounter=0; nodeCounter<nLayerNodes; nodeCounter++){
					// Loop over all nodes of the previous layer
					nodeResult=0.;
					for(std::size_t prevNodeCounter=0; prevNodeCounter<nPrevLayerNodes; prevNodeCounter++){
						nodeResult += prevResults.at(prevNodeCounter) * (*l_it)->data.at(nodeCounter*(nPrevLayerNodes+1)+prevNodeCounter);
					}
					nodeResult -= (*l_it)->data.at(nodeCounter*(nPrevLayerNodes+1)+nPrevLayerNodes);
					nodeResult=transfer(nodeResult);
					currentResults.push_back(nodeResult);
				}

				prevResults=currentResults;
			}

			// At this point prevResults should contain the output values of the output layer

			// Calculate the error made and add it to the result
			for(std::site_t nodeCounter = 0; nodeCounter<prevResults.size(); nodeCounter++){
				result += pow(prevResults.at(nodeCounter) - tS->Output.at(nodeCounter),2);
			}
		}

		return result;
	}

	/********************************************************************************************/

private:
	GNeuralNetworkIndividual()	{ /* nothing */ } ///< Default constructor intentionally private and empty

	/********************************************************************************************/
	/**
	 * A sigmoidal transfer function.
	 *
	 * @param value The value to which the sigmoid function should be applied
	 */
	double sigmoid(double value){
		return 1./(1.+exp(-value));
	}

	/********************************************************************************************/
	/**
	 * A radial basis transfer function.
	 *
	 * @param value The value to which the rbf function should be applied
	 */
	double rbf(double value){
		return exp(-pow(value,2));
	}

	/********************************************************************************************/
	// Local variables

	std::vector<std::size_t> architecture_; ///< Holds the network's architecture data
	boost::shared_ptr<trainingData> tD_; ///< Holds the training data

	transferMode transferMode_;
	boost::function<double(double)> transfer_;
};

} /* namespace GenEvA */
} /* namespace Gem */

#include <boost/serialization/export.hpp>
BOOST_CLASS_EXPORT(Gem::GenEvA::GNeuralNetworkIndividual)

#endif /* GNEURALNETWORKINDIVIDUAL_HPP_ */
