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
#include <concepts>

// Boost header files go here

// Geneva header files go here
#include "geneva/GBooleanAdaptor.hpp"
#include "geneva/GBooleanObject.hpp"
#include "geneva/GDoubleGaussAdaptor.hpp"
#include "geneva/GDoubleObject.hpp"
#include "geneva/GInt32GaussAdaptor.hpp"
#include "geneva/GInt32Object.hpp"
#include "geneva/GParameterBase.hpp"
#include "geneva/GParameterTCollectionT.hpp"

namespace Gem::Geneva {

/******************************************************************************/
/**
 * A collection of GParameterBase objects, ready for use in a
 * GParameterSet derivative.
 */
class GParameterObjectCollection // NOLINT(cppcoreguidelines-special-member-functions)
  : public GParameterTCollectionT<GParameterBase> {
    ///////////////////////////////////////////////////////////////////////
    friend class boost::serialization::access;

    template <typename Archive>
    void serialize(Archive &ar, const unsigned int) {
        using boost::serialization::make_nvp;

        ar &make_nvp(
            "GParameterTCollectionT_gbd",
            boost::serialization::base_object<GParameterTCollectionT<GParameterBase>>(*this)
        );
    }
    ///////////////////////////////////////////////////////////////////////

public:
    /** @brief The default constructor */
    GParameterObjectCollection() = default;
    /** @brief Initialization with a number of GParameterBase objects */
    GParameterObjectCollection(const std::size_t &, std::shared_ptr<GParameterBase>);
    /** @brief The copy constructor */
    GParameterObjectCollection(const GParameterObjectCollection &) = default;
    /** @brief The destructor */
    ~GParameterObjectCollection() override = default;

    /** @brief Prevent shadowing of std::vector<GParameterBase>::at() */
    std::shared_ptr<Gem::Geneva::GParameterBase> at(const std::size_t &pos);

    /***************************************************************************/
    /**
	  * This function returns a parameter item at a given position of the data set.
		* Note that this function will only be accessible to the compiler if parameter_type
		* is a derivative of GParameterBase, thanks to the magic of std::enable_if
		* and type_traits
	  *
	  * @param pos The position in our data array that shall be converted
	  * @return A converted version of the GParameterBase object, as required by the user
	  */
    template <typename parameter_type>
        requires std::derived_from<parameter_type, GParameterBase>
    const std::shared_ptr<parameter_type> at(const std::size_t &pos) const {
#ifdef DEBUG
        if(this->empty() || pos >= this->size()) {
            throw geneva_exception(
                g_error_streamer(DO_LOG, time_and_place)
                << "In GParameterObjectCollection::at<>(): Error!" << '\n'
                << "Tried to access position " << pos << " while size is " << this->size()
                << '\n'
            );

            // Make the compiler happy
            return std::shared_ptr<parameter_type>();
        }
#endif /* DEBUG */

        // Does error checks on the conversion internally
        return Gem::Common::convertSmartPointer<GParameterBase, parameter_type>(data_cnt_.at(pos));
    }

protected:
    /***************************************************************************/
    /** @brief Loads the data of another GObject */
    void load_(const GObject *) override;

    /** @brief Allow access to this classes compare_ function */
    friend void Gem::Common::compare_base_t<GParameterObjectCollection>(
        GParameterObjectCollection const &,
        GParameterObjectCollection const &,
        Gem::Common::GToken &
    );

    /** @brief Searches for compliance with expectations with respect to another object of the same type */
    void compare_(
        const GObject & // the other object
        ,
        const Gem::Common::expectation & // the expectation for this object, e.g. equality
        ,
        const double & // the limit for allowed deviations of floating point types
    ) const override;

    /** @brief Applies modifications to this object. This is needed for testing purposes */
    bool modify_GUnitTests_() override;
    /** @brief Performs self tests that are expected to succeed. This is needed for testing purposes */
    void specificTestsNoFailureExpected_GUnitTests_() override;
    /** @brief Performs self tests that are expected to fail. This is needed for testing purposes */
    void specificTestsFailuresExpected_GUnitTests_() override;

private:
    /** @brief Emits a name for this class / object */
    std::string name_() const override;
    /** @brief Creates a deep clone of this object. */
    GObject *clone_() const override;

    /** @brief Fills the collection with GParameterBase objects */
    void fillWithObjects_();
};

/******************************************************************************/

} /* namespace Gem::Geneva */

BOOST_CLASS_EXPORT_KEY(Gem::Geneva::GParameterObjectCollection) // NOLINT
