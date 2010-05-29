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
#include <boost/filesystem.hpp>

#ifndef GNEURALNETWORKINDIVIDUAL_HPP_
#define GNEURALNETWORKINDIVIDUAL_HPP_

// For Microsoft-compatible compilers
#if defined(_MSC_VER)  &&  (_MSC_VER >= 1020)
#pragma once
#endif

// GenEvA header files go here
#include "GRandom.hpp"
#include "GDoubleObjectCollection.hpp"
#include "GParameterSet.hpp"
#include "GDoubleGaussAdaptor.hpp"
#include "GDouble.hpp"
#include "GExceptions.hpp"
#include "GHelperFunctionsT.hpp"
#include "GStdSimpleVectorInterfaceT.hpp"

namespace Gem
{
namespace GenEvA
{

/************************************************************************************************/
//////////////////////////////////////////////////////////////////////////////////////////////////
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

	/** @brief The destructor */
	virtual ~trainingSet();

	/** @brief Assigns another trainingSet's data to this object */
	const trainingSet& operator=(const trainingSet&);
	/** @brief Checks for equality with another trainingSet object */

	bool operator==(const trainingSet&) const;
	/** @brief Checks for inequality with another trainingSet object */
	bool operator!=(const trainingSet&) const;

	/** @brief Checks whether a given expectation is fulfilled. */
	boost::optional<std::string> checkRelationshipWith(const trainingSet&,
			const Gem::Util::expectation&,
			const double&,
			const std::string&,
			const std::string&,
			const bool&) const;

	/********************************************************************************************/
	// Local data
	std::vector<double> Input; ///< Holds the input data
	std::vector<double> Output; ///< Holds the output data
};

/************************************************************************************************/
//////////////////////////////////////////////////////////////////////////////////////////////////
/************************************************************************************************/
/**
 * This class holds all necessary information for the training of the neural network individual,
 * including the network's geometry. For intermediate storage on disk, we can serialize the
 * entire object using the Boost.Serialization library. networkData objects can themselves be
 * treated as std::vector. The idea is that the architecture is appended to the object, with
 * the first attached number being the input layer and the last one the output layer. Inbetween
 * numbers are hidden layers.
 */
class networkData
	:public GStdSimpleVectorInterfaceT<std::size_t>
{
	///////////////////////////////////////////////////////////////////////
	friend class boost::serialization::access;

	template<class Archive>
	void serialize(Archive & ar, const unsigned int) {
		using boost::serialization::make_nvp;

		ar  & make_nvp("GStdSimpleVectorInterfaceT_size_t", boost::serialization::base_object<GStdSimpleVectorInterfaceT<std::size_t> >(*this))
			& BOOST_SERIALIZATION_NVP(data_)
		    & BOOST_SERIALIZATION_NVP(currentIndex_);
	}

public:
	///////////////////////////////////////////////////////////////////////
	/** @brief Default constructor */
	networkData();
	/** @brief Initialization with data from file */
	explicit networkData(const std::string&);
	/** @brief The copy constructor */
	networkData(const networkData&);
	/** @brief A standard destructor. */
	virtual ~networkData();

	/** @brief Copies the data of another networkData object */
	const networkData& operator=(const networkData&);

	/** @brief Checks for equality with another networkData object */
	bool operator==(const networkData&) const;
	/** @brief Checks for inequality with another networkData object */
	bool operator!=(const networkData&) const;

	/** @brief Checks whether a given expectation is fulfilled. */
	boost::optional<std::string> checkRelationshipWith(const networkData&,
			const Gem::Util::expectation&,
			const double&,
			const std::string&,
			const std::string&,
			const bool&) const;

	/** @brief Saves the data of this struct to disc */
	void saveToDisk(const std::string&) const;
	/** @brief Loads training data from the disc */
	void loadFromDisk(const std::string&);

	/** @brief Adds a new training set to the collection, Requires for the network architecture to be defined already */
	void addTrainingSet(boost::shared_ptr<trainingSet>);
	/** @brief Retrieves the next training set */
	boost::optional<boost::shared_ptr<trainingSet> > getNextTrainingSet() const;
	/** @brief Resets the index of the current training set */
	void resetCurrentIndex() const;

	/** @brief Retrieves the number of input nodes of this network */
	std::size_t getNInputNodes() const;
	/** @brief Retrieves the number of output nodes of this network */
	std::size_t getNOutputNodes() const;

	/** @brief Saves this data set in ROOT format for visual inspection */
	void toRoot(const std::string&, const double&, const double&);
protected:
	/** @brief This function is purely virtual in our parent class*/
	virtual void dummyFunction() { /* nothing */ };

private:
	/********************************************************************************************/
	/** @brief Holds the individual data items */
	std::vector<boost::shared_ptr<trainingSet> > data_;
	/** @brief The index of the current training set */
	mutable std::size_t currentIndex_;
};

/************************************************************************************************/
//////////////////////////////////////////////////////////////////////////////////////////////////
/************************************************************************************************/
/** @brief This enum is used to specify the type of training data that should be generated */
enum trainingDataType {TDTNONE=0, HYPERCUBE=1, HYPERSPHERE=2, AXISCENTRIC=3, SINUS=4};
/** @brief Allows to specify whether we want to use a sigmoidal transfer function or a radial basis function */
enum transferFunction {SIGMOID=0, RBF=1};

/************************************************************************************************/
/** @brief  Reads a Gem::GenEvA::trainingDataType item from a stream */
std::istream& operator>>(std::istream& i, Gem::GenEvA::trainingDataType& tdt);
/** @brief Puts a Gem::GenEvA::trainingDataType item into a stream */
std::ostream& operator<<(std::ostream& o, const Gem::GenEvA::trainingDataType& tdt);
/** @brief Reads a Gem::GenEvA::transferFunction item from a stream. */
std::istream& operator>>(std::istream& i, Gem::GenEvA::transferFunction& tF);
/** @brief Puts a Gem::GenEvA::transferFunction item into a stream. */
std::ostream& operator<<(std::ostream& o, const Gem::GenEvA::transferFunction& tF);

/************************************************************************************************/
//////////////////////////////////////////////////////////////////////////////////////////////////
/************************************************************************************************/
/**
 * With this individual you can use other optimization methods instead of the standard
 * back-propagation algorithm to train feed-forward neural networks.
 */
template <transferFunction tF=SIGMOID>
class GNeuralNetworkIndividual
	:public GParameterSet
{
	///////////////////////////////////////////////////////////////////////

	friend class boost::serialization::access;

	template<typename Archive>
	void load(Archive & ar, const unsigned int) {
		using boost::serialization::make_nvp;

		ar & BOOST_SERIALIZATION_BASE_OBJECT_NVP(GParameterSet)
		   & BOOST_SERIALIZATION_NVP(networkDataFile_);

		// Load the network data from disk
		nD_ = boost::shared_ptr<networkData>(new networkData(networkDataFile_));
	}

	template<typename Archive>
	void save(Archive & ar, const unsigned int) const {
		using boost::serialization::make_nvp;

		ar & BOOST_SERIALIZATION_BASE_OBJECT_NVP(GParameterSet)
		   & BOOST_SERIALIZATION_NVP(networkDataFile_);
	}

	BOOST_SERIALIZATION_SPLIT_MEMBER()

	///////////////////////////////////////////////////////////////////////

public:
	/********************************************************************************************/
	/**
	 * A constructor which initializes the individual with a suitable set of network layers. It
	 * also loads the training data from file.
	 *
	 * @param networkDataFile The name of a file holding the training data
	 * @param architecture Holds the number of nodes in the input layer, hidden(1/2) layer and output layer
	 * @param min The minimum value of random numbers used for initialization of the network layers
	 * @param max The maximum value of random numbers used for initialization of the network layers
	 * @param sigma The sigma used for gauss adaptors
	 * @param sigmaSigma Used for sigma adaption
	 * @oaram minSigma The minimum allowed value for sigma
	 * @param maxSigma The maximum allowed value for sigma
	 */
	GNeuralNetworkIndividual(
			  const std::string& networkDataFile
			, const double& min, const double& max
			, const double& sigma, const double& sigmaSigma
			, const double& minSigma, const double& maxSigma
			, const double& adProb
	)
		: networkDataFile_(networkDataFile)
		, nD_(new networkData(networkDataFile_))
		, transferFunction_(tF)
	{
		// Create a local random number generator
		Gem::Util::GRandom gr(Gem::Util::RNRLOCAL);

		// Check the architecture we've been given and create the layers
		std::size_t nLayers = nD_->size();

		if(nLayers < 2){ // Two layers are required at the minimum (3 and 4 layers are useful)
			std::ostringstream error;
			error << "In GNeuralNetworkIndividual::GNeuralNetworkIndividual<tF>([...]) : Error!" << std::endl
				  << "Invalid number of layers supplied. Did you set up the network architecture ?" << std::endl;
			throw Gem::Common::gemfony_error_condition(error.str());
		}

		networkData::iterator layerIterator;
		std::size_t layerNumber=0;
		std::size_t nNodes=0;
		std::size_t nNodesPrevious=0;

		// Set up the architecture
		for(layerIterator=nD_->begin(); layerIterator!=nD_->end(); ++layerIterator){
			if(*layerIterator){ // Add the next network layer to this class, if possible
				nNodes = *layerIterator;

				// Set up a GDoubleObjectCollection
				boost::shared_ptr<GDoubleObjectCollection> gdoc(new GDoubleObjectCollection());

				// Add GDouble objects
				for(std::size_t i=0; i<(layerNumber==0?2*nNodes:nNodes*(nNodesPrevious+1)); i++) {
					// Set up a GDouble object, initializing it with random data
					boost::shared_ptr<GDouble> gd_ptr(new GDouble(gr.evenRandom(min,max)));

					// Set up an adaptor
					boost::shared_ptr<GDoubleGaussAdaptor> gdga(new GDoubleGaussAdaptor(sigma, sigmaSigma, minSigma, maxSigma));
					gdga->setAdaptionProbability(adProb);

					// Register it with the GDouble object
					gd_ptr->addAdaptor(gdga);

					// Register the GDouble object with the collection
					gdoc->push_back(gd_ptr);
				}

				// Make the parameter collection known to this individual
				this->data.push_back(gdoc);

				nNodesPrevious=nNodes;
				layerNumber++;
			}
			else {
				std::ostringstream error;
				error << "In GNeuralNetworkIndividual<tF>::GNeuralNetworkIndividual([...]) : Error!" << std::endl
					  << "Found invalid number of nodes in layer: " << *layerIterator << std::endl
					  << "Did you set up the network architecture ?" << std::endl;
				throw Gem::Common::gemfony_error_condition(error.str());
			}
		}
	}

	/********************************************************************************************/
	/**
	 * A standard copy constructor
	 *
	 * @param cp A copy of another GNeuralNetworkIndividual object
	 */
	GNeuralNetworkIndividual(const GNeuralNetworkIndividual<tF>& cp)
		: GParameterSet(cp)
		, networkDataFile_(cp.networkDataFile_)
		, nD_(new networkData(*(cp.nD_)))
		, transferFunction_(tF)
	{ /* nothing */ }

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
		GNeuralNetworkIndividual::load_(&cp);
		return *this;
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
		const GNeuralNetworkIndividual *p_load = GObject::conversion_cast<GNeuralNetworkIndividual>(&cp);

		// Will hold possible deviations from the expectation, including explanations
	    std::vector<boost::optional<std::string> > deviations;

	    // Check our parent class ....
	    deviations.push_back(GParameterSet::checkRelationshipWith(cp, e, limit, "GNeuralNetworkIndividual", y_name, withMessages));

	    // ... and then our local data
	    deviations.push_back(checkExpectation(withMessages, "GNeuralNetworkIndividual", networkDataFile_, p_load->networkDataFile_, "networkDataFile_", "p_load->networkDataFile_", e , limit));
	    deviations.push_back(nD_->checkRelationshipWith(*(p_load->nD_), e, limit, "GNeuralNetworkIndividual", y_name, withMessages));

		return evaluateDiscrepancies("GNeuralNetworkIndividual", caller, deviations, e);
	}

	/********************************************************************************************/
	/**
	 * This static function can be called in main() in order to create a suitable set of training
	 * data for this class. It is added here as a means of testing the neural network individual.
	 * We use a simple hyper-cube, ranging from [-edgelength/2,edgelength/2[ in each dimension.
	 * Areas outside of the cube get an output value of 0.99, areas inside of the cube get an output
	 * value of 0.01. The training data is initialized in the range [-edgelength:edgelength[.
	 *
	 * @param architecture The desired architecture of the network
	 * @param nDataSets The number of training sets to create
	 * @param edgelength The desired edge length of the cube
	 * @return A copy of the networkData struct that has been created, wrapped in a shared_ptr
	 */
	static boost::shared_ptr<networkData> createHyperCubeNetworkData (
			const std::vector<std::size_t>& architecture
		  , const std::size_t& nDataSets
		  , const double& edgelength
	) {
		// Check the number of supplied layers
		if(architecture.size() < 2) { // We need at least an input- and an output-layer
			std::ostringstream error;
			error << "In GNeuralNetworkIndividual::createHyperCubeNetworkData(): Error!" << std::endl
				  << "Got invalid number of layers: " << architecture.size() << std::endl;
			throw(Gem::Common::gemfony_error_condition(error.str()));
		}

		// Check that the output layer has exactly one node
		if(architecture.back() != 1) {
			std::ostringstream error;
			error << "In GNeuralNetworkIndividual::createHyperCubeNetworkData(): Error!" << std::endl
				  << "The output layer must have exactly one node for this training data." << std::endl
				  << "Got " << architecture.back() << " instead." << std::endl;
			throw(Gem::Common::gemfony_error_condition(error.str()));
		}

		// Create a local random number generator.
		Gem::Util::GRandom gr(Gem::Util::RNRLOCAL);

		// The dimension of the hypercube is identical to the number of input nodes
		std::size_t nDim = architecture[0];

		// Create the actual networkData object and attach the architecture
		// Checks the architecture on the way
		boost::shared_ptr<networkData> nD(new networkData());
		std::vector<std::size_t>::const_iterator it;
		std::size_t layerCounter = 0;
		for(it=architecture.begin(); it!=architecture.end(); ++it, ++layerCounter) {
			if(*it == 0) {
				std::ostringstream error;
				error << "In GNeuralNetworkIndividual::createHyperCubeNetworkData(): Error!" << std::endl
					  << "Layer " << layerCounter << "has invalid size " << *it << std::endl;
				throw(Gem::Common::gemfony_error_condition(error.str()));
			}

			nD->push_back(*it);
		}

		// Create the required data.
		bool outside=false;
		for(std::size_t datCounter=0; datCounter<nDataSets; datCounter++){
			outside=false;
			boost::shared_ptr<trainingSet> tS(new trainingSet());

			for(std::size_t i=0; i<nDim; i++){
				double oneDimRnd = gr.evenRandom(-edgelength,edgelength);

				// Need to find at least one dimension outside of the perimeter
				// in order to set the outside flag to true.
				if(oneDimRnd < -edgelength/2. || oneDimRnd > edgelength/2.) outside=true;

				tS->Input.push_back(oneDimRnd);
			}

			if(outside) tS->Output.push_back(0.99);
			else tS->Output.push_back(0.01);

			nD->addTrainingSet(tS);
		}

		return nD;
	}

	/********************************************************************************************/
	/**
	 * This static function can be called in main() in order to create a suitable input file for
	 * this class. It is added here as a means of testing this neural network individual. We create
	 * a sphere of radius "radius". See http://en.wikipedia.org/wiki/Hypersphere for a description of
	 * the formulae used.  Areas outside of the sphere get an output value of 0.99, areas inside of the
	 * sphere get an output value of 0.01. The training data is initialized with a radius of 2*radius.
	 *
	 * @param architecture The desired architecture of the network
	 * @param nDataSets The number of training sets to create
	 * @param radius The desired radius of the sphere
	 * @return A copy of the networkData struct that has been created, wrapped in a shared_ptr
	 */
	static boost::shared_ptr<networkData> createHyperSphereNetworkData (
			const std::vector<std::size_t>& architecture
		  , const std::size_t& nDataSets
		  , const double& radius
	) {
		// Check the number of supplied layers
		if(architecture.size() < 2) { // We need at least an input- and an output-layer
			std::ostringstream error;
			error << "In GNeuralNetworkIndividual::createHyperSphereNetworkData(): Error!" << std::endl
				  << "Got invalid number of layers: " << architecture.size() << std::endl;
			throw(Gem::Common::gemfony_error_condition(error.str()));
		}

		// Check that the output layer has exactly one node
		if(architecture.back() != 1) {
			std::ostringstream error;
			error << "In GNeuralNetworkIndividual::createHyperSphereNetworkData(): Error!" << std::endl
				  << "The output layer must have exactly one node for this training data." << std::endl
				  << "Got " << architecture.back() << " instead." << std::endl;
			throw(Gem::Common::gemfony_error_condition(error.str()));
		}

		// Create a local random number generator.
		Gem::Util::GRandom gr(Gem::Util::RNRLOCAL);

		// The dimension of the hypersphere is identical to the number of input nodes
		std::size_t nDim = architecture[0];

		// Create the actual networkData object and attach the architecture
		// Checks the architecture on the way
		boost::shared_ptr<networkData> nD(new networkData());
		std::vector<std::size_t>::const_iterator it;
		std::size_t layerCounter = 0;
		for(it=architecture.begin(); it!=architecture.end(); ++it, ++layerCounter) {
			if(*it == 0) {
				std::ostringstream error;
				error << "In GNeuralNetworkIndividual::createHyperSphereNetworkData(): Error!" << std::endl
					  << "Layer " << layerCounter << "has invalid size " << *it << std::endl;
				throw(Gem::Common::gemfony_error_condition(error.str()));
			}

			nD->push_back(*it);
		}

		double local_radius=1.;

		for(std::size_t datCounter=0; datCounter<nDataSets; datCounter++)
		{
			boost::shared_ptr<trainingSet> tS(new trainingSet());

			local_radius = gr.evenRandom(0,3*radius);
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
					double phi = gr.evenRandom(0,2*M_PI);
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
						angle_collection[i]=gr.evenRandom(0., M_PI);
					}
					angle_collection[nAngles-1]=gr.evenRandom(0., 2.*M_PI); // Range of last angle is [0, 2.*Pi[

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

			nD->addTrainingSet(tS);
		}

		return nD;
	}

	/********************************************************************************************/
	/**
	 * Creates training data where one data set is evenly distributed in the range of [0,1.] in
	 * each dimension, the other centers along the different coordinate axes. It is added here as
	 * a means of testing this neural network individual. The even distribution gets an output
	 * value of 0.01, the "axis-centric" data distribution gets an output value of 0.99. Note that
	 * the creation of training data might take a long time for large dimensions values
	 *
	 * @param architecture The desired architecture of the network
	 * @param nDataSets The number of training sets to create
	 * @return A copy of the networkData struct that has been created, wrapped in a shared_ptr
	 */
	static boost::shared_ptr<networkData> createAxisCentricNetworkData (
			const std::vector<std::size_t>& architecture
		  , const std::size_t& nDataSets
	) {
		// Check the number of supplied layers
		if(architecture.size() < 2) { // We need at least an input- and an output-layer
			std::ostringstream error;
			error << "In GNeuralNetworkIndividual::createAxisCentricNetworkData(): Error!" << std::endl
				  << "Got invalid number of layers: " << architecture.size() << std::endl;
			throw(Gem::Common::gemfony_error_condition(error.str()));
		}

		// Check that the output layer has exactly one node
		if(architecture.back() != 1) {
			std::ostringstream error;
			error << "In GNeuralNetworkIndividual::createAxisCentricNetworkData(): Error!" << std::endl
				  << "The output layer must have exactly one node for this training data." << std::endl
				  << "Got " << architecture.back() << " instead." << std::endl;
			throw(Gem::Common::gemfony_error_condition(error.str()));
		}

		// Create a local random number generator.
		Gem::Util::GRandom gr(Gem::Util::RNRLOCAL);

		// The dimension of the data set is equal to the number of input nodes
		std::size_t nDim = architecture[0];

		// Create the actual networkData object and attach the architecture
		// Checks the architecture on the way
		boost::shared_ptr<networkData> nD(new networkData());
		std::vector<std::size_t>::const_iterator it;
		std::size_t layerCounter = 0;
		for(it=architecture.begin(); it!=architecture.end(); ++it, ++layerCounter) {
			if(*it == 0) {
				std::ostringstream error;
				error << "In GNeuralNetworkIndividual::createAxisCentricNetworkData(): Error!" << std::endl
					  << "Layer " << layerCounter << "has invalid size " << *it << std::endl;
				throw(Gem::Common::gemfony_error_condition(error.str()));
			}

			nD->push_back(*it);
		}

		for(std::size_t dataCounter=0; dataCounter<nDataSets; dataCounter++)
		{
			boost::shared_ptr<trainingSet> tS(new trainingSet());

			// Create even distribution across all dimensions
			if(dataCounter%2 == 0) {
				for(std::size_t dimCounter=0; dimCounter<nDim; dimCounter++) {
					(tS->Input).push_back(gr.evenRandom());
				}
				(tS->Output).push_back(0.01);
			}
  		    // Create entries in a half-cylindrical "cloud" around one axis. The density of
			// this cloud is decreasing with increasing distance from the axis.
			else {
				// Create a test value
				double probeValue = 0.;
				for(std::size_t dimCounter=0; dimCounter<nDim; dimCounter++) probeValue += exp(-5.*gr.evenRandom());

				double functionValue;
				std::vector<double> inputVector(nDim);
				do {
					functionValue = 0.;

					// Create the input vector
					for(std::size_t dimCounter=0; dimCounter<nDim; dimCounter++) {
						inputVector[dimCounter] = gr.evenRandom();
						functionValue += exp(-5*inputVector[dimCounter]);
					}
					functionValue = pow(functionValue, 4.);

				} while(functionValue < probeValue);

				tS->Input = inputVector;
				(tS->Output).push_back(0.99);
			}

			nD->addTrainingSet(tS);
		}

		return nD;
	}

	/********************************************************************************************/
	/**
	 * Creates training data where one data set is evenly distributed above a sin(x) curve, the
	 * other evenly below it. This example only accepts two input nodes.
	 *
	 * @param architecture The desired architecture of the network
	 * @param nDataSets The number of training sets to create
	 * @return A copy of the networkData struct that has been created, wrapped in a shared_ptr
	 */
	static boost::shared_ptr<networkData> createSinNetworkData (
			const std::vector<std::size_t>& architecture
		  , const std::size_t& nDataSets
	) {
		// Check the number of supplied layers
		if(architecture.size() < 2) { // We need at least an input- and an output-layer
			std::ostringstream error;
			error << "In GNeuralNetworkIndividual::createSinNetworkData(): Error!" << std::endl
				  << "Got invalid number of layers: " << architecture.size() << std::endl;
			throw(Gem::Common::gemfony_error_condition(error.str()));
		}

		// Check that the output layer has exactly one node
		if(architecture.back() != 1) {
			std::ostringstream error;
			error << "In GNeuralNetworkIndividual::createSinNetworkData(): Error!" << std::endl
				  << "The output layer must have exactly one node for this training data." << std::endl
				  << "Got " << architecture.back() << " instead." << std::endl;
			throw(Gem::Common::gemfony_error_condition(error.str()));
		}

		// We require the input dimension to be 2
		if(architecture.front() != 2) {
			std::ostringstream error;
			error << "In GNeuralNetworkIndividual::createSinNetworkData(): Error!" << std::endl
				  << "The input layer must have exactly two node for this example." << std::endl
				  << "Got " << architecture.front() << " instead." << std::endl;
			throw(Gem::Common::gemfony_error_condition(error.str()));
		}

		// Create a local random number generator.
		Gem::Util::GRandom gr(Gem::Util::RNRLOCAL);

		// Create the actual networkData object and attach the architecture
		// Checks the architecture on the way
		boost::shared_ptr<networkData> nD(new networkData());
		std::vector<std::size_t>::const_iterator it;
		std::size_t layerCounter = 0;
		for(it=architecture.begin(); it!=architecture.end(); ++it, ++layerCounter) {
			if(*it == 0) {
				std::ostringstream error;
				error << "In GNeuralNetworkIndividual::createSinNetworkData(): Error!" << std::endl
					  << "Layer " << layerCounter << "has invalid size " << *it << std::endl;
				throw(Gem::Common::gemfony_error_condition(error.str()));
			}

			nD->push_back(*it);
		}

		for(std::size_t dataCounter=0; dataCounter<nDataSets; dataCounter++)
		{
			boost::shared_ptr<trainingSet> tS(new trainingSet());

			// create the two test values
			(tS->Input).push_back(gr.evenRandom(-6., 6.)); // x
			(tS->Input).push_back(gr.evenRandom(-6., 6.)); // y

			// Check whether we are below or above the sin function and assign the output value accordingly
			if((tS->Input)[1] > 4.*sin((tS->Input)[0])) {
				(tS->Output).push_back(0.99);
			}
			else {
				(tS->Output).push_back(0.01);
			}

			nD->addTrainingSet(tS);
		}

		return nD;
	}

	/********************************************************************************************/
	/**
	 * Creates a program which in turn creates a program suitable for visualization of optimization
	 * results with the ROOT analysis framework (see http://root.cern.ch for further information).
	 *
	 * @param visFile The name of the file the visualization program should be saved to
	 */
	void writeVisualizationFile(const std::string& visFile) {
		if(visFile == "" || visFile.empty()) {
			std::ostringstream error;
			error << "In GNeuralNetworkIndividual::writeVisualizationFile(const std::string&) : Error" << std::endl
				  << "Received empty file name." << std::endl;
			throw Gem::Common::gemfony_error_condition(error.str());
		}

		std::ofstream visProgram(visFile.c_str());
		if(!visProgram) {
			std::ostringstream error;
			error << "In GNeuralNetworkIndividual::writeVisualizationFile(const std::string&) :" << std::endl
				  << "Attempt to open output file " << visFile << " for writing failed." << std::endl;
			throw Gem::Common::gemfony_error_condition(error.str());
		}

		// The following only makes sense if the input dimension is 2
		if(nD_->getNInputNodes() == 2){
			visProgram  << "/**" << std::endl
			            << " * @file " << visFile << std::endl
			            << " *" << std::endl
			            << " * This program allows to visualize the output of the training example." << std::endl
			            << " * It has been auto-generated by the GNeuralNetworkIndividual class of" << std::endl
			            << " * the GenEvA library" << std::endl
			            << " */" << std::endl
			            << std::endl
			            << "/* Copyright (C) Dr. Ruediger Berlich" << std::endl
						<< " * Copyright (C) Forschungszentrum Karlsruhe GmbH" << std::endl
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
						<< " * g++ -o visualization -I/opt/boost142/include/ visualization.C" << std::endl
						<< " * e.g. on Ubuntu 9.10 (assuming that Boost is installed under /opt" << std::endl
						<< " * in your system). The code should work with virtually any other" << std::endl
						<< " * Linux distribution that supports Boost." << std::endl
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
						<< "#include \"trainedNetwork.hpp\"" << std::endl
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
						<< "          << \"  inside->GetXaxis()->SetRangeUser(-1.,1.);\" << std::endl" << std::endl
						<< "          << \"  inside->GetYaxis()->SetRangeUser(-1.,1.);\" << std::endl" << std::endl
						<< "          << \"  outside->SetMarkerStyle(21);\" << std::endl" << std::endl
						<< "          << \"  outside->SetMarkerSize(0.35);\" << std::endl" << std::endl
						<< "          << \"  outside->SetMarkerColor(17);\" << std::endl" << std::endl
						<< "          << \"  outside->GetXaxis()->SetRangeUser(-1.,1.);\" << std::endl" << std::endl
						<< "          << \"  outside->GetYaxis()->SetRangeUser(-1.,1.);\" << std::endl" << std::endl
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
		else {
			std::ostringstream error;
			error << "In GNeuralNetworkIndividual::writeVisualizationFile(const std::string&) : Warning!" << std::endl
				  << "Request to create visualization program for more than two input dimensions!" << std::endl
				  << "No action taken." << std::endl;
		}

		// Clean up
		visProgram.close();
	}

	/********************************************************************************************/
	/**
	 * Creates a C++ output file for the trained network, suitable for usage in
	 * other projects. If you just want to retrieve the C++ description of the network,
	 * call this function with an empty string "" .
	 *
	 * @param headerFile The name of the header file the network should be saved in
	 */
	void writeTrainedNetwork(const std::string& headerFile) {
		if(headerFile == "" || headerFile.empty()) {
			std::ostringstream error;
			error << "In GNeuralNetworkIndividual::writeTrainedNetwork(const std::string&) : Error" << std::endl
				  << "Received empty file name." << std::endl;
			throw Gem::Common::gemfony_error_condition(error.str());
		}

		std::ofstream header(headerFile.c_str());
		if(!header) {
			std::ostringstream error;
			error << "In GNeuralNetworkIndividual::writeTrainedNetwork(const std::string&) :" << std::endl
				  << "Error writing output file " << headerFile << std::endl;
			throw Gem::Common::gemfony_error_condition(error.str());
		}

		header << "/**" << std::endl
				<< " * @file " << headerFile << std::endl
				<< " *" << std::endl
				<< " * This file represents the results of a feedforward neural network trained" << std::endl
				<< " * using the GenEvA library. It has been auto-generated by the GNeuralNetworkIndividual" << std::endl
				<< " * class." << std::endl
				<< " */" << std::endl
				<< std::endl
				<< "/* Copyright (C) Dr. Ruediger Berlich" << std::endl
				<< " * Copyright (C) Forschungszentrum Karlsruhe GmbH" << std::endl
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

		switch(transferFunction_) {
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

		for(std::size_t i=0; i<nD_->size(); i++) {
			header << "        " << nD_->at(i);
			if(i==nD_->size() - 1)
				header << std::endl;
			else
				header << "," << std::endl;
		}

		std::size_t weightOffset=0;

		header << "      };" << std::endl
				<< "      const std::size_t weightOffset[nLayers] = {" << std::endl
				<< "        " << weightOffset << "," << std::endl;

		weightOffset += 2*(*nD_)[0];
		header << "       " << weightOffset << "," << std::endl;

		for(std::size_t i=1; i<nD_->size()-1; i++) {
			weightOffset += (*nD_)[i]*((*nD_)[i-1]+1);
			header << "        " << weightOffset;

			if(i==nD_->size() - 1)
				header << std::endl;
			else
				header << "," << std::endl;
		}

		header << "      };" << std::endl;

		std::size_t nWeights = 2*(*nD_)[0];
		for(std::size_t i=1; i<nD_->size(); i++) {
			nWeights += (*nD_)[i]*((*nD_)[i-1]+1);
		}

		header << "      const std::size_t nWeights = " << nWeights << ";" << std::endl
				<< "      const double weights[nWeights] = {" << std::endl;

		for(std::size_t i=0; i<nD_->size(); i++) {
			boost::shared_ptr<GDoubleObjectCollection> currentLayer = pc_at<GDoubleObjectCollection>(i);

			for(std::size_t j=0; j<currentLayer->size(); j++) {
				header << "        " << currentLayer->at(j)->value();

				if(i==(nD_->size()-1) && j==(currentLayer->size()-1)) header << std::endl;
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

		// Clean up
		header.close();
	}

protected:
	/********************************************************************************************/
	/**
	 * Loads the data of another GNeuralNetworkIndividual, camouflaged as a GObject
	 *
	 * @param cp A copy of another GNeuralNetworkIndividual, camouflaged as a GObject
	 */
	virtual void load_(const GObject* cp){
		const GNeuralNetworkIndividual *p_load = conversion_cast<GNeuralNetworkIndividual>(cp);

		// Load the parent class'es data
		GParameterSet::load_(cp);

		// Load our local data.
		networkDataFile_ = p_load->networkDataFile_;

		// nD_ is a shared_ptr, hence we need to copy the data itself. We do not do
		// this, if we already have the data present. This happens as we assume that
		// the training data doesn't change.
		if(!nD_) nD_ = boost::shared_ptr<networkData>(new networkData(*(p_load->nD_)));
	}

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
		nD_->resetCurrentIndex();
		boost::optional<boost::shared_ptr<trainingSet> > o;
		while(o = nD_->getNextTrainingSet()) {
			// Retrieve a constant reference to the training data set for faster access
			const trainingSet& tS = **o;

			// The input layer
			std::vector<double> prevResults;
			std::size_t nLayerNodes = (*nD_)[0];
			double nodeResult=0;
			const GDoubleObjectCollection& inputLayer = *(pc_at<GDoubleObjectCollection>(0));
			for(std::size_t nodeCounter=0; nodeCounter<nLayerNodes; nodeCounter++){
				nodeResult=tS.Input[nodeCounter] * inputLayer[2*nodeCounter]->value() - inputLayer[2*nodeCounter+1]->value();
				nodeResult=transfer(nodeResult);
				prevResults.push_back(nodeResult);
			}

			// All other layers
			std::size_t nLayers = this->data.size();
			for(std::size_t layerCounter=1; layerCounter<nLayers; layerCounter++){
				std::vector<double> currentResults;
				nLayerNodes=(*nD_)[layerCounter];
				std::size_t nPrevLayerNodes=(*nD_)[layerCounter-1];
				const GDoubleObjectCollection& currentLayer = *(pc_at<GDoubleObjectCollection>(layerCounter));

				for(std::size_t nodeCounter=0; nodeCounter<nLayerNodes; nodeCounter++){
					// Loop over all nodes of the previous layer
					nodeResult=0.;
					for(std::size_t prevNodeCounter=0; prevNodeCounter<nPrevLayerNodes; prevNodeCounter++){
						nodeResult += prevResults.at(prevNodeCounter) * currentLayer[nodeCounter*(nPrevLayerNodes+1)+prevNodeCounter]->value();
					}
					nodeResult -= currentLayer[nodeCounter*(nPrevLayerNodes+1)+nPrevLayerNodes]->value();
					nodeResult=transfer(nodeResult);
					currentResults.push_back(nodeResult);
				}

				prevResults=currentResults;
			}

			// At this point prevResults should contain the output values of the output layer

			// Calculate the error made and add it to the result
			std::size_t prefResultsSize = prevResults.size();
			for(std::size_t nodeCounter = 0; nodeCounter<prefResultsSize; nodeCounter++){
				result += GSQUARED(prevResults.at(nodeCounter) - tS.Output[nodeCounter]);
			}
		}

		// Let the audience know
		return result;
	}

	/********************************************************************************************/

private:
	/** @brief Default constructor intentionally private and empty */
	GNeuralNetworkIndividual<tF>() :transferFunction_(tF) { /* nothing */ }

	/********************************************************************************************/
	/**
	 * A trap transfer function to capture invalid uses of this class
	 */
	double transfer(const double& value) const {
		std::ostringstream error;
		error << "In GNeuralNetworkIndividual::transfer(): Error!" << std::endl
			  << "Class was instantiated with invalid value for template parameter tF" << std::endl;
		throw(Gem::Common::gemfony_error_condition(error.str()));
	}

	/********************************************************************************************/
	// Local variables
	std::string networkDataFile_; ///< Holds the name of the file with the training data
	boost::shared_ptr<networkData> nD_; ///< Holds the training data
	const transferFunction transferFunction_; ///< Holds the id of the transfer function
};

/************************************************************************************************/
//////////////////////////////////////////////////////////////////////////////////////////////////
/************************************************************************************************/
// Specializations of the transfer function for either SIGMOID or RBF

/** @brief Specialization of the transfer function for case tF==SIGMOID */
template <> double GNeuralNetworkIndividual<SIGMOID>::transfer(const double&) const;
/** @brief Specialization of the transfer function for case tF==RBF */
template <> double GNeuralNetworkIndividual<RBF>::transfer(const double&) const;

/************************************************************************************************/

} /* namespace GenEvA */
} /* namespace Gem */

// Needed for testing purposes
/*************************************************************************************************/
///////////////////////////////////////////////////////////////////////////////////////////////////
/*************************************************************************************************/
/**
 * @brief As the Gem::GenEvA::Gem::GenEvA::GNeuralNetworkIndividual<Gem::GenEvA::SIGMOID> has a private default constructor, we need to provide a
 * specialization of the factory function that creates GStartProjectIndividual objects
 */
template <>
boost::shared_ptr<Gem::GenEvA::GNeuralNetworkIndividual<Gem::GenEvA::SIGMOID> > TFactory_GUnitTests<Gem::GenEvA::GNeuralNetworkIndividual<Gem::GenEvA::SIGMOID> >();

/*************************************************************************************************/
/**
 * @brief As the Gem::GenEvA::Gem::GenEvA::GNeuralNetworkIndividual<Gem::GenEvA::RBF> has a private default constructor, we need to provide a
 * specialization of the factory function that creates GStartProjectIndividual objects
 */
template <>
boost::shared_ptr<Gem::GenEvA::GNeuralNetworkIndividual<Gem::GenEvA::RBF> > TFactory_GUnitTests<Gem::GenEvA::GNeuralNetworkIndividual<Gem::GenEvA::RBF> >();

/*************************************************************************************************/
///////////////////////////////////////////////////////////////////////////////////////////////////
/*************************************************************************************************/

#endif /* GNEURALNETWORKINDIVIDUAL_HPP_ */
