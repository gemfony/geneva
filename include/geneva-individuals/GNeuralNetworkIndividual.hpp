/**
 * @file GNeuralNetworkIndividual.hpp
 */

/*
 * Copyright (C) Gemfony scientific UG (haftungsbeschraenkt)
 *
 * See the AUTHORS file in the top-level directory for a list of authors.
 *
 * Contact: contact [at] gemfony (dot) eu
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
 * http://www.gemfony.eu .
 */

// Global checks, defines and includes needed for all of Geneva
#include "common/GGlobalDefines.hpp"

// Standard header files go here
#include <iostream>
#include <cmath>
#include <sstream>
#include <vector>
#include <fstream>

// Boost header files go here
#include <boost/shared_ptr.hpp>
#include <boost/math/constants/constants.hpp>
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

// Geneva header files go here
#include <common/GCommonEnums.hpp>
#include <common/GExceptions.hpp>
#include <common/GHelperFunctions.hpp>
#include <common/GHelperFunctionsT.hpp>
#include <common/GSingletonT.hpp>
#include <common/GGlobalOptionsT.hpp>
#include <common/GUnitTestFrameworkT.hpp>
#include <common/GLogger.hpp>
#include <common/GFactoryT.hpp>
#include <hap/GRandomT.hpp>
#include <geneva/GDoubleObject.hpp>
#include <geneva/GDoubleGaussAdaptor.hpp>
#include <geneva/GDoubleObjectCollection.hpp>
#include <geneva/GParameterSet.hpp>
#include <geneva/GStdSimpleVectorInterfaceT.hpp>

namespace Gem {
namespace Geneva {

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/
/**
 * A single data set holding the training data of a single training iteration
 */
struct trainingSet
{
	/////////////////////////////////////////////////////////////////////////////
	friend class boost::serialization::access;

   template<typename Archive>
   void load(Archive & ar, const unsigned int) {
      using boost::serialization::make_nvp;

      ar
      & BOOST_SERIALIZATION_NVP(nInputNodes)
      & BOOST_SERIALIZATION_NVP(nOutputNodes);

      if(Input) delete [] Input;
      Input = new double[nInputNodes];

      if(Output) delete [] Output;
      Output = new double[nOutputNodes];

      ar & boost::serialization::make_array(Input, nInputNodes);
      ar & boost::serialization::make_array(Output, nOutputNodes);
   }

   template<typename Archive>
   void save(Archive & ar, const unsigned int) const {
      using boost::serialization::make_nvp;

      ar
      & BOOST_SERIALIZATION_NVP(nInputNodes)
      & BOOST_SERIALIZATION_NVP(nOutputNodes);

      ar & boost::serialization::make_array(Input, nInputNodes);
      ar & boost::serialization::make_array(Output, nOutputNodes);
   }

   BOOST_SERIALIZATION_SPLIT_MEMBER()

	/////////////////////////////////////////////////////////////////////////////

	/** @brief The constructor */
	trainingSet(const std::size_t&, const std::size_t&);
   /** @brief A copy constructor */
   trainingSet(const trainingSet&);
	/** @brief The destructor */
	virtual ~trainingSet();

	/** @brief Assigns another trainingSet's data to this object */
	const trainingSet& operator=(const trainingSet&);
	/** @brief Checks for equality with another trainingSet object */

	bool operator==(const trainingSet&) const;
	/** @brief Checks for inequality with another trainingSet object */
	bool operator!=(const trainingSet&) const;

	/** @brief Checks whether a given expectation is fulfilled. */
	boost::optional<std::string> checkRelationshipWith(
      const trainingSet&
      , const Gem::Common::expectation&
      , const double&
      , const std::string&
      , const std::string&
      , const bool&
	) const;

	/***************************************************************************/
	// Local data

   std::size_t nInputNodes; ///< The number of input nodes
   std::size_t nOutputNodes; ///< The number of output nodes

	double *Input;  ///< Holds the input data
	double *Output; ///< Holds the output data

private:
   /** @brief The default constructor -- intentionally private */
	trainingSet();
};

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/
/**
 * This class holds all necessary information for the training of the neural network individual,
 * including the network's geometry. For intermediate storage on disk, we can serialize the
 * entire object using the Boost.Serialization library. networkData objects can themselves be
 * treated as std::vector. The idea is that the architecture is appended to the object, with
 * the first attached number being the input layer and the last one the output layer. In-between
 * numbers are hidden layers.
 */
class networkData
	:public GStdSimpleVectorInterfaceT<std::size_t>
{
   /////////////////////////////////////////////////////////////////////////////
   friend class boost::serialization::access;

   template<typename Archive>
   void load(Archive & ar, const unsigned int) {
      using boost::serialization::make_nvp;

      ar
      & make_nvp("GStdSimpleVectorInterfaceT_size_t",
            boost::serialization::base_object<GStdSimpleVectorInterfaceT<std::size_t> >(*this))
      & BOOST_SERIALIZATION_NVP(initRange_);

      // Make sure the data vector is empty
      if(data_) {
         for(std::size_t i=0; i<arraySize_; i++) {
            data_[i].reset();
         }
      }
      delete [] data_;

      ar
      & BOOST_SERIALIZATION_NVP(arraySize_);

      data_ = new boost::shared_ptr<trainingSet>[arraySize_];

      ar & boost::serialization::make_array(data_, arraySize_);
   }

   template<typename Archive>
   void save(Archive & ar, const unsigned int) const {
      using boost::serialization::make_nvp;

      ar
      & make_nvp("GStdSimpleVectorInterfaceT_size_t",
            boost::serialization::base_object<GStdSimpleVectorInterfaceT<std::size_t> >(*this))
      & BOOST_SERIALIZATION_NVP(initRange_)
      & BOOST_SERIALIZATION_NVP(arraySize_)
      & boost::serialization::make_array(data_, arraySize_);
   }

   BOOST_SERIALIZATION_SPLIT_MEMBER()

   /////////////////////////////////////////////////////////////////////////////

public:
   /***************************************************************************/
   /** @brief Initialization with the amount of entries */
   explicit networkData(const std::size_t&);
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
      const Gem::Common::expectation&,
      const double&,
      const std::string&,
      const std::string&,
      const bool&
   ) const;

	/** @brief Saves the data of this struct to disc */
	void saveToDisk(const std::string&) const;
	/** @brief Loads training data from the disc */
	void loadFromDisk(const std::string&);

	/** @brief Adds a new training set to the collection, Requires for the network architecture to be defined already */
	void addTrainingSet(boost::shared_ptr<trainingSet>, const std::size_t&);
	/** @brief Retrieves  training set at a given position */
	boost::optional<boost::shared_ptr<trainingSet> > getTrainingSet(const std::size_t&) const;

	/** @brief Retrieves the number of input nodes of this network */
	std::size_t getNInputNodes() const;
	/** @brief Retrieves the number of output nodes of this network */
	std::size_t getNOutputNodes() const;

	/** @brief Saves this data set in ROOT format for visual inspection */
	void toROOT(const std::string&, const double&, const double&);

	/** @brief Allows to check whether an initialization range has been set */
	bool initRangeSet() const;
	/** @brief Allows to set the initialization range */
	void setInitRange(const std::vector<boost::tuple<double, double> >& initRange);
	/** @brief Allows to retrieve the initialization range */
	std::vector<boost::tuple<double, double> > getInitRange() const;

	/** @brief Allows to retrieve a string that describes the network geometry */
	std::string getNetworkGeometryString() const;

	/** @brief Creates a deep clone of this object */
	boost::shared_ptr<networkData> clone() const;

protected:
   /***************************************************************************/
	/** @brief This function is purely virtual in our parent class*/
	virtual void dummyFunction() { /* nothing */ };

private:
   /***************************************************************************/
	/** @brief Default constructor, intentionally private */
   networkData();

	/***************************************************************************/
   /** @brief The size of the training set */
   std::size_t arraySize_;
   /** @brief Holds the individual data items */
	boost::shared_ptr<trainingSet> *data_;

	/** @brief Holds the initialization range in each direction */
	std::vector<boost::tuple<double, double> > initRange_;

	/** @brief Locks access to the clone function */
   mutable boost::mutex m_; ///< Lock get/set operations
};

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/
/** @brief This enum is used to specify the type of training data that should be generated */
enum trainingDataType {TDTNONE=0, HYPERCUBE=1, HYPERSPHERE=2, AXISCENTRIC=3, SINUS=4};
/** @brief Allows to specify whether we want to use a sigmoidal transfer function or a radial basis function */
enum transferFunction {SIGMOID=0, RBF=1};

/******************************************************************************/
/** @brief  Reads a Gem::Geneva::trainingDataType item from a stream */
std::istream& operator>>(std::istream& i, Gem::Geneva::trainingDataType& tdt);
/** @brief Puts a Gem::Geneva::trainingDataType item into a stream */
std::ostream& operator<<(std::ostream& o, const Gem::Geneva::trainingDataType& tdt);
/** @brief Reads a Gem::Geneva::transferFunction item from a stream. */
std::istream& operator>>(std::istream& i, Gem::Geneva::transferFunction& tF);
/** @brief Puts a Gem::Geneva::transferFunction item into a stream. */
std::ostream& operator<<(std::ostream& o, const Gem::Geneva::transferFunction& tF);

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/
// A global singleton giving access to the training data.
// See also the definition of TFactory_GSingletonT<Gem::Geneva::networkData>
typedef Gem::Common::GSingletonT<Gem::Geneva::networkData> GDatStore;
#define GNNTrainingDataStore GDatStore::Instance(0)

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/
// A number of default settings for the factory
const std::string      GNN_DEF_DATAFILE="./Datasets/hyper_cube.dat";
const double           GNN_DEF_ADPROB = 0.05;
const double           GNN_DEF_ADAPTADPROB = 0.1;
const double           GNN_DEF_MINADPROB = 0.05;
const double           GNN_DEF_MAXADPROB = 1.;
const double           GNN_DEF_SIGMA = 0.1;
const double           GNN_DEF_SIGMASIGMA = 0.8;
const double           GNN_DEF_MINSIGMA = 0.01;
const double           GNN_DEF_MAXSIGMA = 0.2;
const double           GNN_DEF_MINVAR = -10.;
const double           GNN_DEF_MAXVAR = 10.;
const transferFunction GNN_DEF_TRANSFER = SIGMOID;

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/
/**
 * With this individual you can use Genevas optimization algorithms instead of the
 * standard back-propagation algorithm to train feed-forward neural networks.
 */
class GNeuralNetworkIndividual
	:public GParameterSet
{
	/////////////////////////////////////////////////////////////////////////////

	friend class boost::serialization::access;

	template<typename Archive>
	void load(Archive & ar, const unsigned int) {
		using boost::serialization::make_nvp;

		ar
		& BOOST_SERIALIZATION_BASE_OBJECT_NVP(GParameterSet);

		// Load the network data from disk
		nD_ = GNNTrainingDataStore; // A glogal singleton
	}

	template<typename Archive>
	void save(Archive & ar, const unsigned int) const {
		using boost::serialization::make_nvp;

		ar
		& BOOST_SERIALIZATION_BASE_OBJECT_NVP(GParameterSet);
	}

	BOOST_SERIALIZATION_SPLIT_MEMBER()

	/////////////////////////////////////////////////////////////////////////////

public:
   /***************************************************************************/
	/** @brief The default constructor */
   GNeuralNetworkIndividual();
	/** @brief A constructor which initializes the individual with a suitable set of network layers */
	GNeuralNetworkIndividual(
      const double& /* min */, const double& /* max */
      , const double& /* sigma */, const double& /* sigmaSigma */
      , const double& /* minSigma */, const double& /* maxSigma */
      , const double& /* adProb */ , const double& /* adaptAdProb */
      , const double& /* minAdProb */, const double& /* maxAdProb */
	);
	/** @brief A standard copy constructor */
	GNeuralNetworkIndividual(const GNeuralNetworkIndividual& cp);
	/** @brief The standard destructor */
	virtual ~GNeuralNetworkIndividual();

	/** @brief A standard assignment operator */
	const GNeuralNetworkIndividual& operator=(const GNeuralNetworkIndividual& cp);

	/** @brief Checks for equality with another GNeuralNetworkIndividual object */
	bool operator==(const GNeuralNetworkIndividual& cp) const;
	/** @brief Checks for inequality with another GNeuralNetworkIndividual object */
	bool operator!=(const GNeuralNetworkIndividual& cp) const;

	/** @brief Checks whether a given expectation for the relationship is fulfilled */
	boost::optional<std::string> checkRelationshipWith(
      const GObject& cp
      , const Gem::Common::expectation& e
      , const double& limit
      , const std::string& caller
      , const std::string& y_name
      , const bool& withMessages
	) const;

	/** @brief Initialization according to user-specifications */
   void init(
      const double& /* min */, const double& /* max */
      , const double& /* sigma */, const double& /* sigmaSigma */
      , const double& /* minSigma */, const double& /* maxSigma */
      , const double& /* adProb */ , const double& /* adaptAdProb */
      , const double& /* minAdProb */, const double& /* maxAdProb */
   );

   /** @brief Sets the type of the transfer function */
   void setTransferFunction(transferFunction tF);
   /** @brief Retrieves the type of the transfer function */
   transferFunction getTransferFunction() const;

	/***************************************************************************/
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
		using namespace Gem::Hap;

		// Check the number of supplied layers
		if(architecture.size() < 2) { // We need at least an input- and an output-layer
		   glogger
		   << "In GNeuralNetworkIndividual::createHyperCubeNetworkData(): Error!" << std::endl
         << "Got invalid number of layers: " << architecture.size() << std::endl
         << GEXCEPTION;
		}

		// Check that the output layer has exactly one node
		if(architecture.back() != 1) {
		   glogger
		   << "In GNeuralNetworkIndividual::createHyperCubeNetworkData(): Error!" << std::endl
         << "The output layer must have exactly one node for this training data." << std::endl
         << "Got " << architecture.back() << " instead." << std::endl
         << GEXCEPTION;
		}

		// Create a local random number generator.
		GRandomT<RANDOMLOCAL> gr_l;

		// Retrieve the number of input- and output nodes for easier reference
      std::size_t nInputNodes  = architecture.front();
      std::size_t nOutputNodes = architecture.back();

		// The dimension of the hyper-cube is identical to the number of input nodes
		std::size_t nDim = nInputNodes;

		// Create the actual networkData object and attach the architecture
		// Checks the architecture on the way
		boost::shared_ptr<networkData> nD(new networkData(nDataSets));
		std::vector<std::size_t>::const_iterator it;
		std::size_t layerCounter = 0;
		for(it=architecture.begin(); it!=architecture.end(); ++it, ++layerCounter) {
			if(*it == 0) {
			   glogger
			   << "In GNeuralNetworkIndividual::createHyperCubeNetworkData(): Error!" << std::endl
            << "Layer " << layerCounter << "has invalid size " << *it << std::endl
            << GEXCEPTION;
			}

			nD->push_back(*it);
		}

		// Create the required data.
		bool outside=false;
		for(std::size_t datCounter=0; datCounter<nDataSets; datCounter++){
			outside=false;
			boost::shared_ptr<trainingSet> tS(new trainingSet(nInputNodes, nOutputNodes));

			for(std::size_t i=0; i<nDim; i++){
				double oneDimRnd = gr_l.uniform_real<double>(-edgelength,edgelength);

				// Need to find at least one dimension outside of the perimeter
				// in order to set the outside flag to true.
				if(oneDimRnd < -edgelength/2. || oneDimRnd > edgelength/2.) outside=true;

				tS->Input[i] = oneDimRnd;
			}

			if(outside) tS->Output[0] = 0.99;
			else tS->Output[0] = 0.01;

			nD->addTrainingSet(tS, datCounter);
		}

		// Make the initialization range known to nD_
		std::vector<boost::tuple<double, double> > initRange;
		initRange.push_back(boost::tuple<double, double>(-edgelength,edgelength)); // x
      initRange.push_back(boost::tuple<double, double>(-edgelength,edgelength)); // y
		nD->setInitRange(initRange);

		return nD;
	}

	/***************************************************************************/
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
		using namespace Gem::Hap;

		// Check the number of supplied layers
		if(architecture.size() < 2) { // We need at least an input- and an output-layer
		   glogger
		   << "In GNeuralNetworkIndividual::createHyperSphereNetworkData(): Error!" << std::endl
         << "Got invalid number of layers: " << architecture.size() << std::endl
         << GEXCEPTION;
		}

		// Check that the output layer has exactly one node
		if(architecture.back() != 1) {
		   glogger
		   << "In GNeuralNetworkIndividual::createHyperSphereNetworkData(): Error!" << std::endl
         << "The output layer must have exactly one node for this training data." << std::endl
         << "Got " << architecture.back() << " instead." << std::endl
         << GEXCEPTION;
		}

		// Create a local random number generator.
		GRandomT<RANDOMLOCAL> gr_l;

      // Retrieve the number of input- and output nodes for easier reference
      std::size_t nInputNodes  = architecture.front();
      std::size_t nOutputNodes = architecture.back();

		// The dimension of the hypersphere is identical to the number of input nodes
		std::size_t nDim = nInputNodes;

		// Create the actual networkData object and attach the architecture
		// Checks the architecture on the way
		boost::shared_ptr<networkData> nD(new networkData(nDataSets));
		std::vector<std::size_t>::const_iterator it;
		std::size_t layerCounter = 0;
		for(it=architecture.begin(); it!=architecture.end(); ++it, ++layerCounter) {
			if(*it == 0) {
			   glogger
			   << "In GNeuralNetworkIndividual::createHyperSphereNetworkData(): Error!" << std::endl
            << "Layer " << layerCounter << "has invalid size " << *it << std::endl
            << GEXCEPTION;
			}

			nD->push_back(*it);
		}

		double local_radius=1.;

		for(std::size_t datCounter=0; datCounter<nDataSets; datCounter++)
		{
			boost::shared_ptr<trainingSet> tS(new trainingSet(nInputNodes, nOutputNodes));

			local_radius = gr_l.uniform_real<double>(3*radius);
			if(local_radius > radius) tS->Output[0] = 0.99;
			else tS->Output[0] = 0.01;

			//////////////////////////////////////////////////////////////////
			// Calculate random Cartesian coordinates for hyper sphere

			// Special cases
			switch(nDim)
			{
			case 1:
				tS->Input[0] = local_radius;
				break;

			case 2:
				{
					double phi = gr_l.uniform_real<double>(2*boost::math::constants::pi<double>());
					tS->Input[0] = local_radius*sin(phi); // x
					tS->Input[1] = local_radius*cos(phi); // y

			      // Make the initialization range known to nD_ . We only do this for 2D-data
			      std::vector<boost::tuple<double, double> > initRange;
			      initRange.push_back(boost::tuple<double, double>(-local_radius,local_radius)); // x
			      initRange.push_back(boost::tuple<double, double>(-local_radius,local_radius)); // y
			      nD->setInitRange(initRange);
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
						angle_collection[i]=gr_l.uniform_real<double>(boost::math::constants::pi<double>());
					}
					angle_collection[nAngles-1]=gr_l.uniform_real<double>(2.*boost::math::constants::pi<double>()); // Range of last angle is [0, 2.*Pi[

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
					for(std::size_t i=0; i<nDim; i++) {
					   tS->Input[i]=cartCoord[i];
					}

				}
				break;
			}

			nD->addTrainingSet(tS, datCounter);
		}

		return nD;
	}

	/***************************************************************************/
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
		using namespace Gem::Hap;

		// Check the number of supplied layers
		if(architecture.size() < 2) { // We need at least an input- and an output-layer
		   glogger
		   << "In GNeuralNetworkIndividual::createAxisCentricNetworkData(): Error!" << std::endl
         << "Got invalid number of layers: " << architecture.size() << std::endl
         << GEXCEPTION;
		}

		// Check that the output layer has exactly one node
		if(architecture.back() != 1) {
		   glogger
		   << "In GNeuralNetworkIndividual::createAxisCentricNetworkData(): Error!" << std::endl
         << "The output layer must have exactly one node for this training data." << std::endl
         << "Got " << architecture.back() << " instead." << std::endl
         << GEXCEPTION;
		}

		// Create a local random number generator.
		GRandomT<RANDOMLOCAL> gr_l;

      // Retrieve the number of input- and output nodes for easier reference
      std::size_t nInputNodes  = architecture.front();
      std::size_t nOutputNodes = architecture.back();

		// The dimension of the data set is equal to the number of input nodes
		std::size_t nDim = nInputNodes;

		// Create the actual networkData object and attach the architecture
		// Checks the architecture on the way
		boost::shared_ptr<networkData> nD(new networkData(nDataSets));
		std::vector<std::size_t>::const_iterator it;
		std::size_t layerCounter = 0;
		for(it=architecture.begin(); it!=architecture.end(); ++it, ++layerCounter) {
			if(*it == 0) {
			   glogger
			   << "In GNeuralNetworkIndividual::createAxisCentricNetworkData(): Error!" << std::endl
            << "Layer " << layerCounter << "has invalid size " << *it << std::endl
            << GEXCEPTION;
			}

			nD->push_back(*it);
		}

		for(std::size_t dataCounter=0; dataCounter<nDataSets; dataCounter++)
		{
			boost::shared_ptr<trainingSet> tS(new trainingSet(nInputNodes, nOutputNodes));

			// Create even distribution across all dimensions
			if(dataCounter%2 == 0) {
				for(std::size_t dimCounter=0; dimCounter<nDim; dimCounter++) {
					tS->Input[dimCounter] = gr_l.uniform_01<double>();
				}
				tS->Output[0]=0.01;
			}
  		    // Create entries in a half-cylindrical "cloud" around one axis. The density of
			// this cloud is decreasing with increasing distance from the axis.
			else {
				// Create a test value
				double probeValue = 0.;
				for(std::size_t dimCounter=0; dimCounter<nDim; dimCounter++) {
				   probeValue += exp(-5.*gr_l.uniform_01<double>());
				}

				double functionValue;
				std::vector<double> inputVector(nDim);
				do {
					functionValue = 0.;

					// Create the input vector
					for(std::size_t dimCounter=0; dimCounter<nDim; dimCounter++) {
						inputVector[dimCounter] = gr_l.uniform_01<double>();
						functionValue += exp(-5*inputVector[dimCounter]);
					}
					functionValue = pow(functionValue, 4.);

				} while(functionValue < probeValue);

				for(std::size_t i=0; i<nDim; i++) {
	            tS->Input[i] = inputVector[i];
				}
				tS->Output[0] = 0.99;
			}

			nD->addTrainingSet(tS, dataCounter);
		}

      // Make the initialization range known to nD_
      std::vector<boost::tuple<double, double> > initRange;
      initRange.push_back(boost::tuple<double, double>(0,1)); // x
      initRange.push_back(boost::tuple<double, double>(0,1)); // y
      nD->setInitRange(initRange);

		return nD;
	}

	/***************************************************************************/
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
		using namespace Gem::Hap;

		// Check the number of supplied layers
		if(architecture.size() < 2) { // We need at least an input- and an output-layer
		   glogger
		   << "In GNeuralNetworkIndividual::createSinNetworkData(): Error!" << std::endl
         << "Got invalid number of layers: " << architecture.size() << std::endl
         << GEXCEPTION;
		}

		// Check that the output layer has exactly one node
		if(architecture.back() != 1) {
		   glogger
		   << "In GNeuralNetworkIndividual::createSinNetworkData(): Error!" << std::endl
         << "The output layer must have exactly one node for this training data." << std::endl
         << "Got " << architecture.back() << " instead." << std::endl
         << GEXCEPTION;
		}

		// We require the input dimension to be 2
		if(architecture.front() != 2) {
		   glogger
		   << "In GNeuralNetworkIndividual::createSinNetworkData(): Error!" << std::endl
         << "The input layer must have exactly two node for this example." << std::endl
         << "Got " << architecture.front() << " instead." << std::endl
         << GEXCEPTION;
		}

		// Create a local random number generator.
		GRandomT<RANDOMLOCAL> gr_l;

      // Retrieve the number of input- and output nodes for easier reference
      std::size_t nInputNodes  = architecture.front();
      std::size_t nOutputNodes = architecture.back();

		// Create the actual networkData object and attach the architecture
		// Checks the architecture on the way
		boost::shared_ptr<networkData> nD(new networkData(nDataSets));
		std::vector<std::size_t>::const_iterator it;
		std::size_t layerCounter = 0;
		for(it=architecture.begin(); it!=architecture.end(); ++it, ++layerCounter) {
			if(*it == 0) {
			   glogger
			   << "In GNeuralNetworkIndividual::createSinNetworkData(): Error!" << std::endl
            << "Layer " << layerCounter << "has invalid size " << *it << std::endl
            << GEXCEPTION;
			}

			nD->push_back(*it);
		}

		for(std::size_t dataCounter=0; dataCounter<nDataSets; dataCounter++)
		{
			boost::shared_ptr<trainingSet> tS(new trainingSet(nInputNodes, nOutputNodes));

			// create the two test values
			tS->Input[0] = gr_l.uniform_real<double>(-6., 6.); // x
			tS->Input[1] = gr_l.uniform_real<double>(-6., 6.); // y

			// Check whether we are below or above the sin function and assign the output value accordingly
			if((tS->Input)[1] > 4.*sin((tS->Input)[0])) {
				tS->Output[0] = 0.99;
			}
			else {
				tS->Output[0] = 0.01;
			}

			nD->addTrainingSet(tS, dataCounter);
		}

      // Make the initialization range known to nD_
      std::vector<boost::tuple<double, double> > initRange;
      initRange.push_back(boost::tuple<double, double>(-6,6)); // x
      initRange.push_back(boost::tuple<double, double>(-6,6)); // y
      nD->setInitRange(initRange);

		return nD;
	}

   /***************************************************************************/
	/**
	 * Creates a data set of the desired type or throws, if that type is not available
	 *
	 * @param type The type of network data to be created
	 * @param outputFile The name of the output training data file
	 * @param architecture_string The desired architecture of the network in std::string format
	 * @param nDataSets The number of data sets to be produced
	 */
	static void createNetworkData(
	      const Gem::Geneva::trainingDataType& t
	     , const std::string& outputFile
	     , const std::string& architecture_string
	     , const std::size_t& nDataSets
	) {
	   // Split the architecture_string as needed. I
	   std::vector<std::size_t> architecture = Gem::Common::splitStringT<std::size_t>(architecture_string, "-");
	   boost::shared_ptr<networkData> nD_ptr;

	   switch(t) {
	   case Gem::Geneva::HYPERCUBE:
	      nD_ptr = GNeuralNetworkIndividual::createHyperCubeNetworkData (
	            architecture
	           , nDataSets
	           , 0.5 // edge-length
	      );

	      // Emit a visualization file, suitable for viewing with ROOT (see http://root.cern.ch)
	      nD_ptr->toROOT(outputFile + ".C", -0.5, 0.5);

	      break;

	   case Gem::Geneva::HYPERSPHERE:
	      nD_ptr = GNeuralNetworkIndividual::createHyperSphereNetworkData (
	            architecture
	           , nDataSets
	           , 0.5 // radius
	      );

	      // Emit a visualization file, suitable for viewing with ROOT (see http://root.cern.ch)
	      nD_ptr->toROOT(outputFile + ".C", -1., 1.);

	      break;

	   case Gem::Geneva::AXISCENTRIC:
	      nD_ptr = GNeuralNetworkIndividual::createAxisCentricNetworkData (
	            architecture
	           , nDataSets
	      );

	      // Emit a visualization file, suitable for viewing with ROOT (see http://root.cern.ch)
	      nD_ptr->toROOT(outputFile + ".C", 0., 1.);

	      break;

	   case Gem::Geneva::SINUS:
	      nD_ptr = GNeuralNetworkIndividual::createSinNetworkData (
	            architecture
	           , nDataSets
	      );

	      // Emit a visualization file, suitable for viewing with ROOT (see http://root.cern.ch)
	      nD_ptr->toROOT(outputFile + ".C", -6., 6.);

	      break;

	   default:
	      { // Error
	         std::ostringstream error;
	         error << "In createDataset(): Error!" << std::endl
	              << "Received invalid data type " << t << std::endl;
	         throw(Gem::Common::gemfony_error_condition(error.str()));
	      }
	      break;
	   }

	   // Write distribution to file
	   nD_ptr->saveToDisk(outputFile);
	}

   /***************************************************************************/
	/** @brief Creates a program used  for the visualization of optimization results */
	void writeVisualizationFile(const std::string& visFile);
	/** @brief Creates a C++ output file for the trained network */
	void writeTrainedNetwork(const std::string& headerFile);

protected:
	/***************************************************************************/
	/** @brief Loads the data of another GNeuralNetworkIndividual */
	virtual void load_(const GObject* cp);
	/** @brief Creates a deep clone of this object */
	virtual GObject* clone_() const;

	/** @brief The actual fitness calculation */
	virtual double fitnessCalculation() OVERRIDE;

private:
	/***************************************************************************/
	/** @brief The transfer function */
	double transfer(const double& value) const;

	/***************************************************************************/
	// Local variables
	transferFunction tF_; ///< The transfer function to be used for the training
	boost::shared_ptr<networkData> nD_; ///< Holds the training data
};

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/
/**
 * A factory for GNeuralNetworkIndividual objects
 */
class GNeuralNetworkIndividualFactory
   : public Gem::Common::GFactoryT<GParameterSet>
{
public:
   /** @brief The standard constructor */
   GNeuralNetworkIndividualFactory(const std::string&);
   /** @brief The destructor */
   virtual ~GNeuralNetworkIndividualFactory();

   /** @brief Sets the type of the transfer function */
   void setTransferFunction(transferFunction tF);
   /** @brief Retrieves the type of the transfer function */
   transferFunction getTransferFunction() const;

protected:
   /** @brief Creates individuals of this type */
   virtual boost::shared_ptr<GParameterSet> getObject_(Gem::Common::GParserBuilder&, const std::size_t&);
   /** @brief Allows to describe local configuration options in derived classes */
   virtual void describeLocalOptions_(Gem::Common::GParserBuilder&);
   /** @brief Allows to act on the configuration options received from the configuration file */
   virtual void postProcess_(boost::shared_ptr<GParameterSet>&);

private:
   /** @brief The default constructor. Intentionally private and undefined */
   GNeuralNetworkIndividualFactory();

   double adProb_;
   double adaptAdProb_;
   double minAdProb_;
   double maxAdProb_;
   double sigma_;
   double sigmaSigma_;
   double minSigma_;
   double maxSigma_;
   double minVar_;
   double maxVar_;

   transferFunction tF_;
};

/******************************************************************************/

} /* namespace Geneva */
} /* namespace Gem */

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/

namespace Gem {
namespace Common {

// A global store for network configuration data
typedef GSingletonT<GGlobalOptionsT<std::string> > GNNOptStore;
#define GNeuralNetworkOptions GNNOptStore::Instance(0)

// A factory function for networkData objects, used by GSingletonT
template <> boost::shared_ptr<Gem::Geneva::networkData> TFactory_GSingletonT();

} /* namespace Common */
} /* namespace Gem */

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/
// Needed for testing purposes

#ifdef GEM_TESTING

/**
 * @brief As the Gem::Geneva::GNeuralNetworkIndividual has a meaningless default
 * constructor, we need to provide a specialization of the factory function that creates
 * GNeuralNetworkIndividual objects
 */
template <>
boost::shared_ptr<Gem::Geneva::GNeuralNetworkIndividual> TFactory_GUnitTests<Gem::Geneva::GNeuralNetworkIndividual>();

#endif /* GEM_TESTING */


/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/

BOOST_CLASS_EXPORT_KEY(Gem::Geneva::trainingSet)
BOOST_CLASS_EXPORT_KEY(Gem::Geneva::networkData)
BOOST_CLASS_EXPORT_KEY(Gem::Geneva::GNeuralNetworkIndividual)

/******************************************************************************/

#endif /* GNEURALNETWORKINDIVIDUAL_HPP_ */
