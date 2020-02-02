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
#include "common/GTypeToStringT.hpp"
#include "common/GPODVectorT.hpp"
#include "geneva/GObject.hpp"
#include "geneva/GParameterBaseWithAdaptorsT.hpp"

namespace Gem {
namespace Geneva {

/******************************************************************************/
/**
 * A class holding a collection of mutable parameters - usually just an atomic value (double,
 * long, bool, ...).
 */
template<typename num_type>
class GParameterCollectionT
    : public GParameterBaseWithAdaptorsT<num_type>
    , public Gem::Common::GPODVectorT<num_type>
{
    ///////////////////////////////////////////////////////////////////////
    friend class boost::serialization::access;

    template<typename Archive>
    void serialize(Archive &ar, const unsigned int) {
        using boost::serialization::make_nvp;
        ar
        & make_nvp(
            "GParameterBaseWithAdaptorsT_num_type"
            , boost::serialization::base_object<GParameterBaseWithAdaptorsT<num_type>>(*this))
        & make_nvp(
            "GStdSimpleVectorInterfaceT_num_type"
            , boost::serialization::base_object<Gem::Common::GPODVectorT<num_type>>(*this));
    }
    ///////////////////////////////////////////////////////////////////////

    // Make sure this class can only be instantiated with num_type as an arithmetic type
    static_assert(
        std::is_arithmetic<num_type>::value
        , "num_type should be an arithmetic type"
    );

public:
    /***************************************************************************/
    /**
     * The default constructor
     */
    GParameterCollectionT() = default;

    /***************************************************************************/
    /**
     * Initialization with a number of variables of predefined values
     *
     * @param nval The number of values
     * @param val  The value to be assigned to each position
     */
    GParameterCollectionT(
        const std::size_t &nval
        , const num_type &val
    )
        :
        GParameterBaseWithAdaptorsT<num_type>()
        , Gem::Common::GPODVectorT<num_type>(
        nval
        , val
    ) { /* nothing */ }

    /***************************************************************************/
    /**
     * The copy constructor
     *
     * @param cp A copy of another GParameterCollectionT<num_type> object
     */
    GParameterCollectionT(const GParameterCollectionT<num_type> &cp) = default;

    /***************************************************************************/
    /**
     * The standard destructor
     */
    ~GParameterCollectionT() override = default;

    /***************************************************************************/
    /**
     * Swap another object's vector with ours
     */
    void swap(GParameterCollectionT<num_type> &cp) {
        Gem::Common::GPODVectorT<num_type>::swap(cp.m_data_cnt);
    }

    /* ----------------------------------------------------------------------------------
     * Tested in GDoubleObject::specificTestsNoFailureExpected_GUnitTests()
     * ----------------------------------------------------------------------------------
     */

    /***************************************************************************/
    /**
     * Retrieval of the value at a given position
     *
     * @param pos The position for which the value needs to be returned
     * @return The value of val_
     */
    virtual num_type value(const std::size_t &pos) BASE {
        return this->at(pos);
    }

    /***************************************************************************/
    /**
     * Allows to set the internal (and usually externally visible) value at a given position. Note
     * that we assume here that num_type has an operator=() or is a basic value type, such as double
     * or int.
     *
     * @param pos The position at which the value shout be stored
     * @param val The new num_type value stored in this class
     */
    virtual void setValue(const std::size_t &pos, const num_type &val) BASE {
        this->at(pos) = val;

#ifdef DEBUG
        assert(this->at(pos) == val);
#endif
    }

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
                    << "In GParameterCollectionT<num_type>::toPropertyTree(): Error!" << std::endl
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

        typename GParameterCollectionT<num_type>::const_iterator cit;
        std::size_t pos;
        for (cit = this->begin(); cit != this->end(); ++cit) {
            pos = std::distance(
                this->begin()
                , cit
            );
            ptr.put(
                baseName + "values.value" + Gem::Common::to_string(pos)
                , *cit
            );
        }
        ptr.put(
            baseName + ".initRandom"
            , false
        ); // Unused for the creation of a property tree
        ptr.put(
            baseName + ".adaptionsActive"
            , this->adaptionsActive());
    }

    /***************************************************************************/
    /**
     * Lets the audience know whether this is a leaf or a branch object
     */
    bool isLeaf() const override {
        return true;
    }

protected:
    /***************************************************************************/
    /**
     * Loads the data of another GParameterCollectionT<num_type> object, camouflaged as a GObject
     *
     * @param cp A copy of another GParameterCollectionT<num_type> object, camouflaged as a GObject
     */
    void load_(const GObject *cp) override {
        // Check that we are dealing with a  GParameterCollectionT<num_type> reference independent of this object and convert the pointer
        const GParameterCollectionT<num_type>
            *p_load = Gem::Common::g_convert_and_compare<GObject, GParameterCollectionT<num_type>>(
            cp
            , this
        );

        // Load our parent class'es data ...
        GParameterBaseWithAdaptorsT<num_type>::load_(cp);
        Gem::Common::GPODVectorT<num_type>::operator=(*p_load);
    }

    /** @brief Allow access to this classes compare_ function */
    friend void Gem::Common::compare_base_t<GParameterCollectionT<num_type>>(
        GParameterCollectionT<num_type> const &
        , GParameterCollectionT<num_type> const &
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

        // Check that we are dealing with a  GParameterCollectionT<num_type> reference independent of this object and convert the pointer
        const GParameterCollectionT<num_type>
            *p_load = Gem::Common::g_convert_and_compare<GObject, GParameterCollectionT<num_type>>(
            cp
            , this
        );

        GToken token(
            "GParameterCollectionT<num_type>"
            , e
        );

        // Compare our parent data ...
        Gem::Common::compare_base_t<GParameterBaseWithAdaptorsT<num_type>>(
            *this
            , *p_load
            , token
        );

        // We access the relevant data of one of the parent classes directly for simplicity reasons
        compare_t(
            IDENTITY(this->m_data_cnt
                     , p_load->m_data_cnt)
            , token
        );

        // React on deviations from the expectation
        token.evaluate();
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
        if (GParameterBaseWithAdaptorsT<num_type>::modify_GUnitTests_()) { result = true; }
        if (Gem::Common::GPODVectorT<num_type>::modify_GUnitTests_()) { result = true; }

        return result;

#else /* GEM_TESTING */  // If this function is called when GEM_TESTING isn't set, throw
        Gem::Common::condnotset("GParameterCollectionT<>::modify_GUnitTests", "GEM_TESTING");
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
        GParameterBaseWithAdaptorsT<num_type>::specificTestsNoFailureExpected_GUnitTests_();
        Gem::Common::GPODVectorT<num_type>::specificTestsNoFailureExpected_GUnitTests_();

#else /* GEM_TESTING */  // If this function is called when GEM_TESTING isn't set, throw
        Gem::Common::condnotset("GParameterCollectionT<>::specificTestsNoFailureExpected_GUnitTests", "GEM_TESTING");
#endif /* GEM_TESTING */
    }

    /***************************************************************************/
    /**
     * Performs self tests that are expected to fail. This is needed for testing purposes
     */
    void specificTestsFailuresExpected_GUnitTests_() override {
#ifdef GEM_TESTING
        // Call the parent classes' functions
        GParameterBaseWithAdaptorsT<num_type>::specificTestsFailuresExpected_GUnitTests_();
        Gem::Common::GPODVectorT<num_type>::specificTestsFailuresExpected_GUnitTests_();


#else /* GEM_TESTING */  // If this function is called when GEM_TESTING isn't set, throw
        Gem::Common::condnotset("GParameterCollectionT<>::specificTestsFailuresExpected_GUnitTests", "GEM_TESTING");
#endif /* GEM_TESTING */
    }

    /******************************************************************************/

private:
    /***************************************************************************/
    /**
     * Emits a name for this class / object
     */
    std::string name_() const override {
        return std::string("GParameterCollectionT");
    }

    /***************************************************************************/
    /**
     * Creates a deep clone of this object. Purely virtual, so this class cannot be instantiated.
     */
    GObject *clone_() const override = 0;

    /***************************************************************************/
    /**
     * Allows to adapt the values stored in this class. applyAdaptor expects a reference
     * to a std::vector<num_type>. As we are derived from a wrapper of this class, we can just pass
     * a reference to its data vector to the function.
     *
     * @return The number of adaptions that were carried out
     */
    std::size_t adapt_(
            Gem::Hap::GRandomBase &gr
    ) override {
        return GParameterBaseWithAdaptorsT<num_type>::applyAdaptor(
                Gem::Common::GPODVectorT<num_type>::m_data_cnt
                , this->range()
                , gr
        );
    }

    /***************************************************************************/
    /**
     * Allows to identify whether we are dealing with a collection or an individual parameter
     * (which is obviously not the case here). This function needs to be overloaded for parameter
     * collections so that its inverse (GParameterBase::isParameterCollection() ) returns the
     * correct value.
     *
     * @return A boolean indicating whether this GParameterBase-derivative is an individual parameter
     */
    bool isIndividualParameter_() const override {
        return false;
    }
};

} /* namespace Geneva */
} /* namespace Gem */

/******************************************************************************/
/**
 * @brief The content of the BOOST_SERIALIZATION_ASSUME_ABSTRACT(num_type) macro. Needed for Boost.Serialization
 */
namespace boost {
namespace serialization {
template<typename num_type>
struct is_abstract<Gem::Geneva::GParameterCollectionT<num_type>> :
    public boost::true_type
{
};
template<typename num_type>
struct is_abstract<const Gem::Geneva::GParameterCollectionT<num_type>> :
    public boost::true_type
{
};
}
}

/******************************************************************************/

