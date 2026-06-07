/**
 * @file GImagePOM.hpp
 */

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
#include <filesystem>
#include <string>
#include <vector>

// Geneva headers go here
#include "geneva/GPluggableOptimizationMonitors.hpp"

// Example-local headers
#include "GImageHelperFunctions.hpp"
#include "GImageIndividual.hpp"
#include "GMonaLisaProblem.hpp"

namespace Gem::Geneva {
/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/
/**
 * A pluggable optimization monitor that saves the iteration's best candidate image to disk. The best
 * GImageIndividual's genome is rasterised with the shared CPU renderer (the same alpha-blend math the
 * GPU kernel uses, see GMonaLisaProblem.hpp) and written as a PNG -- so the evolving superimposition
 * of triangles can be watched as it converges towards the target. No GPU read-back is needed: the
 * picture is reconstructed purely from the candidate's parameters.
 */
class GImagePOM final : public oa::GBasePluggableOM {
    ///////////////////////////////////////////////////////////////////////
    friend class boost::serialization::access;

    template <typename Archive>
    void serialize(Archive &ar, const unsigned int) {
        using boost::serialization::make_nvp;

        ar &make_nvp(
            "GBasePluggableOM",
            boost::serialization::base_object<oa::GBasePluggableOM>(*this)
        ) & BOOST_SERIALIZATION_NVP(resultImageDirectory_) &
            BOOST_SERIALIZATION_NVP(emitBestOnly_);
    }

    ///////////////////////////////////////////////////////////////////////

public:
    /***************************************************************************/
    /**
     * The standard constructor.
     *
     * @param resultDirectory The directory to which result images should be written
     * @param emitBestOnly Whether only images for improved iterations should be emitted
     */
    GImagePOM(const std::string &resultDirectory, bool emitBestOnly)
      : resultImageDirectory_(GImagePOM::trailingSlash(resultDirectory))
      , emitBestOnly_(emitBestOnly) {
        /* nothing */
    }

    /***************************************************************************/
    /** @brief The copy constructor */
    GImagePOM(const GImagePOM &cp) = default;

    /***************************************************************************/
    /** @brief The destructor */
    ~GImagePOM() override = default;

    /***************************************************************************/
    /** @brief Allows to specify whether only images for improved iterations should be emitted */
    [[maybe_unused]] void setEmitBestOnly(const bool &emitBestOnly) {
        emitBestOnly_ = emitBestOnly;
    }

    /***************************************************************************/
    /** @brief Allows to check whether only images for improved iterations should be emitted */
    [[maybe_unused]] [[nodiscard]] bool getEmitBestOnly() const {
        return emitBestOnly_;
    }

protected:
    /************************************************************************/
    /** @brief Loads the data of another object */
    void load_(const oa::GBasePluggableOM *cp) override {
        // Check that we are dealing with a GImagePOM reference independent of this object
        const GImagePOM *p_load = Gem::Common::g_convert_and_compare(cp, this);

        // Load the parent classes' data ...
        oa::GBasePluggableOM::load_(cp);

        // ... and then our local data
        this->resultImageDirectory_ = p_load->resultImageDirectory_;
        this->emitBestOnly_ = p_load->emitBestOnly_;
    }

    /** @brief Allow access to this classes compare_ function */
    friend void Gem::Common::compare_base_t<GImagePOM>(
        GImagePOM const &,
        GImagePOM const &,
        Gem::Common::GToken &
    );

    /** @brief Searches for compliance with expectations with respect to another object of the same type */
    void compare_(
        const oa::GBasePluggableOM &cp,
        const Gem::Common::expectation &e,
        const double &limit
    ) const final {
        using namespace Gem::Common;

        // Check that we are dealing with a GImagePOM reference independent of this object
        const GImagePOM *p_load = Gem::Common::g_convert_and_compare(cp, this);

        GToken token("GImagePOM", e);

        // Compare our parent data ...
        Gem::Common::compare_base_t<oa::GBasePluggableOM>(*this, *p_load, token);

        // ... and then our local data
        compare_t(IDENTITY(resultImageDirectory_, p_load->resultImageDirectory_), token);
        compare_t(IDENTITY(emitBestOnly_, p_load->emitBestOnly_), token);

        // React on deviations from the expectation
        token.evaluate();
    }

    /***************************************************************************/
    /** @brief Applies modifications to this object. This is needed for testing purposes */
    bool modify_GUnitTests_() override {
#ifdef GEM_TESTING
        bool result = false;
        if(oa::GBasePluggableOM::modify_GUnitTests()) {
            result = true;
        }
        return result;
#else /* GEM_TESTING */
        condnotset("GImagePOM::modify_GUnitTests", "GEM_TESTING");
        return false;
#endif /* GEM_TESTING */
    }

    /***************************************************************************/
    /** @brief Performs self tests that are expected to succeed. This is needed for testing purposes */
    void specificTestsNoFailureExpected_GUnitTests_() override {
#ifdef GEM_TESTING
        oa::GBasePluggableOM::specificTestsNoFailureExpected_GUnitTests();
#else /* GEM_TESTING */
        condnotset("GImagePOM::specificTestsNoFailureExpected_GUnitTests", "GEM_TESTING");
#endif /* GEM_TESTING */
    }

    /***************************************************************************/
    /** @brief Performs self tests that are expected to fail. This is needed for testing purposes */
    void specificTestsFailuresExpected_GUnitTests_() override {
#ifdef GEM_TESTING
        oa::GBasePluggableOM::specificTestsFailuresExpected_GUnitTests();
#else /* GEM_TESTING */
        condnotset("GImagePOM::specificTestsFailuresExpected_GUnitTests", "GEM_TESTING");
#endif /* GEM_TESTING */
    }

private:
    /***************************************************************************/
    /** @brief Emits a name for this class / object */
    [[nodiscard]] std::string name_() const override {
        return {"GImagePOM"};
    }

    /************************************************************************/
    /** @brief Creates a deep clone of this object */
    [[nodiscard]] oa::GBasePluggableOM *clone_() const override {
        return new GImagePOM(*this);
    }

    /***************************************************************************/
    /** @brief The default constructor. Intentionally private; only needed for (de-)serialization. */
    GImagePOM() = default;

    /***************************************************************************/
    /**
     * Emits information at the various stages of the information cycle (initialization, during each
     * iteration, and during finalization).
     */
    void informationFunction_(infoMode im, oa::GOptimizationAlgorithmBase const *const goa) override {
        switch(im) {
        case Gem::Geneva::infoMode::INFOINIT: {
            // Make sure the target directory for result images exists.
            const std::filesystem::path dir(resultImageDirectory_);
            if(not std::filesystem::exists(dir)) {
                if(not std::filesystem::create_directories(dir)) {
                    throw geneva_exception(
                        g_error_streamer(DO_LOG, Gem::Common::timeAndPlace())
                        << "GImagePOM: could not create directory " << resultImageDirectory_ << '\n'
                    );
                }
            }
            else if(not std::filesystem::is_directory(dir)) {
                throw geneva_exception(
                    g_error_streamer(DO_LOG, Gem::Common::timeAndPlace())
                    << "GImagePOM: " << resultImageDirectory_ << " is not a directory" << '\n'
                );
            }
        } break;

        case Gem::Geneva::infoMode::INFOPROCESSING: {
            // Emit an image only for improved iterations, unless every iteration was requested.
            if(emitBestOnly_ && not goa->progress()) {
                break;
            }

            auto best_ptr =
                goa->Interface::GOptimizerIT<oa::GOptimizationAlgorithmBase>::getBestIterationIndividual<GImageIndividual>();

            // Rasterise the candidate genome at the target resolution and write it out. The
            // genome scalar type is selected at compile time (gimage_fp_t), so streamline into
            // a matching buffer -- streamline<float> on a double genome collects nothing.
            std::vector<gimage_fp_t> parVec;
            best_ptr->streamline(parVec);
            const MonaLisa::Target &tgt = MonaLisa::target();
            std::vector<unsigned char> rgb;
            MonaLisa::renderToRGB(parVec.data(), static_cast<int>(parVec.size()), tgt.width,
                                  tgt.height, rgb);

            const std::uint32_t iteration =
                goa->Interface::GOptimizerIT<oa::GOptimizationAlgorithmBase>::getIteration();
            const double fitness = best_ptr->raw_fitness(0);
            const std::string resultFileName = resultImageDirectory_ + std::to_string(iteration) +
                                               "_" + std::to_string(fitness) + "_bestIndividual.png";
            Gem::Common::writeRGBtoPNG(resultFileName, rgb, tgt.width, tgt.height);
        } break;

        case Gem::Geneva::infoMode::INFOEND: {
            /* nothing */
        } break;
        };
    }

    /***************************************************************************/
    /** @brief Adds a slash to the end of the path if necessary */
    static std::string trailingSlash(const std::string &path) {
        if(path.empty() || path[path.size() - 1] != '/') {
            return path + '/';
        }
        return path;
    }

    /***************************************************************************/
    // Class data

    std::string resultImageDirectory_ = "./results/"; ///< The target directory for results
    bool emitBestOnly_{true}; ///< Whether images are written only for improved iterations
};

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/
} /* namespace Gem::Geneva */
