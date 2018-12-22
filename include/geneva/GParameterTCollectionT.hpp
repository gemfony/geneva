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
#include "common/GCommonHelperFunctionsT.hpp"
#include "common/GPtrVectorT.hpp"
#include "geneva/GParameterBase.hpp"
#include "geneva/GParameterT.hpp"

namespace Gem {
namespace Geneva {

/******************************************************************************/
/**
 * This class shares many similarities with the GParameterCollectionT class. Instead
 * of individual values that can be modified with adaptors, however, it assumes that
 * the objects stored in it have their own adapt() function. This class has been designed
 * as a collection of GParameterT objects, hence the name.  As an example, one can create a
 * collection of GConstrainedDoubleObject objects with this class rather than a simple GDoubleCollection.
 * In order to facilitate memory management, the GParameterT objects are stored
 * in std::shared_ptr objects.
 */
template<typename T>
class GParameterTCollectionT
    :
        public GParameterBase
        , public Gem::Common::GPtrVectorT<T, GObject>
{
    ///////////////////////////////////////////////////////////////////////
    friend class boost::serialization::access;

    template<typename Archive>
    void serialize(Archive &ar, const unsigned int) {
        using boost::serialization::make_nvp;

        // Save the data
        ar
        & make_nvp(
            "GParameterBase"
            , boost::serialization::base_object<GParameterBase>(*this))
        & make_nvp(
            "GStdPtrVectorInterfaceT_T"
            , boost::serialization::base_object<Gem::Common::GPtrVectorT<T, Gem::Geneva::GObject>>(*this));
    }
    ///////////////////////////////////////////////////////////////////////

    // Make sure T is a derivative of GParameterBase
    static_assert(
        std::is_base_of<GParameterBase, T>::value
        , "GParameterBase is not a base class of T"
    );

public:
    /***************************************************************************/
    /**
     * Allows to find out which type is stored in this class
     */
    using collection_type = T;

    /***************************************************************************/
    /**
     * The default constructor
     */
    GParameterTCollectionT() = default;

    /***************************************************************************/
    /**
     * Initialization with a number of copies of a given GParameterBase derivative
     *
     * @param nCp The amount of copies of the GParameterBase derivative to be stored in this object
     * @param tmpl_ptr The object that serves as the template of all others
     */
    GParameterTCollectionT(
        const std::size_t &nCp
        , std::shared_ptr<T> tmpl_ptr
    ) {
        for (std::size_t i = 0; i < nCp; i++) {
            this->push_back(tmpl_ptr->template clone<T>());
        }
    }

    /***************************************************************************/
    /**
     * The copy constructor
     *
     * @param cp A copy of another GParameterTCollectionT<T> object
     */
    GParameterTCollectionT(const GParameterTCollectionT<T> &cp) = default;

    /***************************************************************************/
    /**
     * The standard destructor
     */
    ~GParameterTCollectionT() override = default;

    /***************************************************************************/
    /**
     * Allows to identify whether we are dealing with a collection or an individual parameter
     * (which is obviously not the case here). This function needs to be overloaded for parameter
     * collections so that its inverse (GParameterBase::isParameterCollection() ) returns the
     * correct value.
     *
     * @return A boolean indicating whether this GParameterBase-derivative is an individual parameter
     */
    bool isIndividualParameter() const override {
        return false;
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
        // Check that the object isn't empty
        if (this->empty()) {
            throw gemfony_exception(
                g_error_streamer(
                    DO_LOG
                    , time_and_place
                )
                    << "In GParameterTCollectionT<T>::toPropertyTree(): Error!" << std::endl
                    << "Object is empty!" << std::endl
            );
        }

        ptr.put(
            baseName + ".name"
            , this->getParameterName());
        ptr.put(
            baseName + ".type"
            , this->name());
        ptr.put(
            baseName + ".isLeaf"
            , this->isLeaf());
        ptr.put(
            baseName + ".nVals"
            , this->size());

        // Loop over all parameter objects and ask them to add their
        // data to our ptree object
        std::string base;
        std::size_t pos;
        typename GParameterTCollectionT<T>::const_iterator cit;
        for (cit = this->begin(); cit != this->end(); ++cit) {
            pos = cit - this->begin();
            base = baseName + ".values.value" + Gem::Common::to_string(pos);
            (*cit)->toPropertyTree(
                ptr
                , base
            );
        }
    }

protected:
    /***************************************************************************/
    /**
     * Loads the data of another GParameterTCollectionT<T> object, camouflaged as a GObject
     *
     * @param cp A copy of another GParameterTCollectionT<T> object, camouflaged as a GObject
     */
    void load_(const GObject *cp) override {
        // Check that we are dealing with a GParameterTCollectionT<T> reference independent of this object and convert the pointer
        const GParameterTCollectionT<T>
            *p_load = Gem::Common::g_convert_and_compare<GObject, GParameterTCollectionT<T>>(
            cp
            , this
        );

        // Load our parent class'es data ...
        GParameterBase::load_(cp);
        Gem::Common::GPtrVectorT<T, Gem::Geneva::GObject>::operator=(*p_load);
    }

    /***************************************************************************/
    /** @brief Allow access to this classes compare_ function */
    friend void Gem::Common::compare_base_t<GParameterTCollectionT<T>>(
        GParameterTCollectionT<T> const &
        , GParameterTCollectionT<T> const &
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

        // Check that we are dealing with a GParameterTCollectionT<T> reference independent of this object and convert the pointer
        const GParameterTCollectionT<T>
            *p_load = Gem::Common::g_convert_and_compare<GObject, GParameterTCollectionT<T>>(
            cp
            , this
        );

        GToken token(
            "GParameterTCollectionT<T>"
            , e
        );

        // Compare our parent data ...
        Gem::Common::compare_base_t<GParameterBase>(
            *this
            , *p_load
            , token
        );

        // We treat GPtrVectorT<T, Gem::Geneva::GObject>::data as local data
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
     * This function distributes the random initialization to other objects
     */
    bool randomInit_(
        const activityMode &am
        , Gem::Hap::GRandomBase &gr
    ) override {
        bool randomized = false;

        typename GParameterTCollectionT<T>::iterator it;
        for (it = this->begin(); it != this->end(); ++it) {
            // Note that we do not call the randomInit_() function. First of all, we
            // do not have access to it. Secondly it might be that re-initialization of
            // a specific object is not desired.
            if ((*it)->GParameterBase::randomInit(
                am
                , gr
            )) {
                randomized = true;
            }
        }

        return randomized;
    }

    /***************************************************************************/
    /**
     * Attach parameters of type float to the vector. This function distributes this task to
     * objects contained in the container.
     *
     * @param parVec The vector to which the float parameters will be attached
     */
    void floatStreamline(
        std::vector<float> &parVec
        , const activityMode &am
    ) const override {
        typename GParameterTCollectionT<T>::const_iterator cit;
        for (cit = this->begin(); cit != this->end(); ++cit) {
            (*cit)->template streamline<float>(
                parVec
                , am
            );
        }
    }

    /* ----------------------------------------------------------------------------------
     * So far untested
     * ----------------------------------------------------------------------------------
     */

    /***************************************************************************/
    /**
     * Attach parameters of type double to the vector. This function distributes this task to
     * objects contained in the container.
     *
     * @param parVec The vector to which the double parameters will be attached
     */
    void doubleStreamline(
        std::vector<double> &parVec
        , const activityMode &am
    ) const override {
        typename GParameterTCollectionT<T>::const_iterator cit;
        for (cit = this->begin(); cit != this->end(); ++cit) {
            (*cit)->template streamline<double>(
                parVec
                , am
            );
        }
    }

    /* ----------------------------------------------------------------------------------
     * So far untested
     * ----------------------------------------------------------------------------------
     */

    /***************************************************************************/
    /**
     * Attach parameters of type std::int32_t to the vector. This function distributes this task
     * to objects contained in the container.
     *
     * @param parVec The vector to which the std::int32_t parameters will be attached
     */
    void int32Streamline(
        std::vector<std::int32_t> &parVec
        , const activityMode &am
    ) const override {
        typename GParameterTCollectionT<T>::const_iterator cit;
        for (cit = this->begin(); cit != this->end(); ++cit) {
            (*cit)->template streamline<std::int32_t>(
                parVec
                , am
            );
        }
    }

    /* ----------------------------------------------------------------------------------
     * So far untested
     * ----------------------------------------------------------------------------------
     */

    /***************************************************************************/
    /**
     * Attach parameters of type bool to the vector.  This function distributes this task
     * to objects contained in the container.
     *
     * @param parVec The vector to which the boolean parameters will be attached
     */
    void booleanStreamline(
        std::vector<bool> &parVec
        , const activityMode &am
    ) const override {
        typename GParameterTCollectionT<T>::const_iterator cit;
        for (cit = this->begin(); cit != this->end(); ++cit) {
            (*cit)->template streamline<bool>(
                parVec
                , am
            );
        }
    }

    /* ----------------------------------------------------------------------------------
     * So far untested
     * ----------------------------------------------------------------------------------
     */

    /***************************************************************************/
    /**
     * Attach parameters of type float to the map. This function distributes this task to
     * objects contained in the container.
     *
     * @param parVec The map to which the float parameters will be attached
     */
    void floatStreamline(
        std::map<std::string, std::vector<float>> &parVec
        , const activityMode &am
    ) const override {
        typename GParameterTCollectionT<T>::const_iterator cit;
        for (cit = this->begin(); cit != this->end(); ++cit) {
            (*cit)->template streamline<float>(
                parVec
                , am
            );
        }
    }

    /* ----------------------------------------------------------------------------------
     * So far untested
     * ----------------------------------------------------------------------------------
     */

    /***************************************************************************/
    /**
     * Attach parameters of type double to the map. This function distributes this task to
     * objects contained in the container.
     *
     * @param parVec The map to which the double parameters will be attached
     */
    void doubleStreamline(
        std::map<std::string, std::vector<double>> &parVec
        , const activityMode &am
    ) const override {
        typename GParameterTCollectionT<T>::const_iterator cit;
        for (cit = this->begin(); cit != this->end(); ++cit) {
            (*cit)->template streamline<double>(
                parVec
                , am
            );
        }
    }

    /* ----------------------------------------------------------------------------------
     * So far untested
     * ----------------------------------------------------------------------------------
     */

    /***************************************************************************/
    /**
     * Attach parameters of type std::int32_t to the map. This function distributes this task
     * to objects contained in the container.
     *
     * @param parVec The map to which the std::int32_t parameters will be attached
     */
    void int32Streamline(
        std::map<std::string, std::vector<std::int32_t>> &parVec
        , const activityMode &am
    ) const override {
        typename GParameterTCollectionT<T>::const_iterator cit;
        for (cit = this->begin(); cit != this->end(); ++cit) {
            (*cit)->template streamline<std::int32_t>(
                parVec
                , am
            );
        }
    }

    /* ----------------------------------------------------------------------------------
     * So far untested
     * ----------------------------------------------------------------------------------
     */

    /***************************************************************************/
    /**
     * Attach parameters of type bool to the map.  This function distributes this task
     * to objects contained in the container.
     *
     * @param parVec The map to which the boolean parameters will be attached
     */
    void booleanStreamline(
        std::map<std::string, std::vector<bool>> &parVec
        , const activityMode &am
    ) const override {
        typename GParameterTCollectionT<T>::const_iterator cit;
        for (cit = this->begin(); cit != this->end(); ++cit) {
            (*cit)->template streamline<bool>(
                parVec
                , am
            );
        }
    }

    /* ----------------------------------------------------------------------------------
     * So far untested
     * ----------------------------------------------------------------------------------
     */

    /***************************************************************************/
    /**
     * Attach boundaries of type float to the vectors
     *
     * @param lBndVec A vector of lower float parameter boundaries
     * @param uBndVec A vector of upper float parameter boundaries
     */
    void floatBoundaries(
        std::vector<float> &lBndVec
        , std::vector<float> &uBndVec
        , const activityMode &am
    ) const override {
        typename GParameterTCollectionT<T>::const_iterator cit;
        for (cit = this->begin(); cit != this->end(); ++cit) {
            (*cit)->template boundaries<float>(
                lBndVec
                , uBndVec
                , am
            );
        }
    }

    /***************************************************************************/
    /**
     * Attach boundaries of type double to the vectors
     *
     * @param lBndVec A vector of lower double parameter boundaries
     * @param uBndVec A vector of upper double parameter boundaries
     */
    void doubleBoundaries(
        std::vector<double> &lBndVec
        , std::vector<double> &uBndVec
        , const activityMode &am
    ) const override {
        typename GParameterTCollectionT<T>::const_iterator cit;
        for (cit = this->begin(); cit != this->end(); ++cit) {
            (*cit)->template boundaries<double>(
                lBndVec
                , uBndVec
                , am
            );
        }
    }

    /***************************************************************************/
    /**
     * Attach boundaries of type std::int32_t to the vectors
     *
     * @param lBndVec A vector of lower std::int32_t parameter boundaries
     * @param uBndVec A vector of upper std::int32_t parameter boundaries
     */
    void int32Boundaries(
        std::vector<std::int32_t> &lBndVec
        , std::vector<std::int32_t> &uBndVec
        , const activityMode &am
    ) const override {
        typename GParameterTCollectionT<T>::const_iterator cit;
        for (cit = this->begin(); cit != this->end(); ++cit) {
            (*cit)->template boundaries<std::int32_t>(
                lBndVec
                , uBndVec
                , am
            );
        }
    }

    /***************************************************************************/
    /**
     * Attach boundaries of type bool to the vectors. This function has been added for
     * completeness - at the very least it can give an indication of the number of boolean
     * parameters. Note, though, that there is a function that lets you count these parameters
     * directly.
     *
     * @param lBndVec A vector of lower bool parameter boundaries
     * @param uBndVec A vector of upper bool parameter boundaries
     */
    void booleanBoundaries(
        std::vector<bool> &lBndVec
        , std::vector<bool> &uBndVec
        , const activityMode &am
    ) const override {
        typename GParameterTCollectionT<T>::const_iterator cit;
        for (cit = this->begin(); cit != this->end(); ++cit) {
            (*cit)->template boundaries<bool>(
                lBndVec
                , uBndVec
                , am
            );
        }
    }

    /***************************************************************************/
    /**
     * Count the number of float parameters. This function returns the responses from all
     * objects contained in this collection.
     *
     * @param am An enum indicating whether only information about active, inactive or all parameters of this type should be extracted
     * @return The number of float parameters in this collection
     */
    std::size_t countFloatParameters(
        const activityMode &am
    ) const override {
        std::size_t result = 0;

        typename GParameterTCollectionT<T>::const_iterator cit;
        for (cit = this->begin(); cit != this->end(); ++cit) {
            result += (*cit)->template countParameters<float>(am);
        }

        return result;
    }

    /***************************************************************************/
    /**
     * Count the number of double parameters. This function returns the responses from all
     * objects contained in this collection.
     *
     * @param am An enum indicating whether only information about active, inactive or all parameters of this type should be extracted
     * @return The number of double parameters in this collection
     */
    std::size_t countDoubleParameters(
        const activityMode &am
    ) const override {
        std::size_t result = 0;

        typename GParameterTCollectionT<T>::const_iterator cit;
        for (cit = this->begin(); cit != this->end(); ++cit) {
            result += (*cit)->template countParameters<double>(am);
        }

        return result;
    }

    /***************************************************************************/
    /**
     * Count the number of std::int32_t parameters. This function returns the responses from all
     * objects contained in this collection.
     *
     * @param am An enum indicating whether only information about active, inactive or all parameters of this type should be extracted
     * @return The number of std::int32_t parameters in this collection
     */
    std::size_t countInt32Parameters(
        const activityMode &am
    ) const override {
        std::size_t result = 0;

        typename GParameterTCollectionT<T>::const_iterator cit;
        for (cit = this->begin(); cit != this->end(); ++cit) {
            result += (*cit)->template countParameters<std::int32_t>(am);
        }

        return result;
    }

    /***************************************************************************/
    /**
     * Count the number of bool parameters. This function returns the responses from all
     * objects contained in this collection.
     *
     * @param am An enum indicating whether only information about active, inactive or all parameters of this type should be extracted
     * @return The number of bool parameters in this collection
     */
    std::size_t countBoolParameters(
        const activityMode &am
    ) const override {
        std::size_t result = 0;

        typename GParameterTCollectionT<T>::const_iterator cit;
        for (cit = this->begin(); cit != this->end(); ++cit) {
            result += (*cit)->template countParameters<bool>(am);
        }

        return result;
    }

    /***************************************************************************/
    /**
     * Assigns part of a value vector to the parameter
     */
    void assignFloatValueVector(
        const std::vector<float> &parVec
        , std::size_t &pos
        , const activityMode &am
    ) override {
        typename GParameterTCollectionT<T>::iterator it;
        for (it = this->begin(); it != this->end(); ++it) {
            (*it)->template assignValueVector<float>(
                parVec
                , pos
                , am
            );
        }
    }

    /* ----------------------------------------------------------------------------------
     * So far untested
     * ----------------------------------------------------------------------------------
     */

    /***************************************************************************/
    /**
     * Assigns part of a value vector to the parameter
     */
    void assignDoubleValueVector(
        const std::vector<double> &parVec
        , std::size_t &pos
        , const activityMode &am
    ) override {
        typename GParameterTCollectionT<T>::iterator it;
        for (it = this->begin(); it != this->end(); ++it) {
            (*it)->template assignValueVector<double>(
                parVec
                , pos
                , am
            );
        }
    }

    /* ----------------------------------------------------------------------------------
     * So far untested
     * ----------------------------------------------------------------------------------
     */

    /***************************************************************************/
    /**
     * Assigns part of a value vector to the parameter
     */
    void assignInt32ValueVector(
        const std::vector<std::int32_t> &parVec
        , std::size_t &pos
        , const activityMode &am
    ) override {
        typename GParameterTCollectionT<T>::iterator it;
        for (it = this->begin(); it != this->end(); ++it) {
            (*it)->template assignValueVector<std::int32_t>(
                parVec
                , pos
                , am
            );
        }
    }

    /* ----------------------------------------------------------------------------------
     * So far untested
     * ----------------------------------------------------------------------------------
     */

    /***************************************************************************/
    /**
     * Assigns part of a value vector to the parameter
     */
    void assignBooleanValueVector(
        const std::vector<bool> &parVec
        , std::size_t &pos
        , const activityMode &am
    ) override {
        typename GParameterTCollectionT<T>::iterator it;
        for (it = this->begin(); it != this->end(); ++it) {
            (*it)->template assignValueVector<bool>(
                parVec
                , pos
                , am
            );
        }
    }

    /* ----------------------------------------------------------------------------------
     * So far untested
     * ----------------------------------------------------------------------------------
     */

    /***************************************************************************/
    /**
     * Assigns part of a value vector to the parameter
     */
    void assignFloatValueVectors(
        const std::map<std::string, std::vector<float>> &parMap
        , const activityMode &am
    ) override {
        typename GParameterTCollectionT<T>::iterator it;
        for (it = this->begin(); it != this->end(); ++it) {
            (*it)->template assignValueVectors<float>(
                parMap
                , am
            );
        }
    }

    /* ----------------------------------------------------------------------------------
     * So far untested
     * ----------------------------------------------------------------------------------
     */

    /***************************************************************************/
    /**
     * Assigns part of a value vector to the parameter
     */
    void assignDoubleValueVectors(
        const std::map<std::string, std::vector<double>> &parMap
        , const activityMode &am
    ) override {
        typename GParameterTCollectionT<T>::iterator it;
        for (it = this->begin(); it != this->end(); ++it) {
            (*it)->template assignValueVectors<double>(
                parMap
                , am
            );
        }
    }

    /* ----------------------------------------------------------------------------------
     * So far untested
     * ----------------------------------------------------------------------------------
     */

    /***************************************************************************/
    /**
     * Assigns part of a value vector to the parameter
     */
    void assignInt32ValueVectors(
        const std::map<std::string, std::vector<std::int32_t>> &parMap
        , const activityMode &am
    ) override {
        typename GParameterTCollectionT<T>::iterator it;
        for (it = this->begin(); it != this->end(); ++it) {
            (*it)->template assignValueVectors<std::int32_t>(
                parMap
                , am
            );
        }
    }

    /* ----------------------------------------------------------------------------------
     * So far untested
     * ----------------------------------------------------------------------------------
     */

    /***************************************************************************/
    /**
     * Assigns part of a value vector to the parameter
     */
    void assignBooleanValueVectors(
        const std::map<std::string, std::vector<bool>> &parMap
        , const activityMode &am
    ) override {
        typename GParameterTCollectionT<T>::iterator it;
        for (it = this->begin(); it != this->end(); ++it) {
            (*it)->template assignValueVectors<bool>(
                parMap
                , am
            );
        }
    }

    /* ----------------------------------------------------------------------------------
     * So far untested
     * ----------------------------------------------------------------------------------
     */

    /***************************************************************************/
    /**
     * Multiplication with a random value in a given range
     */
    void floatMultiplyByRandom(
        const float &min
        , const float &max
        , const activityMode &am
        , Gem::Hap::GRandomBase &gr
    ) override {
        typename GParameterTCollectionT<T>::iterator it;
        for (it = this->begin(); it != this->end(); ++it) {
            (*it)->template multiplyByRandom<float>(
                min
                , max
                , am
                , gr
            );
        }
    }

    /***************************************************************************/
    /**
     * Multiplication with a random value in a given range
     */
    void doubleMultiplyByRandom(
        const double &min
        , const double &max
        , const activityMode &am
        , Gem::Hap::GRandomBase &gr
    ) override {
        typename GParameterTCollectionT<T>::iterator it;
        for (it = this->begin(); it != this->end(); ++it) {
            (*it)->template multiplyByRandom<double>(
                min
                , max
                , am
                , gr
            );
        }
    }

    /***************************************************************************/
    /**
     * Multiplication with a random value in a given range
     */
    void int32MultiplyByRandom(
        const std::int32_t &min
        , const std::int32_t &max
        , const activityMode &am
        , Gem::Hap::GRandomBase &gr
    ) override {
        typename GParameterTCollectionT<T>::iterator it;
        for (it = this->begin(); it != this->end(); ++it) {
            (*it)->template multiplyByRandom<std::int32_t>(
                min
                , max
                , am
                , gr
            );
        }
    }

    /***************************************************************************/
    /**
     * Multiplication with a random value in the range [0,1[
     */
    void floatMultiplyByRandom(
        const activityMode &am
        , Gem::Hap::GRandomBase &gr
    ) override {
        typename GParameterTCollectionT<T>::iterator it;
        for (it = this->begin(); it != this->end(); ++it) {
            (*it)->template multiplyByRandom<float>(
                am
                , gr
            );
        }
    }

    /***************************************************************************/
    /**
     * Multiplication with a random value in the range [0,1[
     */
    void doubleMultiplyByRandom(
        const activityMode &am
        , Gem::Hap::GRandomBase &gr
    ) override {
        typename GParameterTCollectionT<T>::iterator it;
        for (it = this->begin(); it != this->end(); ++it) {
            (*it)->template multiplyByRandom<double>(
                am
                , gr
            );
        }
    }

    /***************************************************************************/
    /**
     * Multiplication with a random value in the range [0,1[
     */
    void int32MultiplyByRandom(
        const activityMode &am
        , Gem::Hap::GRandomBase &gr
    ) override {
        typename GParameterTCollectionT<T>::iterator it;
        for (it = this->begin(); it != this->end(); ++it) {
            (*it)->template multiplyByRandom<std::int32_t>(
                am
                , gr
            );
        }
    }

    /***************************************************************************/
    /**
     * Multiplication with a constant value
     */
    void floatMultiplyBy(
        const float &value
        , const activityMode &am
    ) override {
        typename GParameterTCollectionT<T>::iterator it;
        for (it = this->begin(); it != this->end(); ++it) {
            (*it)->template multiplyBy<float>(
                value
                , am
            );
        }
    }

    /***************************************************************************/
    /**
     * Multiplication with a constant value
     */
    void doubleMultiplyBy(
        const double &value
        , const activityMode &am
    ) override {
        typename GParameterTCollectionT<T>::iterator it;
        for (it = this->begin(); it != this->end(); ++it) {
            (*it)->template multiplyBy<double>(
                value
                , am
            );
        }
    }

    /***************************************************************************/
    /**
     * Multiplication with a constant value
     */
    void int32MultiplyBy(
        const std::int32_t &value
        , const activityMode &am
    ) override {
        typename GParameterTCollectionT<T>::iterator it;
        for (it = this->begin(); it != this->end(); ++it) {
            (*it)->template multiplyBy<std::int32_t>(
                value
                , am
            );
        }
    }

    /***************************************************************************/
    /**
     * Initialization with a constant value
     */
    void floatFixedValueInit(
        const float &value
        , const activityMode &am
    ) override {
        typename GParameterTCollectionT<T>::iterator it;
        for (it = this->begin(); it != this->end(); ++it) {
            (*it)->template fixedValueInit<float>(
                value
                , am
            );
        }
    }

    /***************************************************************************/
    /**
     * Initialization with a constant value
     */
    void doubleFixedValueInit(
        const double &value
        , const activityMode &am
    ) override {
        typename GParameterTCollectionT<T>::iterator it;
        for (it = this->begin(); it != this->end(); ++it) {
            (*it)->template fixedValueInit<double>(
                value
                , am
            );
        }
    }

    /***************************************************************************/
    /**
     * Initialization with a constant value
     */
    void int32FixedValueInit(
        const std::int32_t &value
        , const activityMode &am
    ) override {
        typename GParameterTCollectionT<T>::iterator it;
        for (it = this->begin(); it != this->end(); ++it) {
            (*it)->template fixedValueInit<std::int32_t>(
                value
                , am
            );
        }
    }

    /***************************************************************************/
    /**
     * Initialization with a constant value
     */
    void booleanFixedValueInit(
        const bool &value
        , const activityMode &am
    ) override {
        typename GParameterTCollectionT<T>::iterator it;
        for (it = this->begin(); it != this->end(); ++it) {
            (*it)->template fixedValueInit<bool>(
                value
                , am
            );
        }
    }

    /***************************************************************************/
    /**
     * Adds the "same-type" parameters of another GParameterBase object to this one
     */
    void floatAdd(
        std::shared_ptr<GParameterBase> p_base
        , const activityMode &am
    ) override {
        // We first need to convert p_base into the local type
        std::shared_ptr<GParameterTCollectionT<T>> p
            = GParameterBase::parameterbase_cast<GParameterTCollectionT<T>>(p_base);

        // Check that both collections have the same size
        if (this->size() != p->size()) {
            throw gemfony_exception(
                g_error_streamer(
                    DO_LOG
                    , time_and_place
                )
                    << "In GParameterTCollectionT<T>::floatAdd(): Error!" << std::endl
                    << "Collections have a different size: " << this->size() << " / " << p->size() << std::endl
            );
        }

        typename GParameterTCollectionT<T>::iterator it;
        typename GParameterTCollectionT<T>::iterator p_it;
        for (it = this->begin(), p_it = p->begin(); it != this->end(); ++it, ++p_it) {
            (*it)->template add<float>(
                *p_it
                , am
            );
        }
    }

    /***************************************************************************/
    /**
     * Adds the "same-type" parameters of another GParameterBase object to this one
     */
    void doubleAdd(
        std::shared_ptr<GParameterBase> p_base
        , const activityMode &am
    ) override {
        // We first need to convert p_base into the local type
        std::shared_ptr<GParameterTCollectionT<T>> p
            = GParameterBase::parameterbase_cast<GParameterTCollectionT<T>>(p_base);

        // Check that both collections have the same size
        if (this->size() != p->size()) {
            throw gemfony_exception(
                g_error_streamer(
                    DO_LOG
                    , time_and_place
                )
                    << "In GParameterTCollectionT<T>::doubleAdd(): Error!" << std::endl
                    << "Collections have a different size: " << this->size() << " / " << p->size() << std::endl
            );
        }

        typename GParameterTCollectionT<T>::iterator it;
        typename GParameterTCollectionT<T>::iterator p_it;
        for (it = this->begin(), p_it = p->begin(); it != this->end(); ++it, ++p_it) {
            (*it)->template add<double>(
                *p_it
                , am
            );
        }
    }

    /***************************************************************************/
    /**
     * Adds the "same-type" parameters of another GParameterBase object to this one
     */
    void int32Add(
        std::shared_ptr<GParameterBase> p_base
        , const activityMode &am
    ) override {
        // We first need to convert p_base into the local type
        std::shared_ptr<GParameterTCollectionT<T>> p
            = GParameterBase::parameterbase_cast<GParameterTCollectionT<T>>(p_base);

        // Check that both collections have the same size
        if (this->size() != p->size()) {
            throw gemfony_exception(
                g_error_streamer(
                    DO_LOG
                    , time_and_place
                )
                    << "In GParameterTCollectionT<T>::int32Add(): Error!" << std::endl
                    << "Collections have a different size: " << this->size() << " / " << p->size() << std::endl
            );
        }

        typename GParameterTCollectionT<T>::iterator it;
        typename GParameterTCollectionT<T>::iterator p_it;
        for (it = this->begin(), p_it = p->begin(); it != this->end(); ++it, ++p_it) {
            (*it)->template add<std::int32_t>(
                *p_it
                , am
            );
        }
    }

    /***************************************************************************/
    /**
     * Subtracts the "same-type" parameters of another GParameterBase object from this one
     */
    void floatSubtract(
        std::shared_ptr<GParameterBase> p_base
        , const activityMode &am
    ) override {
        // We first need to convert p_base into the local type
        std::shared_ptr<GParameterTCollectionT<T>> p
            = GParameterBase::parameterbase_cast<GParameterTCollectionT<T>>(p_base);

        // Check that both collections have the same size
        if (this->size() != p->size()) {
            throw gemfony_exception(
                g_error_streamer(
                    DO_LOG
                    , time_and_place
                )
                    << "In GParameterTCollectionT<T>::floatSubtract(): Error!" << std::endl
                    << "Collections have a different size: " << this->size() << " / " << p->size() << std::endl
            );
        }

        typename GParameterTCollectionT<T>::iterator it;
        typename GParameterTCollectionT<T>::iterator p_it;
        for (it = this->begin(), p_it = p->begin(); it != this->end(); ++it, ++p_it) {
            (*it)->template subtract<float>(
                *p_it
                , am
            );
        }
    }

    /***************************************************************************/
    /**
     * Subtracts the "same-type" parameters of another GParameterBase object from this one
     */
    void doubleSubtract(
        std::shared_ptr<GParameterBase> p_base
        , const activityMode &am
    ) override {
        // We first need to convert p_base into the local type
        std::shared_ptr<GParameterTCollectionT<T>> p
            = GParameterBase::parameterbase_cast<GParameterTCollectionT<T>>(p_base);

        // Check that both collections have the same size
        if (this->size() != p->size()) {
            throw gemfony_exception(
                g_error_streamer(
                    DO_LOG
                    , time_and_place
                )
                    << "In GParameterTCollectionT<T>::doubleSubtract(): Error!" << std::endl
                    << "Collections have a different size: " << this->size() << " / " << p->size() << std::endl
            );
        }

        typename GParameterTCollectionT<T>::iterator it;
        typename GParameterTCollectionT<T>::iterator p_it;
        for (it = this->begin(), p_it = p->begin(); it != this->end(); ++it, ++p_it) {
            (*it)->template subtract<double>(
                *p_it
                , am
            );
        }
    }

    /***************************************************************************/
    /**
     * Subtracts the "same-type" parameters of another GParameterBase object from this one
     */
    void int32Subtract(
        std::shared_ptr<GParameterBase> p_base
        , const activityMode &am
    ) override {
        // We first need to convert p_base into the local type
        std::shared_ptr<GParameterTCollectionT<T>> p
            = GParameterBase::parameterbase_cast<GParameterTCollectionT<T>>(p_base);

        // Check that both collections have the same size
        if (this->size() != p->size()) {
            throw gemfony_exception(
                g_error_streamer(
                    DO_LOG
                    , time_and_place
                )
                    << "In GParameterTCollectionT<T>::int32Subtract(): Error!" << std::endl
                    << "Collections have a different size: " << this->size() << " / " << p->size() << std::endl
            );
        }

        typename GParameterTCollectionT<T>::iterator it;
        typename GParameterTCollectionT<T>::iterator p_it;
        for (it = this->begin(), p_it = p->begin(); it != this->end(); ++it, ++p_it) {
            (*it)->template subtract<std::int32_t>(
                *p_it
                , am
            );
        }
    }

private:
    /***************************************************************************/
    /**
     * Emits a name for this class / object
     */
    std::string name_() const override {
        return std::string("GParameterTCollectionT");
    }

    /***************************************************************************/
    /**
     * Creates a deep clone of this object. Declared purely virtual, as this class is not
     * intended to be used directly.
     */
    GObject *clone_() const override = 0;

    /***************************************************************************/
    /**
     * Allows to adapt the values stored in this class. We assume here that
     * each item has its own adapt function. Hence we do not need to use or
     * store own adaptors.
     *
     * @return The number of adaptions that were carried out
     */
    std::size_t adapt_(Gem::Hap::GRandomBase &gr) override {
        std::size_t nAdapted = 0;

        for (auto const & par_ptr : *this) {
            nAdapted += par_ptr->adapt(gr);
        }

        return nAdapted;
    }

    /***************************************************************************/
    /**
     * Triggers updates when the optimization process has stalled
     */
    bool updateAdaptorsOnStall_(std::size_t nStalls) override {
        bool updatePerformed = false;

        for (auto const & par_ptr : *this) {
            if (par_ptr->updateAdaptorsOnStall(nStalls)) {
                updatePerformed = true;
            }
        }

        return updatePerformed;
    }

    /******************************************************************************/
    /**
     * Retrieves information from adaptors with a given property
     *
     * @param adaoptorName The name of the adaptor to be queried
     * @param property The property for which information is sought
     * @param data A vector, to which the properties should be added
     */
    void queryAdaptor_(
            const std::string &adaptorName
            , const std::string &property
            , std::vector<boost::any> &data
    ) const override {
        for(auto const& par_ptr: *this) {
            par_ptr->queryAdaptor(
                adaptorName
                , property
                , data
            );
        }
    }

public:
    /***************************************************************************/
    /**
     * Applies modifications to this object. This is needed for testing purposes
     *
     * @return A boolean which indicates whether modifications were made
     */
    bool modify_GUnitTests() override {
#ifdef GEM_TESTING
        bool result = false;

        // Call the parent classes' functions
        if (GParameterBase::modify_GUnitTests()) { result = true; }
        if (Gem::Common::GPtrVectorT<T, Gem::Geneva::GObject>::modify_GUnitTests()) { result = true; }

        return result;

#else /* GEM_TESTING */  // If this function is called when GEM_TESTING isn't set, throw
        Gem::Common::condnotset("GBrokerEA::modify_GUnitTests", "GEM_TESTING");
       return false;
#endif /* GEM_TESTING */
    }

    /***************************************************************************/
    /**
     * Performs self tests that are expected to succeed. This is needed for testing purposes
     */
    void specificTestsNoFailureExpected_GUnitTests() override {
#ifdef GEM_TESTING
        // Call the parent classes' functions
        GParameterBase::specificTestsNoFailureExpected_GUnitTests();
        Gem::Common::GPtrVectorT<T, Gem::Geneva::GObject>::specificTestsNoFailureExpected_GUnitTests();

        //------------------------------------------------------------------------------

#else /* GEM_TESTING */  // If this function is called when GEM_TESTING isn't set, throw
        Gem::Common::condnotset("GBrokerEA::specificTestsNoFailureExpected_GUnitTests", "GEM_TESTING");
#endif /* GEM_TESTING */
    }

    /***************************************************************************/
    /**
     * Performs self tests that are expected to fail. This is needed for testing purposes
     */
    void specificTestsFailuresExpected_GUnitTests() override {
#ifdef GEM_TESTING
        // Call the parent classes' functions
        GParameterBase::specificTestsFailuresExpected_GUnitTests();
        Gem::Common::GPtrVectorT<T, Gem::Geneva::GObject>::specificTestsFailuresExpected_GUnitTests();

        //------------------------------------------------------------------------------

        { // Some test

        }

        //------------------------------------------------------------------------------

#else /* GEM_TESTING */  // If this function is called when GEM_TESTING isn't set, throw
        Gem::Common::condnotset("GBrokerEA::specificTestsFailuresExpected_GUnitTests", "GEM_TESTING");
#endif /* GEM_TESTING */
    }

};

/******************************************************************************/

} /* namespace Geneva */
} /* namespace Gem */

/******************************************************************************/
/** @brief Mark this class as abstract. This is the content of
 * BOOST_SERIALIZATION_ASSUME_ABSTRACT(T) */

namespace boost {
namespace serialization {
template<typename T>
struct is_abstract<Gem::Geneva::GParameterTCollectionT<T>> :
    public boost::true_type
{
};
template<typename T>
struct is_abstract<const Gem::Geneva::GParameterTCollectionT<T>> :
    public boost::true_type
{
};
}
}

/******************************************************************************/

