/********************************************************************************
 *
 * This file is part of the Geneva library collection. The following license
 * applies to this file:
 *
 * ------------------------------------------------------------------------------
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *    http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 * ------------------------------------------------------------------------------
 *
 * Note that other files in the Geneva library collection may use a different
 * license. Please see the licensing information in each file.
 *
 ********************************************************************************
 *
 * Geneva was started by Dr. Rüdiger Berlich and was later maintained together
 * with Dr. Ariel Garcia under the auspices of Gemfony scientific. For further
 * information on Gemfony scientific, see http://www.gemfomy.eu .
 *
 * The majority of files in Geneva was released under the Apache license v2.0
 * in February 2020.
 *
 * See the NOTICE file in the top-level directory of the Geneva library
 * collection for a list of contributors and copyright information.
 *
 ********************************************************************************/

#include <geneva-individuals/GNeuralNetworkIndividual.hpp>

BOOST_CLASS_EXPORT_IMPLEMENT(Gem::Geneva::trainingSet)
BOOST_CLASS_EXPORT_IMPLEMENT(Gem::Geneva::networkData)
BOOST_CLASS_EXPORT_IMPLEMENT(Gem::Geneva::GNeuralNetworkIndividual)

namespace Gem {
namespace Geneva {

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/
/**
 * The (private) default constructor. It is only needed for (de-)serialization
 * purposes.
 */
trainingSet::trainingSet()
	: nInputNodes(0), nOutputNodes(0), Input(nullptr), Output(nullptr) { /* nothing */ }

/******************************************************************************/
/**
 * Initialization with the number of nodes
 */
trainingSet::trainingSet(
	const std::size_t &nInput, const std::size_t &nOutput
)
	: nInputNodes(nInput), nOutputNodes(nOutput), Input(new double[nInput]), Output(new double[nOutput]) {
	// Make sure the arrays are properly initialized
	for (std::size_t i = 0; i < nInput; i++) {
		Input[i] = 0.;
	}
	for (std::size_t o = 0; o < nOutput; o++) {
		Output[o] = 0.;
	}
}

/******************************************************************************/
/**
 * A copy constructor
 */
trainingSet::trainingSet(const trainingSet &cp)
	: nInputNodes(0), nOutputNodes(0), Input(nullptr), Output(nullptr) {
	Gem::Common::copyArrays(cp.Input, Input, cp.nInputNodes, nInputNodes);
	Gem::Common::copyArrays(cp.Output, Output, cp.nOutputNodes, nOutputNodes);
}

/******************************************************************************/
/**
 * The destructor.
 */
trainingSet::~trainingSet() {
	if (Input) {
		Gem::Common::g_array_delete(Input);
	}
	if (Output) {
		Gem::Common::g_array_delete(Output);
	}
}

/******************************************************************************/
/**
 * Assigns another trainingSet's data to this object
 *
 * @param cp A copy of another trainingSet object
 * @return A constant reference to this object
 */
trainingSet &trainingSet::operator=(const trainingSet &cp) {
	Gem::Common::copyArrays(cp.Input, Input, cp.nInputNodes, nInputNodes);
	Gem::Common::copyArrays(cp.Output, Output, cp.nOutputNodes, nOutputNodes);

	return *this;
}

/******************************************************************************/
/**
 * Searches for compliance with expectations with respect to another object
 * of the same type
 *
 * @param cp A constant reference to another GObject object
 * @param e The expected outcome of the comparison
 * @param limit The maximum deviation for floating point values (important for similarity checks)
 */
void trainingSet::compare(
	const trainingSet &cp, const Gem::Common::expectation &e, const double &limit
) const {
	using namespace Gem::Common;

	Gem::Common::GToken token("trainingSet", e);

	// Compare our local data
	Gem::Common::compare_t(IDENTITY(nInputNodes, cp.nInputNodes), token);
	Gem::Common::compare_t(IDENTITY(nOutputNodes, cp.nOutputNodes), token);

	for (std::size_t i = 0; i < nInputNodes; i++) {
		Gem::Common::compare_t(IDENTITY(Input[i], cp.Input[i]), token);
	}

	for (std::size_t i = 0; i < nInputNodes; i++) {
		Gem::Common::compare_t(IDENTITY(Output[i], cp.Output[i]), token);
	}

	// React on deviations from the expectation
	token.evaluate();
}

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/
/**
 * The default constructor. Private, as it is only needed for (de-)serialization
 * purposes.
 */
networkData::networkData()
	: arraySize_(0), data_(nullptr) { /* nothing */ }

/******************************************************************************/
/**
 * Initialization with the amount of entries
 *
 * @param arraySize The desired size of the array
 */
networkData::networkData(const std::size_t &arraySize)
	: Gem::Common::GPODVectorT<std::size_t>(), arraySize_(arraySize),
	data_(new std::shared_ptr <trainingSet>[arraySize_]) { /* nothing */ }

/******************************************************************************/
/**
 * Initializes the object with data from a file
 *
 * @param networkDataFile The name of a file holding the training data
 */
networkData::networkData(const std::string &networkDataFile)
	: Gem::Common::GPODVectorT<std::size_t>(), arraySize_(0),
	data_(nullptr) {
	this->loadFromDisk(networkDataFile);
}

/******************************************************************************/
/**
 * Initializes with data from another networkData object
 *
 * @param cp A copy of another networkData object
 */
networkData::networkData(const networkData &cp)
	: Gem::Common::GPODVectorT<std::size_t>(cp), arraySize_(0),
	data_(nullptr) {
	// Make sure the local data is copied
	Gem::Common::copySmartPointerArrays(cp.data_, data_, cp.arraySize_, arraySize_);
}

/******************************************************************************/
/**
 * A standard destructor.
 */
networkData::~networkData() {
	// Make sure the data vector is empty
	if (data_) {
		for (std::size_t i = 0; i < arraySize_; i++) {
			data_[i].reset();
		}
	}
	Gem::Common::g_array_delete(data_);
}

/******************************************************************************/
/**
 * Copies the data of another networkData object into this object, using one of Gemfony's
 * utility functions.
 *
 * @param cp A copy of another networkData object
 * @return A constant reference to this object
 */
networkData &networkData::operator=(const networkData &cp) {
	// Make sure the local data is copied
	Gem::Common::copySmartPointerArrays(cp.data_, data_, cp.arraySize_, arraySize_);
	Gem::Common::GPODVectorT<std::size_t>::operator=(cp);
	return *this;
}

/******************************************************************************/
/**
 * Searches for compliance with expectations with respect to another object
 * of the same type
 *
 * @param cp A constant reference to another networkData object object
 * @param e The expected outcome of the comparison
 * @param limit The maximum deviation for floating point values (important for similarity checks)
 */
void networkData::compare(
	const networkData &cp, const Gem::Common::expectation &e, const double &limit
) const {
	Gem::Common::GToken token("networkData", e);

	// Compare our local data
	Gem::Common::compare_t(IDENTITY(arraySize_, cp.arraySize_), token);
	Gem::Common::compare_t(IDENTITY(this->m_data_cnt, cp.m_data_cnt), token);

	// React on deviations from the expectation
	token.evaluate();
}

/******************************************************************************/
/**
 * Saves the data of this struct to disc
 *
 * @param fileName The name of the file that data should be saved to
 */
void networkData::saveToDisk(const std::string &networkDataFile) const {
	std::ofstream trDat(networkDataFile);

	if (not trDat) {
		throw gemfony_exception(
			g_error_streamer(DO_LOG,  time_and_place)
				<< "In networkData::saveToDisk(const std::string&) : Error!" << std::endl
				<< "Data file " << networkDataFile << " could not be opened for writing." << std::endl
		);
	}

	// Load the data, using the Boost.Serialization library
	{
		const networkData *local = this;
		boost::archive::xml_oarchive oa(trDat);
		oa << boost::serialization::make_nvp("networkData", local);
	} // Explicit scope at this point is essential so that ia's destructor is called

	trDat.close();
}

/******************************************************************************/
/**
 * Loads training data from the disc
 *
 * @param fileName The name of the file from which data should be loaded
 */
void networkData::loadFromDisk(const std::string &networkDataFile) {
	networkData *nD;

	std::ifstream trDat(networkDataFile.c_str());

	if (not trDat) {
		std::ostringstream error;
		error << "In networkData::loadFromDisk(const std::string&):" << std::endl
				<< "Data file " << networkDataFile << " could not be opened for reading." << std::endl;

		if (not std::filesystem::exists(networkDataFile.c_str())) {
			error << "File does not exist." << std::endl;
		}

		throw gemfony_exception(
			g_error_streamer(DO_LOG,  time_and_place)
				<< error.str() << std::endl
		);
	}

	// Load the data into nD, using the Boost.Serialization library
	{
		boost::archive::xml_iarchive ia(trDat);
		ia >> boost::serialization::make_nvp("networkData", nD);
	} // Explicit scope at this point is essential so that ia's destructor is called

	trDat.close();

	// Copy the data over, using our own operator=()
	*this = *nD;

	// Clean up
	Gem::Common::g_delete(nD);
}

/******************************************************************************/
/**
 * Adds a new training set to the collection. Note that the training set isn't
 * cloned, simply a copy of the smart pointer is stored in the internal array.
 *
 * @param tS A std::shared_ptr<trainingSet> object, pointing to a training set
 * @param pos The position, in which the data set should be stored.
 */
void networkData::addTrainingSet(
	std::shared_ptr < trainingSet > tS, const
std::size_t &pos
) {
	if(pos >= arraySize_) {
		throw gemfony_exception(
			g_error_streamer(DO_LOG,  time_and_place)
				<< "In networkData::addTrainingSet(): Error!" << std::endl
				<< "pos = " << pos << " exceeds end of array (size = " << arraySize_ << ")" << std::endl
		);
	}
	data_[pos] =
		tS;
}

/******************************************************************************/
/**
 * Retrieves a training set at a given position. If the position exceeds the size of the array,
 * a boost::optional object is returned which evaluates to "false".
 *
 * @param pos The position from which an item should be retreived
 * @return The training set at the requested position (or a boost::optional object which evaluates to "false")
 */
boost::optional<std::shared_ptr < trainingSet>> networkData::getTrainingSet(
	const std::size_t &pos
) const {
	if (pos >= arraySize_) {
		return boost::optional<std::shared_ptr < trainingSet>> (); // amounts to "false"
	} else {
		return boost::optional<std::shared_ptr < trainingSet>> (data_[pos]);
	}
}

/******************************************************************************/
/**
 * Retrieves the number of input nodes of this network
 *
 * @return The number of input nodes of this network
 */
std::size_t networkData::getNInputNodes() const {
	return this->front();
}

/******************************************************************************/
/**
 * Retrieves the number of output nodes of this network
 *
 * @return The number of output nodes of this network
 */
std::size_t networkData::getNOutputNodes() const {
	return this->back();
}

/******************************************************************************/
/**
 * Saves this data set in ROOT format for visual inspection. It assumes that the input dimension
 * is 2 and the output dimension is 1. It will generate two distributions that will be coloured
 * differently -- one with output < 0.5, the other with output >= 0.5.
 *
 * @param outputFile The name of the file used for the visualization of the input data
 * @param min The minimum value of the distribution to be displayed
 * @param max The maximum value of the distribution to be displayed
 */
void networkData::toROOT(
	const std::string &outputFile, const double &min, const double &max
) {
	// Check that we have a matching number of input nodes
	if (getNInputNodes() != 2 || getNOutputNodes() != 1) {
		glogger
			<< "In networkData::toRoot(): Warning!" << std::endl
			<< "Got inappropriate number of input and/or output nodes: " << getNInputNodes() << "/" << getNOutputNodes() << std::endl
			<< "We need 2/1. The function will return without further action." << std::endl
			<< GWARNING;
		return;
	}

	std::size_t entries1 = 0, entries2 = 0;
	std::ofstream of(outputFile);

	of
		<< "{" << std::endl
		<< "  gROOT->Reset();" << std::endl
		<< "  gStyle->SetCanvasColor(0);" << std::endl
		<< "  gStyle->SetStatBorderSize(1);" << std::endl
		<< "  gStyle->SetOptStat(0);" << std::endl
		<< std::endl
		<< "  TCanvas *cc = new TCanvas(\"cc\", \"cc\",0,0,1024,1024);" << std::endl
		<< std::endl
		<< "  TPaveLabel* canvasTitle = new TPaveLabel(0.1,0.95,0.9,0.99, \"Original training data\");" << std::endl
		<< "  canvasTitle->Draw();" << std::endl
		<< std::endl
		<< "  TPad* graphPad = new TPad(\"Graphs\", \"Graphs\", 0.01, 0.01, 0.99, 0.94);" << std::endl
		<< "  graphPad->Draw();" << std::endl
		<< "  graphPad->Divide(1,1);" << std::endl
		<< std::endl
		<< "  double xarr1[" << arraySize_ << "], yarr1[" << arraySize_ << "], xarr2[" << arraySize_ << "], yarr2[" <<
		arraySize_ << "];" << std::endl
		<< std::endl
		<< "  // Filling the data sets" << std::endl;

	for (std::size_t i = 0; i < arraySize_; i++) {
		if (data_[i]->Output[0] < 0.5) {
			of
				<< "  xarr1[" << entries1 << "] = " << data_[i]->Input[0] << ";" << std::endl
				<< "  yarr1[" << entries1 << "] = " << data_[i]->Input[1] << ";" << std::endl;
			entries1++;
		}
		else {
			of
				<< "  xarr2[" << entries2 << "] = " << data_[i]->Input[0] << ";" << std::endl
				<< "  yarr2[" << entries2 << "] = " << data_[i]->Input[1] << ";" << std::endl;
			entries2++;
		}
	}

	of
		<< std::endl
		<< "  // Setting remaining entries to 0" << std::endl
		<< "  for(std::size_t i=" << entries1 << "; i<" << arraySize_ << "; i++) {" << std::endl
		<< "    xarr1[i] = 0.;" << std::endl
		<< "    yarr1[i] = 0.;" << std::endl
		<< "  }" << std::endl
		<< "  for(std::size_t i=" << entries2 << "; i<" << arraySize_ << "; i++) {" << std::endl
		<< "    xarr2[i] = 0.;" << std::endl
		<< "    yarr2[i] = 0.;" << std::endl
		<< "  }" << std::endl
		<< std::endl
		<< "  // Creation of suitable TGraph objects" << std::endl
		<< "  TGraph *gr1 = new TGraph(" << entries1 << ", xarr1, yarr1);" << std::endl
		<< "  TGraph *gr2 = new TGraph(" << entries2 << ", xarr2, yarr2);" << std::endl
		<< std::endl
		<< "  gr1->SetMarkerColor(17);" << std::endl
		<< "  gr2->SetMarkerColor(14);" << std::endl
		<< std::endl
		<< "  gr1->SetMarkerStyle(21);" << std::endl
		<< "  gr2->SetMarkerStyle(21);" << std::endl
		<< std::endl
		<< "  gr1->SetMarkerSize(0.35);" << std::endl
		<< "  gr2->SetMarkerSize(0.35);" << std::endl
		<< std::endl
		<< "  gr2->GetXaxis()->SetLimits(" << min << ", " << max << ");" << std::endl
		<< "  gr2->GetYaxis()->SetRangeUser(" << min << ", " << max << ");" << std::endl
		<< std::endl
		<< "  // Do the drawing" << std::endl
		<< "  graphPad->cd(1);" << std::endl
		<< "  gr2->Draw(\"AP\");" << std::endl
		<< "  gr1->Draw(\"P,same\");" << std::endl
		<< "}" << std::endl;

	of.close();
}

/******************************************************************************/
/**
 * Allows to check whether an initialization range has been set
 */
bool networkData::initRangeSet() const {
	return not initRange_.empty();
}

/******************************************************************************/
/**
 * Allows to set the initialization range
 */
void networkData::setInitRange(const std::vector<std::tuple<double, double>> &initRange) {
	initRange_ = initRange;
}

/******************************************************************************/
/**
 * Allows to retrieve the initialization range
 */
std::vector<std::tuple<double, double>> networkData::getInitRange() const {
	return initRange_;
}

/******************************************************************************/
/**
 * Allows to retrieve a string that describes the network geometry
 */
std::string networkData::getNetworkGeometryString() const {
	std::ostringstream result;

	for (std::size_t i = 0; i < this->size() - 1; i++) {
		result << this->at(i) << "-";
	}
	result << this->at(this->size() - 1);
	return result.str();
}

/******************************************************************************/
/**
 * Creates a deep clone of this object
 */
std::shared_ptr <networkData> networkData::clone() const {
	// Lock access to this function
	std::unique_lock<std::mutex> lock(m_);
	std::shared_ptr <networkData> result(new networkData(*this));
	return result;
}

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/
/**
 * Reads a Gem::Geneva::trainingDataType item from a stream. Needed so we
 * can use boost::program_options to read trainingDataType data.
 *
 * @param i The stream the item should be read from
 * @param tdt The item read from the stream
 * @return The std::istream object used to read the item from
 */
std::istream &operator>>(std::istream &i, Gem::Geneva::trainingDataType &tdt) {
	Gem::Common::ENUMBASETYPE tmp;
	i >> tmp;

#ifdef DEBUG
	tdt = boost::numeric_cast<Gem::Geneva::trainingDataType>(tmp);
#else
	tdt = static_cast<Gem::Geneva::trainingDataType>(tmp);
#endif /* DEBUG */

	return i;
}

/******************************************************************************/
/**
 * Puts a Gem::Geneva::trainingDataType item into a stream. Needed so we
 * can use boost::program_options to output trainingDataType data.
 *
 * @param o The ostream the item should be added to
 * @param tdt the item to be added to the stream
 * @return The std::ostream object used to add the item to
 */
std::ostream &operator<<(std::ostream &o, const Gem::Geneva::trainingDataType &tdt) {
	Gem::Common::ENUMBASETYPE tmp = static_cast<Gem::Common::ENUMBASETYPE>(tdt);
	o << tmp;
	return o;
}

/******************************************************************************/
/**
 * Reads a Gem::Geneva::transferFunction item from a stream. Needed so we
 * can use boost::program_options to read transferFunction data.
 *
 * @param i The stream the item should be read from
 * @param tF The item read from the stream
 * @return The std::istream object used to read the item from
 */
std::istream &operator>>(std::istream &i, Gem::Geneva::transferFunction &tF) {
	Gem::Common::ENUMBASETYPE tmp;
	i >> tmp;

#ifdef DEBUG
	tF = boost::numeric_cast<Gem::Geneva::transferFunction>(tmp);
#else
	tF = static_cast<Gem::Geneva::transferFunction>(tmp);
#endif /* DEBUG */

	return i;
}

/******************************************************************************/
/**
 * Puts a Gem::Geneva::transferFunction item into a stream. Needed so we
 * can use boost::program_options to output transferFunction data.
 *
 * @param o The ostream the item should be added to
 * @param tF the item to be added to the stream
 * @return The std::ostream object used to add the item to
 */
std::ostream &operator<<(std::ostream &o, const Gem::Geneva::transferFunction &tF) {
	Gem::Common::ENUMBASETYPE tmp = static_cast<Gem::Common::ENUMBASETYPE>(tF);
	o << tmp;
	return o;
}

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/
/**
 * The default constructor
 */
GNeuralNetworkIndividual::GNeuralNetworkIndividual()
	: tF_(GNN_DEF_TRANSFER), nD_(GNNTrainingDataStore) { /* nothing */ }

/******************************************************************************/
/**
 * A constructor which initializes the individual with a suitable set of network layers. It
 * also loads the training data from file.
 *
 * @param architecture Holds the number of nodes in the input layer, hidden(1/2) layer and output layer
 * @param min The minimum value of random numbers used for initialization of the network layers
 * @param max The maximum value of random numbers used for initialization of the network layers
 * @param sigma The sigma used for gauss adaptors
 * @param sigmaSigma Used for sigma adaption
 * @oaram minSigma The minimum allowed value for sigma
 * @param maxSigma The maximum allowed value for sigma
 */
GNeuralNetworkIndividual::GNeuralNetworkIndividual(
	const double &min, const double &max, const double &sigma, const double &sigmaSigma, const double &minSigma,
	const double &maxSigma, const double &adProb, const double &adaptAdProb, const double &minAdProb,
	const double &maxAdProb
)
	: tF_(GNN_DEF_TRANSFER), nD_(GNNTrainingDataStore) {
	this->init(
		min, max, sigma, sigmaSigma, minSigma, maxSigma, adProb, adaptAdProb, minAdProb, maxAdProb
	);
}

/******************************************************************************/
/**
 * A standard copy constructor
 *
 * @param cp A copy of another GNeuralNetworkIndividual object
 */
GNeuralNetworkIndividual::GNeuralNetworkIndividual(const GNeuralNetworkIndividual &cp)
	: GParameterSet(cp), tF_(cp.tF_), nD_(GNNTrainingDataStore) // We want a single source for the training data
{ /* nothing */ }

/******************************************************************************/
/**
 * The standard destructor
 */
GNeuralNetworkIndividual::~GNeuralNetworkIndividual() { /* nothing */   }

/******************************************************************************/
/**
 * Searches for compliance with expectations with respect to another object
 * of the same type
 *
 * @param cp A constant reference to another GObject object
 * @param e The expected outcome of the comparison
 * @param limit The maximum deviation for floating point values (important for similarity checks)
 */
void GNeuralNetworkIndividual::compare_(
	const GObject &cp, const Gem::Common::expectation &e, const double &limit
) const {
	using namespace Gem::Common;

	// Check that we are dealing with a GNeuralNetworkIndividual reference independent of this object and convert the pointer
	const GNeuralNetworkIndividual *p_load = Gem::Common::g_convert_and_compare<GObject, GNeuralNetworkIndividual>(cp, this);

	GToken token("GNeuralNetworkIndividual", e);

	// Compare our parent data ...
	Gem::Common::compare_base_t<GParameterSet>(*this, *p_load, token);

	// ... and then the local data
	compare_t(IDENTITY(tF_, p_load->tF_), token);

	// React on deviations from the expectation
	token.evaluate();
}

/******************************************************************************/
/**
 * A function which initializes the individual with a suitable set of network
 * layers, according to user-specifications.
 *
 * @param min The minimum value of random numbers used for initialization of the network layers
 * @param max The maximum value of random numbers used for initialization of the network layers
 * @param sigma The sigma used for gauss adaptors
 * @param sigmaSigma Used for sigma adaption
 * @oaram minSigma The minimum allowed value for sigma
 * @param maxSigma The maximum allowed value for sigma
 * @param adProb The adaption probability in Evolutionary Algorithms
 */
void GNeuralNetworkIndividual::init(
	const double &min, const double &max, const double &sigma, const double &sigmaSigma, const double &minSigma,
	const double &maxSigma, const double &adProb, const double &adaptAdProb, const double &minAdProb,
	const double &maxAdProb
) {
	// Make sure the individual is empty
	this->clear();

#ifdef DEBUG
	if(not nD_) {
		throw gemfony_exception(
			g_error_streamer(DO_LOG,  time_and_place)
				<< "In GNeuralNetworkIndividual::init([...]): Error!" << std::endl
				<< "No network data appears to have been registered." << std::endl
		);
	}
#endif /* DEBUG */

	// Set up our local data structures

	// Check the architecture we've been given and create the layers
	std::size_t nLayers = nD_->size();

	if (nLayers < 2) { // Two layers are required at the minimum (3 and 4 layers are useful)
		throw gemfony_exception(
			g_error_streamer(DO_LOG,  time_and_place)
				<< "In GNeuralNetworkIndividual::init([...]): Error!" << std::endl
				<< "Invalid number of layers supplied (" << nLayers << ")." << std::endl
				<< "Did you set up the network architecture ?" << std::endl
		);
	}

	networkData::iterator layerIterator;
	std::size_t layerNumber = 0;
	std::size_t nNodes = 0;
	std::size_t nNodesPrevious = 0;

	// Access to uniformly distributed doubke random values
	std::uniform_real_distribution<double> uniform_real_distribution(min, max);

	// Set up the architecture
	for (layerIterator = nD_->begin(); layerIterator != nD_->end(); ++layerIterator) {
		if (*layerIterator) { // Add the next network layer to this class, if possible
			nNodes = *layerIterator;

			// Set up a GDoubleObjectCollection
			std::shared_ptr <GDoubleObjectCollection> gdoc(new GDoubleObjectCollection());

			// Add GDoubleObject objects
			for (std::size_t i = 0; i < (layerNumber == 0 ? 2 * nNodes : nNodes * (nNodesPrevious + 1)); i++) {
				// Set up a GDoubleObject object, initializing it with random data
				std::shared_ptr <GDoubleObject> gd_ptr(new GDoubleObject(uniform_real_distribution(m_gr)));

				// Set up an adaptor
				std::shared_ptr <GDoubleGaussAdaptor> gdga(new GDoubleGaussAdaptor(sigma, sigmaSigma, minSigma, maxSigma));
				gdga->setAdaptionProbability(adProb);
				gdga->setAdaptAdProb(adaptAdProb);
				gdga->setAdProbRange(minAdProb, maxAdProb);

				// Register it with the GDoubleObject object
				gd_ptr->addAdaptor(gdga);

				// Register the GDoubleObject object with the collection
				gdoc->push_back(gd_ptr);
			}

			// Make the parameter collection known to this individual
			this->m_data_cnt.push_back(gdoc);

			nNodesPrevious = nNodes;
			layerNumber++;
		} else {
			throw gemfony_exception(
				g_error_streamer(DO_LOG,  time_and_place)
					<< "In GNeuralNetworkIndividual::init([...]): Error!" << std::endl
					<< "Found invalid number of nodes in layer: " << *layerIterator << std::endl
					<< "Did you set up the network architecture ?" << std::endl
			);
		}
	}
}


/******************************************************************************/
/**
 * Sets the type of the transfer function
 */
void GNeuralNetworkIndividual::setTransferFunction(transferFunction tF) {
	tF_ = tF;
}

/******************************************************************************/
/**
 * Retrieves the type of the transfer function
 */
transferFunction GNeuralNetworkIndividual::getTransferFunction() const {
	return tF_;
}

/******************************************************************************/
/**
 * Creates a program which in turn creates a program suitable for visualization of optimization
 * results with the ROOT analysis framework (see http://root.cern.ch for further information).
 *
 * @param visFile The name of the file the visualization program should be saved to
 */
void GNeuralNetworkIndividual::writeVisualizationFile(const std::string &visFile) {
	if (visFile == "" || visFile.empty()) {
		throw gemfony_exception(
			g_error_streamer(DO_LOG,  time_and_place)
				<< "In GNeuralNetworkIndividual::writeVisualizationFile(const std::string&) : Error" << std::endl
				<< "Received empty file name." << std::endl
		);
	}

	std::ofstream visProgram(visFile);
	if (not visProgram) {
		throw gemfony_exception(
			g_error_streamer(DO_LOG,  time_and_place)
				<< "In GNeuralNetworkIndividual::writeVisualizationFile(const std::string&) :" << std::endl
				<< "Attempt to open output file " << visFile << " for writing failed." << std::endl
		);
	}

	// The following only makes sense if the input dimension is 2
	if (nD_->getNInputNodes() == 2) {
		double x_low = 0., x_high = 1.;
		double y_low = 0., y_high = 1.;

		// Retrieve information about the initialization range
		// We only act if initialization ranges have been registered.
		// If not, than the above default values will be used.
		std::vector<std::tuple<double, double>> initRange = nD_->getInitRange();
		if (initRange.size() == 2) {
			x_low = std::get<0>(initRange.at(0));
			x_high = std::get<1>(initRange.at(0));
			y_low = std::get<0>(initRange.at(1));
			y_high = std::get<1>(initRange.at(1));
		}

		// Write the header
		visProgram
			<< "/**" << std::endl
			<< " * @file visualization.C" << std::endl
			<< " *" << std::endl
			<< " * This program allows to visualize the output of the training example." << std::endl
			<< " * It has been auto-generated by the GNeuralNetworkIndividual class of" << std::endl
			<< " * the Geneva library" << std::endl
			<< " *" << std::endl
			<< " * Can be compiled with a command similar to" << std::endl
			<< " * g++ -o visualization -I/opt/boost155/include/ visualization.C" << std::endl
			<< " * e.g. on Ubuntu 14.04 (assuming that Boost is installed under /opt/boost155" << std::endl
			<< " * in your system). The code should work with virtually any other" << std::endl
			<< " * Linux distribution that supports Boost." << std::endl
			<< " */" << std::endl
			<< std::endl
			<< "/*" << std::endl
			<< " * This file is part of the Geneva library collection." << std::endl
			<< " * The same license applies to this generated file." << std::endl
			<< " */" << std::endl
			<< std::endl
			<< std::endl
			<< "#include <iostream>" << std::endl
			<< "#include <sstream>" << std::endl
			<< "#include <fstream>" << std::endl
			<< "#include <vector>" << std::endl
			<< "#include <random>" << std::endl
			<< std::endl
			<< "#include <boost/cstdint.hpp>" << std::endl
			<< std::endl
			<< "#include \"trainedNetwork.hpp\"" << std::endl
			<< std::endl
			<< "const std::uint32_t MAXPOINTS=20000;" << std::endl
			<< std::endl
			<< "using namespace Gem::NeuralNetwork;" << std::endl
			<< std::endl
			<< "int main(int argc, char**argv){" << std::endl
			<< "  std::string geometry = \"" << nD_->getNetworkGeometryString() << "\";" << std::endl
			<< "  double x_low = " << x_low << ", x_high = " << x_high << ";" << std::endl
			<< "  double y_low = " << y_low << ", y_high = " << y_high << ";" << std::endl
			<< std::endl
			<< "  boost::lagged_fibonacci607 lf(123);" << std::endl
			<< std::endl
			<< "  double x=0., y=0., result=0;" << std::endl
			<< "  std::vector<double> in;" << std::endl
			<< "  std::vector<double> out;" << std::endl
			<< std::endl
			<< "  std::vector<double> x01, y01;" << std::endl
			<< "  std::vector<double> x02, y02;" << std::endl
			<< "  std::vector<double> x03, y03;" << std::endl
			<< "  std::vector<double> x04, y04;" << std::endl
			<< "  std::vector<double> x05, y05;" << std::endl
			<< "  std::vector<double> x06, y06;" << std::endl
			<< "  std::vector<double> x07, y07;" << std::endl
			<< "  std::vector<double> x08, y08;" << std::endl
			<< "  std::vector<double> x09, y09;" << std::endl
			<< "  std::vector<double> x10, y10;" << std::endl
			<< std::endl
			<< "  // Create random numbers and check the output" << std::endl
			<< "  for(std::uint32_t i=0; i<MAXPOINTS; i++){" << std::endl
			<< "    x=x_low + (x_high - x_low)*lf();" << std::endl
			<< "    y=x_low + (y_high - y_low)*lf();" << std::endl
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
			<< "    if(output < 0.1) {" << std::endl
			<< "      x01.push_back(x);" << std::endl
			<< "      y01.push_back(y);" << std::endl
			<< "    } else if(output < 0.2) {" << std::endl
			<< "      x02.push_back(x);" << std::endl
			<< "      y02.push_back(y);" << std::endl
			<< "    } else if(output < 0.3) {" << std::endl
			<< "      x03.push_back(x);" << std::endl
			<< "      y03.push_back(y);" << std::endl
			<< "    } else if(output < 0.4) {" << std::endl
			<< "      x04.push_back(x);" << std::endl
			<< "      y04.push_back(y);" << std::endl
			<< "    } else if(output < 0.5) {" << std::endl
			<< "      x05.push_back(x);" << std::endl
			<< "      y05.push_back(y);" << std::endl
			<< "    } else if(output < 0.6) {" << std::endl
			<< "      x06.push_back(x);" << std::endl
			<< "      y06.push_back(y);" << std::endl
			<< "    } else if(output < 0.7) {" << std::endl
			<< "      x07.push_back(x);" << std::endl
			<< "      y07.push_back(y);" << std::endl
			<< "    } else if(output < 0.8) {" << std::endl
			<< "      x08.push_back(x);" << std::endl
			<< "      y08.push_back(y);" << std::endl
			<< "    } else if(output < 0.9) {" << std::endl
			<< "      x09.push_back(x);" << std::endl
			<< "      y09.push_back(y);" << std::endl
			<< "    } else {" << std::endl
			<< "      x10.push_back(x);" << std::endl
			<< "      y10.push_back(y);" << std::endl
			<< "    }" << std::endl
			<< "  }" << std::endl
			<< std::endl
			<< "  // Write test results" << std::endl
			<< "  std::ostringstream results;" << std::endl
			<< "  results" << std::endl
			<< "  << \"{\" << std::endl" << std::endl
			<< "  << \"  gROOT->Reset();\" << std::endl" << std::endl
			<< "  << \"  gStyle->SetCanvasColor(0);\" << std::endl" << std::endl
			<< "  << \"  gStyle->SetStatBorderSize(1);\" << std::endl" << std::endl
			<< "  << \"  gStyle->SetOptStat(0);\" << std::endl" << std::endl
			<< "  << std::endl" << std::endl
			<< "  << \"  TCanvas *cc = new TCanvas(\\\"cc\\\", \\\"cc\\\",0,0,1024,1024);\" << std::endl" << std::endl
			<< "  << std::endl" << std::endl
			<<
			"  << \"  TPaveLabel* canvasTitle = new TPaveLabel(0.1,0.95,0.9,0.99, \\\"Output of Feedforward Neural Network with geometry \" << geometry << \"\\\");\" << std::endl" <<
			std::endl
			<< "  << \"  canvasTitle->Draw();\" << std::endl" << std::endl
			<< "  << std::endl" << std::endl
			<< "  << \"  TPad* graphPad = new TPad(\\\"Graphs\\\", \\\"Graphs\\\", 0.01, 0.01, 0.99, 0.94);\" << std::endl" <<
			std::endl
			<< "  << \"  graphPad->Draw();\" << std::endl" << std::endl
			<< "  << \"  graphPad->Divide(1,1);\" << std::endl" << std::endl
			<< "  << std::endl" << std::endl
			<< "  << \"  double x01[\" << x01.size() << \"];\" << std::endl" << std::endl
			<< "  << \"  double y01[\" << y01.size() << \"];\" << std::endl" << std::endl
			<< "  << \"  double x02[\" << x02.size() << \"];\" << std::endl" << std::endl
			<< "  << \"  double y02[\" << y02.size() << \"];\" << std::endl" << std::endl
			<< "  << \"  double x03[\" << x03.size() << \"];\" << std::endl" << std::endl
			<< "  << \"  double y03[\" << y03.size() << \"];\" << std::endl" << std::endl
			<< "  << \"  double x04[\" << x04.size() << \"];\" << std::endl" << std::endl
			<< "  << \"  double y04[\" << y04.size() << \"];\" << std::endl" << std::endl
			<< "  << \"  double x05[\" << x05.size() << \"];\" << std::endl" << std::endl
			<< "  << \"  double y05[\" << y05.size() << \"];\" << std::endl" << std::endl
			<< "  << \"  double x06[\" << x06.size() << \"];\" << std::endl" << std::endl
			<< "  << \"  double y06[\" << y06.size() << \"];\" << std::endl" << std::endl
			<< "  << \"  double x07[\" << x07.size() << \"];\" << std::endl" << std::endl
			<< "  << \"  double y07[\" << y07.size() << \"];\" << std::endl" << std::endl
			<< "  << \"  double x08[\" << x08.size() << \"];\" << std::endl" << std::endl
			<< "  << \"  double y08[\" << y08.size() << \"];\" << std::endl" << std::endl
			<< "  << \"  double x09[\" << x09.size() << \"];\" << std::endl" << std::endl
			<< "  << \"  double y09[\" << y09.size() << \"];\" << std::endl" << std::endl
			<< "  << \"  double x10[\" << x10.size() << \"];\" << std::endl" << std::endl
			<< "  << \"  double y10[\" << y10.size() << \"];\" << std::endl" << std::endl
			<< "  << std::endl;" << std::endl
			<< std::endl
			<< "  for(std::size_t i=0; i<x01.size(); i++){" << std::endl
			<< "    results" << std::endl
			<< "    << \"  x01[\" << i << \"] = \" << x01[i] << \";\" << std::endl" << std::endl
			<< "    << \"  y01[\" << i << \"] = \" << y01[i] << \";\" << std::endl;" << std::endl
			<< "  }" << std::endl
			<< "  for(std::size_t i=0; i<x02.size(); i++){" << std::endl
			<< "    results" << std::endl
			<< "    << \"  x02[\" << i << \"] = \" << x02[i] << \";\" << std::endl" << std::endl
			<< "    << \"  y02[\" << i << \"] = \" << y02[i] << \";\" << std::endl;" << std::endl
			<< "  }" << std::endl
			<< "  for(std::size_t i=0; i<x03.size(); i++){" << std::endl
			<< "    results" << std::endl
			<< "    << \"  x03[\" << i << \"] = \" << x03[i] << \";\" << std::endl" << std::endl
			<< "    << \"  y03[\" << i << \"] = \" << y03[i] << \";\" << std::endl;" << std::endl
			<< "  }" << std::endl
			<< "  for(std::size_t i=0; i<x04.size(); i++){" << std::endl
			<< "    results" << std::endl
			<< "    << \"  x04[\" << i << \"] = \" << x04[i] << \";\" << std::endl" << std::endl
			<< "    << \"  y04[\" << i << \"] = \" << y04[i] << \";\" << std::endl;" << std::endl
			<< "  }" << std::endl
			<< "  for(std::size_t i=0; i<x05.size(); i++){" << std::endl
			<< "    results" << std::endl
			<< "    << \"  x05[\" << i << \"] = \" << x05[i] << \";\" << std::endl" << std::endl
			<< "    << \"  y05[\" << i << \"] = \" << y05[i] << \";\" << std::endl;" << std::endl
			<< "  }" << std::endl
			<< "  for(std::size_t i=0; i<x06.size(); i++){" << std::endl
			<< "    results" << std::endl
			<< "    << \"  x06[\" << i << \"] = \" << x06[i] << \";\" << std::endl" << std::endl
			<< "    << \"  y06[\" << i << \"] = \" << y06[i] << \";\" << std::endl;" << std::endl
			<< "  }" << std::endl
			<< "  for(std::size_t i=0; i<x07.size(); i++){" << std::endl
			<< "    results" << std::endl
			<< "    << \"  x07[\" << i << \"] = \" << x07[i] << \";\" << std::endl" << std::endl
			<< "    << \"  y07[\" << i << \"] = \" << y07[i] << \";\" << std::endl;" << std::endl
			<< "  }" << std::endl
			<< "  for(std::size_t i=0; i<x08.size(); i++){" << std::endl
			<< "    results" << std::endl
			<< "    << \"  x08[\" << i << \"] = \" << x08[i] << \";\" << std::endl" << std::endl
			<< "    << \"  y08[\" << i << \"] = \" << y08[i] << \";\" << std::endl;" << std::endl
			<< "  }" << std::endl
			<< "  for(std::size_t i=0; i<x09.size(); i++){" << std::endl
			<< "    results" << std::endl
			<< "    << \"  x09[\" << i << \"] = \" << x09[i] << \";\" << std::endl" << std::endl
			<< "    << \"  y09[\" << i << \"] = \" << y09[i] << \";\" << std::endl;" << std::endl
			<< "  }" << std::endl
			<< "  for(std::size_t i=0; i<x10.size(); i++){" << std::endl
			<< "    results" << std::endl
			<< "    << \"  x10[\" << i << \"] = \" << x10[i] << \";\" << std::endl" << std::endl
			<< "    << \"  y10[\" << i << \"] = \" << y10[i] << \";\" << std::endl;" << std::endl
			<< "  }" << std::endl
			<< std::endl
			<< "   results" << std::endl
			<< "   << std::endl" << std::endl
			<< "   << \"  TGraph *inside01 = new TGraph(\" << x01.size() << \", x01, y01);\" << std::endl" << std::endl
			<< "   << \"  TGraph *inside02 = new TGraph(\" << x02.size() << \", x02, y02);\" << std::endl" << std::endl
			<< "   << \"  TGraph *inside03 = new TGraph(\" << x03.size() << \", x03, y03);\" << std::endl" << std::endl
			<< "   << \"  TGraph *inside04 = new TGraph(\" << x04.size() << \", x04, y04);\" << std::endl" << std::endl
			<< "   << \"  TGraph *inside05 = new TGraph(\" << x05.size() << \", x05, y05);\" << std::endl" << std::endl
			<< "   << \"  TGraph *inside06 = new TGraph(\" << x06.size() << \", x06, y06);\" << std::endl" << std::endl
			<< "   << \"  TGraph *inside07 = new TGraph(\" << x07.size() << \", x07, y07);\" << std::endl" << std::endl
			<< "   << \"  TGraph *inside08 = new TGraph(\" << x08.size() << \", x08, y08);\" << std::endl" << std::endl
			<< "   << \"  TGraph *inside09 = new TGraph(\" << x09.size() << \", x09, y09);\" << std::endl" << std::endl
			<< "   << \"  TGraph *inside10 = new TGraph(\" << x10.size() << \", x10, y10);\" << std::endl" << std::endl
			<< "   << std::endl" << std::endl
			<< "   << \"  graphPad->cd(1);\" << std::endl" << std::endl
			<< "   << std::endl" << std::endl
			<<
			"   << \"  inside01->GetHistogram()->SetTitle(\\\"Network outputs in the ranges [0:0.1], ... ,[0.9:1.0]\\\");\" << std::endl" <<
			std::endl
			<< "   << \"  inside01->SetMarkerStyle(21);\" << std::endl" << std::endl
			<< "   << \"  inside01->SetMarkerSize(0.35);\" << std::endl" << std::endl
			<< "   << \"  inside01->SetMarkerColor(17);\" << std::endl" << std::endl
			<< "   << \"  inside01->GetXaxis()->SetLimits(" << x_low << ", " << x_high << ");\" << std::endl" << std::endl
			<< "   << \"  inside01->GetYaxis()->SetRangeUser(" << y_low << ", " << y_high << ");\" << std::endl" << std::endl
			<< "   << std::endl" << std::endl
			<< "   << \"  inside02->SetMarkerStyle(21);\" << std::endl" << std::endl
			<< "   << \"  inside02->SetMarkerSize(0.35);\" << std::endl" << std::endl
			<< "   << \"  inside02->SetMarkerColor(14);\" << std::endl" << std::endl
			<< "   << std::endl" << std::endl
			<< "   << \"  inside03->SetMarkerStyle(21);\" << std::endl" << std::endl
			<< "   << \"  inside03->SetMarkerSize(0.35);\" << std::endl" << std::endl
			<< "   << \"  inside03->SetMarkerColor(17);\" << std::endl" << std::endl
			<< "   << std::endl" << std::endl
			<< "   << \"  inside04->SetMarkerStyle(21);\" << std::endl" << std::endl
			<< "   << \"  inside04->SetMarkerSize(0.35);\" << std::endl" << std::endl
			<< "   << \"  inside04->SetMarkerColor(14);\" << std::endl" << std::endl
			<< "   << std::endl" << std::endl
			<< "   << \"  inside05->SetMarkerStyle(21);\" << std::endl" << std::endl
			<< "   << \"  inside05->SetMarkerSize(0.35);\" << std::endl" << std::endl
			<< "   << \"  inside05->SetMarkerColor(17);\" << std::endl" << std::endl
			<< "   << std::endl" << std::endl
			<< "   << \"  inside06->SetMarkerStyle(21);\" << std::endl" << std::endl
			<< "   << \"  inside06->SetMarkerSize(0.35);\" << std::endl" << std::endl
			<< "   << \"  inside06->SetMarkerColor(14);\" << std::endl" << std::endl
			<< "   << std::endl" << std::endl
			<< "   << \"  inside07->SetMarkerStyle(21);\" << std::endl" << std::endl
			<< "   << \"  inside07->SetMarkerSize(0.35);\" << std::endl" << std::endl
			<< "   << \"  inside07->SetMarkerColor(17);\" << std::endl" << std::endl
			<< "   << std::endl" << std::endl
			<< "   << \"  inside08->SetMarkerStyle(21);\" << std::endl" << std::endl
			<< "   << \"  inside08->SetMarkerSize(0.35);\" << std::endl" << std::endl
			<< "   << \"  inside08->SetMarkerColor(14);\" << std::endl" << std::endl
			<< "   << std::endl" << std::endl
			<< "   << \"  inside09->SetMarkerStyle(21);\" << std::endl" << std::endl
			<< "   << \"  inside09->SetMarkerSize(0.35);\" << std::endl" << std::endl
			<< "   << \"  inside09->SetMarkerColor(17);\" << std::endl" << std::endl
			<< "   << std::endl" << std::endl
			<< "   << \"  inside10->SetMarkerStyle(21);\" << std::endl" << std::endl
			<< "   << \"  inside10->SetMarkerSize(0.35);\" << std::endl" << std::endl
			<< "   << \"  inside10->SetMarkerColor(14);\" << std::endl" << std::endl
			<< "   << std::endl" << std::endl
			<< " << \"  inside01->Draw(\\\"AP\\\");\" << std::endl" << std::endl
			<< " << \"  inside02->Draw(\\\"P\\\");\"  << std::endl" << std::endl
			<< " << \"  inside03->Draw(\\\"P\\\");\"  << std::endl" << std::endl
			<< " << \"  inside04->Draw(\\\"P\\\");\"  << std::endl" << std::endl
			<< " << \"  inside05->Draw(\\\"P\\\");\"  << std::endl" << std::endl
			<< " << \"  inside06->Draw(\\\"P\\\");\"  << std::endl" << std::endl
			<< " << \"  inside07->Draw(\\\"P\\\");\"  << std::endl" << std::endl
			<< " << \"  inside08->Draw(\\\"P\\\");\"  << std::endl" << std::endl
			<< " << \"  inside09->Draw(\\\"P\\\");\"  << std::endl" << std::endl
			<< " << \"  inside10->Draw(\\\"P\\\");\"  << std::endl" << std::endl
			<< " << \"}\" << std::endl;" << std::endl
			<< std::endl
			<< "  std::cout " << std::endl
			<< "  << \"Writing test results into file testResults.C\" << std::endl" << std::endl
			<< "  << \"Test with the command \\\"root -l testResults.C\\\"\" << std::endl;" << std::endl
			<< "  std::ofstream fstr(\"testResults.C\");" << std::endl
			<< "  fstr << results.str();" << std::endl
			<< "  fstr.close();" << std::endl
			<< "}" << std::endl;
	}
	else {
		glogger
			<< "In GNeuralNetworkIndividual::writeVisualizationFile(const std::string&) : Warning!" << std::endl
			<< "Request to create visualization program for more than two input dimensions!" << std::endl
			<< "No action taken." << std::endl
			<< GWARNING;
	}

	// Clean up
	visProgram.close();
}

/******************************************************************************/
/**
 * Creates a C++ output file for the trained network, suitable for usage in
 * other projects. If you just want to retrieve the C++ description of the network,
 * call this function with an empty string "" .
 *
 * @param headerFile The name of the header file the network should be saved in
 */
void GNeuralNetworkIndividual::writeTrainedNetwork(const std::string &headerFile) {
	if (headerFile == "" || headerFile.empty()) {
		throw gemfony_exception(
			g_error_streamer(DO_LOG,  time_and_place)
				<< "In GNeuralNetworkIndividual::writeTrainedNetwork(const std::string&) : Error" << std::endl
				<< "Received empty file name." << std::endl
		);
	}

	std::ofstream header(headerFile);
	if (not header) {
		throw gemfony_exception(
			g_error_streamer(DO_LOG,  time_and_place)
				<< "In GNeuralNetworkIndividual::writeTrainedNetwork(const std::string&) :" << std::endl
				<< "Error writing output file " << headerFile << std::endl
		);
	}

	header << "/**" << std::endl
			 << " * @file " << headerFile << std::endl
			 << " *" << std::endl
			 << " * This file represents the results of a feedforward neural network trained" << std::endl
			 << " * using the Geneva library. It has been auto-generated by the GNeuralNetworkIndividual" << std::endl
			 << " * class." << std::endl
			 << " */" << std::endl
			 << std::endl
			 << "/*" << std::endl
			 << " * This file is part of the Geneva library collection." << std::endl
			 << " * The same license applies to this generated file." << std::endl
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

	switch (tF_) {
		case transferFunction::SIGMOID:
			header << "      return 1./(1.+exp(-value));" << std::endl;
			break;
		case transferFunction::RBF:
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
			 << "      const std::size_t nLayers = " << this->m_data_cnt.size() << ";" << std::endl
			 << "      const std::size_t architecture[nLayers] = {" << std::endl;

	for (std::size_t i = 0; i < nD_->size(); i++) {
		header << "        " << nD_->at(i);
		if (i == nD_->size() - 1)
			header << std::endl;
		else
			header << "," << std::endl;
	}

	std::size_t weightOffset = 0;

	header << "      };" << std::endl
			 << "      const std::size_t weightOffset[nLayers] = {" << std::endl
			 << "        " << weightOffset << "," << std::endl;

	weightOffset += 2 * (*nD_)[0];
	header << "       " << weightOffset << "," << std::endl;

	for (std::size_t i = 1; i < nD_->size() - 1; i++) {
		weightOffset += (*nD_)[i] * ((*nD_)[i - 1] + 1);
		header << "        " << weightOffset;

		if (i == nD_->size() - 1)
			header << std::endl;
		else
			header << "," << std::endl;
	}

	header << "      };" << std::endl;

	std::size_t nWeights = 2 * (*nD_)[0];
	for (std::size_t i = 1; i < nD_->size(); i++) {
		nWeights += (*nD_)[i] * ((*nD_)[i - 1] + 1);
	}

	header
		<< "      const std::size_t nWeights = " << nWeights << ";" << std::endl
		<< "      const double weights[nWeights] = {" << std::endl;

	for (std::size_t i = 0; i < nD_->size(); i++) {
		std::shared_ptr <GDoubleObjectCollection> currentLayer = at<GDoubleObjectCollection>(i);

		for (std::size_t j = 0; j < currentLayer->size(); j++) {
			header << "        " << currentLayer->at(j)->value();

			if (i == (nD_->size() - 1) && j == (currentLayer->size() - 1)) header << std::endl;
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
			 <<
			 "            nodeResult += prevResults[prevNodeCounter]*weights[weightOffset[layerCounter] + nodeCounter*(nPrevLayerNodes+1)+prevNodeCounter];" <<
			 std::endl
			 << "          }" << std::endl
			 <<
			 "          nodeResult -= weights[weightOffset[layerCounter] + nodeCounter*(nPrevLayerNodes+1)+nPrevLayerNodes];" <<
			 std::endl
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

/******************************************************************************/
/**
 * Loads the data of another GNeuralNetworkIndividual, camouflaged as a GObject
 *
 * @param cp A copy of another GNeuralNetworkIndividual, camouflaged as a GObject
 */
void GNeuralNetworkIndividual::load_(const GObject *cp) {
	// Check that we are dealing with a GNeuralNetworkIndividual reference independent of this object and convert the pointer
	const GNeuralNetworkIndividual *p_load = Gem::Common::g_convert_and_compare<GObject, GNeuralNetworkIndividual>(cp, this);

	// Load the parent class'es data
	GParameterSet::load_(cp);

	// Load our local data.
	tF_ = p_load->tF_;

	// We do not copy the network data, as it is always initialized through
	// the constructors, even in the case of a copy constructor
}

/******************************************************************************/
/**
 * Creates a deep clone of this object
 *
 * @return A deep clone of this object, camouflaged as a GObject
 */
GObject *GNeuralNetworkIndividual::clone_() const {
	return new GNeuralNetworkIndividual(*this);
}

/******************************************************************************/
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
 * @param id The id of the target function (ignored here)
 * @return The fitness of this object
 */
double GNeuralNetworkIndividual::fitnessCalculation() {
	double result = 0;

	// Now loop over all data sets
	std::size_t currentPos = 0;
	boost::optional<std::shared_ptr < trainingSet>> o;
	while ((o = nD_->getTrainingSet(currentPos++))) {
		// Retrieve a constant reference to the training data set for faster access
		const trainingSet &tS = **o;

		// The input layer
		std::vector<double> prevResults;
		std::size_t nLayerNodes = (*nD_)[0];
		double nodeResult = 0;
		const GDoubleObjectCollection &inputLayer = *(at<GDoubleObjectCollection>(0));
		for (std::size_t nodeCounter = 0; nodeCounter < nLayerNodes; nodeCounter++) {
			nodeResult =
				tS.Input[nodeCounter] * inputLayer[2 * nodeCounter]->value() - inputLayer[2 * nodeCounter + 1]->value();
			nodeResult = transfer(nodeResult);
			prevResults.push_back(nodeResult);
		}

		// All other layers
		std::size_t nLayers = this->m_data_cnt.size();
		for (std::size_t layerCounter = 1; layerCounter < nLayers; layerCounter++) {
			std::vector<double> currentResults;
			nLayerNodes = (*nD_)[layerCounter];
			std::size_t nPrevLayerNodes = (*nD_)[layerCounter - 1];
			const GDoubleObjectCollection &currentLayer = *(at<GDoubleObjectCollection>(layerCounter));

			for (std::size_t nodeCounter = 0; nodeCounter < nLayerNodes; nodeCounter++) {
				// Loop over all nodes of the previous layer
				nodeResult = 0.;
				for (std::size_t prevNodeCounter = 0; prevNodeCounter < nPrevLayerNodes; prevNodeCounter++) {
					nodeResult += prevResults.at(prevNodeCounter) *
									  (currentLayer[nodeCounter * (nPrevLayerNodes + 1) + prevNodeCounter])->value();
				}
				nodeResult -= (currentLayer[nodeCounter * (nPrevLayerNodes + 1) + nPrevLayerNodes])->value();
				nodeResult = transfer(nodeResult);
				currentResults.push_back(nodeResult);
			}

			prevResults = currentResults;
		}

		// At this point prevResults should contain the output values of the output layer

		// Calculate the error made and add it to the result
		std::size_t prefResultsSize = prevResults.size();
		for (std::size_t nodeCounter = 0; nodeCounter < prefResultsSize; nodeCounter++) {
			result += GSQUARED(prevResults.at(nodeCounter) - tS.Output[nodeCounter]);
		}
	}

	// Let the audience know
	return result;
}


/******************************************************************************/
/**
 * The transfer function, used to switch between radial basis and
 * sigmoid networks
 */
double GNeuralNetworkIndividual::transfer(const double &value) const {
	switch (tF_) {
		case transferFunction::SIGMOID: {
			return 1. / (1. + exp(-value));
		}
			break;

		case transferFunction::RBF: {
			return exp(-GSQUARED(value));
		}
			break;

		default: {
			throw gemfony_exception(
				g_error_streamer(DO_LOG,  time_and_place)
					<< "In GNeuralNetworkIndividual::transfer(): Error!" << std::endl
					<< "Got invalid tranfer function " << tF_ << std::endl
			);
		}
			break;
	}

	// Make the compiler happy
	return 0.;
}

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/
/**
 * A constructor with the ability to switch the parallelization mode. It initializes a
 * target item as needed.
 *
 * @param configFile The name of the configuration file
 */
GNeuralNetworkIndividualFactory::GNeuralNetworkIndividualFactory(std::filesystem::path const &configFile)
	: Gem::Common::GFactoryT<GParameterSet>(configFile), adProb_(GNN_DEF_ADPROB), adaptAdProb_(GNN_DEF_ADAPTADPROB),
	minAdProb_(GNN_DEF_MINADPROB), maxAdProb_(GNN_DEF_MAXADPROB), sigma_(GNN_DEF_SIGMA),
	sigmaSigma_(GNN_DEF_SIGMASIGMA), minSigma_(GNN_DEF_MINSIGMA), maxSigma_(GNN_DEF_MAXSIGMA), minVar_(GNN_DEF_MINVAR),
	maxVar_(GNN_DEF_MAXVAR), tF_(GNN_DEF_TRANSFER) { /* nothing */ }

/******************************************************************************/
/**
 * The destructor
 */
GNeuralNetworkIndividualFactory::~GNeuralNetworkIndividualFactory() { /* nothing */ }

/******************************************************************************/
/**
 * Sets the type of the transfer function
 */
void GNeuralNetworkIndividualFactory::setTransferFunction(transferFunction tF) {
	tF_ = tF;
}

/******************************************************************************/
/**
 * Retrieves the type of the transfer function
 */
transferFunction GNeuralNetworkIndividualFactory::getTransferFunction() const {
	return tF_;
}

/******************************************************************************/
/**
 * Creates items of this type
 *
 * @return Items of the desired type
 */
std::shared_ptr <GParameterSet> GNeuralNetworkIndividualFactory::getObject_(
	Gem::Common::GParserBuilder &gpb, const std::size_t &id
) {
	// Will hold the result
	std::shared_ptr <GNeuralNetworkIndividual> target(new GNeuralNetworkIndividual());

	// Make the object's local configuration options known
	target->addConfigurationOptions(gpb);

	return target;
}

/******************************************************************************/
/**
 * Allows to describe local configuration options for gradient descents
 */
void GNeuralNetworkIndividualFactory::describeLocalOptions_(Gem::Common::GParserBuilder &gpb) {
	// Describe our own options
	using namespace Gem::Courtier;

	std::string comment;

	comment = "";
	comment += "The probability for random adaptions of values in evolutionary algorithms;";
	gpb.registerFileParameter<double>(
		"adProb", adProb_, GNN_DEF_ADPROB, Gem::Common::VAR_IS_ESSENTIAL, comment
	);

	comment = "";
	comment += "Determines the rate of adaption of adProb. Set to 0, if you do not need this feature;";
	gpb.registerFileParameter<double>(
		"adaptAdProb", adaptAdProb_, GNN_DEF_ADAPTADPROB, Gem::Common::VAR_IS_ESSENTIAL, comment
	);

	comment = "";
	comment += "The lower allowed boundary for adProb-variation;";
	gpb.registerFileParameter<double>(
		"minAdProb", minAdProb_, GNN_DEF_MINADPROB, Gem::Common::VAR_IS_ESSENTIAL, comment
	);

	comment = "";
	comment += "The upper allowed boundary for adProb-variation;";
	gpb.registerFileParameter<double>(
		"maxAdProb", maxAdProb_, GNN_DEF_MAXADPROB, Gem::Common::VAR_IS_ESSENTIAL, comment
	);

	comment = "";
	comment += "The sigma for gauss-adaption in ES;";
	gpb.registerFileParameter<double>(
		"sigma", sigma_, GNN_DEF_SIGMA, Gem::Common::VAR_IS_ESSENTIAL, comment
	);

	comment = "";
	comment += "Influences the self-adaption of gauss-mutation in ES;";
	gpb.registerFileParameter<double>(
		"sigmaSigma", sigmaSigma_, GNN_DEF_SIGMASIGMA, Gem::Common::VAR_IS_ESSENTIAL, comment
	);

	comment = "";
	comment += "The minimum amount value of sigma;";
	gpb.registerFileParameter<double>(
		"minSigma", minSigma_, GNN_DEF_MINSIGMA, Gem::Common::VAR_IS_ESSENTIAL, comment
	);

	comment = "";
	comment += "The maximum amount value of sigma;";
	gpb.registerFileParameter<double>(
		"maxSigma", maxSigma_, GNN_DEF_MAXSIGMA, Gem::Common::VAR_IS_ESSENTIAL, comment
	);

	comment = "";
	comment += "The lower boundary of the initialization range for parameters;";
	gpb.registerFileParameter<double>(
		"minVar", minVar_, GNN_DEF_MINVAR, Gem::Common::VAR_IS_ESSENTIAL, comment
	);

	comment = "";
	comment += "The upper boundary of the initialization range for parameters;";
	gpb.registerFileParameter<double>(
		"maxVar", maxVar_, GNN_DEF_MAXVAR, Gem::Common::VAR_IS_ESSENTIAL, comment
	);

	comment = "";
	comment += "The transferFunction: SIGMOID (0) or RBF/Radial Basis (1);";
	gpb.registerFileParameter<transferFunction>(
		"transferFunction", tF_, GNN_DEF_TRANSFER, Gem::Common::VAR_IS_ESSENTIAL, comment
	);


	// Allow our parent class to describe its options
	Gem::Common::GFactoryT<GParameterSet>::describeLocalOptions_(gpb);
}

/******************************************************************************/
/**
 * Allows to act on the configuration options received from the configuration file. Here
 * we can add the options described in describeLocalOptions to the object. In practice,
 * we add the parameter objects here
 *
 * @param p A smart-pointer to be acted on during post-processing
 */
void GNeuralNetworkIndividualFactory::postProcess_(std::shared_ptr < GParameterSet > &p_raw) {
	// Convert the base pointer to the target type
	std::shared_ptr <GNeuralNetworkIndividual> p
		= Gem::Common::convertSmartPointer<GParameterSet, GNeuralNetworkIndividual>(p_raw);

	// Call the initialization function with our parsed data
	p->init(
		minVar_, maxVar_, sigma_, sigmaSigma_, minSigma_, maxSigma_, adProb_, adaptAdProb_, minAdProb_, maxAdProb_
	);

	// Set the transfer function
	p->setTransferFunction(tF_);
}

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/

} /* namespace Geneva */
} /* namespace Gem */


namespace Gem {
namespace Common {

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/
/**
 * A factory function for networkData objects, used by GSingletonT. It
 * queries a global options store for the name of the network data file
 *
 * @return A std::shared_ptr to a newly created T object
 */
template<>
std::shared_ptr <Gem::Geneva::networkData> TFactory_GSingletonT() {
	if (GNeuralNetworkOptions->exists("trainingDataFile")) {
		return std::shared_ptr<Gem::Geneva::networkData>(
			new Gem::Geneva::networkData(GNeuralNetworkOptions->get("trainingDataFile")));
	} else {
		return std::shared_ptr<Gem::Geneva::networkData>(new Gem::Geneva::networkData(Gem::Geneva::GNN_DEF_DATAFILE));
	}
}

} /* namespace Common */
} /* namespace Gem */

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/

