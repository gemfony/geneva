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
#include "geneva/ind/GOptimizableEntity.hpp"
#include "geneva/GPostProcessorT.hpp"

namespace Gem::Geneva::Genome {

/******************************************************************************/
/**
 * This class facilitates handling of factories for GOptimizableEntity-derivatives.
 * In particular it allows to register pre- and post-procesing objects
 */
class GOptimizableEntityFactory // NOLINT(cppcoreguidelines-special-member-functions)
  : public Gem::Common::GFactoryT<GOptimizableEntity> {
    ///////////////////////////////////////////////////////////////////////
    friend class boost::serialization::access;

    template <typename Archive>
    void serialize(Archive &ar, [[maybe_unused]] const unsigned int version) {
        using boost::serialization::make_nvp;

        ar &boost::serialization::make_nvp(
            "GFactoryT_GOptimizableEntity",
            boost::serialization::base_object<Gem::Common::GFactoryT<GOptimizableEntity>>(*this)
        ) &
            BOOST_SERIALIZATION_NVP(pre_processor_) & BOOST_SERIALIZATION_NVP(post_processor_);
    }
    ///////////////////////////////////////////////////////////////////////

public:
    /***************************************************************************/
    /**
	  * The standard constructor
	  *
	  * @param configFile path object of a configuration file holding information about objects of type T
	  */
    explicit GOptimizableEntityFactory(std::filesystem::path const &configFile)
      : Gem::Common::GFactoryT<GOptimizableEntity>(configFile) { /* nothing */
    }

    /***************************************************************************/
    /**
	  * The copy constructor
	  *
	  * @param cp A constant reference to another GOptimizableEntityFactory object to be copied
	  */
    GOptimizableEntityFactory(const GOptimizableEntityFactory &cp)
      : Gem::Common::GFactoryT<GOptimizableEntity>(cp) {
        Gem::Common::copyCloneableSmartPointer(cp.post_processor_, post_processor_);
        Gem::Common::copyCloneableSmartPointer(cp.post_processor_, post_processor_);
    }

    /***************************************************************************/
    // Defaulted and deleted functions
    ~GOptimizableEntityFactory() override = default;

    /***************************************************************************/
    /**
	  * Registration of pre-processor function objects
	  *
	  * @param p A shared pointer to a pre-processor function object to be cloned into each produced individual; must not be empty
	  */
    void registerPreProcessor(
        std::shared_ptr<Gem::Common::GSerializableFunctionObjectT<GOptimizableEntity>> p
    ) {
        if(p) {
            pre_processor_ = p;
        }
        else {
            throw geneva_exception(
                g_error_streamer(DO_LOG, Gem::Common::timeAndPlace())
                << "In GOptimizableEntityFactory::registerPreProcessor(): Error!" << '\n'
                << "Got empty pre-processor" << '\n'
            );
        }
    }

    /***************************************************************************/
    /**
	  * Registration of post-processor function objects
	  *
	  * @param p A shared pointer to a post-processor function object to be cloned into each produced individual; must not be empty
	  */
    void registerPostProcessor(
        std::shared_ptr<Gem::Common::GSerializableFunctionObjectT<GOptimizableEntity>> p
    ) {
        if(p) {
            post_processor_ = p;
        }
        else {
            throw geneva_exception(
                g_error_streamer(DO_LOG, Gem::Common::timeAndPlace())
                << "In GOptimizableEntityFactory::registerPostProcessor(): Error!" << '\n'
                << "Got empty post-processor" << '\n'
            );
        }
    }

    /***************************************************************************/
    /** @brief The registered pre-processor (or empty). Read at slot creation to stamp the work item.
     *  @return The pre-processor function object, or an empty pointer. */
    std::shared_ptr<Gem::Common::GSerializableFunctionObjectT<GOptimizableEntity>> preProcessor() const {
        return pre_processor_;
    }
    /** @brief The registered post-processor (or empty). Read at slot creation to stamp the work item.
     *  @return The post-processor function object, or an empty pointer. */
    std::shared_ptr<Gem::Common::GSerializableFunctionObjectT<GOptimizableEntity>> postProcessor() const {
        return post_processor_;
    }

protected:
    /** @brief A pre-processor for GOptimizableEntity-derivatives */
    std::shared_ptr<Gem::Common::GSerializableFunctionObjectT<GOptimizableEntity>> pre_processor_;

    /** @brief A post-processor for GOptimizableEntity-derivatives */
    std::shared_ptr<Gem::Common::GSerializableFunctionObjectT<GOptimizableEntity>> post_processor_;

    /***************************************************************************/
    /**
     * Production of GOptimizableEntity-derivatives. The genome is PURE DATA and no longer carries the
     * pre-/post-processor: the processor objects are work-item operations, stamped by Go2 / the
     * optimization algorithm onto the GIndividualSlot wrapping each produced genome (see
     * preProcessor()/postProcessor() and Go2::runAlgorithmChain).
     *
     * @return A shared pointer to a newly produced GOptimizableEntity-derivative
     */
    std::shared_ptr<GOptimizableEntity> get_() override {
        return GFactoryT<GOptimizableEntity>::get_();
    }

private:
    // Only needed for (de-)serialization purposes, hence private
    GOptimizableEntityFactory() = default;
};

/******************************************************************************/

} /* namespace Gem::Geneva::Genome */
