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

#include <geneva/individuals/GNeuralNetworkIndividual.hpp>
#include "common/GCommonEnums.hpp"
#include "common/GCommonHelperFunctionsT.hpp"
#include "common/GCommonInterfaceT.hpp"
#include "common/GCommonMathHelperFunctionsT.hpp"
#include "common/GContainerT.hpp"
#include "common/GExceptions.hpp"
#include "common/GExpectationChecksT.hpp"
#include "common/GFactoryT.hpp"
#include "common/GLogger.hpp"
#include "common/GParserBuilder.hpp"
#include "common/GSingletonT.hpp"
#include "geneva/ind/GFlatGenome.hpp"
#include "geneva/ind/GGenomeBuilder.hpp"
#include "geneva/oa/GAdaption.hpp"
#include "geneva/oa/GAdaptionConfig.hpp"
#include <cmath>
#include <cstddef>
#include <filesystem>
#include <fstream>
#include <istream>
#include <memory>
#include <mutex>
#include <optional>
#include <ostream>
#include <random>
#include <sstream>
#include <tuple>
#include <vector>

BOOST_CLASS_EXPORT_IMPLEMENT(Gem::Geneva::Individuals::trainingSet)              // NOLINT
BOOST_CLASS_EXPORT_IMPLEMENT(Gem::Geneva::Individuals::networkData)              // NOLINT
BOOST_CLASS_EXPORT_IMPLEMENT(Gem::Geneva::Individuals::GNeuralNetworkIndividual) // NOLINT
namespace Gem::Geneva::Individuals {

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
trainingSet::trainingSet(const std::size_t &n_input, const std::size_t &n_output)
  : nInputNodes(n_input)
  , nOutputNodes(n_output)
  , Input(new double[n_input])
  , Output(new double[n_output]) {
    // Make sure the arrays are properly initialized
    for(std::size_t i = 0; i < n_input; i++) {
        Input[i] = 0.;
    }
    for(std::size_t o = 0; o < n_output; o++) {
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
 * @param cp A constant reference to another GFlatGenome object
 * @param e The expected outcome of the comparison
 */
void trainingSet::compare(
    const trainingSet &cp,
    const Gem::Common::expectation &e,
    [[maybe_unused]] const double & limit
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
  : array_size_(0)
  , data_(nullptr) { /* nothing */
}

/******************************************************************************/
/**
 * Initialization with the amount of entries
 *
 * @param array_size The desired size of the array
 */
networkData::networkData(const std::size_t &array_size)
  : array_size_(array_size)
  , data_(new std::shared_ptr<trainingSet>[array_size_]) { /* nothing */
}

/******************************************************************************/
/**
 * Initializes the object with data from a file
 *
 * @param network_data_file The name of a file holding the training data
 */
networkData::networkData(const std::string &network_data_file)
  : array_size_(0)
  , data_(nullptr) {
    this->loadFromDisk(network_data_file);
}

/******************************************************************************/
/**
 * Initializes with data from another networkData object
 *
 * @param cp A copy of another networkData object
 */
networkData::networkData(const networkData &cp)
  : Gem::Common::GPodContainerT<std::size_t>(cp)
  , array_size_(0)
  , data_(nullptr) {
    // Make sure the local data is copied
    Gem::Common::copySmartPointerArrays(cp.data_, data_, cp.array_size_, array_size_);
}

/******************************************************************************/
/**
 * A standard destructor.
 */
networkData::~networkData() {
    // Make sure the data vector is empty
    if(data_) {
        for(std::size_t i = 0; i < array_size_; i++) {
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
    Gem::Common::copySmartPointerArrays(cp.data_, data_, cp.array_size_, array_size_);
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
 */
void networkData::compare(
    const networkData &cp,
    const Gem::Common::expectation &e,
    [[maybe_unused]] const double & limit
) const {
    Gem::Common::GToken token("networkData", e);

    // Compare our local data
    Gem::Common::compare_t(IDENTITY(array_size_, cp.array_size_), token);
    Gem::Common::compare_t(IDENTITY(this->data_cnt_, cp.data_cnt_), token);

    // React on deviations from the expectation
    token.evaluate();
}

/******************************************************************************/
/**
 * Saves the data of this struct to disc
 *
 * @param network_data_file The name of the file that data should be saved to
 */
void networkData::saveToDisk(const std::string &network_data_file) const {
    std::ofstream tr_dat(network_data_file);

    if(not tr_dat) {
        throw geneva_exception(
            g_error_streamer(DO_LOG, Gem::Common::timeAndPlace())
            << "In networkData::saveToDisk(const std::string&) : Error!" << '\n'
            << "Data file " << network_data_file << " could not be opened for writing." << '\n'
        );
    }

    // Load the data, using the Boost.Serialization library
    {
        const networkData *local = this;
        boost::archive::xml_oarchive oa(tr_dat);
        oa << boost::serialization::make_nvp("networkData", local);
    } // Explicit scope at this point is essential so that ia's destructor is called

    tr_dat.close();
}

/******************************************************************************/
/**
 * Loads training data from the disc
 *
 * @param network_data_file The name of the file from which data should be loaded
 */
void networkData::loadFromDisk(const std::string &network_data_file) {
    networkData *raw = nullptr;

    std::ifstream tr_dat(network_data_file.c_str());

    if(not tr_dat) {
        std::ostringstream error; // NOLINT(cppcoreguidelines-init-variables)
        error << "In networkData::loadFromDisk(const std::string&):" << '\n'
              << "Data file " << network_data_file << " could not be opened for reading." << '\n';

        if(not std::filesystem::exists(network_data_file.c_str())) {
            error << "File does not exist." << '\n';
        }

        throw geneva_exception(
            g_error_streamer(DO_LOG, Gem::Common::timeAndPlace()) << error.str() << '\n'
        );
    }

    // Load the data into raw, using the Boost.Serialization library. A deserialization failure (a
    // truncated file, or -- most commonly -- a training-data file written by an incompatible Geneva /
    // Boost.Serialization version, e.g. an older archive version) otherwise escapes as an uncaught
    // boost::archive exception and aborts the program with a cryptic "XML start/end tag mismatch"
    // message. Catch it and bail out with an actionable diagnostic instead.
    try {
        boost::archive::xml_iarchive ia(tr_dat);
        ia >> boost::serialization::make_nvp("networkData", raw);
    } // Explicit scope at this point is essential so that ia's destructor is called
    catch(const std::exception &e) {
        delete raw; // may be a partially-loaded object the archive does not own (delete nullptr is safe)
        throw geneva_exception(
            g_error_streamer(DO_LOG, Gem::Common::timeAndPlace())
            << "In networkData::loadFromDisk(const std::string&):" << '\n'
            << "Failed to deserialise the training-data file" << '\n'
            << "  " << network_data_file << '\n'
            << "as a Boost.Serialization XML archive. Reason:" << '\n'
            << "  " << e.what() << '\n'
            << "The file is most likely stale or was written by an incompatible version" << '\n'
            << "(for example an older Boost.Serialization archive version). Regenerate it with:" << '\n'
            << "  GNeuralNetwork --trainingDataFile " << network_data_file
            << " --traininDataType <1-4> --nDataSets <N>" << '\n'
        );
    }

    std::unique_ptr<networkData> n_d(raw);

    // Copy the data over, using our own operator=()
    *this = *n_d;
}

/******************************************************************************/
/**
 * Adds a new training set to the collection. Note that the training set isn't
 * cloned, simply a copy of the smart pointer is stored in the internal array.
 *
 * @param t_s A std::shared_ptr<trainingSet> object, pointing to a training set
 * @param pos The position, in which the data set should be stored.
 */
void networkData::addTrainingSet(std::shared_ptr<trainingSet> t_s, const std::size_t &pos) {
    if(pos >= array_size_) {
        throw geneva_exception(
            g_error_streamer(DO_LOG, Gem::Common::timeAndPlace())
            << "In networkData::addTrainingSet(): Error!" << '\n'
            << "pos = " << pos << " exceeds end of array (size = " << array_size_ << ")" << '\n'
        );
    }
    data_[pos] = t_s;
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
    if(pos >= array_size_) {
        return std::nullopt;
    }
            return data_[pos];
   
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
 * @param output_file The name of the file used for the visualization of the input data
 * @param min The minimum value of the distribution to be displayed
 * @param max The maximum value of the distribution to be displayed
 */
void networkData::toROOT(const std::string &output_file, const double &min, const double &max) {
    // Check that we have a matching number of input nodes
    if(getNInputNodes() != 2 || getNOutputNodes() != 1) {
        glogger << "In networkData::toRoot(): Warning!" << '\n'
                << "Got inappropriate number of input and/or output nodes: " << getNInputNodes()
                << "/" << getNOutputNodes() << '\n'
                << "We need 2/1. The function will return without further action." << '\n'
                << GWARNING;
        return;
    }

    std::size_t entries1 = 0;
    std::size_t entries2 = 0;
    std::ofstream of(output_file);

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
       << "  double xarr1[" << array_size_ << "], yarr1[" << array_size_ << "], xarr2[" << array_size_
       << "], yarr2[" << array_size_ << "];" << '\n'
       << '\n'
       << "  // Filling the data sets" << '\n';

    for(std::size_t i = 0; i < array_size_; i++) {
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
       << "  for(std::size_t i=" << entries1 << "; i<" << array_size_ << "; i++) {" << '\n'
       << "    xarr1[i] = 0.;" << '\n'
       << "    yarr1[i] = 0.;" << '\n'
       << "  }" << '\n'
       << "  for(std::size_t i=" << entries2 << "; i<" << array_size_ << "; i++) {" << '\n'
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
    return not init_range_.empty();
}

/******************************************************************************/
/**
 * Allows to set the initialization range
 */
void networkData::setInitRange(const std::vector<std::tuple<double, double>> &init_range) {
    init_range_ = init_range;
}

/******************************************************************************/
/**
 * Allows to retrieve the initialization range
 */
std::vector<std::tuple<double, double>> networkData::getInitRange() const {
    return init_range_;
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
    std::scoped_lock<std::mutex> lock(m_);
    std::shared_ptr<networkData> result(new networkData(*this));
    return result;
}

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/
/**
 * Reads a Gem::Geneva::Individuals::trainingDataType item from a stream. Needed so we
 * can use boost::program_options to read trainingDataType data.
 *
 * @param i The stream the item should be read from
 * @param tdt The item read from the stream
 * @return The std::istream object used to read the item from
 */
std::istream &operator>>(std::istream &i, Gem::Geneva::Individuals::trainingDataType &tdt) {
    Gem::Common::ENUMBASETYPE tmp = 0;
    i >> tmp;

#ifdef DEBUG
    tdt = Gem::Common::narrow<Gem::Geneva::Individuals::trainingDataType>(tmp);
#else
    tdt = static_cast<Gem::Geneva::Individuals::trainingDataType>(tmp);
#endif /* DEBUG */

    return i;
}

/******************************************************************************/
/**
 * Puts a Gem::Geneva::Individuals::trainingDataType item into a stream. Needed so we
 * can use boost::program_options to output trainingDataType data.
 *
 * @param o The ostream the item should be added to
 * @param tdt the item to be added to the stream
 * @return The std::ostream object used to add the item to
 */
std::ostream &operator<<(std::ostream &o, const Gem::Geneva::Individuals::trainingDataType &tdt) {
    Gem::Common::ENUMBASETYPE tmp = static_cast<Gem::Common::ENUMBASETYPE>(tdt);
    o << tmp;
    return o;
}

/******************************************************************************/
/**
 * Reads a Gem::Geneva::Individuals::transferFunction item from a stream. Needed so we
 * can use boost::program_options to read transferFunction data.
 *
 * @param i The stream the item should be read from
 * @param tF The item read from the stream
 * @return The std::istream object used to read the item from
 */
std::istream &operator>>(std::istream &i, Gem::Geneva::Individuals::transferFunction &t_f) {
    Gem::Common::ENUMBASETYPE tmp = 0;
    i >> tmp;

#ifdef DEBUG
    t_f = Gem::Common::narrow<Gem::Geneva::Individuals::transferFunction>(tmp);
#else
    t_f = static_cast<Gem::Geneva::Individuals::transferFunction>(tmp);
#endif /* DEBUG */

    return i;
}

/******************************************************************************/
/**
 * Puts a Gem::Geneva::Individuals::transferFunction item into a stream. Needed so we
 * can use boost::program_options to output transferFunction data.
 *
 * @param o The ostream the item should be added to
 * @param tF the item to be added to the stream
 * @return The std::ostream object used to add the item to
 */
std::ostream &operator<<(std::ostream &o, const Gem::Geneva::Individuals::transferFunction &t_f) {
    Gem::Common::ENUMBASETYPE tmp = static_cast<Gem::Common::ENUMBASETYPE>(t_f);
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
  : t_f_(GNN_DEF_TRANSFER)
  , n_d_(nnTrainingDataStore()) { /* nothing */
}

/******************************************************************************/
/**
 * A constructor which initializes the individual with a suitable set of network layers. It
 * also loads the training data from file.
 */
GNeuralNetworkIndividual::GNeuralNetworkIndividual(
    const double &min,
    const double &max,
    const double &sigma,
    const double &sigma_sigma,
    const double &min_sigma,
    const double &max_sigma,
    const double &ad_prob,
    const double &adapt_ad_prob,
    const double &min_ad_prob,
    const double &max_ad_prob
)
  : t_f_(GNN_DEF_TRANSFER)
  , n_d_(nnTrainingDataStore()) {
    this->init(
        min,
        max,
        sigma,
        sigma_sigma,
        min_sigma,
        max_sigma,
        ad_prob,
        adapt_ad_prob,
        min_ad_prob,
        max_ad_prob
    );
}

/******************************************************************************/
/**
 * A standard copy constructor
 *
 * @param cp A copy of another GNeuralNetworkIndividual object
 */
GNeuralNetworkIndividual::GNeuralNetworkIndividual(const GNeuralNetworkIndividual &cp)
  : gpar::GFlatGenome(cp)
  , t_f_(cp.t_f_)
  , n_d_(nnTrainingDataStore()) // We want a single source for the training data
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
 * @param cp A constant reference to another GFlatGenome object
 * @param e The expected outcome of the comparison
 */
void GNeuralNetworkIndividual::compare_(
    const gpar::GOptimizableEntity &cp,
    const Gem::Common::expectation &e,
    [[maybe_unused]] const double & limit
) const {
    using namespace Gem::Common;

    // Check that we are dealing with a GNeuralNetworkIndividual reference independent of this object and convert the pointer
    const GNeuralNetworkIndividual *p_load =
        Gem::Common::g_convert_and_compare<gpar::GOptimizableEntity, GNeuralNetworkIndividual>(cp, this);

    GToken token("GNeuralNetworkIndividual", e);

    // Compare our parent data ...
    Gem::Common::compare_base_t<gpar::GFlatGenome>(*this, *p_load, token);

    // ... and then the local data, derived from the single localMembers() declaration
    g_compare_members(localMembers(), p_load->localMembers(), token);

    // React on deviations from the expectation
    token.evaluate();
}

/******************************************************************************/
/**
 * A function which initializes the individual with a suitable set of network
 * layers, according to user-specifications.
 */
void GNeuralNetworkIndividual::init(
    const double &min,
    const double &max,
    const double &sigma,
    const double &sigma_sigma,
    const double &min_sigma,
    const double &max_sigma,
    const double &ad_prob,
    const double &adapt_ad_prob,
    const double &min_ad_prob,
    const double &max_ad_prob
) {
#ifdef DEBUG
    if(not n_d_) {
        throw geneva_exception(
            g_error_streamer(DO_LOG, Gem::Common::timeAndPlace())
            << "In GNeuralNetworkIndividual::init([...]): Error!" << '\n'
            << "No network data appears to have been registered." << '\n'
        );
    }
#endif /* DEBUG */

    // Set up our local data structures

    // Check the architecture we've been given and create the layers
    std::size_t n_layers = n_d_->size(); // NOLINT(cppcoreguidelines-init-variables)

    if(n_layers < 2) { // Two layers are required at the minimum (3 and 4 layers are useful)
        throw geneva_exception(
            g_error_streamer(DO_LOG, Gem::Common::timeAndPlace())
            << "In GNeuralNetworkIndividual::init([...]): Error!" << '\n'
            << "Invalid number of layers supplied (" << n_layers << ")." << '\n'
            << "Did you set up the network architecture ?" << '\n'
        );
    }

    std::size_t layer_number = 0;
    std::size_t n_nodes = 0;
    std::size_t n_nodes_previous = 0;

    // Access to uniformly distributed double random values
    std::uniform_real_distribution<double> uniform_real_distribution(min, max);

    // Build a single flat genome holding all of the network's weights, layer by layer (the same
    // ordering the semantic architecture decodes). Each weight is an unbounded double with its own
    // Gauss adaptor; values are random-initialised in [min, max). The per-layer / per-weight meaning
    // is provided by GNeuralNetworkArchitecture, not by the genome layout.
    gpar::GGenomeBuilder gb;
    for(const auto &layer_n_nodes : *n_d_) {
        if(layer_n_nodes) { // Add the next network layer, if possible
            n_nodes = layer_n_nodes;

            const std::size_t n_weights =
                (layer_number == 0) ? (2 * n_nodes) : (n_nodes * (n_nodes_previous + 1));

            for(std::size_t i = 0; i < n_weights; i++) {
                // Structure only: an unbounded double per weight, random-initialised in [min, max). The
                // Gauss adaptor lives on the OA-owned config (GNeuralNetworkIndividualFactory::
                // getAdaptionConfig()), authored from the same sigma / ad_prob parameters.
                gb.addDouble(uniform_real_distribution(gr_)).perimeter(min, max);
            }

            n_nodes_previous = n_nodes;
            layer_number++;
        }
        else {
            throw geneva_exception(
                g_error_streamer(DO_LOG, Gem::Common::timeAndPlace())
                << "In GNeuralNetworkIndividual::init([...]): Error!" << '\n'
                << "Found invalid number of nodes in layer: " << layer_n_nodes << '\n'
                << "Did you set up the network architecture ?" << '\n'
            );
        }
    }

    this->setGenome(gb.build());

    // (Re)build the cached semantic architecture for this geometry.
    nn_arch_ = makeArchitecture(*n_d_);
}

/******************************************************************************/
/**
 * Sets the type of the transfer function
 */
void GNeuralNetworkIndividual::setTransferFunction(transferFunction t_f) {
    t_f_ = t_f;
}

/******************************************************************************/
/**
 * Retrieves the type of the transfer function
 */
transferFunction GNeuralNetworkIndividual::getTransferFunction() const {
    return t_f_;
}

/******************************************************************************/
/**
 * Creates a program which in turn creates a program suitable for visualization of optimization
 * results with the ROOT analysis framework (see http://root.cern.ch for further information).
 *
 * @param vis_file The name of the file the visualization program should be saved to
 */
void GNeuralNetworkIndividual::writeVisualizationFile(const std::string &vis_file) {
    if(vis_file.empty() || vis_file.empty()) {
        throw geneva_exception(
            g_error_streamer(DO_LOG, Gem::Common::timeAndPlace())
            << "In GNeuralNetworkIndividual::writeVisualizationFile(const std::string&) : Error"
            << '\n'
            << "Received empty file name." << '\n'
        );
    }

    std::ofstream vis_program(vis_file);
    if(not vis_program) {
        throw geneva_exception(
            g_error_streamer(DO_LOG, Gem::Common::timeAndPlace())
            << "In GNeuralNetworkIndividual::writeVisualizationFile(const std::string&) :" << '\n'
            << "Attempt to open output file " << vis_file << " for writing failed." << '\n'
        );
    }

    // The following only makes sense if the input dimension is 2
    if(n_d_->getNInputNodes() == 2) {
        double x_low = 0.;
        double x_high = 1.;
        double y_low = 0.;
        double y_high = 1.;

        // Retrieve information about the initialization range
        // We only act if initialization ranges have been registered.
        // If not, than the above default values will be used.
        std::vector<std::tuple<double, double>> init_range = n_d_->getInitRange();
        if(init_range.size() == 2) {
            x_low = std::get<0>(init_range.at(0));
            x_high = std::get<1>(init_range.at(0));
            y_low = std::get<0>(init_range.at(1));
            y_high = std::get<1>(init_range.at(1));
        }

        // Write the header
        vis_program
            << "/**" << '\n'
            << " * @file visualization.C" << '\n'
            << " *" << '\n'
            << " * This program allows to visualize the output of the training example." << '\n'
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
            << "  std::string geometry = \"" << n_d_->getNetworkGeometryString() << "\";" << '\n'
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
            << "      std::cout << \"Error in calculation of network output\" << std::endl;" << '\n'
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
            << "  << \"Test with the command \\\"root -l testResults.C\\\"\" << std::endl;" << '\n'
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
    vis_program.close();
}

/******************************************************************************/
/**
 * Creates a C++ output file for the trained network, suitable for usage in
 * other projects. If you just want to retrieve the C++ description of the network,
 * call this function with an empty string "" .
 *
 * @param header_file The name of the header file the network should be saved in
 */
void GNeuralNetworkIndividual::writeTrainedNetwork(const std::string &header_file) {
    if(header_file.empty() || header_file.empty()) {
        throw geneva_exception(
            g_error_streamer(DO_LOG, Gem::Common::timeAndPlace())
            << "In GNeuralNetworkIndividual::writeTrainedNetwork(const std::string&) : Error"
            << '\n'
            << "Received empty file name." << '\n'
        );
    }

    std::ofstream header(header_file);
    if(not header) {
        throw geneva_exception(
            g_error_streamer(DO_LOG, Gem::Common::timeAndPlace())
            << "In GNeuralNetworkIndividual::writeTrainedNetwork(const std::string&) :" << '\n'
            << "Error writing output file " << header_file << '\n'
        );
    }

    header
        << "/**" << '\n'
        << " * @file " << header_file << '\n'
        << " *" << '\n'
        << " * This file represents the results of a feedforward neural network trained" << '\n'
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

    switch(t_f_) {
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
           << "      double node_result=0.;" << '\n'
           << '\n'
           << "      register std::size_t node_counter = 0;" << '\n'
           << "      register std::size_t prev_node_counter = 0;" << '\n'
           << '\n'
           << "      const std::size_t n_layers = " << n_d_->size() << ";" << '\n'
           << "      const std::size_t architecture[n_layers] = {" << '\n';

    for(std::size_t i = 0; i < n_d_->size(); i++) {
        header << "        " << n_d_->at(i);
        if(i == n_d_->size() - 1) {
            header << '\n';
        }
        else {
            header << "," << '\n';
        }
    }

    std::size_t weight_offset = 0;

    header << "      };" << '\n'
           << "      const std::size_t weight_offset[n_layers] = {" << '\n'
           << "        " << weight_offset << "," << '\n';

    weight_offset += 2 * (*n_d_)[0];
    header << "       " << weight_offset << "," << '\n';

    for(std::size_t i = 1; i < n_d_->size() - 1; i++) {
        weight_offset += (*n_d_)[i] * ((*n_d_)[i - 1] + 1);
        header << "        " << weight_offset;

        if(i == n_d_->size() - 1) {
            header << '\n';
        }
        else {
            header << "," << '\n';
        }
    }

    header << "      };" << '\n';

    std::size_t n_weights = 2 * (*n_d_)[0]; // NOLINT(cppcoreguidelines-init-variables)
    for(std::size_t i = 1; i < n_d_->size(); i++) {
        n_weights += (*n_d_)[i] * ((*n_d_)[i - 1] + 1);
    }

    header << "      const std::size_t n_weights = " << n_weights << ";" << '\n'
           << "      const double weights[n_weights] = {" << '\n';

    // The flat genome already stores the weights in layer-concatenated order (the same order the
    // architecture decodes), so dump them straight from the genome-agnostic §2 view.
    std::vector<double> all_weights;
    this->streamlineFP(all_weights);
    for(std::size_t i = 0; i < all_weights.size(); i++) {
        header << "        " << all_weights[i];

        if(i == (all_weights.size() - 1)) {
            header << '\n';
        }
        else {
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
        << "      std::vector<double> prev_results;" << '\n'
        << "      std::size_t n_layer_nodes = architecture[0];" << '\n'
        << "      std::size_t n_prev_layer_nodes = 0;" << '\n'
        << '\n'
        << "      for(node_counter=0; node_counter<n_layer_nodes; node_counter++){" << '\n'
        << "        node_result=in[node_counter] * weights[2*node_counter] - weights[2*node_counter+1];"
        << '\n'
        << "        node_result=transfer(node_result);" << '\n'
        << "        prev_results.push_back(node_result);" << '\n'
        << "      }" << '\n'
        << '\n'
        << "      // All other layers" << '\n'
        << "      for(register std::size_t layer_counter=1; layer_counter<n_layers; layer_counter++){"
        << '\n'
        << "        std::vector<double> current_results;" << '\n'
        << "        n_layer_nodes=architecture[layer_counter];" << '\n'
        << "        n_prev_layer_nodes=architecture[layer_counter-1];" << '\n'
        << '\n'
        << "        // For each node" << '\n'
        << "        for(node_counter=0; node_counter<n_layer_nodes; node_counter++){" << '\n'
        << "          node_result=0.;" << '\n'
        << "          // Loop over all nodes of the previous layer" << '\n'
        << "          for(prev_node_counter=0; prev_node_counter<n_prev_layer_nodes; prev_node_counter++){"
        << '\n'
        << "            node_result += "
           "prev_results[prev_node_counter]*weights[weight_offset[layer_counter] + "
           "node_counter*(n_prev_layer_nodes+1)+prev_node_counter];"
        << '\n'
        << "          }" << '\n'
        << "          node_result -= weights[weight_offset[layer_counter] + "
           "node_counter*(n_prev_layer_nodes+1)+n_prev_layer_nodes];"
        << '\n'
        << "          node_result = transfer(node_result);" << '\n'
        << "          current_results.push_back(node_result);" << '\n'
        << "        }" << '\n'
        << '\n'
        << "        prev_results=current_results;" << '\n'
        << "      }" << '\n'
        << '\n'
        << "      // At this point prev_results should contain the output values of the output layer"
        << '\n'
        << "      out=prev_results;" << '\n'
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
 * Loads the data of another GNeuralNetworkIndividual, camouflaged as a GFlatGenome
 *
 * @param cp A copy of another GNeuralNetworkIndividual, camouflaged as a GFlatGenome
 */
void GNeuralNetworkIndividual::load_(const gpar::GOptimizableEntity *cp) {
    // Check that we are dealing with a GNeuralNetworkIndividual reference independent of this object and convert the pointer
    const GNeuralNetworkIndividual *p_load =
        Gem::Common::g_convert_and_compare<gpar::GOptimizableEntity, GNeuralNetworkIndividual>(cp, this);

    // Load the parent class'es data
    gpar::GFlatGenome::load_(cp);

    // Load our local data, derived from the single localMembers() declaration.
    // We do not copy the network data, as it is always initialized through
    // the constructors, even in the case of a copy constructor
    Gem::Common::g_load_members(localMembers(), p_load->localMembers());
}

/******************************************************************************/
/**
 * Creates a deep clone of this object
 *
 * @return A deep clone of this object, camouflaged as a GFlatGenome
 */
gpar::GFlatGenome *GNeuralNetworkIndividual::clone_() const {
    return new GNeuralNetworkIndividual(*this);
}

/******************************************************************************/
/**
 * Builds the shared, immutable semantic architecture for a given network geometry (DM §4). The layer
 * sizes are read from the networkData; the architecture then exposes per-layer weight offsets into the
 * flat genome, layout-agnostically.
 */
std::shared_ptr<const GNeuralNetworkArchitecture>
GNeuralNetworkIndividual::makeArchitecture(const networkData &n_d) {
    std::vector<std::size_t> layers;
    layers.reserve(n_d.size());
    for(std::size_t i = 0; i < n_d.size(); ++i) {
        layers.push_back(n_d.at(i));
    }
    return std::make_shared<const GNeuralNetworkArchitecture>(std::move(layers));
}

/******************************************************************************/
/**
 * Lazily (re)builds and returns the cached semantic architecture. The cache is transient (not
 * serialised, not copied), so it is rebuilt on first use after construction, copy or deserialisation;
 * the network geometry always comes from the (singleton-backed) networkData.
 */
const GNeuralNetworkArchitecture &GNeuralNetworkIndividual::architecture() const {
    if(not nn_arch_) {
        nn_arch_ = makeArchitecture(*n_d_);
    }
    return *nn_arch_;
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
 * @return The fitness of this object
 */
double GNeuralNetworkIndividual::fitnessCalculation() {
    double result = 0;

    // Read all weights out of the flat genome once (genome-agnostic §2 access), then decode their
    // per-layer meaning through the semantic architecture.
    std::vector<double> w;
    this->streamlineFP(w);
    const GNeuralNetworkArchitecture &arch = this->architecture();

    // Now loop over all data sets
    std::size_t current_pos = 0;
    std::optional<std::shared_ptr<trainingSet>> o;
    while((o = n_d_->getTrainingSet(current_pos++))) {
        // Retrieve a constant reference to the training data set for faster access
        const trainingSet &t_s = **o;

        // The input layer (layer 0): two weights per input node (multiplier + bias).
        std::vector<double> prev_results;
        std::size_t n_layer_nodes = arch.layerSize(0);
        double node_result = 0;
        const std::size_t input_offset = arch.layerOffset(0);
        for(std::size_t node_counter = 0; node_counter < n_layer_nodes; node_counter++) {
            node_result = t_s.Input[node_counter] * w[input_offset + 2 * node_counter] -
                          w[input_offset + 2 * node_counter + 1];
            node_result = transfer(node_result);
            prev_results.push_back(node_result);
        }

        // All other layers: one weight per previous-layer node plus a bias, per node.
        std::size_t n_layers = arch.nLayers();
        for(std::size_t layer_counter = 1; layer_counter < n_layers; layer_counter++) {
            std::vector<double> current_results;
            n_layer_nodes = arch.layerSize(layer_counter);
            std::size_t n_prev_layer_nodes = arch.layerSize(layer_counter - 1);
            const std::size_t layer_offset = arch.layerOffset(layer_counter);

            for(std::size_t node_counter = 0; node_counter < n_layer_nodes; node_counter++) {
                // Loop over all nodes of the previous layer
                node_result = 0.;
                for(std::size_t prev_node_counter = 0; prev_node_counter < n_prev_layer_nodes;
                    prev_node_counter++) {
                    node_result +=
                        prev_results.at(prev_node_counter) *
                        w[layer_offset + node_counter * (n_prev_layer_nodes + 1) + prev_node_counter];
                }
                node_result -=
                    w[layer_offset + node_counter * (n_prev_layer_nodes + 1) + n_prev_layer_nodes];
                node_result = transfer(node_result);
                current_results.push_back(node_result);
            }

            prev_results = current_results;
        }

        // At this point prev_results should contain the output values of the output layer

        // Calculate the error made and add it to the result
        std::size_t pref_results_size = prev_results.size();
        for(std::size_t node_counter = 0; node_counter < pref_results_size; node_counter++) {
            result += Gem::Common::gsquared(prev_results.at(node_counter) - t_s.Output[node_counter]);
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
    switch(t_f_) {
    case transferFunction::SIGMOID: {
        return 1. / (1. + exp(-value));
    } break;

    case transferFunction::RBF: {
        return exp(-Gem::Common::gsquared(value));
    } break;

    default: {
        throw geneva_exception(
            g_error_streamer(DO_LOG, Gem::Common::timeAndPlace())
            << "In GNeuralNetworkIndividual::transfer(): Error!" << '\n'
            << "Got invalid tranfer function " << t_f_ << '\n'
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
 * @param config_file The name of the configuration file
 */
GNeuralNetworkIndividualFactory::GNeuralNetworkIndividualFactory(
    std::filesystem::path const &config_file
)
  : Gem::Common::GFactoryT<gpar::GOptimizableEntity>(config_file)
  , ad_prob_(GNN_DEF_ADPROB)
  , adapt_ad_prob_(GNN_DEF_ADAPTADPROB)
  , min_ad_prob_(GNN_DEF_MINADPROB)
  , max_ad_prob_(GNN_DEF_MAXADPROB)
  , sigma_(GNN_DEF_SIGMA)
  , sigma_sigma_(GNN_DEF_SIGMASIGMA)
  , min_sigma_(GNN_DEF_MINSIGMA)
  , max_sigma_(GNN_DEF_MAXSIGMA)
  , min_var_(GNN_DEF_MINVAR)
  , max_var_(GNN_DEF_MAXVAR)
  , t_f_(GNN_DEF_TRANSFER) { /* nothing */
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
void GNeuralNetworkIndividualFactory::setTransferFunction(transferFunction t_f) {
    t_f_ = t_f;
}

/******************************************************************************/
/**
 * Retrieves the type of the transfer function
 */
transferFunction GNeuralNetworkIndividualFactory::getTransferFunction() const {
    return t_f_;
}

/******************************************************************************/
/**
 * Creates items of this type
 *
 * @return Items of the desired type
 */
std::shared_ptr<gpar::GOptimizableEntity> GNeuralNetworkIndividualFactory::getObject_(
    Gem::Common::GParserBuilder &gpb,
    [[maybe_unused]] const std::size_t & id
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
        "ad_prob",
        ad_prob_,
        GNN_DEF_ADPROB,
        Gem::Common::VAR_IS_ESSENTIAL,
        comment
    );

    comment = "";
    comment +=
        "Determines the rate of adaption of ad_prob. Set to 0, if you do not need this feature;";
    gpb.registerFileParameter<double>(
        "adapt_ad_prob",
        adapt_ad_prob_,
        GNN_DEF_ADAPTADPROB,
        Gem::Common::VAR_IS_ESSENTIAL,
        comment
    );

    comment = "";
    comment += "The lower allowed boundary for ad_prob-variation;";
    gpb.registerFileParameter<double>(
        "min_ad_prob",
        min_ad_prob_,
        GNN_DEF_MINADPROB,
        Gem::Common::VAR_IS_ESSENTIAL,
        comment
    );

    comment = "";
    comment += "The upper allowed boundary for ad_prob-variation;";
    gpb.registerFileParameter<double>(
        "max_ad_prob",
        max_ad_prob_,
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
        "sigma_sigma",
        sigma_sigma_,
        GNN_DEF_SIGMASIGMA,
        Gem::Common::VAR_IS_ESSENTIAL,
        comment
    );

    comment = "";
    comment += "The minimum amount value of sigma;";
    gpb.registerFileParameter<double>(
        "min_sigma",
        min_sigma_,
        GNN_DEF_MINSIGMA,
        Gem::Common::VAR_IS_ESSENTIAL,
        comment
    );

    comment = "";
    comment += "The maximum amount value of sigma;";
    gpb.registerFileParameter<double>(
        "max_sigma",
        max_sigma_,
        GNN_DEF_MAXSIGMA,
        Gem::Common::VAR_IS_ESSENTIAL,
        comment
    );

    comment = "";
    comment += "The lower boundary of the initialization range for parameters;";
    gpb.registerFileParameter<double>(
        "min_var",
        min_var_,
        GNN_DEF_MINVAR,
        Gem::Common::VAR_IS_ESSENTIAL,
        comment
    );

    comment = "";
    comment += "The upper boundary of the initialization range for parameters;";
    gpb.registerFileParameter<double>(
        "max_var",
        max_var_,
        GNN_DEF_MAXVAR,
        Gem::Common::VAR_IS_ESSENTIAL,
        comment
    );

    comment = "";
    comment += "The transferFunction: SIGMOID (0) or RBF/Radial Basis (1);";
    gpb.registerFileParameter<transferFunction>(
        "transfer_function",
        t_f_,
        GNN_DEF_TRANSFER,
        Gem::Common::VAR_IS_ESSENTIAL,
        comment
    );

    // Allow our parent class to describe its options
    Gem::Common::GFactoryT<gpar::GOptimizableEntity>::describeLocalOptions_(gpb);
}

/******************************************************************************/
/**
 * Allows to act on the configuration options received from the configuration file. Here
 * we can add the options described in describeLocalOptions to the object. In practice,
 * we add the parameter objects here
 *
 * @param p_raw A smart-pointer to be acted on during post-processing
 */
void GNeuralNetworkIndividualFactory::postProcess_(std::shared_ptr<gpar::GOptimizableEntity> &p_raw) {
    // Convert the base pointer to the target type
    std::shared_ptr<GNeuralNetworkIndividual> p =
        Gem::Common::convertSmartPointer<gpar::GOptimizableEntity, GNeuralNetworkIndividual>(p_raw);

    // Call the initialization function with our parsed data
    p->init(
        min_var_,
        max_var_,
        sigma_,
        sigma_sigma_,
        min_sigma_,
        max_sigma_,
        ad_prob_,
        adapt_ad_prob_,
        min_ad_prob_,
        max_ad_prob_
    );

    // Set the transfer function
    p->setTransferFunction(t_f_);
}

/******************************************************************************/
/**
 * Builds the OA-owned adaption configuration for a network genome produced by this factory: every weight
 * (one double group each) gets a Gauss adaptor with this factory's configured parameters -- exactly the
 * settings init() formerly baked into the genome layout.
 */
std::shared_ptr<OptimizationAlgorithms::GAdaptionConfigBase>
GNeuralNetworkIndividualFactory::getAdaptionConfig(const gpar::GFlatGenome &sample) const {
    namespace oa = Gem::Geneva::OptimizationAlgorithms;
    auto cfg = oa::makeAdaptionConfig<oa::GAdaptionConfigBase>(sample);
    for(std::size_t i = 0; i < cfg->doubleGroups().size(); i++) {
        cfg->groupDouble(i).gauss(
            sigma_, sigma_sigma_, min_sigma_, max_sigma_, ad_prob_, adapt_ad_prob_, 1,
            Gem::Geneva::adaptionMode::WITHPROBABILITY, min_ad_prob_, max_ad_prob_
        );
    }
    return cfg;
}

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/

} /* namespace Gem::Geneva::Individuals */

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
std::shared_ptr<Gem::Geneva::Individuals::networkData> TFactory_GSingletonT() {
    if(neuralNetworkOptions()->exists("trainingDataFile")) {
        return std::make_shared<Gem::Geneva::Individuals::networkData>(neuralNetworkOptions()->get("trainingDataFile"));
    }
            return std::make_shared<Gem::Geneva::Individuals::networkData>(Gem::Geneva::Individuals::GNN_DEF_DATAFILE);
   
}

} /* namespace Gem::Common */

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/
