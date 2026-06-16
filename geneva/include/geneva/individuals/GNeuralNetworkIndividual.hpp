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

#pragma once

// Global checks, defines and includes needed for all of Geneva
#include "common/GGlobalDefines.hpp"

// Standard header files go here
#include <cmath>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <limits>
#include <mutex>
#include <optional>
#include <sstream>
#include <tuple>
#include <vector>

// Boost header files go here
#include <boost/archive/binary_iarchive.hpp>
#include <boost/archive/binary_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>
#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/xml_iarchive.hpp>
#include <boost/archive/xml_oarchive.hpp>
#include <numbers>
#include <boost/serialization/base_object.hpp>
#include <boost/serialization/export.hpp>
#include <boost/serialization/nvp.hpp>
#include <boost/serialization/shared_ptr.hpp>
#include <boost/serialization/split_member.hpp>
#include <boost/serialization/tracking.hpp>
#include <boost/serialization/utility.hpp>
#include <boost/serialization/vector.hpp>
#include <boost/serialization/version.hpp>

// Geneva header files go here
#include "common/GCommonEnums.hpp"
#include "common/GCommonHelperFunctions.hpp"
#include "common/GCommonHelperFunctionsT.hpp"
#include "common/GExceptions.hpp"
#include "common/GGlobalOptionsT.hpp"
#include "common/GLogger.hpp"
#include "common/GContainerT.hpp"
#include "common/GSingletonT.hpp"
#include "common/GUnitTestFrameworkT.hpp"
#include "geneva/ind/GFlatGenome.hpp"
#include "geneva/ind/GFlatIndividualFactory.hpp"
#include "geneva/ind/GGenomeArchitecture.hpp"
#include "geneva/ind/GGenomeBuilder.hpp"
#include "hap/GRandomT.hpp"

namespace Gem::Geneva::OptimizationAlgorithms {
class GAdaptionConfigBase;
} // namespace Gem::Geneva::OptimizationAlgorithms

namespace Gem::Geneva::Individuals {

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/
/**
 * A single data set holding the training data of a single training iteration
 */
struct trainingSet {
    /////////////////////////////////////////////////////////////////////////////
    friend class boost::serialization::access;

    template <typename Archive>
    void load(Archive &ar, const unsigned int) {
        using boost::serialization::make_nvp;

        ar &BOOST_SERIALIZATION_NVP(nInputNodes) & BOOST_SERIALIZATION_NVP(nOutputNodes);

        if(Input) {
            Gem::Common::g_array_delete(Input);
        }
        Input = new double[nInputNodes];

        if(Output) {
            Gem::Common::g_array_delete(Output);
        }
        Output = new double[nOutputNodes];

        ar &boost::serialization::make_array(Input, nInputNodes);
        ar &boost::serialization::make_array(Output, nOutputNodes);
    }

    template <typename Archive>
    void save(Archive &ar, const unsigned int) const {
        using boost::serialization::make_nvp;

        ar &BOOST_SERIALIZATION_NVP(nInputNodes) & BOOST_SERIALIZATION_NVP(nOutputNodes);

        ar &boost::serialization::make_array(Input, nInputNodes);
        ar &boost::serialization::make_array(Output, nOutputNodes);
    }

    BOOST_SERIALIZATION_SPLIT_MEMBER()

    /////////////////////////////////////////////////////////////////////////////

    /** @brief The constructor */
    trainingSet(const std::size_t &, const std::size_t &);
    /** @brief A copy constructor */
    trainingSet(const trainingSet &);

    /** @brief The destructor */
    virtual ~trainingSet();

    /** @brief Assigns another trainingSet's data to this object */
    trainingSet &operator=(const trainingSet &);
    /** @brief Checks for equality with another trainingSet object */

    /** @brief Searches for compliance with expectations with respect to another object of the same type */
    virtual void compare(
        const trainingSet & // the other object
        ,
        const Gem::Common::expectation & // the expectation for this object, e.g. equality
        ,
        const double & // the limit for allowed deviations of floating point types
    ) const;

    /***************************************************************************/
    // Local data

    std::size_t nInputNodes;  ///< The number of input nodes
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
class networkData : public Gem::Common::GPodContainerT<std::size_t> {
    /////////////////////////////////////////////////////////////////////////////
    friend class boost::serialization::access;

    template <typename Archive>
    void load(Archive &ar, const unsigned int) {
        using boost::serialization::make_nvp;

        ar &make_nvp(
            "GStdSimpleVectorInterfaceT_size_t",
            boost::serialization::base_object<Gem::Common::GPodContainerT<std::size_t>>(*this)
        ) & BOOST_SERIALIZATION_NVP(init_range_);

        // Make sure the data vector is empty
        if(data_) {
            for(std::size_t i = 0; i < array_size_; i++) {
                data_[i].reset();
            }
        }
        Gem::Common::g_array_delete(data_);

        ar &BOOST_SERIALIZATION_NVP(array_size_);

        // array_size_ has just been read from the (possibly untrusted) archive. Reject a value that
        // would overflow the array allocation before handing it to new[]. This also bounds the size
        // for the compiler, silencing g++'s -Walloc-size-larger-than for the allocation below.
        if(array_size_ > std::numeric_limits<std::ptrdiff_t>::max() /
                              static_cast<std::ptrdiff_t>(sizeof(std::shared_ptr<trainingSet>))) {
            throw geneva_exception(
                g_error_streamer(DO_LOG, Gem::Common::timeAndPlace())
                << "In GNeuralNetworkIndividual::load(Archive&):" << '\n'
                << "Deserialized array_size_ = " << array_size_ << " is too large" << '\n'
            );
        }

        data_ = new std::shared_ptr<trainingSet>[array_size_];

        ar &boost::serialization::make_array(data_, array_size_);
    }

    template <typename Archive>
    void save(Archive &ar, const unsigned int) const {
        using boost::serialization::make_nvp;

        ar &make_nvp(
            "GStdSimpleVectorInterfaceT_size_t",
            boost::serialization::base_object<Gem::Common::GPodContainerT<std::size_t>>(*this)
        ) & BOOST_SERIALIZATION_NVP(init_range_) &
            BOOST_SERIALIZATION_NVP(array_size_) &
            boost::serialization::make_array(data_, array_size_);
    }

    BOOST_SERIALIZATION_SPLIT_MEMBER()

    /////////////////////////////////////////////////////////////////////////////

public:
    /***************************************************************************/
    /** @brief Initialization with the amount of entries */
    explicit networkData(const std::size_t &);

    /** @brief Initialization with data from file */
    explicit networkData(const std::string &);
    /** @brief The copy constructor */
    networkData(const networkData &);

    /** @brief A standard destructor. */
    ~networkData() override;

    /** @brief Copies the data of another networkData object */
    networkData &operator=(const networkData &);

    /** @brief Searches for compliance with expectations with respect to another object of the same type */
    virtual void compare(
        const networkData & // the other object
        ,
        const Gem::Common::expectation & // the expectation for this object, e.g. equality
        ,
        const double & // the limit for allowed deviations of floating point types
    ) const;

    /** @brief Saves the data of this struct to disc */
    void saveToDisk(const std::string &) const;
    /** @brief Loads training data from the disc */
    void loadFromDisk(const std::string &);

    /** @brief Adds a new training set to the collection, Requires for the network architecture to be defined already */
    void addTrainingSet(std::shared_ptr<trainingSet>, const std::size_t &);
    /** @brief Retrieves  training set at a given position */
    std::optional<std::shared_ptr<trainingSet>>

    getTrainingSet(const std::size_t &) const;

    /** @brief Retrieves the number of input nodes of this network */
    std::size_t getNInputNodes() const;
    /** @brief Retrieves the number of output nodes of this network */
    std::size_t getNOutputNodes() const;

    /** @brief Saves this data set in ROOT format for visual inspection */
    void toROOT(const std::string &, const double &, const double &);

    /** @brief Allows to check whether an initialization range has been set */
    bool initRangeSet() const;
    /** @brief Allows to set the initialization range */
    void setInitRange(const std::vector<std::tuple<double, double>> &init_range);
    /** @brief Allows to retrieve the initialization range */
    std::vector<std::tuple<double, double>> getInitRange() const;

    /** @brief Allows to retrieve a string that describes the network geometry */
    std::string getNetworkGeometryString() const;

    /** @brief Creates a deep clone of this object */
    std::shared_ptr<networkData> clone() const;

private:
    /***************************************************************************/
    /** @brief Default constructor, intentionally private */
    networkData();

    /***************************************************************************/
    /** @brief The size of the training set */
    std::size_t array_size_;
    /** @brief Holds the individual data items */
    std::shared_ptr<trainingSet> *data_;

    /** @brief Holds the initialization range in each direction */
    std::vector<std::tuple<double, double>> init_range_;

    /** @brief Locks access to the clone function */
    mutable std::mutex m_; ///< Lock get/set operations
};

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/
/** @brief This enum is used to specify the type of training data that should be generated */
enum class trainingDataType : Gem::Common::ENUMBASETYPE {
    TDTNONE = 0,
    HYPERCUBE = 1,
    HYPERSPHERE = 2,
    AXISCENTRIC = 3,
    SINUS = 4
};
/** @brief Allows to specify whether we want to use a sigmoidal transfer function or a radial basis function */
enum class transferFunction : Gem::Common::ENUMBASETYPE {
    SIGMOID = 0,
    RBF = 1
};

/******************************************************************************/
/** @brief  Reads a Gem::Geneva::Individuals::trainingDataType item from a stream */
std::istream &operator>>(std::istream &i, Gem::Geneva::Individuals::trainingDataType &tdt);
/** @brief Puts a Gem::Geneva::Individuals::trainingDataType item into a stream */
std::ostream &
operator<<(std::ostream &o, const Gem::Geneva::Individuals::trainingDataType &tdt);
/** @brief Reads a Gem::Geneva::Individuals::transferFunction item from a stream. */
std::istream &operator>>(std::istream &i, Gem::Geneva::Individuals::transferFunction &t_f);
/** @brief Puts a Gem::Geneva::Individuals::transferFunction item into a stream. */
std::ostream &operator<<(std::ostream &o, const Gem::Geneva::Individuals::transferFunction &t_f);

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/
// A global singleton giving access to the training data.
// See also the definition of TFactory_GSingletonT<Gem::Geneva::Individuals::networkData>
using GDatStore = Gem::Common::GSingletonT<Gem::Geneva::Individuals::networkData>;
// Drop-in replacement for the former GNNTrainingDataStore macro.
[[nodiscard]] inline std::shared_ptr<GDatStore::STYPE> nnTrainingDataStore() {
    return GDatStore::instance();
}

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/
// A number of default settings for the factory
const std::string GNN_DEF_DATAFILE = "./Datasets/hyper_cube.dat";
constexpr double GNN_DEF_ADPROB = 0.05;
constexpr double GNN_DEF_ADAPTADPROB = 0.1;
constexpr double GNN_DEF_MINADPROB = 0.05;
constexpr double GNN_DEF_MAXADPROB = 1.;
constexpr double GNN_DEF_SIGMA = 0.1;
constexpr double GNN_DEF_SIGMASIGMA = 0.8;
constexpr double GNN_DEF_MINSIGMA = 0.01;
constexpr double GNN_DEF_MAXSIGMA = 0.2;
constexpr double GNN_DEF_MINVAR = -10.;
constexpr double GNN_DEF_MAXVAR = 10.;
const transferFunction GNN_DEF_TRANSFER = transferFunction::SIGMOID;

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/
/**
 * The semantic architecture (DM §4) of a feed-forward neural network: a layout-agnostic decoder that
 * maps the individual's flat floating-point genome (the concatenated layer weight vectors) to the
 * network's per-layer weight offsets. It is the first real GGenomeArchitecture user. The weight layout
 * (matching the historical tree genome's GDoubleObjectCollection-per-layer ordering) is:
 *  - input layer (layer 0): 2 * nodes[0] weights (per input node: one multiplier + one bias);
 *  - layer L > 0: nodes[L] * (nodes[L-1] + 1) weights (per node: one weight per previous node + bias).
 * The architecture computes only the structure (offsets / sizes); the forward pass + transfer function
 * live in GNeuralNetworkIndividual::fitnessCalculation(), which reads the weights through this view.
 */
class GNeuralNetworkArchitecture : public gen::GGenomeArchitecture {
public:
    /** @brief Initialization from the per-layer node counts (input ... output) */
    explicit GNeuralNetworkArchitecture(std::vector<std::size_t> layer_sizes)
      : layer_sizes_(std::move(layer_sizes)) {
        offsets_.resize(layer_sizes_.size());
        std::size_t off = 0;
        for(std::size_t l = 0; l < layer_sizes_.size(); ++l) {
            offsets_[l] = off;
            off += weightCount(l);
        }
        total_weights_ = off;
    }

    std::string name() const override { return "GNeuralNetworkArchitecture"; }
    std::size_t expectedFPSize() const override { return total_weights_; }

    /** @brief The number of layers (input + hidden + output) */
    std::size_t nLayers() const { return layer_sizes_.size(); }
    /** @brief The number of nodes in layer l */
    std::size_t layerSize(std::size_t l) const { return layer_sizes_.at(l); }
    /** @brief The offset of layer l's weights into the flat genome */
    std::size_t layerOffset(std::size_t l) const { return offsets_.at(l); }
    /** @brief The number of weights owned by layer l */
    std::size_t weightCount(std::size_t l) const {
        return (l == 0) ? (2 * layer_sizes_[0]) : (layer_sizes_[l] * (layer_sizes_[l - 1] + 1));
    }

private:
    std::vector<std::size_t> layer_sizes_; ///< nodes per layer
    std::vector<std::size_t> offsets_;     ///< per-layer weight offset into the flat genome
    std::size_t total_weights_ = 0;        ///< total number of weights
};

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/
/**
 * With this individual you can use Genevas optimization algorithms instead of the
 * standard back-propagation algorithm to train feed-forward neural networks.
 */
class GNeuralNetworkIndividual // NOLINT(cppcoreguidelines-special-member-functions)
  : public gen::GFlatGenome {
    /////////////////////////////////////////////////////////////////////////////

    friend class boost::serialization::access;

    /** @brief The single declaration of this class'es serialised local data
     *  members. n_d_ is intentionally NOT listed: it is recovered from a global
     *  singleton in load() (asymmetric) rather than stored. */
    auto localMembers() {
        return std::make_tuple(Gem::Common::make_member("t_f_", t_f_));
    }
    auto localMembers() const {
        return std::make_tuple(Gem::Common::make_member("t_f_", t_f_));
    }

    template <typename Archive>
    void load(Archive &ar, const unsigned int) {
        using boost::serialization::make_nvp;

        ar &BOOST_SERIALIZATION_BASE_OBJECT_NVP(gen::GFlatGenome);
        // t_f_ was previously never (de)serialised and silently reset to its
        // default; read it back via the single localMembers() declaration. In a
        // split save()/load(), the same serialize_members() drives both -- the
        // non-const localMembers() overload here yields writable refs to read into.
        Gem::Common::serialize_members(ar, this->localMembers());

        // Load the network data from disk
        n_d_ = nnTrainingDataStore(); // A glogal singleton
    }

    template <typename Archive>
    void save(Archive &ar, const unsigned int) const {
        using boost::serialization::make_nvp;

        ar &BOOST_SERIALIZATION_BASE_OBJECT_NVP(gen::GFlatGenome);
        // The const localMembers() overload yields const refs, which the output
        // archive writes -- the symmetric counterpart to load() above.
        Gem::Common::serialize_members(ar, this->localMembers());
    }

    BOOST_SERIALIZATION_SPLIT_MEMBER()

    /////////////////////////////////////////////////////////////////////////////

public:
    /***************************************************************************/
    /** @brief The default constructor */
    GNeuralNetworkIndividual();
    /** @brief A constructor which initializes the individual with a suitable set of network layers */
    GNeuralNetworkIndividual(
        [[maybe_unused]] const double & min,
        [[maybe_unused]] const double & max
        ,
        [[maybe_unused]] const double & sigma,
        [[maybe_unused]] const double & sigma_sigma
        ,
        [[maybe_unused]] const double & min_sigma,
        [[maybe_unused]] const double & max_sigma
        ,
        [[maybe_unused]] const double & ad_prob,
        [[maybe_unused]] const double & adapt_ad_prob
        ,
        [[maybe_unused]] const double & min_ad_prob,
        [[maybe_unused]] const double & max_ad_prob
    );
    /** @brief A standard copy constructor */
    GNeuralNetworkIndividual(const GNeuralNetworkIndividual &cp);

    /** @brief The standard destructor */
    ~GNeuralNetworkIndividual() override;

    /** @brief Initialization according to user-specifications */
    void init(
        [[maybe_unused]] const double & min,
        [[maybe_unused]] const double & max
        ,
        [[maybe_unused]] const double & sigma,
        [[maybe_unused]] const double & sigma_sigma
        ,
        [[maybe_unused]] const double & min_sigma,
        [[maybe_unused]] const double & max_sigma
        ,
        [[maybe_unused]] const double & ad_prob,
        [[maybe_unused]] const double & adapt_ad_prob
        ,
        [[maybe_unused]] const double & min_ad_prob,
        [[maybe_unused]] const double & max_ad_prob
    );

    /** @brief Sets the type of the transfer function */
    void setTransferFunction(transferFunction t_f);
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
	  * @param n_data_sets The number of training sets to create
	  * @param edgelength The desired edge length of the cube
	  * @return A copy of the networkData struct that has been created, wrapped in a shared_ptr
	  */
    static std::shared_ptr<networkData> createHyperCubeNetworkData(
        const std::vector<std::size_t> &architecture,
        const std::size_t &n_data_sets,
        const double &edgelength
    ) {
        using namespace Gem::Hap;

        // Check the number of supplied layers
        if(architecture.size() < 2) { // We need at least an input- and an output-layer
            throw geneva_exception(
                g_error_streamer(DO_LOG, Gem::Common::timeAndPlace())
                << "In GNeuralNetworkIndividual::createHyperCubeNetworkData(): Error!" << '\n'
                << "Got invalid number of layers: " << architecture.size() << '\n'
            );
        }

        // Check that the output layer has exactly one node
        if(architecture.back() != 1) {
            throw geneva_exception(
                g_error_streamer(DO_LOG, Gem::Common::timeAndPlace())
                << "In GNeuralNetworkIndividual::createHyperCubeNetworkData(): Error!" << '\n'
                << "The output layer must have exactly one node for this training data."
                << '\n'
                << "Got " << architecture.back() << " instead." << '\n'
            );
        }

        // Create a local random number generator.
        GRandomT<RANDFLAVOURS::RANDOMPROXY> gr_l;
        std::uniform_real_distribution<double> uniform_real_distribution;

        // Retrieve the number of input- and output nodes for easier reference
        std::size_t n_input_nodes = architecture.front();
        std::size_t n_output_nodes = architecture.back();

        // The dimension of the hyper-cube is identical to the number of input nodes
        std::size_t n_dim = n_input_nodes;

        // Create the actual networkData object and attach the architecture
        // Checks the architecture on the way
        std::shared_ptr<networkData> n_d(new networkData(n_data_sets));
        std::vector<std::size_t>::const_iterator it;
        std::size_t layer_counter = 0;
        for(it = architecture.begin(); it != architecture.end(); ++it, ++layer_counter) {
            if(*it == 0) {
                throw geneva_exception(
                    g_error_streamer(DO_LOG, Gem::Common::timeAndPlace())
                    << "In GNeuralNetworkIndividual::createHyperCubeNetworkData(): Error!" << '\n'
                    << "Layer " << layer_counter << "has invalid size " << *it << '\n'
                );
            }

            n_d->push_back(*it);
        }

        // Create the required data.
        bool outside = false;
        for(std::size_t dat_counter = 0; dat_counter < n_data_sets; dat_counter++) {
            outside = false;
            std::shared_ptr<trainingSet> t_s(new trainingSet(n_input_nodes, n_output_nodes));

            for(std::size_t i = 0; i < n_dim; i++) {
                double one_dim_rnd = uniform_real_distribution(
                    gr_l,
                    std::uniform_real_distribution<double>::param_type(-edgelength, edgelength)
                );

                // Need to find at least one dimension outside of the perimeter
                // in order to set the outside flag to true.
                if(one_dim_rnd < -edgelength / 2. || one_dim_rnd > edgelength / 2.) {
                    outside = true;
                }

                t_s->Input[i] = one_dim_rnd;
            }

            if(outside) {
                t_s->Output[0] = 0.99;
            }
            else {
                t_s->Output[0] = 0.01;
            }

            n_d->addTrainingSet(t_s, dat_counter);
        }

        // Make the initialization range known to nD_
        std::vector<std::tuple<double, double>> init_range;
        init_range.emplace_back(-edgelength, edgelength); // x
        init_range.emplace_back(-edgelength, edgelength); // y
        n_d->setInitRange(init_range);

        return n_d;
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
	  * @param n_data_sets The number of training sets to create
	  * @param radius The desired radius of the sphere
	  * @return A copy of the networkData struct that has been created, wrapped in a shared_ptr
	  */
    static std::shared_ptr<networkData> createHyperSphereNetworkData(
        const std::vector<std::size_t> &architecture,
        const std::size_t &n_data_sets,
        const double &radius
    ) {
        using namespace Gem::Hap;

        // Check the number of supplied layers
        if(architecture.size() < 2) { // We need at least an input- and an output-layer
            throw geneva_exception(
                g_error_streamer(DO_LOG, Gem::Common::timeAndPlace())
                << "In GNeuralNetworkIndividual::createHyperSphereNetworkData(): Error!"
                << '\n'
                << "Got invalid number of layers: " << architecture.size() << '\n'
            );
        }

        // Check that the output layer has exactly one node
        if(architecture.back() != 1) {
            throw geneva_exception(
                g_error_streamer(DO_LOG, Gem::Common::timeAndPlace())
                << "In GNeuralNetworkIndividual::createHyperSphereNetworkData(): Error!"
                << '\n'
                << "The output layer must have exactly one node for this training data."
                << '\n'
                << "Got " << architecture.back() << " instead." << '\n'
            );
        }

        // Create a local random number generator.
        GRandomT<RANDFLAVOURS::RANDOMPROXY> gr_l;
        std::uniform_real_distribution<double> uniform_real_distribution;

        // Retrieve the number of input- and output nodes for easier reference
        std::size_t n_input_nodes = architecture.front();
        std::size_t n_output_nodes = architecture.back();

        // The dimension of the hypersphere is identical to the number of input nodes
        std::size_t n_dim = n_input_nodes;

        // Create the actual networkData object and attach the architecture
        // Checks the architecture on the way
        std::shared_ptr<networkData> n_d(new networkData(n_data_sets));
        std::vector<std::size_t>::const_iterator it;
        std::size_t layer_counter = 0;
        for(it = architecture.begin(); it != architecture.end(); ++it, ++layer_counter) {
            if(*it == 0) {
                throw geneva_exception(
                    g_error_streamer(DO_LOG, Gem::Common::timeAndPlace())
                    << "In GNeuralNetworkIndividual::createHyperSphereNetworkData(): Error!" << '\n'
                    << "Layer " << layer_counter << "has invalid size " << *it << '\n'
                );
            }

            n_d->push_back(*it);
        }

        double local_radius = 1.;

        for(std::size_t dat_counter = 0; dat_counter < n_data_sets; dat_counter++) {
            std::shared_ptr<trainingSet> t_s(new trainingSet(n_input_nodes, n_output_nodes));

            local_radius = uniform_real_distribution(
                gr_l,
                std::uniform_real_distribution<double>::param_type(0., 3 * radius)
            );
            if(local_radius > radius) {
                t_s->Output[0] = 0.99;
            }
            else {
                t_s->Output[0] = 0.01;
            }

            //////////////////////////////////////////////////////////////////
            // Calculate random Cartesian coordinates for hyper sphere

            // Special cases
            switch(n_dim) {
            case 1:
                t_s->Input[0] = local_radius;
                break;

            case 2: {
                double phi = uniform_real_distribution(
                    gr_l,
                    std::uniform_real_distribution<double>::param_type(
                        0.,
                        2 * std::numbers::pi
                    )
                );
                t_s->Input[0] = local_radius * sin(phi); // x
                t_s->Input[1] = local_radius * cos(phi); // y

                // Make the initialization range known to nD_ . We only do this for 2D-data
                std::vector<std::tuple<double, double>> init_range;
                init_range.emplace_back(-local_radius, local_radius); // x
                init_range.emplace_back(-local_radius, local_radius); // y
                n_d->setInitRange(init_range);
            } break;

            default: // dimensions 3 ... inf
            {
                //////////////////////////////////////////////////////////////////
                // Create the required random numbers in spherical coordinates.
                // n_dim will be at least 3 here.
                // n_dim will be at least 3 here.
                std::size_t n_angles = n_dim - 1;
                std::vector<double> angle_collection(n_angles);
                for(std::size_t i = 0; i < (n_angles - 1); i++) { // Angles in range [0,Pi[
                    angle_collection[i] = uniform_real_distribution(
                        gr_l,
                        std::uniform_real_distribution<double>::param_type(
                            0.,
                            std::numbers::pi
                        )
                    );
                }
                angle_collection[n_angles - 1] = uniform_real_distribution(
                    gr_l,
                    std::uniform_real_distribution<double>::param_type(
                        0.,
                        2 * std::numbers::pi
                    )
                ); // Range of last angle is [0, 2.*Pi[

                //////////////////////////////////////////////////////////////////
                // Now we can fill the source-vector itself
                std::vector<double> cart_coord(n_dim);

                for(std::size_t i = 0; i < n_dim; i++) {
                    cart_coord[i] = local_radius; // They all have that
                }

                cart_coord[0] *= cos(angle_collection[0]); // x_1 / cart_coord[0]

                for(std::size_t i = 1; i < n_dim - 1;
                    i++) { // x_2 ... x_(n-1) / cart_coord[1] .... cart_coord[n-2]
                    for(std::size_t j = 0; j < i; j++) {
                        cart_coord[i] *= sin(angle_collection[j]);
                    }
                    cart_coord[i] *= cos(angle_collection[i]);
                }

                for(std::size_t j = 0; j < n_angles; j++) { // x_n / cart_coord[n-1]
                    cart_coord[n_dim - 1] *= sin(angle_collection[j]);
                }

                // Transfer the results
                for(std::size_t i = 0; i < n_dim; i++) {
                    t_s->Input[i] = cart_coord[i];
                }

            } break;
            }

            n_d->addTrainingSet(t_s, dat_counter);
        }

        return n_d;
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
	  * @param n_data_sets The number of training sets to create
	  * @return A copy of the networkData struct that has been created, wrapped in a shared_ptr
	  */
    static std::shared_ptr<networkData> createAxisCentricNetworkData(
        const std::vector<std::size_t> &architecture,
        const std::size_t &n_data_sets
    ) {
        using namespace Gem::Hap;

        // Check the number of supplied layers
        if(architecture.size() < 2) { // We need at least an input- and an output-layer
            throw geneva_exception(
                g_error_streamer(DO_LOG, Gem::Common::timeAndPlace())
                << "In GNeuralNetworkIndividual::createAxisCentricNetworkData(): Error!"
                << '\n'
                << "Got invalid number of layers: " << architecture.size() << '\n'
            );
        }

        // Check that the output layer has exactly one node
        if(architecture.back() != 1) {
            throw geneva_exception(
                g_error_streamer(DO_LOG, Gem::Common::timeAndPlace())
                << "In GNeuralNetworkIndividual::createAxisCentricNetworkData(): Error!"
                << '\n'
                << "The output layer must have exactly one node for this training data."
                << '\n'
                << "Got " << architecture.back() << " instead." << '\n'
            );
        }

        // Create a local random number generator.
        GRandomT<RANDFLAVOURS::RANDOMPROXY> gr_l;
        std::uniform_real_distribution<double> uniform_real_distribution;

        // Retrieve the number of input- and output nodes for easier reference
        std::size_t n_input_nodes = architecture.front();
        std::size_t n_output_nodes = architecture.back();

        // The dimension of the data set is equal to the number of input nodes
        std::size_t n_dim = n_input_nodes;

        // Create the actual networkData object and attach the architecture
        // Checks the architecture on the way
        std::shared_ptr<networkData> n_d(new networkData(n_data_sets));
        std::vector<std::size_t>::const_iterator it;
        std::size_t layer_counter = 0;
        for(it = architecture.begin(); it != architecture.end(); ++it, ++layer_counter) {
            if(*it == 0) {
                throw geneva_exception(
                    g_error_streamer(DO_LOG, Gem::Common::timeAndPlace())
                    << "In GNeuralNetworkIndividual::createAxisCentricNetworkData(): Error!" << '\n'
                    << "Layer " << layer_counter << "has invalid size " << *it << '\n'
                );
            }

            n_d->push_back(*it);
        }

        for(std::size_t data_counter = 0; data_counter < n_data_sets; data_counter++) {
            std::shared_ptr<trainingSet> t_s(new trainingSet(n_input_nodes, n_output_nodes));

            // Create even distribution across all dimensions
            if(data_counter % 2 == 0) {
                for(std::size_t dim_counter = 0; dim_counter < n_dim; dim_counter++) {
                    t_s->Input[dim_counter] = uniform_real_distribution(gr_l);
                }
                t_s->Output[0] = 0.01;
            }
            // Create entries in a half-cylindrical "cloud" around one axis. The density of
            // this cloud is decreasing with increasing distance from the axis.
            else {
                // Create a test value
                double probe_value = 0.;
                for(std::size_t dim_counter = 0; dim_counter < n_dim; dim_counter++) {
                    probe_value += exp(-5. * uniform_real_distribution(gr_l));
                }

                double function_value = 0.;
                std::vector<double> input_vector(n_dim);
                do {
                    function_value = 0.;

                    // Create the input vector
                    for(std::size_t dim_counter = 0; dim_counter < n_dim; dim_counter++) {
                        input_vector[dim_counter] = uniform_real_distribution(gr_l);
                        function_value += exp(-5 * input_vector[dim_counter]);
                    }
                    function_value = pow(function_value, 4.);
                }
                while(function_value < probe_value);

                for(std::size_t i = 0; i < n_dim; i++) {
                    t_s->Input[i] = input_vector[i];
                }
                t_s->Output[0] = 0.99;
            }

            n_d->addTrainingSet(t_s, data_counter);
        }

        // Make the initialization range known to nD_
        std::vector<std::tuple<double, double>> init_range;
        init_range.emplace_back(0, 1); // x
        init_range.emplace_back(0, 1); // y
        n_d->setInitRange(init_range);

        return n_d;
    }

    /***************************************************************************/
    /**
	  * Creates training data where one data set is evenly distributed above a sin(x) curve, the
	  * other evenly below it. This example only accepts two input nodes.
	  *
	  * @param architecture The desired architecture of the network
	  * @param n_data_sets The number of training sets to create
	  * @return A copy of the networkData struct that has been created, wrapped in a shared_ptr
	  */
    static std::shared_ptr<networkData> createSinNetworkData(
        const std::vector<std::size_t> &architecture,
        const std::size_t &n_data_sets
    ) {
        using namespace Gem::Hap;

        // Check the number of supplied layers
        if(architecture.size() < 2) { // We need at least an input- and an output-layer
            throw geneva_exception(
                g_error_streamer(DO_LOG, Gem::Common::timeAndPlace())
                << "In GNeuralNetworkIndividual::createSinNetworkData(): Error!" << '\n'
                << "Got invalid number of layers: " << architecture.size() << '\n'
            );
        }

        // Check that the output layer has exactly one node
        if(architecture.back() != 1) {
            throw geneva_exception(
                g_error_streamer(DO_LOG, Gem::Common::timeAndPlace())
                << "In GNeuralNetworkIndividual::createSinNetworkData(): Error!" << '\n'
                << "The output layer must have exactly one node for this training data."
                << '\n'
                << "Got " << architecture.back() << " instead." << '\n'
            );
        }

        // We require the input dimension to be 2
        if(architecture.front() != 2) {
            throw geneva_exception(
                g_error_streamer(DO_LOG, Gem::Common::timeAndPlace())
                << "In GNeuralNetworkIndividual::createSinNetworkData(): Error!" << '\n'
                << "The input layer must have exactly two node for this example." << '\n'
                << "Got " << architecture.front() << " instead." << '\n'
            );
        }

        // Create a local random number generator.
        GRandomT<RANDFLAVOURS::RANDOMPROXY> gr_l;
        std::uniform_real_distribution<double> uniform_real_distribution;

        // Retrieve the number of input- and output nodes for easier reference
        std::size_t n_input_nodes = architecture.front();
        std::size_t n_output_nodes = architecture.back();

        // Create the actual networkData object and attach the architecture
        // Checks the architecture on the way
        std::shared_ptr<networkData> n_d(new networkData(n_data_sets));
        std::vector<std::size_t>::const_iterator it;
        std::size_t layer_counter = 0;
        for(it = architecture.begin(); it != architecture.end(); ++it, ++layer_counter) {
            if(*it == 0) {
                throw geneva_exception(
                    g_error_streamer(DO_LOG, Gem::Common::timeAndPlace())
                    << "In GNeuralNetworkIndividual::createSinNetworkData(): Error!" << '\n'
                    << "Layer " << layer_counter << "has invalid size " << *it << '\n'
                );
            }

            n_d->push_back(*it);
        }

        for(std::size_t data_counter = 0; data_counter < n_data_sets; data_counter++) {
            std::shared_ptr<trainingSet> t_s(new trainingSet(n_input_nodes, n_output_nodes));

            // create the two test values
            t_s->Input[0] = uniform_real_distribution(
                gr_l,
                std::uniform_real_distribution<double>::param_type(-6., 6.)
            ); // x
            t_s->Input[1] = uniform_real_distribution(
                gr_l,
                std::uniform_real_distribution<double>::param_type(-6., 6.)
            ); // y

            // Check whether we are below or above the sin function and assign the output value accordingly
            if((t_s->Input)[1] > 4. * sin((t_s->Input)[0])) {
                t_s->Output[0] = 0.99;
            }
            else {
                t_s->Output[0] = 0.01;
            }

            n_d->addTrainingSet(t_s, data_counter);
        }

        // Make the initialization range known to nD_
        std::vector<std::tuple<double, double>> init_range;
        init_range.emplace_back(-6, 6); // x
        init_range.emplace_back(-6, 6); // y
        n_d->setInitRange(init_range);

        return n_d;
    }

    /***************************************************************************/
    /**
	  * Creates a data set of the desired type or throws, if that type is not available
	  *
	  * @param t The type of network data to be created
	  * @param output_file The name of the output training data file
	  * @param architecture_string The desired architecture of the network in std::string format
	  * @param n_data_sets The number of data sets to be produced
	  */
    static void createNetworkData(
        const Gem::Geneva::Individuals::trainingDataType &t,
        const std::string &output_file,
        const std::string &architecture_string,
        const std::size_t &n_data_sets
    ) {
        // Split the architecture_string as needed. I
        std::vector<std::size_t> architecture =
            Gem::Common::splitStringT<std::size_t>(architecture_string, "-");
        std::shared_ptr<networkData> n_d_ptr;

        switch(t) {
        case Gem::Geneva::Individuals::trainingDataType::HYPERCUBE:
            n_d_ptr = GNeuralNetworkIndividual::createHyperCubeNetworkData(
                architecture,
                n_data_sets,
                0.5 // edge-length
            );

            // Emit a visualization file, suitable for viewing with ROOT (see http://root.cern.ch)
            n_d_ptr->toROOT(output_file + ".C", -0.5, 0.5);

            break;

        case Gem::Geneva::Individuals::trainingDataType::HYPERSPHERE:
            n_d_ptr = GNeuralNetworkIndividual::createHyperSphereNetworkData(
                architecture,
                n_data_sets,
                0.5 // radius
            );

            // Emit a visualization file, suitable for viewing with ROOT (see http://root.cern.ch)
            n_d_ptr->toROOT(output_file + ".C", -1., 1.);

            break;

        case Gem::Geneva::Individuals::trainingDataType::AXISCENTRIC:
            n_d_ptr =
                GNeuralNetworkIndividual::createAxisCentricNetworkData(architecture, n_data_sets);

            // Emit a visualization file, suitable for viewing with ROOT (see http://root.cern.ch)
            n_d_ptr->toROOT(output_file + ".C", 0., 1.);

            break;

        case Gem::Geneva::Individuals::trainingDataType::SINUS:
            n_d_ptr = GNeuralNetworkIndividual::createSinNetworkData(architecture, n_data_sets);

            // Emit a visualization file, suitable for viewing with ROOT (see http://root.cern.ch)
            n_d_ptr->toROOT(output_file + ".C", -6., 6.);

            break;

        default: {                    // Error
            std::ostringstream error; // NOLINT(cppcoreguidelines-init-variables)
            error << "In createDataset(): Error!" << '\n'
                  << "Received invalid data type " << t << '\n';
            throw(geneva_exception(error.str()));
        } break;
        }

        // Write distribution to file
        n_d_ptr->saveToDisk(output_file);
    }

    /***************************************************************************/
    /** @brief Creates a program used  for the visualization of optimization results */
    void writeVisualizationFile(const std::string &vis_file);
    /** @brief Creates a C++ output file for the trained network */
    void writeTrainedNetwork(const std::string &header_file);

    /***************************************************************************/
    /**
     * The configuration read from the config file by GFlatIndividualFactory<GNeuralNetworkIndividual>.
     * The genome geometry itself comes from the global training-data store (see buildGenome); these are
     * the per-weight Gauss-adaptor settings plus the parameter init range and transfer function.
     */
    struct Config {
        double ad_prob = GNN_DEF_ADPROB;
        double adapt_ad_prob = GNN_DEF_ADAPTADPROB;
        double min_ad_prob = GNN_DEF_MINADPROB;
        double max_ad_prob = GNN_DEF_MAXADPROB;
        double sigma = GNN_DEF_SIGMA;
        double sigma_sigma = GNN_DEF_SIGMASIGMA;
        double min_sigma = GNN_DEF_MINSIGMA;
        double max_sigma = GNN_DEF_MAXSIGMA;
        double min_var = GNN_DEF_MINVAR;
        double max_var = GNN_DEF_MAXVAR;
        transferFunction t_f = GNN_DEF_TRANSFER;
    };

    /** @brief Registers the config-file options, binding them to the passed Config */
    static void describeConfig(Gem::Common::GParserBuilder &gpb, Config &c);
    /** @brief Builds the flat weight genome (structure only) for the geometry in the global data store */
    static gen::Genome buildGenome(const Config &c);
    /** @brief The OA-owned Gauss adaption config: every weight group gets the configured Gauss adaptor */
    static std::shared_ptr<OptimizationAlgorithms::GAdaptionConfigBase>
    buildAdaptionConfig(const gen::GFlatGenome &sample, const Config &c);
    /** @brief Per-object post-config hook: applies the (non-genome) transfer function to a produced individual */
    static void applyConfig(GNeuralNetworkIndividual &ind, const Config &c);

protected:
    /***************************************************************************/

    /***************************************************************************/
    /** @brief Loads the data of another GNeuralNetworkIndividual */
    void load_(const gen::GOptimizableEntity *cp) final;

    /** @brief Allow access to this classes compare_ function */
    friend void Gem::Common::compare_base_t<GNeuralNetworkIndividual>(
        GNeuralNetworkIndividual const &,
        GNeuralNetworkIndividual const &,
        Gem::Common::GToken &
    );

    /** @brief Searches for compliance with expectations with respect to another object of the same type */
    void compare_(
        const gen::GOptimizableEntity & // the other object
        ,
        const Gem::Common::expectation & // the expectation for this object, e.g. equality
        ,
        const double & // the limit for allowed deviations of floating point types
    ) const final;

    /** @brief The actual fitness calculation */
    double fitnessCalculation() final;

public:
    /** @brief Builds the (shared, immutable) semantic architecture for a given network geometry */
    static std::shared_ptr<const GNeuralNetworkArchitecture>
    makeArchitecture(const networkData &n_d);

private:
    /***************************************************************************/
    /** @brief Creates a deep clone of this object */
    gen::GFlatGenome *clone_() const final;

    /** @brief The transfer function */
    double transfer(const double &value) const;

    /** @brief The semantic architecture (lazily built from n_d_; not serialised -- recoverable) */
    const GNeuralNetworkArchitecture &architecture() const;

    /***************************************************************************/
    // Local variables
    transferFunction t_f_;             ///< The transfer function to be used for the training
    std::shared_ptr<networkData> n_d_; ///< Holds the training data
    mutable std::shared_ptr<const GNeuralNetworkArchitecture>
        nn_arch_; ///< Cached semantic architecture (transient; rebuilt from n_d_ on demand)
};

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/
/**
 * A factory for GNeuralNetworkIndividual objects. The bespoke factory has been replaced by the generic,
 * config-driven GFlatIndividualFactory; GNeuralNetworkIndividual supplies the static describeConfig /
 * buildGenome / buildAdaptionConfig / applyConfig hooks. The alias keeps existing call sites (ctor(path),
 * get_as<>(), getAdaptionConfig(), registerContentCreator()) compiling unchanged.
 */
using GNeuralNetworkIndividualFactory =
    Gem::Geneva::Genome::GFlatIndividualFactory<GNeuralNetworkIndividual>;

/******************************************************************************/

} /* namespace Gem::Geneva::Individuals */

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/

namespace Gem::Common {

// A global store for network configuration data
using GNNOptStore = GSingletonT<GGlobalOptionsT<std::string>>;
// Drop-in replacement for the former GNeuralNetworkOptions macro.
[[nodiscard]] inline std::shared_ptr<GNNOptStore::STYPE> neuralNetworkOptions() {
    return GNNOptStore::instance();
}

// A factory function for networkData objects, used by GSingletonT
template <>
std::shared_ptr<Gem::Geneva::Individuals::networkData> TFactory_GSingletonT();

} /* namespace Gem::Common */

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/
// For testing purposes

#ifdef GEM_TESTING

/**
 * As the Gem::Geneva::Individuals::GNeuralNetworkIndividual<Gem::Geneva::SIGMOID> has a private default constructor, we need to provide a
 * specialization of the factory function that creates GStartProjectIndividual objects
 */
template <>
inline std::shared_ptr<Gem::Geneva::Individuals::GNeuralNetworkIndividual>
TFactory_GUnitTests<Gem::Geneva::Individuals::GNeuralNetworkIndividual>() {
    return std::make_shared<Gem::Geneva::Individuals::GNeuralNetworkIndividual>(
        Gem::Geneva::Individuals::GNN_DEF_MINVAR,
        Gem::Geneva::Individuals::GNN_DEF_MAXVAR,
        Gem::Geneva::Individuals::GNN_DEF_SIGMA,
        Gem::Geneva::Individuals::GNN_DEF_SIGMASIGMA,
        Gem::Geneva::Individuals::GNN_DEF_MINSIGMA,
        Gem::Geneva::Individuals::GNN_DEF_MAXSIGMA,
        Gem::Geneva::Individuals::GNN_DEF_ADPROB,
        Gem::Geneva::Individuals::GNN_DEF_ADAPTADPROB,
        Gem::Geneva::Individuals::GNN_DEF_MINADPROB,
        Gem::Geneva::Individuals::GNN_DEF_MAXADPROB
    );
}

#endif /* GEM_TESTING */

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/

BOOST_CLASS_EXPORT_KEY(Gem::Geneva::Individuals::trainingSet)              // NOLINT
BOOST_CLASS_EXPORT_KEY(Gem::Geneva::Individuals::networkData)              // NOLINT
BOOST_CLASS_EXPORT_KEY(Gem::Geneva::Individuals::GNeuralNetworkIndividual) // NOLINT
/******************************************************************************/
