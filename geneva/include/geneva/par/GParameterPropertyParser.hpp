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
#include <iostream>
#include <string>
#include <tuple>
#include <vector>

// Geneva headers go here
#include "common/GCommonInterfaceT.hpp"
#include "common/GErrorStreamer.hpp"
#include "common/GExceptions.hpp"
#include "common/GExpectationChecksT.hpp"
#include "common/GLogger.hpp"

namespace Gem::Geneva::Genome {

/******************************************************************************/
/** @brief Storage of variable-related properties */
using NAMEANDIDTYPE = std::tuple<std::size_t, std::string, std::size_t>;

/******************************************************************************/
/**
 * This class holds common properties of supported parameters in the context
 * of parameter scans. This is targeted at float, double, integer and boolean
 * parameter types. Note that particularly the nSteps variable can have different
 * meanings for different types. E.g., for double variables it may stand for the
 * number of steps from the lower (inclusive) to the upper (exclusive) boundary
 * OR the number of random values picked from this range, whereas for booleans
 * it may only signify the latter. Note that we have given this class the standard
 * Gemfony interface, so that we may serialize it more easily as part of some
 * other classes.
 *
 * @tparam par_type The parameter type described by this spec (e.g. float, double, std::int32_t or bool)
 */
template <typename par_type>
class parPropSpec // NOLINT(cppcoreguidelines-special-member-functions)
  : public Gem::Common::GCommonInterfaceT<parPropSpec<par_type>> {
    ///////////////////////////////////////////////////////////////////////
    friend class boost::serialization::access;

    template <typename Archive>
    void serialize(Archive &ar, const unsigned int) {
        using boost::serialization::make_nvp;
        using namespace Gem::Common;

        ar &BOOST_SERIALIZATION_NVP(var) & BOOST_SERIALIZATION_NVP(lowerBoundary) &
            BOOST_SERIALIZATION_NVP(upperBoundary) & BOOST_SERIALIZATION_NVP(nSteps);
    }
    ///////////////////////////////////////////////////////////////////////

public:
    /***************************************************************************/
    /**
	  * @brief The (trivial) default constructor. Class members are initialized in the
	  * class body.
	  */
    parPropSpec() = default;

    /***************************************************************************/
    /**
	  * @brief The copy constructor
	  *
	  * @param cp Another parPropSpec object whose contents are copied into this one
	  */
    parPropSpec(const parPropSpec<par_type> &cp) = default;

    /***************************************************************************/
    /**
	  * @brief The standard destructor
	  * */
    ~parPropSpec() override = default;

    /***************************************************************************/
    /**
	  * @brief Swaps the contents of this object with another parPropSpec
	  *
	  * @param b The other parPropSpec whose data members are exchanged with this object's
	  */
    void swap(parPropSpec<par_type> &b) noexcept {
        NAMEANDIDTYPE var_c = b.var;
        b.var = this->var;
        this->var = var_c;
        par_type lower_boundary_c = b.lowerBoundary;
        b.lowerBoundary = this->lowerBoundary;
        this->lowerBoundary = lower_boundary_c;
        par_type upper_boundary_c = b.upperBoundary;
        b.upperBoundary = this->upperBoundary;
        this->upperBoundary = upper_boundary_c;
        std::size_t n_steps_c = b.nSteps;
        b.nSteps = this->nSteps;
        this->nSteps = n_steps_c;
    }

    /***************************************************************************/
    // Data ...

    // mode: (0, ...), (VarName[0], ...) or (VarName, ...)
    // variable name
    // optional index
    NAMEANDIDTYPE var = NAMEANDIDTYPE(0, std::string(""), 0);
    par_type lowerBoundary = par_type(0); ///< The lower boundary for the parameter scan
    par_type upperBoundary = par_type(1); ///< The upper boundary for the parameter scan
    std::size_t nSteps =
        10; ///< The number of steps from the lower boundary to the upper boundary (or possibly the number of random values from this parameter range, depending on the scan mode and parameter type)

protected:
    /************************************************************************/
    /**
     * @brief The single declaration of this class'es local data members. load_() and
     * compare_() are derived from it, so the member list lives in one place.
     *
     * @return A tuple of named members (mutable references) bundling var, lowerBoundary, upperBoundary and nSteps
     */
    auto localMembers() { // NOLINT -- intentionally hides the base localMembers() (each class is its own single source; the base members are handled via the base-class serialize/load_/compare_ call)
        return std::make_tuple(
            Gem::Common::make_member("var", var),
            Gem::Common::make_member("lowerBoundary", lowerBoundary),
            Gem::Common::make_member("upperBoundary", upperBoundary),
            Gem::Common::make_member("nSteps", nSteps)
        );
    }
    /**
     * @brief The const overload of the local data member declaration (see above).
     *
     * @return A tuple of named members (const references) bundling var, lowerBoundary, upperBoundary and nSteps
     */
    auto localMembers() const { // NOLINT -- intentionally hides the base localMembers() (each class is its own single source; the base members are handled via the base-class serialize/load_/compare_ call)
        return std::make_tuple(
            Gem::Common::make_member("var", var),
            Gem::Common::make_member("lowerBoundary", lowerBoundary),
            Gem::Common::make_member("upperBoundary", upperBoundary),
            Gem::Common::make_member("nSteps", nSteps)
        );
    }

    /************************************************************************/
    /**
	  * @brief Loads the data of another object into this one
	  *
	  * @param cp A pointer to another parPropSpec<par_type> object whose data is copied into this one
	  */
    void load_(const parPropSpec<par_type> *cp) override {
        // Check that we are dealing with a parPropSpec<T> reference independent of this object and convert the pointer
        const parPropSpec<par_type> *p_load = Gem::Common::g_convert_and_compare(cp, this);

        // No parent class with loadable data

        // Load local data, derived from the single localMembers() declaration
        Gem::Common::g_load_members(localMembers(), p_load->localMembers());
    }

    /***************************************************************************/
    /** @brief Allow access to this classes compare_ function */
    friend void Gem::Common::compare_base_t<parPropSpec<par_type>>(
        parPropSpec<par_type> const &,
        parPropSpec<par_type> const &,
        Gem::Common::GToken &
    );

    /***************************************************************************/
    /**
     * @brief Checks for compliance with expectations with respect to another object
     * of the same type. This function ensures the well-formedness of the
     * compare hierarchy in derived classes.
     *
     * @param cp A constant reference to another parPropSpec object to compare against
     * @param e The expected outcome of the comparison (e.g. equality or inequality)
     * @param limit The maximum allowed deviation for floating point comparisons (unused here)
     */
    void compare_(
        const parPropSpec<par_type> &cp // the other object
        ,
        const Gem::Common::expectation &e // the expectation for this object, e.g. equality
        ,
        const double & /*limit*/ // the limit for allowed deviations of floating point types
    ) const override {
        using namespace Gem::Common;

        // Check that we are dealing with a parPropSpec reference independent of this object and convert the pointer
        const parPropSpec<par_type> *p_load = Gem::Common::g_convert_and_compare(cp, this);

        Gem::Common::GToken token("parPropSpec<T>", e);

        // Compare our parent data ...
        Gem::Common::compare_base_t<GCommonInterfaceT<parPropSpec<par_type>>>(
            *this,
            *p_load,
            token
        );

        // ... and then the local data, derived from the single localMembers() declaration
        Gem::Common::g_compare_members(localMembers(), p_load->localMembers(), token);

        // React on deviations from the expectation
        token.evaluate();
    }

    /** @brief Applies modifications to this object. This is needed for testing purposes */
    bool modify_GUnitTests_() override {
        return false;
    };
    /** @brief Performs self tests that are expected to succeed. This is needed for testing purposes */
    void specificTestsNoFailureExpected_GUnitTests_() override { /* nothing */ };
    /** @brief Performs self tests that are expected to fail. This is needed for testing purposes */
    void specificTestsFailuresExpected_GUnitTests_() override { /* nothing */ };

private:
    /***************************************************************************/
    /**
	  * @brief Emits a name for this class / object
	  *
	  * @return The string identifier of this class ("parPropSpec<T>")
	  */
    std::string name_() const override {
        return std::string{"parPropSpec<T>"};
    }

    /************************************************************************/
    /**
	  * @brief Creates a deep clone of this object
	  *
	  * @return A pointer to a newly allocated, independent copy of this object (caller takes ownership)
	  */
    parPropSpec<par_type> *clone_() const override {
        return new parPropSpec<par_type>(*this);
    }
};

/******************************************************************************/
/**
 * @brief This struct holds all information relating to "simple" parameter scans, i.e.
 * parameter scans, where all variables are varied randomly. Currently the only
 * data component is the number of items to be scanned.
 */
struct simpleScanSpec {
    std::size_t nItems;
};

/******************************************************************************/
/**
 * @brief A simple output operator, mostly for debugging purposes
 *
 * @tparam par_type The parameter type of the parPropSpec being emitted
 * @param o A reference to the output stream the object is written to
 * @param s The parPropSpec object to be emitted
 * @return A reference to the (modified) output stream o, to allow chaining
 */
template <typename par_type>
std::ostream &operator<<(std::ostream &o, const parPropSpec<par_type> &s) {
    if(0 == std::get<0>(s.var)) {
        o << "index       = " << std::get<2>(s.var) << '\n';
    }
    else if(1 == std::get<0>(s.var)) {
        o << "Address     = " << std::get<1>(s.var) << "[" << std::get<2>(s.var) << "]"
          << '\n';
    }
    else if(2 == std::get<0>(s.var)) {
        o << "Name        = " << std::get<1>(s.var) << '\n';
    }
    else {
        throw geneva_exception(
            g_error_streamer(DO_LOG, Gem::Common::timeAndPlace())
            << "In std::ostream& operator<<(std::ostream& o, const parPropSpec<par_type>& s): "
               "Error!"
            << '\n'
            << "Got invalid mode " << std::get<0>(s.var) << '\n'
        );
    }

    o << "mode          = " << std::get<0>(s.var) << '\n'
      << "lowerBoundary = " << s.lowerBoundary << '\n'
      << "upperBoundary = " << s.upperBoundary << '\n'
      << "nSteps        = " << s.nSteps << '\n';

    return o;
}

/******************************************************************************/

} /* namespace Gem::Geneva::Genome */

/******************************************************************************/

namespace Gem::Geneva::Genome {

/******************************************************************************/
/**
 * @brief This class accepts a "raw" parameter description, parses it and provides
 * functions to access individual parameter properties. This is used by parameter
 * scans to parse a string holding informations about the variables to be scanned
 * (including ranges and steps). Note that this class is meant for setup purposes
 * only and thus cannot be serialized (nor can it be copied).
 */
class GParameterPropertyParser {
public:
    /** @brief The deleted copy constructor -- this class must not be copied
     *  @param The (unused) source object */
    GParameterPropertyParser(const GParameterPropertyParser&) = delete;
    /** @brief The deleted copy-assignment operator -- this class must not be assigned
     *  @param The (unused) source object
     *  @return (deleted, never returns) */
    GParameterPropertyParser& operator=(const GParameterPropertyParser&) = delete;

    /** @brief The default constructor -- deleted, as a raw description string is mandatory */
    GParameterPropertyParser() = delete;
    /** @brief The standard constructor -- assignment of the "raw" parameter property string
     *  @param The raw parameter description string to be parsed */
    explicit GParameterPropertyParser(const std::string &);

    /** @brief Retrieves the raw parameter description
     *  @return The raw, unparsed parameter description string held by this object */
    std::string getRawParameterDescription() const;
    /** @brief Allows to check whether parsing has already taken place
     *  @return true if the raw string has already been parsed, false otherwise */
    bool isParsed() const;

    /** @brief Allows to reset the internal structures and to parse a new parameter string
     *  @param The new raw parameter description string that replaces the current one */
    void setNewParameterDescription(std::string);

    /** @brief Initiates parsing of the raw string */
    void parse();

    /** @brief Retrieve the number of "simple scan" items
     *  @return The number of items requested for simple (fully random) parameter scans */
    std::size_t getNSimpleScanItems() const;

    /***************************************************************************/
    /**
	  * @brief This function returns a set of const_iterators that allow to retrieve
	  * the information from the parsers. Note that these iterators may go out
	  * of scope, if a new parameter description is supplied to this class.
	  *
	  * The first tuple-entry allows you to access all parameter entries. When the
	  * function is called, it is set to the start of the vector. The second tuple
	  * entry is set to the vector end.
	  *
	  * The function will throw if parsing hasn't happened yet.
	  *
	  * Note that this implementation is a trap. Use one of the overloads for
	  * supported types instead.
	  *
	  * @tparam par_type The parameter type whose specification iterators are requested
	  * @return A tuple holding the begin and end const_iterators of the matching spec vector (never returns here -- always throws)
	  */
    template <typename par_type>
    std::tuple<
        typename std::vector<parPropSpec<par_type>>::const_iterator,
        typename std::vector<parPropSpec<par_type>>::const_iterator>
    getIterators() const {
        std::tuple<
            typename std::vector<parPropSpec<par_type>>::const_iterator,
            typename std::vector<parPropSpec<par_type>>::const_iterator>
            result;

        throw geneva_exception(
            g_error_streamer(DO_LOG, Gem::Common::timeAndPlace())
            << "In generic GParameterPropertyParser::getIterators<par_type>() function: Error!"
            << '\n'
            << "Function was called for an unsupported type" << '\n'
        );

        // Make the compiler happy
        return result;
    }

private:
    /***************************************************************************/

    std::string raw_; ///< Holds the "raw" parameter description
    bool parsed_;     ///< Indicates whether the raw_ string has already been parsed

    std::vector<simpleScanSpec> s_spec_vec_;      ///< Holds parameter specifications for simple scans
    std::vector<parPropSpec<double>> d_spec_vec_; ///< Holds parameter specifications for double values
    std::vector<parPropSpec<float>> f_spec_vec_;  ///< Holds parameter specifications for float values
    std::vector<parPropSpec<std::int32_t>>
        i_spec_vec_;                            ///< Holds parameter specifications for integer values
    std::vector<parPropSpec<bool>> b_spec_vec_; ///< Holds parameter specifications for boolean values
};

/******************************************************************************/
/**
 * @brief This function returns a set of const_iterators that allow to retrieve
 * the information from the parsers. Note that these iterators may go out
 * of scope, if a new parameter description is supplied to this class.
 *
 * The first tuple-entry allows you to access all parameter entries. When the
 * function is called, it is set to the start of the vector. The second tuple
 * entry is set to the vector end.
 *
 * The function will throw if parsing hasn't happened yet.
 *
 * This is the overload for double parameters.
 *
 * @return A tuple holding the begin and end const_iterators of the double spec vector
 */
template <>
inline std::tuple<
    std::vector<parPropSpec<double>>::const_iterator,
    std::vector<parPropSpec<double>>::const_iterator>
GParameterPropertyParser::getIterators<double>() const {
    // Make sure parsing has happened.
    if(not parsed_) {
        throw geneva_exception(
            g_error_streamer(DO_LOG, Gem::Common::timeAndPlace())
            << "In GParameterPropertyParser::getIterators<double>(): Error!" << '\n'
            << "Tried to retrieve iterators when parsing hasn't happened yet" << '\n'
        );
    }

    auto runner_it = d_spec_vec_.begin();
    auto end_it = d_spec_vec_.end();

    return std::tuple<
        std::vector<parPropSpec<double>>::const_iterator,
        std::vector<parPropSpec<double>>::const_iterator>{runner_it, end_it};
}

/******************************************************************************/
/**
 * @brief This function returns a set of const_iterators that allow to retrieve
 * the information from the parsers. Note that these iterators may go out
 * of scope, if a new parameter description is supplied to this class.
 *
 * The first tuple-entry allows you to access all parameter entries. When the
 * function is called, it is set to the start of the vector. The second tuple
 * entry is set to the vector end.
 *
 * The function will throw if parsing hasn't happened yet.
 *
 * This is the overload for float parameters.
 *
 * @return A tuple holding the begin and end const_iterators of the float spec vector
 */
template <>
inline std::tuple<
    std::vector<parPropSpec<float>>::const_iterator,
    std::vector<parPropSpec<float>>::const_iterator>
GParameterPropertyParser::getIterators<float>() const {
    // Make sure parsing has happened.
    if(not parsed_) {
        throw geneva_exception(
            g_error_streamer(DO_LOG, Gem::Common::timeAndPlace())
            << "In GParameterPropertyParser::getIterators<float>(): Error!" << '\n'
            << "Tried to retrieve iterators when parsing hasn't happened yet" << '\n'
        );
    }

    auto runner_it = f_spec_vec_.begin();
    auto end_it = f_spec_vec_.end();

    return std::tuple<
        std::vector<parPropSpec<float>>::const_iterator,
        std::vector<parPropSpec<float>>::const_iterator>{runner_it, end_it};
}

/******************************************************************************/
/**
 * @brief This function returns a set of const_iterators that allow to retrieve
 * the information from the parsers. Note that these iterators may go out
 * of scope, if a new parameter description is supplied to this class.
 *
 * The first tuple-entry allows you to access all parameter entries. When the
 * function is called, it is set to the start of the vector. The second tuple
 * entry is set to the vector end.
 *
 * The function will throw if parsing hasn't happened yet.
 *
 * This is the overload for std::int32_t parameters.
 *
 * @return A tuple holding the begin and end const_iterators of the std::int32_t spec vector
 */
template <>
inline std::tuple<
    std::vector<parPropSpec<std::int32_t>>::const_iterator,
    std::vector<parPropSpec<std::int32_t>>::const_iterator>
GParameterPropertyParser::getIterators<std::int32_t>() const {
    // Make sure parsing has happened.
    if(not parsed_) {
        throw geneva_exception(
            g_error_streamer(DO_LOG, Gem::Common::timeAndPlace())
            << "In GParameterPropertyParser::getIterators<std::int32_t>(): Error!" << '\n'
            << "Tried to retrieve iterators when parsing hasn't happened yet" << '\n'
        );
    }

    auto runner_it = i_spec_vec_.begin();
    auto end_it = i_spec_vec_.end();

    return std::tuple<
        std::vector<parPropSpec<std::int32_t>>::const_iterator,
        std::vector<parPropSpec<std::int32_t>>::const_iterator>{runner_it, end_it};
}

/******************************************************************************/
/**
 * @brief This function returns a set of const_iterators that allow to retrieve
 * the information from the parsers. Note that these iterators may go out
 * of scope, if a new parameter description is supplied to this class.
 *
 * The first tuple-entry allows you to access all parameter entries. When the
 * function is called, it is set to the start of the vector. The second tuple
 * entry is set to the vector end.
 *
 * The function will throw if parsing hasn't happened yet.
 *
 * This is the overload for bool parameters.
 *
 * @return A tuple holding the begin and end const_iterators of the bool spec vector
 */
template <>
inline std::tuple<
    std::vector<parPropSpec<bool>>::const_iterator,
    std::vector<parPropSpec<bool>>::const_iterator>
GParameterPropertyParser::getIterators<bool>() const {
    // Make sure parsing has happened.
    if(not parsed_) {
        throw geneva_exception(
            g_error_streamer(DO_LOG, Gem::Common::timeAndPlace())
            << "In GParameterPropertyParser::getIterators<bool>(): Error!" << '\n'
            << "Tried to retrieve iterators when parsing hasn't happened yet" << '\n'
        );
    }

    auto runner_it = b_spec_vec_.begin();
    auto end_it = b_spec_vec_.end();

    return std::tuple<
        std::vector<parPropSpec<bool>>::const_iterator,
        std::vector<parPropSpec<bool>>::const_iterator>{runner_it, end_it};
}

/******************************************************************************/

} /* namespace Gem::Geneva::Genome */
