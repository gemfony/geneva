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
 * See the NOTICE file in the top-level directory of the Geneva library
 * collection for a list of contributors and copyright information.
 *
 ********************************************************************************/

#include <geneva-individuals/GNeuralNetworkIndividual.hpp>

BOOST_CLASS_EXPORT_IMPLEMENT(Gem::Geneva::trainingSet)              // NOLINT
BOOST_CLASS_EXPORT_IMPLEMENT(Gem::Geneva::networkData)              // NOLINT
BOOST_CLASS_EXPORT_IMPLEMENT(Gem::Geneva::GNeuralNetworkIndividual) // NOLINT
namespace Gem::Geneva {

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/
/**
 * The (private) default constructor. It is only needed for (de-)serialization
 * purposes.
 */
trainingSet::trainingSet()
  : nInputNodes(0)
  , nOutputNodes(0)
  , Input(nullptr)
  , Output(nullptr) { /* nothing */
}

/******************************************************************************/
/**
 * Initialization with the number of nodes
 */
trainingSet::trainingSet(const std::size_t &nInput, const std::size_t &nOutput)
  : nInputNodes(nInput)
  , nOutputNodes(nOutput)
  , Input(new double[nInput])
  , Output(new double[nOutput]) {
    // Make sure the arrays are properly initialized
    for(std::size_t i = 0; i < nInput; i++) {
        Input[i] = 0.;
    }
    for(std::size_t o = 0; o < nOutput; o++) {
        Output[o] = 0.;
    }
}

/******************************************************************************/
/**
 * A copy constructor
 */
trainingSet::trainingSet(const trainingSet &cp)
  : nInputNodes(0)
  , nOutputNodes(0)
  , Input(nullptr)
  , Output(nullptr) {
    Gem::Common::copyArrays(cp.Input, Input, cp.nInputNodes, nInputNodes);
    Gem::Common::copyArrays(cp.Output, Output, cp.nOutputNodes, nOutputNodes);
}

/******************************************************************************/
/**
 * The destructor.
 */
trainingSet::~trainingSet() {
    if(Input) {
        Gem::Common::g_array_delete(Input);
    }
    if(Output) {
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
    const trainingSet &cp,
    const Gem::Common::expectation &e,
    const double & /*limit*/
) const {
    using namespace Gem::Common;

    Gem::Common::GToken token("trainingSet", e);

    // Compare our local data
    Gem::Common::compare_t(IDENTITY(nInputNodes, cp.nInputNodes), token);
    Gem::Common::compare_t(IDENTITY(nOutputNodes, cp.nOutputNodes), token);

    for(std::size_t i = 0; i < nInputNodes; i++) {
        Gem::Common::compare_t(IDENTITY(Input[i], cp.Input[i]), token);
    }

    for(std::size_t i = 0; i < nOutputNodes; i++) {
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
  : arraySize_(0)
  , data_(nullptr) { /* nothing */
}

/******************************************************************************/
/**
 * Initialization with the amount of entries
 *
 * @param arraySize The desired size of the array
 */
networkData::networkData(const std::size_t &arraySize)
  : Gem::Common::GPodContainerT<std::size_t>()
  , arraySize_(arraySize)
  , data_(new std::shared_ptr<trainingSet>[arraySize_]) { /* nothing */
}

/******************************************************************************/
/**
 * Initializes the object with data from a file
 *
 * @param networkDataFile The name of a file holding the training data
 */
networkData::networkData(const std::string &networkDataFile)
  : Gem::Common::GPodContainerT<std::size_t>()
  , arraySize_(0)
  , data_(nullptr) {
    this->loadFromDisk(networkDataFile);
}

/******************************************************************************/
/**
 * Initializes with data from another networkData object
 *
 * @param cp A copy of another networkData object
 */
networkData::networkData(const networkData &cp)
  : Gem::Common::GPodContainerT<std::size_t>(cp)
  , arraySize_(0)
  , data_(nullptr) {
    // Make sure the local data is copied
    Gem::Common::copySmartPointerArrays(cp.data_, data_, cp.arraySize_, arraySize_);
}

/******************************************************************************/
/**
 * A standard destructor.
 */
networkData::~networkData() {
    // Make sure the data vector is empty
    if(data_) {
        for(std::size_t i = 0; i < arraySize_; i++) {
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
    Gem::Common::GPodContainerT<std::size_t>::operator=(cp);
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
    const networkData &cp,
    const Gem::Common::expectation &e,
    const double & /*limit*/
) const {
    Gem::Common::GToken token("networkData", e);

    // Compare our local data
    Gem::Common::compare_t(IDENTITY(arraySize_, cp.arraySize_), token);
    Gem::Common::compare_t(IDENTITY(this->data_cnt_, cp.data_cnt_), token);

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

    if(not trDat) {
        throw geneva_exception(
            g_error_streamer(DO_LOG, time_and_place)
            << "In networkData::saveToDisk(const std::string&) : Error!" << '\n'
            << "Data file " << networkDataFile << " could not be opened for writing." << '\n'
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
    networkData *raw = nullptr;

    std::ifstream trDat(networkDataFile.c_str());

    if(not trDat) {
        std::ostringstream error; // NOLINT(cppcoreguidelines-init-variables)
        error << "In networkData::loadFromDisk(const std::string&):" << '\n'
              << "Data file " << networkDataFile << " could not be opened for reading."
              << '\n';

        if(not std::filesystem::exists(networkDataFile.c_str())) {
            error << "File does not exist." << '\n';
        }

        throw geneva_exception(
            g_error_streamer(DO_LOG, time_and_place) << error.str() << '\n'
        );
    }

    // Load the data into raw, using the Boost.Serialization library
    {
        boost::archive::xml_iarchive ia(trDat);
        ia >> boost::serialization::make_nvp("networkData", raw);
    } // Explicit scope at this point is essential so that ia's destructor is called

    std::unique_ptr<networkData> nD(raw);

    // Copy the data over, using our own operator=()
    *this = *nD;
}

/******************************************************************************/
/**
 * Adds a new training set to the collection. Note that the training set isn't
 * cloned, simply a copy of the smart pointer is stored in the internal array.
 *
 * @param tS A std::shared_ptr<trainingSet> object, pointing to a training set
 * @param pos The position, in which the data set should be stored.
 */
void networkData::addTrainingSet(std::shared_ptr<trainingSet> tS, const std::size_t &pos) {
    if(pos >= arraySize_) {
        throw geneva_exception(
            g_error_streamer(DO_LOG, time_and_place)
            << "In networkData::addTrainingSet(): Error!" << '\n'
            << "pos = " << pos << " exceeds end of array (size = " << arraySize_ << ")" << '\n'
        );
    }
    data_[pos] = tS;
}

/******************************************************************************/
/**
 * Retrieves a training set at a given position. If the position exceeds the size of the array,
 * a std::nullopt is returned which evaluates to "false".
 *
 * @param pos The position from which an item should be retreived
 * @return The training set at the requested position (or std::nullopt)
 */
std::optional<std::shared_ptr<trainingSet>>
networkData::getTrainingSet(const std::size_t &pos) const {
    if(pos >= arraySize_) {
        return std::nullopt;
    }
    else {
        return data_[pos];
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
void networkData::toROOT(const std::string &outputFile, const double &min, const double &max) {
    // Check that we have a matching number of input nodes
    if(getNInputNodes() != 2 || getNOutputNodes() != 1) {
        glogger << "In networkData::toRoot(): Warning!" << '\n'
                << "Got inappropriate number of input and/or output nodes: " << getNInputNodes()
                << "/" << getNOutputNodes() << '\n'
                << "We need 2/1. The function will return without further action." << '\n'
                << GWARNING;
        return;
    }

    std::size_t entries1 = 0, entries2 = 0;
    std::ofstream of(outputFile);

    of << "{" << '\n'
       << "  gROOT->Reset();" << '\n'
       << "  gStyle->SetCanvasColor(0);" << '\n'
       << "  gStyle->SetStatBorderSize(1);" << '\n'
       << "  gStyle->SetOptStat(0);" << '\n'
       << '\n'
       << "  TCanvas *cc = new TCanvas(\"cc\", \"cc\",0,0,1024,1024);" << '\n'
       << '\n'
       << "  TPaveLabel* canvasTitle = new TPaveLabel(0.1,0.95,0.9,0.99, \"Original training "
          "data\");"
       << '\n'
       << "  canvasTitle->Draw();" << '\n'
       << '\n'
       << "  TPad* graphPad = new TPad(\"Graphs\", \"Graphs\", 0.01, 0.01, 0.99, 0.94);"
       << '\n'
       << "  graphPad->Draw();" << '\n'
       << "  graphPad->Divide(1,1);" << '\n'
       << '\n'
       << "  double xarr1[" << arraySize_ << "], yarr1[" << arraySize_ << "], xarr2[" << arraySize_
       << "], yarr2[" << arraySize_ << "];" << '\n'
       << '\n'
       << "  // Filling the data sets" << '\n';

    for(std::size_t i = 0; i < arraySize_; i++) {
        if(data_[i]->Output[0] < 0.5) {
            of << "  xarr1[" << entries1 << "] = " << data_[i]->Input[0] << ";" << '\n'
               << "  yarr1[" << entries1 << "] = " << data_[i]->Input[1] << ";" << '\n';
            entries1++;
        }
        else {
            of << "  xarr2[" << entries2 << "] = " << data_[i]->Input[0] << ";" << '\n'
               << "  yarr2[" << entries2 << "] = " << data_[i]->Input[1] << ";" << '\n';
            entries2++;
        }
    }

    of << '\n'
       << "  // Setting remaining entries to 0" << '\n'
       << "  for(std::size_t i=" << entries1 << "; i<" << arraySize_ << "; i++) {" << '\n'
       << "    xarr1[i] = 0.;" << '\n'
       << "    yarr1[i] = 0.;" << '\n'
       << "  }" << '\n'
       << "  for(std::size_t i=" << entries2 << "; i<" << arraySize_ << "; i++) {" << '\n'
       << "    xarr2[i] = 0.;" << '\n'
       << "    yarr2[i] = 0.;" << '\n'
       << "  }" << '\n'
       << '\n'
       << "  // Creation of suitable TGraph objects" << '\n'
       << "  TGraph *gr1 = new TGraph(" << entries1 << ", xarr1, yarr1);" << '\n'
       << "  TGraph *gr2 = new TGraph(" << entries2 << ", xarr2, yarr2);" << '\n'
       << '\n'
       << "  gr1->SetMarkerColor(17);" << '\n'
       << "  gr2->SetMarkerColor(14);" << '\n'
       << '\n'
       << "  gr1->SetMarkerStyle(21);" << '\n'
       << "  gr2->SetMarkerStyle(21);" << '\n'
       << '\n'
       << "  gr1->SetMarkerSize(0.35);" << '\n'
       << "  gr2->SetMarkerSize(0.35);" << '\n'
       << '\n'
       << "  gr2->GetXaxis()->SetLimits(" << min << ", " << max << ");" << '\n'
       << "  gr2->GetYaxis()->SetRangeUser(" << min << ", " << max << ");" << '\n'
       << '\n'
       << "  // Do the drawing" << '\n'
       << "  graphPad->cd(1);" << '\n'
       << "  gr2->Draw(\"AP\");" << '\n'
       << "  gr1->Draw(\"P,same\");" << '\n'
       << "}" << '\n';

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
    std::ostringstream result; // NOLINT(cppcoreguidelines-init-variables)

    for(std::size_t i = 0; i < this->size() - 1; i++) {
        result << this->at(i) << "-";
    }
    result << this->at(this->size() - 1);
    return result.str();
}

/******************************************************************************/
/**
 * Creates a deep clone of this object
 */
std::shared_ptr<networkData> networkData::clone() const {
    // Lock access to this function
    std::unique_lock<std::mutex> lock(m_);
    std::shared_ptr<networkData> result(new networkData(*this));
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
    Gem::Common::ENUMBASETYPE tmp = 0;
    i >> tmp;

#ifdef DEBUG
    tdt = Gem::Common::narrow_cast<Gem::Geneva::trainingDataType>(tmp);
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
    Gem::Common::ENUMBASETYPE tmp = 0;
    i >> tmp;

#ifdef DEBUG
    tF = Gem::Common::narrow_cast<Gem::Geneva::transferFunction>(tmp);
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
  : tF_(GNN_DEF_TRANSFER)
  , nD_(GNNTrainingDataStore) { /* nothing */
}

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
    const double &min,
    const double &max,
    const double &sigma,
    const double &sigmaSigma,
    const double &minSigma,
    const double &maxSigma,
    const double &adProb,
    const double &adaptAdProb,
    const double &minAdProb,
    const double &maxAdProb
)
  : tF_(GNN_DEF_TRANSFER)
  , nD_(GNNTrainingDataStore) {
    this->init(
        min,
        max,
        sigma,
        sigmaSigma,
        minSigma,
        maxSigma,
        adProb,
        adaptAdProb,
        minAdProb,
        maxAdProb
    );
}

/******************************************************************************/
/**
 * A standard copy constructor
 *
 * @param cp A copy of another GNeuralNetworkIndividual object
 */
GNeuralNetworkIndividual::GNeuralNetworkIndividual(const GNeuralNetworkIndividual &cp)
  : GParameterSet(cp)
  , tF_(cp.tF_)
  , nD_(GNNTrainingDataStore) // We want a single source for the training data
{                             /* nothing */
}

/******************************************************************************/
/**
 * The standard destructor
 */
GNeuralNetworkIndividual::~GNeuralNetworkIndividual() { /* nothing */
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
void GNeuralNetworkIndividual::compare_(
    const GObject &cp,
    const Gem::Common::expectation &e,
    const double & /*limit*/
) const {
    using namespace Gem::Common;

    // Check that we are dealing with a GNeuralNetworkIndividual reference independent of this object and convert the pointer
    const GNeuralNetworkIndividual *p_load =
        Gem::Common::g_convert_and_compare<GObject, GNeuralNetworkIndividual>(cp, this);

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
    const double &min,
    const double &max,
    const double &sigma,
    const double &sigmaSigma,
    const double &minSigma,
    const double &maxSigma,
    const double &adProb,
    const double &adaptAdProb,
    const double &minAdProb,
    const double &maxAdProb
) {
    // Make sure the individual is empty
    this->clear();

#ifdef DEBUG
    if(not nD_) {
        throw geneva_exception(
            g_error_streamer(DO_LOG, time_and_place)
            << "In GNeuralNetworkIndividual::init([...]): Error!" << '\n'
            << "No network data appears to have been registered." << '\n'
        );
    }
#endif /* DEBUG */

    // Set up our local data structures

    // Check the architecture we've been given and create the layers
    std::size_t nLayers = nD_->size(); // NOLINT(cppcoreguidelines-init-variables)

    if(nLayers < 2) { // Two layers are required at the minimum (3 and 4 layers are useful)
        throw geneva_exception(
            g_error_streamer(DO_LOG, time_and_place)
            << "In GNeuralNetworkIndividual::init([...]): Error!" << '\n'
            << "Invalid number of layers supplied (" << nLayers << ")." << '\n'
            << "Did you set up the network architecture ?" << '\n'
        );
    }

    networkData::iterator layerIterator;
    std::size_t layerNumber = 0;
    std::size_t nNodes = 0;
    std::size_t nNodesPrevious = 0;

    // Access to uniformly distributed doubke random values
    std::uniform_real_distribution<double> uniform_real_distribution(min, max);

    // Set up the architecture
    for(layerIterator = nD_->begin(); layerIterator != nD_->end(); ++layerIterator) {
        if(*layerIterator) { // Add the next network layer to this class, if possible
            nNodes = *layerIterator;

            // Set up a GDoubleObjectCollection
            std::shared_ptr<GDoubleObjectCollection> gdoc(new GDoubleObjectCollection());

            // Add GDoubleObject objects
            for(std::size_t i = 0;
                i < (layerNumber == 0 ? 2 * nNodes : nNodes * (nNodesPrevious + 1));
                i++) {
                // Set up a GDoubleObject object, initializing it with random data
                std::shared_ptr<GDoubleObject> gd_ptr(
                    new GDoubleObject(uniform_real_distribution(gr_))
                );

                // Set up an adaptor
                std::shared_ptr<GDoubleGaussAdaptor> gdga(
                    new GDoubleGaussAdaptor(sigma, sigmaSigma, minSigma, maxSigma)
                );
                gdga->setAdaptionProbability(adProb);
                gdga->setAdaptAdProb(adaptAdProb);
                gdga->setAdProbRange(minAdProb, maxAdProb);

                // Register it with the GDoubleObject object
                gd_ptr->addAdaptor(gdga);

                // Register the GDoubleObject object with the collection
                gdoc->push_back(gd_ptr);
            }

            // Make the parameter collection known to this individual
            this->data_cnt_.push_back(gdoc);

            nNodesPrevious = nNodes;
            layerNumber++;
        }
        else {
            throw geneva_exception(
                g_error_streamer(DO_LOG, time_and_place)
                << "In GNeuralNetworkIndividual::init([...]): Error!" << '\n'
                << "Found invalid number of nodes in layer: " << *layerIterator << '\n'
                << "Did you set up the network architecture ?" << '\n'
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
    if(visFile == "" || visFile.empty()) {
        throw geneva_exception(
            g_error_streamer(DO_LOG, time_and_place)
            << "In GNeuralNetworkIndividual::writeVisualizationFile(const std::string&) : Error"
            << '\n'
            << "Received empty file name." << '\n'
        );
    }

    std::ofstream visProgram(visFile);
    if(not visProgram) {
        throw geneva_exception(
            g_error_streamer(DO_LOG, time_and_place)
            << "In GNeuralNetworkIndividual::writeVisualizationFile(const std::string&) :"
            << '\n'
            << "Attempt to open output file " << visFile << " for writing failed." << '\n'
        );
    }

    // The following only makes sense if the input dimension is 2
    if(nD_->getNInputNodes() == 2) {
        double x_low = 0., x_high = 1.;
        double y_low = 0., y_high = 1.;

        // Retrieve information about the initialization range
        // We only act if initialization ranges have been registered.
        // If not, than the above default values will be used.
        std::vector<std::tuple<double, double>> initRange = nD_->getInitRange();
        if(initRange.size() == 2) {
            x_low = std::get<0>(initRange.at(0));
            x_high = std::get<1>(initRange.at(0));
            y_low = std::get<0>(initRange.at(1));
            y_high = std::get<1>(initRange.at(1));
        }

        // Write the header
        visProgram
            << "/**" << '\n'
            << " * @file visualization.C" << '\n'
            << " *" << '\n'
            << " * This program allows to visualize the output of the training example."
            << '\n'
            << " * It has been auto-generated by the GNeuralNetworkIndividual class of" << '\n'
            << " * the Geneva library" << '\n'
            << " *" << '\n'
            << " * Can be compiled with a command similar to" << '\n'
            << " * g++ -o visualization -I/opt/boost155/include/ visualization.C" << '\n'
            << " * e.g. on Ubuntu 14.04 (assuming that Boost is installed under /opt/boost155"
            << '\n'
            << " * in your system). The code should work with virtually any other" << '\n'
            << " * Linux distribution that supports Boost." << '\n'
            << " */" << '\n'
            << '\n'
            << "/*" << '\n'
            << " * This file is part of the Geneva library collection." << '\n'
            << " * The same license applies to this generated file." << '\n'
            << " */" << '\n'
            << '\n'
            << '\n'
            << "#include <iostream>" << '\n'
            << "#include <sstream>" << '\n'
            << "#include <fstream>" << '\n'
            << "#include <vector>" << '\n'
            << "#include <random>" << '\n'
            << '\n'
            << "#include <cstdint>" << '\n'
            << '\n'
            << "#include \"trainedNetwork.hpp\"" << '\n'
            << '\n'
            << "const std::uint32_t MAXPOINTS=20000;" << '\n'
            << '\n'
            << "using namespace Gem::NeuralNetwork;" << '\n'
            << '\n'
            << "int main(int argc, char**argv){" << '\n'
            << "  std::string geometry = \"" << nD_->getNetworkGeometryString() << "\";"
            << '\n'
            << "  double x_low = " << x_low << ", x_high = " << x_high << ";" << '\n'
            << "  double y_low = " << y_low << ", y_high = " << y_high << ";" << '\n'
            << '\n'
            << "  boost::lagged_fibonacci607 lf(123);" << '\n'
            << '\n'
            << "  double x=0., y=0., result=0;" << '\n'
            << "  std::vector<double> in;" << '\n'
            << "  std::vector<double> out;" << '\n'
            << '\n'
            << "  std::vector<double> x01, y01;" << '\n'
            << "  std::vector<double> x02, y02;" << '\n'
            << "  std::vector<double> x03, y03;" << '\n'
            << "  std::vector<double> x04, y04;" << '\n'
            << "  std::vector<double> x05, y05;" << '\n'
            << "  std::vector<double> x06, y06;" << '\n'
            << "  std::vector<double> x07, y07;" << '\n'
            << "  std::vector<double> x08, y08;" << '\n'
            << "  std::vector<double> x09, y09;" << '\n'
            << "  std::vector<double> x10, y10;" << '\n'
            << '\n'
            << "  // Create random numbers and check the output" << '\n'
            << "  for(std::uint32_t i=0; i<MAXPOINTS; i++){" << '\n'
            << "    x=x_low + (x_high - x_low)*lf();" << '\n'
            << "    y=x_low + (y_high - y_low)*lf();" << '\n'
            << '\n'
            << "    in.clear();" << '\n'
            << "    out.clear();" << '\n'
            << '\n'
            << "    in.push_back(x);" << '\n'
            << "    in.push_back(y);" << '\n'
            << '\n'
            << "    if(!network(in,out) || out.size()==0){" << '\n'
            << "      std::cout << \"Error in calculation of network output\" << std::endl;"
            << '\n'
            << "      exit(1);" << '\n'
            << "    }" << '\n'
            << '\n'
            << "    double output = out[0];" << '\n'
            << '\n'
            << "    if(output < 0.1) {" << '\n'
            << "      x01.push_back(x);" << '\n'
            << "      y01.push_back(y);" << '\n'
            << "    } else if(output < 0.2) {" << '\n'
            << "      x02.push_back(x);" << '\n'
            << "      y02.push_back(y);" << '\n'
            << "    } else if(output < 0.3) {" << '\n'
            << "      x03.push_back(x);" << '\n'
            << "      y03.push_back(y);" << '\n'
            << "    } else if(output < 0.4) {" << '\n'
            << "      x04.push_back(x);" << '\n'
            << "      y04.push_back(y);" << '\n'
            << "    } else if(output < 0.5) {" << '\n'
            << "      x05.push_back(x);" << '\n'
            << "      y05.push_back(y);" << '\n'
            << "    } else if(output < 0.6) {" << '\n'
            << "      x06.push_back(x);" << '\n'
            << "      y06.push_back(y);" << '\n'
            << "    } else if(output < 0.7) {" << '\n'
            << "      x07.push_back(x);" << '\n'
            << "      y07.push_back(y);" << '\n'
            << "    } else if(output < 0.8) {" << '\n'
            << "      x08.push_back(x);" << '\n'
            << "      y08.push_back(y);" << '\n'
            << "    } else if(output < 0.9) {" << '\n'
            << "      x09.push_back(x);" << '\n'
            << "      y09.push_back(y);" << '\n'
            << "    } else {" << '\n'
            << "      x10.push_back(x);" << '\n'
            << "      y10.push_back(y);" << '\n'
            << "    }" << '\n'
            << "  }" << '\n'
            << '\n'
            << "  // Write test results" << '\n'
            << "  std::ostringstream results;" << '\n'
            << "  results" << '\n'
            << "  << \"{\" << std::endl" << '\n'
            << "  << \"  gROOT->Reset();\" << std::endl" << '\n'
            << "  << \"  gStyle->SetCanvasColor(0);\" << std::endl" << '\n'
            << "  << \"  gStyle->SetStatBorderSize(1);\" << std::endl" << '\n'
            << "  << \"  gStyle->SetOptStat(0);\" << std::endl" << '\n'
            << "  << std::endl" << '\n'
            << "  << \"  TCanvas *cc = new TCanvas(\\\"cc\\\", \\\"cc\\\",0,0,1024,1024);\" << "
               "std::endl"
            << '\n'
            << "  << std::endl" << '\n'
            << "  << \"  TPaveLabel* canvasTitle = new TPaveLabel(0.1,0.95,0.9,0.99, \\\"Output of "
               "Feedforward Neural Network with geometry \" << geometry << \"\\\");\" << std::endl"
            << '\n'
            << "  << \"  canvasTitle->Draw();\" << std::endl" << '\n'
            << "  << std::endl" << '\n'
            << "  << \"  TPad* graphPad = new TPad(\\\"Graphs\\\", \\\"Graphs\\\", 0.01, 0.01, "
               "0.99, 0.94);\" << std::endl"
            << '\n'
            << "  << \"  graphPad->Draw();\" << std::endl" << '\n'
            << "  << \"  graphPad->Divide(1,1);\" << std::endl" << '\n'
            << "  << std::endl" << '\n'
            << "  << \"  double x01[\" << x01.size() << \"];\" << std::endl" << '\n'
            << "  << \"  double y01[\" << y01.size() << \"];\" << std::endl" << '\n'
            << "  << \"  double x02[\" << x02.size() << \"];\" << std::endl" << '\n'
            << "  << \"  double y02[\" << y02.size() << \"];\" << std::endl" << '\n'
            << "  << \"  double x03[\" << x03.size() << \"];\" << std::endl" << '\n'
            << "  << \"  double y03[\" << y03.size() << \"];\" << std::endl" << '\n'
            << "  << \"  double x04[\" << x04.size() << \"];\" << std::endl" << '\n'
            << "  << \"  double y04[\" << y04.size() << \"];\" << std::endl" << '\n'
            << "  << \"  double x05[\" << x05.size() << \"];\" << std::endl" << '\n'
            << "  << \"  double y05[\" << y05.size() << \"];\" << std::endl" << '\n'
            << "  << \"  double x06[\" << x06.size() << \"];\" << std::endl" << '\n'
            << "  << \"  double y06[\" << y06.size() << \"];\" << std::endl" << '\n'
            << "  << \"  double x07[\" << x07.size() << \"];\" << std::endl" << '\n'
            << "  << \"  double y07[\" << y07.size() << \"];\" << std::endl" << '\n'
            << "  << \"  double x08[\" << x08.size() << \"];\" << std::endl" << '\n'
            << "  << \"  double y08[\" << y08.size() << \"];\" << std::endl" << '\n'
            << "  << \"  double x09[\" << x09.size() << \"];\" << std::endl" << '\n'
            << "  << \"  double y09[\" << y09.size() << \"];\" << std::endl" << '\n'
            << "  << \"  double x10[\" << x10.size() << \"];\" << std::endl" << '\n'
            << "  << \"  double y10[\" << y10.size() << \"];\" << std::endl" << '\n'
            << "  << std::endl;" << '\n'
            << '\n'
            << "  for(std::size_t i=0; i<x01.size(); i++){" << '\n'
            << "    results" << '\n'
            << "    << \"  x01[\" << i << \"] = \" << x01[i] << \";\" << std::endl" << '\n'
            << "    << \"  y01[\" << i << \"] = \" << y01[i] << \";\" << std::endl;" << '\n'
            << "  }" << '\n'
            << "  for(std::size_t i=0; i<x02.size(); i++){" << '\n'
            << "    results" << '\n'
            << "    << \"  x02[\" << i << \"] = \" << x02[i] << \";\" << std::endl" << '\n'
            << "    << \"  y02[\" << i << \"] = \" << y02[i] << \";\" << std::endl;" << '\n'
            << "  }" << '\n'
            << "  for(std::size_t i=0; i<x03.size(); i++){" << '\n'
            << "    results" << '\n'
            << "    << \"  x03[\" << i << \"] = \" << x03[i] << \";\" << std::endl" << '\n'
            << "    << \"  y03[\" << i << \"] = \" << y03[i] << \";\" << std::endl;" << '\n'
            << "  }" << '\n'
            << "  for(std::size_t i=0; i<x04.size(); i++){" << '\n'
            << "    results" << '\n'
            << "    << \"  x04[\" << i << \"] = \" << x04[i] << \";\" << std::endl" << '\n'
            << "    << \"  y04[\" << i << \"] = \" << y04[i] << \";\" << std::endl;" << '\n'
            << "  }" << '\n'
            << "  for(std::size_t i=0; i<x05.size(); i++){" << '\n'
            << "    results" << '\n'
            << "    << \"  x05[\" << i << \"] = \" << x05[i] << \";\" << std::endl" << '\n'
            << "    << \"  y05[\" << i << \"] = \" << y05[i] << \";\" << std::endl;" << '\n'
            << "  }" << '\n'
            << "  for(std::size_t i=0; i<x06.size(); i++){" << '\n'
            << "    results" << '\n'
            << "    << \"  x06[\" << i << \"] = \" << x06[i] << \";\" << std::endl" << '\n'
            << "    << \"  y06[\" << i << \"] = \" << y06[i] << \";\" << std::endl;" << '\n'
            << "  }" << '\n'
            << "  for(std::size_t i=0; i<x07.size(); i++){" << '\n'
            << "    results" << '\n'
            << "    << \"  x07[\" << i << \"] = \" << x07[i] << \";\" << std::endl" << '\n'
            << "    << \"  y07[\" << i << \"] = \" << y07[i] << \";\" << std::endl;" << '\n'
            << "  }" << '\n'
            << "  for(std::size_t i=0; i<x08.size(); i++){" << '\n'
            << "    results" << '\n'
            << "    << \"  x08[\" << i << \"] = \" << x08[i] << \";\" << std::endl" << '\n'
            << "    << \"  y08[\" << i << \"] = \" << y08[i] << \";\" << std::endl;" << '\n'
            << "  }" << '\n'
            << "  for(std::size_t i=0; i<x09.size(); i++){" << '\n'
            << "    results" << '\n'
            << "    << \"  x09[\" << i << \"] = \" << x09[i] << \";\" << std::endl" << '\n'
            << "    << \"  y09[\" << i << \"] = \" << y09[i] << \";\" << std::endl;" << '\n'
            << "  }" << '\n'
            << "  for(std::size_t i=0; i<x10.size(); i++){" << '\n'
            << "    results" << '\n'
            << "    << \"  x10[\" << i << \"] = \" << x10[i] << \";\" << std::endl" << '\n'
            << "    << \"  y10[\" << i << \"] = \" << y10[i] << \";\" << std::endl;" << '\n'
            << "  }" << '\n'
            << '\n'
            << "   results" << '\n'
            << "   << std::endl" << '\n'
            << "   << \"  TGraph *inside01 = new TGraph(\" << x01.size() << \", x01, y01);\" << "
               "std::endl"
            << '\n'
            << "   << \"  TGraph *inside02 = new TGraph(\" << x02.size() << \", x02, y02);\" << "
               "std::endl"
            << '\n'
            << "   << \"  TGraph *inside03 = new TGraph(\" << x03.size() << \", x03, y03);\" << "
               "std::endl"
            << '\n'
            << "   << \"  TGraph *inside04 = new TGraph(\" << x04.size() << \", x04, y04);\" << "
               "std::endl"
            << '\n'
            << "   << \"  TGraph *inside05 = new TGraph(\" << x05.size() << \", x05, y05);\" << "
               "std::endl"
            << '\n'
            << "   << \"  TGraph *inside06 = new TGraph(\" << x06.size() << \", x06, y06);\" << "
               "std::endl"
            << '\n'
            << "   << \"  TGraph *inside07 = new TGraph(\" << x07.size() << \", x07, y07);\" << "
               "std::endl"
            << '\n'
            << "   << \"  TGraph *inside08 = new TGraph(\" << x08.size() << \", x08, y08);\" << "
               "std::endl"
            << '\n'
            << "   << \"  TGraph *inside09 = new TGraph(\" << x09.size() << \", x09, y09);\" << "
               "std::endl"
            << '\n'
            << "   << \"  TGraph *inside10 = new TGraph(\" << x10.size() << \", x10, y10);\" << "
               "std::endl"
            << '\n'
            << "   << std::endl" << '\n'
            << "   << \"  graphPad->cd(1);\" << std::endl" << '\n'
            << "   << std::endl" << '\n'
            << "   << \"  inside01->GetHistogram()->SetTitle(\\\"Network outputs in the ranges "
               "[0:0.1], ... ,[0.9:1.0]\\\");\" << std::endl"
            << '\n'
            << "   << \"  inside01->SetMarkerStyle(21);\" << std::endl" << '\n'
            << "   << \"  inside01->SetMarkerSize(0.35);\" << std::endl" << '\n'
            << "   << \"  inside01->SetMarkerColor(17);\" << std::endl" << '\n'
            << "   << \"  inside01->GetXaxis()->SetLimits(" << x_low << ", " << x_high
            << ");\" << std::endl" << '\n'
            << "   << \"  inside01->GetYaxis()->SetRangeUser(" << y_low << ", " << y_high
            << ");\" << std::endl" << '\n'
            << "   << std::endl" << '\n'
            << "   << \"  inside02->SetMarkerStyle(21);\" << std::endl" << '\n'
            << "   << \"  inside02->SetMarkerSize(0.35);\" << std::endl" << '\n'
            << "   << \"  inside02->SetMarkerColor(14);\" << std::endl" << '\n'
            << "   << std::endl" << '\n'
            << "   << \"  inside03->SetMarkerStyle(21);\" << std::endl" << '\n'
            << "   << \"  inside03->SetMarkerSize(0.35);\" << std::endl" << '\n'
            << "   << \"  inside03->SetMarkerColor(17);\" << std::endl" << '\n'
            << "   << std::endl" << '\n'
            << "   << \"  inside04->SetMarkerStyle(21);\" << std::endl" << '\n'
            << "   << \"  inside04->SetMarkerSize(0.35);\" << std::endl" << '\n'
            << "   << \"  inside04->SetMarkerColor(14);\" << std::endl" << '\n'
            << "   << std::endl" << '\n'
            << "   << \"  inside05->SetMarkerStyle(21);\" << std::endl" << '\n'
            << "   << \"  inside05->SetMarkerSize(0.35);\" << std::endl" << '\n'
            << "   << \"  inside05->SetMarkerColor(17);\" << std::endl" << '\n'
            << "   << std::endl" << '\n'
            << "   << \"  inside06->SetMarkerStyle(21);\" << std::endl" << '\n'
            << "   << \"  inside06->SetMarkerSize(0.35);\" << std::endl" << '\n'
            << "   << \"  inside06->SetMarkerColor(14);\" << std::endl" << '\n'
            << "   << std::endl" << '\n'
            << "   << \"  inside07->SetMarkerStyle(21);\" << std::endl" << '\n'
            << "   << \"  inside07->SetMarkerSize(0.35);\" << std::endl" << '\n'
            << "   << \"  inside07->SetMarkerColor(17);\" << std::endl" << '\n'
            << "   << std::endl" << '\n'
            << "   << \"  inside08->SetMarkerStyle(21);\" << std::endl" << '\n'
            << "   << \"  inside08->SetMarkerSize(0.35);\" << std::endl" << '\n'
            << "   << \"  inside08->SetMarkerColor(14);\" << std::endl" << '\n'
            << "   << std::endl" << '\n'
            << "   << \"  inside09->SetMarkerStyle(21);\" << std::endl" << '\n'
            << "   << \"  inside09->SetMarkerSize(0.35);\" << std::endl" << '\n'
            << "   << \"  inside09->SetMarkerColor(17);\" << std::endl" << '\n'
            << "   << std::endl" << '\n'
            << "   << \"  inside10->SetMarkerStyle(21);\" << std::endl" << '\n'
            << "   << \"  inside10->SetMarkerSize(0.35);\" << std::endl" << '\n'
            << "   << \"  inside10->SetMarkerColor(14);\" << std::endl" << '\n'
            << "   << std::endl" << '\n'
            << " << \"  inside01->Draw(\\\"AP\\\");\" << std::endl" << '\n'
            << " << \"  inside02->Draw(\\\"P\\\");\"  << std::endl" << '\n'
            << " << \"  inside03->Draw(\\\"P\\\");\"  << std::endl" << '\n'
            << " << \"  inside04->Draw(\\\"P\\\");\"  << std::endl" << '\n'
            << " << \"  inside05->Draw(\\\"P\\\");\"  << std::endl" << '\n'
            << " << \"  inside06->Draw(\\\"P\\\");\"  << std::endl" << '\n'
            << " << \"  inside07->Draw(\\\"P\\\");\"  << std::endl" << '\n'
            << " << \"  inside08->Draw(\\\"P\\\");\"  << std::endl" << '\n'
            << " << \"  inside09->Draw(\\\"P\\\");\"  << std::endl" << '\n'
            << " << \"  inside10->Draw(\\\"P\\\");\"  << std::endl" << '\n'
            << " << \"}\" << std::endl;" << '\n'
            << '\n'
            << "  std::cout " << '\n'
            << "  << \"Writing test results into file testResults.C\" << std::endl" << '\n'
            << "  << \"Test with the command \\\"root -l testResults.C\\\"\" << std::endl;"
            << '\n'
            << "  std::ofstream fstr(\"testResults.C\");" << '\n'
            << "  fstr << results.str();" << '\n'
            << "  fstr.close();" << '\n'
            << "}" << '\n';
    }
    else {
        glogger
            << "In GNeuralNetworkIndividual::writeVisualizationFile(const std::string&) : Warning!"
            << '\n'
            << "Request to create visualization program for more than two input dimensions!"
            << '\n'
            << "No action taken." << '\n'
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
    if(headerFile == "" || headerFile.empty()) {
        throw geneva_exception(
            g_error_streamer(DO_LOG, time_and_place)
            << "In GNeuralNetworkIndividual::writeTrainedNetwork(const std::string&) : Error"
            << '\n'
            << "Received empty file name." << '\n'
        );
    }

    std::ofstream header(headerFile);
    if(not header) {
        throw geneva_exception(
            g_error_streamer(DO_LOG, time_and_place)
            << "In GNeuralNetworkIndividual::writeTrainedNetwork(const std::string&) :" << '\n'
            << "Error writing output file " << headerFile << '\n'
        );
    }

    header
        << "/**" << '\n'
        << " * @file " << headerFile << '\n'
        << " *" << '\n'
        << " * This file represents the results of a feedforward neural network trained"
        << '\n'
        << " * using the Geneva library. It has been auto-generated by the GNeuralNetworkIndividual"
        << '\n'
        << " * class." << '\n'
        << " */" << '\n'
        << '\n'
        << "/*" << '\n'
        << " * This file is part of the Geneva library collection." << '\n'
        << " * The same license applies to this generated file." << '\n'
        << " */" << '\n'
        << '\n'
        << "#include <cmath>" << '\n'
        << "#include <vector>" << '\n'
        << '\n'
        << "#ifndef GENEVANEURALNETHEADER_HPP_" << '\n'
        << "#define GENEVANEURALNETHEADER_HPP_" << '\n'
        << '\n'
        << "namespace Gem" << '\n'
        << "{" << '\n'
        << "  namespace NeuralNetwork" << '\n'
        << "  {" << '\n'
        << "    double transfer(double value) {" << '\n';

    switch(tF_) {
    case transferFunction::SIGMOID:
        header << "      return 1./(1.+exp(-value));" << '\n';
        break;
    case transferFunction::RBF:
        header << "      return exp(-value*value);" << '\n';
        break;
    }

    header << "    }" << '\n'
           << '\n'
           << "    bool network(const std::vector<double>& in, std::vector<double>& out){"
           << '\n'
           << "      double nodeResult=0.;" << '\n'
           << '\n'
           << "      register std::size_t nodeCounter = 0;" << '\n'
           << "      register std::size_t prevNodeCounter = 0;" << '\n'
           << '\n'
           << "      const std::size_t nLayers = " << this->data_cnt_.size() << ";" << '\n'
           << "      const std::size_t architecture[nLayers] = {" << '\n';

    for(std::size_t i = 0; i < nD_->size(); i++) {
        header << "        " << nD_->at(i);
        if(i == nD_->size() - 1)
            header << '\n';
        else
            header << "," << '\n';
    }

    std::size_t weightOffset = 0;

    header << "      };" << '\n'
           << "      const std::size_t weightOffset[nLayers] = {" << '\n'
           << "        " << weightOffset << "," << '\n';

    weightOffset += 2 * (*nD_)[0];
    header << "       " << weightOffset << "," << '\n';

    for(std::size_t i = 1; i < nD_->size() - 1; i++) {
        weightOffset += (*nD_)[i] * ((*nD_)[i - 1] + 1);
        header << "        " << weightOffset;

        if(i == nD_->size() - 1)
            header << '\n';
        else
            header << "," << '\n';
    }

    header << "      };" << '\n';

    std::size_t nWeights = 2 * (*nD_)[0]; // NOLINT(cppcoreguidelines-init-variables)
    for(std::size_t i = 1; i < nD_->size(); i++) {
        nWeights += (*nD_)[i] * ((*nD_)[i - 1] + 1);
    }

    header << "      const std::size_t nWeights = " << nWeights << ";" << '\n'
           << "      const double weights[nWeights] = {" << '\n';

    for(std::size_t i = 0; i < nD_->size(); i++) {
        std::shared_ptr<GDoubleObjectCollection> currentLayer = at<GDoubleObjectCollection>(i);

        for(std::size_t j = 0; j < currentLayer->size(); j++) {
            header << "        " << currentLayer->at(j)->value();

            if(i == (nD_->size() - 1) && j == (currentLayer->size() - 1))
                header << '\n';
            else
                header << "," << '\n';
        }
    }

    header
        << "      };" << '\n'
        << '\n'
        << "      // Rudimentary error check" << '\n'
        << "      if(in.size() != architecture[0]) return false;" << '\n'
        << '\n'
        << "      // Clear the result vector" << '\n'
        << "      out.clear();" << '\n'
        << '\n'
        << "      // The input layer" << '\n'
        << "      std::vector<double> prevResults;" << '\n'
        << "      std::size_t nLayerNodes = architecture[0];" << '\n'
        << "      std::size_t nPrevLayerNodes = 0;" << '\n'
        << '\n'
        << "      for(nodeCounter=0; nodeCounter<nLayerNodes; nodeCounter++){" << '\n'
        << "        nodeResult=in[nodeCounter] * weights[2*nodeCounter] - weights[2*nodeCounter+1];"
        << '\n'
        << "        nodeResult=transfer(nodeResult);" << '\n'
        << "        prevResults.push_back(nodeResult);" << '\n'
        << "      }" << '\n'
        << '\n'
        << "      // All other layers" << '\n'
        << "      for(register std::size_t layerCounter=1; layerCounter<nLayers; layerCounter++){"
        << '\n'
        << "        std::vector<double> currentResults;" << '\n'
        << "        nLayerNodes=architecture[layerCounter];" << '\n'
        << "        nPrevLayerNodes=architecture[layerCounter-1];" << '\n'
        << '\n'
        << "        // For each node" << '\n'
        << "        for(nodeCounter=0; nodeCounter<nLayerNodes; nodeCounter++){" << '\n'
        << "          nodeResult=0.;" << '\n'
        << "          // Loop over all nodes of the previous layer" << '\n'
        << "          for(prevNodeCounter=0; prevNodeCounter<nPrevLayerNodes; prevNodeCounter++){"
        << '\n'
        << "            nodeResult += "
           "prevResults[prevNodeCounter]*weights[weightOffset[layerCounter] + "
           "nodeCounter*(nPrevLayerNodes+1)+prevNodeCounter];"
        << '\n'
        << "          }" << '\n'
        << "          nodeResult -= weights[weightOffset[layerCounter] + "
           "nodeCounter*(nPrevLayerNodes+1)+nPrevLayerNodes];"
        << '\n'
        << "          nodeResult = transfer(nodeResult);" << '\n'
        << "          currentResults.push_back(nodeResult);" << '\n'
        << "        }" << '\n'
        << '\n'
        << "        prevResults=currentResults;" << '\n'
        << "      }" << '\n'
        << '\n'
        << "      // At this point prevResults should contain the output values of the output layer"
        << '\n'
        << "      out=prevResults;" << '\n'
        << '\n'
        << "      return true;" << '\n'
        << "    }" << '\n'
        << '\n'
        << "  } /* namespace NeuralNetwork */" << '\n'
        << "} /* namespace Gem */" << '\n'
        << '\n'
        << "#endif /* GENEVANEURALNETHEADER_HPP_ */" << '\n';

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
    const GNeuralNetworkIndividual *p_load =
        Gem::Common::g_convert_and_compare<GObject, GNeuralNetworkIndividual>(cp, this);

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
    std::optional<std::shared_ptr<trainingSet>> o;
    while((o = nD_->getTrainingSet(currentPos++))) {
        // Retrieve a constant reference to the training data set for faster access
        const trainingSet &tS = **o;

        // The input layer
        std::vector<double> prevResults;
        std::size_t nLayerNodes = (*nD_)[0]; // NOLINT(cppcoreguidelines-init-variables)
        double nodeResult = 0;
        const GDoubleObjectCollection &inputLayer = *(at<GDoubleObjectCollection>(0));
        for(std::size_t nodeCounter = 0; nodeCounter < nLayerNodes; nodeCounter++) {
            nodeResult = tS.Input[nodeCounter] * inputLayer[2 * nodeCounter]->value() -
                         inputLayer[2 * nodeCounter + 1]->value();
            nodeResult = transfer(nodeResult);
            prevResults.push_back(nodeResult);
        }

        // All other layers
        std::size_t nLayers = this->data_cnt_.size();
        for(std::size_t layerCounter = 1; layerCounter < nLayers; layerCounter++) {
            std::vector<double> currentResults;
            nLayerNodes = (*nD_)[layerCounter];
            std::size_t nPrevLayerNodes =
                (*nD_)[layerCounter - 1]; // NOLINT(cppcoreguidelines-init-variables)
            const GDoubleObjectCollection &currentLayer =
                *(at<GDoubleObjectCollection>(layerCounter));

            for(std::size_t nodeCounter = 0; nodeCounter < nLayerNodes; nodeCounter++) {
                // Loop over all nodes of the previous layer
                nodeResult = 0.;
                for(std::size_t prevNodeCounter = 0; prevNodeCounter < nPrevLayerNodes;
                    prevNodeCounter++) {
                    nodeResult +=
                        prevResults.at(prevNodeCounter) *
                        (currentLayer[nodeCounter * (nPrevLayerNodes + 1) + prevNodeCounter])
                            ->value();
                }
                nodeResult -=
                    (currentLayer[nodeCounter * (nPrevLayerNodes + 1) + nPrevLayerNodes])->value();
                nodeResult = transfer(nodeResult);
                currentResults.push_back(nodeResult);
            }

            prevResults = currentResults;
        }

        // At this point prevResults should contain the output values of the output layer

        // Calculate the error made and add it to the result
        std::size_t prefResultsSize = prevResults.size();
        for(std::size_t nodeCounter = 0; nodeCounter < prefResultsSize; nodeCounter++) {
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
    switch(tF_) {
    case transferFunction::SIGMOID: {
        return 1. / (1. + exp(-value));
    } break;

    case transferFunction::RBF: {
        return exp(-GSQUARED(value));
    } break;

    default: {
        throw geneva_exception(
            g_error_streamer(DO_LOG, time_and_place)
            << "In GNeuralNetworkIndividual::transfer(): Error!" << '\n'
            << "Got invalid tranfer function " << tF_ << '\n'
        );
    } break;
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
GNeuralNetworkIndividualFactory::GNeuralNetworkIndividualFactory(
    std::filesystem::path const &configFile
)
  : Gem::Common::GFactoryT<GParameterSet>(configFile)
  , adProb_(GNN_DEF_ADPROB)
  , adaptAdProb_(GNN_DEF_ADAPTADPROB)
  , minAdProb_(GNN_DEF_MINADPROB)
  , maxAdProb_(GNN_DEF_MAXADPROB)
  , sigma_(GNN_DEF_SIGMA)
  , sigmaSigma_(GNN_DEF_SIGMASIGMA)
  , minSigma_(GNN_DEF_MINSIGMA)
  , maxSigma_(GNN_DEF_MAXSIGMA)
  , minVar_(GNN_DEF_MINVAR)
  , maxVar_(GNN_DEF_MAXVAR)
  , tF_(GNN_DEF_TRANSFER) { /* nothing */
}

/******************************************************************************/
/**
 * The destructor
 */
GNeuralNetworkIndividualFactory::~GNeuralNetworkIndividualFactory() { /* nothing */
}

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
std::shared_ptr<GParameterSet> GNeuralNetworkIndividualFactory::getObject_(
    Gem::Common::GParserBuilder &gpb,
    const std::size_t & /*id*/
) {
    // Will hold the result
    std::shared_ptr<GNeuralNetworkIndividual> target(new GNeuralNetworkIndividual());

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

    std::string comment; // NOLINT(cppcoreguidelines-init-variables)

    comment = "";
    comment += "The probability for random adaptions of values in evolutionary algorithms;";
    gpb.registerFileParameter<double>(
        "adProb",
        adProb_,
        GNN_DEF_ADPROB,
        Gem::Common::VAR_IS_ESSENTIAL,
        comment
    );

    comment = "";
    comment +=
        "Determines the rate of adaption of adProb. Set to 0, if you do not need this feature;";
    gpb.registerFileParameter<double>(
        "adaptAdProb",
        adaptAdProb_,
        GNN_DEF_ADAPTADPROB,
        Gem::Common::VAR_IS_ESSENTIAL,
        comment
    );

    comment = "";
    comment += "The lower allowed boundary for adProb-variation;";
    gpb.registerFileParameter<double>(
        "minAdProb",
        minAdProb_,
        GNN_DEF_MINADPROB,
        Gem::Common::VAR_IS_ESSENTIAL,
        comment
    );

    comment = "";
    comment += "The upper allowed boundary for adProb-variation;";
    gpb.registerFileParameter<double>(
        "maxAdProb",
        maxAdProb_,
        GNN_DEF_MAXADPROB,
        Gem::Common::VAR_IS_ESSENTIAL,
        comment
    );

    comment = "";
    comment += "The sigma for gauss-adaption in ES;";
    gpb.registerFileParameter<double>(
        "sigma",
        sigma_,
        GNN_DEF_SIGMA,
        Gem::Common::VAR_IS_ESSENTIAL,
        comment
    );

    comment = "";
    comment += "Influences the self-adaption of gauss-mutation in ES;";
    gpb.registerFileParameter<double>(
        "sigmaSigma",
        sigmaSigma_,
        GNN_DEF_SIGMASIGMA,
        Gem::Common::VAR_IS_ESSENTIAL,
        comment
    );

    comment = "";
    comment += "The minimum amount value of sigma;";
    gpb.registerFileParameter<double>(
        "minSigma",
        minSigma_,
        GNN_DEF_MINSIGMA,
        Gem::Common::VAR_IS_ESSENTIAL,
        comment
    );

    comment = "";
    comment += "The maximum amount value of sigma;";
    gpb.registerFileParameter<double>(
        "maxSigma",
        maxSigma_,
        GNN_DEF_MAXSIGMA,
        Gem::Common::VAR_IS_ESSENTIAL,
        comment
    );

    comment = "";
    comment += "The lower boundary of the initialization range for parameters;";
    gpb.registerFileParameter<double>(
        "minVar",
        minVar_,
        GNN_DEF_MINVAR,
        Gem::Common::VAR_IS_ESSENTIAL,
        comment
    );

    comment = "";
    comment += "The upper boundary of the initialization range for parameters;";
    gpb.registerFileParameter<double>(
        "maxVar",
        maxVar_,
        GNN_DEF_MAXVAR,
        Gem::Common::VAR_IS_ESSENTIAL,
        comment
    );

    comment = "";
    comment += "The transferFunction: SIGMOID (0) or RBF/Radial Basis (1);";
    gpb.registerFileParameter<transferFunction>(
        "transferFunction",
        tF_,
        GNN_DEF_TRANSFER,
        Gem::Common::VAR_IS_ESSENTIAL,
        comment
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
void GNeuralNetworkIndividualFactory::postProcess_(std::shared_ptr<GParameterSet> &p_raw) {
    // Convert the base pointer to the target type
    std::shared_ptr<GNeuralNetworkIndividual> p =
        Gem::Common::convertSmartPointer<GParameterSet, GNeuralNetworkIndividual>(p_raw);

    // Call the initialization function with our parsed data
    p->init(
        minVar_,
        maxVar_,
        sigma_,
        sigmaSigma_,
        minSigma_,
        maxSigma_,
        adProb_,
        adaptAdProb_,
        minAdProb_,
        maxAdProb_
    );

    // Set the transfer function
    p->setTransferFunction(tF_);
}

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/

} /* namespace Gem::Geneva */

namespace Gem::Common {

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/
/**
 * A factory function for networkData objects, used by GSingletonT. It
 * queries a global options store for the name of the network data file
 *
 * @return A std::shared_ptr to a newly created T object
 */
template <>
std::shared_ptr<Gem::Geneva::networkData> TFactory_GSingletonT() {
    if(GNeuralNetworkOptions->exists("trainingDataFile")) {
        return std::make_shared<Gem::Geneva::networkData>(GNeuralNetworkOptions->get("trainingDataFile"));
    }
    else {
        return std::make_shared<Gem::Geneva::networkData>(Gem::Geneva::GNN_DEF_DATAFILE);
    }
}

} /* namespace Gem::Common */

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/
