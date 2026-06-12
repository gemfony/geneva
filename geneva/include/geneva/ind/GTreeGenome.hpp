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
#include <map>
#include <memory>
#include <string>
#include <tuple>
#include <typeinfo>
#include <vector>

// Boost header files go here
#include <boost/serialization/split_member.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/ptree_serialization.hpp>

// Geneva headers go here
#include "common/GCommonInterfaceT.hpp"
#include "common/GContainerT.hpp"
#include "common/GExceptions.hpp"
#include "common/GLogger.hpp"
#include "geneva/ind/GOptimizableEntity.hpp"
#include "geneva/par/GParameterBase.hpp"
#include "geneva/GOptimizationEnums.hpp"

// aliases for ease of use
namespace pt = boost::property_tree;

#ifdef GEM_TESTING

#include "common/GUnitTestFrameworkT.hpp"
#include "geneva/par/GBooleanObject.hpp"
#include "geneva/par/GConstrainedDoubleObject.hpp"
#include "geneva/par/GConstrainedDoubleObjectCollection.hpp"
#include "geneva/par/GConstrainedInt32Object.hpp"
#include "geneva/par/GConstrainedInt32ObjectCollection.hpp"
#include "geneva/par/GDoubleCollection.hpp"
#include "geneva/par/GDoubleObject.hpp"
#include "geneva/par/GInt32Collection.hpp"
#include "geneva/par/GParameterObjectCollection.hpp"

#endif /* GEM_TESTING */

namespace Gem::Geneva::Individuals {
class GTestIndividual1; // forward declaration, needed for testing purposes
} /* namespace Gem::Geneva::Individuals */

namespace Gem::Geneva::Parameters {

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/
/**
 * This class implements a collection of GParameterBase objects, organised as a
 * tree. It is the tree-genome implementation of GOptimizableEntity and forms the
 * basis of many user-defined individuals.
 */
class GTreeGenome // NOLINT(cppcoreguidelines-special-member-functions)
  : public GOptimizableEntity
  , public Gem::Common::GUniquePtrContainerT<GParameterBase> {
    friend class Gem::Geneva::Individuals::GTestIndividual1; ///< Needed for testing purposes

    ///////////////////////////////////////////////////////////////////////
    friend class boost::serialization::access;

    template <typename Archive>
    void serialize(Archive &ar, const unsigned int) {
        using boost::serialization::make_nvp;

        // The individual-level state lives in the GOptimizableEntity base; the
        // GParameterBase container is the tree-specific base. Both are serialized
        // as base-objects.
        ar &make_nvp(
                "GOptimizableEntity",
                boost::serialization::base_object<GOptimizableEntity>(*this)
            ) &
            make_nvp(
                "GStdPtrVectorInterfaceT_GParameterBase",
                boost::serialization::base_object<Gem::Common::GUniquePtrContainerT<GParameterBase>>(*this)
            );
    }
    ///////////////////////////////////////////////////////////////////////

public:
    /** @brief The default constructor */
    GTreeGenome();
    /** @brief Initialization with the number of fitness criteria */
    explicit GTreeGenome(std::size_t);
    /** @brief The copy constructor */
    GTreeGenome(GTreeGenome const &);
    /** @brief The destructor */
    ~GTreeGenome() override = default;

    /** @brief Transformation of the individual's parameter objects into a boost::property_tree object */
    void toPropertyTree(pt::ptree &, std::string const & = "parameterset") const override;

    /** @brief Transformation of the individual's parameter objects into a list of comma-separated values */
    std::string toCSV(
        bool = false // with_name_and_type
        ,
        bool = true // with_commas
        ,
        bool = true // use_raw_fitness
        ,
        bool = true // show_validity
    ) const override;

    /** @brief Prevent shadowing of std::vector<GParameterBase>::at() */
    Gem::Common::GUniquePtrContainerT<GParameterBase>::reference
    at(std::size_t const &pos);

    /** @brief Perform a cross-over operation between this object and another */
    std::shared_ptr<GOptimizableEntity>
    crossOverWith(GOptimizableEntity const &) const override;

    /** @brief Triggers updates of adaptors contained in this object */
    void updateAdaptorsOnStall(std::uint32_t) override;

    /** @brief Retrieves information from adaptors with a given property */
    void queryAdaptor(
        std::string const &adaptor_name,
        std::string const &property,
        std::vector<std::any> &data
    ) const override;

    /** @brief Retrieves parameters relevant for the evaluation from another GOptimizableEntity */
    void cannibalize(GOptimizableEntity &) override;

    /***************************************************************************/
    /**
     * This function returns a parameter set at a given position of the data set.
     * Note that this function will only be accessible to the compiler if par_type
     * is a derivative of GParameterBase, thanks to the magic of std::enable_if and
     * type_traits.
     *
     * @param pos The position in our data array that shall be converted
     * @return A converted version of the GParameterBase object, as required by the user
     */
    template <typename par_type>
        requires std::derived_from<par_type, GParameterBase>
    const std::shared_ptr<par_type> at(std::size_t const &pos) const {
        // The parameters are owned by unique_ptr; hand the caller a NON-OWNING shared_ptr view of the
        // live element (it outlives the transient access). Error checks on the conversion are internal.
        return Gem::Common::convertSmartPointer<GParameterBase, par_type>(
            Gem::Common::nonOwningShared(data_cnt_.at(pos))
        );
    }

    /* ----------------------------------------------------------------------------------
     * So far untested. See also the second version of the at() function.
     * ----------------------------------------------------------------------------------
     */

    /***************************************************************************/
    /**
     * Retrieves an item according to a description provided by the target tuple. Parameters are
     * addressed positionally: std::get<2>(target) is the index into the flat parameter vector of
     * type par_type. The string field (std::get<0> mode / std::get<1> name) is retained only as a
     * display label for monitors (e.g. GProgressPlotter) -- it no longer identifies a parameter.
     */
    template <typename par_type>
    std::any getVarItem(std::tuple<std::size_t, std::string, std::size_t> const &target) {
        std::vector<par_type> vars;
        this->streamline<par_type>(vars);
        return std::any(vars.at(std::get<2>(target)));
    }

    /***************************************************************************/
    /**
     * Retrieve information about the total number of parameters of type
     * par_type in the individual. Note that the GParameterBase-template
     * function will throw if this function is called for an unsupported type.
     *
     * @param am An enum indicating whether only information about active, inactive or all parameters of this type should be extracted
     */
    template <typename par_type>
    std::size_t countParameters(activityMode const &am = activityMode::DEFAULTACTIVITYMODE) const {
        std::size_t result = 0;

        // Loop over all GParameterBase objects. Each object
        // will contribute the amount of its parameters of this type
        // to the result.
        for(const auto &parm_ptr : *this) {
            result += parm_ptr->countParameters<par_type>(am);
        }

        return result;
    }

    /* ----------------------------------------------------------------------------------
     * So far untested.
     * ----------------------------------------------------------------------------------
     */

    /***************************************************************************/
    /**
     * Loops over all GParameterBase objects. Each object will add the
     * values of its parameters to the vector, if they comply with the
     * type of the parameters to be stored in the vector.
     *
     * @param par_vec The vector to which the parameters will be added
     * @param am An enum indicating whether only information about active, inactive or all parameters of this type should be extracted
     */
    template <typename par_type>
    void streamline(
        std::vector<par_type> &par_vec,
        activityMode const &am = activityMode::DEFAULTACTIVITYMODE
    ) const {
        // Make sure the vector is clean
        par_vec.clear();

        // Loop over all GParameterBase objects.
        for(const auto &parm_ptr : *this) {
            parm_ptr->streamline<par_type>(par_vec, am);
        }
    }

    /* ----------------------------------------------------------------------------------
     * So far untested.
     * ----------------------------------------------------------------------------------
     */

    /***************************************************************************/
    /**
     * Assigns values from a std::vector to the parameters in the collection
     *
     * @param par_vec A vector of values, to be assigned to be added to GParameterBase derivatives
     * @param am An enum indicating whether only information about active, inactive or all parameters of this type should be assigned
     */
    template <typename par_type>
    void assignValueVector(
        std::vector<par_type> const &par_vec,
        activityMode const &am = activityMode::DEFAULTACTIVITYMODE
    ) {
#ifdef DEBUG
        if(countParameters<par_type>() != par_vec.size()) {
            throw geneva_exception(
                g_error_streamer(DO_LOG, Gem::Common::timeAndPlace())
                << "In GTreeGenome::assignValueVector(const std::vector<pat_type>&):" << '\n'
                << "Sizes don't match: " << countParameters<par_type>() << " / " << par_vec.size()
                << '\n'
            );
        }
#endif /* DEBUG */

        // Start assignment at the beginning of par_vec
        std::size_t pos = 0;

        // Loop over all GParameterBase objects. Each object will extract the relevant
        // parameters and increment the position counter as required.
        for(const auto &parm_ptr : *this) {
            parm_ptr->assignValueVector<par_type>(par_vec, pos, am);
        }

        // As we have modified our internal data sets, make sure the item is reprocessed
        this->mark_as_due_for_processing();
    }

    /***************************************************************************/
    /**
     * A precision-agnostic view of "the floating point parameters", i.e. the double-typed and the
     * float-typed parameters together. Algorithms that operate geometrically on the floating point
     * parameters -- (conjugate) gradient descent, Nelder-Mead, swarm -- should not care whether the
     * individual was built from double- or float-precision parameter objects; countFPParameters() /
     * streamlineFP() / assignFPValueVector() give them one double-typed working view that spans both.
     * Float parameters are widened to double on read and narrowed back on write, so the optimization
     * math always runs in double regardless of the genome's storage precision.
     *
     * @return The combined number of double-typed and float-typed parameters
     */
    std::size_t
    countFPParameters(activityMode const &am) const override {
        return countParameters<double>(am) + countParameters<float>(am);
    }

    /***************************************************************************/
    /**
     * Streamlines all floating point parameters into a single double vector: the double-typed
     * parameters first, then the (widened) float-typed parameters. This fixed ordering is the contract
     * assignFPValueVector() relies on to scatter the values back. See countFPParameters().
     *
     * @param par_vec The vector the floating point parameters are written to (cleared first)
     * @param am An enum indicating whether only active, inactive or all parameters should be extracted
     */
    void streamlineFP(
        std::vector<double> &par_vec,
        activityMode const &am
    ) const override {
        par_vec.clear();
        this->streamline<double>(par_vec, am);

        std::vector<float> float_vec;
        this->streamline<float>(float_vec, am);
        par_vec.reserve(par_vec.size() + float_vec.size());
        for(float f : float_vec) {
            par_vec.push_back(static_cast<double>(f));
        }
    }

    /***************************************************************************/
    /**
     * Scatters a double vector produced by streamlineFP() back onto the floating point parameters: the
     * first countParameters<double>() values are assigned to the double-typed parameters, the remaining
     * countParameters<float>() values are narrowed to float and assigned to the float-typed parameters.
     *
     * @param par_vec A combined value vector ordered as streamlineFP() produces it
     * @param am An enum indicating whether only active, inactive or all parameters should be assigned
     */
    void assignFPValueVector(
        std::vector<double> const &par_vec,
        activityMode const &am
    ) override {
        const std::size_t n_double = countParameters<double>(am);
        const std::size_t n_float = countParameters<float>(am);

#ifdef DEBUG
        if(n_double + n_float != par_vec.size()) {
            throw geneva_exception(
                g_error_streamer(DO_LOG, Gem::Common::timeAndPlace())
                << "In GTreeGenome::assignFPValueVector():" << '\n'
                << "Sizes don't match: " << (n_double + n_float) << " / " << par_vec.size() << '\n'
            );
        }
#endif /* DEBUG */

        if(n_double > 0) {
            std::vector<double> double_vec(
                par_vec.begin(),
                par_vec.begin() + static_cast<std::ptrdiff_t>(n_double)
            );
            this->assignValueVector<double>(double_vec, am);
        }
        if(n_float > 0) {
            std::vector<float> float_vec(n_float);
            for(std::size_t i = 0; i < n_float; ++i) {
                float_vec[i] = static_cast<float>(par_vec[n_double + i]);
            }
            this->assignValueVector<float>(float_vec, am);
        }
    }

    /***************************************************************************/
    /**
     * Loops over all GParameterBase objects. Each object will add the
     * lower and upper boundaries of its parameters to the vector, if
     * they comply with the type of the parameters to be stored in the
     * vector.
     *
     * @param l_bnd_vec The vector to which the lower boundaries will be added
     * @param u_bnd_vec The vector to which the upper boundaries will be added
     * @param am An enum indicating whether only information about active, inactive or all parameters of this type should be extracted
     */
    template <typename par_type>
    void boundaries(
        std::vector<par_type> &l_bnd_vec,
        std::vector<par_type> &u_bnd_vec,
        activityMode const &am = activityMode::DEFAULTACTIVITYMODE
    ) const {
        // Make sure the vectors are clean
        l_bnd_vec.clear();
        u_bnd_vec.clear();

        // Loop over all GParameterBase objects.
        for(const auto &parm_ptr : *this) {
            parm_ptr->boundaries<par_type>(l_bnd_vec, u_bnd_vec, am);
        }
    }

    /* ----------------------------------------------------------------------------------
     * So far untested.
     * ----------------------------------------------------------------------------------
     */

    /***************************************************************************/
    /**
     * The precision-agnostic counterpart of boundaries() for all floating point parameters: the lower
     * and upper boundaries of the double-typed parameters first, then those of the (widened) float-typed
     * parameters. The ordering matches streamlineFP()/assignFPValueVector(), so a per-parameter quantity
     * derived from these boundaries (e.g. a gradient method's per-parameter step) lines up with the
     * working vector index-for-index. See countFPParameters().
     *
     * @param l_bnd_vec The vector the lower boundaries are written to (cleared first)
     * @param u_bnd_vec The vector the upper boundaries are written to (cleared first)
     * @param am An enum indicating whether only active, inactive or all parameters should be extracted
     */
    void boundariesFP(
        std::vector<double> &l_bnd_vec,
        std::vector<double> &u_bnd_vec,
        activityMode const &am
    ) const override {
        std::vector<double> l_double;
        std::vector<double> u_double;
        this->boundaries<double>(l_double, u_double, am);

        std::vector<float> l_float;
        std::vector<float> u_float;
        this->boundaries<float>(l_float, u_float, am);

        l_bnd_vec.clear();
        u_bnd_vec.clear();
        l_bnd_vec.reserve(l_double.size() + l_float.size());
        u_bnd_vec.reserve(u_double.size() + u_float.size());
        l_bnd_vec.insert(l_bnd_vec.end(), l_double.begin(), l_double.end());
        u_bnd_vec.insert(u_bnd_vec.end(), u_double.begin(), u_double.end());
        for(float v : l_float) {
            l_bnd_vec.push_back(static_cast<double>(v));
        }
        for(float v : u_float) {
            u_bnd_vec.push_back(static_cast<double>(v));
        }
    }

    /***************************************************************************/
    /**
     * Multiplication with a random value in a given range
     */
    template <typename par_type>
    void multiplyByRandom(par_type const &min, par_type const &max, activityMode const &am) {
        // Loop over all GParameterBase objects.
        for(auto &parm_ptr : *this) {
            parm_ptr->multiplyByRandom<par_type>(min, max, am, gr_);
        }

        // As we have modified our internal data sets, make sure the item is reprocessed
        this->mark_as_due_for_processing();
    }

    /***************************************************************************/
    /**
     * Multiplication with a random value in the range [0, 1[
     */
    template <typename par_type>
    void multiplyByRandom(activityMode const &am) {
        // Loop over all GParameterBase objects.
        for(auto &parm_ptr : *this) {
            parm_ptr->multiplyByRandom<par_type>(am, gr_);
        }

        // As we have modified our internal data sets, make sure the item is reprocessed
        this->mark_as_due_for_processing();
    }

    /***************************************************************************/
    /**
     * Multiplication with a constant value
     */
    template <typename par_type>
    void multiplyBy(par_type const &val, activityMode const &am) {
        // Loop over all GParameterBase objects.
        for(auto &parm_ptr : *this) {
            parm_ptr->multiplyBy<par_type>(val, am);
        }

        // As we have modified our internal data sets, make sure the item is reprocessed
        this->mark_as_due_for_processing();
    }

    /***************************************************************************/
    /**
     * Initializes all parameters of a given type with a constant value
     */
    template <typename par_type>
    void fixedValueInit(par_type const &val, activityMode const &am) {
        // Loop over all GParameterBase objects.
        for(auto const &item_ptr : *this) {
            item_ptr->fixedValueInit<par_type>(val, am);
        }

        // As we have modified our internal data sets, make sure the item is reprocessed
        this->mark_as_due_for_processing();
    }

    /***************************************************************************/
    /**
     * Adds the parameters of another GTreeGenome object to this one
     */
    template <typename par_type>
    void add(std::shared_ptr<GTreeGenome> const &p, activityMode const &am) {
        GTreeGenome::iterator it;
        GTreeGenome::const_iterator cit;

        // Note that the GParameterBase objects need to accept a
        // std::shared_ptr<GParameterBase>, contrary to the calling conventions
        // of this function.
        for(it = this->begin(), cit = p->begin(); it != this->end(); ++it, ++cit) {
            (*it)->add<par_type>(Gem::Common::nonOwningShared(*cit), am);
        }

        // As we have modified our internal data sets, make sure the item is reprocessed
        this->mark_as_due_for_processing();
    }

    /***************************************************************************/
    /**
     * Subtracts the parameters of another GTreeGenome object from this one
     */
    template <typename par_type>
    void subtract(std::shared_ptr<GTreeGenome> const &p, activityMode const &am) {
        GTreeGenome::iterator it;
        GTreeGenome::const_iterator cit;

        // Note that the GParameterBase objects need to accept a
        // std::shared_ptr<GParameterBase>, contrary to the calling conventions
        // of this function.
        for(it = this->begin(), cit = p->begin(); it != this->end(); ++it, ++cit) {
            (*it)->subtract<par_type>(Gem::Common::nonOwningShared(*cit), am);
        }

        // As we have modified our internal data sets, make sure the item is reprocessed
        this->mark_as_due_for_processing();
    }

    /***************************************************************************/
    // Deleted functions

    explicit GTreeGenome(float const &) = delete;  ///< Intentionally undefined
    explicit GTreeGenome(double const &) = delete; ///< Intentionally undefined

protected:
    /** @brief Loads the data of another GOptimizableEntity, camouflaged as a base pointer */
    void load_(const GOptimizableEntity *) override;

    /** @brief Allow access to this classes compare_ function */
    friend void Gem::Common::compare_base_t<GTreeGenome>(
        GTreeGenome const &,
        GTreeGenome const &,
        Gem::Common::GToken &
    );

    /** @brief Searches for compliance with expectations with respect to another object of the same type */
    void compare_(
        GOptimizableEntity const & // the other object
        ,
        Gem::Common::expectation const & // the expectation for this object, e.g. equality
        ,
        double const & // the limit for allowed deviations of floating point types
    ) const override;

    /** @brief Random initialization */
    bool randomInit_(activityMode const &) override;

    /* @brief The actual adaption operations. */
    std::size_t customAdaptions() override;

    /** @brief Applies modifications to this object. This is needed for testing purposes */
    bool modify_GUnitTests_() override;
    /** @brief Performs self tests that are expected to succeed. This is needed for testing purposes */
    void specificTestsNoFailureExpected_GUnitTests_() override;
    /** @brief Performs self tests that are expected to fail. This is needed for testing purposes */
    void specificTestsFailuresExpected_GUnitTests_() override;

private:
    /***************************************************************************/
    // Overridden or virtual private functions

    /** @brief Emits a name for this class / object */
    std::string name_() const override;
    /** @brief Creates a deep clone of this object */
    GTreeGenome *clone_() const override = 0;

    /** @brief Retrieves a parameter of a given type at the specified position (genome-specific dispatch) */
    std::any
    getVarValImpl(const std::string &, const std::tuple<std::size_t, std::string, std::size_t> &target)
        override;

    /** @brief Retrieval of a suitable position for cross over inside of a vector */
    std::size_t getCrossOverPos(std::size_t, std::size_t);
};

} /* namespace Gem::Geneva::Parameters */

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/
/**
 * @brief Needed for Boost.Serialization
 */
BOOST_SERIALIZATION_ASSUME_ABSTRACT(Gem::Geneva::Parameters::GTreeGenome) // NOLINT
/******************************************************************************/
