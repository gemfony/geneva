/**
* @file GImageCUDAWorker.hpp
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

#pragma once

// Includes check for correct Boost version(s)
#include "common/GGlobalDefines.hpp"

// Standard headers go here
#include <memory>
#include <functional>

// Third party header files go here

// Boost headers go here

// Geneva headers go here
#include "common/GParserBuilder.hpp"
#include "geneva/GParameterSet.hpp"
#include "courtier/GWorkerT.hpp"
#include "courtier/GStdThreadConsumerT.hpp"

// Local headers for the image individual and canvas
#include "GImageIndividual.hpp"
#include "GImageIndividualEvaluator.hpp"

namespace Gem::Courtier
{
    /******************************************************************************/
    // Some default settings
    constexpr unsigned int GII_DEF_BS_X{16};
    constexpr unsigned int GII_DEF_BS_Y{16};

    // 0 means: determine automatically
    constexpr unsigned int GII_DEF_GS_X{0}; // 64 is a good choice for block.x == 16
    constexpr unsigned int GII_DEF_GS_Y{0}; // 48 is a good choice for block.y == 16

    const std::string GII_DEF_IMAGEFILE{"./pictures/ml.png"};
    constexpr bool GII_DEF_USEGPU = true;

    constexpr int GII_DEF_IMAGE_WIDTH{1024};
    constexpr int GII_DEF_IMAGE_HEIGHT{768};

    /******************************************************************************/
    /**
     * A GWorkerT-derivative for the GStdThreadConsumerT, targeted at CUDA work.
     */
    class GImageCUDAWorker final
        : public GLocalConsumerWorkerT<Geneva::GParameterSet>
    {
    public:
        /** @brief Initialization with the name of a configuration file. */
        explicit GImageCUDAWorker(const std::string&);
        /** @brief Copy constructor */
        GImageCUDAWorker(const GImageCUDAWorker&);
        /** @brief The destructor */
        ~GImageCUDAWorker() override = default;

        //------------------------------------------------------------------
        // Get rid of the default constructor

        /** @brief Disabled default constructor */
        GImageCUDAWorker() = delete;

        //------------------------------------------------------------------
        // Provide access to some key data to eliminate redundancy in
        // other classes (in particular GImageIndividualEvaluator

        /** @brief Retrieval of the name of the target image */
        std::string getTargetImageFileName() const;
        /** @brief Check whether the GPU should be used for calculations */
        bool useGPU() const;
        /** @brief Retrieval of the block size (x/y) */
        std::tuple<int,int> getBlockSize() const;
        /** @brief  Retrieval of the grid size (x/y) */
        std::tuple<int,int> getGridSize() const;

        //------------------------------------------------------------------

    protected:
        /** @brief Initialization code for processing */
        void processInit_(std::shared_ptr<Geneva::GParameterSet>) override;
        /** @brief The actual per-item work is done here */
        void process_(std::shared_ptr<Geneva::GParameterSet>) override;
        /** @brief Finalization code after processing */
        void processFinalize_() override;
        /** @brief Adds local configuration options to a GParserBuilder object */
        void addConfigurationOptions_(Gem::Common::GParserBuilder&) override;

    private:
        /** @brief Creates a deep clone of this object, camouflaged as a GWorker */
        std::shared_ptr<GWorkerT<Geneva::GParameterSet>> clone_() const override;

        // Our evaluator
        std::shared_ptr<Geneva::GImageIndividualEvaluator> evaluator_ptr_;

        //---------- Data --------------

        Gem::Common::GOneTimeRefParameterT<unsigned int> blockSize_x_{GII_DEF_BS_X};
        Gem::Common::GOneTimeRefParameterT<unsigned int> blockSize_y_{GII_DEF_BS_Y};

        Gem::Common::GOneTimeRefParameterT<unsigned int> gridSize_x_{GII_DEF_GS_X};
        Gem::Common::GOneTimeRefParameterT<unsigned int> gridSize_y_{GII_DEF_GS_Y};

        Gem::Common::GOneTimeRefParameterT<std::string> targetImageFileName_{GII_DEF_IMAGEFILE};
        Gem::Common::GOneTimeRefParameterT<bool> useGPU_{GII_DEF_USEGPU};

        int width_{GII_DEF_IMAGE_WIDTH}; ///< The width of the target image
        int height_{GII_DEF_IMAGE_HEIGHT}; ///< The height of the target image
    };

    /******************************************************************************/
} /* namespace Gem::Geneva */
