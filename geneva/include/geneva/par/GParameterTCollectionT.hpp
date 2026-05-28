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

// Standard header files go here
#include <any>
#include <type_traits>

// Boost header files go here

// Geneva header files go here
#include "common/GCommonHelperFunctionsT.hpp"
#include "common/GContainerT.hpp"
#include "geneva/par/GParameterBase.hpp"
#include "geneva/par/GParameterT.hpp"

namespace Gem::Geneva::Parameters {

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
template <typename T>
class GParameterTCollectionT // NOLINT(cppcoreguidelines-special-member-functions)
  : public GParameterBase
  , public Gem::Common::GPtrContainerT<T> {
    ///////////////////////////////////////////////////////////////////////
    friend class boost::serialization::access;

    template <typename Archive>
    void serialize(Archive &ar, const unsigned int) {
        using boost::serialization::make_nvp;

        // Save the data
        ar &make_nvp("GParameterBase", boost::serialization::base_object<GParameterBase>(*this)) &
            make_nvp(
                "GStdPtrVectorInterfaceT_T",
                boost::serialization::base_object<Gem::Common::GPtrContainerT<T>>(*this)
            );
    }
    ///////////////////////////////////////////////////////////////////////

    // Make sure T is a derivative of GParameterBase
    static_assert(
        std::is_base_of_v<GParameterBase, T>,
        "GParameterBase is not a base class of T"
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
     * @param n_cp The amount of copies of the GParameterBase derivative to be stored in this object
     * @param tmpl_ptr The object that serves as the template of all others
     */
    GParameterTCollectionT(const std::size_t &n_cp, std::shared_ptr<T> tmpl_ptr) {
        for(std::size_t i = 0; i < n_cp; i++) {
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
     * Converts the local data to a boost::property_tree node
     *
     * @param ptr The boost::property_tree object the data should be saved to
     * @param base_name The id assigned to this object
     */
    void toPropertyTree(pt::ptree &ptr, const std::string &base_name) const override {
        // Check that the object isn't empty
        if(this->empty()) {
            throw geneva_exception(
                g_error_streamer(DO_LOG, Gem::Common::timeAndPlace())
                << "In GParameterTCollectionT<T>::toPropertyTree(): Error!" << '\n'
                << "Object is empty!" << '\n'
            );
        }

        ptr.put(base_name + ".name", this->getParameterName());
        ptr.put(base_name + ".type", this->name());
        ptr.put(base_name + ".isLeaf", this->isLeaf());
        ptr.put(base_name + ".n_vals", this->size());

        // Loop over all parameter objects and ask them to add their
        // data to our ptree object
        std::string base; // NOLINT(cppcoreguidelines-init-variables)
        std::size_t pos = 0;
        typename GParameterTCollectionT<T>::const_iterator cit;
        for(cit = this->begin(); cit != this->end(); ++cit) {
            pos = cit - this->begin();
            base = base_name + ".values.value" + Gem::Common::to_string(pos);
            (*cit)->toPropertyTree(ptr, base);
        }
    }

protected:
    /***************************************************************************/
    /**
     * Loads the data of another GParameterTCollectionT<T> object, camouflaged as a GParameterBase
     *
     * @param cp A copy of another GParameterTCollectionT<T> object, camouflaged as a GParameterBase
     */
    void load_(const GParameterBase *cp) override {
        // Check that we are dealing with a GParameterTCollectionT<T> reference independent of this object and convert the pointer
        const GParameterTCollectionT<T> *p_load =
            Gem::Common::g_convert_and_compare<GParameterBase, GParameterTCollectionT<T>>(cp, this);

        // Load our parent class'es data ...
        GParameterBase::load_(cp);
        Gem::Common::GPtrContainerT<T>::operator=(*p_load);
    }

    /***************************************************************************/
    /** @brief Allow access to this classes compare_ function */
    friend void Gem::Common::compare_base_t<GParameterTCollectionT<T>>(
        GParameterTCollectionT<T> const &,
        GParameterTCollectionT<T> const &,
        Gem::Common::GToken &
    );

    /***************************************************************************/
    /**
     * Searches for compliance with expectations with respect to another object
     * of the same type
     *
     * @param cp A constant reference to another GParameterBase object
     * @param e The expected outcome of the comparison
     */
    void compare_(
        const GParameterBase &cp,
        const Gem::Common::expectation &e,
        const double & /*limit*/
    ) const override {
        using namespace Gem::Common;

        // Check that we are dealing with a GParameterTCollectionT<T> reference independent of this object and convert the pointer
        const GParameterTCollectionT<T> *p_load =
            Gem::Common::g_convert_and_compare<GParameterBase, GParameterTCollectionT<T>>(cp, this);

        GToken token("GParameterTCollectionT<T>", e);

        // Compare our parent data ...
        Gem::Common::compare_base_t<GParameterBase>(*this, *p_load, token);

        // We treat GPtrContainerT<T>::data as local data
        compare_t(IDENTITY(this->data_cnt_, p_load->data_cnt_), token);

        // React on deviations from the expectation
        token.evaluate();
    }

    /***************************************************************************/
    /**
     * This function distributes the random initialization to other objects
     */
    bool randomInit_(const activityMode &am, Gem::Hap::GRandomBase &gr) override {
        bool randomized = false;

        typename GParameterTCollectionT<T>::iterator it;
        for(it = this->begin(); it != this->end(); ++it) {
            // Note that we do not call the randomInit_() function. First of all, we
            // do not have access to it. Secondly it might be that re-initialization of
            // a specific object is not desired.
            if((*it)->GParameterBase::randomInit(am, gr)) {
                randomized = true;
            }
        }

        return randomized;
    }

    /***************************************************************************/
    /**
     * Attach parameters of type float to the vector. This function distributes this task to
     * objects contained in the container.
     */
    void floatStreamline(std::vector<float> &par_vec, const activityMode &am) const override {
        typename GParameterTCollectionT<T>::const_iterator cit;
        for(cit = this->begin(); cit != this->end(); ++cit) {
            (*cit)->template streamline<float>(par_vec, am);
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
     */
    void doubleStreamline(std::vector<double> &par_vec, const activityMode &am) const override {
        typename GParameterTCollectionT<T>::const_iterator cit;
        for(cit = this->begin(); cit != this->end(); ++cit) {
            (*cit)->template streamline<double>(par_vec, am);
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
     */
    void
    int32Streamline(std::vector<std::int32_t> &par_vec, const activityMode &am) const override {
        typename GParameterTCollectionT<T>::const_iterator cit;
        for(cit = this->begin(); cit != this->end(); ++cit) {
            (*cit)->template streamline<std::int32_t>(par_vec, am);
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
     */
    void booleanStreamline(std::vector<bool> &par_vec, const activityMode &am) const override {
        typename GParameterTCollectionT<T>::const_iterator cit;
        for(cit = this->begin(); cit != this->end(); ++cit) {
            (*cit)->template streamline<bool>(par_vec, am);
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
     */
    void floatStreamline(
        std::map<std::string, std::vector<float>> &par_vec,
        const activityMode &am
    ) const override {
        typename GParameterTCollectionT<T>::const_iterator cit;
        for(cit = this->begin(); cit != this->end(); ++cit) {
            (*cit)->template streamline<float>(par_vec, am);
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
     */
    void doubleStreamline(
        std::map<std::string, std::vector<double>> &par_vec,
        const activityMode &am
    ) const override {
        typename GParameterTCollectionT<T>::const_iterator cit;
        for(cit = this->begin(); cit != this->end(); ++cit) {
            (*cit)->template streamline<double>(par_vec, am);
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
     */
    void int32Streamline(
        std::map<std::string, std::vector<std::int32_t>> &par_vec,
        const activityMode &am
    ) const override {
        typename GParameterTCollectionT<T>::const_iterator cit;
        for(cit = this->begin(); cit != this->end(); ++cit) {
            (*cit)->template streamline<std::int32_t>(par_vec, am);
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
     */
    void booleanStreamline(
        std::map<std::string, std::vector<bool>> &par_vec,
        const activityMode &am
    ) const override {
        typename GParameterTCollectionT<T>::const_iterator cit;
        for(cit = this->begin(); cit != this->end(); ++cit) {
            (*cit)->template streamline<bool>(par_vec, am);
        }
    }

    /* ----------------------------------------------------------------------------------
     * So far untested
     * ----------------------------------------------------------------------------------
     */

    /***************************************************************************/
    /**
     * Attach boundaries of type float to the vectors
     */
    void floatBoundaries(
        std::vector<float> &l_bnd_vec,
        std::vector<float> &u_bnd_vec,
        const activityMode &am
    ) const override {
        typename GParameterTCollectionT<T>::const_iterator cit;
        for(cit = this->begin(); cit != this->end(); ++cit) {
            (*cit)->template boundaries<float>(l_bnd_vec, u_bnd_vec, am);
        }
    }

    /***************************************************************************/
    /**
     * Attach boundaries of type double to the vectors
     */
    void doubleBoundaries(
        std::vector<double> &l_bnd_vec,
        std::vector<double> &u_bnd_vec,
        const activityMode &am
    ) const override {
        typename GParameterTCollectionT<T>::const_iterator cit;
        for(cit = this->begin(); cit != this->end(); ++cit) {
            (*cit)->template boundaries<double>(l_bnd_vec, u_bnd_vec, am);
        }
    }

    /***************************************************************************/
    /**
     * Attach boundaries of type std::int32_t to the vectors
     */
    void int32Boundaries(
        std::vector<std::int32_t> &l_bnd_vec,
        std::vector<std::int32_t> &u_bnd_vec,
        const activityMode &am
    ) const override {
        typename GParameterTCollectionT<T>::const_iterator cit;
        for(cit = this->begin(); cit != this->end(); ++cit) {
            (*cit)->template boundaries<std::int32_t>(l_bnd_vec, u_bnd_vec, am);
        }
    }

    /***************************************************************************/
    /**
     * Attach boundaries of type bool to the vectors. This function has been added for
     * completeness - at the very least it can give an indication of the number of boolean
     * parameters. Note, though, that there is a function that lets you count these parameters
     * directly.
     */
    void booleanBoundaries(
        std::vector<bool> &l_bnd_vec,
        std::vector<bool> &u_bnd_vec,
        const activityMode &am
    ) const override {
        typename GParameterTCollectionT<T>::const_iterator cit;
        for(cit = this->begin(); cit != this->end(); ++cit) {
            (*cit)->template boundaries<bool>(l_bnd_vec, u_bnd_vec, am);
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
    std::size_t countFloatParameters(const activityMode &am) const override {
        std::size_t result = 0;

        typename GParameterTCollectionT<T>::const_iterator cit;
        for(cit = this->begin(); cit != this->end(); ++cit) {
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
    std::size_t countDoubleParameters(const activityMode &am) const override {
        std::size_t result = 0;

        typename GParameterTCollectionT<T>::const_iterator cit;
        for(cit = this->begin(); cit != this->end(); ++cit) {
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
    std::size_t countInt32Parameters(const activityMode &am) const override {
        std::size_t result = 0;

        typename GParameterTCollectionT<T>::const_iterator cit;
        for(cit = this->begin(); cit != this->end(); ++cit) {
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
    std::size_t countBoolParameters(const activityMode &am) const override {
        std::size_t result = 0;

        typename GParameterTCollectionT<T>::const_iterator cit;
        for(cit = this->begin(); cit != this->end(); ++cit) {
            result += (*cit)->template countParameters<bool>(am);
        }

        return result;
    }

    /***************************************************************************/
    /**
     * Assigns part of a value vector to the parameter
     */
    void assignFloatValueVector(
        const std::vector<float> &par_vec,
        std::size_t &pos,
        const activityMode &am
    ) override {
        typename GParameterTCollectionT<T>::iterator it;
        for(it = this->begin(); it != this->end(); ++it) {
            (*it)->template assignValueVector<float>(par_vec, pos, am);
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
        const std::vector<double> &par_vec,
        std::size_t &pos,
        const activityMode &am
    ) override {
        typename GParameterTCollectionT<T>::iterator it;
        for(it = this->begin(); it != this->end(); ++it) {
            (*it)->template assignValueVector<double>(par_vec, pos, am);
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
        const std::vector<std::int32_t> &par_vec,
        std::size_t &pos,
        const activityMode &am
    ) override {
        typename GParameterTCollectionT<T>::iterator it;
        for(it = this->begin(); it != this->end(); ++it) {
            (*it)->template assignValueVector<std::int32_t>(par_vec, pos, am);
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
        const std::vector<bool> &par_vec,
        std::size_t &pos,
        const activityMode &am
    ) override {
        typename GParameterTCollectionT<T>::iterator it;
        for(it = this->begin(); it != this->end(); ++it) {
            (*it)->template assignValueVector<bool>(par_vec, pos, am);
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
        const std::map<std::string, std::vector<float>> &par_map,
        const activityMode &am
    ) override {
        typename GParameterTCollectionT<T>::iterator it;
        for(it = this->begin(); it != this->end(); ++it) {
            (*it)->template assignValueVectors<float>(par_map, am);
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
        const std::map<std::string, std::vector<double>> &par_map,
        const activityMode &am
    ) override {
        typename GParameterTCollectionT<T>::iterator it;
        for(it = this->begin(); it != this->end(); ++it) {
            (*it)->template assignValueVectors<double>(par_map, am);
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
        const std::map<std::string, std::vector<std::int32_t>> &par_map,
        const activityMode &am
    ) override {
        typename GParameterTCollectionT<T>::iterator it;
        for(it = this->begin(); it != this->end(); ++it) {
            (*it)->template assignValueVectors<std::int32_t>(par_map, am);
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
        const std::map<std::string, std::vector<bool>> &par_map,
        const activityMode &am
    ) override {
        typename GParameterTCollectionT<T>::iterator it;
        for(it = this->begin(); it != this->end(); ++it) {
            (*it)->template assignValueVectors<bool>(par_map, am);
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
        const float &min,
        const float &max,
        const activityMode &am,
        Gem::Hap::GRandomBase &gr
    ) override {
        typename GParameterTCollectionT<T>::iterator it;
        for(it = this->begin(); it != this->end(); ++it) {
            (*it)->template multiplyByRandom<float>(min, max, am, gr);
        }
    }

    /***************************************************************************/
    /**
     * Multiplication with a random value in a given range
     */
    void doubleMultiplyByRandom(
        const double &min,
        const double &max,
        const activityMode &am,
        Gem::Hap::GRandomBase &gr
    ) override {
        typename GParameterTCollectionT<T>::iterator it;
        for(it = this->begin(); it != this->end(); ++it) {
            (*it)->template multiplyByRandom<double>(min, max, am, gr);
        }
    }

    /***************************************************************************/
    /**
     * Multiplication with a random value in a given range
     */
    void int32MultiplyByRandom(
        const std::int32_t &min,
        const std::int32_t &max,
        const activityMode &am,
        Gem::Hap::GRandomBase &gr
    ) override {
        typename GParameterTCollectionT<T>::iterator it;
        for(it = this->begin(); it != this->end(); ++it) {
            (*it)->template multiplyByRandom<std::int32_t>(min, max, am, gr);
        }
    }

    /***************************************************************************/
    /**
     * Multiplication with a random value in the range [0,1[
     */
    void floatMultiplyByRandom(const activityMode &am, Gem::Hap::GRandomBase &gr) override {
        typename GParameterTCollectionT<T>::iterator it;
        for(it = this->begin(); it != this->end(); ++it) {
            (*it)->template multiplyByRandom<float>(am, gr);
        }
    }

    /***************************************************************************/
    /**
     * Multiplication with a random value in the range [0,1[
     */
    void doubleMultiplyByRandom(const activityMode &am, Gem::Hap::GRandomBase &gr) override {
        typename GParameterTCollectionT<T>::iterator it;
        for(it = this->begin(); it != this->end(); ++it) {
            (*it)->template multiplyByRandom<double>(am, gr);
        }
    }

    /***************************************************************************/
    /**
     * Multiplication with a random value in the range [0,1[
     */
    void int32MultiplyByRandom(const activityMode &am, Gem::Hap::GRandomBase &gr) override {
        typename GParameterTCollectionT<T>::iterator it;
        for(it = this->begin(); it != this->end(); ++it) {
            (*it)->template multiplyByRandom<std::int32_t>(am, gr);
        }
    }

    /***************************************************************************/
    /**
     * Multiplication with a constant value
     */
    void floatMultiplyBy(const float &value, const activityMode &am) override {
        typename GParameterTCollectionT<T>::iterator it;
        for(it = this->begin(); it != this->end(); ++it) {
            (*it)->template multiplyBy<float>(value, am);
        }
    }

    /***************************************************************************/
    /**
     * Multiplication with a constant value
     */
    void doubleMultiplyBy(const double &value, const activityMode &am) override {
        typename GParameterTCollectionT<T>::iterator it;
        for(it = this->begin(); it != this->end(); ++it) {
            (*it)->template multiplyBy<double>(value, am);
        }
    }

    /***************************************************************************/
    /**
     * Multiplication with a constant value
     */
    void int32MultiplyBy(const std::int32_t &value, const activityMode &am) override {
        typename GParameterTCollectionT<T>::iterator it;
        for(it = this->begin(); it != this->end(); ++it) {
            (*it)->template multiplyBy<std::int32_t>(value, am);
        }
    }

    /***************************************************************************/
    /**
     * Initialization with a constant value
     */
    void floatFixedValueInit(const float &value, const activityMode &am) override {
        typename GParameterTCollectionT<T>::iterator it;
        for(it = this->begin(); it != this->end(); ++it) {
            (*it)->template fixedValueInit<float>(value, am);
        }
    }

    /***************************************************************************/
    /**
     * Initialization with a constant value
     */
    void doubleFixedValueInit(const double &value, const activityMode &am) override {
        typename GParameterTCollectionT<T>::iterator it;
        for(it = this->begin(); it != this->end(); ++it) {
            (*it)->template fixedValueInit<double>(value, am);
        }
    }

    /***************************************************************************/
    /**
     * Initialization with a constant value
     */
    void int32FixedValueInit(const std::int32_t &value, const activityMode &am) override {
        typename GParameterTCollectionT<T>::iterator it;
        for(it = this->begin(); it != this->end(); ++it) {
            (*it)->template fixedValueInit<std::int32_t>(value, am);
        }
    }

    /***************************************************************************/
    /**
     * Initialization with a constant value
     */
    void booleanFixedValueInit(const bool &value, const activityMode &am) override {
        typename GParameterTCollectionT<T>::iterator it;
        for(it = this->begin(); it != this->end(); ++it) {
            (*it)->template fixedValueInit<bool>(value, am);
        }
    }

    /***************************************************************************/
    /**
     * Adds the "same-type" parameters of another GParameterBase object to this one
     */
    void floatAdd(std::shared_ptr<GParameterBase> p_base, const activityMode &am) override {
        // We first need to convert p_base into the local type
        std::shared_ptr<GParameterTCollectionT<T>> p =
            GParameterBase::parameterbase_cast<GParameterTCollectionT<T>>(p_base);

        // Check that both collections have the same size
        if(this->size() != p->size()) {
            throw geneva_exception(
                g_error_streamer(DO_LOG, Gem::Common::timeAndPlace())
                << "In GParameterTCollectionT<T>::floatAdd(): Error!" << '\n'
                << "Collections have a different size: " << this->size() << " / " << p->size()
                << '\n'
            );
        }

        typename GParameterTCollectionT<T>::iterator it;
        typename GParameterTCollectionT<T>::iterator p_it;
        for(it = this->begin(), p_it = p->begin(); it != this->end(); ++it, ++p_it) {
            (*it)->template add<float>(*p_it, am);
        }
    }

    /***************************************************************************/
    /**
     * Adds the "same-type" parameters of another GParameterBase object to this one
     */
    void doubleAdd(std::shared_ptr<GParameterBase> p_base, const activityMode &am) override {
        // We first need to convert p_base into the local type
        std::shared_ptr<GParameterTCollectionT<T>> p =
            GParameterBase::parameterbase_cast<GParameterTCollectionT<T>>(p_base);

        // Check that both collections have the same size
        if(this->size() != p->size()) {
            throw geneva_exception(
                g_error_streamer(DO_LOG, Gem::Common::timeAndPlace())
                << "In GParameterTCollectionT<T>::doubleAdd(): Error!" << '\n'
                << "Collections have a different size: " << this->size() << " / " << p->size()
                << '\n'
            );
        }

        typename GParameterTCollectionT<T>::iterator it;
        typename GParameterTCollectionT<T>::iterator p_it;
        for(it = this->begin(), p_it = p->begin(); it != this->end(); ++it, ++p_it) {
            (*it)->template add<double>(*p_it, am);
        }
    }

    /***************************************************************************/
    /**
     * Adds the "same-type" parameters of another GParameterBase object to this one
     */
    void int32Add(std::shared_ptr<GParameterBase> p_base, const activityMode &am) override {
        // We first need to convert p_base into the local type
        std::shared_ptr<GParameterTCollectionT<T>> p =
            GParameterBase::parameterbase_cast<GParameterTCollectionT<T>>(p_base);

        // Check that both collections have the same size
        if(this->size() != p->size()) {
            throw geneva_exception(
                g_error_streamer(DO_LOG, Gem::Common::timeAndPlace())
                << "In GParameterTCollectionT<T>::int32Add(): Error!" << '\n'
                << "Collections have a different size: " << this->size() << " / " << p->size()
                << '\n'
            );
        }

        typename GParameterTCollectionT<T>::iterator it;
        typename GParameterTCollectionT<T>::iterator p_it;
        for(it = this->begin(), p_it = p->begin(); it != this->end(); ++it, ++p_it) {
            (*it)->template add<std::int32_t>(*p_it, am);
        }
    }

    /***************************************************************************/
    /**
     * Subtracts the "same-type" parameters of another GParameterBase object from this one
     */
    void floatSubtract(std::shared_ptr<GParameterBase> p_base, const activityMode &am) override {
        // We first need to convert p_base into the local type
        std::shared_ptr<GParameterTCollectionT<T>> p =
            GParameterBase::parameterbase_cast<GParameterTCollectionT<T>>(p_base);

        // Check that both collections have the same size
        if(this->size() != p->size()) {
            throw geneva_exception(
                g_error_streamer(DO_LOG, Gem::Common::timeAndPlace())
                << "In GParameterTCollectionT<T>::floatSubtract(): Error!" << '\n'
                << "Collections have a different size: " << this->size() << " / " << p->size()
                << '\n'
            );
        }

        typename GParameterTCollectionT<T>::iterator it;
        typename GParameterTCollectionT<T>::iterator p_it;
        for(it = this->begin(), p_it = p->begin(); it != this->end(); ++it, ++p_it) {
            (*it)->template subtract<float>(*p_it, am);
        }
    }

    /***************************************************************************/
    /**
     * Subtracts the "same-type" parameters of another GParameterBase object from this one
     */
    void doubleSubtract(std::shared_ptr<GParameterBase> p_base, const activityMode &am) override {
        // We first need to convert p_base into the local type
        std::shared_ptr<GParameterTCollectionT<T>> p =
            GParameterBase::parameterbase_cast<GParameterTCollectionT<T>>(p_base);

        // Check that both collections have the same size
        if(this->size() != p->size()) {
            throw geneva_exception(
                g_error_streamer(DO_LOG, Gem::Common::timeAndPlace())
                << "In GParameterTCollectionT<T>::doubleSubtract(): Error!" << '\n'
                << "Collections have a different size: " << this->size() << " / " << p->size()
                << '\n'
            );
        }

        typename GParameterTCollectionT<T>::iterator it;
        typename GParameterTCollectionT<T>::iterator p_it;
        for(it = this->begin(), p_it = p->begin(); it != this->end(); ++it, ++p_it) {
            (*it)->template subtract<double>(*p_it, am);
        }
    }

    /***************************************************************************/
    /**
     * Subtracts the "same-type" parameters of another GParameterBase object from this one
     */
    void int32Subtract(std::shared_ptr<GParameterBase> p_base, const activityMode &am) override {
        // We first need to convert p_base into the local type
        std::shared_ptr<GParameterTCollectionT<T>> p =
            GParameterBase::parameterbase_cast<GParameterTCollectionT<T>>(p_base);

        // Check that both collections have the same size
        if(this->size() != p->size()) {
            throw geneva_exception(
                g_error_streamer(DO_LOG, Gem::Common::timeAndPlace())
                << "In GParameterTCollectionT<T>::int32Subtract(): Error!" << '\n'
                << "Collections have a different size: " << this->size() << " / " << p->size()
                << '\n'
            );
        }

        typename GParameterTCollectionT<T>::iterator it;
        typename GParameterTCollectionT<T>::iterator p_it;
        for(it = this->begin(), p_it = p->begin(); it != this->end(); ++it, ++p_it) {
            (*it)->template subtract<std::int32_t>(*p_it, am);
        }
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
        if(GParameterBase::modify_GUnitTests_()) {
            result = true;
        }
        if(Gem::Common::GPtrContainerT<T>::modify_GUnitTests_()) {
            result = true;
        }

        return result;

#else /* GEM_TESTING */ // If this function is called when GEM_TESTING isn't set, throw
        Gem::Common::condnotset("GBrokerEA::modify_GUnitTests", "GEM_TESTING");
        return false;
#endif                  /* GEM_TESTING */
    }

    /***************************************************************************/
    /**
     * Performs self tests that are expected to succeed. This is needed for testing purposes
     */
    void specificTestsNoFailureExpected_GUnitTests_() override {
#ifdef GEM_TESTING
        // Call the parent classes' functions
        GParameterBase::specificTestsNoFailureExpected_GUnitTests_();
        Gem::Common::GPtrContainerT<T>::specificTestsNoFailureExpected_GUnitTests_();

        //------------------------------------------------------------------------------

#else /* GEM_TESTING */ // If this function is called when GEM_TESTING isn't set, throw
        Gem::Common::condnotset(
            "GBrokerEA::specificTestsNoFailureExpected_GUnitTests",
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
        // Call the parent classes' functions
        GParameterBase::specificTestsFailuresExpected_GUnitTests_();
        Gem::Common::GPtrContainerT<T>::specificTestsFailuresExpected_GUnitTests_();

        //------------------------------------------------------------------------------

        { // Some test
        }

        //------------------------------------------------------------------------------

#else /* GEM_TESTING */ // If this function is called when GEM_TESTING isn't set, throw
        Gem::Common::condnotset(
            "GBrokerEA::specificTestsFailuresExpected_GUnitTests",
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
        return std::string("GParameterTCollectionT");
    }

    /***************************************************************************/
    /**
     * Creates a deep clone of this object. Declared purely virtual, as this class is not
     * intended to be used directly.
     */
    GParameterBase *clone_() const override = 0;

    /***************************************************************************/
    /**
     * Allows to adapt the values stored in this class. We assume here that
     * each item has its own adapt function. Hence we do not need to use or
     * store own adaptors.
     *
     * @return The number of adaptions that were carried out
     */
    std::size_t adapt_(Gem::Hap::GRandomBase &gr) override {
        std::size_t n_adapted = 0;

        for(auto const &par_ptr : *this) {
            n_adapted += par_ptr->adapt(gr);
        }

        return n_adapted;
    }

    /***************************************************************************/
    /**
     * Triggers updates when the optimization process has stalled
     */
    bool updateAdaptorsOnStall_(std::size_t n_stalls) override {
        bool update_performed = false;

        for(auto const &par_ptr : *this) {
            if(par_ptr->updateAdaptorsOnStall(n_stalls)) {
                update_performed = true;
            }
        }

        return update_performed;
    }

    /******************************************************************************/
    /**
     * Retrieves information from adaptors with a given property
     *
     * @param adaptor_name The name of the adaptor to be queried
     * @param property The property for which information is sought
     * @param data A vector, to which the properties should be added
     */
    void queryAdaptor_(
        const std::string &adaptor_name,
        const std::string &property,
        std::vector<std::any> &data
    ) const override {
        for(auto const &par_ptr : *this) {
            par_ptr->queryAdaptor(adaptor_name, property, data);
        }
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

/******************************************************************************/

} /* namespace Gem::Geneva::Parameters */

/******************************************************************************/
/** @brief Mark this class as abstract. This is the content of
 * BOOST_SERIALIZATION_ASSUME_ABSTRACT(T) */

namespace boost::serialization {
template <typename T>
struct is_abstract<Gem::Geneva::Parameters::GParameterTCollectionT<T>> : public boost::true_type {};
template <typename T>
struct is_abstract<const Gem::Geneva::Parameters::GParameterTCollectionT<T>> : public boost::true_type {};
} /* namespace boost::serialization */
/******************************************************************************/
