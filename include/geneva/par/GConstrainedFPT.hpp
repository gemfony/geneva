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
#include <cfloat>
#include <cmath>
#include <type_traits>

// Boost headers go here

// Geneva headers go here
#include "common/GExceptions.hpp"
#include "geneva/par/GConstrainedNumT.hpp"
#include "geneva/GObject.hpp"
#include "geneva/par/GParameterBase.hpp"
#include "hap/GRandomBase.hpp"
#include "hap/GRandomT.hpp"

#ifdef GEM_TESTING
#include <catch2/catch_test_macros.hpp>
#endif /* GEM_TESTING */

namespace Gem::Geneva::Parameters {

/******************************************************************************/
/* The GConstrainedFPT class represents a floating point type, such as a double,
 * equipped with the ability to adapt itself. The value range can have an upper and a lower
 * limit.  Adapted values will only appear inside the given range to the user. Note that
 * appropriate adaptors (see e.g the GDoubleGaussAdaptor class) need to be loaded in order
 * to benefit from the adaption capabilities.
 */
template <typename fp_type>
class GConstrainedFPT // NOLINT(cppcoreguidelines-special-member-functions)
  : public GConstrainedNumT<fp_type> {
    ///////////////////////////////////////////////////////////////////////
    friend class boost::serialization::access;

    template <typename Archive>
    void serialize(Archive &ar, const unsigned int) {
        using boost::serialization::make_nvp;

        // Save data
        ar &make_nvp(
            "GConstrainedNumT_T",
            boost::serialization::base_object<GConstrainedNumT<fp_type>>(*this)
        );
    }
    ///////////////////////////////////////////////////////////////////////

    // Make sure this class can only be instantiated if fp_type really is a floating point type
    static_assert(std::is_floating_point<fp_type>::value, "fp_type must be a floating point type");

public:
    /***************************************************************************/
    /**
	  * The default constructor.
	  */
    GConstrainedFPT() = default;

    /***************************************************************************/
    /**
	  * A constructor that initializes the value only. The boundaries will
	  * be set to the maximum and minimum values of the corresponding type.
	  *
	  * @param val The desired external value of this object
	  */
    explicit GConstrainedFPT(const fp_type &val)
      : GConstrainedNumT<fp_type>(val) { /* nothing */
    }

    /***************************************************************************/
    /**
	  * Initializes the boundaries and assigns a random value.
	  *
	  * @param lowerBoundary The lower boundary of the value range
	  * @param upperBoundary The upper boundary of the value range
	  */
    GConstrainedFPT(const fp_type &lower_boundary, const fp_type &upper_boundary)
      : GConstrainedNumT<fp_type>(
            lower_boundary,
            std::nextafter(upper_boundary, -std::numeric_limits<fp_type>::infinity())
        ) {
        Gem::Hap::GRandomT<Gem::Hap::RANDFLAVOURS::RANDOMLOCAL> gr;
        typename std::uniform_real_distribution<fp_type> uniform_real_distribution(
            lower_boundary,
            upper_boundary
        );
        GParameterT<fp_type>::setValue(uniform_real_distribution(gr));
    }

    /***************************************************************************/
    /**
	  * Initialization with value and boundaries. We need somewhat tighter
	  * constraints for the allowed value range than implemented in the
	  * parent class. Note that we take the liberty to adapt val, if it is
	  * equal to the unmodified upper boundary. Otherwise you will get an
	  * error, where what you likely really meant was to start with the
	  * upper boundary.
	  *
	  * @param val The desired value of this object
	  * @param lowerBoundary The lower boundary of the value range
	  * @param upperBoundary The upper boundary of the value range
	  */
    GConstrainedFPT(
        const fp_type &val,
        const fp_type &lower_boundary,
        const fp_type &upper_boundary
    )
      : GConstrainedNumT<fp_type>(
            lower_boundary,
            std::nextafter(upper_boundary, -std::numeric_limits<fp_type>::infinity())
        ) {
        if(val == upper_boundary) {
            GConstrainedNumT<fp_type>::setValue(
                std::nextafter(upper_boundary, -std::numeric_limits<fp_type>::infinity())
            );
        }
        else {
            GConstrainedNumT<fp_type>::setValue(val);
        }
    }

    /***************************************************************************/
    /**
	  * A standard copy constructor. Most work is done by the parent
	  * classes.
	  *
	  * @param cp Another GConstrainedNumT<fp_type> object
	  */
    GConstrainedFPT(const GConstrainedFPT<fp_type> &cp) = default;

    /***************************************************************************/
    /**
	  * The standard destructor
	  */
    ~GConstrainedFPT() override = default;

    /***************************************************************************/
    /**
	  * A standard assignment operator for T values.
	  *
	  * @param The desired new external value
	  * @return The new external value of this object
	  */
    GConstrainedNumT<fp_type> &operator=(const fp_type &val) override {
        fp_type tmp_val = val;
        if(val == std::nextafter(this->getUpperBoundary(), std::numeric_limits<fp_type>::infinity())) {
            tmp_val = std::nextafter(val, -std::numeric_limits<fp_type>::infinity());
        }

        GConstrainedNumT<fp_type>::operator=(tmp_val);

        return *this;
    }

    /* ----------------------------------------------------------------------------------
	  * Throwing tested in GConstrainedFPT<fp_type>::specificTestsFailuresExpected_GUnitTests()
	  * Tested in GConstrainedFPT<fp_type>::specificTestsNoFailuresExpected_GUnitTests()
	  * ----------------------------------------------------------------------------------
	  */

    /***************************************************************************/
    /**
	  * Allows to set the value. Has the same constraints as the parent class'es function,
	  * applies additional restrictions.  Note that we take the liberty to adapt val, if it
	  * is equal to the unmodified upper boundary. Otherwise you will get an error,
	  * where what you likely really meant was to start with the upper boundary.
	  *
	  * @param val The new fp_type value stored in this class
	  */
    void setValue(const fp_type &val) override {
        fp_type tmp_val = val;
        if(val == std::nextafter(this->getUpperBoundary(), std::numeric_limits<fp_type>::infinity())) {
            tmp_val = std::nextafter(val, -std::numeric_limits<fp_type>::infinity());
        }

        GConstrainedNumT<fp_type>::setValue(tmp_val);
    }

    /* ----------------------------------------------------------------------------------
	  * Throwing tested in GConstrainedFPT<fp_type>::specificTestsFailuresExpected_GUnitTests()
	  * Tested in GConstrainedFPT<fp_type>::specificTestsNoFailuresExpected_GUnitTests()
	  * ----------------------------------------------------------------------------------
	  */

    /***************************************************************************/
    /**
	  * Allows to set the value of this object together with its boundaries. Note
	  * that we take the liberty to adapt val, if it is equal to the unmodified upper
	  * boundary. Otherwise you will get an error, where what you likely really meant
	  * was to start with the upper boundary.
	  *
	  * @param val The desired value of this object
	  * @param lowerBoundary The lower boundary of the value range
	  * @param upperBoundary The upper boundary of the value range
	  */
    void setValue(
        const fp_type &val,
        const fp_type &lower_boundary,
        const fp_type &upper_boundary
    ) override {
        fp_type tmp_val = val;
        if(val == upper_boundary) {
            tmp_val = std::nextafter(val, -std::numeric_limits<fp_type>::infinity());
        }

        GConstrainedNumT<fp_type>::setValue(
            tmp_val,
            lower_boundary,
            std::nextafter(upper_boundary, -std::numeric_limits<fp_type>::infinity())
        );
    }

    /* ----------------------------------------------------------------------------------
	  * Throwing tested in GConstrainedFPT<fp_type>::specificTestsFailuresExpected_GUnitTests()
	  * Tested in GConstrainedFPT<fp_type>::specificTestsNoFailuresExpected_GUnitTests()
	  * ----------------------------------------------------------------------------------
	  */

    /***************************************************************************/
    /**
	  * Sets the boundaries of this object. This function differs from the parent
	  * class'es function in that it calculates an additional quantity, the closed
	  * upper boundary (upper is assumed to be an open, i.e. non-inclusive boundary).
	  *
	  * @param lower The new lower boundary for this object
	  * @param upper The new upper boundary for this object
	  */
    void setBoundaries(const fp_type &lower_boundary, const fp_type &upper_boundary) override {
        // Set the actual boundaries
        GConstrainedNumT<fp_type>::setBoundaries(
            lower_boundary,
            std::nextafter(upper_boundary, -std::numeric_limits<fp_type>::infinity())
        );
    }

    /***************************************************************************/
    /**
	  * The transfer function needed to calculate the externally visible value.
	  * Note that in GConstrainedNumT<>::value() val is shifted to the
	  * "mapping" value, so it doesn't get too large. This happens centrally,
	  * as it is also relevant for the integer case. We calculate in long double
	  * precision here in order to avoid as much as possible numeric instabilities.
	  * Likewise we use int64_t for the region.
	  *
	  * @param val The value to which the transformation should be applied
	  * @return The transformed value
	  */
    fp_type transfer(const fp_type &val) const override {
        // NaN and infinity are poison values: they bypass the range comparison
        // below (every comparison against NaN is false) and silently corrupt
        // every individual subsequently (de)serialised from this object. They
        // must be rejected in all build types, not only DEBUG.
        if(std::isnan(val) || std::isinf(val)) {
            throw geneva_exception(
                g_error_streamer(DO_LOG, time_and_place)
                << "In GConstrainedFPT::transfer(): Error" << '\n'
                << "val is " << (std::isnan(val) ? "NaN" : "infinite") << '\n'
            );
        }

        // The remaining classification (subnormal / unknown) is a diagnostic
        // only. Subnormals are finite, well-defined values that legitimately
        // occur near convergence, so rejecting them unconditionally would turn
        // working release runs into failures; keep this DEBUG-only.
#ifdef DEBUG
        switch(std::fpclassify(val)) {
        case FP_NORMAL:
        case FP_ZERO:
        case FP_INFINITE: // already rejected unconditionally above
        case FP_NAN: {    // already rejected unconditionally above
            /* nothing */
        } break;

        case FP_SUBNORMAL: {
            throw geneva_exception(
                g_error_streamer(DO_LOG, time_and_place)
                << "In GConstrainedFPT::transfer(): Error" << '\n'
                << "val is subnormal" << '\n'
            );
        } break;

        default: {
            throw geneva_exception(
                g_error_streamer(DO_LOG, time_and_place)
                << "In GConstrainedFPT::transfer(): Error" << '\n'
                << "Unknown value type" << '\n'
            );
        }
        }
#endif /* DEBUG */

        long double local_val = Gem::Common::narrow_cast<long double>(val);
        long double lower_boundary =
            Gem::Common::narrow_cast<long double>(GConstrainedNumT<fp_type>::getLowerBoundary());
        long double upper_boundary =
            Gem::Common::narrow_cast<long double>(GConstrainedNumT<fp_type>::getUpperBoundary());

        if(local_val >= lower_boundary && local_val < upper_boundary) {
            return val; // no cast needed
        }
        else {
            // Find out which region the value is in (compare figure transferFunction.pdf
            // that should have been delivered with this software). Note that Gem::Common::narrow_cast<>
            // may throw - exceptions must be caught in surrounding functions.
            std::int64_t region = 0;

#ifdef DEBUG
            long double fp_region = std::floor(
                (local_val - (lower_boundary)) /
                ((upper_boundary) - (lower_boundary))
            );

            if(std::abs(fp_region) <
               Gem::Common::narrow_cast<long double>((std::numeric_limits<std::int64_t>::max)())) {
                // We need floor here, as an integer cast rounds towards 0, which would be wrong for negative values of val
                region = Gem::Common::narrow_cast<std::int64_t>(fp_region);
            }
            else {
                throw geneva_exception(
                    g_error_streamer(DO_LOG, time_and_place)
                    << "In GConstrainedFPT::transfer(): Error" << '\n'
                    << "fp_region = " << fp_region << " is too large and cannot be" << '\n'
                    << "converted to a std::int64_t, which has a maximum value of "
                    << (std::numeric_limits<std::int64_t>::max)() << '\n'
                );
            }
#else  /* DEBUG */
            region = static_cast<std::int64_t>(std::floor(
                (local_val - static_cast<long double>(lower_boundary)) /
                (static_cast<long double>(upper_boundary) - static_cast<long double>(lower_boundary))
            ));
#endif /* DEBUG */

            // Check whether we are in an odd or an even range and calculate the
            // external value accordingly
            long double mapping = static_cast<long double>(0.);
            if(region % 2 ==
               0) { // can it be divided by 2 ? Region 0,2,... or a negative even range
                mapping = local_val - static_cast<long double>(region) * (upper_boundary - lower_boundary);
            }
            else { // Range 1,3,... or a negative odd range
                mapping =
                    -local_val + (static_cast<long double>(region - 1) * (upper_boundary - lower_boundary) +
                                  2 * upper_boundary);
            }

            // fabs(mapping) will always be <= fabs(val), so this cast should never fail (if val was a valid fp value)
            return Gem::Common::narrow_cast<fp_type>(mapping);
        }

        // Make the compiler happy
        return fp_type(0.);
    }

    /* ----------------------------------------------------------------------------------
	  * Tested in GConstrainedFPT<fp_type>::specificTestsNoFailuresExpected_GUnitTests()
	  * ----------------------------------------------------------------------------------
	  */

protected:
    /***************************************************************************/
    /**
	  * Loads the data of another GConstrainedFPT<fp_type>, camouflaged as a GObject.
	  *
	  * @param cp Another GConstrainedFPT<fp_type> object, camouflaged as a GObject
	  */
    void load_(const GObject *cp) override {
        // Check that we are dealing with a GConstrainedFPT<fp_type>  reference independent of this object and convert the pointer
        const GConstrainedFPT<fp_type> *p_load =
            Gem::Common::g_convert_and_compare<GObject, GConstrainedFPT<fp_type>>(cp, this);

        // Load our parent class'es data ...
        GConstrainedNumT<fp_type>::load_(cp);

        // no local data ...
    }

    /***************************************************************************/
    /** @brief Allow access to this classes compare_ function */
    friend void Gem::Common::compare_base_t<GConstrainedFPT<fp_type>>(
        GConstrainedFPT<fp_type> const &,
        GConstrainedFPT<fp_type> const &,
        Gem::Common::GToken &
    );

    /***************************************************************************/
    /**
     * Searches for compliance with expectations with respect to another object
     * of the same type
     *
     * @param cp A constant reference to another GObject object
     * @param e The expected outcome of the comparison
     * @param limit The maximum deviation for floating point values (important for similarity checks)
     */
    void compare_(
        const GObject &cp,
        const Gem::Common::expectation &e,
        const double & /*limit*/
    ) const override {
        using namespace Gem::Common;

        // Check that we are dealing with a GConstrainedFPT<fp_type>  reference independent of this object and convert the pointer
        const GConstrainedFPT<fp_type> *p_load =
            Gem::Common::g_convert_and_compare<GObject, GConstrainedFPT<fp_type>>(cp, this);

        GToken token("GConstrainedFPT<fp_type>", e);

        // Compare our parent data ...
        Gem::Common::compare_base_t<GConstrainedNumT<fp_type>>(*this, *p_load, token);

        // React on deviations from the expectation
        token.evaluate();
    }

    /***************************************************************************/
    /**
	  * Randomly initializes the parameter (within its limits)
	  */
    bool randomInit_(const activityMode &, Gem::Hap::GRandomBase &gr) override {
        typename std::uniform_real_distribution<fp_type> uniform_real_distribution(
            GConstrainedNumT<fp_type>::getLowerBoundary(),
            GConstrainedNumT<fp_type>::getUpperBoundary()
        );

        this->setValue(uniform_real_distribution(gr));

        return true;
    }

    /* ----------------------------------------------------------------------------------
	  * Tested in GConstrainedFPT<fp_type>::specificTestsNoFailuresExpected_GUnitTests()
	  * ----------------------------------------------------------------------------------
	  */

    /***************************************************************************/
    /**
     * Applies modifications to this object. This is needed for testing purposes
     *
     * @return A boolean which indicates whether modifications were made
     */
    bool modify_GUnitTests_() override {
#ifdef GEM_TESTING
        bool result = false;

        // Call the parent classes' functions
        if(GConstrainedNumT<fp_type>::modify_GUnitTests_()) {
            result = true;
        }

        return result;

#else /* GEM_TESTING */ // If this function is called when GEM_TESTING isn't set, throw
        Gem::Common::condnotset("GConstrainedFPT<>::modify_GUnitTests", "GEM_TESTING");
        return false;
#endif                  /* GEM_TESTING */
    }

    /***************************************************************************/
    /**
     * Performs self tests that are expected to succeed. This is needed for testing purposes
     */
    void specificTestsNoFailureExpected_GUnitTests_() override {
#ifdef GEM_TESTING
        // Some general settings
        const std::size_t n_tests = 10000;
        const fp_type test_val = fp_type(42);
        const fp_type test_val2 = fp_type(17);
        const fp_type lower_boundary = fp_type(0);
        const fp_type upper_boundary = fp_type(100);
        const fp_type lower_random_boundary = fp_type(-100000);
        const fp_type upper_random_boundary = fp_type(100000);

        // Call the parent classes' functions
        GConstrainedNumT<fp_type>::specificTestsNoFailureExpected_GUnitTests_();

        // A random generator
        Gem::Hap::GRandomT<Gem::Hap::RANDFLAVOURS::RANDOMPROXY> gr;

        //------------------------------------------------------------------------------

        { // Check that assignment of a value with operator= works both for set and unset boundaries
            std::shared_ptr<GConstrainedFPT<fp_type>> p_test =
                this->template clone<GConstrainedFPT<fp_type>>();

            // Reset the boundaries so we are free to do what we want
            CHECK_NOTHROW(p_test->resetBoundaries());

            // Assign a value with operator=
            CHECK_NOTHROW(*p_test = test_val2);

            // Check the value
            CHECK(p_test->value() == test_val2);

            // Assign boundaries and values
            CHECK_NOTHROW(p_test->setValue(test_val2, lower_boundary, upper_boundary));

            // Check the value again
            CHECK(p_test->value() == test_val2);

            // Assign a value with operator=
            CHECK_NOTHROW(*p_test = test_val);

            // Check the value again, should have changed
            CHECK(p_test->value() == test_val);
        }

        //------------------------------------------------------------------------------

        { // Check that assignment of a value with setValue(val) works both for set and unset boundaries
            std::shared_ptr<GConstrainedFPT<fp_type>> p_test =
                this->template clone<GConstrainedFPT<fp_type>>();

            // Reset the boundaries so we are free to do what we want
            CHECK_NOTHROW(p_test->resetBoundaries());

            // Assign a value
            CHECK_NOTHROW(p_test->setValue(test_val2));

            // Check the value
            CHECK(p_test->value() == test_val2);

            // Assign new boundaries
            CHECK_NOTHROW(p_test->setBoundaries(lower_boundary, upper_boundary));

            // Cross-check that boundaries are o.k.
            CHECK(p_test->getLowerBoundary() == lower_boundary);
            CHECK(
                p_test->getUpperBoundary() ==
                std::nextafter(upper_boundary, -std::numeric_limits<fp_type>::infinity())
            );

            // Check the value again
            CHECK(p_test->value() == test_val2);

            // Assign a new value
            CHECK_NOTHROW(p_test->setValue(test_val));

            // Check the value again, should have changed
            CHECK(p_test->value() == test_val);
        }

        //------------------------------------------------------------------------------

        { // Check that simultaneous assignment of a valid value and boundaries works
            std::shared_ptr<GConstrainedFPT<fp_type>> p_test =
                this->template clone<GConstrainedFPT<fp_type>>();

            // Reset the boundaries so we are free to do what we want
            CHECK_NOTHROW(p_test->resetBoundaries());

            // Assign boundaries and values
            CHECK_NOTHROW(p_test->setValue(test_val, lower_boundary, upper_boundary));

            // Cross-check that value and boundaries are o.k.
            INFO(
                "\n"
                << std::setprecision(16) << "Invalid lower boundary found:\n"
                << "getLowerBoundary() = " << p_test->getLowerBoundary() << "expected "
                << lower_boundary << "\n"
            );
            CHECK(p_test->getLowerBoundary() == lower_boundary);

            INFO(
                "\n"
                << std::setprecision(16) << "Invalid upper boundary found:\n"
                << "getUpperBoundary() = " << p_test->getUpperBoundary() << "\n"
                << "expected "
                << std::nextafter(upper_boundary, -std::numeric_limits<fp_type>::infinity()) << "\n"
                << "Difference is "
                << p_test->getUpperBoundary() -
                       std::nextafter(upper_boundary, -std::numeric_limits<fp_type>::infinity())
                << "\n"
            );
            CHECK(
                p_test->getUpperBoundary() ==
                std::nextafter(upper_boundary, -std::numeric_limits<fp_type>::infinity())
            );

            CHECK(p_test->value() == test_val);
        }

        //------------------------------------------------------------------------------

        { // Check a number of times that calls to the transfer function do not lie outside of the allowed boundaries
            std::shared_ptr<GConstrainedFPT<fp_type>> p_test =
                this->template clone<GConstrainedFPT<fp_type>>();

            fp_type result = 0.;
            for(fp_type offset = fp_type(-100); offset < fp_type(100); offset += fp_type(10)) {
                fp_type tmp_lower_boundary = lower_boundary + offset;
                fp_type tmp_upper_boundary = upper_boundary + offset;

                // Assign valid boundaries and value
                CHECK_NOTHROW(
                    p_test->setValue(tmp_lower_boundary, tmp_lower_boundary, tmp_upper_boundary)
                );

                typename std::uniform_real_distribution<fp_type> uniform_real_distribution(
                    lower_random_boundary,
                    upper_random_boundary
                );
                for(std::size_t i = 0; i < n_tests; i++) {
                    fp_type random_value = uniform_real_distribution(gr);

                    CHECK_NOTHROW(result = p_test->transfer(random_value));
                    INFO(
                        "\n"
                        << std::setprecision(6) << "random_value = " << random_value << "\n"
                        << "after transfer = " << result << "\n"
                        << "lowerBoundary = " << tmp_lower_boundary << "\n"
                        << "upperBoundary = " << tmp_upper_boundary << "\n"
                    );
                    CHECK((result >= tmp_lower_boundary && result < tmp_upper_boundary));
                }
            }
        }

        //------------------------------------------------------------------------------

        { // Test initialization with a single "fixed" value (chosen randomly in a given range)
            std::shared_ptr<GConstrainedFPT<fp_type>> p_test =
                this->template clone<GConstrainedFPT<fp_type>>();

            // Assign a valid value and boundaries
            CHECK_NOTHROW(p_test->setValue(test_val, lower_boundary, upper_boundary));

            typename std::uniform_real_distribution<fp_type> uniform_real_distribution(
                lower_random_boundary,
                upper_random_boundary
            );
            for(std::size_t i = 0; i < n_tests; i++) {
                fp_type random_value = uniform_real_distribution(gr);

                // Randomly initialize with a "fixed" value
                CHECK_NOTHROW(p_test->GParameterBase::template fixedValueInit<fp_type>(
                    random_value,
                    activityMode::ALLPARAMETERS
                ));

                // Check that the external value is inside of the allowed value range
                // Check that the value is still in the allowed range
                INFO(
                    "\n"
                    << std::setprecision(10) << "p_test->value() = " << p_test->value() << "\n"
                    << "lowerBoundary = " << lower_boundary << "\n"
                    << "upperBoundary = " << upper_boundary << "\n"
                );
                CHECK((p_test->value() >= lower_boundary && p_test->value() < upper_boundary));
            }
        }

        //------------------------------------------------------------------------------

        { // Test multiplication with a single floating point value that won't make the internal value leave the boundaries
            std::shared_ptr<GConstrainedFPT<fp_type>> p_test =
                this->template clone<GConstrainedFPT<fp_type>>();

            // Reset the boundaries so we are free to do what we want
            CHECK_NOTHROW(p_test->resetBoundaries());

            // Assign a value
            CHECK_NOTHROW(p_test->setValue(fp_type(1), fp_type(0), fp_type(100)));

            for(std::size_t i = 1; i < 99; i++) {
                // Multiply by the counter variable
                CHECK_NOTHROW(p_test->GParameterBase::template multiplyBy<fp_type>(
                    fp_type(i),
                    activityMode::ALLPARAMETERS
                ));

                // Check that the external value is in the expected range
                INFO(
                    "\n"
                    << std::setprecision(10) << "p_test->value() = " << p_test->value() << "\n"
                    << "fp_type(i) = " << fp_type(i) << "\n"
                    << "pow(10, -8) = " << pow(10, -8) << "\n"
                );
                CHECK(
                    fabs(p_test->value() - fp_type(i)) < pow(10, -8)
                ); // This also means that the value has changed from its start value

                // Check that the internal value is in the expected range
                INFO(
                    "\n"
                    << std::setprecision(10)
                    << "p_test->getInternalValue() = " << p_test->getInternalValue() << "\n"
                    << "fp_type(i) = " << fp_type(i) << "\n"
                    << "pow(10, -8) = " << pow(10, -8) << "\n"
                );
                CHECK(fabs(p_test->getInternalValue() - fp_type(i)) < pow(10, -8));

                // Reset the value
                CHECK_NOTHROW(p_test->setValue(fp_type(1)));
            }
        }

        //------------------------------------------------------------------------------

        { // Test multiplication with a single floating point value that will make the internal value leave its boundaries
            std::shared_ptr<GConstrainedFPT<fp_type>> p_test =
                this->template clone<GConstrainedFPT<fp_type>>();

            // Assign boundaries and values
            CHECK_NOTHROW(p_test->setValue(fp_type(1), lower_boundary, upper_boundary));

            typename std::uniform_real_distribution<fp_type> uniform_real_distribution(
                lower_random_boundary,
                upper_random_boundary
            );
            for(std::size_t i = 0; i < n_tests; i++) {
                // Multiply with a random value in a very wide
                CHECK_NOTHROW(p_test->GParameterBase::template multiplyBy<fp_type>(
                    uniform_real_distribution(gr),
                    activityMode::ALLPARAMETERS
                ));

                // Check that the value is still in the allowed range
                INFO(
                    "\n"
                    << std::setprecision(10) << "p_test->value() = " << p_test->value() << "\n"
                    << "lowerBoundary = " << lower_boundary << "\n"
                    << "upperBoundary = " << upper_boundary << "\n"
                );
                CHECK((p_test->value() >= lower_boundary && p_test->value() < upper_boundary));

                // Reset the value
                CHECK_NOTHROW(p_test->setValue(fp_type(1)));
            }
        }

        //------------------------------------------------------------------------------

        { // Check multiplication with a random number in a wide range that might make the internal value leave its boundaries
            std::shared_ptr<GConstrainedFPT<fp_type>> p_test =
                this->template clone<GConstrainedFPT<fp_type>>();

            // Assign boundaries and values
            CHECK_NOTHROW(p_test->setValue(fp_type(1), lower_boundary, upper_boundary));

            for(std::size_t i = 0; i < n_tests; i++) {
                // Multiply with a random value in a very wide
                CHECK_NOTHROW(p_test->GParameterBase::template multiplyByRandom<fp_type>(
                    lower_random_boundary,
                    upper_random_boundary,
                    activityMode::ALLPARAMETERS,
                    gr
                ));

                // Check that the value is still in the allowed range
                INFO(
                    "\n"
                    << std::setprecision(10) << "p_test->value() = " << p_test->value() << "\n"
                    << "lowerBoundary = " << lower_boundary << "\n"
                    << "upperBoundary = " << upper_boundary << "\n"
                );
                CHECK((p_test->value() >= lower_boundary && p_test->value() < upper_boundary));

                // Reset the value
                CHECK_NOTHROW(p_test->setValue(fp_type(1)));
            }
        }

        //------------------------------------------------------------------------------

        { // Check multiplication with a random number in the range [0:1[. As the value used for the
            // basis of this multiplication is the lower boundary, multiplication will bring the internal
            // value outside of the external boundaries
            std::shared_ptr<GConstrainedFPT<fp_type>> p_test =
                this->template clone<GConstrainedFPT<fp_type>>();

            // Assign boundaries and values
            CHECK_NOTHROW(p_test->setValue(lower_boundary, lower_boundary, upper_boundary));

            for(std::size_t i = 0; i < n_tests; i++) {
                // Multiply with a random value in a very wide
                CHECK_NOTHROW(p_test->GParameterBase::template multiplyByRandom<fp_type>(
                    activityMode::ALLPARAMETERS,
                    gr
                ));

                // Check that the value is still in the allowed range
                INFO(
                    "\n"
                    << std::setprecision(10) << "p_test->value() = " << p_test->value() << "\n"
                    << "lowerBoundary = " << lower_boundary << "\n"
                    << "upperBoundary = " << upper_boundary << "\n"
                );
                CHECK((p_test->value() >= lower_boundary && p_test->value() < upper_boundary));

                // Reset the value
                CHECK_NOTHROW(p_test->setValue(lower_boundary));
            }
        }

        //------------------------------------------------------------------------------

        { // Test adding of objects with fpAdd. We try to stay inside of the value range
            const fp_type lower = fp_type(-10000.), upper = fp_type(10000.);

            std::shared_ptr<GConstrainedFPT<fp_type>> p_test1 =
                this->template clone<GConstrainedFPT<fp_type>>();
            std::shared_ptr<GConstrainedFPT<fp_type>> p_test2 =
                this->template clone<GConstrainedFPT<fp_type>>();

            // Reset the boundaries so we are free to do what we want
            CHECK_NOTHROW(p_test1->resetBoundaries());

            // Assign a value and boundaries
            CHECK_NOTHROW(p_test1->setValue(lower, lower, upper));

            // Load p_test1 into p_test2
            CHECK_NOTHROW(p_test2->load(p_test1));

            // Assign a value of 1 to p_test2
            CHECK_NOTHROW(p_test2->GParameterBase::template fixedValueInit<fp_type>(
                fp_type(1.),
                activityMode::ALLPARAMETERS
            ));

            fp_type current_val = fp_type(-10000.);
            for(std::int32_t i = -9999; i < 9999; i++) {
                CHECK_NOTHROW(p_test1->GParameterBase::template add<fp_type>(
                    p_test2,
                    activityMode::ALLPARAMETERS
                ));
                current_val += fp_type(1.);
                CHECK(p_test1->value() == current_val);
            }
        }

        //------------------------------------------------------------------------------

        { // Test subtraction of objects with fpSubtract. We try to stay inside of the value range
            const fp_type lower = fp_type(-10000.), upper = fp_type(10000.);

            std::shared_ptr<GConstrainedFPT<fp_type>> p_test1 =
                this->template clone<GConstrainedFPT<fp_type>>();
            std::shared_ptr<GConstrainedFPT<fp_type>> p_test2 =
                this->template clone<GConstrainedFPT<fp_type>>();

            // Reset the boundaries so we are free to do what we want
            CHECK_NOTHROW(p_test1->resetBoundaries());

            // Assign a value and boundaries
            CHECK_NOTHROW(p_test1->setValue(upper - fp_type(1.), lower, upper));

            // Load p_test1 into p_test2
            CHECK_NOTHROW(p_test2->load(p_test1));

            // Assign a value of 1 to p_test2
            CHECK_NOTHROW(p_test2->GParameterBase::template fixedValueInit<fp_type>(
                fp_type(1.),
                activityMode::ALLPARAMETERS
            ));

            fp_type current_val = fp_type(upper - fp_type(1));
            for(std::int32_t i = 9999; i >= -9998; i--) {
                CHECK_NOTHROW(p_test1->GParameterBase::template subtract<fp_type>(
                    p_test2,
                    activityMode::ALLPARAMETERS
                ));
                current_val -= fp_type(1.);
                CHECK(p_test1->value() == current_val);
            }
        }

        //------------------------------------------------------------------------------

        { // Test random initialization, as well as adding and subtraction of random values, which may leave the value range
            const fp_type lower = fp_type(-10000.), upper = fp_type(10000.);

            std::shared_ptr<GConstrainedFPT<fp_type>> p_test1 =
                this->template clone<GConstrainedFPT<fp_type>>();
            std::shared_ptr<GConstrainedFPT<fp_type>> p_test2 =
                this->template clone<GConstrainedFPT<fp_type>>();

            // Assign a value and boundaries
            CHECK_NOTHROW(p_test1->setValue(fp_type(0.), lower, upper));
            CHECK_NOTHROW(p_test2->setValue(fp_type(0.), lower, upper));

            // Repeatedly add and subtract a randomly initialized p_test2 from p_test1
            for(std::size_t i = 0; i < n_tests; i++) {
                // Randomly initialize p_test2
                CHECK_NOTHROW(p_test2->randomInit_(activityMode::ALLPARAMETERS, gr));

                fp_type first_value = p_test2->value();

                // Inside of the allowed value range ?
                CHECK(first_value >= lower);
                CHECK(first_value < upper);

                // Add to p_test1
                CHECK_NOTHROW(p_test1->GParameterBase::template add<fp_type>(
                    p_test2,
                    activityMode::ALLPARAMETERS
                ));

                // Check that p_test1 is still inside of the allowed value range
                CHECK(p_test1->value() >= lower);
                CHECK(p_test1->value() < upper);

                // Randomly initialize p_test2 again
                CHECK_NOTHROW(p_test2->randomInit_(activityMode::ALLPARAMETERS, gr));

                fp_type second_value = p_test2->value();

                // Inside of the allowed value range ?
                CHECK(second_value >= lower);
                CHECK(second_value < upper);

                // Has the value changed at all ?
                CHECK(first_value != second_value);

                // Subtract from p_test1
                CHECK_NOTHROW(p_test1->GParameterBase::template subtract<fp_type>(
                    p_test2,
                    activityMode::ALLPARAMETERS
                ));

                // Check that p_test1 is still inside of the allowed value range
                CHECK(p_test1->value() >= lower);
                CHECK(p_test1->value() < upper);
            }
        }

        //------------------------------------------------------------------------------

#else /* GEM_TESTING */ // If this function is called when GEM_TESTING isn't set, throw
        Gem::Common::condnotset(
            "GConstrainedFPT<>::specificTestsNoFailureExpected_GUnitTests",
            "GEM_TESTING"
        );
#endif                  /* GEM_TESTING */
    }

    /***************************************************************************/
    /**
     * Performs self tests that are expected to fail. This is needed for testing purposes
     */
    void specificTestsFailuresExpected_GUnitTests_() override {
#ifdef GEM_TESTING
        // Some general settings
        const fp_type test_val = fp_type(42);
        const fp_type lower_boundary = fp_type(0);
        const fp_type upper_boundary = fp_type(100);

        // Call the parent classes' functions
        GConstrainedNumT<fp_type>::specificTestsFailuresExpected_GUnitTests_();

        //------------------------------------------------------------------------------

        { // Check that assignment of a value equal to the upper boundary with setValue(val, lower, upper) throws
            std::shared_ptr<GConstrainedFPT<fp_type>> p_test =
                this->template clone<GConstrainedFPT<fp_type>>();

            // Reset the boundaries so we are free to do what we want
            CHECK_NOTHROW(p_test->resetBoundaries());

            // Set value, upper and lower boundaries; should throw, as value >= upperBoundary
            CHECK_THROWS_AS(
                (p_test->setValue(1.1 * upper_boundary, lower_boundary, upper_boundary)),
                geneva_exception
            );
        }

        //------------------------------------------------------------------------------

        { // Check that assignment of a value equal to the upper boundary with setValue() throws
            std::shared_ptr<GConstrainedFPT<fp_type>> p_test =
                this->template clone<GConstrainedFPT<fp_type>>();

            // Reset the boundaries so we are free to do what we want
            CHECK_NOTHROW(p_test->resetBoundaries());

            // Set value, upper and lower boundaries
            CHECK_NOTHROW(p_test->setValue(test_val, lower_boundary, upper_boundary));

            // Try to set a value equal to the upper boundary, should throw
            CHECK_THROWS_AS((p_test->setValue(1.1 * upper_boundary)), geneva_exception);
        }

        //------------------------------------------------------------------------------

        { // Check that setting an upper boundary <= lower boundary with setBoundaries(lower, upper) throws
            std::shared_ptr<GConstrainedFPT<fp_type>> p_test =
                this->template clone<GConstrainedFPT<fp_type>>();

            // Reset the boundaries so we are free to do what we want
            CHECK_NOTHROW(p_test->resetBoundaries());

            // Try to set an upper boundary == lower boundary
            CHECK_THROWS_AS(
                (p_test->setBoundaries(lower_boundary, lower_boundary)),
                geneva_exception
            );
        }

        //------------------------------------------------------------------------------

        { // Check that setting an upper boundary <= lower boundary with setValue(val, lower, upper) throws
            std::shared_ptr<GConstrainedFPT<fp_type>> p_test =
                this->template clone<GConstrainedFPT<fp_type>>();

            // Reset the boundaries so we are free to do what we want
            CHECK_NOTHROW(p_test->resetBoundaries());

            // Try to set an upper boundary == lower boundary
            CHECK_THROWS_AS(
                (p_test->setValue(lower_boundary, lower_boundary, lower_boundary)),
                geneva_exception
            );
        }

        //------------------------------------------------------------------------------

        { // Check that setting an upper boundary larger than the allowed value (see GConstrainedValueLimit<T>) with the setValue(val, lower, upper) function throws
            std::shared_ptr<GConstrainedFPT<fp_type>> p_test =
                this->template clone<GConstrainedFPT<fp_type>>();

            // Reset the boundaries so we are free to do what we want
            CHECK_NOTHROW(p_test->resetBoundaries());

            // Check that the boundaries have the expected values
            CHECK(p_test->getLowerBoundary() == GConstrainedValueLimitT<fp_type>::lowest());
            CHECK(
                p_test->getUpperBoundary() ==
                std::nextafter(GConstrainedValueLimitT<fp_type>::highest(), -std::numeric_limits<fp_type>::infinity())
            );

            // Try to set a boundary to a bad value
            CHECK_THROWS_AS(
                (p_test->setValue(
                    lower_boundary,
                    lower_boundary,
                    std::numeric_limits<fp_type>::max()
                )),
                geneva_exception
            );
        }

        //------------------------------------------------------------------------------

        { // Check that setting a lower boundary smaller than the allowed value (see GConstrainedValueLimit<T>)  with the setValue(val, lower, upper) function throws
            std::shared_ptr<GConstrainedFPT<fp_type>> p_test =
                this->template clone<GConstrainedFPT<fp_type>>();

            // Reset the boundaries so we are free to do what we want
            CHECK_NOTHROW(p_test->resetBoundaries());

            // Check that the boundaries have the expected values
            CHECK(p_test->getLowerBoundary() == GConstrainedValueLimitT<fp_type>::lowest());
            CHECK(
                p_test->getUpperBoundary() ==
                std::nextafter(GConstrainedValueLimitT<fp_type>::highest(), -std::numeric_limits<fp_type>::infinity())
            );

            // Try to set a boundary to a bad value
            CHECK_THROWS_AS(
                (p_test->setValue(0., std::numeric_limits<fp_type>::lowest(), upper_boundary)),
                geneva_exception
            );
        }

        //------------------------------------------------------------------------------

        { // Check that setting an upper boundary larger than the allowed value (see GConstrainedValueLimit<T>) with the setBoundaries(lower, upper) function throws
            std::shared_ptr<GConstrainedFPT<fp_type>> p_test =
                this->template clone<GConstrainedFPT<fp_type>>();

            // Reset the boundaries so we are free to do what we want
            CHECK_NOTHROW(p_test->resetBoundaries());

            // Check that the boundaries have the expected values
            CHECK(p_test->getLowerBoundary() == GConstrainedValueLimitT<fp_type>::lowest());
            CHECK(
                p_test->getUpperBoundary() ==
                std::nextafter(GConstrainedValueLimitT<fp_type>::highest(), -std::numeric_limits<fp_type>::infinity())
            );

            // Try to set a boundary to a bad value
            CHECK_THROWS_AS(
                (p_test->setBoundaries(lower_boundary, std::numeric_limits<fp_type>::max())),
                geneva_exception
            );
        }

        //------------------------------------------------------------------------------

        { // Check that setting a lower boundary smaller than the allowed value (see GConstrainedValueLimit<T>) with the setBoundaries(lower, upper) function throws
            std::shared_ptr<GConstrainedFPT<fp_type>> p_test =
                this->template clone<GConstrainedFPT<fp_type>>();

            // Reset the boundaries so we are free to do what we want
            CHECK_NOTHROW(p_test->resetBoundaries());

            // Check that the boundaries have the expected values
            CHECK(p_test->getLowerBoundary() == GConstrainedValueLimitT<fp_type>::lowest());
            CHECK(
                p_test->getUpperBoundary() ==
                std::nextafter(GConstrainedValueLimitT<fp_type>::highest(), -std::numeric_limits<fp_type>::infinity())
            );

            // Try to set a boundary to a bad value
            CHECK_THROWS_AS(
                (p_test->setBoundaries(std::numeric_limits<fp_type>::lowest(), upper_boundary)),
                geneva_exception
            );
        }

        //------------------------------------------------------------------------------

#else /* GEM_TESTING */ // If this function is called when GEM_TESTING isn't set, throw
        Gem::Common::condnotset(
            "GConstrainedFPT<>::specificTestsFailuresExpected_GUnitTests",
            "GEM_TESTING"
        );
#endif                  /* GEM_TESTING */
    }

private:
    /***************************************************************************/
    /**
	  * Emits a name for this class / object
	  */
    std::string name_() const override {
        return std::string("GConstrainedFPT");
    }

    /***************************************************************************/
    /** @brief Create a deep copy of this object */
    GObject *clone_() const override = 0;

    /***************************************************************************/
};

} /* namespace Gem::Geneva::Parameters */

/******************************************************************************/
// The content of BOOST_SERIALIZATION_ASSUME_ABSTRACT(T) // NOLINT
namespace boost::serialization {
template <typename fp_type>
struct is_abstract<Gem::Geneva::Parameters::GConstrainedFPT<fp_type>> : public boost::true_type {};
template <typename fp_type>
struct is_abstract<const Gem::Geneva::Parameters::GConstrainedFPT<fp_type>> : public boost::true_type {};
} /* namespace boost::serialization */
/******************************************************************************/
