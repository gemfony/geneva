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
#include <type_traits>

// Boost header files go here

// Geneva header files go here
#include "common/GExceptions.hpp"
#include "common/GTypeToStringT.hpp"
#include "geneva/GObject.hpp"
#include "geneva/GDoubleGaussAdaptor.hpp"
#include "geneva/GParameterCollectionT.hpp"

namespace Gem {
namespace Geneva {

const double DEFAULTLOWERINITBOUNDARYCOLLECTION = 0.;
const double DEFAULTUPPERINITBOUNDARYCOLLECTION = 1.;

/******************************************************************************/
/**
 * This class represents a collection of numeric values, all modified
 * using the same algorithm. The most likely types to be stored in this
 * class are double and std::int32_t . By using the framework provided
 * by GParameterCollectionT, this class becomes rather simple.
 */
template<typename num_type>
class GNumCollectionT
    : public GParameterCollectionT<num_type>
{
    ///////////////////////////////////////////////////////////////////////
    friend class boost::serialization::access;

    template<typename Archive>
    void serialize(Archive &ar, const unsigned int) {
        using boost::serialization::make_nvp;
        ar
        & make_nvp(
            "GParameterCollectionT"
            , boost::serialization::base_object<GParameterCollectionT<num_type>>(*this))
        & BOOST_SERIALIZATION_NVP(lowerInitBoundary_)
        & BOOST_SERIALIZATION_NVP(upperInitBoundary_);
    }
    ///////////////////////////////////////////////////////////////////////

    // Make sure this class can only be instantiated with num_type as an arithmetic type
    static_assert(
        std::is_arithmetic<num_type>::value
        , "num_type should be an arithmetic type"
    );


public:
    /** @brief Specifies the type of parameters stored in this collection */
    using collection_type = num_type;

    /***************************************************************************/
    /**
     * The default constructor.
     */
    GNumCollectionT() = default;

    /***************************************************************************/
    /**
     * Specifies the boundaries for random initialization and initializes
     * the data vector with a given size. Note that we need to care for the
     * assignment of random values ourself in derived classes.
     *
     * @param nval The number of entries in the vector
     * @param min The lower boundary for random entries
     * @param max The upper boundary for random entries
     */
    GNumCollectionT(
        const std::size_t &nval
        , const num_type &min
        , const num_type &max
    )
        :
        GParameterCollectionT<num_type>(
            nval
            , min
        )
        , lowerInitBoundary_(min)
        , upperInitBoundary_(max)
    { /* nothing */ }

    /***************************************************************************/
    /**
     * Specifies the size of the data vector and an item to be assigned
     * to each position. We enforce setting of the lower and upper boundaries
     * for random initialization, as these double up as the preferred value
     * range in some optimization algorithms, such as swarm algorithms.
     *
     * @param nval The number of entries in the vector
     * @param val  The value to be assigned to each position
     * @param min The lower boundary for random entries
     * @param max The upper boundary for random entries
     */
    GNumCollectionT(
        const std::size_t &nval
        , const num_type &val
        , const num_type &min
        , const num_type &max
    )
        :
        GParameterCollectionT<num_type>(
            nval
            , val
        )
        , lowerInitBoundary_(min)
        , upperInitBoundary_(max)
    { /* nothing */ }

    /***************************************************************************/
    /**
     * The standard copy constructor
     */
    GNumCollectionT(GNumCollectionT const &cp) = default;

    /***************************************************************************/
    /**
     * The standard destructor
     */
    ~GNumCollectionT() override = default;

    /***************************************************************************/
    /**
     * Sets the initialization boundaries
     *
     * @param lowerInitBoundary The lower boundary for random initialization
     * @param upperInitBoundary The upper boundary for random initialization
     */
    void setInitBoundaries(
        const num_type &lowerInitBoundary
        , const num_type &upperInitBoundary
    ) {
        // Do some error checking
        if (lowerInitBoundary >= upperInitBoundary) {
            throw gemfony_exception(
                g_error_streamer(
                    DO_LOG
                    , time_and_place
                )
                    << "In GNumCollectionT<num_type>::setInitBoundaries():" << std::endl
                    << "Invalid boundaries provided: " << std::endl
                    << "lowerInitBoundary = " << lowerInitBoundary << std::endl
                    << "upperInitBoundary = " << upperInitBoundary << std::endl
            );
        }

        lowerInitBoundary_ = lowerInitBoundary;
        upperInitBoundary_ = upperInitBoundary;
    }

    /* ----------------------------------------------------------------------------------
     * Tested GFPNumCollectionT<num_type>::specificTestsNoFailureExpected_GUnitTests()
     * Tested GNumCollectionT<num_type>::specificTestsNoFailureExpected_GUnitTests()
     * Assignment of invalid boundaries Tested GNumCollectionT<num_type>::specificTestsFailuresExpected_GUnitTests()
     * ----------------------------------------------------------------------------------
     */

    /***************************************************************************/
    /**
     * Retrieves the value of the lower initialization boundary
     *
     * @return The value of the lower initialization boundary
     */
    num_type getLowerInitBoundary() const {
        return lowerInitBoundary_;
    }

    /* ----------------------------------------------------------------------------------
     * Tested GFPNumCollectionT<num_type>::specificTestsNoFailureExpected_GUnitTests()
     * Tested GNumCollectionT<num_type>::specificTestsNoFailureExpected_GUnitTests()
     * ----------------------------------------------------------------------------------
     */

    /***************************************************************************/
    /**
     * Retrieves the value of the upper initialization boundary
     *
     * @return The value of the upper initialization boundary
     */
    num_type getUpperInitBoundary() const {
        return upperInitBoundary_;
    }

    /* ----------------------------------------------------------------------------------
     * Tested GFPNumCollectionT<num_type>::specificTestsNoFailureExpected_GUnitTests()
     * Tested GNumCollectionT<num_type>::specificTestsNoFailureExpected_GUnitTests()
     * ----------------------------------------------------------------------------------
     */

    /***************************************************************************/
    /**
     * Converts the local data to a boost::property_tree node
     *
     * @param ptr The boost::property_tree object the data should be saved to
     * @param id The id assigned to this object
     */
    void toPropertyTree(
        pt::ptree &ptr
        , const std::string &baseName
    ) const override {
#ifdef DEBUG
        // Check that the object isn't empty
        if (this->empty()) {
            throw gemfony_exception(
                g_error_streamer(
                    DO_LOG
                    , time_and_place
                )
                    << "In GNumCollection<num_type>::toPropertyTree(): Error!" << std::endl
                    << "Object is empty!" << std::endl
            );
        }
#endif /* DEBUG */

        ptr.put(
            baseName + ".name"
            , this->getParameterName());
        ptr.put(
            baseName + ".type"
            , this->name());
        ptr.put(
            baseName + ".baseType"
            , Gem::Common::GTypeToStringT<num_type>::value());
        ptr.put(
            baseName + ".isLeaf"
            , this->isLeaf());
        ptr.put(
            baseName + ".nVals"
            , this->size());

        typename GNumCollectionT<num_type>::const_iterator cit;
        std::size_t pos;
        for (cit = this->begin(); cit != this->end(); ++cit) {
            pos = cit - this->begin();
            ptr.put(
                baseName + "values.value" + Gem::Common::to_string(pos)
                , *cit
            );
        }
        ptr.put(
            baseName + ".lowerBoundary"
            , this->getLowerInitBoundary());
        ptr.put(
            baseName + ".upperBoundary"
            , this->getUpperInitBoundary());
        ptr.put(
            baseName + ".initRandom"
            , false
        ); // Unused for the creation of a property tree
        ptr.put(
            baseName + ".adaptionsActive"
            , this->adaptionsActive());
    }

protected:
    /***************************************************************************/
    /**
     * Loads the data of another GNumCollectionT<num_type> object,
     * camouflaged as a GObject. We have no local data, so
     * all we need to do is to the standard identity check,
     * preventing that an object is assigned to itself.
     *
     * @param cp A copy of another GNumCollectionT<num_type> object, camouflaged as a GObject
     */
    void load_(const GObject *cp) override {
        // Check that we are dealing with a GNumCollectionT<num_type> reference independent of this object and convert the pointer
        const GNumCollectionT<num_type>
            *p_load = Gem::Common::g_convert_and_compare<GObject, GNumCollectionT<num_type>>(
            cp
            , this
        );

        // Load our parent class'es data ...
        GParameterCollectionT<num_type>::load_(cp);

        // ... and then our local data
        lowerInitBoundary_ = p_load->lowerInitBoundary_;
        upperInitBoundary_ = p_load->upperInitBoundary_;
    }

    /***************************************************************************/
    /** @brief Allow access to this classes compare_ function */
    friend void Gem::Common::compare_base_t<GNumCollectionT<num_type>>(
        GNumCollectionT<num_type> const &
        , GNumCollectionT<num_type> const &
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

        // Check that we are dealing with a GNumCollectionT<num_type> reference independent of this object and convert the pointer
        const GNumCollectionT<num_type>
            *p_load = Gem::Common::g_convert_and_compare<GObject, GNumCollectionT<num_type>>(
            cp
            , this
        );

        GToken token(
            "GNumCollectionT<num_type>"
            , e
        );

        // Compare our parent data ...
        Gem::Common::compare_base_t<GParameterCollectionT<num_type>>(
            *this
            , *p_load
            , token
        );

        // ... and then the local data
        compare_t(
            IDENTITY(lowerInitBoundary_
                     , p_load->lowerInitBoundary_)
            , token
        );
        compare_t(
            IDENTITY(upperInitBoundary_
                     , p_load->upperInitBoundary_)
            , token
        );

        // React on deviations from the expectation
        token.evaluate();
    }

    /***************************************************************************/
    /**
     * Returns a "comparative range". This is e.g. used to make Gauss-adaption
     * independent of a parameters value range
     */
    num_type range() const override {
        return upperInitBoundary_ - lowerInitBoundary_;
    }

    /***************************************************************************/
    /** @brief Triggers random initialization of the parameter collection */
    bool randomInit_(
        const activityMode &
        , Gem::Hap::GRandomBase &
    ) override = 0;

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
        if (GParameterCollectionT<num_type>::modify_GUnitTests_()) { result = true; }

        return result;

#else /* GEM_TESTING */  // If this function is called when GEM_TESTING isn't set, throw
        Gem::Common::condnotset("GNumCollectionT<>::modify_GUnitTests", "GEM_TESTING");
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
        GParameterCollectionT<num_type>::specificTestsNoFailureExpected_GUnitTests_();

        // A few settings
        const num_type
                LOWERTESTINITVAL = num_type(1); // Do not choose a negative value as num_type might be an unsigned type
        const num_type UPPERTESTINITVAL = num_type(3);

        //------------------------------------------------------------------------------

        { // Test setting and retrieval of initialization boundaries
            std::shared_ptr<GNumCollectionT<num_type>> p_test = this->template clone<GNumCollectionT<num_type>>();

            // Set the boundaries
            BOOST_CHECK_NO_THROW(p_test->setInitBoundaries(
                    LOWERTESTINITVAL
                    , UPPERTESTINITVAL
            ));

            // Check that these values have indeed been assigned
            BOOST_CHECK(p_test->getLowerInitBoundary() == LOWERTESTINITVAL);
            BOOST_CHECK(p_test->getUpperInitBoundary() == UPPERTESTINITVAL);
        }

        //------------------------------------------------------------------------------

#else /* GEM_TESTING */ // If this function is called when GEM_TESTING isn't set, throw
        Gem::Common::condnotset("GNumCollectionT<>::specificTestsNoFailureExpected_GUnitTests", "GEM_TESTING");
#endif /* GEM_TESTING */
    }

    /***************************************************************************/
    /**
     * Performs self tests that are expected to fail. This is needed for testing purposes
     */
    void specificTestsFailuresExpected_GUnitTests_() override {
#ifdef GEM_TESTING
        // Call the parent classes' functions
        GParameterCollectionT<num_type>::specificTestsFailuresExpected_GUnitTests_();

        // A few settings
        const num_type
                LOWERTESTINITVAL = num_type(1); // Do not choose a negative value as num_type might be an unsigned type
        const num_type UPPERTESTINITVAL = num_type(3);

        //------------------------------------------------------------------------------

        { // Check that assignement of initialization boundaries throws for invalid boundaries
            std::shared_ptr<GNumCollectionT<num_type>> p_test = this->template clone<GNumCollectionT<num_type>>();

            BOOST_CHECK_THROW(p_test->setInitBoundaries(
                    UPPERTESTINITVAL
                    , LOWERTESTINITVAL
            )
            , gemfony_exception);
        }

        //------------------------------------------------------------------------------

#else /* GEM_TESTING */ // If this function is called when GEM_TESTING isn't set, throw
        Gem::Common::condnotset("GNumCollectionT<>::specificTestsFailuresExpected_GUnitTests", "GEM_TESTING");
#endif /* GEM_TESTING */
    }

    /***************************************************************************/

private:
    /***************************************************************************/
    /**
     * Emits a name for this class / object
     */
    std::string name_() const override {
        return std::string("GNumCollectionT");
    }

    /***************************************************************************/
    /**
     * Creates a deep copy of this object. Purely virtual as this class
     * should not be instantiable.
     *
     * @return A pointer to a deep clone of this object
     */
    GObject *clone_() const override = 0;

    /***************************************************************************/
    num_type lowerInitBoundary_ = num_type(DEFAULTLOWERINITBOUNDARYCOLLECTION); ///< The lower boundary for random initialization
    num_type upperInitBoundary_ = num_type(DEFAULTUPPERINITBOUNDARYCOLLECTION); ///< The upper boundary for random initialization
};

/******************************************************************************/

} /* namespace Geneva */
} /* namespace Gem */

/******************************************************************************/
// The content of BOOST_SERIALIZATION_ASSUME_ABSTRACT(num_type)

namespace boost {
namespace serialization {
template<typename num_type>
struct is_abstract<Gem::Geneva::GNumCollectionT<num_type>> :
    public boost::true_type
{ /* nothing */ };
template<typename num_type>
struct is_abstract<const Gem::Geneva::GNumCollectionT<num_type>> :
    public boost::true_type
{ /* nothing */ };
}
}
/******************************************************************************/

