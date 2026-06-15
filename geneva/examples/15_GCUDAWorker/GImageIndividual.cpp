/**
 * @file GImageIndividual.cpp
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

#include "GImageIndividual.hpp"
#include "GMonaLisaProblem.hpp"

#include "geneva/oa/GAdaption.hpp"

#ifdef GEM_TESTING
#include <catch2/catch_test_macros.hpp>
#endif /* GEM_TESTING */

BOOST_CLASS_EXPORT_IMPLEMENT(Gem::Geneva::GImageIndividual) // NOLINT
namespace Gem::Geneva {
/******************************************************************************/
//////////////////////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/
// Add a comparison-operator for our tests
bool operator==(const CircleTriangle &lhs, const CircleTriangle &rhs) {
    return (
        (lhs.r == rhs.r) && (lhs.g == rhs.g) && (lhs.b == rhs.b) && (lhs.a == rhs.a) &&
        (lhs.cx == rhs.cx) && (lhs.cy == rhs.cy) && (lhs.radius == rhs.radius) &&
        (lhs.angle1 == rhs.angle1) && (lhs.angle2 == rhs.angle2) && (lhs.angle3 == rhs.angle3)
    );
}

// Output operator for the CircleStruct
std::ostream &operator<<(std::ostream &os, const CircleTriangle &ct) {
    os << "CircleTriangle(" << '\n';
    os << "r=" << static_cast<int>(ct.r) << ", " << '\n';
    os << "g=" << static_cast<int>(ct.g) << ", " << '\n';
    os << "b=" << static_cast<int>(ct.b) << ", " << '\n';
    os << "a=" << static_cast<int>(ct.a) << ", " << '\n';
    os << "cx=" << ct.cx << ", " << '\n';
    os << "cy=" << ct.cy << ", " << '\n';
    os << "radius=" << ct.radius << ", " << '\n';
    os << "angle1=" << ct.angle1 << ", " << '\n';
    os << "angle2=" << ct.angle2 << ", " << '\n';
    os << "angle3=" << ct.angle3 << '\n';
    os << ")" << '\n' << '\n';
    return os;
}

/******************************************************************************/
//////////////////////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/
/**
	 * Fills the individual with parameters. Our parameter set consists of nTriangles GParameterObjectCollection
	 * objects. Each holds 10 parameters:
	 * - a GConstrainedDoubleCollection holding the middle of a circle
	 * - a single GConstrainedDoubleObject for the radius
	 * - a GConstrainedDoubleCollection holding three angles which point to the corners of the triangle (on the circles edge).
	 * - a GConstrainedDoubleCollection for the three colors
	 * - a single GConstrainedDoubleObject for the alpha channel
	 *
	 * @param startSize The initial size of triangles
	 * @param minSize The minimum size of triangles
	 * @param maxSize The maximum size of triangles
	 * @param sigma The step-width used for Gauss-Adaptions
	 * @param sigmaSigma Indicates the level of adaption of sigma
	 * @param minSigma The minimum allowed value for sigma
	 * @param maxSigma The maximum allowed value for sigma
	 * @param minOpaqueness The minimum opaqueness allowed for objects
	 * @param maxOpaqueness The maximum opaqueness allowed for objects
	 * @param adProb Specifies the adaption probability for adaptors
	 * @param nTriangles The number of triangles constituting a candidate image
	 * @param alphaSort Whether triangles should be sorted according to their alpha channel
	 */
void GImageIndividual::init(
    const std::size_t &nTriangles,
    const double &bgRed,
    const double &bgGreen,
    const double &bgBlue,
    const double &startSize,
    const double &minSize,
    const double &maxSize,
    const double &minOpaqueness,
    const double &maxOpaqueness,
    const bool &alphaSort,
    const bool &changeBGColor,
    const bool &mutateAlphaChannel,
    const double &sigma,
    const double &sigmaSigma,
    const double &minSigma,
    const double &maxSigma,
    const double &adProb,
    const double &adaptAdProb,
    const double &minAdProb,
    const double &maxAdProb,
    const double &loc_sigma,
    const double &loc_sigmaSigma,
    const double &loc_minSigma,
    const double &loc_maxSigma,
    const double &loc_adProb,
    const double &loc_adaptAdProb,
    const double &loc_minAdProb,
    const double &loc_maxAdProb
) {
    if(minSize < 0. || maxSize > 1. || minSize >= maxSize) {
        throw geneva_exception(
            g_error_streamer(DO_LOG, Gem::Common::timeAndPlace())
            << "In GImageIndividual::init() : Error!" << '\n'
            << "Invalid values for minSize and maxSize provided: " << minSize << " / " << maxSize
            << '\n'
        );
    }

    // A startSize < 0 means random initialization in the range [minSize, maxSize]
    if(startSize >= 0. && startSize < minSize) {
        // Cannot be < 0 as minSize may not be <= 0
        throw geneva_exception(
            g_error_streamer(DO_LOG, Gem::Common::timeAndPlace())
            << "In GImageIndividual::init() : Error!" << '\n'
            << "Invalid values for minSize and startSize provided: " << minSize << " / "
            << startSize << '\n'
        );
    }

    if(startSize > maxSize) {
        throw geneva_exception(
            g_error_streamer(DO_LOG, Gem::Common::timeAndPlace())
            << "In GImageIndividual::init() : Error!" << '\n'
            << "Invalid values for maxSize and startSize provided: " << maxSize << " / "
            << startSize << '\n'
        );
    }

    if(adaptAdProb < 0. || adaptAdProb > 1.) {
        throw geneva_exception(
            g_error_streamer(DO_LOG, Gem::Common::timeAndPlace())
            << "In GImageIndividual::init() : Error!" << '\n'
            << "Invalid value for adaptAdProb provided: " << adaptAdProb << '\n'
        );
    }

    if(loc_adaptAdProb < 0. || loc_adaptAdProb > 1.) {
        throw geneva_exception(
            g_error_streamer(DO_LOG, Gem::Common::timeAndPlace())
            << "In GImageIndividual::init() : Error!" << '\n'
            << "Invalid value for loc_adaptAdProb provided: " << loc_adaptAdProb << '\n'
        );
    }

    if(minAdProb >= maxAdProb || minAdProb < 0. || maxAdProb > 1. || adProb < minAdProb ||
       adProb > maxAdProb) {
        throw geneva_exception(
            g_error_streamer(DO_LOG, Gem::Common::timeAndPlace())
            << "In GImageIndividual::init() : Error!" << '\n'
            << "Invalid values for minAdprob, maxAdProb or adProb provided: " << minAdProb << " / "
            << maxAdProb << " / " << adProb << '\n'
        );
    }

    if(loc_minAdProb >= loc_maxAdProb || loc_minAdProb < 0. || loc_maxAdProb > 1. ||
       loc_adProb < loc_minAdProb || loc_adProb > loc_maxAdProb) {
        throw geneva_exception(
            g_error_streamer(DO_LOG, Gem::Common::timeAndPlace())
            << "In GImageIndividual::init() : Error!" << '\n'
            << "Invalid values for loc_minAdprob, loc_maxAdProb or loc_adProb provided: "
            << loc_minAdProb << " / " << loc_maxAdProb << " / " << loc_adProb << '\n'
        );
    }

    nTriangles_ = nTriangles;
    alphaSort_ = alphaSort;
    mutateAlphaChannel_ = mutateAlphaChannel;

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // Create suitable adaptors

    // Build the flat genome via GGenomeBuilder. The streamline order matches the historical tree's
    // push_back order exactly -- per triangle: cx, cy, radius, angle1..3, r, g, b, a (10 values), then
    // the 3 background colours -- so getTriangleData() / getBackGroundColor() / the GPU marshaller read
    // it positionally and the rasteriser is unchanged. Every value is a constrained gimage_fp_t with
    // its own Gauss group (the tree gave each parameter its own cloned adaptor); the location params
    // (cx, cy) use the "loc" adaptor config, everything else the main one.
    changeBGColor_ = changeBGColor;

    gpar::GGenomeBuilder bld;

    // Adds one constrained gimage_fp_t parameter, dispatching to the matching builder channel.
    auto addParam = [&bld](gimage_fp_t init, gimage_fp_t lo, gimage_fp_t hi) {
        if constexpr(std::is_same_v<gimage_fp_t, float>) {
            return bld.addFloat(init, lo, hi);
        }
        else {
            return bld.addDouble(init, lo, hi);
        }
    };
    // The Gauss adaptors live on the OA-owned config (built below), not the genome layout. Tag each group
    // with its adaptor class -- "main" (size / angles / colours / alpha / background) or "loc" (centre x/y)
    // -- so the config can author the matching adaptor onto them by label.
    auto mainGauss = [](gpar::ParamHandle<gimage_fp_t> &h) { h.label("main"); };
    auto locGauss = [](gpar::ParamHandle<gimage_fp_t> &h) { h.label("loc"); };

    for(std::size_t t_cnt = 0; t_cnt < nTriangles_; t_cnt++) {
        // middle-x and -y: the location adaptor.
        auto cx = addParam(gimage_fp_t(0), gimage_fp_t(0), gimage_fp_t(1));
        locGauss(cx);
        auto cy = addParam(gimage_fp_t(0), gimage_fp_t(0), gimage_fp_t(1));
        locGauss(cy);

        // radius: startSize >= 0 seeds it (else random init below); the main adaptor.
        const gimage_fp_t radius_init =
            static_cast<gimage_fp_t>(startSize >= 0. ? startSize : minSize);
        auto rad = addParam(
            radius_init, static_cast<gimage_fp_t>(minSize), static_cast<gimage_fp_t>(maxSize)
        );
        mainGauss(rad);

        // three angles.
        auto a1 = addParam(gimage_fp_t(0), gimage_fp_t(0), gimage_fp_t(1));
        mainGauss(a1);
        auto a2 = addParam(gimage_fp_t(0), gimage_fp_t(0), gimage_fp_t(1));
        mainGauss(a2);
        auto a3 = addParam(gimage_fp_t(0), gimage_fp_t(0), gimage_fp_t(1));
        mainGauss(a3);

        // three colours.
        auto cr = addParam(gimage_fp_t(0), gimage_fp_t(0), gimage_fp_t(1));
        mainGauss(cr);
        auto cg = addParam(gimage_fp_t(0), gimage_fp_t(0), gimage_fp_t(1));
        mainGauss(cg);
        auto cb = addParam(gimage_fp_t(0), gimage_fp_t(0), gimage_fp_t(1));
        mainGauss(cb);

        // alpha channel: frozen at maxOpaqueness unless alpha mutation is enabled.
        auto ca = addParam(
            static_cast<gimage_fp_t>(maxOpaqueness),
            static_cast<gimage_fp_t>(minOpaqueness),
            static_cast<gimage_fp_t>(maxOpaqueness)
        );
        mainGauss(ca);
        if(not mutateAlphaChannel) {
            ca.adaptionMode(adaptionMode::NEVER);
        }
    }

    //---------------------------------------------------------------------------
    // Add the three background colours (the last three values of the genome). A negative bg* means
    // random init; otherwise it seeds the value. When background colours are not adapted, the groups
    // are inactive (NEVER) and keep their seed across the whole population (randomInit skips them).
    auto addBg = [&](double bgVal) {
        const gimage_fp_t init = static_cast<gimage_fp_t>(bgVal >= 0. ? bgVal : 0.);
        auto h = addParam(init, gimage_fp_t(0), gimage_fp_t(1));
        mainGauss(h);
        if(not changeBGColor) {
            h.adaptionMode(adaptionMode::NEVER);
        }
    };
    addBg(bgRed);
    addBg(bgGreen);
    addBg(bgBlue);

    std::cout << (changeBGColor ? "Background colors will be adapted"
                                : "Background colors will not be adapted")
              << '\n';

    // Install the value arrays + shared layout, then randomly initialise the ACTIVE parameters within
    // their bounds (mirroring the tree's random-init constructors). Inactive groups (frozen alpha /
    // frozen background) keep their seeds.
    this->setGenome(bld.build());
    this->randomInit(activityMode::ACTIVEONLY);

    // Author the OA-owned adaption config from the labelled genome: the location adaptor on the "loc"
    // (centre) groups and the main adaptor on every "main" group. The frozen alpha / background groups are
    // labelled "main" too but were built adaptionMode::NEVER (inactive), so the adaption kernel skips them
    // -- exactly as the former per-group gaussAdaptor + NEVER did.
    namespace oa = Gem::Geneva::OptimizationAlgorithms;
    adaption_config_ = oa::makeAdaptionConfig<oa::GAdaptionConfigBase>(*this);
    adaption_config_->forLabel("loc").gauss(
        loc_sigma, loc_sigmaSigma, loc_minSigma, loc_maxSigma, loc_adProb, loc_adaptAdProb, 1,
        adaptionMode::WITHPROBABILITY, loc_minAdProb, loc_maxAdProb
    );
    adaption_config_->forLabel("main").gauss(
        sigma, sigmaSigma, minSigma, maxSigma, adProb, adaptAdProb, 1, adaptionMode::WITHPROBABILITY,
        minAdProb, maxAdProb
    );
}

/** @brief Allows an external entity to set our fitness */
void GImageIndividual::setFitness(std::vector<double> const &result_vec) {
    this->setFitness_(result_vec);
}

/***************************************************************************/
/**
	 * Searches for compliance with expectations with respect to another object
	 * of the same type
	 *
	 * @param cp A constant reference to another GTreeGenome object
	 * @param e The expected outcome of the comparison
	 * @param limit The maximum deviation for floating point values (important for similarity checks)
	 */
void GImageIndividual::compare_(
    const gpar::GOptimizableEntity &cp,
    const Gem::Common::expectation &e,
    const double &limit
) const {
    using namespace Gem::Common;

    // Check that we are dealing with a GImageIndividual reference independent of this object and convert the pointer
    const GImageIndividual *p_load =
        Gem::Common::g_convert_and_compare<gpar::GOptimizableEntity, GImageIndividual>(cp, this);

    GToken token("GImageIndividual", e);

    // Compare our parent data ...
    Gem::Common::compare_base_t<gpar::GFlatGenome>(*this, *p_load, token);

    // ... and then the local data
    Gem::Common::compare_t(IDENTITY(width_, p_load->width_), token);
    Gem::Common::compare_t(IDENTITY(height_, p_load->height_), token);
    Gem::Common::compare_t(IDENTITY(nTriangles_, p_load->nTriangles_), token);
    Gem::Common::compare_t(IDENTITY(alphaSort_, p_load->alphaSort_), token);
    Gem::Common::compare_t(IDENTITY(changeBGColor_, p_load->changeBGColor_), token);
    Gem::Common::compare_t(IDENTITY(mutateAlphaChannel_, p_load->mutateAlphaChannel_), token);

    // React on deviations from the expectation
    token.evaluate();
}

/*******************************************************************************************/
/**
	 * Retrieves the number of triangles
	 */
std::size_t GImageIndividual::getNTriangles() const {
    return nTriangles_;
}

/*******************************************************************************************/
/**
	 * Checks whether background colors shall be changed
	 *
	 * @return A boolean indicating whether background colors shall be changed
	 */
bool GImageIndividual::getChangeBGColor() const {
    return changeBGColor_;
}

/*******************************************************************************************/
/**
	 * Checks whether the alpha channel of triangles shall be mutated
	 *
	 * @return A boolean indicating whether the alpha-channel of triangles shall be mutated
	 */
bool GImageIndividual::getMutateAlphaChannel() const {
    return mutateAlphaChannel_;
}

/*******************************************************************************************/
/**
	 * Retrieve an array with the triangles' data, using the circular triangle definition.
	 * Note that this array might be sorted in ascending order of opacity and might thus not be
	 * identical to the order in which triangles are sorted in this individual.
	 *
	 * @return An array with the triangle data
	 */
std::vector<CircleTriangle> GImageIndividual::getTriangleData() const {
    // The flat genome stores 10 contiguous values per triangle in the canonical render order, so each
    // triangle's fields are read positionally from the streamlined value array (10*nTriangles + 3 bg).
    std::vector<gimage_fp_t> parVec;
    this->streamline(parVec);

#ifdef DEBUG
    if(parVec.size() != 10 * nTriangles_ + 3) {
        // including background color
        throw geneva_exception(
            g_error_streamer(DO_LOG, Gem::Common::timeAndPlace())
            << "In GImageIndividual::getTriangleData(): Error!" << '\n'
            << "Invalid number of parameters in this class " << parVec.size() << " / "
            << (10 * nTriangles_ + 3) << '\n'
        );
    }
#endif /* DEBUG */

    std::size_t offset = 0;
    std::vector<CircleTriangle> circle_cnt(nTriangles_);
    for(std::size_t i = 0; i < nTriangles_; i++) {
        offset = i * 10;

        circle_cnt[i].cx = parVec[offset + 0];
        circle_cnt[i].cy = parVec[offset + 1];
        circle_cnt[i].radius = parVec[offset + 2];

        circle_cnt[i].angle1 = parVec[offset + 3];
        circle_cnt[i].angle2 = parVec[offset + 4];
        circle_cnt[i].angle3 = parVec[offset + 5];

        circle_cnt[i].r = parVec[offset + 6];
        circle_cnt[i].g = parVec[offset + 7];
        circle_cnt[i].b = parVec[offset + 8];
        circle_cnt[i].a = parVec[offset + 9];
    }

    if(alphaSort_) {
        // Sort circle_cnt so that items with higher opacity are in the front position
        // As a result, they will be drawn first
        std::ranges::sort(circle_cnt, [](const CircleTriangle &x, const CircleTriangle &y) {
            return x.a > y.a;
        });
    }

    return circle_cnt;
}

/******************************************************************************/
/**
	 * Loads the data of another GImageIndividual, camouflaged as a GFlatGenome.
	 *
	 * @param cp A copy of another GImageIndividual, camouflaged as a GFlatGenome
	 */
void GImageIndividual::load_(const gpar::GOptimizableEntity *cp) {
    // Check that we are indeed dealing with a GImageIndividual reference
    const GImageIndividual *p_load =
        Gem::Common::g_convert_and_compare<gpar::GOptimizableEntity, GImageIndividual>(cp, this);

    // Load our parent's data
    gpar::GFlatGenome::load_(cp);

    // Load local data
    nTriangles_ = p_load->nTriangles_;
    alphaSort_ = p_load->alphaSort_;
    changeBGColor_ = p_load->changeBGColor_;
    mutateAlphaChannel_ = p_load->mutateAlphaChannel_;
    width_ = p_load->width_;
    height_ = p_load->height_;
    adaption_config_ = p_load->adaption_config_; // share the OA-owned config (transient, read-only)
}

/******************************************************************************/
/**
	 * Creates a deep clone of this object
	 *
	 * @return A deep clone of this object, camouflaged as a GFlatGenome
	 */
gpar::GFlatGenome *GImageIndividual::clone_() const {
    return new GImageIndividual(*this);
}

/******************************************************************************/
/**
	 * The actual fitness calculation takes place here.
	 *
	 * @return The value of this object
	 */
double GImageIndividual::fitnessCalculation() {
    // The host fitness is computed on the CPU via the SAME render+score the GPU kernel uses
    // (Gem::Geneva::MonaLisa::score, shared in GMonaLisaProblem.hpp). This makes the individual
    // evaluable purely on the CPU -- to cross-check the GPU result and compare speed -- while the
    // GGPUConsumer path uses the device kernel. Requires the target image to have been loaded
    // (Gem::Geneva::MonaLisa::loadTarget) beforehand.
    // The genome scalar type is selected at compile time (gimage_fp_t); streamline<gimage_fp_t>
    // is required -- streamline<float> collects nothing from a double genome and vice-versa.
    std::vector<gimage_fp_t> parVec;
    this->streamline(parVec);
    return Gem::Geneva::MonaLisa::scoreAgainstTarget(parVec.data(), static_cast<int>(parVec.size()));
}

/******************************************************************************/
/**
	 * Applies modifications to this object. This is needed for testing purposes
	 *
	 * @return A boolean which indicates whether modifications were made
	 */
bool GImageIndividual::modify_GUnitTests_() {
#ifdef GEM_TESTING

    // Call the parent classes' functions
    gpar::GFlatGenome::modify_GUnitTests();

    // Change the parameter settings. The adaption state + logic are OA-owned (Phase 10); a standalone
    // individual drives them via a self-owned scratch + the config it authored in init() (StandaloneAdapter).
    if(adaption_config_) {
        Gem::Geneva::OptimizationAlgorithms::StandaloneAdapter(*this, adaption_config_).adapt(*this);
    }

    return true;
#else /* GEM_TESTING */ // If this function is called when GEM_TESTING isn't set, throw
    condnotset("GImageIndividual::modify_GUnitTests", "GEM_TESTING");
    return false;
#endif                  /* GEM_TESTING */
}

/******************************************************************************/
/**
	 * Performs self tests that are expected to succeed. This is needed for testing purposes
	 */
void GImageIndividual::specificTestsNoFailureExpected_GUnitTests_() {
#ifdef GEM_TESTING
    using namespace Gem::Geneva;

    // Call the parent classes' functions
    gpar::GFlatGenome::specificTestsNoFailureExpected_GUnitTests();

    const std::size_t NTESTS = 100;

    //------------------------------------------------------------------------------

    {
        // Test that repeated extraction of an object's data results in the same output
        const std::shared_ptr<GImageIndividual> p_test = this->clone<GImageIndividual>();

        const auto circles = p_test->getTriangleData();

        for(std::size_t i = 0; i < NTESTS; i++) {
            auto circles_new = p_test->getTriangleData();
            REQUIRE(circles_new == circles);
        }
    }

    //------------------------------------------------------------------------------
#else /* GEM_TESTING */ // If this function is called when GEM_TESTING isn't set, throw
    condnotset("GImageIndividual::modify_GUnitTests", "GEM_TESTING");
#endif                  /* GEM_TESTING */
}

/******************************************************************************/
/**
	 * Performs self tests that are expected to fail. This is needed for testing purposes
	 */
void GImageIndividual::specificTestsFailuresExpected_GUnitTests_() {
#ifdef GEM_TESTING
    using namespace Gem::Geneva;

    // Call the parent classes' functions
    gpar::GFlatGenome::specificTestsFailuresExpected_GUnitTests();

    //------------------------------------------------------------------------------
    //------------------------------------------------------------------------------

#else /* GEM_TESTING */ // If this function is called when GEM_TESTING isn't set, throw
    condnotset("GImageIndividual::modify_GUnitTests", "GEM_TESTING");
#endif                  /* GEM_TESTING */
}

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/
/**
	 * A constructor with the ability to switch the parallelization mode. It initializes a
	 * target item as needed.
	 *
	 * @param configFile The name of the configuration file
	 */
GImageIndividualFactory::GImageIndividualFactory(const std::string &configFile)
  : Gem::Common::GFactoryT<GImageIndividual>(configFile) {
    /* nothing */
}

/******************************************************************************/
/**
	 * Creates items of this type
	 *
	 * @return Items of the desired type
	 */
std::shared_ptr<GImageIndividual>
GImageIndividualFactory::getObject_(Gem::Common::GParserBuilder &gpb, const std::size_t &id) {
    // Will hold the result
    std::shared_ptr<GImageIndividual> target(new GImageIndividual());

    // Make the object's local configuration options known
    target->addConfigurationOptions(gpb);

    return target;
}

/******************************************************************************/
/**
	 * Allows to describe local configuration options for the image individual
	 */
void GImageIndividualFactory::describeLocalOptions_(Gem::Common::GParserBuilder &gpb) {
    // Describe our own options
    using namespace Gem::Courtier;
    using namespace Gem::Common;

    std::string comment;

    comment = "";
    comment += "The minimum size of the triangle in percent of the canvas;";
    comment += "The allowed value range is [0,maxSize[;";
    gpb.registerFileParameter<double>(
        "min_size",
        minSize_.reference(),
        GII_DEF_MINSIZE,
        Gem::Common::VAR_IS_ESSENTIAL,
        comment
    );
    checkValueRange(minSize_.value(), 0., 1., GFPLOWERCLOSED, GFPUPPEROPEN, GFNOWARNING, "min_size");

    comment = "";
    comment += "The maximum size of the triangle in percent of the canvas;";
    comment += "The allowed value range is ]minSize,1];";
    gpb.registerFileParameter<double>(
        "max_size",
        maxSize_.reference(),
        GII_DEF_MAXSIZE,
        Gem::Common::VAR_IS_ESSENTIAL,
        comment
    );
    checkValueRange(
        maxSize_.value(),
        minSize_.value(),
        1.,
        GFPLOWEROPEN,
        GFPUPPEROPEN,
        GFNOWARNING,
        "max_size"
    );

    comment = "";
    comment += "The start size of the triangle in percent of the canvas;";
    comment += "The allowed value range is [minSize,maxSize];";
    comment += "A value < 0 means random in the range [minSize,maxSize];";
    gpb.registerFileParameter<double>(
        "start_size",
        startSize_.reference(),
        GII_DEF_MINSIZE,
        Gem::Common::VAR_IS_ESSENTIAL,
        comment
    );

    // A value < 0 means random in the range [minSize,maxSize]
    if(startSize_.value() >= 0.) {
        checkValueRange(
            startSize_.value(),
            minSize_.value(),
            maxSize_.value(),
            GFPLOWERCLOSED,
            GFPUPPEROPEN,
            GFNOWARNING,
            "start_size"
        );
    }

    comment = "";
    comment += "The minimum allowed opaqueness of triangles;";
    comment += "The allowed value range is [0,maxOpaqueness];";
    gpb.registerFileParameter<double>(
        "min_opaqueness",
        minOpaqueness_.reference(),
        GII_DEF_MINOPAQUENESS,
        Gem::Common::VAR_IS_ESSENTIAL,
        comment
    );
    checkValueRange(
        minOpaqueness_.value(),
        0.,
        1.,
        GFPLOWERCLOSED,
        GFPUPPEROPEN,
        GFNOWARNING,
        "min_opaqueness"
    );

    comment = "";
    comment += "The maximum allowed opaqueness of triangles;";
    comment += "The allowed value range is [minOpaqueness,1];";
    gpb.registerFileParameter<double>(
        "max_opaqueness",
        maxOpaqueness_.reference(),
        GII_DEF_MAXOPAQUENESS,
        Gem::Common::VAR_IS_ESSENTIAL,
        comment
    );
    checkValueRange(
        maxOpaqueness_.value(),
        minOpaqueness_.value(),
        1.,
        GFPLOWEROPEN,
        GFPUPPEROPEN,
        GFNOWARNING,
        "max_opaqueness"
    );

    comment = "";
    comment +=
        "Determines the rate of adaption of adProb. Set to 0, if you do not need this feature;";
    gpb.registerFileParameter<double>(
        "adapt_ad_prob",
        adaptAdProb_.reference(),
        GII_DEF_ADAPTADPROB,
        Gem::Common::VAR_IS_ESSENTIAL,
        comment
    );

    comment = "";
    comment += "Determines the rate of adaption of location-adProb. Set to 0, if you do not need "
               "this feature;";
    gpb.registerFileParameter<double>(
        "loc_adapt_ad_prob",
        loc_adaptAdProb_.reference(),
        GII_DEF_LOC_ADAPTADPROB,
        Gem::Common::VAR_IS_ESSENTIAL,
        comment
    );

    comment = "";
    comment += "The lower allowed boundary for adProb-variation;";
    gpb.registerFileParameter<double>(
        "min_ad_prob",
        minAdProb_.reference(),
        GII_DEF_MINADPROB,
        Gem::Common::VAR_IS_ESSENTIAL,
        comment
    );
    checkValueRange(
        minAdProb_.value(),
        0.,
        1.,
        GFPLOWERCLOSED,
        GFPUPPERCLOSED,
        GFNOWARNING,
        "min_ad_prob"
    );

    comment = "";
    comment += "The upper allowed boundary for adProb-variation;";
    gpb.registerFileParameter<double>(
        "max_ad_prob",
        maxAdProb_.reference(),
        GII_DEF_MAXADPROB,
        Gem::Common::VAR_IS_ESSENTIAL,
        comment
    );
    checkValueRange(
        maxAdProb_.value(),
        minAdProb_.value(),
        1.,
        GFPLOWERCLOSED,
        GFPUPPERCLOSED,
        GFNOWARNING,
        "max_ad_prob"
    );

    comment = "";
    comment += "The lower allowed boundary for loc_adProb-variation;";
    gpb.registerFileParameter<double>(
        "loc_min_ad_prob",
        loc_minAdProb_.reference(),
        GII_DEF_LOC_MINADPROB,
        Gem::Common::VAR_IS_ESSENTIAL,
        comment
    );
    checkValueRange(
        loc_minAdProb_.value(),
        0.,
        1.,
        GFPLOWERCLOSED,
        GFPUPPERCLOSED,
        GFNOWARNING,
        "loc_min_ad_prob"
    );

    comment = "";
    comment += "The upper allowed boundary for loc_adProb-variation;";
    gpb.registerFileParameter<double>(
        "loc_max_ad_prob",
        loc_maxAdProb_.reference(),
        GII_DEF_LOC_MAXADPROB,
        Gem::Common::VAR_IS_ESSENTIAL,
        comment
    );
    checkValueRange(
        loc_maxAdProb_.value(),
        loc_minAdProb_.value(),
        1.,
        GFPLOWERCLOSED,
        GFPUPPERCLOSED,
        GFNOWARNING,
        "loc_max_ad_prob"
    );

    comment = "";
    comment += "The probability for random adaptions of values in evolutionary algorithms;";
    comment += "The allowed value range is [0,1];";
    gpb.registerFileParameter<double>(
        "ad_prob",
        adProb_.reference(),
        GII_DEF_ADPROB,
        Gem::Common::VAR_IS_ESSENTIAL,
        comment
    );
    checkValueRange(
        adProb_.value(),
        minAdProb_.value(),
        maxAdProb_.value(),
        GFPLOWERCLOSED,
        GFPUPPERCLOSED,
        GFNOWARNING,
        "ad_prob"
    );

    comment = "";
    comment += "The probability for random adaptions of location parameters of values in "
               "evolutionary algorithms;";
    comment += "The allowed value range is [0,1];";
    gpb.registerFileParameter<double>(
        "loc_ad_prob",
        loc_adProb_.reference(),
        GII_DEF_LOC_ADPROB,
        Gem::Common::VAR_IS_ESSENTIAL,
        comment
    );
    checkValueRange(
        loc_adProb_.value(),
        loc_minAdProb_.value(),
        loc_maxAdProb_.value(),
        GFPLOWERCLOSED,
        GFPUPPERCLOSED,
        GFNOWARNING,
        "loc_ad_prob"
    );

    comment = "";
    comment += "The sigma for gauss-adaption in ES;";
    comment += "sigma must be positive;";
    comment += "Recommended value range [0,1];";
    gpb.registerFileParameter<double>(
        "sigma",
        sigma_.reference(),
        GII_DEF_SIGMA,
        Gem::Common::VAR_IS_ESSENTIAL,
        comment
    );
    checkValueRange(sigma_.value(), 0., 1., GFPLOWERCLOSED, GFPUPPEROPEN, GFNOWARNING, "sigma");

    comment = "";
    comment += "The minimum value of sigma;";
    comment += "minSigma must be positive and smaller than maxSigma;";
    gpb.registerFileParameter<double>(
        "min_sigma",
        minSigma_.reference(),
        GII_DEF_MINSIGMA,
        Gem::Common::VAR_IS_ESSENTIAL,
        comment
    );
    checkValueRange(
        minSigma_.value(),
        0.,
        1.,
        GFPLOWERCLOSED,
        GFPUPPEROPEN,
        GFNOWARNING,
        "min_sigma"
    );

    comment = "";
    comment += "The maximum value of sigma;";
    comment += "maxSigma must be positive and larger than minSigma;";
    gpb.registerFileParameter<double>(
        "max_sigma",
        maxSigma_.reference(),
        GII_DEF_MAXSIGMA,
        Gem::Common::VAR_IS_ESSENTIAL,
        comment
    );
    checkValueRange(
        maxSigma_.value(),
        minSigma_.value(),
        1.,
        GFPLOWEROPEN,
        GFPUPPEROPEN,
        GFNOWARNING,
        "max_sigma"
    );

    comment = "";
    comment += "Influences the self-adaption of gauss-mutation in ES;";
    comment += "sigmaSigma must be positive;";
    comment += "The allowed value range is [0,1];";
    gpb.registerFileParameter<double>(
        "sigma_sigma",
        sigmaSigma_.reference(),
        GII_DEF_SIGMASIGMA,
        Gem::Common::VAR_IS_ESSENTIAL,
        comment
    );
    checkValueRange(
        sigmaSigma_.value(),
        0.,
        1.,
        GFPLOWERCLOSED,
        GFPUPPERCLOSED,
        GFNOWARNING,
        "sigma_sigma"
    );

    comment = "";
    comment += "The sigma for gauss-adaption of location parameters in ES;";
    comment += "loc_sigma must be positive;";
    comment += "Recommended value range [0,1];";
    gpb.registerFileParameter<double>(
        "loc_sigma",
        loc_sigma_.reference(),
        GII_DEF_LOC_SIGMA,
        Gem::Common::VAR_IS_ESSENTIAL,
        comment
    );
    checkValueRange(
        loc_sigma_.value(),
        0.,
        1.,
        GFPLOWERCLOSED,
        GFPUPPEROPEN,
        GFNOWARNING,
        "loc_sigma"
    );

    comment = "";
    comment += "The minimum value of sigma for location parameters;";
    comment += "loc_minSigma must be positive and smaller than loc_maxSigma;";
    gpb.registerFileParameter<double>(
        "loc_min_sigma",
        loc_minSigma_.reference(),
        GII_DEF_LOC_MINSIGMA,
        Gem::Common::VAR_IS_ESSENTIAL,
        comment
    );
    checkValueRange(
        loc_minSigma_.value(),
        0.,
        1.,
        GFPLOWERCLOSED,
        GFPUPPEROPEN,
        GFNOWARNING,
        "loc_min_sigma"
    );

    comment = "";
    comment += "The maximum value of sigma for location parameters;";
    comment += "loc_maxSigma must be positive and larger than loc_minSigma;";
    gpb.registerFileParameter<double>(
        "loc_max_sigma",
        loc_maxSigma_.reference(),
        GII_DEF_LOC_MAXSIGMA,
        Gem::Common::VAR_IS_ESSENTIAL,
        comment
    );
    checkValueRange(
        loc_maxSigma_.value(),
        loc_minSigma_.value(),
        1.,
        GFPLOWEROPEN,
        GFPUPPEROPEN,
        GFNOWARNING,
        "loc_max_sigma"
    );

    comment = "";
    comment += "Influences the self-adaption of gauss-mutation in ES for location parameters;";
    comment += "loc_sigmaSigma must be positive;";
    comment += "The allowed value range is [0,1];";
    gpb.registerFileParameter<double>(
        "loc_sigma_sigma",
        loc_sigmaSigma_.reference(),
        GII_DEF_LOC_SIGMASIGMA,
        Gem::Common::VAR_IS_ESSENTIAL,
        comment
    );
    checkValueRange(
        loc_sigmaSigma_.value(),
        0.,
        1.,
        GFPLOWERCLOSED,
        GFPUPPERCLOSED,
        GFNOWARNING,
        "loc_sigma_sigma"
    );

    comment = "";
    comment += "The initial background color (red channel);";
    comment += "Negative values mean random initialization;";
    comment += "Otherwise the allowed value range is [0.,1.];";
    gpb.registerFileParameter<double>(
        "bg_red",
        bgRed_.reference(),
        GII_DEF_BGRED,
        Gem::Common::VAR_IS_ESSENTIAL,
        comment
    );
    checkValueRange(bgRed_.value(), 0., 1., GFPLOWERCLOSED, GFPUPPEROPEN, GFNOWARNING, "bg_red");

    comment = "";
    comment += "The initial background color (green channel);";
    comment += "Negative values mean random initialization;";
    comment += "Otherwise the allowed value range is [0.,1.];";
    gpb.registerFileParameter<double>(
        "bg_green",
        bgGreen_.reference(),
        GII_DEF_BGGREEN,
        Gem::Common::VAR_IS_ESSENTIAL,
        comment
    );
    checkValueRange(bgGreen_.value(), 0., 1., GFPLOWERCLOSED, GFPUPPEROPEN, GFNOWARNING, "bg_green");

    comment = "";
    comment += "The initial background color (blue channel);";
    comment += "Negative values mean random initialization;";
    comment += "Otherwise the allowed value range is [0.,1.];";
    gpb.registerFileParameter<double>(
        "bg_blue",
        bgBlue_.reference(),
        GII_DEF_BGBLUE,
        Gem::Common::VAR_IS_ESSENTIAL,
        comment
    );
    checkValueRange(bgBlue_.value(), 0., 1., GFPLOWERCLOSED, GFPUPPEROPEN, GFNOWARNING, "bg_blue");

    comment = "";
    comment += "The number of triangles that will constitute;";
    comment += "each candidate image;";
    comment += "Allowed value range [1,1000]";
    gpb.registerFileParameter<std::size_t>(
        "n_triangles",
        nTriangles_.reference(),
        GII_DEF_NTRIANGLES,
        Gem::Common::VAR_IS_ESSENTIAL,
        comment
    );
    // checkValueRange<std::size_t>(std::size_t(nTriangles_.value()), std::size_t(1), std::size_t(1000), GINTLOWERCLOSED, GINTUPPERCLOSED, GFNOWARNING, "n_triangles");

    comment = "";
    comment += "Whether triangles should be sorted according;";
    comment += "to their alpha channel;";
    gpb.registerFileParameter<bool>(
        "alpha_sort",
        alphaSort_.reference(),
        GII_DEF_ALPHASORT,
        Gem::Common::VAR_IS_ESSENTIAL,
        comment
    );

    comment = "";
    comment += "Whether the alpha channel shall be mutated;";
    gpb.registerFileParameter<bool>(
        "mutate_alpha_channel",
        mutateAlphaChannel_.reference(),
        GII_DEF_MUTATE_ALPHA_CHANNEL,
        Gem::Common::VAR_IS_ESSENTIAL,
        comment
    );

    comment = "";
    comment += "Whether the background color shall be mutated;";
    gpb.registerFileParameter<bool>(
        "change_bg_color",
        changeBGColor_.reference(),
        GII_DEF_CHBGCOLOR,
        Gem::Common::VAR_IS_ESSENTIAL,
        comment
    );

    // Allow our parent class to describe its options
    Gem::Common::GFactoryT<GImageIndividual>::describeLocalOptions_(gpb);
}

/******************************************************************************/
/**
	 * Allows to act on the configuration options received from the configuration file. Here
	 * we can add the options described in describeLocalOptions to the object.
	 *
	 * @param p A smart-pointer to be acted on during post-processing
	 */
void GImageIndividualFactory::postProcess_(std::shared_ptr<GImageIndividual> &p) {
    // The image must already have been loaded for this function to work properly
    p->init(
        nTriangles_,
        bgRed_,
        bgGreen_,
        bgBlue_,
        startSize_,
        minSize_,
        maxSize_,
        minOpaqueness_,
        maxOpaqueness_,
        alphaSort_,
        changeBGColor_,
        mutateAlphaChannel_,
        sigma_,
        sigmaSigma_,
        minSigma_,
        maxSigma_,
        adProb_,
        adaptAdProb_,
        minAdProb_,
        maxAdProb_,
        loc_sigma_,
        loc_sigmaSigma_,
        loc_minSigma_,
        loc_maxSigma_,
        loc_adProb_,
        loc_adaptAdProb_,
        loc_minAdProb_,
        loc_maxAdProb_
    );
}

/******************************************************************************/
/**
	 * Returns the value of the startSize_ variable
	 */
double GImageIndividualFactory::getStartSize() const {
    return startSize_;
}

/******************************************************************************/
/**
	 * Returns the value of the adProb_ variable
	 */
double GImageIndividualFactory::getAdProb() const {
    return adProb_;
}

/******************************************************************************/
/**
	 * Returns the value of the loc_adaptAdProb_ variable
	 */
double GImageIndividualFactory::getLocAdaptAdProb() const {
    return loc_adaptAdProb_;
}

/******************************************************************************/
/**
	 * Returns the value of the adaptAdProb variable
	 */
double GImageIndividualFactory::getAdaptAdProb() const {
    return adaptAdProb_;
}

/******************************************************************************/
/**
	 * Returns the value of the loc_adProb variable
	 */
double GImageIndividualFactory::getLocAdProb() const {
    return loc_adProb_;
}

/******************************************************************************/
/**
	 * Allows to specify an adaption factor for adProb_ (or 0, if you do not want this feature)
	 */
void GImageIndividualFactory::setAdaptAdProb(double adaptAdProb) {
#ifdef DEBUG
    if(adaptAdProb < 0. || adaptAdProb > 1.) {
        throw geneva_exception(
            g_error_streamer(DO_LOG, Gem::Common::timeAndPlace())
            << "In GImageIndividualFactory::setAdaptAdProb(): Error!" << '\n'
            << "Invalid value for adaptAdProb given: " << adaptAdProb << '\n'
            << "Expected range of [0:1]" << '\n'
        );
    }
#endif /* DEBUG */

    adaptAdProb_ = adaptAdProb;
}

/******************************************************************************/
/**
	 * Allows to specify an adaption factor for loc_adProb_ (or 0, if you do not want this feature)
	 */
void GImageIndividualFactory::setLocAdaptAdProb(double loc_adaptAdProb) {
#ifdef DEBUG
    if(loc_adaptAdProb < 0. || loc_adaptAdProb > 1.) {
        throw geneva_exception(
            g_error_streamer(DO_LOG, Gem::Common::timeAndPlace())
            << "In GImageIndividualFactory::setLocAdaptAdProb(): Error!" << '\n'
            << "Invalid value for loc_adaptAdProb given: " << loc_adaptAdProb << '\n'
            << "Expected range of [0:1]" << '\n'
        );
    }
#endif /* DEBUG */

    loc_adaptAdProb_ = loc_adaptAdProb;
}

/******************************************************************************/
/**
	 * Allows to retrieve the allowed range for adProb_ variation
	 */
auto GImageIndividualFactory::getAdProbRange() const {
    return std::tuple<double, double>{minAdProb_.value(), maxAdProb_.value()};
}

/******************************************************************************/
/**
	 * Allows to retrieve the allowed range for loc_adProb_ variation
	 */
auto GImageIndividualFactory::getLocAdProbRange() const {
    return std::tuple<double, double>{loc_minAdProb_.value(), loc_maxAdProb_.value()};
}

/******************************************************************************/
/**
	 * Allows to set the allowed range for adaption probability variation
	 */
void GImageIndividualFactory::setAdProbRange(double minAdProb, double maxAdProb) {
#ifdef DEBUG
    if(minAdProb < 0.) {
        throw geneva_exception(
            g_error_streamer(DO_LOG, Gem::Common::timeAndPlace())
            << "In GImageIndividualFactory::setAdProbRange(): Error!" << '\n'
            << "minAdProb < 0: " << minAdProb << '\n'
        );
    }

    if(minAdProb > maxAdProb) {
        throw geneva_exception(
            g_error_streamer(DO_LOG, Gem::Common::timeAndPlace())
            << "In GImageIndividualFactory::setAdProbRange(): Error!" << '\n'
            << "Invalid minAdProb and/or maxAdProb: " << minAdProb << " / " << maxAdProb
            << '\n'
        );
    }

    if(maxAdProb > 1.) {
        throw geneva_exception(
            g_error_streamer(DO_LOG, Gem::Common::timeAndPlace())
            << "In GImageIndividualFactory::setAdProbRange(): Error!" << '\n'
            << "maxAdProb > 1: " << maxAdProb << '\n'
        );
    }
#endif /* DEBUG */

    minAdProb_ = minAdProb;
    maxAdProb_ = maxAdProb;
}

/******************************************************************************/
/**
	 * Allows to set the allowed range for location adaption probability variation
	 */
void GImageIndividualFactory::setLocAdProbRange(double minLocAdProb, double maxLocAdProb) {
#ifdef DEBUG
    if(minLocAdProb < 0.) {
        throw geneva_exception(
            g_error_streamer(DO_LOG, Gem::Common::timeAndPlace())
            << "In GImageIndividualFactory::setLocAdProbRange(): Error!" << '\n'
            << "minLocAdProb < 0: " << minLocAdProb << '\n'
        );
    }

    if(minLocAdProb > maxLocAdProb) {
        throw geneva_exception(
            g_error_streamer(DO_LOG, Gem::Common::timeAndPlace())
            << "In GImageIndividualFactory::setLocAdProbRange(): Error!" << '\n'
            << "Invalid minLocAdProb and/or maxLocAdProb: " << minLocAdProb << " / " << maxLocAdProb
            << '\n'
        );
    }

    if(maxLocAdProb > 1.) {
        throw geneva_exception(
            g_error_streamer(DO_LOG, Gem::Common::timeAndPlace())
            << "In GImageIndividualFactory::setLocAdProbRange(): Error!" << '\n'
            << "maxLocAdProb > 1: " << maxLocAdProb << '\n'
        );
    }
#endif /* DEBUG */

    loc_minAdProb_ = minLocAdProb;
    loc_maxAdProb_ = maxLocAdProb;
}

/******************************************************************************/
/**
	 * Returns the value of the maxOpaqueness variable
	 */
double GImageIndividualFactory::getMaxOpaqueness() const {
    return maxOpaqueness_;
}

/******************************************************************************/
/**
	 * Returns the value of the maxSigma variable
	 */
double GImageIndividualFactory::getMaxSigma() const {
    return maxSigma_;
}

/******************************************************************************/
/**
	 * Returns the value of the loc_maxSigma variable
	 */
double GImageIndividualFactory::getLocMaxSigma() const {
    return loc_maxSigma_;
}

/******************************************************************************/
/**
	 * Returns the value of the maxSize_ variable
	 */
double GImageIndividualFactory::getMaxSize() const {
    return maxSize_;
}

/******************************************************************************/
/**
	 * Returns the value of the minOpaqueness variable
	 */
double GImageIndividualFactory::getMinOpaqueness() const {
    return minOpaqueness_;
}

/******************************************************************************/
/**
	 * Returns the value of the minSigma variable
	 */
double GImageIndividualFactory::getMinSigma() const {
    return minSigma_;
}

/******************************************************************************/
/**
	 * Returns the value of the loc_minSigma variable
	 */
double GImageIndividualFactory::getLocMinSigma() const {
    return loc_minSigma_;
}

/******************************************************************************/
/**
	 * Returns the value of the minSize_ variable
	 */
double GImageIndividualFactory::getMinSize() const {
    return minSize_;
}

/******************************************************************************/
/**
	 * Returns the value of the sigma variable
	 */
double GImageIndividualFactory::getSigma() const {
    return sigma_;
}

/******************************************************************************/
/**
	 * Returns the value of the loc_sigma variable
	 */
double GImageIndividualFactory::getLocSigma() const {
    return loc_sigma_;
}

/******************************************************************************/
/**
	 * Returns the value of the loc_sigmaSigma_ variable
	 */
double GImageIndividualFactory::getLocSigmaSigma() const {
    return loc_sigmaSigma_;
}

/******************************************************************************/
/**
	 * Returns the value of the sigmaSigma variable
	 */
double GImageIndividualFactory::getSigmaSigma() const {
    return sigmaSigma_;
}

/******************************************************************************/
/**
	 * Returns the value of the bgRed variable
	 */
double GImageIndividualFactory::getBGRed() const {
    return bgRed_;
}

/******************************************************************************/
/**
	 * Returns the value of the bgGreen variable
	 */
double GImageIndividualFactory::getBGGreen() const {
    return bgGreen_;
}

/*******************************************************************************/
/**
	 * Returns the value of the bgBlue variable
	 */
double GImageIndividualFactory::getBGBlue() const {
    return bgBlue_;
}

/******************************************************************************/
/**
	 * Returns the value of the nTriangles variable
	 */
std::size_t GImageIndividualFactory::getNTriangles() const {
    return nTriangles_;
}

/******************************************************************************/
/**
	 * Returns the value of the alphaSort variable
	 */
bool GImageIndividualFactory::getAlphaSort() const {
    return alphaSort_;
}

/******************************************************************************/
} // namespace Gem::Geneva

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/

#ifdef GEM_TESTING

template <>
std::shared_ptr<Gem::Geneva::GImageIndividual>
TFactory_GUnitTests<Gem::Geneva::GImageIndividual>() {
    // Create an image individual factory and create the first individual
    Gem::Geneva::GImageIndividualFactory f("../../config/GImageIndividual.json");
    return f();
}

#endif /* GEM_TESTING */
