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

// Standard header files go here

// Boost header files go here

// Geneva header files go here

#include "geneva/GNumCollectionT.hpp"

namespace Gem {
namespace Geneva {

/******************************************************************************/
/**
 * A collection of integer objects without boundaries
 */
template<typename int_type>
class GIntNumCollectionT
    : public GNumCollectionT<int_type>
{
    ///////////////////////////////////////////////////////////////////////
    friend class boost::serialization::access;

    template<typename Archive>
    void serialize(Archive &ar, const unsigned int) {
        using boost::serialization::make_nvp;

        ar
        & make_nvp(
            "GNumCollectionT_intType"
            , boost::serialization::base_object<GNumCollectionT<int_type>>(*this));
    }
    ///////////////////////////////////////////////////////////////////////

    // Make sure this class can only be instantiated if int_type is a *signed* integer type
    static_assert(
        std::is_signed<int_type>::value
        , "int_type should be a signed integer type"
    );

public:
    /***************************************************************************/
    /**
     * The default constructor
     */
    GIntNumCollectionT() = default;

    /***************************************************************************/
    /**
     * Initialization with a number of random values in a given range
     *
     * @param nval The amount of random values
     * @param min The minimum random value
     * @param max The maximum random value
     */
    GIntNumCollectionT(
        const std::size_t &nval
        , const int_type &min
        , const int_type &max
    )
        :
        GNumCollectionT<int_type>(
            nval
            , min
            , min
            , max
        ) // Initialization of a vector with nval variables of value "min"
    {
        Gem::Hap::GRandomT<Gem::Hap::RANDFLAVOURS::RANDOMLOCAL> gr;
        typename std::uniform_int_distribution<int_type> uniform_int(
            min
            , max
        );

        // Fill the vector with random values
        typename GIntNumCollectionT<int_type>::iterator it;
        for (it = this->begin(); it != this->end(); ++it) {
            *it = uniform_int(gr);
        }
    }

    /***************************************************************************/
    /**
     * Specifies the size of the data vector and an item to be assigned
     * to each position. We enforce setting of the lower and upper boundaries
     * for random initialization, as these may double up as the preferred value
     * range in some optimization algorithms.
     *
     * @param nval The amount of random values
     * @param min The minimum random value
     * @param max The maximum random value
     */
    GIntNumCollectionT(
        const std::size_t &nval
        , const int_type &val
        , const int_type &min
        , const int_type &max
    )
        :
        GNumCollectionT<int_type>(
            nval
            , val
            , min
            , max
        ) { /* nothing */ }

    /***************************************************************************/
    /**
     * The copy constructor
     *
     * @param cp A copy of another GIntNumCollectionT<int_type> object
     */
    GIntNumCollectionT(const GIntNumCollectionT<int_type> &cp) = default;

    /***************************************************************************/
    /**
     * The destructor
     */
    ~GIntNumCollectionT() override = default;

protected:
    /***************************************************************************/
    /**
     * Loads the data of another GObject
     *
     * @param cp A copy of another GIntNumCollectionT<int_type>  object, camouflaged as a GObject
     */
    void load_(const GObject *cp) override {
        // Convert the pointer to our target type and check for self-assignment
        const GIntNumCollectionT<int_type>
            *p_load = Gem::Common::g_convert_and_compare<GObject, GIntNumCollectionT<int_type>>(
            cp
            , this
        );

        // Load our parent class'es data ...
        GNumCollectionT<int_type>::load_(cp);

        // ... no local data
    }

    /***************************************************************************/
    /** @brief Allow access to this classes compare_ function */
    friend void Gem::Common::compare_base_t<GIntNumCollectionT<int_type>>(
        GIntNumCollectionT<int_type> const &
        , GIntNumCollectionT<int_type> const &
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

        // Check that we are dealing with a GIntNumCollectionT<int_type> reference independent of this object and convert the pointer
        const GIntNumCollectionT<int_type>
            *p_load = Gem::Common::g_convert_and_compare<GObject, GIntNumCollectionT<int_type>>(
            cp
            , this
        );

        GToken token(
            "GIntNumCollectionT<int_type>"
            , e
        );

        // Compare our parent data ...
        Gem::Common::compare_base_t<GNumCollectionT<int_type>>(
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
     * Triggers random initialization of the parameter collection. Note that this
     * function assumes that the collection has been completely set up. Data
     * that is added later will remain unaffected.
     */
    bool randomInit_(
        const activityMode &am
        , Gem::Hap::GRandomBase &gr
    ) override {
        int_type lowerBoundary = GNumCollectionT<int_type>::getLowerInitBoundary();
        int_type upperBoundary = GNumCollectionT<int_type>::getUpperInitBoundary();

        typename std::uniform_int_distribution<int_type> uniform_int(
            lowerBoundary
            , upperBoundary
        );
        typename GIntNumCollectionT<int_type>::iterator it;
        for (it = this->begin(); it != this->end(); ++it) {
            (*it) = uniform_int(gr);
        }

        return true;
    }

    /* ----------------------------------------------------------------------------------
     * Tested in GIntNumCollectionT<int_type>::specificTestsNoFailuresExpected_GUnitTests()
     * ----------------------------------------------------------------------------------
     */

    /***************************************************************************/
    /**
     * Applies modifications to this object. This is needed for testing purposes
     */
    bool modify_GUnitTests_() override {
#ifdef GEM_TESTING
        bool result = false;

        // Call the parent class'es function
        if (GNumCollectionT<int_type>::modify_GUnitTests_()) { result = true; }

        return result;

#else /* GEM_TESTING */  // If this function is called when GEM_TESTING isn't set, throw
        Gem::Common::condnotset("GIntNumCollectionT<>::modify_GUnitTests", "GEM_TESTING");
       return false;
#endif /* GEM_TESTING */
    }

    /***************************************************************************/
    /**
     * Performs self tests that are expected to succeed. This is needed for testing purposes
     */
    void specificTestsNoFailureExpected_GUnitTests_() override {
#ifdef GEM_TESTING
        // A few general settings
        const std::size_t nItems = 100;
        const int_type LOWERINITBOUNDARY = int_type(0); // non-negative value, as int_type might be negative
        const int_type UPPERINITBOUNDARY = int_type(10);
        const int_type FIXEDVALUEINIT = int_type(1);

        // Call the parent class'es function
        GNumCollectionT<int_type>::specificTestsNoFailureExpected_GUnitTests_();

        // A random generator
        Gem::Hap::GRandomT<Gem::Hap::RANDFLAVOURS::RANDOMPROXY> gr;

        //------------------------------------------------------------------------------

        { // Initialize with a fixed value, then check setting and retrieval of boundaries and random initialization
            std::shared_ptr<GIntNumCollectionT<int_type>>
                    p_test1 = this->template clone<GIntNumCollectionT<int_type>>();
            std::shared_ptr<GIntNumCollectionT<int_type>>
                    p_test2 = this->template clone<GIntNumCollectionT<int_type>>();

            // Make sure p_test1 and p_test2 are empty
            BOOST_CHECK_NO_THROW(p_test1->clear());
            BOOST_CHECK_NO_THROW(p_test2->clear());

            // Add a few items
            for (std::size_t i = 0; i < nItems; i++) {
                p_test1->push_back(
                        2 * UPPERINITBOUNDARY
                ); // Make sure random initialization cannot randomly leave the value unchanged
            }

            // Set initialization boundaries
            BOOST_CHECK_NO_THROW(p_test1->setInitBoundaries(
                    LOWERINITBOUNDARY
                    , UPPERINITBOUNDARY
            ));

            // Check that the boundaries have been set as expected
            BOOST_CHECK(p_test1->getLowerInitBoundary() == LOWERINITBOUNDARY);
            BOOST_CHECK(p_test1->getUpperInitBoundary() == UPPERINITBOUNDARY);

            // Load the data of p_test1 into p_test2
            BOOST_CHECK_NO_THROW(p_test2->load(p_test1));
            // Cross check that both are indeed equal
            BOOST_CHECK(*p_test1 == *p_test2);

            // Randomly initialize one of the two objects. Note: we are using the protected function rather than the "global" function
            BOOST_CHECK_NO_THROW(p_test1->randomInit_(
                    activityMode::ALLPARAMETERS
                    , gr
            ));

            // Check that the object has indeed changed
            BOOST_CHECK(*p_test1 != *p_test2);

            // Check that the values of p_test1 are inside of the allowed boundaries
            for (std::size_t i = 0; i < nItems; i++) {
                BOOST_CHECK(p_test1->at(i) != p_test2->at(i));
                BOOST_CHECK(p_test1->at(i) >= LOWERINITBOUNDARY);
                BOOST_CHECK(p_test1->at(i) <= UPPERINITBOUNDARY);
            }
        }

        //------------------------------------------------------------------------------

        { // Check that the fp-family of functions doesn't have an effect on this object
            std::shared_ptr<GIntNumCollectionT<int_type>>
                    p_test1 = this->template clone<GIntNumCollectionT<int_type>>();
            std::shared_ptr<GIntNumCollectionT<int_type>>
                    p_test2 = this->template clone<GIntNumCollectionT<int_type>>();
            std::shared_ptr<GIntNumCollectionT<int_type>>
                    p_test3 = this->template clone<GIntNumCollectionT<int_type>>();

            // Add a few items to p_test1
            for (std::size_t i = 0; i < nItems; i++) {
                p_test1->push_back(FIXEDVALUEINIT);
            }

            // Load into p_test2 and p_test3 and test equality
            BOOST_CHECK_NO_THROW(p_test2->load(p_test1));
            BOOST_CHECK_NO_THROW(p_test3->load(p_test1));
            BOOST_CHECK(*p_test2 == *p_test1);
            BOOST_CHECK(*p_test3 == *p_test1);
            BOOST_CHECK(*p_test3 == *p_test2);

            // Check that initialization with a fixed floating point value has no effect on this object
            BOOST_CHECK_NO_THROW(p_test2->template fixedValueInit<double>(
                    2.
                    , activityMode::ALLPARAMETERS
            ));
            BOOST_CHECK(*p_test2 == *p_test1);

            // Check that multiplication with a fixed floating point value has no effect on this object
            BOOST_CHECK_NO_THROW(p_test2->template multiplyBy<double>(
                    2.
                    , activityMode::ALLPARAMETERS
            ));
            BOOST_CHECK(*p_test2 == *p_test1);

            // Check that a component-wise multiplication with a random fp value in a given range does not have an effect on this object
            BOOST_CHECK_NO_THROW(p_test2->template multiplyByRandom<double>(
                    1.
                    , 2.
                    , activityMode::ALLPARAMETERS
                    , gr
            ));
            BOOST_CHECK(*p_test2 == *p_test1);

            // Check that a component-wise multiplication with a random fp value in the range [0:1[ does not have an effect on this object
            BOOST_CHECK_NO_THROW(p_test2->template multiplyByRandom<double>(
                    activityMode::ALLPARAMETERS
                    , gr
            ));
            BOOST_CHECK(*p_test2 == *p_test1);

            // Check that adding p_test1 to p_test3 does not have an effect
            BOOST_CHECK_NO_THROW(p_test3->template add<double>(
                    p_test1
                    , activityMode::ALLPARAMETERS
            ));
            BOOST_CHECK(*p_test3 == *p_test2);

            // Check that subtracting p_test1 from p_test3 does not have an effect
            BOOST_CHECK_NO_THROW(p_test3->template subtract<double>(
                    p_test1
                    , activityMode::ALLPARAMETERS
            ));
            BOOST_CHECK(*p_test3 == *p_test2);
        }

        //------------------------------------------------------------------------------

#else /* GEM_TESTING */  // If this function is called when GEM_TESTING isn't set, throw
        Gem::Common::condnotset("GIntNumCollectionT<>::specificTestsNoFailureExpected_GUnitTests", "GEM_TESTING");
#endif /* GEM_TESTING */
    }

    /***************************************************************************/
    /**
     * Performs self tests that are expected to fail. This is needed for testing purposes
     */
    void specificTestsFailuresExpected_GUnitTests_() override {
#ifdef GEM_TESTING
        // Call the parent class'es function
        GNumCollectionT<int_type>::specificTestsFailuresExpected_GUnitTests_();

#else /* GEM_TESTING */  // If this function is called when GEM_TESTING isn't set, throw
        Gem::Common::condnotset("GIntNumCollectionT<>::specificTestsFailuresExpected_GUnitTests", "GEM_TESTING");
#endif /* GEM_TESTING */
    }

    /***************************************************************************/

private:
    /***************************************************************************/
    /**
     * Emits a name for this class / object
     */
    std::string name_() const override {
        return std::string("GIntNumCollectionT");
    }

    /***************************************************************************/
    /** @brief Creates a deep clone of this object. Purely virtual, needs to be defined by derived classes */
    GObject *clone_() const override = 0;
};

/******************************************************************************/

} /* namespace Geneva */
} /* namespace Gem */

/******************************************************************************/
// The content of BOOST_SERIALIZATION_ASSUME_ABSTRACT(T)

namespace boost {
namespace serialization {
template<typename int_type>
struct is_abstract<Gem::Geneva::GIntNumCollectionT<int_type>> :
    public boost::true_type
{
};
template<typename int_type>
struct is_abstract<const Gem::Geneva::GIntNumCollectionT<int_type>> :
    public boost::true_type
{
};
}
}

/******************************************************************************/

