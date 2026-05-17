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
#include <chrono>
#include <filesystem>
#include <any>
#include <fstream>
#include <string>
#include <type_traits>

// Boost header files go here

// Geneva headers go here
#include "common/GCommonEnums.hpp"
#include "common/GCommonHelperFunctions.hpp"
#include "common/GLogger.hpp"
#include "common/GPlotDesigner.hpp"
#include "courtier/GExecutorT.hpp"
#include "geneva/GParameterPropertyParser.hpp"
#include "geneva/GParameterSet.hpp"
#include "geneva/oa/GBase.hpp"

namespace Gem::Geneva {

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/
/**
 * This class implements the standard output common to all optimization algorithms.
 * It will usually already be registered as a pluggable optimization monitor, when
 * you instantiate a new optimization algorithm.
 */
class GStandardMonitor // NOLINT(cppcoreguidelines-special-member-functions)
  : public oa::GBasePluggableOM {
    ///////////////////////////////////////////////////////////////////////
    friend class boost::serialization::access;

    template <typename Archive>
    void serialize(Archive &ar, const unsigned int) {
        using boost::serialization::make_nvp;

        ar &make_nvp(
            "GBasePluggableOM",
            boost::serialization::base_object<oa::GBasePluggableOM>(*this)
        );
    }

    ///////////////////////////////////////////////////////////////////////

public:
    /***************************************************************************/

    /** @brief The default constructor */
    GStandardMonitor() = default;
    /** @brief The copy constructor */
    GStandardMonitor(const GStandardMonitor &cp) = default;
    /** @brief The destructor */
    ~GStandardMonitor() override = default;

protected:
    /***************************************************************************/
    /** @brief Loads the data of another object */
    void load_(const GObject *cp) override;

    /** @brief Allow access to this classes compare_ function */
    friend void Gem::Common::compare_base_t<GStandardMonitor>(
        GStandardMonitor const &,
        GStandardMonitor const &,
        Gem::Common::GToken &
    );

    /** @brief Searches for compliance with expectations with respect to another object of the same type */
    void compare_(
        const GObject &cp,
        const Gem::Common::expectation &e,
        const double &limit
    ) const override;

    /** @brief Applies modifications to this object. */
    bool modify_GUnitTests_() override;
    /** @brief Performs self tests that are expected to succeed. */
    void specificTestsNoFailureExpected_GUnitTests_() override;
    /** @brief Performs self tests that are expected to fail. */
    void specificTestsFailuresExpected_GUnitTests_() override;

private:
    /** @brief Emits a name for this class / object */
    std::string name_() const override;
    /** @brief Creates a deep clone of this object */
    GObject *clone_() const override;

    /** @brief Aggregates the work of all registered pluggable monitors */
    void
    informationFunction_(infoMode, oa::GBase const *const goa) override;
};

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/
/**
 * This class allows to output fitness information for a given optimization run.
 * It takes care of successive runs and marks them in the output. Information
 * will be output in the same histogram both for the best individual(s) found so far
 * and for the best individual(s) of each iteration.
 */
class GFitnessMonitor // NOLINT(cppcoreguidelines-special-member-functions)
  : public oa::GBasePluggableOM {
    ///////////////////////////////////////////////////////////////////////
    friend class boost::serialization::access;

    template <typename Archive>
    void serialize(Archive &ar, const unsigned int) {
        using boost::serialization::make_nvp;

        ar &make_nvp(
            "GBasePluggableOM",
            boost::serialization::base_object<oa::GBasePluggableOM>(*this)
        ) & BOOST_SERIALIZATION_NVP(xDim_) &
            BOOST_SERIALIZATION_NVP(yDim_) & BOOST_SERIALIZATION_NVP(nMonitorInds_) &
            BOOST_SERIALIZATION_NVP(resultFile_) & BOOST_SERIALIZATION_NVP(infoInitRun_) &
            BOOST_SERIALIZATION_NVP(globalFitnessGraphVec_) &
            BOOST_SERIALIZATION_NVP(iterationFitnessGraphVec_);
    }

    ///////////////////////////////////////////////////////////////////////

public:
    /************************************************************************/

    /** @brief The default constructor */
    GFitnessMonitor() = default;
    /** @brief The copy constructor */
    GFitnessMonitor(const GFitnessMonitor &cp);
    /** @brief The destructor */
    ~GFitnessMonitor() override = default;

    /** @brief Allows to specify a different name for the result file */
    void setResultFileName(const std::string &result_file);
    /** @brief Allows to retrieve the current value of the result file name */
    std::string getResultFileName() const;

    /** @brief Allows to set the dimensions of the canvas */
    void setDims(const std::uint32_t &x_dim, const std::uint32_t &y_dim);
    /** @brief Retrieve the dimensions as a tuple */
    std::tuple<std::uint32_t, std::uint32_t> getDims() const;
    /** @brief Retrieves the dimension of the canvas in x-direction */
    std::uint32_t getXDim() const;
    /** @brief Retrieves the dimension of the canvas in y-direction */
    std::uint32_t getYDim() const;

    /** @brief Sets the number of individuals in the population that should be monitored */
    void setNMonitorIndividuals(const std::size_t &n_monitor_inds);
    /** @brief Retrieves the number of individuals that are being monitored */
    std::size_t getNMonitorIndividuals() const;

protected:
    /************************************************************************/
    /** @brief Loads the data of another object */
    void load_(const GObject *cp) override;

    /** @brief Allow access to this classes compare_ function */
    friend void Gem::Common::compare_base_t<GFitnessMonitor>(
        GFitnessMonitor const &,
        GFitnessMonitor const &,
        Gem::Common::GToken &
    );

    /** @brief Searches for compliance with expectations with respect to another object of the same type */
    void compare_(
        const GObject &cp,
        const Gem::Common::expectation &e,
        const double &limit
    ) const override;

    /** @brief Applies modifications to this object. This is needed for testing purposes */
    bool modify_GUnitTests_() override;
    /** @brief Performs self tests that are expected to succeed. This is needed for testing purposes */
    void specificTestsNoFailureExpected_GUnitTests_() override;
    /** @brief Performs self tests that are expected to fail. This is needed for testing purposes */
    void specificTestsFailuresExpected_GUnitTests_() override;

private:
    /************************************************************************/
    /** @brief Emits a name for this class / object */
    std::string name_() const override;
    /** @brief Creates a deep clone of this object */
    GObject *clone_() const override;

    /** @brief Aggregates the work of all registered pluggable monitors */
    void
    informationFunction_(infoMode, oa::GBase const *const goa) override;

    /************************************************************************/

    std::uint32_t xDim_ = DEFAULTXDIMOM; ///< The dimension of the canvas in x-direction
    std::uint32_t yDim_ = DEFAULTYDIMOM; ///< The dimension of the canvas in y-direction
    std::size_t nMonitorInds_ =
        DEFNMONITORINDS; ///< The number of individuals that should be monitored
    std::string resultFile_ =
        DEFAULTROOTRESULTFILEOM; ///< The name of the file to which data is emitted

    bool infoInitRun_ =
        false; ///< Allows to check whether the INFOINIT section of informationFunction has already been passed at least once
    std::vector<std::shared_ptr<Gem::Common::GGraph2D>>
        globalFitnessGraphVec_; ///< Will hold progress information for the globally best individual
    std::vector<std::shared_ptr<Gem::Common::GGraph2D>>
        iterationFitnessGraphVec_; ///< Will hold progress information for an iteration best's individual
};

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/
/**
 * This class accepts a number of other pluggable monitors and executes them
 * in sequence.
 */
class GCollectiveMonitor // NOLINT(cppcoreguidelines-special-member-functions)
  : public oa::GBasePluggableOM {
    ///////////////////////////////////////////////////////////////////////
    friend class boost::serialization::access;

    template <typename Archive>
    void serialize(Archive &ar, const unsigned int) {
        using boost::serialization::make_nvp;

        ar &make_nvp(
            "GBasePluggableOM",
            boost::serialization::base_object<oa::GBasePluggableOM>(*this)
        );

        // Some preparation needed if this is a load operation.
        // This is needed to work around a problem in Boost 1.58
        if(Archive::is_loading::value) {
            pluggable_monitors_.clear();
        }

        ar &BOOST_SERIALIZATION_NVP(pluggable_monitors_);
    }
    ///////////////////////////////////////////////////////////////////////

public:
    /***************************************************************************/
    /** @brief The default constructor */
    GCollectiveMonitor() = default;
    /** @brief The copy constructor */
    GCollectiveMonitor(const GCollectiveMonitor &cp);
    /** @brief The destructor */
    ~GCollectiveMonitor() override = default;

    /** @brief Allows to register a new pluggable monitor */
    void registerPluggableOM(std::shared_ptr<oa::GBasePluggableOM> om_ptr);
    /** @brief Checks if adaptors have been registered in the collective monitor */
    bool hasOptimizationMonitors() const;
    /** @brief Allows to clear all registered monitors */
    void resetPluggbleOM();

protected:
    /***************************************************************************/
    /** @brief Loads the data of another object */
    void load_(const GObject *cp) override;

    /** @brief Allow access to this classes compare_ function */
    friend void Gem::Common::compare_base_t<GCollectiveMonitor>(
        GCollectiveMonitor const &,
        GCollectiveMonitor const &,
        Gem::Common::GToken &
    );

    /** @brief Searches for compliance with expectations with respect to another object of the same type */
    void compare_(
        const GObject &cp,
        const Gem::Common::expectation &e,
        const double &limit
    ) const override;

    /** @brief Applies modifications to this object */
    bool modify_GUnitTests_() override;
    /** @brief Performs self tests that are expected to succeed */
    void specificTestsNoFailureExpected_GUnitTests_() override;
    /** @brief Performs self tests that are expected to fail */
    void specificTestsFailuresExpected_GUnitTests_() override;

private:
    /***************************************************************************/
    /** @brief Emits a name for this class / object */
    std::string name_() const override;
    /** @brief Creates a deep clone of this object */
    GObject *clone_() const override;

    /** @brief Aggregates the work of all registered pluggable monitors */
    void
    informationFunction_(infoMode, oa::GBase const *const goa) override;

    std::vector<std::shared_ptr<oa::GBasePluggableOM>>
        pluggable_monitors_; ///< The collection of monitors
};

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/
/**
 * This class allows to monitor a given set of variables inside of all or of the
 * best individuals of a population, creating a graphical output using ROOT. It
 * supports floating point types only. double and float values may not be mixed.
 */
template <typename fp_type>
class GProgressPlotterT // NOLINT(cppcoreguidelines-special-member-functions)
  : public oa::GBasePluggableOM {
    ///////////////////////////////////////////////////////////////////////
    friend class boost::serialization::access;

    template <typename Archive>
    void serialize(Archive &ar, const unsigned int) {
        using boost::serialization::make_nvp;

        ar &make_nvp(
            "GBasePluggableOM",
            boost::serialization::base_object<oa::GBasePluggableOM>(*this)
        ) & BOOST_SERIALIZATION_NVP(fp_profVarVec_) &
            BOOST_SERIALIZATION_NVP(gpd_) & BOOST_SERIALIZATION_NVP(progressPlotter2D_oa_) &
            BOOST_SERIALIZATION_NVP(progressPlotter3D_oa_) &
            BOOST_SERIALIZATION_NVP(progressPlotter4D_oa_) & BOOST_SERIALIZATION_NVP(fileName_) &
            BOOST_SERIALIZATION_NVP(canvasDimensions_) &
            BOOST_SERIALIZATION_NVP(monitorBestOnly_) &
            BOOST_SERIALIZATION_NVP(monitorValidOnly_) &
            BOOST_SERIALIZATION_NVP(observeBoundaries_) &
            BOOST_SERIALIZATION_NVP(addPrintCommand_);
    }
    ///////////////////////////////////////////////////////////////////////

    // Make sure this class can only be instantiated if fp_type really is a floating point type
    static_assert(
        std::is_floating_point<fp_type>::value,
        "fp_type should be a floating point type"
    );

public:
    /***************************************************************************/
    /**
	  * The default constructor
	  */
    GProgressPlotterT() = default;

    /***************************************************************************/
    /**
	  * Construction with the information whether only the best individuals
	  * should be monitored and whether only valid items should be recorded.
	  * Some member variables may be initialized in the class body.
	  */
    GProgressPlotterT(bool monitor_best_only, bool monitor_valid_only)
      : gpd_("Progress information", 1, 1)
      , canvasDimensions_(std::tuple<std::uint32_t, std::uint32_t>(1024, 768))
      , monitorBestOnly_(monitor_best_only)
      , monitorValidOnly_(monitor_valid_only) { /* nothing */
    }

    /***************************************************************************/
    /**
	  * The copy constructor
	  */
    GProgressPlotterT(const GProgressPlotterT<fp_type> &cp)
      : oa::GBasePluggableOM(cp)
      , gpd_(cp.gpd_)
      , fileName_(cp.fileName_)
      , canvasDimensions_(cp.canvasDimensions_)
      , monitorBestOnly_(cp.monitorBestOnly_)
      , monitorValidOnly_(cp.monitorValidOnly_)
      , observeBoundaries_(cp.observeBoundaries_)
      , addPrintCommand_(cp.addPrintCommand_) {
        Gem::Common::copyCloneableSmartPointer(cp.progressPlotter2D_oa_, progressPlotter2D_oa_);
        Gem::Common::copyCloneableSmartPointer(cp.progressPlotter3D_oa_, progressPlotter3D_oa_);
        Gem::Common::copyCloneableSmartPointer(cp.progressPlotter4D_oa_, progressPlotter4D_oa_);
        Gem::Common::copyCloneableObjectsContainer(cp.fp_profVarVec_, fp_profVarVec_);
    }

    /***************************************************************************/
    /**
	  * The destuctor
	  */
    ~GProgressPlotterT() override = default;

    /**************************************************************************/
    /**
	  * Sets the specifications of the variables to be profiled. Note that
	  * boolean and integer variables specified in the argument will simply
	  * be ignored.
	  */
    void setProfileSpec(std::string const &par_str) {
        // Check that the parameter string isn't empty
        if(par_str.empty()) {
            throw geneva_exception(
                g_error_streamer(DO_LOG, time_and_place)
                << "In GPluggableOptimizationMonitors<>::setProfileSpec(std::string): Error!"
                << '\n'
                << "Parameter string " << par_str << " is empty" << '\n'
            );
        }

        //---------------------------------------------------------------------------
        // Clear the parameter vectors
        fp_profVarVec_.clear();

        // Parse the parameter string
        GParameterPropertyParser ppp(par_str);

        //---------------------------------------------------------------------------
        // Retrieve the parameters

        std::tuple<
            typename std::vector<parPropSpec<fp_type>>::const_iterator,
            typename std::vector<parPropSpec<fp_type>>::const_iterator>
            t_d = ppp.getIterators<fp_type>();

        typename std::vector<parPropSpec<fp_type>>::const_iterator fp_cit = std::get<0>(t_d);
        typename std::vector<parPropSpec<fp_type>>::const_iterator d_end = std::get<1>(t_d);
        for(; fp_cit != d_end;
            ++fp_cit) { // Note: fp_cit is already set to the begin of the double parameter arrays
            fp_profVarVec_.push_back(*fp_cit);
        }

        //---------------------------------------------------------------------------
    }

    /***************************************************************************/
    /**
	  * Allows to specify whether only the best individuals should be monitored.
	  */
    void setMonitorBestOnly(bool monitor_best_only = true) {
        monitorBestOnly_ = monitor_best_only;
    }

    /***************************************************************************/
    /**
	  * Allows to check whether only the best individuals should be monitored.
	  */
    bool getMonitorBestOnly() const {
        return monitorBestOnly_;
    }

    /***************************************************************************/
    /**
	  * Allows to specify whether only valid individuals should be monitored.
	  */
    void setMonitorValidOnly(bool monitor_valid_only = true) {
        monitorValidOnly_ = monitor_valid_only;
    }

    /***************************************************************************/
    /**
	  * Allows to check whether only valid individuals should be monitored.
	  */
    bool getMonitorValidOnly() const {
        return monitorValidOnly_;
    }

    /***************************************************************************/
    /**
	  * Allows to spefify whether scan boundaries should be observed
	  */
    void setObserveBoundaries(bool observe_boundaries) {
        observeBoundaries_ = observe_boundaries;
    }

    /***************************************************************************/
    /**
	  * Allows to check whether boundaries should be observed
	  */
    bool getObserveBoundaries() const {
        return observeBoundaries_;
    }

    /***************************************************************************/
    /**
	  * Allows to check whether parameters should be profiled
	  */
    bool parameterProfileCreationRequested() const {
        return not fp_profVarVec_.empty();
    }

    /***************************************************************************/
    /**
	  * Retrieves the number of variables that will be profiled
	  */
    std::size_t nProfileVars() const {
        return fp_profVarVec_.size();
    }

    /***************************************************************************/
    /**
	  * Allows to set the canvas dimensions
	  */
    void setCanvasDimensions(std::tuple<std::uint32_t, std::uint32_t> canvas_dimensions) {
        canvasDimensions_ = canvas_dimensions;
    }

    /***************************************************************************/
    /**
	  * Allows to set the canvas dimensions using separate x and y values
	  */
    void setCanvasDimensions(std::uint32_t x, std::uint32_t y) {
        canvasDimensions_ = std::tuple<std::uint32_t, std::uint32_t>(x, y);
    }

    /***************************************************************************/
    /**
	  * Gives access to the canvas dimensions
	  */
    std::tuple<std::uint32_t, std::uint32_t> getCanvasDimensions() const {
        return canvasDimensions_;
    }

    /******************************************************************************/
    /**
	  * Allows to add a "Print" command to the end of the script so that picture files are created
	  */
    void setAddPrintCommand(bool add_print_command) {
        addPrintCommand_ = add_print_command;
    }

    /******************************************************************************/
    /**
	  * Allows to retrieve the current value of the addPrintCommand_ variable
	  */
    bool getAddPrintCommand() const {
        return addPrintCommand_;
    }

    /***************************************************************************/
    /**
	  * Allows to set the filename
	  */
    void setFileName(const std::string &file_name) {
        fileName_ = file_name;
    }

    /***************************************************************************/
    /**
	  * Retrieves the current filename to which information will be emitted
	  */
    std::string getFileName() const {
        return fileName_;
    }

    /***************************************************************************/
    /**
	  * Allows to set the canvas label
	  */
    void setCanvasLabel(const std::string &canvas_label) {
        gpd_.setCanvasLabel(canvas_label);
    }

    /***************************************************************************/
    /**
	  * Allows to retrieve the canvas label
	  */
    std::string getCanvasLabel() const {
        return gpd_.getCanvasLabel();
    }

    /***************************************************************************/
    /**
	  * Determines a suitable label for a given parPropSpec value
	  */
    std::string getLabel(const parPropSpec<fp_type> &s) const {
        std::string result; // NOLINT(cppcoreguidelines-init-variables)

        std::size_t var_mode = std::get<0>(s.var);
        std::string var_name = std::get<1>(s.var);
        std::size_t var_pos = std::get<2>(s.var);

        switch(var_mode) {
        //--------------------------------------------------------------------
        case 0: // parameters are identified by id
        {
            result = std::string("variable id ") + Gem::Common::to_string(var_pos);
        } break;

            //--------------------------------------------------------------------
        case 1: {
            result = var_name + "[" + Gem::Common::to_string(var_pos) + "]";
        } break;

            //--------------------------------------------------------------------
        case 2: {
            result = var_name;
        } break;

            //--------------------------------------------------------------------
        default: {
            throw geneva_exception(
                g_error_streamer(DO_LOG, time_and_place)
                << "In GProgressPlotterT<fp_type>::getLabel(): Error" << '\n'
                << "Invalid mode " << var_mode << " requested" << '\n'
            );
        }

            //--------------------------------------------------------------------
        };

        return result;
    }

protected:
    /************************************************************************/
    /**
	  * Loads the data of another object
	  *
	  * cp A pointer to another GProgressPlotterTT<fp_type> object, camouflaged as a GObject
	  */
    void load_(const GObject *cp) override {
        // Check that we are dealing with a GProgressPlotterT<fp_type> reference independent of this object and convert the pointer
        const GProgressPlotterT<fp_type> *p_load = Gem::Common::g_convert_and_compare(cp, this);

        // Load the parent classes' data ...
        oa::GBasePluggableOM::load_(cp);

        // ... and then our local data
        Gem::Common::copyCloneableObjectsContainer(p_load->fp_profVarVec_, fp_profVarVec_);
        gpd_.load(p_load->gpd_);
        copyCloneableSmartPointer(p_load->progressPlotter2D_oa_, progressPlotter2D_oa_);
        copyCloneableSmartPointer(p_load->progressPlotter3D_oa_, progressPlotter3D_oa_);
        copyCloneableSmartPointer(p_load->progressPlotter4D_oa_, progressPlotter4D_oa_);
        fileName_ = p_load->fileName_;
        canvasDimensions_ = p_load->canvasDimensions_;
        monitorBestOnly_ = p_load->monitorBestOnly_;
        monitorValidOnly_ = p_load->monitorValidOnly_;
        observeBoundaries_ = p_load->observeBoundaries_;
        addPrintCommand_ = p_load->addPrintCommand_;
    }

    /***************************************************************************/
    /** @brief Allow access to this classes compare_ function */
    friend void Gem::Common::compare_base_t<GProgressPlotterT<fp_type>>(
        GProgressPlotterT<fp_type> const &,
        GProgressPlotterT<fp_type> const &,
        Gem::Common::GToken &
    );

    /***************************************************************************/
    /**
     * Searches for compliance with expectations with respect to another object
     * of the same type
     *
     * @param cp A constant reference to another GObject object
     * @param e The expected outcome of the comparison
     * @param limit The maximum deviation for floating point values (important for similarity checks)
     */
    void compare_(
        const GObject &cp,
        const Gem::Common::expectation &e,
        const double & /*limit*/
    ) const override {
        using namespace Gem::Common;

        // Check that we are dealing with a GProgressPlotterT<fp_type reference independent of this object and convert the pointer
        const GProgressPlotterT<fp_type> *p_load = Gem::Common::g_convert_and_compare(cp, this);

        GToken token("GProgressPlotterT<fp_type>", e);

        // Compare our parent data ...
        Gem::Common::compare_base_t<oa::GBasePluggableOM>(*this, *p_load, token);

        // ... and then our local data
        compare_t(IDENTITY(fp_profVarVec_, p_load->fp_profVarVec_), token);
        compare_t(IDENTITY(gpd_, p_load->gpd_), token);
        compare_t(IDENTITY(progressPlotter2D_oa_, p_load->progressPlotter2D_oa_), token);
        compare_t(IDENTITY(progressPlotter3D_oa_, p_load->progressPlotter3D_oa_), token);
        compare_t(IDENTITY(progressPlotter4D_oa_, p_load->progressPlotter4D_oa_), token);
        compare_t(IDENTITY(fileName_, p_load->fileName_), token);
        compare_t(IDENTITY(canvasDimensions_, p_load->canvasDimensions_), token);
        compare_t(IDENTITY(monitorBestOnly_, p_load->monitorBestOnly_), token);
        compare_t(IDENTITY(monitorValidOnly_, p_load->monitorValidOnly_), token);
        compare_t(IDENTITY(observeBoundaries_, p_load->observeBoundaries_), token);
        compare_t(IDENTITY(addPrintCommand_, p_load->addPrintCommand_), token);

        // React on deviations from the expectation
        token.evaluate();
    }

    /***************************************************************************/
    /**
     * Applies modifications to this object. This is needed for testing purposes
     *
     * @return A boolean which indicates whether modifications were made
     */
    bool modify_GUnitTests_() override {
#ifdef GEM_TESTING

        bool result = false;

        // Call the parent classes' functions
        if(oa::GBasePluggableOM::modify_GUnitTests_()) {
            result = true;
        }

        // no local data -- nothing to change

        return result;

#else /* GEM_TESTING */ // If this function is called when GEM_TESTING isn't set, throw
        Gem::Common::condnotset("GProgressPlotterT<fp_type>::modify_GUnitTests", "GEM_TESTING");
        return false;
#endif                  /* GEM_TESTING */
    }

    /***************************************************************************/
    /**
     * Performs self tests that are expected to succeed. This is needed for testing purposes
     */
    void specificTestsNoFailureExpected_GUnitTests_() override {
#ifdef GEM_TESTING

        // Call the parent classes' functions
        oa::GBasePluggableOM::specificTestsNoFailureExpected_GUnitTests_();
#else /* GEM_TESTING */ // If this function is called when GEM_TESTING isn't set, throw
        Gem::Common::condnotset(
            "GProgressPlotterT<fp_type>::specificTestsNoFailureExpected_GUnitTests",
            "GEM_TESTING"
        );
#endif                  /* GEM_TESTING */
    }

    /***************************************************************************/
    /**
     * Performs self tests that are expected to fail. This is needed for testing purposes
     */
    void specificTestsFailuresExpected_GUnitTests_() override {
#ifdef GEM_TESTING

        // Call the parent classes' functions
        oa::GBasePluggableOM::specificTestsFailuresExpected_GUnitTests_();

#else /* GEM_TESTING */ // If this function is called when GEM_TESTING isn't set, throw
        Gem::Common::condnotset(
            "GProgressPlotterT<fp_type>::specificTestsFailuresExpected_GUnitTests",
            "GEM_TESTING"
        );
#endif                  /* GEM_TESTING */
    }
    /***************************************************************************/

private:
    /***************************************************************************/
    /**
	  * Emits a name for this class / object
	  */
    std::string name_() const override {
        return std::string("GProgressPlotterT<fp_type>");
    }

    /************************************************************************/
    /**
	  * Creates a deep clone of this object
	  */
    GObject *clone_() const override {
        return new GProgressPlotterT<fp_type>(*this);
    }

    /***************************************************************************/
    /**
     * Allows to emit information in different stages of the information cycle
     * (initialization, during each cycle and during finalization)
     */
    void informationFunction_(infoMode im, oa::GBase const *const goa) override {
        switch(im) {
        case Gem::Geneva::infoMode::INFOINIT: {
            switch(this->nProfileVars()) {
            case 1: {
                progressPlotter2D_oa_ = std::make_shared<Gem::Common::GGraph2D>();

                progressPlotter2D_oa_->setPlotMode(Gem::Common::graphPlotMode::CURVE);
                progressPlotter2D_oa_->setPlotLabel("Fitness as a function of a parameter value");
                progressPlotter2D_oa_->setXAxisLabel(this->getLabel(fp_profVarVec_[0]));
                progressPlotter2D_oa_->setYAxisLabel("Fitness");

                gpd_.registerPlotter(progressPlotter2D_oa_);
            } break;
            case 2: {
                progressPlotter3D_oa_ = std::make_shared<Gem::Common::GGraph3D>();

                progressPlotter3D_oa_->setPlotLabel("Fitness as a function of parameter values");
                progressPlotter3D_oa_->setXAxisLabel(this->getLabel(fp_profVarVec_[0]));
                progressPlotter3D_oa_->setYAxisLabel(this->getLabel(fp_profVarVec_[1]));
                progressPlotter3D_oa_->setZAxisLabel("Fitness");

                gpd_.registerPlotter(progressPlotter3D_oa_);
            } break;

            case 3: {
                progressPlotter4D_oa_ = std::make_shared<Gem::Common::GGraph4D>();

                progressPlotter4D_oa_->setPlotLabel(
                    "Fitness (color-coded) as a function of parameter values"
                );
                progressPlotter4D_oa_->setXAxisLabel(this->getLabel(fp_profVarVec_[0]));
                progressPlotter4D_oa_->setYAxisLabel(this->getLabel(fp_profVarVec_[1]));
                progressPlotter4D_oa_->setZAxisLabel(this->getLabel(fp_profVarVec_[2]));

                gpd_.registerPlotter(progressPlotter4D_oa_);
            } break;

            default: {
                glogger << "NOTE: In "
                           "GProgressPlotterT<fp_type>::informationFunction_(infoMode::INFOINIT):"
                        << '\n'
                        << "Number of profiling dimensions " << this->nProfileVars()
                        << " can not be displayed." << '\n'
                        << "No graphical output will be created." << '\n'
                        << GLOGGING;
            } break;
            }

            gpd_.setCanvasDimensions(canvasDimensions_);
        } break;

        case Gem::Geneva::infoMode::INFOPROCESSING: {
            bool is_dirty = true;
            double primary_fitness = 0.;

            if(monitorBestOnly_) { // Monitor the best individuals only
                std::shared_ptr<GParameterSet> p =
                    goa->Interface::GOptimizerIT<oa::GBase>::template getBestGlobalIndividual<GParameterSet>();
                if(oa::GBasePluggableOM::useRawEvaluation_) {
                    primary_fitness = p->raw_fitness(0);
                }
                else {
                    primary_fitness = p->transformed_fitness(0);
                }

                if(not monitorValidOnly_ || p->isValid()) {
                    switch(this->nProfileVars()) {
                    case 1: {
                        fp_type val0 = p->GParameterSet::getVarVal<fp_type>(fp_profVarVec_[0].var);

                        if(observeBoundaries_) {
                            if(val0 >= fp_profVarVec_[0].lowerBoundary &&
                               val0 <= fp_profVarVec_[0].upperBoundary) {
                                progressPlotter2D_oa_->add(double(val0), primary_fitness);
                            }
                        }
                        else {
                            progressPlotter2D_oa_->add(double(val0), primary_fitness);
                        }
                    } break;

                    case 2: {
                        fp_type val0 = p->GParameterSet::getVarVal<fp_type>(fp_profVarVec_[0].var);
                        fp_type val1 = p->GParameterSet::getVarVal<fp_type>(fp_profVarVec_[1].var);

                        if(observeBoundaries_) {
                            if(val0 >= fp_profVarVec_[0].lowerBoundary &&
                               val0 <= fp_profVarVec_[0].upperBoundary &&
                               val1 >= fp_profVarVec_[1].lowerBoundary &&
                               val1 <= fp_profVarVec_[1].upperBoundary) {
                                progressPlotter3D_oa_->add(
                                    std::tuple<double, double, double>(
                                        double(val0),
                                        double(val1),
                                        primary_fitness
                                    )
                                );
                            }
                        }
                        else {
                            progressPlotter3D_oa_->add(
                                std::tuple<double, double, double>(
                                    double(val0),
                                    double(val1),
                                    primary_fitness
                                )
                            );
                        }
                    } break;

                    case 3: {
                        fp_type val0 = p->GParameterSet::getVarVal<fp_type>(fp_profVarVec_[0].var);
                        fp_type val1 = p->GParameterSet::getVarVal<fp_type>(fp_profVarVec_[1].var);
                        fp_type val2 = p->GParameterSet::getVarVal<fp_type>(fp_profVarVec_[2].var);

                        if(observeBoundaries_) {
                            if(val0 >= fp_profVarVec_[0].lowerBoundary &&
                               val0 <= fp_profVarVec_[0].upperBoundary &&
                               val1 >= fp_profVarVec_[1].lowerBoundary &&
                               val1 <= fp_profVarVec_[1].upperBoundary &&
                               val2 >= fp_profVarVec_[2].lowerBoundary &&
                               val2 <= fp_profVarVec_[2].upperBoundary) {
                                progressPlotter4D_oa_->add(
                                    std::tuple<double, double, double, double>(
                                        double(val0),
                                        double(val1),
                                        double(val2),
                                        primary_fitness
                                    )
                                );
                            }
                        }
                        else {
                            progressPlotter4D_oa_->add(
                                std::tuple<double, double, double, double>(
                                    double(val0),
                                    double(val1),
                                    double(val2),
                                    primary_fitness
                                )
                            );
                        }
                    } break;

                    default: // Do nothing by default. The number of profiling dimensions is too large
                        break;
                    }
                }
            }
            else { // Monitor all individuals
                for(const auto &ind_ptr : *goa) {
                    if(oa::GBasePluggableOM::useRawEvaluation_) {
                        primary_fitness = ind_ptr->raw_fitness(0);
                    }
                    else {
                        primary_fitness = ind_ptr->transformed_fitness(0);
                    }

                    if(not monitorValidOnly_ || ind_ptr->isValid()) {
                        switch(this->nProfileVars()) {
                        case 1: {
                            fp_type val0 = ind_ptr->GParameterSet::template getVarVal<fp_type>(
                                fp_profVarVec_[0].var
                            );

                            if(observeBoundaries_) {
                                if(val0 >= fp_profVarVec_[0].lowerBoundary &&
                                   val0 <= fp_profVarVec_[0].upperBoundary) {
                                    progressPlotter2D_oa_->add(double(val0), primary_fitness);
                                }
                            }
                            else {
                                progressPlotter2D_oa_->add(double(val0), primary_fitness);
                            }
                        } break;

                        case 2: {
                            fp_type val0 = ind_ptr->GParameterSet::template getVarVal<fp_type>(
                                fp_profVarVec_[0].var
                            );
                            fp_type val1 = ind_ptr->GParameterSet::template getVarVal<fp_type>(
                                fp_profVarVec_[1].var
                            );

                            if(observeBoundaries_) {
                                if(val0 >= fp_profVarVec_[0].lowerBoundary &&
                                   val0 <= fp_profVarVec_[0].upperBoundary &&
                                   val1 >= fp_profVarVec_[1].lowerBoundary &&
                                   val1 <= fp_profVarVec_[1].upperBoundary) {
                                    progressPlotter3D_oa_->add(
                                        std::tuple<double, double, double>(
                                            double(val0),
                                            double(val1),
                                            primary_fitness
                                        )
                                    );
                                }
                            }
                            else {
                                progressPlotter3D_oa_->add(
                                    std::tuple<double, double, double>(
                                        double(val0),
                                        double(val1),
                                        primary_fitness
                                    )
                                );
                            }
                        } break;

                        case 3: {
                            fp_type val0 = ind_ptr->GParameterSet::template getVarVal<fp_type>(
                                fp_profVarVec_[0].var
                            );
                            fp_type val1 = ind_ptr->GParameterSet::template getVarVal<fp_type>(
                                fp_profVarVec_[1].var
                            );
                            fp_type val2 = ind_ptr->GParameterSet::template getVarVal<fp_type>(
                                fp_profVarVec_[2].var
                            );

                            if(observeBoundaries_) {
                                if(val0 >= fp_profVarVec_[0].lowerBoundary &&
                                   val0 <= fp_profVarVec_[0].upperBoundary &&
                                   val1 >= fp_profVarVec_[1].lowerBoundary &&
                                   val1 <= fp_profVarVec_[1].upperBoundary &&
                                   val2 >= fp_profVarVec_[2].lowerBoundary &&
                                   val2 <= fp_profVarVec_[2].upperBoundary) {
                                    progressPlotter4D_oa_->add(
                                        std::tuple<double, double, double, double>(
                                            double(val0),
                                            double(val1),
                                            double(val2),
                                            primary_fitness
                                        )
                                    );
                                }
                            }
                            else {
                                progressPlotter4D_oa_->add(
                                    std::tuple<double, double, double, double>(
                                        double(val0),
                                        double(val1),
                                        double(val2),
                                        primary_fitness
                                    )
                                );
                            }
                        } break;

                        default: // Do nothing by default. The number of profiling dimensions is too large
                            break;
                        }
                    }
                }
            }
        } break;

        case Gem::Geneva::infoMode::INFOEND: {
            // Make sure 1-D data is sorted
            if(1 == this->nProfileVars()) {
                progressPlotter2D_oa_->sortX();
            }

            // Inform the plot designer whether it should print png files
            gpd_.setAddPrintCommand(addPrintCommand_);

            // Write out the result.
            gpd_.writeToFile(fileName_);

            // Remove all plotters
            gpd_.resetPlotters();
            progressPlotter2D_oa_.reset();
            progressPlotter3D_oa_.reset();
            progressPlotter4D_oa_.reset();
        } break;
        };
    }

    /************************************************************************/

    std::vector<parPropSpec<fp_type>>
        fp_profVarVec_; ///< Holds information about variables to be profiled

    Gem::Common::GPlotDesigner gpd_{"Progress information", 1, 1}; ///< A wrapper for the plots

    // These are temporaries
    std::shared_ptr<Gem::Common::GGraph2D> progressPlotter2D_oa_;
    std::shared_ptr<Gem::Common::GGraph3D> progressPlotter3D_oa_;
    std::shared_ptr<Gem::Common::GGraph4D> progressPlotter4D_oa_;

    std::string fileName_ = std::string(
        "progressScan.C"
    ); ///< The name of the file the output should be written to. Note that the class will add the name of the algorithm it acts on
    std::tuple<std::uint32_t, std::uint32_t> canvasDimensions_ =
        std::tuple<std::uint32_t, std::uint32_t>(1024, 768); ///< The dimensions of the canvas

    bool monitorBestOnly_ =
        false; ///< Indicates whether only the best individuals should be monitored
    bool monitorValidOnly_ = false; ///< Indicates whether only valid individuals should be plotted
    bool observeBoundaries_ =
        false; ///< When set to true, the plotter will ignore values outside of a scan boundary

    bool addPrintCommand_ =
        false; ///< Asks the GPlotDesigner to add a print command to result files
};

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/
/**
 * This class allows to log all candidate solutions found to a file, including the parameetr
 * values. NOTE that the file may become very large! Results are output in the following format:
 * param1 param2 ... param_m eval1 eval2 ... eval_n . By default, no commas and
 * explanations are printed. If with_name_and_type is set to true, the values are
 * prepended by a line with variable names and types. If with_commas is set to true,
 * commas will be printed in-between values. It is possible to filter the results by
 * asking the class to only log solutions better than a given set of values. What
 * is considered better depends on whether evaluation criteria are maximized or minimized
 * and is determined from the individual. Note that this class can only be instantiated
 * if individual_type is either a derivative of GParamterSet or is an object of the
 * GParameterSet class itself.
 */
class GAllSolutionFileLogger // NOLINT(cppcoreguidelines-special-member-functions)
  : public oa::GBasePluggableOM {
    ///////////////////////////////////////////////////////////////////////
    friend class boost::serialization::access;

    template <typename Archive>
    void serialize(Archive &ar, const unsigned int) {
        using boost::serialization::make_nvp;

        ar &make_nvp(
            "GBasePluggableOM",
            boost::serialization::base_object<oa::GBasePluggableOM>(*this)
        ) & BOOST_SERIALIZATION_NVP(fileName_) &
            BOOST_SERIALIZATION_NVP(boundaries_) & BOOST_SERIALIZATION_NVP(boundariesActive_) &
            BOOST_SERIALIZATION_NVP(withNameAndType_) & BOOST_SERIALIZATION_NVP(withCommas_) &
            BOOST_SERIALIZATION_NVP(useRawFitness_) & BOOST_SERIALIZATION_NVP(showValidity_) &
            BOOST_SERIALIZATION_NVP(printInitial_) &
            BOOST_SERIALIZATION_NVP(showIterationBoundaries_);
    }
    ///////////////////////////////////////////////////////////////////////

public:
    /***************************************************************************/

    /** @brief The default constructor */
    GAllSolutionFileLogger() = default;
    /** @brief Initialization with a file name */
    explicit GAllSolutionFileLogger(const std::string &file_name);
    /** @brief Initialization with a file name and boundaries */

    GAllSolutionFileLogger(const std::string &file_name, const std::vector<double> &boundaries);
    /** @brief The copy constructor */
    GAllSolutionFileLogger(const GAllSolutionFileLogger &cp) = default;
    /** @brief The destructor */
    ~GAllSolutionFileLogger() override = default;

    /** @brief Sets the file name */
    void setFileName(const std::string &file_name);
    /** @brief Retrieves the current file name */
    std::string getFileName() const;

    /** @brief Sets the boundaries */
    void setBoundaries(const std::vector<double> &boundaries);
    /** @brief Allows to retrieve the boundaries */
    std::vector<double> getBoundaries() const;
    /** @brief Allows to check whether boundaries are active */
    bool boundariesActive() const;
    /** @brief Allows to inactivate boundaries */
    void setBoundariesInactive();

    /** @brief  Allows to specify whether explanations should be printed for parameter- and fitness values. */
    void setPrintWithNameAndType(bool with_name_and_type = true);
    /** @brief Allows to check whether explanations should be printed for parameter-and fitness values */
    bool getPrintWithNameAndType() const;

    /** @brief Allows to specify whether commas should be printed in-between values */
    void setPrintWithCommas(bool with_commas = true);
    /** @brief Allows to check whether commas should be printed in-between values */
    bool getPrintWithCommas() const;

    /** @brief Allows to specify whether the true (instead of the transformed) fitness should be shown */
    void setUseTrueFitness(bool use_raw_fitness = true);
    /** @brief Allows to retrieve whether the true (instead of the transformed) fitness should be shown */
    bool getUseTrueFitness() const;

    /** @brief Allows to specify whether the validity of a solution should be shown */
    void setShowValidity(bool show_validity = true);
    /** @brief Allows to check whether the validity of a solution will be shown */
    bool getShowValidity() const;

    /** @brief Allows to specifiy whether the initial population should be printed. */
    void setPrintInitial(bool print_initial = true);
    /** @brief Allows to check whether the initial population should be printed. */
    bool getPrintInitial() const;

    /** @brief Allows to specifiy whether a comment line should be inserted between iterations */
    void setShowIterationBoundaries(bool show_iteration_boundaries = true);
    /** @brief Allows to check whether a comment line should be inserted between iterations */
    bool getShowIterationBoundaries() const;

protected:
    /************************************************************************/

    /** @brief Loads the data of another object */
    void load_(const GObject *cp) override;

    /** @brief Allow access to this classes compare_ function */
    friend void Gem::Common::compare_base_t<GAllSolutionFileLogger>(
        GAllSolutionFileLogger const &,
        GAllSolutionFileLogger const &,
        Gem::Common::GToken &
    );

    /** @brief Searches for compliance with expectations with respect to another object of the same type */
    void compare_(
        const GObject &cp,
        const Gem::Common::expectation &e,
        const double &limit
    ) const override;

    /** @brief Applies modifications to this object */
    bool modify_GUnitTests_() override;
    /** @brief Performs self tests that are expected to succeed */
    void specificTestsNoFailureExpected_GUnitTests_() override;
    /** @brief Performs self tests that are expected to fail */
    void specificTestsFailuresExpected_GUnitTests_() override;

private:
    /***************************************************************************/
    /** @brief Emits a name for this class / object */
    std::string name_() const override;
    /** @brief Creates a deep clone of this object */
    GObject *clone_() const override;

    /** @brief Allows to emit information in different stages of the information cycle */
    void
    informationFunction_(infoMode, oa::GBase const *const goa) override;

    /** @brief Does the actual printing */
    void printPopulation(
        const std::string &iteration_description,
        oa::GBase const *const goa
    );

    /***************************************************************************/
    // Data

    std::string fileName_ =
        "CompleteSolutionLog.txt";    ///< The name of the file to which solutions should be stored
    std::vector<double> boundaries_; ///< Value boundaries used to filter logged solutions
    bool boundariesActive_ = false;  ///< Set to true if boundaries have been set
    bool withNameAndType_ = false;   ///< When set to true, explanations for values are printed
    bool withCommas_ = false; ///< When set to true, commas will be printed in-between values
    bool useRawFitness_ =
        true;                   ///< Indicates whether true- or transformed fitness should be output
    bool showValidity_ = true; ///< Indicates whether the validity of a solution should be shown
    bool printInitial_ = false; ///< Indicates whether the initial population should be printed
    bool showIterationBoundaries_ =
        false; ///< Indicates whether a comment indicating the end of an iteration should be printed
};

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/
/**
 * This class prints out all evaluations of each iteration. The format is
 * eval0_0, eval0_1, ... ,eval0_n, ..., evalm_0, evalm_1, ... ,evalm_n
 */
class GIterationResultsFileLogger // NOLINT(cppcoreguidelines-special-member-functions)
  : public oa::GBasePluggableOM {
    ///////////////////////////////////////////////////////////////////////
    friend class boost::serialization::access;

    template <typename Archive>
    void serialize(Archive &ar, const unsigned int) {
        using boost::serialization::make_nvp;

        ar &make_nvp(
            "GBasePluggableOM",
            boost::serialization::base_object<oa::GBasePluggableOM>(*this)
        ) & BOOST_SERIALIZATION_NVP(fileName_) &
            BOOST_SERIALIZATION_NVP(withCommas_) & BOOST_SERIALIZATION_NVP(useRawFitness_);
    }
    ///////////////////////////////////////////////////////////////////////

public:
    /***************************************************************************/

    /** @brief The default constructor */
    GIterationResultsFileLogger() = default;
    /** @brief Initialization with a file name */
    explicit GIterationResultsFileLogger(const std::string &file_name);
    /** @brief The copy constructor */
    GIterationResultsFileLogger(const GIterationResultsFileLogger &cp) = default;
    /** @brief The destructor */
    ~GIterationResultsFileLogger() override = default;

    /** @brief Sets the file name */
    void setFileName(const std::string &file_name);
    /** @brief Retrieves the current file name */
    std::string getFileName() const;

    /** @brief Allows to specify whether commas should be printed in-between values */
    void setPrintWithCommas(bool with_commas);
    /** @brief Allows to check whether commas should be printed in-between values */
    bool getPrintWithCommas() const;

    /** @brief Allows to specify whether the true (instead of the transformed) fitness should be shown */
    void setUseTrueFitness(bool use_raw_fitness);
    /** @brief Allows to retrieve whether the true (instead of the transformed) fitness should be shown */
    bool getUseTrueFitness() const;

protected:
    /************************************************************************/
    /** @brief Loads the data of another object */
    void load_(const GObject *cp) override;

    /** @brief Allow access to this classes compare_ function */
    friend void Gem::Common::compare_base_t<GIterationResultsFileLogger>(
        GIterationResultsFileLogger const &,
        GIterationResultsFileLogger const &,
        Gem::Common::GToken &
    );

    /** @brief Searches for compliance with expectations with respect to another object of the same type */
    void compare_(
        const GObject &cp,
        const Gem::Common::expectation &e,
        const double &limit
    ) const override;

    /** @brief Applies modifications to this object */
    bool modify_GUnitTests_() override;
    /** @brief Performs self tests that are expected to succeed */
    void specificTestsNoFailureExpected_GUnitTests_() override;
    /** @brief Performs self tests that are expected to fail. This is needed for testing purposes */
    void specificTestsFailuresExpected_GUnitTests_() override;

private:
    /***************************************************************************/
    /** @brief Emits a name for this class / object */
    std::string name_() const override;
    /** @brief Creates a deep clone of this object */
    GObject *clone_() const override;

    /** @brief Allows to emit information in different stages of the information cycle */
    void
    informationFunction_(infoMode, oa::GBase const *const goa) override;

    std::string fileName_ =
        "IterationResultsLog.txt"; ///< The name of the file to which solutions should be stored
    bool withCommas_ = true;      ///< When set to true, commas will be printed in-between values
    bool useRawFitness_ =
        false; ///< Indicates whether true- or transformed fitness should be output
};

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/
/**
 * This class allows to log the number of adaptions made inside of adaptors
 * to a file. This is mostly needed for debugging and profiling purposes. The
 * number of adaptions made is a good measure for the adaption probability.
 */
class GNAdpationsLogger // NOLINT(cppcoreguidelines-special-member-functions)
  : public oa::GBasePluggableOM {
    ///////////////////////////////////////////////////////////////////////
    friend class boost::serialization::access;

    template <typename Archive>
    void serialize(Archive &ar, const unsigned int) {
        using boost::serialization::make_nvp;

        ar &make_nvp(
            "GBasePluggableOM",
            boost::serialization::base_object<oa::GBasePluggableOM>(*this)
        ) & BOOST_SERIALIZATION_NVP(fileName_) &
            BOOST_SERIALIZATION_NVP(canvasDimensions_) & BOOST_SERIALIZATION_NVP(gpd_) &
            BOOST_SERIALIZATION_NVP(nAdaptionsHist2D_oa_) &
            BOOST_SERIALIZATION_NVP(nAdaptionsGraph2D_oa_) &
            BOOST_SERIALIZATION_NVP(fitnessGraph2D_oa_) &
            BOOST_SERIALIZATION_NVP(monitorBestOnly_) &
            BOOST_SERIALIZATION_NVP(addPrintCommand_) & BOOST_SERIALIZATION_NVP(maxIteration_) &
            BOOST_SERIALIZATION_NVP(nIterationsRecorded_) &
            BOOST_SERIALIZATION_NVP(nAdaptionsStore_);
    }
    ///////////////////////////////////////////////////////////////////////

public:
    /***************************************************************************/

    /** @brief The default constructor */
    GNAdpationsLogger() = default;
    /** @brief Initialization with a file name */
    explicit GNAdpationsLogger(const std::string &file_name);
    /** @brief The copy constructor */
    GNAdpationsLogger(const GNAdpationsLogger &cp);
    /** @brief The destructor */
    ~GNAdpationsLogger() override = default;

    /** @brief Sets the file name */
    void setFileName(const std::string &file_name);
    /** @brief Retrieves the current file name */
    std::string getFileName() const;

    /** @brief Allows to specify whether only the best individuals should be monitored */
    void setMonitorBestOnly(bool monitor_best_only = true);
    /** @brief Allows to check whether only the best individuals should be monitored */
    bool getMonitorBestOnly() const;

    /** @brief Allows to set the canvas dimensions */
    void setCanvasDimensions(std::tuple<std::uint32_t, std::uint32_t> canvas_dimensions);
    /** @brief Allows to set the canvas dimensions using separate x and y values */
    void setCanvasDimensions(std::uint32_t x, std::uint32_t y);
    /** @brief Gives access to the canvas dimensions */
    std::tuple<std::uint32_t, std::uint32_t> getCanvasDimensions() const;

    /** @brief Allows to add a "Print" command to the end of the script so that picture files are created */
    void setAddPrintCommand(bool add_print_command);
    /** @brief Allows to retrieve the current value of the addPrintCommand_ variable */
    bool getAddPrintCommand() const;

protected:
    /************************************************************************/

    /** @brief Loads the data of another object */
    void load_(const GObject *cp) override;

    /** @brief Allow access to this classes compare_ function */
    friend void Gem::Common::compare_base_t<GNAdpationsLogger>(
        GNAdpationsLogger const &,
        GNAdpationsLogger const &,
        Gem::Common::GToken &
    );

    /** @brief Searches for compliance with expectations with respect to another object */
    void compare_(
        const GObject &cp,
        const Gem::Common::expectation &e,
        const double &limit
    ) const override;

    /** @brief Applies modifications to this object */
    bool modify_GUnitTests_() override;
    /** @brief Performs self tests that are expected to succeed */
    void specificTestsNoFailureExpected_GUnitTests_() override;
    /** @brief Performs self tests that are expected to fail. This is needed for testing purposes */
    void specificTestsFailuresExpected_GUnitTests_() override;

private:
    /***************************************************************************/
    /** @brief Creates a deep clone of this object */
    GObject *clone_() const override;

    /** @brief Allows to emit information in different stages of the information cycle */
    void
    informationFunction_(infoMode, oa::GBase const *const goa) override;

    std::string fileName_ =
        "NAdaptions.C"; ///< The name of the file to which solutions should be stored

    std::tuple<std::uint32_t, std::uint32_t> canvasDimensions_ =
        std::tuple<std::uint32_t, std::uint32_t>(1200, 1600); ///< The dimensions of the canvas

    Gem::Common::GPlotDesigner gpd_{
        "Number of adaptions per iteration",
        1,
        2
    }; ///< A wrapper for the plots

    std::shared_ptr<Gem::Common::GHistogram2D>
        nAdaptionsHist2D_oa_; ///< Holds the actual histogram
    std::shared_ptr<Gem::Common::GGraph2D>
        nAdaptionsGraph2D_oa_; ///< Used if we only monitor the best solution in each iteration
    std::shared_ptr<Gem::Common::GGraph2D>
        fitnessGraph2D_oa_; ///< Lets us monitor the current fitness of the population

    bool monitorBestOnly_ =
        false; ///< Indicates whether only the best individuals should be monitored
    bool addPrintCommand_ =
        false; ///< Asks the GPlotDesigner to add a print command to result files

    std::size_t maxIteration_ = 0; ///< Holds the largest iteration recorded for the algorithm
    std::size_t nIterationsRecorded_ =
        0; ///< Holds the number of iterations that were recorded (not necessarily == maxIteration_

    std::vector<std::tuple<double, double>>
        nAdaptionsStore_; ///< Holds all information about the number of adaptions
};

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/
/**
 * This class allows to log chosen properties of adaptors. Such properties
 * are limited to numeric entities, that may be converted to double
 */
template <typename num_type>
class GAdaptorPropertyLoggerT // NOLINT(cppcoreguidelines-special-member-functions)
  : public oa::GBasePluggableOM {
    ///////////////////////////////////////////////////////////////////////
    friend class boost::serialization::access;

    template <typename Archive>
    void serialize(Archive &ar, const unsigned int) {
        using boost::serialization::make_nvp;

        ar &make_nvp(
            "GBasePluggableOM",
            boost::serialization::base_object<oa::GBasePluggableOM>(*this)
        ) & BOOST_SERIALIZATION_NVP(fileName_) &
            BOOST_SERIALIZATION_NVP(adaptorName_) & BOOST_SERIALIZATION_NVP(property_) &
            BOOST_SERIALIZATION_NVP(canvasDimensions_) & BOOST_SERIALIZATION_NVP(gpd_) &
            BOOST_SERIALIZATION_NVP(adaptorPropertyHist2D_oa_) &
            BOOST_SERIALIZATION_NVP(fitnessGraph2D_oa_) &
            BOOST_SERIALIZATION_NVP(monitorBestOnly_) &
            BOOST_SERIALIZATION_NVP(addPrintCommand_) & BOOST_SERIALIZATION_NVP(maxIteration_) &
            BOOST_SERIALIZATION_NVP(nIterationsRecorded_) &
            BOOST_SERIALIZATION_NVP(adaptorPropertyStore_);
    }
    ///////////////////////////////////////////////////////////////////////

    static_assert(std::is_arithmetic<num_type>::value, "num_type should be an arithmetic type");

public:
    /***************************************************************************/
    /**
	  * The default constructor. Note that some parmeters may be initialized in
	  * the class body.
	  */
    GAdaptorPropertyLoggerT() = default;

    /***************************************************************************/
    /**
	  * Initialization with a file name
	  */
    GAdaptorPropertyLoggerT(std::string file_name, std::string adaptor_name, std::string property)
      : fileName_(std::move(file_name))
      , adaptorName_(std::move(adaptor_name))
      , property_(std::move(property))
      , canvasDimensions_(std::tuple<std::uint32_t, std::uint32_t>(1200, 1600))
      , gpd_("Adaptor properties", 1, 2) { /* nothing */
    }

    /***************************************************************************/
    /**
	  * The copy constructor
	  */
    GAdaptorPropertyLoggerT(const GAdaptorPropertyLoggerT<num_type> &cp)
      : fileName_(cp.fileName_)
      , adaptorName_(cp.adaptorName_)
      , property_(cp.property_)
      , canvasDimensions_(cp.canvasDimensions_)
      , gpd_(cp.gpd_)
      , monitorBestOnly_(cp.monitorBestOnly_)
      , addPrintCommand_(cp.addPrintCommand_)
      , maxIteration_(cp.maxIteration_)
      , nIterationsRecorded_(cp.nIterationsRecorded_)
      , adaptorPropertyStore_(cp.adaptorPropertyStore_) {
        // Copy the smart pointers over
        Gem::Common::copyCloneableSmartPointer(
            cp.adaptorPropertyHist2D_oa_,
            adaptorPropertyHist2D_oa_
        );
        Gem::Common::copyCloneableSmartPointer(cp.fitnessGraph2D_oa_, fitnessGraph2D_oa_);
    }

    /***************************************************************************/
    /**
	  * The destructor
	  */
    ~GAdaptorPropertyLoggerT() override = default;

    /***************************************************************************/
    /**
	  * Sets the file name
	  */
    void setFileName(const std::string &file_name) {
        fileName_ = file_name;
    }

    /***************************************************************************/
    /**
	  * Retrieves the current file name
	  */
    std::string getFileName() const {
        return fileName_;
    }

    /***************************************************************************/
    /**
	  * Sets the name of the adaptor
	  */
    void setAdaptorName(std::string adaptor_name) {
        adaptorName_ = adaptor_name;
    }

    /***************************************************************************/
    /**
	  * Retrieves the name of the adaptor
	  */
    std::string getAdaptorName() const {
        return adaptorName_;
    }

    /***************************************************************************/
    /**
	  * Sets the name of the property
	  */
    void setPropertyName(std::string property) {
        property_ = property;
    }

    /***************************************************************************/
    /**
	  * Retrieves the name of the property
	  */
    std::string getPropertyName() const {
        return property_;
    }

    /***************************************************************************/
    /**
	  * Allows to specify whether only the best individuals should be monitored.
	  */
    void setMonitorBestOnly(bool monitor_best_only = true) {
        monitorBestOnly_ = monitor_best_only;
    }

    /***************************************************************************/
    /**
	  * Allows to check whether only the best individuals should be monitored.
	  */
    bool getMonitorBestOnly() const {
        return monitorBestOnly_;
    }

    /***************************************************************************/
    /**
	  * Allows to set the canvas dimensions
	  */
    void setCanvasDimensions(std::tuple<std::uint32_t, std::uint32_t> canvas_dimensions) {
        canvasDimensions_ = canvas_dimensions;
    }

    /***************************************************************************/
    /**
	  * Allows to set the canvas dimensions using separate x and y values
	  */
    void setCanvasDimensions(std::uint32_t x, std::uint32_t y) {
        canvasDimensions_ = std::tuple<std::uint32_t, std::uint32_t>(x, y);
    }

    /***************************************************************************/
    /**
	  * Gives access to the canvas dimensions
	  */
    std::tuple<std::uint32_t, std::uint32_t> getCanvasDimensions() const {
        return canvasDimensions_;
    }

    /******************************************************************************/
    /**
	  * Allows to add a "Print" command to the end of the script so that picture files are created
	  */
    void setAddPrintCommand(bool add_print_command) {
        addPrintCommand_ = add_print_command;
    }

    /******************************************************************************/
    /**
	  * Allows to retrieve the current value of the addPrintCommand_ variable
	  */
    bool getAddPrintCommand() const {
        return addPrintCommand_;
    }

protected:
    /************************************************************************/
    /**
	  * Loads the data of another object
	  *
	  * cp A pointer to another GAdaptorPropertyLoggerTT<num_type object, camouflaged as a GObject
	  */
    void load_(const GObject *cp) override {
        // Check that we are dealing with a GAdaptorPropertyLoggerT<num_type> reference independent of this object and convert the pointer
        const GAdaptorPropertyLoggerT<num_type> *p_load =
            Gem::Common::g_convert_and_compare<GObject, GAdaptorPropertyLoggerT<num_type>>(
                cp,
                this
            );

        // Load the parent classes' data ...
        oa::GBasePluggableOM::load_(cp);

        // ... and then our local data
        fileName_ = p_load->fileName_;
        adaptorName_ = p_load->adaptorName_;
        property_ = p_load->property_;
        canvasDimensions_ = p_load->canvasDimensions_;
        gpd_ = p_load->gpd_;
        Gem::Common::copyCloneableSmartPointer(
            p_load->adaptorPropertyHist2D_oa_,
            adaptorPropertyHist2D_oa_
        );
        Gem::Common::copyCloneableSmartPointer(p_load->fitnessGraph2D_oa_, fitnessGraph2D_oa_);
        monitorBestOnly_ = p_load->monitorBestOnly_;
        addPrintCommand_ = p_load->addPrintCommand_;
        maxIteration_ = p_load->maxIteration_;
        nIterationsRecorded_ = p_load->nIterationsRecorded_;
        adaptorPropertyStore_ = p_load->adaptorPropertyStore_;
    }

    /** @brief Allow access to this classes compare_ function */
    friend void Gem::Common::compare_base_t<GAdaptorPropertyLoggerT<num_type>>(
        GAdaptorPropertyLoggerT<num_type> const &,
        GAdaptorPropertyLoggerT<num_type> const &,
        Gem::Common::GToken &
    );

    /***************************************************************************/
    /**
     * Searches for compliance with expectations with respect to another object
     * of the same type
     *
     * @param cp A constant reference to another GObject object
     * @param e The expected outcome of the comparison
     * @param limit The maximum deviation for floating point values (important for similarity checks)
     */
    void compare_(
        const GObject &cp,
        const Gem::Common::expectation &e,
        const double & /*limit*/
    ) const override {
        using namespace Gem::Common;

        // Check that we are dealing with a GAdaptorPropertyLoggerT<num_type> reference independent of this object and convert the pointer
        const GAdaptorPropertyLoggerT<num_type> *p_load =
            Gem::Common::g_convert_and_compare<GObject, GAdaptorPropertyLoggerT<num_type>>(
                cp,
                this
            );

        GToken token("GAdaptorPropertyLoggerT", e);

        // Compare our parent data ...
        Gem::Common::compare_base_t<oa::GBasePluggableOM>(*this, *p_load, token);

        // ... and then our local data
        compare_t(IDENTITY(fileName_, p_load->fileName_), token);
        compare_t(IDENTITY(adaptorName_, p_load->adaptorName_), token);
        compare_t(IDENTITY(property_, p_load->property_), token);
        compare_t(IDENTITY(canvasDimensions_, p_load->canvasDimensions_), token);
        compare_t(IDENTITY(gpd_, p_load->gpd_), token);
        compare_t(IDENTITY(adaptorPropertyHist2D_oa_, p_load->adaptorPropertyHist2D_oa_), token);
        compare_t(IDENTITY(fitnessGraph2D_oa_, p_load->fitnessGraph2D_oa_), token);
        compare_t(IDENTITY(monitorBestOnly_, p_load->monitorBestOnly_), token);
        compare_t(IDENTITY(addPrintCommand_, p_load->addPrintCommand_), token);
        compare_t(IDENTITY(maxIteration_, p_load->maxIteration_), token);
        compare_t(IDENTITY(nIterationsRecorded_, p_load->nIterationsRecorded_), token);
        compare_t(IDENTITY(adaptorPropertyStore_, p_load->adaptorPropertyStore_), token);

        // React on deviations from the expectation
        token.evaluate();
    }

    /***************************************************************************/
    /**
     * Applies modifications to this object. This is needed for testing purposes
     *
     * @return A boolean which indicates whether modifications were made
     */
    bool modify_GUnitTests_() override {
#ifdef GEM_TESTING

        bool result = false;

        // Call the parent classes' functions
        if(oa::GBasePluggableOM::modify_GUnitTests_()) {
            result = true;
        }

        // no local data -- nothing to change

        return result;

#else /* GEM_TESTING */ // If this function is called when GEM_TESTING isn't set, throw
        Gem::Common::condnotset(
            "GAdaptorPropertyLoggerT<num_type>::modify_GUnitTests",
            "GEM_TESTING"
        );
        return false;
#endif                  /* GEM_TESTING */
    }

    /***************************************************************************/
    /**
     * Performs self tests that are expected to succeed. This is needed for testing purposes
     */
    void specificTestsNoFailureExpected_GUnitTests_() override {
#ifdef GEM_TESTING

        // Call the parent classes' functions
        oa::GBasePluggableOM::specificTestsNoFailureExpected_GUnitTests_();
#else /* GEM_TESTING */ // If this function is called when GEM_TESTING isn't set, throw
        Gem::Common::condnotset(
            "GAdaptorPropertyLoggerT<num_type>::specificTestsNoFailureExpected_GUnitTests",
            "GEM_TESTING"
        );
#endif                  /* GEM_TESTING */
    }

    /***************************************************************************/
    /**
     * Performs self tests that are expected to fail. This is needed for testing purposes
     */
    void specificTestsFailuresExpected_GUnitTests_() override {
#ifdef GEM_TESTING

        // Call the parent classes' functions
        oa::GBasePluggableOM::specificTestsFailuresExpected_GUnitTests_();

#else /* GEM_TESTING */ // If this function is called when GEM_TESTING isn't set, throw
        Gem::Common::condnotset(
            "GAdaptorPropertyLoggerT<num_type>::specificTestsFailuresExpected_GUnitTests",
            "GEM_TESTING"
        );
#endif                  /* GEM_TESTING */
    }

private:
    /***************************************************************************/
    /**
	  * Emits a name for this class / object
	  */
    std::string name_() const override {
        return std::string("GAdaptorPropertyLoggerT");
    }

    /************************************************************************/
    /**
	  * Creates a deep clone of this object
	  */
    GObject *clone_() const override {
        return new GAdaptorPropertyLoggerT<num_type>(*this);
    }

    /***************************************************************************/
    /**
     * Allows to emit information in different stages of the information cycle
     * (initialization, during each cycle and during finalization)
     */
    void informationFunction_(infoMode im, oa::GBase const *const goa) override {
        using namespace Gem::Common;

        switch(im) {
        case Gem::Geneva::infoMode::INFOINIT: {
            // If the file pointed to by fileName_ already exists, make a back-up
            if(std::filesystem::exists(fileName_)) {
                std::string new_file_name =
                    fileName_ + ".bak_" +
                    Gem::Common::getMSSince1970(); // NOLINT(cppcoreguidelines-init-variables)

                glogger << "In GAdaptorPropertyLoggerT::informationFunction_(): Error!" << '\n'
                        << "Attempt to output information to file " << fileName_ << '\n'
                        << "which already exists. We will rename the old file to" << '\n'
                        << new_file_name << '\n'
                        << GWARNING;

                std::filesystem::rename(fileName_, new_file_name);
            }

            // Make sure the progress plotter has the desired size
            gpd_.setCanvasDimensions(canvasDimensions_);

            // Set up a graph to monitor the best fitness found
            fitnessGraph2D_oa_ = std::make_shared<Gem::Common::GGraph2D>();
            fitnessGraph2D_oa_->setXAxisLabel("Iteration");
            fitnessGraph2D_oa_->setYAxisLabel("Fitness");
            fitnessGraph2D_oa_->setPlotMode(Gem::Common::graphPlotMode::CURVE);
        } break;

        case Gem::Geneva::infoMode::INFOPROCESSING: {
            std::uint32_t iteration = goa->getIteration();

            // Record the current fitness
            std::shared_ptr<GParameterSet> p =
                goa->Interface::GOptimizerIT<oa::GBase>::template getBestGlobalIndividual<GParameterSet>();
            (*fitnessGraph2D_oa_) &
                std::tuple<double, double>(static_cast<double>(iteration), p->raw_fitness(0));

            // Update the largest known iteration and the number of recorded iterations
            maxIteration_ = iteration;
            nIterationsRecorded_++;

            // Will hold the adaptor properties
            std::vector<std::any> data;

            // Do the actual logging
            if(monitorBestOnly_) {
                std::shared_ptr<GParameterSet> best =
                    goa->Interface::GOptimizerIT<oa::GBase>::template getBestGlobalIndividual<GParameterSet>();

                // Retrieve the adaptor data (e.g. the sigma of a GDoubleGaussAdaptor
                best->queryAdaptor(adaptorName_, property_, data);

                // Attach the data to adaptorPropertyStore_
                std::vector<std::any>::iterator prop_it;
                for(prop_it = data.begin(); prop_it != data.end(); ++prop_it) {
                    adaptorPropertyStore_.emplace_back(
                        static_cast<double>(iteration),
                        double(std::any_cast<num_type>(*prop_it))

                    );
                }
            }
            else { // Monitor all individuals
                // Loop over all individuals of the algorithm.
                for(std::size_t pos = 0; pos < goa->size(); pos++) {
                    std::shared_ptr<GParameterSet> ind =
                        goa->template individual_cast<GParameterSet>(pos);

                    // Retrieve the adaptor data (e.g. the sigma of a GDoubleGaussAdaptor
                    ind->queryAdaptor(adaptorName_, property_, data);

                    // Attach the data to adaptorPropertyStore_
                    std::vector<std::any>::iterator prop_it;
                    for(prop_it = data.begin(); prop_it != data.end(); ++prop_it) {
                        adaptorPropertyStore_.emplace_back(
                            static_cast<double>(iteration),
                            double(std::any_cast<num_type>(*prop_it))

                        );
                    }
                }
            }
        } break;

        case Gem::Geneva::infoMode::INFOEND: {
            std::vector<std::tuple<double, double>>::iterator it;

            // Within adaptorPropertyStore_, find the largest number of adaptions performed
            double max_property = 0.;
            for(it = adaptorPropertyStore_.begin(); it != adaptorPropertyStore_.end(); ++it) {
                if(std::get<1>(*it) > max_property) {
                    max_property = std::get<1>(*it);
                }
            }

            // Create the histogram object
            adaptorPropertyHist2D_oa_ = std::make_shared<GHistogram2D>(
                nIterationsRecorded_,
                100,
                0.,
                double(maxIteration_),
                0.,
                max_property
            );

            adaptorPropertyHist2D_oa_->setXAxisLabel("Iteration");
            adaptorPropertyHist2D_oa_->setYAxisLabel(
                std::string("Adaptor-Name: ") + adaptorName_ + std::string(", Property: ") +
                property_
            );
            adaptorPropertyHist2D_oa_->setDrawingArguments("BOX");

            // Fill the object with data
            for(it = adaptorPropertyStore_.begin(); it != adaptorPropertyStore_.end(); ++it) {
                (*adaptorPropertyHist2D_oa_) & *it;
            }

            // Add the histogram to the plot designer
            gpd_.registerPlotter(adaptorPropertyHist2D_oa_);

            // Add the fitness monitor
            gpd_.registerPlotter(fitnessGraph2D_oa_);

            // Inform the plot designer whether it should print png files
            gpd_.setAddPrintCommand(addPrintCommand_);

            // Write out the result. Note that we add
            gpd_.writeToFile(fileName_);

            // Remove all plotters (they will survive inside of gpd)
            gpd_.resetPlotters();
            adaptorPropertyHist2D_oa_.reset();
        } break;

        default: {
            throw geneva_exception(
                g_error_streamer(DO_LOG, time_and_place)
                << "In GAdaptorPropertyLoggerT: Received invalid infoMode " << im << '\n'
            );
        }
        };
    }

    /************************************************************************/

    std::string fileName_ =
        "NAdaptions.C"; ///< The name of the file to which solutions should be stored

    std::string adaptorName_ =
        "GDoubleGaussAdaptor"; ///< The  name of the adaptor for which properties should be logged
    std::string property_ = "sigma"; ///< The name of the property to be logged

    std::tuple<std::uint32_t, std::uint32_t> canvasDimensions_ =
        std::tuple<std::uint32_t, std::uint32_t>(1200, 1600); ///< The dimensions of the canvas

    Gem::Common::GPlotDesigner gpd_{"Adaptor properties", 1, 2}; ///< A wrapper for the plots

    std::shared_ptr<Gem::Common::GHistogram2D>
        adaptorPropertyHist2D_oa_; ///< Holds the actual histogram
    std::shared_ptr<Gem::Common::GGraph2D>
        fitnessGraph2D_oa_; ///< Lets us monitor the current fitness of the population

    bool monitorBestOnly_ =
        false; ///< Indicates whether only the best individuals should be monitored
    bool addPrintCommand_ =
        false; ///< Asks the GPlotDesigner to add a print command to result files

    std::size_t maxIteration_ = 0; ///< Holds the largest iteration recorded for the algorithm
    std::size_t nIterationsRecorded_ =
        0; ///< Holds the number of iterations that were recorded (not necessarily == maxIteration_

    std::vector<std::tuple<double, double>>
        adaptorPropertyStore_; ///< Holds all information about the number of adaptions
};

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/
/**
 * This class allows to log the time needed for the processing step of each
 * individual. The output happens in the form of two root files, one holding histograms
 * for the processing times, the other showing the distribution of processing times for
 * each iteration in a 2D histogram
 */
class GProcessingTimesLogger // NOLINT(cppcoreguidelines-special-member-functions)
  : public oa::GBasePluggableOM {
    ///////////////////////////////////////////////////////////////////////

    friend class boost::serialization::access;

    template <typename Archive>
    void serialize(Archive &ar, const unsigned int) {
        using boost::serialization::make_nvp;

        ar &make_nvp(
            "GBasePluggableOM",
            boost::serialization::base_object<oa::GBasePluggableOM>(*this)
        ) & BOOST_SERIALIZATION_NVP(fileName_pth_) &
            BOOST_SERIALIZATION_NVP(canvasDimensions_pth_) & BOOST_SERIALIZATION_NVP(gpd_pth_) &
            BOOST_SERIALIZATION_NVP(fileName_pth2_) &
            BOOST_SERIALIZATION_NVP(canvasDimensions_pth2_) & BOOST_SERIALIZATION_NVP(gpd_pth2_) &
            BOOST_SERIALIZATION_NVP(fileName_txt_) &
            BOOST_SERIALIZATION_NVP(pre_processing_times_hist_) &
            BOOST_SERIALIZATION_NVP(processing_times_hist_) &
            BOOST_SERIALIZATION_NVP(post_processing_times_hist_) &
            BOOST_SERIALIZATION_NVP(all_processing_times_hist_) &
            BOOST_SERIALIZATION_NVP(pre_processing_times_hist2D_) &
            BOOST_SERIALIZATION_NVP(processing_times_hist2D_) &
            BOOST_SERIALIZATION_NVP(post_processing_times_hist2D_) &
            BOOST_SERIALIZATION_NVP(all_processing_times_hist2D_) &
            BOOST_SERIALIZATION_NVP(nBinsX_) & BOOST_SERIALIZATION_NVP(nBinsY_);
    }
    ///////////////////////////////////////////////////////////////////////

public:
    /***************************************************************************/

    /** @brief The default constructor */
    GProcessingTimesLogger();
    /** @brief Initialization with a file name */
    GProcessingTimesLogger(
        const std::string &file_name_pth,
        const std::string &file_name_pth2,
        const std::string &file_name_txt,
        std::size_t n_bins_x,
        std::size_t n_bins_y
    );
    /** @brief The copy constructor */
    GProcessingTimesLogger(const GProcessingTimesLogger &cp) = default;
    /** @brief  The destructor */
    ~GProcessingTimesLogger() override = default;

    /** @brief Sets the file name for the processing times histogram */
    void setFileName_pth(const std::string &file_name);
    /** @brief Retrieves the current file name for the processing times histogram */
    std::string getFileName_pth() const;
    /** @brief Sets the file name for the processing times histograms (2D) */
    void setFileName_pth2(const std::string &file_name);
    /** @brief Retrieves the current file name for the processing times histograms (2D) */
    std::string getFileName_pth2() const;

    /** @brief Sets the file name for the text output */
    void setFileName_txt(const std::string &file_name);
    /** @brief Retrieves the current file name for the text output */
    std::string getFileName_txt() const;

    /** @brief Allows to set the canvas dimensions for the processing times histograms */
    void setCanvasDimensions_pth(std::tuple<std::uint32_t, std::uint32_t> canvas_dimensions);
    /** @brief Allows to set the canvas dimensions using separate x and y values for the processing times histograms */
    void setCanvasDimensions_pth(std::uint32_t x, std::uint32_t y);

    /** @brief Gives access to the canvas dimensions of the processing times histograms */
    std::tuple<std::uint32_t, std::uint32_t> getCanvasDimensions_pth() const;
    /** @brief Allows to set the canvas dimensions for the processing times histograms (2D) */
    void setCanvasDimensions_pth2(std::tuple<std::uint32_t, std::uint32_t> canvas_dimensions);

    /** @brief Allows to set the canvas dimensions using separate x and y values for the processing times histograms (2D) */
    void setCanvasDimensions_pth2(std::uint32_t x, std::uint32_t y);
    /** @brief Gives access to the canvas dimensions of the processing times histograms (2D) */
    std::tuple<std::uint32_t, std::uint32_t> getCanvasDimensions_pth2() const;

    /** @brief Sets the number of bins for the processing times histograms in y-direction */
    void setNBinsX(std::size_t n_bins_x);
    /** @brief Retrieves the current number of bins for the processing times histograms in x-direction */
    std::size_t getNBinsX() const;

    /** @brief Sets the number of bins for the processing times histograms in y-direction */
    void setNBinsY(std::size_t n_bins_y);
    /** @brief Retrieves the current number of bins for the processing times histograms in y-direction */
    std::size_t getNBinsY() const;

protected:
    /************************************************************************/

    /** @brief Loads the data of another object */
    void load_(const GObject *cp) override;

    /** @brief Allow access to this classes compare_ function */
    friend void Gem::Common::compare_base_t<GProcessingTimesLogger>(
        GProcessingTimesLogger const &,
        GProcessingTimesLogger const &,
        Gem::Common::GToken &
    );

    /** @brief Searches for compliance with expectations with respect to another object of the same type */
    void compare_(
        const GObject &cp,
        const Gem::Common::expectation &e,
        const double &limit
    ) const override;

    /** @brief Applies modifications to this object */
    bool modify_GUnitTests_() override;
    /** @brief Performs self tests that are expected to succeed */
    void specificTestsNoFailureExpected_GUnitTests_() override;
    /** @brief Performs self tests that are expected to fail. This is needed for testing purposes */
    void specificTestsFailuresExpected_GUnitTests_() override;

private:
    /************************************************************************/
    /** @brief Emits a name for this class / object */
    std::string name_() const override;
    /** @brief Creates a deep clone of this object */
    GObject *clone_() const override;

    /** @brief Allows to emit information in different stages of the information cycle */
    void
    informationFunction_(infoMode, oa::GBase const *const goa) override;

    /************************************************************************/

    std::string fileName_pth_ =
        "processingTimingsHist.C"; ///< The name of the file to which timings should be written in ROOT format
    std::tuple<std::uint32_t, std::uint32_t> canvasDimensions_pth_ =
        std::tuple<std::uint32_t, std::uint32_t>(1600, 1200); ///< The dimensions of the canvas
    Gem::Common::GPlotDesigner gpd_pth_{
        "Timings for the processing steps of individuals",
        2,
        2
    }; ///< A wrapper for the plots

    std::string fileName_pth2_ =
        "processingTimingsVsIteration.C"; ///< The name of the file to which timings should be written in ROOT format
    std::tuple<std::uint32_t, std::uint32_t> canvasDimensions_pth2_ =
        std::tuple<std::uint32_t, std::uint32_t>(1600, 1200); ///< The dimensions of the canvas
    Gem::Common::GPlotDesigner gpd_pth2_{
        "Timings for the processing steps of individuals vs. iteration",
        2,
        2
    }; ///< A wrapper for the plots

    std::string fileName_txt_ =
        "processingTimings.txt"; ///< The name of the file to which timings should be written in text format

    std::shared_ptr<Gem::Common::GHistogram1D>
        pre_processing_times_hist_; ///< The amount of time needed for pre-processing
    std::shared_ptr<Gem::Common::GHistogram1D>
        processing_times_hist_; ///< The amount of time needed for processing
    std::shared_ptr<Gem::Common::GHistogram1D>
        post_processing_times_hist_; ///< The amount of time needed for post-processing
    std::shared_ptr<Gem::Common::GHistogram1D>
        all_processing_times_hist_; ///< The amount of time needed for the entire processing step

    std::shared_ptr<Gem::Common::GHistogram2D>
        pre_processing_times_hist2D_; ///< The amount of time needed for pre-processing
    std::shared_ptr<Gem::Common::GHistogram2D>
        processing_times_hist2D_; ///< The amount of time needed for processing
    std::shared_ptr<Gem::Common::GHistogram2D>
        post_processing_times_hist2D_; ///< The amount of time needed for post-processing
    std::shared_ptr<Gem::Common::GHistogram2D>
        all_processing_times_hist2D_; ///< The amount of time needed for the entire processing step

    std::size_t nBinsX_ =
        Gem::Common::DEFAULTNBINSGPD; ///< The number of bins in the histograms in x-direction
    std::size_t nBinsY_ =
        Gem::Common::DEFAULTNBINSGPD; ///< The number of bins in the histograms in y-direction
};

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/

using GProgressPlotter = GProgressPlotterT<double>;
template <typename num_type>
using GAdaptorPropertyLogger = GAdaptorPropertyLoggerT<num_type>;

/******************************************************************************/

} /* namespace Gem::Geneva */

/******************************************************************************/
// Exports of classes

BOOST_CLASS_EXPORT_KEY(Gem::Geneva::GStandardMonitor)                     // NOLINT
BOOST_CLASS_EXPORT_KEY(Gem::Geneva::GFitnessMonitor)                      // NOLINT
BOOST_CLASS_EXPORT_KEY(Gem::Geneva::GCollectiveMonitor)                   // NOLINT
BOOST_CLASS_EXPORT_KEY(Gem::Geneva::GProgressPlotter)                     // NOLINT
BOOST_CLASS_EXPORT_KEY(Gem::Geneva::GAllSolutionFileLogger)               // NOLINT
BOOST_CLASS_EXPORT_KEY(Gem::Geneva::GIterationResultsFileLogger)          // NOLINT
BOOST_CLASS_EXPORT_KEY(Gem::Geneva::GNAdpationsLogger)                    // NOLINT
BOOST_CLASS_EXPORT_KEY(Gem::Geneva::GAdaptorPropertyLogger<double>)       // NOLINT
BOOST_CLASS_EXPORT_KEY(Gem::Geneva::GAdaptorPropertyLogger<std::int32_t>) // NOLINT
BOOST_CLASS_EXPORT_KEY(Gem::Geneva::GAdaptorPropertyLogger<bool>)         // NOLINT
BOOST_CLASS_EXPORT_KEY(Gem::Geneva::GProcessingTimesLogger)               // NOLINT
/******************************************************************************/
