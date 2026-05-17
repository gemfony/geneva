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

// Standard heders go here

// Boosrt headers go here

// Geneva headers go here
#include "common/GExceptions.hpp"
#include "common/GFactoryT.hpp"
#include "common/GLogger.hpp"
#include "geneva/par/GParameterSet.hpp"
#include "geneva/GPostProcessorT.hpp"

namespace Gem::Geneva::Parameters {

/******************************************************************************/
/**
 * This class facilitates handling of factories for GParameterSet-derivatives.
 * In particular it allows to register pre- and post-procesing objects
 */
class GParameterSetFactory // NOLINT(cppcoreguidelines-special-member-functions)
  : public Gem::Common::GFactoryT<GParameterSet> {
    ///////////////////////////////////////////////////////////////////////
    friend class boost::serialization::access;

    template <typename Archive>
    void serialize(Archive &ar, const unsigned int) {
        using boost::serialization::make_nvp;

        ar &BOOST_SERIALIZATION_BASE_OBJECT_NVP(Gem::Common::GFactoryT<GParameterSet>) &
            BOOST_SERIALIZATION_NVP(preProcessor_) & BOOST_SERIALIZATION_NVP(postProcessor_);
    }
    ///////////////////////////////////////////////////////////////////////

public:
    /***************************************************************************/
    /**
	  * The standard constructor
	  *
	  * @param configFile path object of a configuration file holding information about objects of type T
	  */
    explicit GParameterSetFactory(std::filesystem::path const &configFile)
      : Gem::Common::GFactoryT<GParameterSet>(configFile) { /* nothing */
    }

    /***************************************************************************/
    /**
	  * The copy constructor
	  */
    GParameterSetFactory(const GParameterSetFactory &cp)
      : Gem::Common::GFactoryT<GParameterSet>(cp) {
        Gem::Common::copyCloneableSmartPointer(cp.postProcessor_, postProcessor_);
        Gem::Common::copyCloneableSmartPointer(cp.postProcessor_, postProcessor_);
    }

    /***************************************************************************/
    // Defaulted and deleted functions
    ~GParameterSetFactory() override = default;

    /***************************************************************************/
    /**
	  * Registration of pre-processor function objects
	  */
    void registerPreProcessor(
        std::shared_ptr<Gem::Common::GSerializableFunctionObjectT<GParameterSet>> p
    ) {
        if(p) {
            preProcessor_ = p;
        }
        else {
            throw geneva_exception(
                g_error_streamer(DO_LOG, time_and_place)
                << "In GParameterSetFactory::registerPreProcessor(): Error!" << '\n'
                << "Got empty pre-processor" << '\n'
            );
        }
    }

    /***************************************************************************/
    /**
	  * Registration of post-processor function objects
	  */
    void registerPostProcessor(
        std::shared_ptr<Gem::Common::GSerializableFunctionObjectT<GParameterSet>> p
    ) {
        if(p) {
            postProcessor_ = p;
        }
        else {
            throw geneva_exception(
                g_error_streamer(DO_LOG, time_and_place)
                << "In GParameterSetFactory::registerPostProcessor(): Error!" << '\n'
                << "Got empty post-processor" << '\n'
            );
        }
    }

protected:
    /** @brief A pre-processor for GParameterSet-derivatives */
    std::shared_ptr<Gem::Common::GSerializableFunctionObjectT<GParameterSet>> preProcessor_;

    /** @brief A post-processor for GParameterSet-derivatives */
    std::shared_ptr<Gem::Common::GSerializableFunctionObjectT<GParameterSet>> postProcessor_;

    /***************************************************************************/
    /**
     * Production of GParameterSet-derivatives
     */
    std::shared_ptr<GParameterSet> get_() override {
        std::shared_ptr<GParameterSet> p = GFactoryT<GParameterSet>::get_();

        if(preProcessor_) {
            p->registerPreProcessor(preProcessor_->clone());
        }

        if(postProcessor_) {
            p->registerPostProcessor(postProcessor_->clone());
        }

        return p;
    }

private:
    // Only needed for (de-)serialization purposes, hence private
    GParameterSetFactory() = default;
};

/******************************************************************************/

} /* namespace Gem::Geneva::Parameters */
