/**
* @file GImageCUDAWorker.cpp
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

#include "GImageCUDAWorker.hpp"

namespace Gem::Courtier
{
    /******************************************************************************/
    /**
     * The constructor
     */
    GImageCUDAWorker::GImageCUDAWorker(const std::string& configFile)
        : GLocalConsumerWorkerT<Geneva::GParameterSet>()
    {
        // Load configuration options specific to this class
        this->parseConfigFile(configFile);
    }

    /******************************************************************************/
    /**
     * The copy constructor. May not be "= default", as we do not want to copy
     * the evaluator over.
     *
     * @param cp A copy of another GImageCUDAWorker
     */
    GImageCUDAWorker::GImageCUDAWorker(const GImageCUDAWorker& cp)
        : GLocalConsumerWorkerT<Geneva::GParameterSet>(cp)
    {
        // We want to create a seperate evaluator
        evaluator_ptr_.reset();

        // Copy all other data verbatim
        blockSize_x_ = cp.blockSize_x_;
        blockSize_y_ = cp.blockSize_y_;

        gridSize_x_ = cp.gridSize_x_;
        gridSize_y_ = cp.gridSize_y_;

        targetImageFileName_ = cp.targetImageFileName_;

        useGPU_ = cp.useGPU_;

        width_ = cp.width_;
        height_ = cp.height_;
    }

    /******************************************************************************/
    /**
     * Retrieval of the name of the target image
     *
     * @return A std::string indicating the name and path of the target image
     */
    std::string GImageCUDAWorker::getTargetImageFileName() const
    {
        return targetImageFileName_;
    }

    /******************************************************************************/
    /**
     * Check whether the GPU should be used for calculations
     *
     * @return A boolean indicating whether the GPU or the CPU shall be used for the evaluation
     */
    bool GImageCUDAWorker::useGPU() const
    {
        return useGPU_;
    }

    /******************************************************************************/
    /**
     * Retrieval of the block size (x/y)
     *
     * @return A std::tuple holding the block size
     */
    std::tuple<int, int> GImageCUDAWorker::getBlockSize() const
    {
        return std::make_tuple(blockSize_x_, blockSize_y_);
    }

    /******************************************************************************/
    /**
     * Retrieval of the grid size (x/y)
     *
     * @return A std::tuple holding the grid size
     */
    std::tuple<int, int> GImageCUDAWorker::getGridSize() const
    {
        return std::make_tuple(gridSize_x_, gridSize_y_);
    }

    /******************************************************************************/
    /**
     * Allows to specify additional configuration options.
     *
     * @param gpb An object handling configuration options
     */
    void GImageCUDAWorker::addConfigurationOptions_(Common::GParserBuilder& gpb)
    {
        // Call our parent class'es function
        GLocalConsumerWorkerT<Geneva::GParameterSet>::addConfigurationOptions_(gpb);

        std::string comment;
        std::string comment1;
        std::string comment2;

        comment = "";
        comment += "Indicates whether evaluation should run on the GPU (1); or the CPU (0)";
        gpb.registerFileParameter<bool>(
            "useGPU"
            , useGPU_.reference()
            , GII_DEF_USEGPU
            , Gem::Common::VAR_IS_ESSENTIAL
            , comment
        );

        comment1 = "The block size (x-component);";
        comment2 = "The block size (y-component);";
        gpb.registerFileParameter<unsigned int, unsigned int>(
            "block_x", "block_y",
            GII_DEF_BS_X, GII_DEF_BS_Y,
            [&](const unsigned int& x, const unsigned int& y)
            {
                blockSize_x_.setValue(x);
                blockSize_y_.setValue(y);
            }
            , "CUDA block size"
            , Gem::Common::VAR_IS_ESSENTIAL
            , comment1, comment2
        );

        comment1 = "The grid size (x-component); Will be calculated automatically if set to 0";
        comment2 = "The grid size (y-component); Will be calculated automatically if set to 0";
        gpb.registerFileParameter<unsigned int, unsigned int>(
            "grid_x", "grid_y",
            GII_DEF_GS_X, GII_DEF_GS_Y,
            [&](const unsigned int& x, const unsigned int& y)
            {
                gridSize_x_.setValue(x);;
                gridSize_y_.setValue(y);
            }
            , "CUDA grid size"
            , Gem::Common::VAR_IS_ESSENTIAL
            , comment1, comment2
        );

        comment.clear();
        comment += "The name of the file holding the target image;";
        gpb.registerFileParameter<std::string>(
            "imageFile"
            , targetImageFileName_.reference()
            , GII_DEF_IMAGEFILE
            , Common::VAR_IS_ESSENTIAL
            , comment
        );
    }

    /******************************************************************************/
    /**
     * Initialization code for processing
     *
     * @param p A GIndividual object camouflaged as a GParameterSet
     */
    void
    GImageCUDAWorker::processInit_(std::shared_ptr<Geneva::GParameterSet> p)
    {
#ifdef DEBUG
        // Check that p actually points somewhere
        if (not p)
        {
            throw gemfony_exception(
                g_error_streamer(DO_LOG, time_and_place)
                << "In GImageCUDAWorker::processInit_ : Error!" << std::endl
                << "p is empty" << std::endl
            );
        }

        auto p_conv = std::dynamic_pointer_cast<Geneva::GImageIndividual>(p);
        if (not p_conv)
        {
            throw gemfony_exception(
                g_error_streamer(DO_LOG, time_and_place)
                << "In GImageCUDAWorker::processInit_(): Error!" << std::endl
                << "Conversion failed" << std::endl
            );
        }
#else
        // Translate the individual to the target type GImageIndividual
        auto p_conv = std::static_pointer_cast<Geneva::GImageIndividual>(p);
#endif /* DEBUG */

        // Create an evaluator for GImageIndividuals ...
        evaluator_ptr_.reset(new Geneva::GImageIndividualEvaluator(targetImageFileName_.value(),
                                                                   useGPU_.value(),
                                                                   false, // getGPUCandidateImage
                                                                   blockSize_x_.value(),
                                                                   blockSize_y_.value(),
                                                                   gridSize_x_.value(),
                                                                   gridSize_y_.value()
        ));

        // ... and initialize the evaluation process
        evaluator_ptr_->init(p_conv);
    }

    /******************************************************************************/
    /**
     * The actual per-item work is done here
     *
     * @param p A GIndividual object camouflaged as a GParameterSet
     */
    void
    GImageCUDAWorker::process_(std::shared_ptr<Geneva::GParameterSet> p)
    {
#ifdef DEBUG
        // Check that p actually points somewhere
        if (not p)
        {
            throw gemfony_exception(
                g_error_streamer(DO_LOG, time_and_place)
                << "In GImageCUDAWorker::process_ : Error!" << std::endl
                << "p is empty" << std::endl
            );
        }

        auto p_conv = std::dynamic_pointer_cast<Geneva::GImageIndividual>(p);
        if (not p_conv)
        {
            throw gemfony_exception(
                g_error_streamer(DO_LOG, time_and_place)
                << "In GImageCUDAWorker::process_(): Error!" << std::endl
                << "Conversion failed" << std::endl
            );
        }
#else
        // Translate the individual to the target type GImageIndividual
        auto p_conv = std::static_pointer_cast<Geneva::GImageIndividual>(p);
#endif /* DEBUG */

        // Do the actual processing. This will also set the fitness of the individual.
        double fitness = evaluator_ptr_->evaluate(p_conv);
    }

    /******************************************************************************/
    /**
     * Finalization code for processing
     */
    void
    GImageCUDAWorker::processFinalize_()
    {
        // Finalize processing, using the appropriate
        // evaluator-function
        evaluator_ptr_->finalize();
    }

    /******************************************************************************/
    /**
     * Creation of deep clones of this object. Note that a new broker ferry
     * needs to be registered with this object.
     */
    std::shared_ptr<GWorkerT<Geneva::GParameterSet>> GImageCUDAWorker::clone_() const
    {
        return std::make_shared<GImageCUDAWorker>(*this);
    }

    /******************************************************************************/
} /* namespace Gem::Courtier */
