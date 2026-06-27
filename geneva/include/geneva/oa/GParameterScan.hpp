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
#include "common/GCommonInterfaceT.hpp"
#include "common/GExceptions.hpp"
#include "common/GContainerT.hpp"
#include "dietrich/GPlotDesigner.hpp"
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
/** @brief A dependent-false helper so the generic traps below fire only when actually instantiated. */
template <typename>
inline constexpr bool always_false_scan_v = false;

/**
 * @brief Fills a std::vector<T> with the grid points for a scan dimension.
 *
 * Only the explicit specializations for bool, std::int32_t, float and double exist. The generic
 * template is a compile-time trap: instantiating it for any other type is a hard error, so an
 * unsupported scan type fails at compile time rather than at runtime.
 *
 * @tparam T The parameter type for which grid points are generated
 * @param n_steps The number of steps (grid points) to generate, including the boundaries
 * @param lower The lower boundary of the scanned range (inclusive)
 * @param upper The upper boundary of the scanned range (inclusive)
 * @return A vector holding the generated grid points
 */
template <typename T>
std::vector<T> fillWithData(
    [[maybe_unused]] std::size_t n_steps
    ,
    [[maybe_unused]] T lower
    ,
    [[maybe_unused]] T upper
) {
    static_assert(
        always_false_scan_v<T>,
        "fillWithData<T> is only available for bool, std::int32_t, float and double"
    );
    return std::vector<T>();
}

/**
 * @brief Specialization of fillWithData() for type bool (yields the values false and true).
 * @param n_steps The number of steps to generate
 * @param lower The lower boundary of the scanned range
 * @param upper The upper boundary of the scanned range
 * @return A vector holding the boolean grid points
 */
template <>
std::vector<bool> fillWithData<bool>(std::size_t n_steps, bool lower, bool upper);

/**
 * @brief Specialization of fillWithData() for type std::int32_t.
 * @param n_steps The number of steps to generate (only used for random entries)
 * @param lower The lower boundary of the scanned range (inclusive)
 * @param upper The upper boundary of the scanned range (inclusive)
 * @return A vector holding the integer grid points
 */
template <>
std::vector<std::int32_t> fillWithData<std::int32_t>(
    std::size_t n_steps // will only be used for random entries
    ,
    std::int32_t lower,
    std::int32_t upper // inclusive
);

/**
 * @brief Specialization of fillWithData() for type float.
 * @param n_steps The number of steps to generate, including the boundaries
 * @param lower The lower boundary of the scanned range (inclusive)
 * @param upper The upper boundary of the scanned range (inclusive)
 * @return A vector holding the floating point grid points
 */
template <>
std::vector<float> fillWithData<float>(std::size_t n_steps, float lower, float upper);

/**
 * @brief Specialization of fillWithData() for type double.
 * @param n_steps The number of steps to generate, including the boundaries
 * @param lower The lower boundary of the scanned range (inclusive)
 * @param upper The upper boundary of the scanned range (inclusive)
 * @return A vector holding the floating point grid points
 */
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
    /** @brief The (defaulted) virtual destructor */
    virtual ~GScanParInterface() = default;
    /** @brief Retrieves the name and/or position address of the scanned variable
     *  @return The address (name and id) of the variable inside the individual */
    virtual gen::NAMEANDIDTYPE getVarAddress() const = 0;
    /** @brief Advances to the next grid position, rewinding to the start at the end
     *  @return true if a warp (rewind to the first position) has taken place, false otherwise */
    virtual bool goToNextItem() = 0;
    /** @brief Checks whether the current position is past the last grid item
     *  @return true if the position is at (or beyond) the terminal position, false otherwise */
    virtual bool isAtTerminalPosition() const = 0;
    /** @brief Checks whether the current position is the first grid item
     *  @return true if the position points to the first item, false otherwise */
    virtual bool isAtFirstPosition() const = 0;
    /** @brief Resets the current position back to the start of the grid */
    virtual void resetPosition() = 0;
    /** @brief Retrieves a textual identifier for the scanned type
     *  @return A string descriptor of the parameter type */
    virtual std::string getTypeDescriptor() const = 0;
};

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/
/**
 * @brief Basic parameter functionality shared by all scan-parameter types.
 *
 * @tparam T The parameter type held and scanned by this object (bool, std::int32_t, float, double)
 */
template <typename T>
class GBaseScanParT // NOLINT(cppcoreguidelines-special-member-functions)
  : public Gem::Common::GPodContainerT<T>
  , public GScanParInterface
  , public Gem::Common::GCommonInterfaceT<GBaseScanParT<T>> {
    ///////////////////////////////////////////////////////////////////////
    friend class boost::serialization::access;

    /** @brief Single declaration of this class'es scan-state members, so serialize(), load_() and
     *  compareScanPar() all derive from one list. var_ (a std::tuple) is listed as its three named
     *  sub-elements: that lets it be compared (compare_t cannot stream a whole tuple). The pre-computed
     *  grid lives in the GPodContainerT base and is handled separately via base_object / operator= /
     *  the data_cnt_ comparison. */
    // The member list is written ONCE, in the static template helper below. Self is deduced as
    // GBaseScanParT or const GBaseScanParT, so each member binds with the matching const-ness and
    // make_member() deduces accordingly; the two localMembers() overloads are trivial forwarders.
    template <typename Self>
    static auto localMembers_(Self &self) {
        return std::make_tuple(
            Gem::Common::make_member("var_mode", std::get<0>(self.var_)),
            Gem::Common::make_member("var_name", std::get<1>(self.var_)),
            Gem::Common::make_member("var_pos", std::get<2>(self.var_)),
            Gem::Common::make_member("step_", self.step_),
            Gem::Common::make_member("n_steps_", self.n_steps_),
            Gem::Common::make_member("lower_", self.lower_),
            Gem::Common::make_member("upper_", self.upper_),
            Gem::Common::make_member("random_scan_", self.random_scan_),
            Gem::Common::make_member("type_description_", self.type_description_)
        );
    }
    /** @brief Serializes this object via Boost.Serialization
     *  @tparam Archive The archive type used for (de-)serialization
     *  @param ar The archive to serialize to / from
     *  @param version The class version supplied by Boost.Serialization */
    template <typename Archive>
    void serialize(Archive &ar, [[maybe_unused]] const unsigned int version) {
        using boost::serialization::make_nvp;

        // The pre-computed grid (GPodContainerT base) ...
        ar &boost::serialization::make_nvp(
            "GPodContainerT_T",
            boost::serialization::base_object<Gem::Common::GPodContainerT<T>>(*this)
        );
        // ... and the scan-state members, derived from the single localMembers_() declaration.
        Gem::Common::serialize_members(ar, localMembers_(*this));
    }

    ///////////////////////////////////////////////////////////////////////

public:
    /***************************************************************************/
    /**
     * @brief The standard constructor.
     *
     * Stores the variable address, boundaries and step count from the property
     * specification. For a (non-random) grid scan the grid points are pre-computed
     * via fillWithData<T>().
     *
     * @param pps The parameter property specification (variable address, boundaries, number of steps)
     * @param random_scan If true, items are drawn randomly; if false, a grid is pre-filled
     * @param t A textual identifier for the parameter type (stored as type_description_)
     */
    GBaseScanParT(
        gen::parPropSpec<T> pps,
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
     * @brief Copy constructor. Not defaulted, so we can avoid copying of the
     * random number generator.
     *
     * @param cp Another GBaseScanParT object whose state is copied
     */
    GBaseScanParT(const GBaseScanParT<T> &cp)
      : Gem::Common::GPodContainerT<T>(cp) // copy the pre-computed grid (the random members default-construct)
      , var_(cp.var_)
      , step_(cp.step_)
      , n_steps_(cp.n_steps_)
      , lower_(cp.lower_)
      , upper_(cp.upper_)
      , random_scan_(cp.random_scan_)
      , type_description_(cp.type_description_) { /* nothing */
    }

    /***************************************************************************/
    /**
     * @brief The (defaulted) destructor
     */
    ~GBaseScanParT() override = default;

    /***************************************************************************/
    /**
     * @brief Retrieve the address (name and id) of the scanned variable
     * @return The variable address inside the individual
     */
    gen::NAMEANDIDTYPE getVarAddress() const override {
        return var_;
    }

    /***************************************************************************/
    /**
     * @brief Retrieves the current item position in the grid
     * @return The current step index
     */
    std::size_t getCurrentItemPos() const {
        return step_;
    }

    /***************************************************************************/
    /**
     * @brief Retrieve the current item -- a grid point or, in random mode, a random draw.
     * @param gr A reference to a random number generator (used in random-scan mode)
     * @return The current parameter value
     */
    T getCurrentItem(Gem::Hap::GRandomBase &gr) const {
        if(random_scan_) {
            return getRandomItem(gr);
        }
        return this->at(step_);
    }

    /***************************************************************************/
    /**
     * @brief Compares this scan parameter against another one of the same type.
     *
     * Feeds all scan state -- the variable address, the current/total step counts, the boundaries,
     * the random-scan flag, the type descriptor and the pre-computed grid points -- to the given token,
     * so a round-trip / clone comparison detects any difference in scan state.
     *
     * @param other The other scan parameter to compare against
     * @param token The comparison token collecting the results
     */
    void compareScanPar(const GBaseScanParT<T> &other, Gem::Common::GToken &token) const {
        using namespace Gem::Common;
        // The pre-computed grid (held by the GPodContainerT base) ...
        compare_t(Gem::Common::getIdentity(this->data_cnt_, other.data_cnt_, "this->data_cnt_", "other.data_cnt_"), token);
        // ... and all the scan-state members, derived from the single localMembers_() declaration.
        g_compare_members(localMembers_(*this), localMembers_(other), token);
    }

    /***************************************************************************/
    /**
     * @brief Switch to the next position in the vector or rewind
     *
     * @return true if a warp (rewind to the start) has taken place, false otherwise
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
     * @brief Checks whether step_ points past the last item in the array
     * @return true if the position is at (or beyond) the terminal position, false otherwise
     */
    bool isAtTerminalPosition() const override {
        return step_ >= n_steps_;
    }

    /***************************************************************************/
    /**
     * @brief Checks whether step_ points to the first item in the array
     * @return true if the position points to the first item, false otherwise
     */
    bool isAtFirstPosition() const override {
        return 0 == step_;
    }

    /***************************************************************************/
    /**
     * @brief Resets the current position back to the start of the grid
     */
    void resetPosition() override {
        step_ = 0;
    }

    /***************************************************************************/
    /**
     * @brief Retrieve the type descriptor
     * @return A string identifier for the parameter type
     */
    std::string getTypeDescriptor() const override {
        return type_description_;
    }

protected:
    /***************************************************************************/
    // Gemfony common interface. Giving each scan-parameter type clone_()/load_()/compare_() lets the
    // GParameterScan scan-parameter vectors be (de)serialized, deep-copied and compared through the
    // single-source make_cloneable_container_member() route, instead of a hand-written tail.

    /**
     * @brief Loads the data of another GBaseScanParT<T> in place (deep-copies the grid + scan state).
     * @param cp The object to load from (its dynamic type matches this object's)
     */
    void load_(const GBaseScanParT<T> *cp) override {
        // GCommonInterfaceT carries no data of its own; copy the pre-computed grid held by the container
        // base ...
        Gem::Common::GPodContainerT<T>::operator=(*cp);
        // ... and the scan state, derived from the single localMembers_() declaration.
        Gem::Common::g_load_members(localMembers_(*this), localMembers_(*cp));
    }

    /**
     * @brief Searches for compliance with expectations with respect to another scan parameter.
     * @param cp The object to compare against (its dynamic type matches this object's)
     * @param e The expected outcome of the comparison
     * @param limit The maximum acceptable deviation (unused -- the scan state compares exactly)
     */
    void compare_(
        const GBaseScanParT<T> &cp,
        const Gem::Common::expectation &e,
        [[maybe_unused]] const double &limit
    ) const override {
        Gem::Common::GToken token("GBaseScanParT", e);
        this->compareScanPar(cp, token);
        token.evaluate();
    }

    /** @brief Test hook: applies modifications to this object (no-op for this plain value type). */
    bool modify_GUnitTests_() override { return false; }
    /** @brief Test hook: self tests expected to succeed (none for this plain value type). */
    void specificTestsNoFailureExpected_GUnitTests_() override { /* nothing */ }
    /** @brief Test hook: self tests expected to fail (none for this plain value type). */
    void specificTestsFailuresExpected_GUnitTests_() override { /* nothing */ }

    /***************************************************************************/
    // Data

    gen::NAMEANDIDTYPE var_;           ///< Name and/or position of the variable
    std::size_t step_;            ///< The current position in the data vector
    std::size_t n_steps_;          ///< The number of steps to be taken in a scan
    T lower_;                     ///< The lower boundary of an item
    T upper_;                     ///< The upper boundary of an item
    bool random_scan_;             ///< Indicates whether we are dealing with a random scan or not
    std::string type_description_; ///< Holds an identifier for the type described by this class

    /***************************************************************************/
    /** @brief The default constructor -- only needed for de-serialization, hence protected */
    GBaseScanParT()
      : var_(gen::NAMEANDIDTYPE(0, "empty", 0))
      , step_(0)
      , n_steps_(2)
      , lower_(T(0))
      , upper_(T(1))
      , random_scan_(true) { /* nothing */
    }

    /***************************************************************************/
    /**
     * @brief Retrieves a random item. Re-implemented for each supported type (bool, std::int32_t,
     * float, double) via explicit specialization.
     *
     * The generic version is a compile-time trap: instantiating it for any other type is a hard error.
     *
     * @param gr A reference to a random number generator
     * @return A random parameter value within the configured boundaries
     */
    T getRandomItem(
        [[maybe_unused]] Gem::Hap::GRandomBase & gr
    ) const {
        static_assert(
            always_false_scan_v<T>,
            "getRandomItem() is only available for bool, std::int32_t, float and double"
        );
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
 * @brief Retrieval of a random value for type bool
 * @param gr A reference to a random number generator
 * @return A uniformly distributed random boolean
 */
template <>
inline bool GBaseScanParT<bool>::getRandomItem(Gem::Hap::GRandomBase &gr) const {
    return uniform_bool_(gr);
}

/******************************************************************************/
/**
 * @brief Retrieval of a random value for type float
 * @param gr A reference to a random number generator
 * @return A uniformly distributed random float within [lower_, upper_)
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
 * @brief Retrieval of a random value for type double
 * @param gr A reference to a random number generator
 * @return A uniformly distributed random double within [lower_, upper_)
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
 * @brief Retrieval of a random value for type std::int32_t
 * @param gr A reference to a random number generator
 * @return A uniformly distributed random integer within [lower_, upper_] (upper is inclusive)
 */
template <>
inline std::int32_t GBaseScanParT<std::int32_t>::getRandomItem(Gem::Hap::GRandomBase &gr) const {
    return uniform_int_distribution_(
        gr,
        // std::uniform_int_distribution treats both bounds as INCLUSIVE, so the upper bound is passed
        // as-is (adding 1 would let the draw exceed the configured inclusive upper boundary).
        std::uniform_int_distribution<std::int32_t>::param_type(lower_, upper_)
    );
}

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/
/**
 * @brief A CRTP helper supplying the boilerplate shared by every concrete scan-parameter type.
 *
 * It provides construction from a property specification (injecting the derived type's single-character
 * descriptor), the de-serialization default constructor and typed cloning. Each concrete type
 * (GBScanPar, GInt32ScanPar, GDScanPar, GFScanPar) only declares its Boost export identity (a distinct
 * serialize() NVP, so the export keys keep working) and a one-line scanTypeDescriptor().
 *
 * @tparam Derived The concrete scan-parameter class (CRTP)
 * @tparam T The parameter type held and scanned (bool, std::int32_t, float, double)
 */
template <typename Derived, typename T>
class GScanParT // NOLINT(cppcoreguidelines-special-member-functions)
  : public GBaseScanParT<T> {
public:
    /** @brief Construction from a parameter property specification and a random-scan flag.
     *  @param pps The parameter property specification (variable address, boundaries, steps)
     *  @param random_scan If true, items are drawn randomly; if false, a grid is pre-filled */
    GScanParT(gen::parPropSpec<T> pps, bool random_scan)
      : GBaseScanParT<T>(pps, random_scan, Derived::scanTypeDescriptor()) { /* nothing */ }
    /** @brief Copy constructor (deep-copies the base, including the pre-computed grid) */
    GScanParT(const GScanParT &) = default;
    /** @brief The destructor */
    ~GScanParT() override = default;

private:
    /** @brief Creates a deep clone of this object (Gemfony common-interface hook)
     *  @return A raw, owning pointer to a deep clone (as the GBaseScanParT<T> root) */
    GBaseScanParT<T> *clone_() const override {
        return new Derived(static_cast<const Derived &>(*this));
    }

protected:
    /** @brief The default constructor -- only needed for de-serialization, hence protected */
    GScanParT() = default;
};

/******************************************************************************/
/**
 * @brief Holds boolean parameters to be scanned.
 */
class GBScanPar final : public GScanParT<GBScanPar, bool> {
    friend class boost::serialization::access;

    /** @brief Serializes this object via Boost.Serialization */
    template <typename Archive>
    void serialize(Archive &ar, [[maybe_unused]] const unsigned int version) {
        ar &boost::serialization::make_nvp(
            "baseScanParT_bool", boost::serialization::base_object<GBaseScanParT<bool>>(*this)
        );
    }

public:
    using GScanParT<GBScanPar, bool>::GScanParT;
    /** @brief The single-character type descriptor injected into the base */
    static constexpr const char *scanTypeDescriptor() { return "b"; }
};

/******************************************************************************/
/**
 * @brief Holds std::int32_t parameters to be scanned.
 */
class GInt32ScanPar final : public GScanParT<GInt32ScanPar, std::int32_t> {
    friend class boost::serialization::access;

    /** @brief Serializes this object via Boost.Serialization */
    template <typename Archive>
    void serialize(Archive &ar, [[maybe_unused]] const unsigned int version) {
        ar &boost::serialization::make_nvp(
            "baseScanParT_int32", boost::serialization::base_object<GBaseScanParT<std::int32_t>>(*this)
        );
    }

public:
    using GScanParT<GInt32ScanPar, std::int32_t>::GScanParT;
    /** @brief The single-character type descriptor injected into the base */
    static constexpr const char *scanTypeDescriptor() { return "i"; }
};

/******************************************************************************/
/**
 * @brief Holds double parameters to be scanned.
 */
class GDScanPar final : public GScanParT<GDScanPar, double> {
    friend class boost::serialization::access;

    /** @brief Serializes this object via Boost.Serialization */
    template <typename Archive>
    void serialize(Archive &ar, [[maybe_unused]] const unsigned int version) {
        ar &boost::serialization::make_nvp(
            "baseScanParT_double", boost::serialization::base_object<GBaseScanParT<double>>(*this)
        );
    }

public:
    using GScanParT<GDScanPar, double>::GScanParT;
    /** @brief The single-character type descriptor injected into the base */
    static constexpr const char *scanTypeDescriptor() { return "d"; }
};

/******************************************************************************/
/**
 * @brief Holds float parameters to be scanned.
 */
class GFScanPar final : public GScanParT<GFScanPar, float> {
    friend class boost::serialization::access;

    /** @brief Serializes this object via Boost.Serialization */
    template <typename Archive>
    void serialize(Archive &ar, [[maybe_unused]] const unsigned int version) {
        ar &boost::serialization::make_nvp(
            "baseScanParT_float", boost::serialization::base_object<GBaseScanParT<float>>(*this)
        );
    }

public:
    using GScanParT<GFScanPar, float>::GScanParT;
    /** @brief The single-character type descriptor injected into the base */
    static constexpr const char *scanTypeDescriptor() { return "f"; }
};

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/

/******************************************************************************/
/**
 * @brief A single scanned parameter value together with its addressing metadata.
 *
 * Replaces the former positional 4-tuple (value, mode, name, position), whose anonymous std::get<N>
 * accesses were error-prone.
 *
 * @tparam T The parameter value type (bool, std::int32_t, float, double)
 */
template <typename T>
struct singleParameter {
    T value{};            ///< The parameter value to be written into the individual
    std::size_t mode{0};  ///< The addressing mode (always 0 = positional / by-index)
    std::string name;   ///< The parameter's name (may be empty for purely positional addressing)
    std::size_t pos{0};   ///< The parameter's position within its value channel
};

// Convenience aliases for the value/position descriptor of a single scanned parameter
using singleBPar = singleParameter<bool>;
using singleInt32Par = singleParameter<std::int32_t>;
using singleFPar = singleParameter<float>;
using singleDPar = singleParameter<double>;

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
/** @brief A simple output operator for parSet object, mostly meant for debugging
 *  @param os The output stream to write to
 *  @param p_s The parameter set to be streamed
 *  @return A reference to the output stream, for chaining */
std::ostream &operator<<(std::ostream &os, const parSet &p_s);

/******************************************************************************/
/** @brief The default number of "best" individuals to be kept during the algorithm run */
constexpr std::size_t DEFAULTNMONITORINDS = 10;

/******************************************************************************/
/**
 * @brief An exhaustive / sampling parameter-space explorer (not an optimizer in the search-heuristic
 * sense): it evaluates a prescribed set of parameter combinations and reports the best one found.
 *
 * @details
 * A single registered individual defines the genome; it is duplicated and its scanned parameters are
 * overwritten with each combination to be tested. The algorithm runs in one of three modes.
 *
 * @par Grid scan
 * Each scanned coordinate \f$ j \f$ is given an (inclusive) range \f$ [l_j, u_j] \f$ and a step count
 * \f$ s_j \f$; its grid points are spread evenly over the range,
 * \f[
 *   x_{j,k} = l_j + (u_j - l_j)\,\frac{k}{s_j-1},\qquad k = 0,\dots,s_j-1
 * \f]
 * (integer coordinates round each \f$ x_{j,k} \f$ to the nearest integer; booleans always test both
 * \f$ \{\text{false}, \text{true}\} \f$, i.e. \f$ s_j=2 \f$). The Cartesian product of the per-coordinate
 * grids is enumerated odometer-style: the scan advances the first coordinate until it wraps, then carries
 * into the next, and terminates when the last coordinate wraps. It therefore evaluates exactly
 * \f[
 *   N = \prod_{j} s_j
 * \f]
 * combinations. Note that \f$ N \f$ grows @e exponentially with the number of scanned parameters: a mere
 * \f$ 4 \f$ values for each of \f$ 100 \f$ parameters is \f$ 4^{100} \approx 1.6\times10^{60} \f$
 * evaluations -- astronomically infeasible, which is the very reason search heuristics exist. Grid mode is
 * thus only practical for a handful of parameters and steps.
 *
 * @par Random scan
 * In random mode each scanned coordinate's value is instead drawn uniformly at random from its range on
 * every visit (floating point: \f$ [l_j, u_j) \f$; integer: the inclusive \f$ [l_j, u_j] \f$; boolean:
 * a fair coin). The per-coordinate step count then bounds how many random draws are taken, scattering
 * sample points throughout the chosen subspace.
 *
 * @par Simple scan
 * In simple-scan mode (setNSimpleScans(N)) the per-parameter specification is ignored: the @e whole
 * individual is randomly (re-)initialized \f$ N \f$ times, sampling the entire active parameter space.
 *
 * In every mode the work is spread across iterations of the configured population size, the best result
 * seen is retained, and the run halts automatically once all combinations / samples have been evaluated.
 * The associated optimization monitor stores all parameters and results in an XML file.
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

    /** @brief Single declaration of this class'es local data members.
     *
     * The plain members are handled identically (plain assignment / direct compare / NVP) in serialize(),
     * load_() and compare_() -- including cycle_logic_halt_, which is persisted like the rest (it is reset
     * to false by init() at the start of each optimize() anyway, so persisting it is harmless and keeps
     * the handling uniform). The scan-parameter vectors b_cnt_ / int32_cnt_ / d_cnt_ / f_cnt_ (vectors of
     * std::shared_ptr<...ScanPar>) now also live here: their element types carry the Gemfony common
     * interface (clone_()/load_()/compare_()), so make_cloneable_container_member() deep-clones them on
     * load and compares them element-by-element through each scan parameter's compare_() -- no hand-written
     * tail is needed. */
    // The member list is written ONCE, in the static template helper below; the two localMembers()
    // overloads are trivial forwarders. Self is deduced as GParameterScan or const GParameterScan, so
    // each member binds with the matching const-ness and make_member()/make_cloneable_container_member()
    // deduce accordingly.
    template <typename Self>
    static auto localMembers_(Self &self) {
        return std::make_tuple(
            Gem::Common::make_member("scan_randomly_", self.scan_randomly_),
            Gem::Common::make_member("n_monitor_inds_", self.n_monitor_inds_),
            Gem::Common::make_member("simple_scan_items_", self.simple_scan_items_),
            Gem::Common::make_member("scans_performed_", self.scans_performed_),
            Gem::Common::make_member("cycle_logic_halt_", self.cycle_logic_halt_),
            Gem::Common::make_cloneable_container_member("b_cnt_", self.b_cnt_),
            Gem::Common::make_cloneable_container_member("int32_cnt_", self.int32_cnt_),
            Gem::Common::make_cloneable_container_member("d_cnt_", self.d_cnt_),
            Gem::Common::make_cloneable_container_member("f_cnt_", self.f_cnt_)
        );
    }

    /** @brief Serializes this object via Boost.Serialization
     *  @tparam Archive The archive type used for (de-)serialization
     *  @param ar The archive to serialize to / from
     *  @param version The class version supplied by Boost.Serialization */
    template <typename Archive>
    void serialize(Archive &ar, [[maybe_unused]] const unsigned int version) {
        using boost::serialization::make_nvp;

        ar &make_nvp("GOptimizationAlgorithmBase", boost::serialization::base_object<GOptimizationAlgorithmBase>(*this));
        // All members -- the plain scalars AND the scan-parameter vectors -- are derived from the single
        // localMembers() declaration; the scan parameters now carry the Gemfony common interface, so no
        // hand-written tail is needed.
        Gem::Common::serialize_members(ar, localMembers_(*this));
    }

    ///////////////////////////////////////////////////////////////////////

public:
    /** @brief The default constructor */
    GParameterScan() = default;
    /** @brief A standard copy constructor
     *  @param cp Another GParameterScan object to be copied */
    GParameterScan(const GParameterScan &cp);
    /** @brief The destructor */
    ~GParameterScan() override = default;

    /** @brief Allows to set the number of "best" individuals to be monitored over the course of the algorithm run
     *  @param n_monitor_inds The number of best individuals of the entire run to be kept */
    void setNMonitorInds(std::size_t n_monitor_inds);
    /** @brief Allows to retrieve the number of "best" individuals to be monitored over the course of the algorithm run
     *  @return The number of best individuals being monitored */
    std::size_t getNMonitorInds() const;

    /** @brief Fills the parameter vectors from a textual parameter specification
     *  @param par_str The parameter specification string to be parsed */
    void setParameterSpecs(std::string par_str);

    /** @brief Puts the class in "simple scan" mode
     *  @param simple_scan_items The number of random samples of the whole parameter space to take (0 disables simple-scan mode) */
    void setNSimpleScans(std::size_t simple_scan_items);
    /** @brief Retrieves the number of simple scans (or 0, if disabled)
     *  @return The configured number of simple scans */
    std::size_t getNSimpleScans() const;
    /** @brief Retrieves the number of scans performed so far
     *  @return The number of scans processed so far */
    std::size_t getNScansPerformed() const;

    /** @brief Allows to specify whether the parameter space should be scanned randomly or on a grid
     *  @param scan_randomly If true the space is scanned randomly, if false on a grid */
    void setScanRandomly(bool scan_randomly);
    /** @brief Allows to check whether the parameter space should be scanned randomly or on a grid
     *  @return true if the space is scanned randomly, false if on a grid */
    bool getScanRandomly() const;

protected:
    /***************************************************************************/
    // Virtual or overridden protected functions

private:
    /** @brief Need-all algorithm: a missing or failed evaluation cannot be tolerated, so it submits
     *  through courtier under full-success-or-fatal (matches the legacy throw-on-error). */
    Gem::Courtier::GSubmissionPolicy getSubmissionPolicy_() const override {
        return Gem::Courtier::GSubmissionPolicy::full_success_or_fatal();
    }

protected:
    /** @brief Adds local configuration options to a GParserBuilder object
     *  @param gpb The parser builder to which the configuration options are added */
    void addConfigurationOptions_(Gem::Common::GParserBuilder &gpb) override;
    /** @brief Loads the data of another population
     *  @param cp A pointer to another GParameterScan object, camouflaged as a GOptimizationAlgorithmBase */
    void load_(const GOptimizationAlgorithmBase *cp) override;

    /** @brief Allow access to this classes compare_ function */
    friend void Gem::Common::compare_base_t<GParameterScan>(
        GParameterScan const &,
        GParameterScan const &,
        Gem::Common::GToken &
    );

    /** @brief Searches for compliance with expectations with respect to another object of the same type
     *  @param cp The other object to be compared against (a GParameterScan as a GOptimizationAlgorithmBase)
     *  @param e The expectation for this object, e.g. equality
     *  @param limit The limit for allowed deviations of floating point types */
    void compare_(
        const GOptimizationAlgorithmBase &cp // the other object
        ,
        const Gem::Common::expectation &e // the expectation for this object, e.g. equality
        ,
        const double &limit // the limit for allowed deviations of floating point types
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

    /** @brief The actual business logic to be performed during each iteration.
     *  @return A tuple holding the best raw and transformed fitness achieved this iteration */
    std::tuple<double, double> cycleLogic_() override;
    /** @brief Triggers fitness calculation of a number of individuals */
    void runFitnessCalculation_() override;

    /** @brief Retrieves the number of processable items for the current iteration
     *  @return The number of items that can be processed in the current iteration */
    std::size_t getNProcessableItems_() const override;

    /** @brief A custom halt criterion for the optimization, allowing to stop the loop when no items are left to be scanned
     *  @return true if the optimization should be halted, false otherwise */
    bool customHalt_() const override;

    /** @brief Resizes the population to the desired level and does some error checks */
    void adjustPopulation_() override;
    /** @brief Retrieve a GPersonalityTraits object belonging to this algorithm
     *  @return A shared_ptr to a personality-traits object for the parameter scan */
    std::shared_ptr<GPersonalityTraits> getPersonalityTraits_() const override;
    /** @brief Gives individuals an opportunity to update their internal structures */
    void actOnStalls_() override;

    /***************************************************************************/
    /**
     * @brief Writes a single scanned parameter into a value-channel vector at its encoded position.
     *
     * @tparam data_type The parameter value type held by the data point
     * @param data_point The scanned parameter (value + addressing metadata); only mode 0 is valid here
     * @param data_vec The destination vector, written at data_point.pos
     */
    template <typename data_type>
    static void addDataPoint(
        const singleParameter<data_type> &data_point,
        std::vector<data_type> &data_vec
    ) {
#ifdef DEBUG
        if(0 != data_point.mode) {
            throw geneva_exception(
                g_error_streamer(DO_LOG, Gem::Common::timeAndPlace())
                << "In GParameterScan::addDataPoint(mode 0): Error!" << '\n'
                << "Function was called for invalid mode " << data_point.mode << '\n'
            );
        }
#endif

        // Check that we haven't exceeded the size of the data vector
        if(data_point.pos >= data_vec.size()) {
            throw geneva_exception(
                g_error_streamer(DO_LOG, Gem::Common::timeAndPlace())
                << "In GParameterScan::addDataPoint(): Error!" << '\n'
                << "Got position beyond end of data vector: " << data_point.pos << " / "
                << data_vec.size() << '\n'
            );
        }

        data_vec.at(data_point.pos) = data_point.value;
    }

    /***************************************************************************/
    /** @brief Resets all parameter objects */
    void resetParameterObjects();

    /** @brief Adds new parameter sets to the population */
    void updateSelectedParameters();

    /** @brief Randomly re-initializes the work items (simple-scan mode) */
    void randomInitPopulation();

    /** @brief Retrieves the next available parameter set
     *  @param mode An out-parameter receiving the running index of the returned parameter set
     *  @return A shared_ptr to the next parameter set to be evaluated */
    std::shared_ptr<parSet> getParameterSet(std::size_t &mode);

    /** @brief Switches to the next parameter set
     *  @return true if all parameter sets have been exhausted (warp-around), false otherwise */
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

