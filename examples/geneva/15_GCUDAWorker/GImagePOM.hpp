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
#include <type_traits>
#include <future>
#include <filesystem>
#include <atomic>
#include <cstdlib>

// Geneva headers go here
#include "geneva/GPluggableOptimizationMonitors.hpp"
#include "GImageIndividual.hpp"
#include "GImageIndividualEvaluator.hpp"

namespace Gem::Geneva
{
    /******************************************************************************/
    ////////////////////////////////////////////////////////////////////////////////
    /******************************************************************************/
    /**
     * This class saves the best image of each iteration to disk
     */
    class GImagePOM final
        : public GBasePluggableOM
    {
        ///////////////////////////////////////////////////////////////////////
        friend class boost::serialization::access;

        template <typename Archive>
        void serialize(Archive& ar, const unsigned int)
        {
            using boost::serialization::make_nvp;

            ar
                & make_nvp("GBasePluggableOM", boost::serialization::base_object<GBasePluggableOM>(*this))
                & BOOST_SERIALIZATION_NVP(resultImageDirectory_)
                & BOOST_SERIALIZATION_NVP(targetFileName_)
                & BOOST_SERIALIZATION_NVP(emitBestOnly_)
                & BOOST_SERIALIZATION_NVP(useGPU_)
                & BOOST_SERIALIZATION_NVP(blockSize_)
                & BOOST_SERIALIZATION_NVP(gridSize_)
            ;
        }

        ///////////////////////////////////////////////////////////////////////

    public:
        /*
         * The standard constructor.
         *
         * @param resultDirectory The directory to which result information should be written
         * @param emitBestOnly Whether only the best individuals should be emitted
         */
        GImagePOM(
            const std::string& resultDirectory,
            const std::string& targetFileName,
            bool emitBestOnly,
            bool useGPU,
            const std::tuple<int, int>& blockSize,
            const std::tuple<int, int>& gridSize
        )
            : resultImageDirectory_(GImagePOM::trailingSlash(resultDirectory)),
              targetFileName_(targetFileName),
              emitBestOnly_(emitBestOnly),
              useGPU_(useGPU),
              blockSize_(blockSize),
              gridSize_(gridSize)
        {
            /* nothing */
        }

        /***************************************************************************/
        /**
         * The copy constructor
         *
         * @param cp A copy of another GImagePOM object
         */
        GImagePOM(const GImagePOM& cp)
            : resultImageDirectory_(cp.resultImageDirectory_),
              targetFileName_(cp.targetFileName_),
              emitBestOnly_(cp.emitBestOnly_),
              useGPU_(cp.useGPU_),
              blockSize_(cp.blockSize_),
              gridSize_(cp.gridSize_),
              first_(true), // will result in a seperate evaluatpr
              evaluator_ptr_{}
        {
            /* nothing */
        }

        /***************************************************************************/
        /**
         * The destructor
         */
        ~GImagePOM() override = default;

        /***************************************************************************/
        /**
         * Allows to specify whether only images for improved iterations should be emitted
         */
        [[maybe_unused]] void setEmitBestOnly(const bool& emitBestOnly)
        {
            emitBestOnly_ = emitBestOnly;
        }

        /***************************************************************************/
        /**
         * Allows to check whether only images for improved iterations should be emitted
         */
        [[maybe_unused]] [[nodiscard]] bool getEmitBestOnly() const
        {
            return emitBestOnly_;
        }

    protected:
        /************************************************************************/
        /**
         * Loads the data of another object
         *
         * cp A pointer to another GCollectiveMonitorT<ind_type> object, camouflaged as a GObject
         */
        void load_(const GObject* cp) override
        {
            // Check that we are dealing with a GImagePOM reference independent of this object and convert the pointer
            const GImagePOM* p_load = Gem::Common::g_convert_and_compare(cp, this);

            // Load the parent classes' data ...
            GBasePluggableOM::load_(cp);

            // ... and then our local data
            this->resultImageDirectory_ = p_load->resultImageDirectory_;
            this->targetFileName_ = p_load->targetFileName_;
            this->emitBestOnly_ = p_load->emitBestOnly_;
            this->useGPU_ = p_load->useGPU_;
            blockSize_ = p_load->blockSize_;
            gridSize_ = p_load->gridSize_;
        }

        /** @brief Allow access to this classes compare_ function */
        friend void Gem::Common::compare_base_t<GImagePOM>(
            GImagePOM const&
            , GImagePOM const&
            , Gem::Common::GToken&);

        /** @brief Searches for compliance with expectations with respect to another object of the same type */
        void compare_(
            const GObject& cp
            , const Gem::Common::expectation& e
            , const double& limit) const final
        {
            using namespace Gem::Common;

            // Check that we are dealing with a GImagePOM reference independent of this object and convert the pointer
            const GImagePOM* p_load = Gem::Common::g_convert_and_compare(cp, this);

            GToken token("GImagePOM", e);

            // Compare our parent data ...
            Gem::Common::compare_base_t<Gem::Geneva::GBasePluggableOM>(*this, *p_load, token);

            // ... and then our local data
            compare_t(IDENTITY(resultImageDirectory_, p_load->resultImageDirectory_), token);
            compare_t(IDENTITY(targetFileName_, p_load->targetFileName_), token);
            compare_t(IDENTITY(emitBestOnly_, p_load->emitBestOnly_), token);
            compare_t(IDENTITY(useGPU_, p_load->useGPU_), token);
            compare_t(IDENTITY(blockSize_, p_load->blockSize_), token);
            compare_t(IDENTITY(gridSize_, p_load->gridSize_), token);

            // React on deviations from the expectation
            token.evaluate();
        }

        /***************************************************************************/
        /**
         * Applies modifications to this object. This is needed for testing purposes
         *
         * @return A boolean which indicates whether modifications were made
         */
        bool modify_GUnitTests_() override
        {
#ifdef GEM_TESTING
            using boost::unit_test_framework::test_suite;
            using boost::unit_test_framework::test_case;

            bool result = false;

            // Call the parent classes' functions
            if (GBasePluggableOM::modify_GUnitTests())
            {
                result = true;
            }

            // no local data -- nothing to change

            return result;

#else /* GEM_TESTING */  // If this function is called when GEM_TESTING isn't set, throw
         condnotset("GImagePOM::modify_GUnitTests", "GEM_TESTING");
         return false;
#endif /* GEM_TESTING */
        }

        /***************************************************************************/
        /**
         * Performs self tests that are expected to succeed. This is needed for testing purposes
         */
        void specificTestsNoFailureExpected_GUnitTests_() override
        {
#ifdef GEM_TESTING
            using boost::unit_test_framework::test_suite;
            using boost::unit_test_framework::test_case;

            // Call the parent classes' functions
            GBasePluggableOM::specificTestsNoFailureExpected_GUnitTests();
#else /* GEM_TESTING */  // If this function is called when GEM_TESTING isn't set, throw
         condnotset("GImagePOM::specificTestsNoFailureExpected_GUnitTests", "GEM_TESTING");
#endif /* GEM_TESTING */
        }

        /***************************************************************************/
        /**
         * Performs self tests that are expected to fail. This is needed for testing purposes
         */
        void specificTestsFailuresExpected_GUnitTests_() override
        {
#ifdef GEM_TESTING
            using boost::unit_test_framework::test_suite;
            using boost::unit_test_framework::test_case;

            // Call the parent classes' functions
            GBasePluggableOM::specificTestsFailuresExpected_GUnitTests();

#else /* GEM_TESTING */  // If this function is called when GEM_TESTING isn't set, throw
         condnotset("GImagePOM::specificTestsFailuresExpected_GUnitTests", "GEM_TESTING");
#endif /* GEM_TESTING */
        }

    private:
        /***************************************************************************/
        /**
         * Emits a name for this class / object
         */
        [[nodiscard]] std::string name_() const override
        {
            return {"GImagePOM"};
        }

        /************************************************************************/
        /**
         * Creates a deep clone of this object
         */
        [[nodiscard]] GObject* clone_() const override
        {
            return new GImagePOM(*this);
        }

        /***************************************************************************/
        /**
         * The default constructor. It is intentionally private, as it is only needed
         * for (de-)serialization purposes.
         */
        GImagePOM()
        {
            /* nothing */
        }

        /***************************************************************************/
        /**
         * Allows to emit information in different stages of the information cycle
         * (initialization, during each cycle and during finalization)
         */
        void informationFunction_(
            infoMode im
            , G_OptimizationAlgorithm_Base const* const goa
        ) override
        {
            switch (im)
            {
            case Gem::Geneva::infoMode::INFOINIT:
                {
                    // Check that the target directory for result files exists. If not, try to create it.
                    if (!std::filesystem::exists(std::filesystem::path(resultImageDirectory_)))
                    {
                        if (!std::filesystem::create_directory(std::filesystem::path(resultImageDirectory_)))
                        {
                            throw geneva_exception(
                                g_error_streamer(DO_LOG, time_and_place)
                                << "Error: could not create directory " << resultImageDirectory_ << std::endl
                            );
                        }
                    }
                    else
                    {
                        // Check that resultImageDirectory_ is indeed a directory and not a file
                        if (!std::filesystem::is_directory(std::filesystem::path(resultImageDirectory_)))
                        {
                            throw geneva_exception(
                                g_error_streamer(DO_LOG, time_and_place)
                                << "Error: " << resultImageDirectory_ << " is not a directory" << std::endl
                            );
                        }
                    }
                }
                break;

            case Gem::Geneva::infoMode::INFOPROCESSING:
                {
                    // -----------------------------------------------------------------------------------------
                    // Get the current best individual
                    auto bestIndividual_ptr
                        = goa->G_Interface_OptimizerT::getBestIterationIndividual<GImageIndividual>();

                    // Enforce processing. Together with getGPUCandidateImage_= true this will result
                    // in a retrieval of the image from the GPU, which is not normally the case.
                    bestIndividual_ptr->set_processing_status(Courtier::processingStatus::DO_PROCESS);

                    // -----------------------------------------------------------------------------------------
                    // We need an individual to initialize the evaluator, so we have to
                    // do this here instead of inside of the INFOINIT-section
                    if (first_)
                    {
                        first_ = false;

                        // Instantiate, then initialize the evaluator
                        evaluator_ptr_.reset(new GImageIndividualEvaluator(
                            targetFileName_,
                            useGPU_,
                            useGPU_, // getGPUCandidateImage_; We need the image back, if we run on the GPU
                            std::get<0>(blockSize_),
                            std::get<1>(blockSize_),
                            std::get<0>(gridSize_),
                            std::get<1>(gridSize_)
                        ));

                        evaluator_ptr_->init(bestIndividual_ptr);
                    }

                    // -----------------------------------------------------------------------------------------
                    // Perform the actual evaluation
                    const double fitness = evaluator_ptr_->evaluate(bestIndividual_ptr);

                    // -----------------------------------------------------------------------------------------
                    // Trigger output of a result picture
                    if (not emitBestOnly_ || goa->progress())
                    {
                        const std::string resultFileName
                            = resultImageDirectory_
                            + std::to_string(goa->G_Interface_OptimizerT::getIteration())
                            + "_"
                            + std::to_string(fitness) + "_bestIndividual.png";
                        evaluator_ptr_->saveCandidateImageToDisc(resultFileName);
                    }

                    // -----------------------------------------------------------------------------------------
                }
                break;

            case Gem::Geneva::infoMode::INFOEND:
                {
                    // Terminate the evaluator
                    evaluator_ptr_->finalize();
                }
                break;
            };
        }

        /***************************************************************************/
        /**
         * Adds a slash to the end of the path if necessary
         */
        static std::string trailingSlash(const std::string& path)
        {
            if (path[path.size() - 1] != '/')
            {
                return path + '/';
            }
            else
            {
                return path;
            }
        }

        /***************************************************************************/
        // Class data

        std::string resultImageDirectory_ = "./results/"; ///< The target directory for results
        std::string targetFileName_{"./pictures/ml.png"}; ///< The name of the target image

        bool emitBestOnly_{true}; ///< Indicates whether images should only be written for improved iterations
        bool useGPU_{true}; ///< Whether the GPU shall be used for evaluation
        std::tuple<int, int> blockSize_{16, 16}; ///< The CUDA block size
        std::tuple<int, int> gridSize_{64, 48}; ///< The CUDA grid size

        // For the evaluation process
        bool first_{true}; ///< Check whether we have already initialized the evaluator
        std::shared_ptr<Geneva::GImageIndividualEvaluator> evaluator_ptr_; ///< Will point to the evaluator
    };

    /******************************************************************************/
    ////////////////////////////////////////////////////////////////////////////////
    /******************************************************************************/
} /* namespace Gem::Geneva */
