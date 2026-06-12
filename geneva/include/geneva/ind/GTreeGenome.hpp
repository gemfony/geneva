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
    // The per-type value channels (streamline<T> / assignValueVector<T> / countParameters<T> /
    // boundaries<T>) and the precision-agnostic FP view (streamlineFP / assignFPValueVector /
    // countFPParameters / boundariesFP) now live on GOptimizableEntity; GTreeGenome only supplies
    // the per-type virtual implementations (see the private streamline_/assignValueVector_/
    // countParameters*_/boundaries_ overrides below, which iterate the GParameterBase objects).

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

    /***************************************************************************/
    // The per-type genome value channels declared on GOptimizableEntity, implemented by iterating
    // the GParameterBase objects. One private template carries the logic per operation; the typed
    // virtual overrides are thin forwarders the base templates dispatch to.

    template <typename par_type>
    void streamlineTree_(std::vector<par_type> &v, activityMode const &am) const {
        v.clear();
        for(const auto &parm_ptr : *this) {
            parm_ptr->streamline<par_type>(v, am);
        }
    }
    template <typename par_type>
    std::size_t countParametersTree_(activityMode const &am) const {
        std::size_t result = 0;
        for(const auto &parm_ptr : *this) {
            result += parm_ptr->countParameters<par_type>(am);
        }
        return result;
    }
    template <typename par_type>
    void assignValueVectorTree_(std::vector<par_type> const &v, activityMode const &am) {
        std::size_t pos = 0;
        for(const auto &parm_ptr : *this) {
            parm_ptr->assignValueVector<par_type>(v, pos, am);
        }
    }
    template <typename par_type>
    void boundariesTree_(
        std::vector<par_type> &l,
        std::vector<par_type> &u,
        activityMode const &am
    ) const {
        l.clear();
        u.clear();
        for(const auto &parm_ptr : *this) {
            parm_ptr->boundaries<par_type>(l, u, am);
        }
    }

    void streamline_(std::vector<double> &v, activityMode const &am) const override { streamlineTree_(v, am); }
    void streamline_(std::vector<float> &v, activityMode const &am) const override { streamlineTree_(v, am); }
    void streamline_(std::vector<std::int32_t> &v, activityMode const &am) const override { streamlineTree_(v, am); }
    void streamline_(std::vector<bool> &v, activityMode const &am) const override { streamlineTree_(v, am); }

    void assignValueVector_(std::vector<double> const &v, activityMode const &am) override { assignValueVectorTree_(v, am); }
    void assignValueVector_(std::vector<float> const &v, activityMode const &am) override { assignValueVectorTree_(v, am); }
    void assignValueVector_(std::vector<std::int32_t> const &v, activityMode const &am) override { assignValueVectorTree_(v, am); }
    void assignValueVector_(std::vector<bool> const &v, activityMode const &am) override { assignValueVectorTree_(v, am); }

    std::size_t countParametersDouble_(activityMode const &am) const override { return countParametersTree_<double>(am); }
    std::size_t countParametersFloat_(activityMode const &am) const override { return countParametersTree_<float>(am); }
    std::size_t countParametersInt32_(activityMode const &am) const override { return countParametersTree_<std::int32_t>(am); }
    std::size_t countParametersBool_(activityMode const &am) const override { return countParametersTree_<bool>(am); }

    void boundaries_(std::vector<double> &l, std::vector<double> &u, activityMode const &am) const override { boundariesTree_(l, u, am); }
    void boundaries_(std::vector<float> &l, std::vector<float> &u, activityMode const &am) const override { boundariesTree_(l, u, am); }
    void boundaries_(std::vector<std::int32_t> &l, std::vector<std::int32_t> &u, activityMode const &am) const override { boundariesTree_(l, u, am); }
    void boundaries_(std::vector<bool> &l, std::vector<bool> &u, activityMode const &am) const override { boundariesTree_(l, u, am); }
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
