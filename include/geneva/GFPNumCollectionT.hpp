/**
 * @file
 */

/*
 * Copyright (C) Gemfony scientific UG (haftungsbeschraenkt)
 *
 * See the AUTHORS file in the top-level directory for a list of authors.
 *
 * Contact: contact [at] gemfony (dot) eu
 *
 * This file is part of the Geneva library collection.
 *
 * Geneva was developed with kind support from Karlsruhe Institute of
 * Technology (KIT) and Steinbuch Centre for Computing (SCC). Further
 * information about KIT and SCC can be found at http://www.kit.edu/english
 * and http://scc.kit.edu .
 *
 * Geneva is free software: you can redistribute and/or modify it under
 * the terms of version 3 of the GNU Affero General Public License
 * as published by the Free Software Foundation.
 *
 * Geneva is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with the Geneva library. If not, see <http://www.gnu.org/licenses/>.
 *
 * For further information on Gemfony scientific and Geneva, visit
 * http://www.gemfony.eu .
 */

#pragma once

// Global checks, defines and includes needed for all of Geneva
#include "common/GGlobalDefines.hpp"

// Standard header files go here
#include <type_traits>

// Boost header files go here

// Geneva header files go here
#include "geneva/GObject.hpp"
#include "geneva/GDoubleGaussAdaptor.hpp"
#include "geneva/GNumCollectionT.hpp"
#include "common/GExceptions.hpp"

namespace Gem {
namespace Geneva {

/******************************************************************************/
/**
 * This class represents a collection of floating point values, all modified
 * using the same algorithm. The most likely type to be stored in this
 * class is a double.
 */
template<typename fp_type>
class GFPNumCollectionT
    : public GNumCollectionT<fp_type>
{
    ///////////////////////////////////////////////////////////////////////
    friend class boost::serialization::access;

    template<typename Archive>
    void serialize(Archive &ar, const unsigned int) {
        using boost::serialization::make_nvp;
        ar
        & make_nvp(
            "GNumCollectionT_fpType"
            , boost::serialization::base_object<GNumCollectionT<fp_type>>(*this));
    }
    ///////////////////////////////////////////////////////////////////////

    // Make sure this class can only be instantiated if fp_type really is a floating point type
    static_assert(
        std::is_floating_point<fp_type>::value
        , "fp_type should be a floating point type"
    );

public:
    /***************************************************************************/
    /**
     * The default constructor.
     */
    GFPNumCollectionT() = default;

    /***************************************************************************/
    /**
     * Initialization with a number of random values in a given range
     *
     * @param nval The amount of random values
     * @param min The minimum random value
     * @param max The maximum random value
     */
    GFPNumCollectionT(
        const std::size_t &nval
        , const fp_type &min
        , const fp_type &max
    )
        :
        GNumCollectionT<fp_type>(
            nval
            , min
            , min
            , max
        ) // The vector is preset to nval entries with value "min"
    {
        // No need to resize the vector. as the parent class should already have the desired size

        Gem::Hap::GRandomT<Gem::Hap::RANDFLAVOURS::RANDOMLOCAL> gr;
        GFPNumCollectionT<fp_type>::randomInit(
            activityMode::ACTIVEONLY
            , gr
        );
    }

    /***************************************************************************/
    /**
     * Initialization with a number of items of predefined value. We
     * enforce setting of the lower and upper boundaries for random initialization,
     * as these double up as the preferred value range in some optimization algorithms,
     * such as swarm algorithms.
     *
     * @param nval The number of variables to be stored in the collection
     * @param val  The value to be used for their initialization
     */
    GFPNumCollectionT(
        const std::size_t &nval
        , const fp_type &val
        , const fp_type &min
        , const fp_type &max
    )
        :
        GNumCollectionT<fp_type>(
            nval
            , val
            , min
            , max
        ) { /* nothing */ }

    /***************************************************************************/
    /**
     * The standard copy constructor
     */
    GFPNumCollectionT(const GFPNumCollectionT<fp_type> &cp) = default;

    /***************************************************************************/
    /**
     * The standard destructor
     */
    ~GFPNumCollectionT() override = default;

protected:
    /***************************************************************************/
    /**
     * Loads the data of another GFPNumCollectionT<fp_type> object,
     * camouflaged as a GObject. We have no local data, so
     * all we need to do is to the standard identity check,
     * preventing that an object is assigned to itself.
     *
     * @param cp A copy of another GFPNumCollectionT<fp_type> object, camouflaged as a GObject
     */
    void load_(const GObject *cp) override {
        // Check that we are dealing with a GFPNumCollectionT<fp_type> reference independent of this object and convert the pointer
        const GFPNumCollectionT<fp_type>
            *p_load = Gem::Common::g_convert_and_compare<GObject, GFPNumCollectionT<fp_type>>(
            cp
            , this
        );

        // Load our parent class'es data ...
        GNumCollectionT<fp_type>::load_(cp);

        // no local data ...
    }


    /***************************************************************************/
    /** @brief Allow access to this classes compare_ function */
    friend void Gem::Common::compare_base_t<GFPNumCollectionT<fp_type>>(
        GFPNumCollectionT<fp_type> const &
        , GFPNumCollectionT<fp_type> const &
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

        // Check that we are dealing with a GFPNumCollectionT<fp_type> reference independent of this object and convert the pointer
        const GFPNumCollectionT<fp_type>
            *p_load = Gem::Common::g_convert_and_compare<GObject, GFPNumCollectionT<fp_type>>(
            cp
            , this
        );

        GToken token(
            "GFPNumCollectionT<fp_type>"
            , e
        );

        // Compare our parent data ...
        Gem::Common::compare_base_t<GNumCollectionT<fp_type>>(
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
     * Triggers random initialization of the parameter collection. Note
     * that this function assumes that the collection has been completely
     * set up. Data that is added later will remain unaffected.
     */
    bool randomInit_(
        const activityMode &am
        , Gem::Hap::GRandomBase &gr
    ) override {
        fp_type lowerBoundary = GNumCollectionT<fp_type>::getLowerInitBoundary();
        fp_type upperBoundary = GNumCollectionT<fp_type>::getUpperInitBoundary();

        typename std::uniform_real_distribution<fp_type> uniform_real_distribution(
            lowerBoundary
            , upperBoundary
        );
        typename GFPNumCollectionT<fp_type>::iterator it;
        for (it = this->begin(); it != this->end(); ++it) {
            (*it) = uniform_real_distribution(gr);
        }

        return true;
    }

    /* ----------------------------------------------------------------------------------
     * Tested in GFPNumCollectionT<fp_type>::specificTestsNoFailuresExpected_GUnitTests()
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
        if (GNumCollectionT<fp_type>::modify_GUnitTests_()) { result = true; }

        return result;

#else /* GEM_TESTING */  // If this function is called when GEM_TESTING isn't set, throw
        Gem::Common::condnotset("GFPNumCollectionT::modify_GUnitTests", "GEM_TESTING");
       return false;
#endif /* GEM_TESTING */
    }

    /***************************************************************************/
    /**
     * Performs self tests that are expected to succeed. This is needed for testing purposes
     */
    void specificTestsNoFailureExpected_GUnitTests_() override {
#ifdef GEM_TESTING
        // Call the parent classes' functions
        GNumCollectionT<fp_type>::specificTestsNoFailureExpected_GUnitTests_();

        // A random generator
        Gem::Hap::GRandomT<Gem::Hap::RANDFLAVOURS::RANDOMPROXY> gr;

        // A few settings
        const std::size_t nItems = 100;
        const fp_type LOWERINITBOUNDARY = -10.1;
        const fp_type UPPERINITBOUNDARY = 10.1;
        const fp_type FIXEDVALUEINIT = 1.;
        const fp_type MULTVALUE = 3.;
        const fp_type RANDLOWERBOUNDARY = 0.;
        const fp_type RANDUPPERBOUNDARY = 10.;

        //------------------------------------------------------------------------------

        { // Check initialization with a fixed value, setting and retrieval of boundaries and random initialization
            std::shared_ptr<GFPNumCollectionT<fp_type>> p_test1 = this->template clone<GFPNumCollectionT<fp_type>>();
            std::shared_ptr<GFPNumCollectionT<fp_type>> p_test2 = this->template clone<GFPNumCollectionT<fp_type>>();

            // Make sure p_test1 and p_test2 are empty
            BOOST_CHECK_NO_THROW(p_test1->clear());
            BOOST_CHECK_NO_THROW(p_test2->clear());

            // Add a few items
            for (std::size_t i = 0; i < nItems; i++) {
                p_test1->push_back(fp_type(0));
                p_test2->push_back(fp_type(0));
            }

            // Initialize with a fixed value
            BOOST_CHECK_NO_THROW(p_test1->GParameterBase::template fixedValueInit<fp_type>(
                    FIXEDVALUEINIT
                    , activityMode::ALLPARAMETERS
            ));
            BOOST_CHECK_NO_THROW(p_test2->GParameterBase::template fixedValueInit<fp_type>(
                    FIXEDVALUEINIT
                    , activityMode::ALLPARAMETERS
            ));

            // Check that values have indeed been set
            for (std::size_t i = 0; i < nItems; i++) {
                BOOST_CHECK(p_test1->at(i) == FIXEDVALUEINIT);
                BOOST_CHECK(p_test2->at(i) == FIXEDVALUEINIT);
            }

            // Set initialization boundaries
            BOOST_CHECK_NO_THROW(p_test1->setInitBoundaries(
                    LOWERINITBOUNDARY
                    , UPPERINITBOUNDARY
            ));
            BOOST_CHECK_NO_THROW(p_test2->setInitBoundaries(
                    LOWERINITBOUNDARY
                    , UPPERINITBOUNDARY
            ));

            // Randomly initialize one of the two objects. Note: we are using the protected function rather than the "global" function
            BOOST_CHECK_NO_THROW(p_test1->randomInit_(
                    activityMode::ALLPARAMETERS
                    , gr
            ));

            // Check that the object has indeed changed
            BOOST_CHECK(*p_test1 != *p_test2);

            // Check that each value is different and that the values of p_test1 are inside of the allowed boundaries
            for (std::size_t i = 0; i < nItems; i++) {
                BOOST_CHECK(p_test1->at(i) != p_test2->at(i));
                BOOST_CHECK(p_test1->at(i) >= LOWERINITBOUNDARY);
                BOOST_CHECK(p_test1->at(i) <= UPPERINITBOUNDARY);
            }
        }

        //------------------------------------------------------------------------------

        { // Test multiplication with a fixed value
            std::shared_ptr<GFPNumCollectionT<fp_type>> p_test1 = this->template clone<GFPNumCollectionT<fp_type>>();
            std::shared_ptr<GFPNumCollectionT<fp_type>> p_test2 = this->template clone<GFPNumCollectionT<fp_type>>();

            // Make sure p_test1 and p_test2 are empty
            BOOST_CHECK_NO_THROW(p_test1->clear());

            // Add a few items
            for (std::size_t i = 0; i < nItems; i++) {
                p_test1->push_back(fp_type(0));
            }

            // Initialize with a fixed value
            BOOST_CHECK_NO_THROW(p_test1->GParameterBase::template fixedValueInit<fp_type>(
                    FIXEDVALUEINIT
                    , activityMode::ALLPARAMETERS
            ));

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
            for (std::size_t i = 0; i < nItems; i++) {
                BOOST_CHECK(p_test1->at(i) == MULTVALUE * p_test2->at(i));
            }
        }

        //------------------------------------------------------------------------------

        { // Test multiplication with a random value in fixed range
            std::shared_ptr<GFPNumCollectionT<fp_type>> p_test1 = this->template clone<GFPNumCollectionT<fp_type>>();
            std::shared_ptr<GFPNumCollectionT<fp_type>> p_test2 = this->template clone<GFPNumCollectionT<fp_type>>();

            // Make sure p_test1 and p_test2 are empty
            BOOST_CHECK_NO_THROW(p_test1->clear());

            // Add a few items
            for (std::size_t i = 0; i < nItems; i++) {
                p_test1->push_back(fp_type(0));
            }

            // Initialize with a fixed value
            BOOST_CHECK_NO_THROW(p_test1->GParameterBase::template fixedValueInit<fp_type>(
                    1.
                    , activityMode::ALLPARAMETERS
            ));

            // Multiply with random values in a given range
            BOOST_CHECK_NO_THROW(p_test1->GParameterBase::template multiplyByRandom<fp_type>(
                    RANDLOWERBOUNDARY
                    , RANDUPPERBOUNDARY
                    , activityMode::ALLPARAMETERS
                    , gr
            ));

            // Check that all values are in this range
            for (std::size_t i = 0; i < nItems; i++) {
                BOOST_CHECK(p_test1->at(i) >= RANDLOWERBOUNDARY);
                BOOST_CHECK(p_test1->at(i) <= RANDUPPERBOUNDARY);
            }
        }

        //------------------------------------------------------------------------------

        { // Test multiplication with a random value in the range [0:1[
            std::shared_ptr<GFPNumCollectionT<fp_type>> p_test1 = this->template clone<GFPNumCollectionT<fp_type>>();
            std::shared_ptr<GFPNumCollectionT<fp_type>> p_test2 = this->template clone<GFPNumCollectionT<fp_type>>();

            // Make sure p_test1 and p_test2 are empty
            BOOST_CHECK_NO_THROW(p_test1->clear());

            // Add a few items
            for (std::size_t i = 0; i < nItems; i++) {
                p_test1->push_back(fp_type(0));
            }

            // Initialize with a fixed value
            BOOST_CHECK_NO_THROW(p_test1->GParameterBase::template fixedValueInit<fp_type>(
                    1.
                    , activityMode::ALLPARAMETERS
            ));

            // Multiply with random values in a given range
            BOOST_CHECK_NO_THROW(p_test1->GParameterBase::template multiplyByRandom<fp_type>(
                    activityMode::ALLPARAMETERS
                    , gr
            ));

            // Check that all values are in this range
            for (std::size_t i = 0; i < nItems; i++) {
                BOOST_CHECK(p_test1->at(i) >= 0.);
                BOOST_CHECK(p_test1->at(i) <= 1.);
            }
        }

        //------------------------------------------------------------------------------

        { // Test addition of other GFPNumCollectionT<fp_type> objects
            std::shared_ptr<GFPNumCollectionT<fp_type>> p_test1 = this->template clone<GFPNumCollectionT<fp_type>>();
            std::shared_ptr<GFPNumCollectionT<fp_type>> p_test2 = this->template clone<GFPNumCollectionT<fp_type>>();
            std::shared_ptr<GFPNumCollectionT<fp_type>> p_test3 = this->template clone<GFPNumCollectionT<fp_type>>();

            // Make sure all clones are empty
            BOOST_CHECK_NO_THROW(p_test1->clear());
            BOOST_CHECK_NO_THROW(p_test2->clear());
            BOOST_CHECK_NO_THROW(p_test3->clear());

            // Add a few items
            for (std::size_t i = 0; i < nItems; i++) {
                p_test1->push_back(fp_type(0));
            }

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

            // Cross check that for each i p_test3[i] == p_test1[i] + p_test2[i]
            for (std::size_t i = 0; i < nItems; i++) {
                BOOST_CHECK(p_test3->at(i) == p_test1->at(i) + p_test2->at(i));
            }
        }

        //------------------------------------------------------------------------------

        { // Test subtraction of other GFPNumCollectionT<fp_type> objects
            std::shared_ptr<GFPNumCollectionT<fp_type>> p_test1 = this->template clone<GFPNumCollectionT<fp_type>>();
            std::shared_ptr<GFPNumCollectionT<fp_type>> p_test2 = this->template clone<GFPNumCollectionT<fp_type>>();
            std::shared_ptr<GFPNumCollectionT<fp_type>> p_test3 = this->template clone<GFPNumCollectionT<fp_type>>();

            // Make sure all clones are empty
            BOOST_CHECK_NO_THROW(p_test1->clear());
            BOOST_CHECK_NO_THROW(p_test2->clear());
            BOOST_CHECK_NO_THROW(p_test3->clear());

            // Add a few items
            for (std::size_t i = 0; i < nItems; i++) {
                p_test1->push_back(fp_type(0));
            }

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
            BOOST_CHECK_NO_THROW(p_test3->GParameterBase::template subtract<fp_type>(
                    p_test1
                    , activityMode::ALLPARAMETERS
            ));

            // Cross check that for each i p_test3[i] == p_test1[i] - p_test2[i]
            for (std::size_t i = 0; i < nItems; i++) {
                BOOST_CHECK(p_test3->at(i) == p_test2->at(i) - p_test1->at(i));
            }
        }

        //------------------------------------------------------------------------------

#else /* GEM_TESTING */ // If this function is called when GEM_TESTING isn't set, throw
        Gem::Common::condnotset("GFPNumCollectionT<>::specificTestsNoFailureExpected_GUnitTests", "GEM_TESTING");
#endif /* GEM_TESTING */
    }

    /***************************************************************************/
    /**
     * Performs self tests that are expected to fail. This is needed for testing purposes
     */
    void specificTestsFailuresExpected_GUnitTests_() override {
#ifdef GEM_TESTING
        // A few settings
        const std::size_t nItems = 100;

        // Call the parent classes' functions
        GNumCollectionT<fp_type>::specificTestsFailuresExpected_GUnitTests_();

        // A random generator
        Gem::Hap::GRandomT<Gem::Hap::RANDFLAVOURS::RANDOMPROXY> gr;

        //------------------------------------------------------------------------------

        { // Check that adding another object of different size throws
            std::shared_ptr<GFPNumCollectionT<fp_type>> p_test1 = this->template clone<GFPNumCollectionT<fp_type>>();
            std::shared_ptr<GFPNumCollectionT<fp_type>> p_test2 = this->template clone<GFPNumCollectionT<fp_type>>();

            // Add a few items to p_test1, but not to p_test2
            for (std::size_t i = 0; i < nItems; i++) {
                p_test1->push_back(fp_type(0));
            }

            BOOST_CHECK_THROW(p_test1->GParameterBase::template add<fp_type>(
                    p_test2
                    , activityMode::ALLPARAMETERS
            )
            , gemfony_exception);
        }

        //------------------------------------------------------------------------------

        { // Check that subtracting another object of different size throws
            std::shared_ptr<GFPNumCollectionT<fp_type>> p_test1 = this->template clone<GFPNumCollectionT<fp_type>>();
            std::shared_ptr<GFPNumCollectionT<fp_type>> p_test2 = this->template clone<GFPNumCollectionT<fp_type>>();

            // Add a few items to p_test1, but not to p_test2
            for (std::size_t i = 0; i < nItems; i++) {
                p_test1->push_back(fp_type(0));
            }

            BOOST_CHECK_THROW(p_test1->GParameterBase::template subtract<fp_type>(
                    p_test2
                    , activityMode::ALLPARAMETERS
            )
            , gemfony_exception);
        }

        //------------------------------------------------------------------------------

#else /* GEM_TESTING */ // If this function is called when GEM_TESTING isn't set, throw
        Gem::Common::condnotset("GFPNumCollectionT<>::specificTestsFailuresExpected_GUnitTests", "GEM_TESTING");
#endif /* GEM_TESTING */
    }

private:
    /***************************************************************************/
    /**
     * Emits a name for this class / object
     */
    std::string name_() const override {
        return std::string("GFPNumCollectionT");
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

/******************************************************************************/

} /* namespace Geneva */
} /* namespace Gem */

/******************************************************************************/
// The content of BOOST_SERIALIZATION_ASSUME_ABSTRACT(T)

namespace boost {
namespace serialization {
template<typename fp_type>
struct is_abstract<Gem::Geneva::GFPNumCollectionT<fp_type>> :
    public boost::true_type
{
};
template<typename fp_type>
struct is_abstract<const Gem::Geneva::GFPNumCollectionT<fp_type>> :
    public boost::true_type
{
};
}
}
/******************************************************************************/

