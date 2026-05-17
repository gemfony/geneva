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

// Boost headers go here
#include <boost/fusion/adapted/boost_tuple.hpp>
#include <boost/fusion/adapted/std_tuple.hpp>
#include <boost/fusion/include/adapt_struct.hpp>
#include <boost/fusion/include/boost_tuple.hpp>
#include <boost/fusion/include/io.hpp>
#include <boost/fusion/include/tuple.hpp>
#include <boost/phoenix/core.hpp>
#include <boost/phoenix/operator.hpp>
#include <boost/phoenix/stl.hpp>
#include <boost/spirit/include/qi_action.hpp>
#include <boost/spirit/include/qi_auxiliary.hpp>
#include <boost/spirit/include/qi_char.hpp>
#include <boost/spirit/include/qi_hold.hpp>
#include <boost/spirit/include/qi_lit.hpp>
#include <boost/spirit/include/qi_nonterminal.hpp>
#include <boost/spirit/include/qi_numeric.hpp>
#include <boost/spirit/include/qi_operator.hpp>
#include <boost/spirit/include/qi_raw.hpp>
#include <boost/spirit/include/qi_string.hpp>

// Geneva headers go here
#include "common/GCommonInterfaceT.hpp"
#include "common/GErrorStreamer.hpp"
#include "common/GExceptions.hpp"
#include "common/GExpectationChecksT.hpp"
#include "common/GLogger.hpp"

namespace Gem::Geneva::Parameters {

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
	  * The (trivial) default constructor. Class members are initialized in the
	  * class body.
	  */
    parPropSpec() = default;

    /***************************************************************************/
    /**
	  * The copy constructor
	  */
    parPropSpec(const parPropSpec<par_type> &cp) = default;

    /***************************************************************************/
    /**
	  * The standard destructor
	  * */
    ~parPropSpec() override = default;

    /***************************************************************************/
    /**
	  * Swap with another parPropSpec
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
	  * Loads the data of another object
	  *
	  * cp A pointer to another parPropSpec<T> object
	  */
    void load_(const parPropSpec<par_type> *cp) override {
        // Check that we are dealing with a parPropSpec<T> reference independent of this object and convert the pointer
        const parPropSpec<par_type> *p_load = Gem::Common::g_convert_and_compare(cp, this);

        // No parent class with loadable data

        // Load local data
        var = p_load->var;
        lowerBoundary = p_load->lowerBoundary;
        upperBoundary = p_load->upperBoundary;
        nSteps = p_load->nSteps;
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
     * Checks for compliance with expectations with respect to another object
     * of type T. This purely virtual function ensures the well-formedness of the
     * compare hierarchy in derived classes.
     *
     * @param cp A constant reference to another object of the same type, camouflaged as a base object
     * @param e The expected outcome of the comparison
     * @param limit The maximum deviation for floating point values (important for similarity checks)
     */
    void compare_(
        const parPropSpec<par_type> &cp // the other object
        ,
        const Gem::Common::expectation &e // the expectation for this object, e.g. equality
        ,
        const double & /*limit*/ // the limit for allowed deviations of floating point types
    ) const override {
        using namespace Gem::Common;

        // Check that we are dealing with a GPlotDesigner reference independent of this object and convert the pointer
        const parPropSpec<par_type> *p_load = Gem::Common::g_convert_and_compare(cp, this);

        Gem::Common::GToken token("parPropSpec<T>", e);

        // Compare our parent data ...
        Gem::Common::compare_base_t<GCommonInterfaceT<parPropSpec<par_type>>>(
            *this,
            *p_load,
            token
        );

        // ... and then the local data
        Gem::Common::compare_t(IDENTITY(var, p_load->var), token);
        Gem::Common::compare_t(IDENTITY(lowerBoundary, p_load->lowerBoundary), token);
        Gem::Common::compare_t(IDENTITY(upperBoundary, p_load->upperBoundary), token);
        Gem::Common::compare_t(IDENTITY(nSteps, p_load->nSteps), token);

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
	  * Emits a name for this class / object
	  */
    std::string name_() const override {
        return std::string{"parPropSpec<T>"};
    }

    /************************************************************************/
    /**
	  * Creates a deep clone of this object
	  */
    parPropSpec<par_type> *clone_() const override {
        return new parPropSpec<par_type>(*this);
    }
};

/******************************************************************************/
/**
 * This struct holds all information relating to "simple" parameter scans, i.e.
 * parameter scans, where all variables are varied randomly. Currently the only
 * data component is the number of items to be scanned.
 */
struct simpleScanSpec {
    std::size_t nItems;
};

/******************************************************************************/
/**
 * A simple output operator, mostly for debugging purposes
 *
 * @param o A reference to the output stream
 * @param s The object to be emitted
 * @return A reference to the output stream
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
            g_error_streamer(DO_LOG, time_and_place)
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

} /* namespace Gem::Geneva::Parameters */

/******************************************************************************/
// Make our structs accessible to boost.fusion. These macros must be
// called in the global namespace

/** @brief Makes the struct boost.fusion-compatible */
BOOST_FUSION_ADAPT_STRUCT(
    Gem::Geneva::Parameters::parPropSpec<double>,
    (Gem::Geneva::Parameters::NAMEANDIDTYPE,
     var)(double, lowerBoundary)(double, upperBoundary)(std::size_t, nSteps)
)

/** @brief Makes the struct boost.fusion-compatible */
BOOST_FUSION_ADAPT_STRUCT(
    Gem::Geneva::Parameters::parPropSpec<float>,
    (Gem::Geneva::Parameters::NAMEANDIDTYPE,
     var)(float, lowerBoundary)(float, upperBoundary)(std::size_t, nSteps)
)

/** @brief Makes the struct boost.fusion-compatible */
BOOST_FUSION_ADAPT_STRUCT(
    Gem::Geneva::Parameters::parPropSpec<std::int32_t>,
    (Gem::Geneva::Parameters::NAMEANDIDTYPE,
     var)(std::int32_t, lowerBoundary)(std::int32_t, upperBoundary)(std::size_t, nSteps)
)

/** @brief Makes the struct boost.fusion-compatible */
BOOST_FUSION_ADAPT_STRUCT(
    Gem::Geneva::Parameters::parPropSpec<bool>,
    (Gem::Geneva::Parameters::NAMEANDIDTYPE, var)(bool, lowerBoundary)(bool, upperBoundary)(std::size_t, nSteps)
)

/** @brief Makes the struct boost.fusion-compatible */
BOOST_FUSION_ADAPT_STRUCT(Gem::Geneva::Parameters::simpleScanSpec, (std::size_t, nItems))

/******************************************************************************/

namespace Gem::Geneva::Parameters {

/******************************************************************************/
/**
 * This class accepts a "raw" parameter description, parses it and provides
 * functions to access individual parameter properties. This is used by parameter
 * scans to parse a string holding informations about the variables to be scanned
 * (including ranges and steps). Note that this class is meant for setup purposes
 * only and thus cannot be serialized (nor can it be copied).
 */
class GParameterPropertyParser {
public:
    GParameterPropertyParser(const GParameterPropertyParser&) = delete;
    GParameterPropertyParser& operator=(const GParameterPropertyParser&) = delete;

    /** @brief The default constructor -- intentionally undefined */
    GParameterPropertyParser() = delete;
    /** @brief The standard constructor -- assignment of the "raw" paramter property string */
    explicit GParameterPropertyParser(const std::string &);

    /** @brief Retrieves the raw parameter description */
    std::string getRawParameterDescription() const;
    /** @brief Allows to check whether parsing has already taken place */
    bool isParsed() const;

    /** @brief Allows to reset the internal structures and to parse a new parameter string */
    void setNewParameterDescription(std::string);

    /** @brief Initiates parsing of the raw string */
    void parse();

    /** @brief Retrieve the number of "simple scan" items */
    std::size_t getNSimpleScanItems() const;

    /***************************************************************************/
    /**
	  * This function returns a set of const_iterators that allow to retrieve
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
            g_error_streamer(DO_LOG, time_and_place)
            << "In generic GParameterPropertyParser::getIterators<par_type>() function: Error!"
            << '\n'
            << "Function was called for an unsupported type" << '\n'
        );

        // Make the compiler happy
        return result;
    }

private:
    /***************************************************************************/

    boost::spirit::qi::
        rule<std::string::const_iterator, std::string(), boost::spirit::ascii::space_type>
            var_spec_;
    boost::spirit::qi::rule<
        std::string::const_iterator,
        std::tuple<char, std::string>(),
        boost::spirit::ascii::space_type>
        var_string_;

    boost::spirit::qi::
        rule<std::string::const_iterator, std::string(), boost::spirit::ascii::space_type>
            identifier_;
    boost::spirit::qi::
        rule<std::string::const_iterator, NAMEANDIDTYPE(), boost::spirit::ascii::space_type>
            var_reference_;

    boost::spirit::qi::
        rule<std::string::const_iterator, simpleScanSpec(), boost::spirit::ascii::space_type>
            simple_scan_parser_;
    boost::spirit::qi::
        rule<std::string::const_iterator, parPropSpec<double>(), boost::spirit::ascii::space_type>
            double_string_parser_;
    boost::spirit::qi::
        rule<std::string::const_iterator, parPropSpec<float>(), boost::spirit::ascii::space_type>
            float_string_parser_;
    boost::spirit::qi::rule<
        std::string::const_iterator,
        parPropSpec<std::int32_t>(),
        boost::spirit::ascii::space_type>
        int_string_parser_;
    boost::spirit::qi::
        rule<std::string::const_iterator, parPropSpec<bool>(), boost::spirit::ascii::space_type>
            bool_string_parser_;

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
 * This function returns a set of const_iterators that allow to retrieve
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
 */
template <>
inline std::tuple<
    std::vector<parPropSpec<double>>::const_iterator,
    std::vector<parPropSpec<double>>::const_iterator>
GParameterPropertyParser::getIterators<double>() const {
    // Make sure parsing has happened.
    if(not parsed_) {
        throw geneva_exception(
            g_error_streamer(DO_LOG, time_and_place)
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
 * This function returns a set of const_iterators that allow to retrieve
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
 */
template <>
inline std::tuple<
    std::vector<parPropSpec<float>>::const_iterator,
    std::vector<parPropSpec<float>>::const_iterator>
GParameterPropertyParser::getIterators<float>() const {
    // Make sure parsing has happened.
    if(not parsed_) {
        throw geneva_exception(
            g_error_streamer(DO_LOG, time_and_place)
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
 * This function returns a set of const_iterators that allow to retrieve
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
 */
template <>
inline std::tuple<
    std::vector<parPropSpec<std::int32_t>>::const_iterator,
    std::vector<parPropSpec<std::int32_t>>::const_iterator>
GParameterPropertyParser::getIterators<std::int32_t>() const {
    // Make sure parsing has happened.
    if(not parsed_) {
        throw geneva_exception(
            g_error_streamer(DO_LOG, time_and_place)
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
 * This function returns a set of const_iterators that allow to retrieve
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
 */
template <>
inline std::tuple<
    std::vector<parPropSpec<bool>>::const_iterator,
    std::vector<parPropSpec<bool>>::const_iterator>
GParameterPropertyParser::getIterators<bool>() const {
    // Make sure parsing has happened.
    if(not parsed_) {
        throw geneva_exception(
            g_error_streamer(DO_LOG, time_and_place)
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

} /* namespace Gem::Geneva::Parameters */

// Needed for rules to work. Follows http://boost.2283326.n4.nabble.com/hold-multi-pass-backtracking-swap-compliant-ast-td4664679.html
namespace boost::spirit {

void
swap(Gem::Geneva::Parameters::parPropSpec<double> &, Gem::Geneva::Parameters::parPropSpec<double> &) noexcept;
void
swap(Gem::Geneva::Parameters::parPropSpec<float> &, Gem::Geneva::Parameters::parPropSpec<float> &) noexcept;
void
swap(Gem::Geneva::Parameters::parPropSpec<std::int32_t> &, Gem::Geneva::Parameters::parPropSpec<std::int32_t> &) noexcept;
void swap(Gem::Geneva::Parameters::parPropSpec<bool> &, Gem::Geneva::Parameters::parPropSpec<bool> &) noexcept;

} /* namespace boost::spirit */
