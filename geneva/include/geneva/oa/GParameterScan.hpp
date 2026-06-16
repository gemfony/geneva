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

// Standard headers go here
#include <fstream>
#include <memory>

// Boost headers go here

// Geneva headers go here
#include "common/GCommonHelperFunctionsT.hpp"
#include "common/GExceptions.hpp"
#include "common/GContainerT.hpp"
#include "common/GPlotDesigner.hpp"
#include "common/GSerializeTupleT.hpp"
#include "geneva/GOptimizationEnums.hpp"
#include "geneva/par/GParameterPropertyParser.hpp"
#include "geneva/ind/GOptimizableEntity.hpp"
#include "geneva/oa/GOptimizationAlgorithmBase.hpp"
#include "geneva/oa/GOptimizationAlgorithmT.hpp"
#include "geneva/oa/GParameterScan_PersonalityTraits.hpp"
#include "hap/GRandomT.hpp"

namespace Gem::Geneva::OptimizationAlgorithms {

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/
/** Indicates that all possible parameter values have been explored */
class GEndOfPar : public std::exception {
public:
    using std::exception::exception;
};

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/
/**
 * This function fills a given std::vector<T> with items. It needs to be re-implemented
 * in concrete specializations. This generic function is just a trap.
 */
template <typename T>
std::vector<T> fillWithData(
    [[maybe_unused]] std::size_t nSteps
    ,
    [[maybe_unused]] T lower
    ,
    [[maybe_unused]] T upper
) {
    throw geneva_exception(
        g_error_streamer(DO_LOG, Gem::Common::timeAndPlace())
        << "In generic function template <typename T> std::vector<T> fillWithData(): Error!"
        << '\n'
        << "This function should never be called directly. Use one of the specializations."
        << '\n'
    );

    // Make the compiler happy
    return std::vector<T>();
}

template <>
std::vector<bool> fillWithData<bool>(std::size_t n_steps, bool lower, bool upper);

template <>
std::vector<std::int32_t> fillWithData<std::int32_t>(
    std::size_t n_steps // will only be used for random entries
    ,
    std::int32_t lower,
    std::int32_t upper // inclusive
);

template <>
std::vector<float> fillWithData<float>(std::size_t n_steps, float lower, float upper);

template <>
std::vector<double> fillWithData<double>(std::size_t n_steps, double lower, double upper);

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/
/**
 * An interface class for parameter scan objects
 */
class GScanParInterface {
public:
    virtual ~GScanParInterface() = default;
    virtual gpar::NAMEANDIDTYPE getVarAddress() const = 0;
    virtual bool goToNextItem() = 0;
    virtual bool isAtTerminalPosition() const = 0;
    virtual bool isAtFirstPosition() const = 0;
    virtual void resetPosition() = 0;
    virtual std::string getTypeDescriptor() const = 0;
};

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/
/**
 * Basic parameter functionality
 */
template <typename T>
class GBaseScanParT // NOLINT(cppcoreguidelines-special-member-functions)
  : public Gem::Common::GPodContainerT<T>
  , public GScanParInterface {
    ///////////////////////////////////////////////////////////////////////
    friend class boost::serialization::access;

    template <typename Archive>
    void serialize(Archive &ar, const unsigned int) {
        using boost::serialization::make_nvp;

        ar &boost::serialization::make_nvp(
            "GPodContainerT_T",
            boost::serialization::base_object<Gem::Common::GPodContainerT<T>>(*this)
        ) &
            BOOST_SERIALIZATION_NVP(var_) & BOOST_SERIALIZATION_NVP(step_) &
            BOOST_SERIALIZATION_NVP(n_steps_) & BOOST_SERIALIZATION_NVP(lower_) &
            BOOST_SERIALIZATION_NVP(upper_) & BOOST_SERIALIZATION_NVP(random_scan_) &
            BOOST_SERIALIZATION_NVP(type_description_);
    }

    ///////////////////////////////////////////////////////////////////////

public:
    /***************************************************************************/
    /**
     * The standard constructor
     */
    GBaseScanParT(
        gpar::parPropSpec<T> pps,
        bool random_scan,
        const std::string &t // type_description_
    )
      : Gem::Common::GPodContainerT<T>()
      , var_(pps.var)
      , step_(0)
      , n_steps_(pps.nSteps)
      , lower_(pps.lowerBoundary)
      , upper_(pps.upperBoundary)
      , random_scan_(random_scan)
      , type_description_(t) {
        if(not random_scan_) {
            // Fill the object with data
            this->data_cnt_ = fillWithData<T>(n_steps_, lower_, upper_);
        }
    }

    /***************************************************************************/
    /**
     * Copy constructor. Not defaulted, so we can avoid copying of the
     * random number generator.
     */
    GBaseScanParT(const GBaseScanParT<T> &cp)
      : var_(cp.var_)
      , step_(cp.step_)
      , n_steps_(cp.n_steps_)
      , lower_(cp.lower_)
      , upper_(cp.upper_)
      , random_scan_(cp.random_scan_)
      , type_description_(cp.type_description_) { /* nothing */
    }

    /***************************************************************************/
    /**
     * The destructor
     */
    ~GBaseScanParT() override = default;

    /***************************************************************************/
    /**
     * Retrieve the address of this object
     */
    gpar::NAMEANDIDTYPE getVarAddress() const override {
        return var_;
    }

    /***************************************************************************/
    /**
     * Retrieves the current item position
     */
    std::size_t getCurrentItemPos() const {
        return step_;
    }

    /***************************************************************************/
    /**
     * Retrieve the current item
     */
    T getCurrentItem(Gem::Hap::GRandomBase &gr) const {
        if(random_scan_) {
            return getRandomItem(gr);
        }
                    return this->at(step_);
       
    }

    /***************************************************************************/
    /**
     * Switch to the next position in the vector or rewind
     *
     * @return A boolean indicating whether a warp has taken place
     */
    bool goToNextItem() override {
        if(++step_ >= n_steps_) {
            step_ = 0;
            return true;
        }
        return false;
    }

    /***************************************************************************/
    /**
     * Checks whether step_ points to the last item in the array
     */
    bool isAtTerminalPosition() const override {
        return step_ >= n_steps_;
    }

    /***************************************************************************/
    /**
     * Checks whether step_ points to the first item in the array
     */
    bool isAtFirstPosition() const override {
        return 0 == step_;
    }

    /***************************************************************************/
    /**
     * Resets the current position
     */
    void resetPosition() override {
        step_ = 0;
    }

    /***************************************************************************/
    /**
     * Retrieve the type descriptor
     */
    std::string getTypeDescriptor() const override {
        return type_description_;
    }

protected:
    /***************************************************************************/
    // Data

    gpar::NAMEANDIDTYPE var_;           ///< Name and/or position of the variable
    std::size_t step_;            ///< The current position in the data vector
    std::size_t n_steps_;          ///< The number of steps to be taken in a scan
    T lower_;                     ///< The lower boundary of an item
    T upper_;                     ///< The upper boundary of an item
    bool random_scan_;             ///< Indicates whether we are dealing with a random scan or not
    std::string type_description_; ///< Holds an identifier for the type described by this class

    mutable Gem::Hap::GRandom gr_; ///< Simple access to a random number generator

    /***************************************************************************/
    /** @brief The default constructor -- only needed for de-serialization, hence protected */
    GBaseScanParT()
      : var_(gpar::NAMEANDIDTYPE(0, "empty", 0))
      , step_(0)
      , n_steps_(2)
      , lower_(T(0))
      , upper_(T(1))
      , random_scan_(true) { /* nothing */
    }

    /***************************************************************************/
    /**
     * Retrieves a random item. To be re-implemented for each supported type
     */
    T getRandomItem(
        [[maybe_unused]] Gem::Hap::GRandomBase & gr
    ) const {
        // A trap. This function needs to be re-implemented for each supported type
        throw geneva_exception(
            g_error_streamer(DO_LOG, Gem::Common::timeAndPlace())
            << "In GBaseScanParT::getRandomItem(): Error!" << '\n'
            << "Function called for unsupported type" << '\n'
        );

        // Make the compiler happy
        return T(0);
    }

private:
    mutable std::bernoulli_distribution
        uniform_bool_; ///< boolean random numbers with an even distribution
    mutable std::uniform_real_distribution<float>
        uniform_float_distribution_; ///< Uniformly distributed fp numbers
    mutable std::uniform_real_distribution<double>
        uniform_double_distribution_; ///< Uniformly distributed fp numbers
    mutable std::uniform_int_distribution<std::int32_t>
        uniform_int_distribution_; ///< Uniformly distributed integer numbers
};

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/
/**
 * Retrieval of a random value for type bool
 */
template <>
inline bool GBaseScanParT<bool>::getRandomItem(Gem::Hap::GRandomBase &gr) const {
    return uniform_bool_(gr);
}

/******************************************************************************/
/**
 * Retrieval of a random value for type float
 */
template <>
inline float GBaseScanParT<float>::getRandomItem(Gem::Hap::GRandomBase &gr) const {
    return uniform_float_distribution_(
        gr,
        std::uniform_real_distribution<float>::param_type(lower_, upper_)
    );
}

/******************************************************************************/
/**
 * Retrieval of a random value for type double
 */
template <>
inline double GBaseScanParT<double>::getRandomItem(Gem::Hap::GRandomBase &gr) const {
    return uniform_double_distribution_(
        gr,
        std::uniform_real_distribution<double>::param_type(lower_, upper_)
    );
}

/******************************************************************************/
/**
 * Retrieval of a random value for type std::int32_t
 */
template <>
inline std::int32_t GBaseScanParT<std::int32_t>::getRandomItem(Gem::Hap::GRandomBase &gr) const {
    return uniform_int_distribution_(
        gr,
        std::uniform_int_distribution<std::int32_t>::param_type(lower_, upper_ + 1)
    );
}

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/
/**
 * This class holds boolean parameters
 */
class GBScanPar // NOLINT(cppcoreguidelines-special-member-functions)
  : public GBaseScanParT<bool> {
    ///////////////////////////////////////////////////////////////////////
    friend class boost::serialization::access;

    template <typename Archive>
    void serialize(Archive &ar, const unsigned int) {
        using boost::serialization::make_nvp;

        ar &boost::serialization::make_nvp(
            "baseScanParT_bool", boost::serialization::base_object<GBaseScanParT<bool>>(*this)
        );
    }

    ///////////////////////////////////////////////////////////////////////

public:
    /** @brief Construction from local variables */
    GBScanPar(gpar::parPropSpec<bool>, bool);
    /** @brief Copy constructor */
    GBScanPar(const GBScanPar &) = default;
    /** @brief The destructor */
    ~GBScanPar() override = default;

    /** @brief Cloning of this object */
    std::shared_ptr<GBScanPar> clone() const;

private:
    /** @brief The default constructor -- only needed for de-serialization, hence private */
    GBScanPar();
};

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/
/**
 * A derivative of GBaseScanParT for std::int32_t values
 */
class GInt32ScanPar // NOLINT(cppcoreguidelines-special-member-functions)
  : public GBaseScanParT<std::int32_t> {
    ///////////////////////////////////////////////////////////////////////
    friend class boost::serialization::access;

    template <typename Archive>
    void serialize(Archive &ar, const unsigned int) {
        using boost::serialization::make_nvp;

        ar &boost::serialization::make_nvp(
            "baseScanParT_int32", boost::serialization::base_object<GBaseScanParT<std::int32_t>>(*this)
        );
    }

    ///////////////////////////////////////////////////////////////////////

public:
    /** @brief The standard destructor */
    GInt32ScanPar(gpar::parPropSpec<std::int32_t>, bool);
    /** @brief Copy constructor */
    GInt32ScanPar(const GInt32ScanPar &) = default;
    /** @brief The destructor */
    ~GInt32ScanPar() override = default;

    /** @brief Cloning of this object */
    std::shared_ptr<GInt32ScanPar> clone() const;

private:
    /** @brief The default constructor -- only needed for de-serialization, hence private */
    GInt32ScanPar();
};

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/
/**
 * A derivative of fpScanParT for double values
 */
class GDScanPar // NOLINT(cppcoreguidelines-special-member-functions)
  : public GBaseScanParT<double> {
    ///////////////////////////////////////////////////////////////////////
    friend class boost::serialization::access;

    template <typename Archive>
    void serialize(Archive &ar, const unsigned int) {
        using boost::serialization::make_nvp;

        ar &boost::serialization::make_nvp(
            "baseScanParT_double", boost::serialization::base_object<GBaseScanParT<double>>(*this)
        );
    }

    ///////////////////////////////////////////////////////////////////////

public:
    /** @brief The standard destructor */
    GDScanPar(gpar::parPropSpec<double>, bool);
    /** @brief The copy constructor */
    GDScanPar(const GDScanPar &) = default;
    /** @brief The destructor */
    ~GDScanPar() override = default;

    /** @brief Cloning of this object */
    std::shared_ptr<GDScanPar> clone() const;

private:
    /** @brief The default constructor -- only needed for de-serialization, hence private */
    GDScanPar();
};

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/
/**
 * A derivative of fpScanParT for float values
 */
class GFScanPar // NOLINT(cppcoreguidelines-special-member-functions)
  : public GBaseScanParT<float> {
    ///////////////////////////////////////////////////////////////////////
    friend class boost::serialization::access;

    template <typename Archive>
    void serialize(Archive &ar, const unsigned int) {
        using boost::serialization::make_nvp;

        ar &boost::serialization::make_nvp(
            "baseScanParT_float", boost::serialization::base_object<GBaseScanParT<float>>(*this)
        );
    }

    ///////////////////////////////////////////////////////////////////////

public:
    /** @brief The standard destructor */
    GFScanPar(gpar::parPropSpec<float>, bool);
    /** @brief The copy constructor */
    GFScanPar(const GFScanPar &) = default;
    /** @brief The destructor */
    ~GFScanPar() override = default;

    /** @brief Cloning of this object */
    std::shared_ptr<GFScanPar> clone() const;

private:
    /** @brief The default constructor -- only needed for de-serialization, hence private */
    GFScanPar();
};

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/

/******************************************************************************/
// A number of typedefs that indicate the position and value of a parameter inside of an individual
using singleBPar = std::tuple<bool, std::size_t, std::string, std::size_t>;
using singleInt32Par = std::tuple<std::int32_t, std::size_t, std::string, std::size_t>;
using singleFPar = std::tuple<float, std::size_t, std::string, std::size_t>;
using singleDPar = std::tuple<double, std::size_t, std::string, std::size_t>;

/******************************************************************************/
/**
 * This struct holds the entire data to be updated inside of an individual
 */
struct parSet {
    std::vector<singleBPar> bParVec;
    std::vector<singleInt32Par> iParVec;
    std::vector<singleFPar> fParVec;
    std::vector<singleDPar> dParVec;
};

/******************************************************************************/
/** @brief A simple output operator for parSet object, mostly meant for debugging */
std::ostream &operator<<(std::ostream &os, const parSet &p_s);

/******************************************************************************/
/** @brief The default number of "best" individuals to be kept during the algorithm run */
constexpr std::size_t DEFAULTNMONITORINDS = 10;

/******************************************************************************/
/**
 * This algorithm scans a given parameter range, either in a random order,
 * or on a grid. On a grid, for each integer- or floating point-coordinate to be scanned,
 * it is given the lower and upper boundaries (both inclusive) and the number
 * of steps (including the boundaries). For boolean parameters, both true and
 * false will be tested. The algorithm only takes into consideration the first
 * individual that was registered. It will be duplicated for all possible
 * combinations, and the parameters adapted as required. The algorithm will
 * decide itself about the number of iterations, based on the number of required
 * tests and the desired population size. Please note that the amount of tests
 * required grows quickly with the number of steps and parameters and can easily
 * extend beyond the range where computation still makes sense. E.g., if you
 * plan to test but 4 values for each of 100 parameters, you'd have to evaluate
 * 4^100 individuals which, at a millisecond evaluation time per individual, would
 * require approximately 7*10^49 years to compute ... (on a side note, this is
 * the very reason why optimization algorithms are needed to search for the
 * best solution). So realistically, this algorithm can only be used for small
 * numbers of parameters and steps. In random sampling mode, the algorithm will
 * try to evenly scatter random individuals throughout the parameter space (defined
 * by those parameters intended to be modified). The optimization monitor associated
 * with this class will simply store all parameters and results in an XML file.
 */
class GParameterScan // NOLINT(cppcoreguidelines-special-member-functions)
  : public GOptimizationAlgorithmT<GParameterScan> {
    ///////////////////////////////////////////////////////////////////////
    friend class boost::serialization::access;

public:
    // Identifiers consumed by the GOptimizationAlgorithmT scaffold to generate name_(),
    // getAlgorithmName_() and getAlgorithmPersonalityType_().
    static constexpr std::string_view oa_class_name = "GParameterScan";
    static constexpr std::string_view oa_algorithm_name = "Parameter Scan";
    static constexpr std::string_view oa_personality_type = "PERSONALITY_PS";

private:

    /** @brief Single declaration of this class'es UNCONDITIONALLY-handled, plain local data members.
     *
     * Only members handled identically (plain assignment / direct compare / NVP) in
     * serialize(), load_() and compare_() live here. Excluded:
     *  - cycle_logic_halt_: a LOAD-ONLY transient (assigned in load_(), compared, but NOT
     *    serialized), kept manual in load_()/compare_() and out of serialize().
     *  - b_cnt_ / int32_cnt_ / d_cnt_ / f_cnt_: vectors of std::shared_ptr<...ScanPar>.
     *    Their element type (GBScanPar etc., via GContainerT/GPodContainerT) does NOT carry
     *    the Gemfony common interface, so they cannot use make_cloneable_container_member
     *    (which needs clone<T>()/load()/compare()); load_() deep-copies them with the
     *    scan classes' own clone(), and compare_() does not compare them at all. Hence
     *    they stay in the manual tail. */
    auto localMembers() {
        return std::make_tuple(
            Gem::Common::make_member("scan_randomly_", scan_randomly_),
            Gem::Common::make_member("n_monitor_inds_", n_monitor_inds_),
            Gem::Common::make_member("simple_scan_items_", simple_scan_items_),
            Gem::Common::make_member("scans_performed_", scans_performed_)
        );
    }
    auto localMembers() const {
        return std::make_tuple(
            Gem::Common::make_member("scan_randomly_", scan_randomly_),
            Gem::Common::make_member("n_monitor_inds_", n_monitor_inds_),
            Gem::Common::make_member("simple_scan_items_", simple_scan_items_),
            Gem::Common::make_member("scans_performed_", scans_performed_)
        );
    }

    template <typename Archive>
    void serialize(Archive &ar, const unsigned int) {
        using boost::serialization::make_nvp;

        ar &make_nvp("GOptimizationAlgorithmBase", boost::serialization::base_object<GOptimizationAlgorithmBase>(*this));
        // Unconditional plain members, derived from the single localMembers() declaration ...
        Gem::Common::serialize_members(ar, this->localMembers());
        // ... and the manual tail for the parameter-object vectors (cycle_logic_halt_ is
        // intentionally NOT serialized -- it is a load-only transient).
        ar & BOOST_SERIALIZATION_NVP(b_cnt_) & BOOST_SERIALIZATION_NVP(int32_cnt_) &
            BOOST_SERIALIZATION_NVP(d_cnt_) & BOOST_SERIALIZATION_NVP(f_cnt_);
    }

    ///////////////////////////////////////////////////////////////////////

public:
    /** @brief The default constructor */
    GParameterScan() = default;
    /** @brief A standard copy constructor */
    GParameterScan(const GParameterScan &);
    /** @brief The destructor */
    ~GParameterScan() override = default;

    /** @brief Allows to set the number of "best" individuals to be monitored over the course of the algorithm run */
    void setNMonitorInds(std::size_t);
    /** @brief Allows to retrieve  the number of "best" individuals to be monitored over the course of the algorithm run */
    std::size_t getNMonitorInds() const;

    /** @brief Fills vectors with parameter specifications */
    void setParameterSpecs(std::string);

    /** @brief Puts the class in "simple scan" mode */
    void setNSimpleScans(std::size_t);
    /** @brief Retrieves the number of simple scans (or 0, if disabled) */
    std::size_t getNSimpleScans() const;
    /** @brief Retrieves the number of scans performed so far */
    std::size_t getNScansPerformed() const;

    /** @brief Allows to specify whether the parameter space should be scanned randomly or on a grid */
    void setScanRandomly(bool);
    /** @brief Allows to check whether the parameter space should be scanned randomly or on a grid */
    bool getScanRandomly() const;

protected:
    /***************************************************************************/
    // Virtual or overridden protected functions

    /** @brief Need-all algorithm: a missing or failed evaluation cannot be tolerated, so it submits
     *  through courtier under full-success-or-fatal (matches the legacy throw-on-error). */
    Gem::Courtier::GSubmissionPolicy getSubmissionPolicy_() const override {
        return Gem::Courtier::GSubmissionPolicy::full_success_or_fatal();
    }

    /** @brief Adds local configuration options to a GParserBuilder object */
    void addConfigurationOptions_(Gem::Common::GParserBuilder &gpb) override;
    /** @brief Loads the data of another population */
    void load_(const GOptimizationAlgorithmBase *) override;

    /** @brief Allow access to this classes compare_ function */
    friend void Gem::Common::compare_base_t<GParameterScan>(
        GParameterScan const &,
        GParameterScan const &,
        Gem::Common::GToken &
    );

    /** @brief Searches for compliance with expectations with respect to another object of the same type */
    void compare_(
        const GOptimizationAlgorithmBase & // the other object
        ,
        const Gem::Common::expectation & // the expectation for this object, e.g. equality
        ,
        const double & // the limit for allowed deviations of floating point types
    ) const override;

    /** @brief Resets the settings of this population to what was configured when the optimize()-call was issued */
    void resetToOptimizationStart_() override;

    /** @brief Does some preparatory work before the optimization starts */
    void init() override;
    /** @brief Does any necessary finalization work */
    void finalize() override;

    // name_(), clone_(), getAlgorithmName_(), getAlgorithmPersonalityType_() and the GUnitTests
    // stubs are generated by the GOptimizationAlgorithmT scaffold from the oa_* identifiers above.

    /***************************************************************************/

private:
    /***************************************************************************/
    // Virtual or overridden private functions

    /** @brief The actual business logic to be performed during each iteration. Returns the best achieved fitness */
    std::tuple<double, double> cycleLogic_() override;
    /** @brief Triggers fitness calculation of a number of individuals */
    void runFitnessCalculation_() override;

    /** @brief Retrieves the number of processable items for the current iteration */
    std::size_t getNProcessableItems_() const override;

    /** @brief A custom halt criterion for the optimization, allowing to stop the loop when no items are left to be scanned */
    bool customHalt_() const override;

    /** @brief Resizes the population to the desired level and does some error checks */
    void adjustPopulation_() override;
    /** @brief Retrieve a GPersonalityTraits object belonging to this algorithm */
    std::shared_ptr<GPersonalityTraits> getPersonalityTraits_() const override;
    /** @brief Gives individuals an opportunity to update their internal structures */
    void actOnStalls_() override;

    /***************************************************************************/
    /**
     * Adds a given data point to a data vector
     */
    template <typename data_type>
    void addDataPoint(
        const std::tuple<data_type, std::size_t, std::string, std::size_t> &data_point,
        std::vector<data_type> &data_vec
    ) {
#ifdef DEBUG
        if(0 != std::get<1>(data_point)) {
            throw geneva_exception(
                g_error_streamer(DO_LOG, Gem::Common::timeAndPlace())
                << "In GParameterScan::addDataPoint(mode 0): Error!" << '\n'
                << "Function was called for invalid mode " << std::get<1>(data_point) << '\n'
            );
        }
#endif

        data_type l_data = std::get<0>(data_point);
        std::size_t l_pos = std::get<3>(data_point);

        // Check that we haven't exceeded the size of the boolean data vector
        if(l_pos >= data_vec.size()) {
            throw geneva_exception(
                g_error_streamer(DO_LOG, Gem::Common::timeAndPlace())
                << "In GParameterScan::addDataPoint(): Error!" << '\n'
                << "Got position beyond end of data vector: " << l_pos << " / " << data_vec.size()
                << '\n'
            );
        }

        data_vec.at(l_pos) = l_data;
    }

    /***************************************************************************/
    /**
     * Adds a given data point to a data map
     */
    template <typename data_type>
    void addDataPoint(
        const std::tuple<data_type, std::size_t, std::string, std::size_t> &data_point,
        std::map<std::string, std::vector<data_type>> &data_map
    ) {
        data_type l_data = std::get<0>(data_point);
        std::string l_name = std::get<2>(data_point);
        std::size_t l_pos = std::get<3>(data_point);

        (Gem::Common::getMapItem(data_map, l_name)).at(l_pos) = l_data;
    }

    /***************************************************************************/
    /** @brief Resets all parameter objects */
    void resetParameterObjects();

    /** @brief Adds new parameter sets to the population */
    void updateSelectedParameters();

    /** @brief Randomly shuffle the work items a number of times */
    void randomShuffle();

    /** @brief Retrieves the next available parameter set */
    std::shared_ptr<parSet> getParameterSet(std::size_t &);

    /** @brief Switches to the next parameter set */
    bool switchToNextParameterSet();

    /** @brief Fills all parameter objects into the all_par_vec_ vector */
    void fillAllParVec();

    /** @brief Clears the all_par_vec_ vector */
    void clearAllParVec();

    bool cycle_logic_halt_ =
        false; ///< Temporary flag used to specify that the optimization should be halted
    bool scan_randomly_ =
        true; ///< Determines whether the algorithm should scan the parameter space randomly or on a grid
    std::size_t n_monitor_inds_ =
        DEFAULTNMONITORINDS; ///< The number of best individuals of the entire run to be kept

    std::vector<std::shared_ptr<GBScanPar>> b_cnt_; ///< Holds boolean parameters to be scanned
    std::vector<std::shared_ptr<GInt32ScanPar>>
        int32_cnt_; ///< Holds 32 bit integer parameters to be scanned
    std::vector<std::shared_ptr<GDScanPar>> d_cnt_; ///< Holds double values to be scanned
    std::vector<std::shared_ptr<GFScanPar>> f_cnt_; ///< Holds float values to be scanned

    std::vector<std::shared_ptr<GScanParInterface>>
        all_par_cnt_; /// Holds pointers to all parameter objects

    std::size_t simple_scan_items_ =
        0; ///< When set to a value > 0, a random scan of the entire parameter space will be made instead of individual parameters -- set through the configuration file
    std::size_t scans_performed_ =
        0; ///< Holds the number of processed items so far while a simple scan is performed

    /***************************************************************************/
};

} /* namespace Gem::Geneva::OptimizationAlgorithms */

BOOST_CLASS_EXPORT_KEY(Gem::Geneva::OptimizationAlgorithms::GBScanPar)       // NOLINT
BOOST_CLASS_EXPORT_KEY(Gem::Geneva::OptimizationAlgorithms::GInt32ScanPar)   // NOLINT
BOOST_CLASS_EXPORT_KEY(Gem::Geneva::OptimizationAlgorithms::GDScanPar)       // NOLINT
BOOST_CLASS_EXPORT_KEY(Gem::Geneva::OptimizationAlgorithms::GFScanPar)       // NOLINT
BOOST_CLASS_EXPORT_KEY(Gem::Geneva::OptimizationAlgorithms::GParameterScan) // NOLINT

