/**
 * @file GImageBuilder.cpp
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

/**
 * @file
 * Example 15 -- the Mona-Lisa problem: evolve a set of alpha-blended circle-triangles so that their
 * superimposition resembles a target image. The candidate fitness is the per-pixel/per-channel
 * deviation from the target through a rational saturation function.
 *
 * The evaluation runs on the GPU via the unified courtier GPU consumer
 * (Gem::Courtier::GPU::GGPUConsumerT): a whole generation is flattened and scored in ONE bulk kernel
 * launch (runtime-compiled with NVRTC; pixel-parallel so even a small population fills the GPU). The
 * SAME render+score math is also available on the CPU (GImageIndividual::fitnessCalculation and the
 * marshaller's host reference), so a CPU run cross-checks the GPU. Switch backend / kernel in
 * config/GGPUConsumer.json -- no recompilation needed; set backend=cpu to run without a GPU.
 *
 * The useful output is produced at the end of each iteration: GImagePOM rasterises the current best
 * candidate and writes it to ./results/ as a PNG, so the picture can be watched converging.
 *
 * Because there is no network involved, this example is also a convenient stress-test harness for the
 * courtier framework, the broker, and Hap.
 */

// Standard headers
#include <csignal>
#include <memory>
#include <string>

// Geneva headers
#include "common/GParserBuilder.hpp"
#include "courtier/gpu/GGPUConsumer.hpp"
#include "geneva/Go2.hpp"
#include "geneva/oa/GEvolutionaryAlgorithm.hpp"
#include "geneva/oa/GEvolutionaryAlgorithmFactory.hpp"

// Example-local headers
#include "GImageIndividual.hpp"
#include "GImagePOM.hpp"
#include "GImageScalar.hpp"
#include "GMonaLisaGPUMarshaller.hpp"
#include "GMonaLisaProblem.hpp"

using namespace Gem::Geneva;
namespace gpu = Gem::Courtier::GPU;
namespace gen = Gem::Geneva::Genome;

int main(int argc, char **argv) {
    // ---- example-specific settings, read from a Geneva config file ----------------------------
    // These were previously command-line-only; they are now ordinary config-file parameters like
    // every other Geneva setting, parsed through the standard GParserBuilder (which also writes the
    // file with documented defaults on first run). Edit config/GImageGeneral.json and re-run -- no
    // rebuild, no command-line flags needed.
    std::string targetFile;
    std::string consumerConfig;
    bool logImages = false;
    bool emitBestOnly = false;

    Gem::Common::GParserBuilder gpb;
    gpb.registerFileParameter<std::string>(
        "target_image", targetFile, std::string("./pictures/ml-small.png"),
        Gem::Common::VAR_IS_ESSENTIAL,
        "The target image (PNG) the triangle superimposition should resemble;"
        " also defines the canvas resolution");
    gpb.registerFileParameter<std::string>(
        "gpu_config", consumerConfig, std::string(GIMAGE_DEFAULT_GPUCONFIG),
        Gem::Common::VAR_IS_ESSENTIAL,
        "The courtier GPU consumer configuration file (backend + kernel selection)");
    gpb.registerFileParameter<bool>(
        "log_images", logImages, true, Gem::Common::VAR_IS_SECONDARY,
        "Whether to write the best candidate image to ./results/ after each iteration");
    gpb.registerFileParameter<bool>(
        "emit_best_only", emitBestOnly, true, Gem::Common::VAR_IS_SECONDARY,
        "When logging images, only emit one for iterations that improved the best result");
    gpb.parseConfigFile("./config/GImageGeneral.json");

    // Go2 parses its own framework options from the command line / its own config file.
    Go2 go(argc, argv, "./config/Go2.json");

    // ---- load the target image (defines the canvas resolution and the fitness reference) -----
    Gem::Geneva::MonaLisa::loadTarget(targetFile);
    const auto &tgt = Gem::Geneva::MonaLisa::target();
    glogger << "Example 15 (Mona-Lisa): target " << targetFile << " (" << tgt.width << "x"
            << tgt.height << ")" << '\n'
            << GLOGGING;

    // ---- register the GPU consumer builder; select it with "--consumer gpu" -------------------
    // The GPU consumer is now a first-class, mnemonic-selectable consumer: run with "--consumer gpu" to
    // evaluate on the device (backend cpu/cuda chosen in GGPUConsumer.json), or with any other
    // consumer (e.g. the default "--consumer stc") to evaluate on the CPU via the individual's
    // fitnessCalculation(). We only contribute the problem-specific piece -- a closure that builds the
    // device marshaller + consumer; Go2 owns selection and lifecycle. The closure is invoked lazily at
    // optimize() (after the target is loaded), only when gpu is selected. GGPUConsumerT evaluates a whole
    // generation in one bulk launch via the marshaller.
    go.registerGPUConsumerBuilder([consumerConfig]() {
        auto marshaller = std::make_shared<MonaLisa::GMonaLisaGPUMarshaller>();
        auto consumer =
            std::make_shared<gpu::GGPUConsumerT<gen::GOptimizableEntity, gimage_fp_t>>(consumerConfig, marshaller);
        // The clone-on-partial-return policy used by the evolutionary algorithm needs a polymorphic clone.
        consumer->setCloneFunction([](const std::unique_ptr<gen::GOptimizableEntity> &p) {
            return p->clone_unique();
        });
        return std::shared_ptr<Gem::Courtier::GBaseConsumerT<gen::GOptimizableEntity>>(consumer);
    });

    // ---- as this is a server, allow interrupting the run "on the fly" -------------------------
    signal(G_SIGHUP, Gem::Geneva::sigHupHandler);

    // ---- register the image-emitting pluggable optimization monitor --------------------------
    if(logImages) {
        go.registerPluggableOM(std::make_shared<GImagePOM>("./results/", emitBestOnly));
    }

    // ---- create the initial individual from its factory and add it to Go2 ---------------------
    GImageIndividualFactory f("config/GImageIndividual.json");
    auto initial = f.get_as<GImageIndividual>();
    go.push_back(initial);

    // The genome carries only structure; its main + location Gauss adaptors live on an OA-owned config the
    // factory authors from the configuration. Register it for the only adapting algorithm we may select: the
    // dimension-aware evolutionary algorithm (ea). The from-scratch sep-CMA strategy (sepcma) manages
    // its own search distribution and needs no adaption config.
    go.registerAdaptionConfig("PERSONALITY_EA", f.getAdaptionConfig(*initial));

    // ---- run the optimization with the algorithm chosen by mnemonic --------------------------
    // Default is "ea"; override on the command line to compare against the from-scratch
    // sep-CMA strategy, e.g.  --optimizationAlgorithms "sepcma".
    go.registerDefaultAlgorithm("ea");

    // Perform the optimization. The per-iteration picture (the useful output) is written by GImagePOM.
    std::shared_ptr<GImageIndividual> best = go.optimize()->getBestGlobalIndividual<GImageIndividual>();

    glogger << "Example 15 finished. Best fitness: " << best->raw_fitness(0) << '\n' << GLOGGING;

    return 0;
}
