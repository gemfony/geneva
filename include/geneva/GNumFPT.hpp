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

#pragma once

// Global checks, defines and includes needed for all of Geneva
#include "common/GGlobalDefines.hpp"

// Standard headers go here
#include <type_traits>

// Boost headers go here

// Geneva headers go here
#include "geneva/GNumT.hpp"

namespace Gem {
namespace Geneva {

/******************************************************************************/
/**
 * This class represents floating point values. The most likely type to be stored
 * in this class is a double. It adds floating point initialization and multiplication
 * to GNumT
 */
template<typename fp_type>
class GNumFPT
    : public GNumT<fp_type>
{
    ///////////////////////////////////////////////////////////////////////
    friend class boost::serialization::access;

    template<typename Archive>
    void serialize(Archive &ar, const unsigned int) {
        using boost::serialization::make_nvp;
        ar & make_nvp(
            "GNumT"
            , boost::serialization::base_object<GNumT<fp_type>>(*this));
    }
    ///////////////////////////////////////////////////////////////////////

    // Make sure this class can only be instantiated if fp_type really is a floating point type
    static_assert(
        std::is_floating_point<fp_type>::value
        , "fp_type should be a floating point type"
    );


public:
    /** @brief Specifies the type of parameters stored in this object */
    using parameter_type = fp_type;

    /***************************************************************************/
    /**
     * The default constructor.
     */
    GNumFPT() = default;

    /***************************************************************************/
    /*
     * Initialize with a single value
     *
     * @param val The value used for the initialization
     */
    explicit GNumFPT(const fp_type &val)
        :
        GNumT<fp_type>(val) { /* nothing */ }

    /***************************************************************************/
    /**
     * Initialize with a random value within given boundaries.
     *
     * @param min The lower boundary for random entries
     * @param max The upper boundary for random entries
     */
    GNumFPT(
        const fp_type &min
        , const fp_type &max
    )
        :
        GNumT<fp_type>(
            min
            , max
        ) {
        Gem::Hap::GRandomT<Gem::Hap::RANDFLAVOURS::RANDOMLOCAL> gr;
        GNumFPT<fp_type>::randomInit(
            activityMode::ACTIVEONLY
            , gr
        );
    }

    /***************************************************************************/
    /**
     * Initialize with a fixed value. Note that we enforce initialization
     * of the initialization boundaries as well, as these may play a role
     * in some optimization algorithms. Note that we do not enforce that the
     * assigned value lies inside these boundaries, as they are meant for
     * random initialization only.
     *
     * @param val The value to be assigned to the object
     * @param min The lower boundary for random entries
     * @param max The upper boundary for random entries
     */
    GNumFPT(
        const fp_type &val
        , const fp_type &min
        , const fp_type &max
    )
        :
        GNumT<fp_type>(
            min
            , max
        ) {
        GParameterT<fp_type>::setValue(val);
    }

    /***************************************************************************/
    /**
     * The standard copy constructor
     *
     * @param cp A constant reference to another GNumFPT<fp_type> object
     */
    GNumFPT(const GNumFPT<fp_type> &cp) = default;

    /***************************************************************************/
    /**
     * The standard destructor
     */
    ~GNumFPT() override = default;

    /***************************************************************************/
    /**
     * An assignment operator for the contained value type
     *
     * @param val The value to be assigned to this object
     * @return The value that was assigned to this object
     */
    GNumFPT<fp_type>& operator=(const fp_type &val) override {
        GNumT<fp_type>::operator=(val);
        return *this;
    }

protected:
    /***************************************************************************/
    /**
     * Loads the data of another GNumFPT<fp_type> object,
     * camouflaged as a GObject. We have no local data, so
     * all we need to do is to the standard identity check,
     * preventing that an object is assigned to itself.
     *
     * @param cp A copy of another GNumFPT<fp_type> object, camouflaged as a GObject
     */
    void load_(const GObject *cp) override {
        // Check that we are dealing with a GNumFPT<fp_type> reference independent of this object and convert the pointer
        const GNumFPT<fp_type> *p_load = Gem::Common::g_convert_and_compare<GObject, GNumFPT<fp_type>>(
            cp
            , this
        );

        // Load our parent class'es data ...
        GNumT<fp_type>::load_(cp);

        // no local data ...
    }

    /***************************************************************************/
    /** @brief Allow access to this classes compare_ function */
    friend void Gem::Common::compare_base_t<GNumFPT<fp_type>>(
        GNumFPT<fp_type> const &
        , GNumFPT<fp_type> const &
        , Gem::Common::GToken &
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
        const GObject &cp
        , const Gem::Common::expectation &e
        , const double &limit
    ) const override {
        using namespace Gem::Common;

        // Check that we are dealing with a GNumFPT<fp_type> reference independent of this object and convert the pointer
        const GNumFPT<fp_type> *p_load = Gem::Common::g_convert_and_compare<GObject, GNumFPT<fp_type>>(
            cp
            , this
        );

        GToken token(
            "GNumFPT<fp_type>"
            , e
        );

        // Compare our parent data ...
        Gem::Common::compare_base_t<GNumT<fp_type>>(
            *this
            , *p_load
            , token
        );

        // ... no local data

        // React on deviations from the expectation
        token.evaluate();
    }

    /***************************************************************************/
    /**
     * Triggers random initialization of the parameter
     */
    bool randomInit_(
        const activityMode &am
        , Gem::Hap::GRandomBase &gr
    ) override {
        fp_type lowerBoundary = GNumT<fp_type>::getLowerInitBoundary();
        fp_type upperBoundary = GNumT<fp_type>::getUpperInitBoundary();

        typename std::uniform_real_distribution<fp_type> uniform_real_distribution(
            lowerBoundary
            , upperBoundary
        );
        GParameterT<fp_type>::setValue(uniform_real_distribution(gr));

        return true;
    }

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
        if (GNumT<fp_type>::modify_GUnitTests_()) { result = true; }

        return result;

#else /* GEM_TESTING */  // If this function is called when GEM_TESTING isn't set, throw
        Gem::Common::condnotset("GNumFPT<>::modify_GUnitTests", "GEM_TESTING");
       return false;
#endif /* GEM_TESTING */
    }

    /***************************************************************************/
    /**
     * Performs self tests that are expected to succeed. This is needed for testing purposes
     */
    void specificTestsNoFailureExpected_GUnitTests_() override {
#ifdef GEM_TESTING
        // A few settings
        const std::size_t nTests = 100;
        const fp_type LOWERINITBOUNDARY = -10.1;
        const fp_type UPPERINITBOUNDARY = 10.1;
        const fp_type FIXEDVALUEINIT = 1.;
        const fp_type MULTVALUE = 3.;
        const fp_type RANDLOWERBOUNDARY = 0.;
        const fp_type RANDUPPERBOUNDARY = 10.;

        // Call the parent classes' functions
        GNumT<fp_type>::specificTestsNoFailureExpected_GUnitTests_();

        // A random generator
        Gem::Hap::GRandomT<Gem::Hap::RANDFLAVOURS::RANDOMPROXY> gr;

        //------------------------------------------------------------------------------

        { // Check initialization with a fixed value, setting and retrieval of boundaries and random initialization
            std::shared_ptr<GNumFPT<fp_type>> p_test1 = this->template clone<GNumFPT<fp_type>>();
            std::shared_ptr<GNumFPT<fp_type>> p_test2 = this->template clone<GNumFPT<fp_type>>();

            // Assign a defined start value
            p_test1->setValue(fp_type(0));

            // Initialize with a fixed value
            BOOST_CHECK_NO_THROW(p_test1->GParameterBase::template fixedValueInit<fp_type>(
                    boost::numeric_cast<fp_type>(
                            2. * UPPERINITBOUNDARY
                    )
                    , activityMode::ALLPARAMETERS
            )); // Make sure the parameters indeed change

            // Check that the value has indeed been set.
            BOOST_CHECK_MESSAGE(
                    fabs(p_test1->value() - fp_type(2. * UPPERINITBOUNDARY)) < pow(
                            10.
                            , -6
                    )
            , "\n"
                            << std::setprecision(10)
                            << "p_test1->value() = " << p_test1->value() << "\n"
                            << "2.*UPPERINITBOUNDARY = " << 2. * UPPERINITBOUNDARY << "\n"
                            << "fabs(p_test1->value() - 2.*UPPERINITBOUNDARY) = " << fabs(p_test1->value() - 2. * UPPERINITBOUNDARY)
                            << "\n"
                            << "pow(10., -8) = " << pow(
                    10.
                    , -8
            ) << "\n"
            );

            // Set initialization boundaries
            BOOST_CHECK_NO_THROW(p_test1->setInitBoundaries(
                    LOWERINITBOUNDARY
                    , UPPERINITBOUNDARY
            ));

            // Cross-check the boundaries
            BOOST_CHECK(p_test1->getLowerInitBoundary() == LOWERINITBOUNDARY);
            BOOST_CHECK(p_test1->getUpperInitBoundary() == UPPERINITBOUNDARY);

            // Check that each value is different and that the values of p_test1 are inside of the allowed boundaries
            for (std::size_t i = 0; i < nTests; i++) {
                // Load p_test1 into p_test2
                BOOST_CHECK_NO_THROW(p_test2->load(p_test1));
                // Cross-check that both objects are equal
                BOOST_CHECK(*p_test1 == *p_test2);

                // Randomly initialize one of the two objects. Note: we are using the protected function rather than the "global" function
                BOOST_CHECK_NO_THROW(p_test2->randomInit_(
                        activityMode::ALLPARAMETERS
                        , gr
                ));

                // Check that the object has indeed changed
                BOOST_CHECK(*p_test2 != *p_test1);

                BOOST_CHECK(p_test2->value() != p_test1->value());
                BOOST_CHECK(p_test2->value() >= LOWERINITBOUNDARY);
                BOOST_CHECK(p_test2->value() <= UPPERINITBOUNDARY);
            }
        }

        //------------------------------------------------------------------------------

        { // Test multiplication with a fixed value
            std::shared_ptr<GNumFPT<fp_type>> p_test1 = this->template clone<GNumFPT<fp_type>>();
            std::shared_ptr<GNumFPT<fp_type>> p_test2 = this->template clone<GNumFPT<fp_type>>();

            // Initialize with a fixed value
            BOOST_CHECK_NO_THROW(p_test1->GParameterBase::template fixedValueInit<fp_type>(
                    FIXEDVALUEINIT
                    , activityMode::ALLPARAMETERS
            ));

            // Check that this value has been set
            BOOST_CHECK(p_test1->value() == FIXEDVALUEINIT);

            // Set initialization boundaries
            BOOST_CHECK_NO_THROW(p_test1->setInitBoundaries(
                    LOWERINITBOUNDARY
                    , UPPERINITBOUNDARY
            ));

            // Randomly initialize one of the two objects. Note: we are using the protected function rather than the "global" function
            BOOST_CHECK_NO_THROW(p_test1->randomInit_(
                    activityMode::ALLPARAMETERS
                    , gr
            ));

            // Load the data into p_test2 and check that both objects are equal
            BOOST_CHECK_NO_THROW(p_test2->load(p_test1));
            BOOST_CHECK(*p_test1 == *p_test2);

            // Multiply p_test1 with a fixed value
            BOOST_CHECK_NO_THROW(p_test1->GParameterBase::template multiplyBy<fp_type>(
                    MULTVALUE
                    , activityMode::ALLPARAMETERS
            ));

            // Check that the multiplication has succeeded
            BOOST_CHECK(p_test1->value() == MULTVALUE * p_test2->value());
        }

        //------------------------------------------------------------------------------

        { // Test multiplication with a random value in fixed range
            std::shared_ptr<GNumFPT<fp_type>> p_test1 = this->template clone<GNumFPT<fp_type>>();

            // Initialize with a fixed value
            BOOST_CHECK_NO_THROW(p_test1->GParameterBase::template fixedValueInit<fp_type>(
                    1.
                    , activityMode::ALLPARAMETERS
            )); // 1. chosen so we see the multiplication value of the random number generator

            // Check that this value has been set
            BOOST_CHECK(p_test1->value() == 1.);

            // Multiply with random values in a given range
            BOOST_CHECK_NO_THROW(p_test1->GParameterBase::template multiplyByRandom<fp_type>(
                    RANDLOWERBOUNDARY
                    , RANDUPPERBOUNDARY
                    , activityMode::ALLPARAMETERS
                    , gr
            ));

            // Check that all values are in the allowed range
            BOOST_CHECK(p_test1->value() >= RANDLOWERBOUNDARY);
            BOOST_CHECK(p_test1->value() <= RANDUPPERBOUNDARY);
        }

        //------------------------------------------------------------------------------

        { // Test multiplication with a random value in the range [0:1[
            std::shared_ptr<GNumFPT<fp_type>> p_test1 = this->template clone<GNumFPT<fp_type>>();

            // Initialize with a fixed value
            BOOST_CHECK_NO_THROW(p_test1->GParameterBase::template fixedValueInit<fp_type>(
                    1.
                    , activityMode::ALLPARAMETERS
            )); // 1. chosen so we see the multiplication value of the random number generator

            // Check that this value has been set
            BOOST_CHECK(p_test1->value() == 1.);

            // Multiply with random values in a given range
            BOOST_CHECK_NO_THROW(p_test1->GParameterBase::template multiplyByRandom<fp_type>(
                    activityMode::ALLPARAMETERS
                    , gr
            ));

            // Check that all values are in the allowed range
            BOOST_CHECK(p_test1->value() >= 0.);
            BOOST_CHECK(p_test1->value() <= 1.);
        }

        //------------------------------------------------------------------------------

        { // Test addition of other GNumFPT<fp_type> objets
            std::shared_ptr<GNumFPT<fp_type>> p_test1 = this->template clone<GNumFPT<fp_type>>();
            std::shared_ptr<GNumFPT<fp_type>> p_test2 = this->template clone<GNumFPT<fp_type>>();
            std::shared_ptr<GNumFPT<fp_type>> p_test3 = this->template clone<GNumFPT<fp_type>>();

            // Initialize with a fixed value
            BOOST_CHECK_NO_THROW(p_test1->GParameterBase::template fixedValueInit<fp_type>(
                    0.
                    , activityMode::ALLPARAMETERS
            ));
            BOOST_CHECK(p_test1->value() == 0.);

            // Set initialization boundaries
            BOOST_CHECK_NO_THROW(p_test1->setInitBoundaries(
                    LOWERINITBOUNDARY
                    , UPPERINITBOUNDARY
            ));

            // Load the data of p_test_1 into p_test2
            BOOST_CHECK_NO_THROW(p_test2->load(p_test1));

            // Randomly initialize p_test1 and p_test2, so that both objects are different
            BOOST_CHECK_NO_THROW(p_test1->randomInit_(
                    activityMode::ALLPARAMETERS
                    , gr
            ));
            BOOST_CHECK_NO_THROW(p_test2->randomInit_(
                    activityMode::ALLPARAMETERS
                    , gr
            ));

            // Check that they are indeed different
            BOOST_CHECK(*p_test1 != *p_test2);

            // Load p_test2's data into p_test_3
            BOOST_CHECK_NO_THROW(p_test3->load(p_test2));

            // Add p_test1 to p_test3
            BOOST_CHECK_NO_THROW(p_test3->GParameterBase::template add<fp_type>(
                    p_test1
                    , activityMode::ALLPARAMETERS
            ));

            // Cross-check that the addition has worked
            BOOST_CHECK(p_test3->value() == p_test1->value() + p_test2->value());
        }

        //------------------------------------------------------------------------------

        { // Test subtraction of other GNumFPT<fp_type> objects
            std::shared_ptr<GNumFPT<fp_type>> p_test1 = this->template clone<GNumFPT<fp_type>>();
            std::shared_ptr<GNumFPT<fp_type>> p_test2 = this->template clone<GNumFPT<fp_type>>();
            std::shared_ptr<GNumFPT<fp_type>> p_test3 = this->template clone<GNumFPT<fp_type>>();

            // Initialize with a fixed value
            BOOST_CHECK_NO_THROW(p_test1->GParameterBase::template fixedValueInit<fp_type>(
                    0.
                    , activityMode::ALLPARAMETERS
            ));
            BOOST_CHECK(p_test1->value() == 0.);

            // Set initialization boundaries
            BOOST_CHECK_NO_THROW(p_test1->setInitBoundaries(
                    LOWERINITBOUNDARY
                    , UPPERINITBOUNDARY
            ));

            // Load the data of p_test_1 into p_test2
            BOOST_CHECK_NO_THROW(p_test2->load(p_test1));

            // Randomly initialize p_test1 and p_test2, so that both objects are different
            BOOST_CHECK_NO_THROW(p_test1->randomInit_(
                    activityMode::ALLPARAMETERS
                    , gr
            ));
            BOOST_CHECK_NO_THROW(p_test2->randomInit_(
                    activityMode::ALLPARAMETERS
                    , gr
            ));

            // Check that they are indeed different
            BOOST_CHECK(*p_test1 != *p_test2);

            // Load p_test2's data into p_test_3
            BOOST_CHECK_NO_THROW(p_test3->load(p_test2));

            // Subtract p_test1 from p_test3
            BOOST_CHECK_NO_THROW(p_test3->template subtract<fp_type>(
                    p_test1
                    , activityMode::ALLPARAMETERS
            ));

            // Cross-check that the addition has worked. Note that we do need to take into
            // account effects of floating point accuracy
            BOOST_CHECK(p_test3->value() == (p_test2->value() - p_test1->value()));
        }

        //------------------------------------------------------------------------------

#else /* GEM_TESTING */  // If this function is called when GEM_TESTING isn't set, throw
        Gem::Common::condnotset("GNumFPT<>::specificTestsNoFailureExpected_GUnitTests", "GEM_TESTING");
#endif /* GEM_TESTING */
    }

    /***************************************************************************/
    /**
     * Performs self tests that are expected to fail. This is needed for testing purposes
     */
    void specificTestsFailuresExpected_GUnitTests_() override {
#ifdef GEM_TESTING
        // Call the parent classes' functions
        GNumT<fp_type>::specificTestsFailuresExpected_GUnitTests_();

#else /* GEM_TESTING */  // If this function is called when GEM_TESTING isn't set, throw
        Gem::Common::condnotset("GNumFPT<>::specificTestsFailuresExpected_GUnitTests", "GEM_TESTING");
#endif /* GEM_TESTING */
    }

    /***************************************************************************/

private:
    /***************************************************************************/
    /**
     * Emits a name for this class / object
     */
    std::string name_() const override {
        return std::string("GNumFPT");
    }

    /***************************************************************************/
    /**
     * Creates a deep copy of this object. Purely virtual as this class
     * should not be instantiable.
     *
     * @return A pointer to a deep clone of this object
     */
    GObject *clone_() const override = 0;
};

} /* namespace Geneva */
} /* namespace Gem */

/******************************************************************************/
// The content of BOOST_SERIALIZATION_ASSUME_ABSTRACT(T)

namespace boost {
namespace serialization {
template<typename fp_type>
struct is_abstract<Gem::Geneva::GNumFPT<fp_type>> :
    public boost::true_type
{
};
template<typename fp_type>
struct is_abstract<const Gem::Geneva::GNumFPT<fp_type>> :
    public boost::true_type
{
};
}
}
/******************************************************************************/

