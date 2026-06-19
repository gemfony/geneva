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
#include "geneva/par/GParameterPropertyParser.hpp"
#include "geneva/ind/GOptimizableEntity.hpp"
#include "geneva/oa/GOptimizationAlgorithmBase.hpp"
#include "geneva/oa/GAdaption.hpp"
#include "geneva/oa/GAdaptionConfig.hpp"

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
    /**
     * @brief The copy constructor
     * @param cp A constant reference to another GStandardMonitor object whose data is deep-copied
     */
    GStandardMonitor(const GStandardMonitor &cp) = default;
    /** @brief The destructor */
    ~GStandardMonitor() override = default;

protected:
    /***************************************************************************/
    /**
     * @brief Loads the data of another object into this one
     * @param cp A pointer to another object of this type, camouflaged as a GBasePluggableOM, whose data is copied
     */
    void load_(const oa::GBasePluggableOM *cp) override;

    /** @brief Allow access to this classes compare_ function */
    friend void Gem::Common::compare_base_t<GStandardMonitor>(
        GStandardMonitor const &,
        GStandardMonitor const &,
        Gem::Common::GToken &
    );

    /**
     * @brief Searches for compliance with expectations with respect to another object of the same type
     * @param cp A constant reference to another GBasePluggableOM object to compare against
     * @param e The expected outcome of the comparison (e.g. equality or inequality)
     * @param limit The maximum allowed deviation for comparisons of floating point types
     */
    void compare_(
        const oa::GBasePluggableOM &cp,
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
    /**
     * @brief Emits a name for this class / object
     * @return The human-readable class name of this object
     */
    std::string name_() const override;
    /**
     * @brief Creates a deep clone of this object
     * @return A pointer to a freshly allocated deep copy of this object (ownership passes to the caller)
     */
    oa::GBasePluggableOM *clone_() const override;

    /**
     * @brief Aggregates the work of all registered pluggable monitors
     *
     * The first (unnamed) argument is the information mode (init, processing or finalization).
     * @param goa A constant pointer to the optimization algorithm whose state is being monitored
     */
    void
    informationFunction_(infoMode, oa::GOptimizationAlgorithmBase const *const goa) override;
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

    /***************************************************************************/
    /**
     * Single declaration of this class'es local data members, driving
     * serialize(), load_() and compare_() from one place. All members are
     * handled unconditionally: the two graph vectors are deep-cloned on load
     * (make_cloneable_container_member); the rest are plain (make_member).
     * No manual tail is needed.
     */
    template <typename Self>
    static auto localMembers_(Self &self) {
        return std::make_tuple(
            Gem::Common::make_member("x_dim_", self.x_dim_),
            Gem::Common::make_member("y_dim_", self.y_dim_),
            Gem::Common::make_member("n_monitor_inds_", self.n_monitor_inds_),
            Gem::Common::make_member("result_file_", self.result_file_),
            Gem::Common::make_member("info_init_run_", self.info_init_run_),
            Gem::Common::make_cloneable_container_member("global_fitness_graph_vec_", self.global_fitness_graph_vec_),
            Gem::Common::make_cloneable_container_member("iteration_fitness_graph_vec_", self.iteration_fitness_graph_vec_)
        );
    }

    template <typename Archive>
    void serialize(Archive &ar, const unsigned int) {
        using boost::serialization::make_nvp;

        ar &make_nvp(
            "GBasePluggableOM",
            boost::serialization::base_object<oa::GBasePluggableOM>(*this)
        );

        // All members are derived from the single localMembers() declaration.
        Gem::Common::serialize_members(ar, localMembers_(*this));
    }

    ///////////////////////////////////////////////////////////////////////

public:
    /************************************************************************/

    /** @brief The default constructor */
    GFitnessMonitor() = default;
    /**
     * @brief The copy constructor
     * @param cp A constant reference to another GFitnessMonitor object whose data is deep-copied
     */
    GFitnessMonitor(const GFitnessMonitor &cp);
    /** @brief The destructor */
    ~GFitnessMonitor() override = default;

    /**
     * @brief Allows to specify a different name for the result file
     * @param result_file The new name of the result file
     */
    void setResultFileName(const std::string &result_file);
    /**
     * @brief Allows to retrieve the current value of the result file name
     * @return The current name of the result file
     */
    std::string getResultFileName() const;

    /**
     * @brief Allows to set the dimensions of the canvas
     * @param x_dim The canvas dimension in x-direction
     * @param y_dim The canvas dimension in y-direction
     */
    void setDims(const std::uint32_t &x_dim, const std::uint32_t &y_dim);
    /**
     * @brief Retrieve the dimensions as a tuple
     * @return A tuple holding the canvas dimensions in x- and y-direction
     */
    std::tuple<std::uint32_t, std::uint32_t> getDims() const;
    /**
     * @brief Retrieves the dimension of the canvas in x-direction
     * @return The canvas dimension in x-direction
     */
    std::uint32_t getXDim() const;
    /**
     * @brief Retrieves the dimension of the canvas in y-direction
     * @return The canvas dimension in y-direction
     */
    std::uint32_t getYDim() const;

    /**
     * @brief Sets the number of individuals in the population that should be monitored
     * @param n_monitor_inds The number of individuals that should be monitored
     */
    void setNMonitorIndividuals(const std::size_t &n_monitor_inds);
    /**
     * @brief Retrieves the number of individuals that are being monitored
     * @return The number of individuals that are being monitored
     */
    std::size_t getNMonitorIndividuals() const;

protected:
    /************************************************************************/
    /**
     * @brief Loads the data of another object into this one
     * @param cp A pointer to another object of this type, camouflaged as a GBasePluggableOM, whose data is copied
     */
    void load_(const oa::GBasePluggableOM *cp) override;

    /** @brief Allow access to this classes compare_ function */
    friend void Gem::Common::compare_base_t<GFitnessMonitor>(
        GFitnessMonitor const &,
        GFitnessMonitor const &,
        Gem::Common::GToken &
    );

    /**
     * @brief Searches for compliance with expectations with respect to another object of the same type
     * @param cp A constant reference to another GBasePluggableOM object to compare against
     * @param e The expected outcome of the comparison (e.g. equality or inequality)
     * @param limit The maximum allowed deviation for comparisons of floating point types
     */
    void compare_(
        const oa::GBasePluggableOM &cp,
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
    /**
     * @brief Emits a name for this class / object
     * @return The human-readable class name of this object
     */
    std::string name_() const override;
    /**
     * @brief Creates a deep clone of this object
     * @return A pointer to a freshly allocated deep copy of this object (ownership passes to the caller)
     */
    oa::GBasePluggableOM *clone_() const override;

    /**
     * @brief Aggregates the work of all registered pluggable monitors
     *
     * The first (unnamed) argument is the information mode (init, processing or finalization).
     * @param goa A constant pointer to the optimization algorithm whose state is being monitored
     */
    void
    informationFunction_(infoMode, oa::GOptimizationAlgorithmBase const *const goa) override;

    /************************************************************************/

    std::uint32_t x_dim_ = DEFAULTXDIMOM; ///< The dimension of the canvas in x-direction
    std::uint32_t y_dim_ = DEFAULTYDIMOM; ///< The dimension of the canvas in y-direction
    std::size_t n_monitor_inds_ =
        DEFNMONITORINDS; ///< The number of individuals that should be monitored
    std::string result_file_ =
        DEFAULTROOTRESULTFILEOM; ///< The name of the file to which data is emitted

    bool info_init_run_ =
        false; ///< Allows to check whether the INFOINIT section of informationFunction has already been passed at least once
    std::vector<std::shared_ptr<Gem::Common::GGraph2D>>
        global_fitness_graph_vec_; ///< Will hold progress information for the globally best individual
    std::vector<std::shared_ptr<Gem::Common::GGraph2D>>
        iteration_fitness_graph_vec_; ///< Will hold progress information for an iteration best's individual
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

    /***************************************************************************/
    /**
     * Single declaration of this class'es only local data member,
     * pluggable_monitors_ (a container of cloneable monitor pointers), driving
     * load_() and compare_() from one place via make_cloneable_container_member.
     *
     * serialize() is intentionally NOT derived from this: it needs a load-time
     * pluggable_monitors_.clear() workaround (a Boost issue) before reading the
     * container, which serialize_members() cannot express. So serialize() keeps
     * its bespoke body below.
     */
    template <typename Self>
    static auto localMembers_(Self &self) {
        return std::make_tuple(
            Gem::Common::make_cloneable_container_member("pluggable_monitors_", self.pluggable_monitors_)
        );
    }

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
    /**
     * @brief The copy constructor
     * @param cp A constant reference to another GCollectiveMonitor object whose data is deep-copied
     */
    GCollectiveMonitor(const GCollectiveMonitor &cp);
    /** @brief The destructor */
    ~GCollectiveMonitor() override = default;

    /**
     * @brief Allows to register a new pluggable monitor
     * @param om_ptr A shared pointer to the pluggable optimization monitor to add to this collection
     */
    void registerPluggableOM(std::shared_ptr<oa::GBasePluggableOM> om_ptr);
    /**
     * @brief Checks if pluggable monitors have been registered in the collective monitor
     * @return true if at least one pluggable monitor has been registered, false otherwise
     */
    bool hasOptimizationMonitors() const;
    /** @brief Allows to clear all registered monitors */
    void resetPluggbleOM();

protected:
    /***************************************************************************/
    /**
     * @brief Loads the data of another object into this one
     * @param cp A pointer to another object of this type, camouflaged as a GBasePluggableOM, whose data is copied
     */
    void load_(const oa::GBasePluggableOM *cp) override;

    /** @brief Allow access to this classes compare_ function */
    friend void Gem::Common::compare_base_t<GCollectiveMonitor>(
        GCollectiveMonitor const &,
        GCollectiveMonitor const &,
        Gem::Common::GToken &
    );

    /**
     * @brief Searches for compliance with expectations with respect to another object of the same type
     * @param cp A constant reference to another GBasePluggableOM object to compare against
     * @param e The expected outcome of the comparison (e.g. equality or inequality)
     * @param limit The maximum allowed deviation for comparisons of floating point types
     */
    void compare_(
        const oa::GBasePluggableOM &cp,
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
    /**
     * @brief Emits a name for this class / object
     * @return The human-readable class name of this object
     */
    std::string name_() const override;
    /**
     * @brief Creates a deep clone of this object
     * @return A pointer to a freshly allocated deep copy of this object (ownership passes to the caller)
     */
    oa::GBasePluggableOM *clone_() const override;

    /**
     * @brief Aggregates the work of all registered pluggable monitors
     *
     * The first (unnamed) argument is the information mode (init, processing or finalization).
     * @param goa A constant pointer to the optimization algorithm whose state is being monitored
     */
    void
    informationFunction_(infoMode, oa::GOptimizationAlgorithmBase const *const goa) override;

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
 *
 * @tparam fp_type The floating point type of the monitored variables (e.g. double or float)
 */
template <typename fp_type>
class GProgressPlotterT // NOLINT(cppcoreguidelines-special-member-functions)
  : public oa::GBasePluggableOM {
    ///////////////////////////////////////////////////////////////////////
    friend class boost::serialization::access;

    /***************************************************************************/
    /**
     * Single declaration of this class'es unconditionally-handled local data
     * members, driving serialize(), load_() and compare_() from one place.
     * The three progress_plotter*_d_oa_ smart pointers are deep-cloned on load
     * (make_cloneable_member); the rest are plain config members (make_member).
     *
     * Handled manually (NOT in this tuple): fp_prof_var_vec_, a
     * std::vector<parPropSpec<fp_type>> of VALUE objects deep-copied via
     * copyCloneableObjectsContainer() (no tagged tie for value containers), and
     * gpd_, a GPlotDesigner VALUE member loaded via gpd_.load() rather than a
     * plain assignment. Both stay in the documented manual tail.
     */
    template <typename Self>
    static auto localMembers_(Self &self) {
        return std::make_tuple(
            Gem::Common::make_cloneable_member("progress_plotter2_d_oa_", self.progress_plotter2_d_oa_),
            Gem::Common::make_cloneable_member("progress_plotter3_d_oa_", self.progress_plotter3_d_oa_),
            Gem::Common::make_cloneable_member("progress_plotter4_d_oa_", self.progress_plotter4_d_oa_),
            Gem::Common::make_member("file_name_", self.file_name_),
            Gem::Common::make_member("canvas_dimensions_", self.canvas_dimensions_),
            Gem::Common::make_member("monitor_best_only_", self.monitor_best_only_),
            Gem::Common::make_member("monitor_valid_only_", self.monitor_valid_only_),
            Gem::Common::make_member("observe_boundaries_", self.observe_boundaries_),
            Gem::Common::make_member("add_print_command_", self.add_print_command_)
        );
    }

    template <typename Archive>
    void serialize(Archive &ar, const unsigned int) {
        using boost::serialization::make_nvp;

        ar &make_nvp(
            "GBasePluggableOM",
            boost::serialization::base_object<oa::GBasePluggableOM>(*this)
        );

        // Manual tail (kept first to preserve the wire order): the value-object
        // container and the GPlotDesigner value member.
        ar & BOOST_SERIALIZATION_NVP(fp_prof_var_vec_) & BOOST_SERIALIZATION_NVP(gpd_);

        // The unconditionally-handled members, derived from localMembers().
        Gem::Common::serialize_members(ar, localMembers_(*this));
    }
    ///////////////////////////////////////////////////////////////////////

    // Make sure this class can only be instantiated if fp_type really is a floating point type
    static_assert(
        std::is_floating_point_v<fp_type>,
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
	  * @brief Construction with the information whether only the best individuals
	  * should be monitored and whether only valid items should be recorded.
	  * Some member variables may be initialized in the class body.
	  *
	  * @param monitor_best_only If true, only the best individual(s) of the population are monitored
	  * @param monitor_valid_only If true, only individuals with a valid parameter set are recorded
	  */
    GProgressPlotterT(bool monitor_best_only, bool monitor_valid_only)
      : gpd_("Progress information", 1, 1)
      , canvas_dimensions_(std::tuple<std::uint32_t, std::uint32_t>(1024, 768))
      , monitor_best_only_(monitor_best_only)
      , monitor_valid_only_(monitor_valid_only) { /* nothing */
    }

    /***************************************************************************/
    /**
	  * @brief The copy constructor
	  *
	  * @param cp A constant reference to another GProgressPlotterT object whose data is deep-copied
	  */
    GProgressPlotterT(const GProgressPlotterT<fp_type> &cp)
      : oa::GBasePluggableOM(cp)
      , gpd_(cp.gpd_)
      , file_name_(cp.file_name_)
      , canvas_dimensions_(cp.canvas_dimensions_)
      , monitor_best_only_(cp.monitor_best_only_)
      , monitor_valid_only_(cp.monitor_valid_only_)
      , observe_boundaries_(cp.observe_boundaries_)
      , add_print_command_(cp.add_print_command_) {
        Gem::Common::copyCloneableSmartPointer(cp.progress_plotter2_d_oa_, progress_plotter2_d_oa_);
        Gem::Common::copyCloneableSmartPointer(cp.progress_plotter3_d_oa_, progress_plotter3_d_oa_);
        Gem::Common::copyCloneableSmartPointer(cp.progress_plotter4_d_oa_, progress_plotter4_d_oa_);
        Gem::Common::copyCloneableObjectsContainer(cp.fp_prof_var_vec_, fp_prof_var_vec_);
    }

    /***************************************************************************/
    /**
	  * The destuctor
	  */
    ~GProgressPlotterT() override = default;

    /**************************************************************************/
    /**
	  * @brief Sets the specifications of the variables to be profiled. Note that
	  * boolean and integer variables specified in the argument will simply
	  * be ignored.
	  *
	  * @param par_str A non-empty parameter-property specification string describing which floating-point variables to profile
	  */
    void setProfileSpec(std::string const &par_str) {
        // Check that the parameter string isn't empty
        if(par_str.empty()) {
            throw geneva_exception(
                g_error_streamer(DO_LOG, Gem::Common::timeAndPlace())
                << "In GPluggableOptimizationMonitors<>::setProfileSpec(std::string): Error!"
                << '\n'
                << "Parameter string " << par_str << " is empty" << '\n'
            );
        }

        //---------------------------------------------------------------------------
        // Clear the parameter vectors
        fp_prof_var_vec_.clear();

        // Parse the parameter string
        gen::GParameterPropertyParser ppp(par_str);

        //---------------------------------------------------------------------------
        // Retrieve the parameters

        std::tuple<
            typename std::vector<gen::parPropSpec<fp_type>>::const_iterator,
            typename std::vector<gen::parPropSpec<fp_type>>::const_iterator>
            t_d = ppp.getIterators<fp_type>();

        typename std::vector<gen::parPropSpec<fp_type>>::const_iterator fp_cit = std::get<0>(t_d);
        typename std::vector<gen::parPropSpec<fp_type>>::const_iterator d_end = std::get<1>(t_d);
        for(; fp_cit != d_end;
            ++fp_cit) { // Note: fp_cit is already set to the begin of the double parameter arrays
            fp_prof_var_vec_.push_back(*fp_cit);
        }

        //---------------------------------------------------------------------------
    }

    /***************************************************************************/
    /**
	  * @brief Allows to specify whether only the best individuals should be monitored.
	  *
	  * @param monitor_best_only If true, only the best individual(s) of the population are monitored
	  */
    void setMonitorBestOnly(bool monitor_best_only = true) {
        monitor_best_only_ = monitor_best_only;
    }

    /***************************************************************************/
    /**
	  * @brief Allows to check whether only the best individuals should be monitored.
	  *
	  * @return true if only the best individual(s) are monitored, false otherwise
	  */
    bool getMonitorBestOnly() const {
        return monitor_best_only_;
    }

    /***************************************************************************/
    /**
	  * @brief Allows to specify whether only valid individuals should be monitored.
	  *
	  * @param monitor_valid_only If true, only individuals with a valid parameter set are recorded
	  */
    void setMonitorValidOnly(bool monitor_valid_only = true) {
        monitor_valid_only_ = monitor_valid_only;
    }

    /***************************************************************************/
    /**
	  * @brief Allows to check whether only valid individuals should be monitored.
	  *
	  * @return true if only valid individuals are monitored, false otherwise
	  */
    bool getMonitorValidOnly() const {
        return monitor_valid_only_;
    }

    /***************************************************************************/
    /**
	  * @brief Allows to specify whether scan boundaries should be observed
	  *
	  * @param observe_boundaries If true, values outside a scan boundary are ignored when plotting
	  */
    void setObserveBoundaries(bool observe_boundaries) {
        observe_boundaries_ = observe_boundaries;
    }

    /***************************************************************************/
    /**
	  * @brief Allows to check whether boundaries should be observed
	  *
	  * @return true if scan boundaries are observed, false otherwise
	  */
    bool getObserveBoundaries() const {
        return observe_boundaries_;
    }

    /***************************************************************************/
    /**
	  * @brief Allows to check whether parameters should be profiled
	  *
	  * @return true if at least one variable has been registered for profiling, false otherwise
	  */
    bool parameterProfileCreationRequested() const {
        return not fp_prof_var_vec_.empty();
    }

    /***************************************************************************/
    /**
	  * @brief Retrieves the number of variables that will be profiled
	  *
	  * @return The number of registered profiling variables
	  */
    std::size_t nProfileVars() const {
        return fp_prof_var_vec_.size();
    }

    /***************************************************************************/
    /**
	  * @brief Allows to set the canvas dimensions
	  *
	  * @param canvas_dimensions A tuple holding the canvas dimensions in x- and y-direction
	  */
    void setCanvasDimensions(std::tuple<std::uint32_t, std::uint32_t> canvas_dimensions) {
        canvas_dimensions_ = canvas_dimensions;
    }

    /***************************************************************************/
    /**
	  * @brief Allows to set the canvas dimensions using separate x and y values
	  *
	  * @param x The canvas dimension in x-direction
	  * @param y The canvas dimension in y-direction
	  */
    void setCanvasDimensions(std::uint32_t x, std::uint32_t y) {
        canvas_dimensions_ = std::tuple<std::uint32_t, std::uint32_t>(x, y);
    }

    /***************************************************************************/
    /**
	  * @brief Gives access to the canvas dimensions
	  *
	  * @return A tuple holding the canvas dimensions in x- and y-direction
	  */
    std::tuple<std::uint32_t, std::uint32_t> getCanvasDimensions() const {
        return canvas_dimensions_;
    }

    /******************************************************************************/
    /**
	  * @brief Allows to add a "Print" command to the end of the script so that picture files are created
	  *
	  * @param add_print_command If true, a print command is appended so that picture files are created
	  */
    void setAddPrintCommand(bool add_print_command) {
        add_print_command_ = add_print_command;
    }

    /******************************************************************************/
    /**
	  * @brief Allows to retrieve the current value of the add_print_command_ variable
	  *
	  * @return true if a print command will be appended to the script, false otherwise
	  */
    bool getAddPrintCommand() const {
        return add_print_command_;
    }

    /***************************************************************************/
    /**
	  * @brief Allows to set the filename
	  *
	  * @param file_name The name of the file to which information will be emitted
	  */
    void setFileName(const std::string &file_name) {
        file_name_ = file_name;
    }

    /***************************************************************************/
    /**
	  * @brief Retrieves the current filename to which information will be emitted
	  *
	  * @return The name of the file to which information will be emitted
	  */
    std::string getFileName() const {
        return file_name_;
    }

    /***************************************************************************/
    /**
	  * @brief Allows to set the canvas label
	  *
	  * @param canvas_label The label to assign to the canvas
	  */
    void setCanvasLabel(const std::string &canvas_label) {
        gpd_.setCanvasLabel(canvas_label);
    }

    /***************************************************************************/
    /**
	  * @brief Allows to retrieve the canvas label
	  *
	  * @return The current canvas label
	  */
    std::string getCanvasLabel() const {
        return gpd_.getCanvasLabel();
    }

    /***************************************************************************/
    /**
	  * @brief Determines a suitable label for a given parPropSpec value
	  *
	  * @param s A parameter-property specification whose variable descriptor is turned into a label
	  * @return A human-readable label string for the given variable
	  */
    std::string getLabel(const gen::parPropSpec<fp_type> &s) const {
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
                g_error_streamer(DO_LOG, Gem::Common::timeAndPlace())
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
	  * @brief Loads the data of another object into this one
	  *
	  * @param cp A pointer to another GProgressPlotterT<fp_type> object, camouflaged as a GBasePluggableOM, whose data is copied
	  */
    void load_(const oa::GBasePluggableOM *cp) override {
        // Check that we are dealing with a GProgressPlotterT<fp_type> reference independent of this object and convert the pointer
        const GProgressPlotterT<fp_type> *p_load = Gem::Common::g_convert_and_compare(cp, this);

        // Load the parent classes' data ...
        oa::GBasePluggableOM::load_(cp);

        // ... the manual tail (value container + GPlotDesigner value member) ...
        Gem::Common::copyCloneableObjectsContainer(p_load->fp_prof_var_vec_, fp_prof_var_vec_);
        gpd_.load(p_load->gpd_);

        // ... and then the unconditionally-handled members, derived from localMembers().
        Gem::Common::g_load_members(localMembers_(*this), localMembers_(*p_load));
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
     * @param cp A constant reference to another GBasePluggableOM object to compare against
     * @param e The expected outcome of the comparison (e.g. equality or inequality)
     * @param limit The maximum allowed deviation for comparisons of floating point types (unused here)
     */
    void compare_(
        const oa::GBasePluggableOM &cp,
        const Gem::Common::expectation &e,
        [[maybe_unused]] const double & limit
    ) const override {
        using namespace Gem::Common;

        // Check that we are dealing with a GProgressPlotterT<fp_type reference independent of this object and convert the pointer
        const GProgressPlotterT<fp_type> *p_load = Gem::Common::g_convert_and_compare(cp, this);

        GToken token("GProgressPlotterT<fp_type>", e);

        // Compare our parent data ...
        Gem::Common::compare_base_t<oa::GBasePluggableOM>(*this, *p_load, token);

        // ... the manual tail (value container + GPlotDesigner value member) ...
        compare_t(Gem::Common::getIdentity(fp_prof_var_vec_, p_load->fp_prof_var_vec_, "fp_prof_var_vec_", "p_load->fp_prof_var_vec_"), token);
        compare_t(Gem::Common::getIdentity(gpd_, p_load->gpd_, "gpd_", "p_load->gpd_"), token);

        // ... and then the unconditionally-handled members, derived from localMembers().
        Gem::Common::g_compare_members(localMembers_(*this), localMembers_(*p_load), token);

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
	  * @brief Emits a name for this class / object
	  *
	  * @return The class name of this object
	  */
    std::string name_() const override {
        return std::string("GProgressPlotterT<fp_type>");
    }

    /************************************************************************/
    /**
	  * @brief Creates a deep clone of this object
	  *
	  * @return A pointer to a freshly allocated deep copy of this object (ownership passes to the caller)
	  */
    oa::GBasePluggableOM *clone_() const override {
        return new GProgressPlotterT<fp_type>(*this);
    }

    /***************************************************************************/
    /**
     * @brief Allows to emit information in different stages of the information cycle
     * (initialization, during each cycle and during finalization)
     *
     * @param im The information mode (initialization, processing or finalization) for this call
     * @param goa A constant pointer to the optimization algorithm whose state is being monitored
     */
    void informationFunction_(infoMode im, oa::GOptimizationAlgorithmBase const *const goa) override {
        switch(im) {
        case Gem::Geneva::infoMode::INFOINIT: {
            switch(this->nProfileVars()) {
            case 1: {
                progress_plotter2_d_oa_ = std::make_shared<Gem::Common::GGraph2D>();

                progress_plotter2_d_oa_->setPlotMode(Gem::Common::graphPlotMode::CURVE);
                progress_plotter2_d_oa_->setPlotLabel("Fitness as a function of a parameter value");
                progress_plotter2_d_oa_->setXAxisLabel(this->getLabel(fp_prof_var_vec_[0]));
                progress_plotter2_d_oa_->setYAxisLabel("Fitness");

                gpd_.registerPlotter(progress_plotter2_d_oa_);
            } break;
            case 2: {
                progress_plotter3_d_oa_ = std::make_shared<Gem::Common::GGraph3D>();

                progress_plotter3_d_oa_->setPlotLabel("Fitness as a function of parameter values");
                progress_plotter3_d_oa_->setXAxisLabel(this->getLabel(fp_prof_var_vec_[0]));
                progress_plotter3_d_oa_->setYAxisLabel(this->getLabel(fp_prof_var_vec_[1]));
                progress_plotter3_d_oa_->setZAxisLabel("Fitness");

                gpd_.registerPlotter(progress_plotter3_d_oa_);
            } break;

            case 3: {
                progress_plotter4_d_oa_ = std::make_shared<Gem::Common::GGraph4D>();

                progress_plotter4_d_oa_->setPlotLabel(
                    "Fitness (color-coded) as a function of parameter values"
                );
                progress_plotter4_d_oa_->setXAxisLabel(this->getLabel(fp_prof_var_vec_[0]));
                progress_plotter4_d_oa_->setYAxisLabel(this->getLabel(fp_prof_var_vec_[1]));
                progress_plotter4_d_oa_->setZAxisLabel(this->getLabel(fp_prof_var_vec_[2]));

                gpd_.registerPlotter(progress_plotter4_d_oa_);
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

            gpd_.setCanvasDimensions(canvas_dimensions_);
        } break;

        case Gem::Geneva::infoMode::INFOPROCESSING: {
            bool is_dirty = true;
            double primary_fitness = 0.;

            if(monitor_best_only_) { // Monitor the best individuals only
                std::shared_ptr<gen::GOptimizableEntity> p =
                    goa->Interface::GOptimizerIT<oa::GOptimizationAlgorithmBase>::template getBestGlobalIndividual<gen::GOptimizableEntity>();
                if(oa::GBasePluggableOM::use_raw_evaluation_) {
                    primary_fitness = p->raw_fitness(0);
                }
                else {
                    primary_fitness = p->transformed_fitness(0);
                }

                if(not monitor_valid_only_ || p->isValid()) {
                    switch(this->nProfileVars()) {
                    case 1: {
                        fp_type val0 = p->GOptimizableEntity::getVarVal<fp_type>(fp_prof_var_vec_[0].var);

                        if(observe_boundaries_) {
                            if(val0 >= fp_prof_var_vec_[0].lowerBoundary &&
                               val0 <= fp_prof_var_vec_[0].upperBoundary) {
                                progress_plotter2_d_oa_->add(double(val0), primary_fitness);
                            }
                        }
                        else {
                            progress_plotter2_d_oa_->add(double(val0), primary_fitness);
                        }
                    } break;

                    case 2: {
                        fp_type val0 = p->GOptimizableEntity::getVarVal<fp_type>(fp_prof_var_vec_[0].var);
                        fp_type val1 = p->GOptimizableEntity::getVarVal<fp_type>(fp_prof_var_vec_[1].var);

                        if(observe_boundaries_) {
                            if(val0 >= fp_prof_var_vec_[0].lowerBoundary &&
                               val0 <= fp_prof_var_vec_[0].upperBoundary &&
                               val1 >= fp_prof_var_vec_[1].lowerBoundary &&
                               val1 <= fp_prof_var_vec_[1].upperBoundary) {
                                progress_plotter3_d_oa_->add(
                                    std::tuple<double, double, double>(
                                        double(val0),
                                        double(val1),
                                        primary_fitness
                                    )
                                );
                            }
                        }
                        else {
                            progress_plotter3_d_oa_->add(
                                std::tuple<double, double, double>(
                                    double(val0),
                                    double(val1),
                                    primary_fitness
                                )
                            );
                        }
                    } break;

                    case 3: {
                        fp_type val0 = p->GOptimizableEntity::getVarVal<fp_type>(fp_prof_var_vec_[0].var);
                        fp_type val1 = p->GOptimizableEntity::getVarVal<fp_type>(fp_prof_var_vec_[1].var);
                        fp_type val2 = p->GOptimizableEntity::getVarVal<fp_type>(fp_prof_var_vec_[2].var);

                        if(observe_boundaries_) {
                            if(val0 >= fp_prof_var_vec_[0].lowerBoundary &&
                               val0 <= fp_prof_var_vec_[0].upperBoundary &&
                               val1 >= fp_prof_var_vec_[1].lowerBoundary &&
                               val1 <= fp_prof_var_vec_[1].upperBoundary &&
                               val2 >= fp_prof_var_vec_[2].lowerBoundary &&
                               val2 <= fp_prof_var_vec_[2].upperBoundary) {
                                progress_plotter4_d_oa_->add(
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
                            progress_plotter4_d_oa_->add(
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
                    if(oa::GBasePluggableOM::use_raw_evaluation_) {
                        primary_fitness = ind_ptr->individual().raw_fitness(0);
                    }
                    else {
                        primary_fitness = ind_ptr->individual().transformed_fitness(0);
                    }

                    if(not monitor_valid_only_ || ind_ptr->individual().isValid()) {
                        switch(this->nProfileVars()) {
                        case 1: {
                            fp_type val0 = ind_ptr->individual().template getVarVal<fp_type>(
                                fp_prof_var_vec_[0].var
                            );

                            if(observe_boundaries_) {
                                if(val0 >= fp_prof_var_vec_[0].lowerBoundary &&
                                   val0 <= fp_prof_var_vec_[0].upperBoundary) {
                                    progress_plotter2_d_oa_->add(double(val0), primary_fitness);
                                }
                            }
                            else {
                                progress_plotter2_d_oa_->add(double(val0), primary_fitness);
                            }
                        } break;

                        case 2: {
                            fp_type val0 = ind_ptr->individual().template getVarVal<fp_type>(
                                fp_prof_var_vec_[0].var
                            );
                            fp_type val1 = ind_ptr->individual().template getVarVal<fp_type>(
                                fp_prof_var_vec_[1].var
                            );

                            if(observe_boundaries_) {
                                if(val0 >= fp_prof_var_vec_[0].lowerBoundary &&
                                   val0 <= fp_prof_var_vec_[0].upperBoundary &&
                                   val1 >= fp_prof_var_vec_[1].lowerBoundary &&
                                   val1 <= fp_prof_var_vec_[1].upperBoundary) {
                                    progress_plotter3_d_oa_->add(
                                        std::tuple<double, double, double>(
                                            double(val0),
                                            double(val1),
                                            primary_fitness
                                        )
                                    );
                                }
                            }
                            else {
                                progress_plotter3_d_oa_->add(
                                    std::tuple<double, double, double>(
                                        double(val0),
                                        double(val1),
                                        primary_fitness
                                    )
                                );
                            }
                        } break;

                        case 3: {
                            fp_type val0 = ind_ptr->individual().template getVarVal<fp_type>(
                                fp_prof_var_vec_[0].var
                            );
                            fp_type val1 = ind_ptr->individual().template getVarVal<fp_type>(
                                fp_prof_var_vec_[1].var
                            );
                            fp_type val2 = ind_ptr->individual().template getVarVal<fp_type>(
                                fp_prof_var_vec_[2].var
                            );

                            if(observe_boundaries_) {
                                if(val0 >= fp_prof_var_vec_[0].lowerBoundary &&
                                   val0 <= fp_prof_var_vec_[0].upperBoundary &&
                                   val1 >= fp_prof_var_vec_[1].lowerBoundary &&
                                   val1 <= fp_prof_var_vec_[1].upperBoundary &&
                                   val2 >= fp_prof_var_vec_[2].lowerBoundary &&
                                   val2 <= fp_prof_var_vec_[2].upperBoundary) {
                                    progress_plotter4_d_oa_->add(
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
                                progress_plotter4_d_oa_->add(
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
                progress_plotter2_d_oa_->sortX();
            }

            // Inform the plot designer whether it should print png files
            gpd_.setAddPrintCommand(add_print_command_);

            // Write out the result.
            gpd_.writeToFile(file_name_);

            // Remove all plotters
            gpd_.resetPlotters();
            progress_plotter2_d_oa_.reset();
            progress_plotter3_d_oa_.reset();
            progress_plotter4_d_oa_.reset();
        } break;
        };
    }

    /************************************************************************/

    std::vector<gen::parPropSpec<fp_type>>
        fp_prof_var_vec_; ///< Holds information about variables to be profiled

    Gem::Common::GPlotDesigner gpd_{"Progress information", 1, 1}; ///< A wrapper for the plots

    // These are temporaries
    std::shared_ptr<Gem::Common::GGraph2D> progress_plotter2_d_oa_;
    std::shared_ptr<Gem::Common::GGraph3D> progress_plotter3_d_oa_;
    std::shared_ptr<Gem::Common::GGraph4D> progress_plotter4_d_oa_;

    std::string file_name_ = std::string(
        "progressScan.C"
    ); ///< The name of the file the output should be written to. Note that the class will add the name of the algorithm it acts on
    std::tuple<std::uint32_t, std::uint32_t> canvas_dimensions_ =
        std::tuple<std::uint32_t, std::uint32_t>(1024, 768); ///< The dimensions of the canvas

    bool monitor_best_only_ =
        false; ///< Indicates whether only the best individuals should be monitored
    bool monitor_valid_only_ = false; ///< Indicates whether only valid individuals should be plotted
    bool observe_boundaries_ =
        false; ///< When set to true, the plotter will ignore values outside of a scan boundary

    bool add_print_command_ =
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
 * and is determined from the individual. Note that this class operates on the
 * GOptimizableEntity hierarchy (the flat-genome GFlatGenome / its GenomeData), i.e. on
 * the optimizable entities managed by the algorithm.
 */
class GAllSolutionFileLogger // NOLINT(cppcoreguidelines-special-member-functions)
  : public oa::GBasePluggableOM {
    ///////////////////////////////////////////////////////////////////////
    friend class boost::serialization::access;

    /***************************************************************************/
    /**
     * Single declaration of this class'es local data members, driving
     * serialize(), load_() and compare_() from one place. All members are plain
     * config values (make_member); no manual tail is needed.
     */
    template <typename Self>
    static auto localMembers_(Self &self) {
        return std::make_tuple(
            Gem::Common::make_member("file_name_", self.file_name_),
            Gem::Common::make_member("boundaries_", self.boundaries_),
            Gem::Common::make_member("boundaries_active_", self.boundaries_active_),
            Gem::Common::make_member("with_name_and_type_", self.with_name_and_type_),
            Gem::Common::make_member("with_commas_", self.with_commas_),
            Gem::Common::make_member("use_raw_fitness_", self.use_raw_fitness_),
            Gem::Common::make_member("show_validity_", self.show_validity_),
            Gem::Common::make_member("print_initial_", self.print_initial_),
            Gem::Common::make_member("show_iteration_boundaries_", self.show_iteration_boundaries_)
        );
    }

    template <typename Archive>
    void serialize(Archive &ar, const unsigned int) {
        using boost::serialization::make_nvp;

        ar &make_nvp(
            "GBasePluggableOM",
            boost::serialization::base_object<oa::GBasePluggableOM>(*this)
        );

        // All members are derived from the single localMembers() declaration.
        Gem::Common::serialize_members(ar, localMembers_(*this));
    }
    ///////////////////////////////////////////////////////////////////////

public:
    /***************************************************************************/

    /** @brief The default constructor */
    GAllSolutionFileLogger() = default;
    /**
     * @brief Initialization with a file name
     * @param file_name The name of the file to which solutions should be logged
     */
    explicit GAllSolutionFileLogger(const std::string &file_name);
    /**
     * @brief Initialization with a file name and boundaries
     * @param file_name The name of the file to which solutions should be logged
     * @param boundaries Value boundaries used to filter which solutions are logged
     */
    GAllSolutionFileLogger(const std::string &file_name, const std::vector<double> &boundaries);
    /** @brief The copy constructor */
    GAllSolutionFileLogger(const GAllSolutionFileLogger &cp) = default;
    /** @brief The destructor */
    ~GAllSolutionFileLogger() override = default;

    /**
     * @brief Sets the file name
     * @param file_name The name of the file to which solutions should be logged
     */
    void setFileName(const std::string &file_name);
    /**
     * @brief Retrieves the current file name
     * @return The name of the file to which solutions are logged
     */
    std::string getFileName() const;

    /**
     * @brief Sets the boundaries
     * @param boundaries Value boundaries used to filter which solutions are logged
     */
    void setBoundaries(const std::vector<double> &boundaries);
    /**
     * @brief Allows to retrieve the boundaries
     * @return The value boundaries used to filter which solutions are logged
     */
    std::vector<double> getBoundaries() const;
    /**
     * @brief Allows to check whether boundaries are active
     * @return true if boundaries have been set and are active, false otherwise
     */
    bool boundariesActive() const;
    /** @brief Allows to inactivate boundaries */
    void setBoundariesInactive();

    /**
     * @brief Allows to specify whether explanations should be printed for parameter- and fitness values.
     * @param with_name_and_type If true, a header line with variable names and types is prepended
     */
    void setPrintWithNameAndType(bool with_name_and_type = true);
    /**
     * @brief Allows to check whether explanations should be printed for parameter-and fitness values
     * @return true if a header line with variable names and types is prepended, false otherwise
     */
    bool getPrintWithNameAndType() const;

    /**
     * @brief Allows to specify whether commas should be printed in-between values
     * @param with_commas If true, commas are printed in-between values
     */
    void setPrintWithCommas(bool with_commas = true);
    /**
     * @brief Allows to check whether commas should be printed in-between values
     * @return true if commas are printed in-between values, false otherwise
     */
    bool getPrintWithCommas() const;

    /**
     * @brief Allows to specify whether the true (instead of the transformed) fitness should be shown
     * @param use_raw_fitness If true, the raw (true) fitness is shown instead of the transformed fitness
     */
    void setUseTrueFitness(bool use_raw_fitness = true);
    /**
     * @brief Allows to retrieve whether the true (instead of the transformed) fitness should be shown
     * @return true if the raw (true) fitness is shown, false if the transformed fitness is shown
     */
    bool getUseTrueFitness() const;

    /**
     * @brief Allows to specify whether the validity of a solution should be shown
     * @param show_validity If true, the validity of each solution is shown
     */
    void setShowValidity(bool show_validity = true);
    /**
     * @brief Allows to check whether the validity of a solution will be shown
     * @return true if the validity of each solution is shown, false otherwise
     */
    bool getShowValidity() const;

    /**
     * @brief Allows to specify whether the initial population should be printed.
     * @param print_initial If true, the initial population is also printed
     */
    void setPrintInitial(bool print_initial = true);
    /**
     * @brief Allows to check whether the initial population should be printed.
     * @return true if the initial population is printed, false otherwise
     */
    bool getPrintInitial() const;

    /**
     * @brief Allows to specify whether a comment line should be inserted between iterations
     * @param show_iteration_boundaries If true, a comment line is inserted between iterations
     */
    void setShowIterationBoundaries(bool show_iteration_boundaries = true);
    /**
     * @brief Allows to check whether a comment line should be inserted between iterations
     * @return true if a comment line is inserted between iterations, false otherwise
     */
    bool getShowIterationBoundaries() const;

protected:
    /************************************************************************/

    /**
     * @brief Loads the data of another object into this one
     * @param cp A pointer to another object of this type, camouflaged as a GBasePluggableOM, whose data is copied
     */
    void load_(const oa::GBasePluggableOM *cp) override;

    /** @brief Allow access to this classes compare_ function */
    friend void Gem::Common::compare_base_t<GAllSolutionFileLogger>(
        GAllSolutionFileLogger const &,
        GAllSolutionFileLogger const &,
        Gem::Common::GToken &
    );

    /**
     * @brief Searches for compliance with expectations with respect to another object of the same type
     * @param cp A constant reference to another GBasePluggableOM object to compare against
     * @param e The expected outcome of the comparison (e.g. equality or inequality)
     * @param limit The maximum allowed deviation for comparisons of floating point types
     */
    void compare_(
        const oa::GBasePluggableOM &cp,
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
    /**
     * @brief Emits a name for this class / object
     * @return The human-readable class name of this object
     */
    std::string name_() const override;
    /**
     * @brief Creates a deep clone of this object
     * @return A pointer to a freshly allocated deep copy of this object (ownership passes to the caller)
     */
    oa::GBasePluggableOM *clone_() const override;

    /**
     * @brief Allows to emit information in different stages of the information cycle
     *
     * The first (unnamed) argument is the information mode (init, processing or finalization).
     * @param goa A constant pointer to the optimization algorithm whose state is being monitored
     */
    void
    informationFunction_(infoMode, oa::GOptimizationAlgorithmBase const *const goa) override;

    /**
     * @brief Does the actual printing of the population to the log file
     * @param iteration_description A textual description of the current iteration, used in the output
     * @param goa A constant pointer to the optimization algorithm whose population is printed
     */
    void printPopulation(
        const std::string &iteration_description,
        oa::GOptimizationAlgorithmBase const *const goa
    );

    /***************************************************************************/
    // Data

    std::string file_name_ =
        "CompleteSolutionLog.txt";    ///< The name of the file to which solutions should be stored
    std::vector<double> boundaries_; ///< Value boundaries used to filter logged solutions
    bool boundaries_active_ = false;  ///< Set to true if boundaries have been set
    bool with_name_and_type_ = false;   ///< When set to true, explanations for values are printed
    bool with_commas_ = false; ///< When set to true, commas will be printed in-between values
    bool use_raw_fitness_ =
        true;                   ///< Indicates whether true- or transformed fitness should be output
    bool show_validity_ = true; ///< Indicates whether the validity of a solution should be shown
    bool print_initial_ = false; ///< Indicates whether the initial population should be printed
    bool show_iteration_boundaries_ =
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

    /***************************************************************************/
    /**
     * Single declaration of this class'es local data members, driving
     * serialize(), load_() and compare_() from one place. All members are plain
     * config values (make_member); no manual tail is needed.
     */
    template <typename Self>
    static auto localMembers_(Self &self) {
        return std::make_tuple(
            Gem::Common::make_member("file_name_", self.file_name_),
            Gem::Common::make_member("with_commas_", self.with_commas_),
            Gem::Common::make_member("use_raw_fitness_", self.use_raw_fitness_)
        );
    }

    template <typename Archive>
    void serialize(Archive &ar, const unsigned int) {
        using boost::serialization::make_nvp;

        ar &make_nvp(
            "GBasePluggableOM",
            boost::serialization::base_object<oa::GBasePluggableOM>(*this)
        );

        // All members are derived from the single localMembers() declaration.
        Gem::Common::serialize_members(ar, localMembers_(*this));
    }
    ///////////////////////////////////////////////////////////////////////

public:
    /***************************************************************************/

    /** @brief The default constructor */
    GIterationResultsFileLogger() = default;
    /**
     * @brief Initialization with a file name
     * @param file_name The name of the file to which iteration results should be logged
     */
    explicit GIterationResultsFileLogger(const std::string &file_name);
    /** @brief The copy constructor */
    GIterationResultsFileLogger(const GIterationResultsFileLogger &cp) = default;
    /** @brief The destructor */
    ~GIterationResultsFileLogger() override = default;

    /**
     * @brief Sets the file name
     * @param file_name The name of the file to which iteration results should be logged
     */
    void setFileName(const std::string &file_name);
    /**
     * @brief Retrieves the current file name
     * @return The name of the file to which iteration results are logged
     */
    std::string getFileName() const;

    /**
     * @brief Allows to specify whether commas should be printed in-between values
     * @param with_commas If true, commas are printed in-between values
     */
    void setPrintWithCommas(bool with_commas);
    /**
     * @brief Allows to check whether commas should be printed in-between values
     * @return true if commas are printed in-between values, false otherwise
     */
    bool getPrintWithCommas() const;

    /**
     * @brief Allows to specify whether the true (instead of the transformed) fitness should be shown
     * @param use_raw_fitness If true, the raw (true) fitness is shown instead of the transformed fitness
     */
    void setUseTrueFitness(bool use_raw_fitness);
    /**
     * @brief Allows to retrieve whether the true (instead of the transformed) fitness should be shown
     * @return true if the raw (true) fitness is shown, false if the transformed fitness is shown
     */
    bool getUseTrueFitness() const;

protected:
    /************************************************************************/
    /**
     * @brief Loads the data of another object into this one
     * @param cp A pointer to another object of this type, camouflaged as a GBasePluggableOM, whose data is copied
     */
    void load_(const oa::GBasePluggableOM *cp) override;

    /** @brief Allow access to this classes compare_ function */
    friend void Gem::Common::compare_base_t<GIterationResultsFileLogger>(
        GIterationResultsFileLogger const &,
        GIterationResultsFileLogger const &,
        Gem::Common::GToken &
    );

    /**
     * @brief Searches for compliance with expectations with respect to another object of the same type
     * @param cp A constant reference to another GBasePluggableOM object to compare against
     * @param e The expected outcome of the comparison (e.g. equality or inequality)
     * @param limit The maximum allowed deviation for comparisons of floating point types
     */
    void compare_(
        const oa::GBasePluggableOM &cp,
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
    /**
     * @brief Emits a name for this class / object
     * @return The human-readable class name of this object
     */
    std::string name_() const override;
    /**
     * @brief Creates a deep clone of this object
     * @return A pointer to a freshly allocated deep copy of this object (ownership passes to the caller)
     */
    oa::GBasePluggableOM *clone_() const override;

    /**
     * @brief Allows to emit information in different stages of the information cycle
     *
     * The first (unnamed) argument is the information mode (init, processing or finalization).
     * @param goa A constant pointer to the optimization algorithm whose state is being monitored
     */
    void
    informationFunction_(infoMode, oa::GOptimizationAlgorithmBase const *const goa) override;

    std::string file_name_ =
        "IterationResultsLog.txt"; ///< The name of the file to which solutions should be stored
    bool with_commas_ = true;      ///< When set to true, commas will be printed in-between values
    bool use_raw_fitness_ =
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

    /***************************************************************************/
    /**
     * Single declaration of this class'es local data members, driving
     * serialize(), load_() and compare_() from one place. All members are
     * handled unconditionally: the three GGraph/GHistogram smart pointers are
     * deep-cloned on load (make_cloneable_member); every other member (including
     * the gpd_ GPlotDesigner value, which load_() assigns plainly) uses
     * make_member. No manual tail is needed.
     */
    template <typename Self>
    static auto localMembers_(Self &self) {
        return std::make_tuple(
            Gem::Common::make_member("file_name_", self.file_name_),
            Gem::Common::make_member("canvas_dimensions_", self.canvas_dimensions_),
            Gem::Common::make_member("gpd_", self.gpd_),
            Gem::Common::make_cloneable_member("n_adaptions_hist2_d_oa_", self.n_adaptions_hist2_d_oa_),
            Gem::Common::make_cloneable_member("n_adaptions_graph2_d_oa_", self.n_adaptions_graph2_d_oa_),
            Gem::Common::make_cloneable_member("fitness_graph2_d_oa_", self.fitness_graph2_d_oa_),
            Gem::Common::make_member("monitor_best_only_", self.monitor_best_only_),
            Gem::Common::make_member("add_print_command_", self.add_print_command_),
            Gem::Common::make_member("max_iteration_", self.max_iteration_),
            Gem::Common::make_member("n_iterations_recorded_", self.n_iterations_recorded_),
            Gem::Common::make_member("n_adaptions_store_", self.n_adaptions_store_)
        );
    }

    template <typename Archive>
    void serialize(Archive &ar, const unsigned int) {
        using boost::serialization::make_nvp;

        ar &make_nvp(
            "GBasePluggableOM",
            boost::serialization::base_object<oa::GBasePluggableOM>(*this)
        );

        // All members are derived from the single localMembers() declaration.
        Gem::Common::serialize_members(ar, localMembers_(*this));
    }
    ///////////////////////////////////////////////////////////////////////

public:
    /***************************************************************************/

    /** @brief The default constructor */
    GNAdpationsLogger() = default;
    /**
     * @brief Initialization with a file name
     * @param file_name The name of the file to which the number of adaptions should be logged
     */
    explicit GNAdpationsLogger(const std::string &file_name);
    /**
     * @brief The copy constructor
     * @param cp A constant reference to another GNAdpationsLogger object whose data is deep-copied
     */
    GNAdpationsLogger(const GNAdpationsLogger &cp);
    /** @brief The destructor */
    ~GNAdpationsLogger() override = default;

    /**
     * @brief Sets the file name
     * @param file_name The name of the file to which the number of adaptions should be logged
     */
    void setFileName(const std::string &file_name);
    /**
     * @brief Retrieves the current file name
     * @return The name of the file to which the number of adaptions is logged
     */
    std::string getFileName() const;

    /**
     * @brief Allows to specify whether only the best individuals should be monitored
     * @param monitor_best_only If true, only the best individual(s) of the population are monitored
     */
    void setMonitorBestOnly(bool monitor_best_only = true);
    /**
     * @brief Allows to check whether only the best individuals should be monitored
     * @return true if only the best individual(s) are monitored, false otherwise
     */
    bool getMonitorBestOnly() const;

    /**
     * @brief Allows to set the canvas dimensions
     * @param canvas_dimensions A tuple holding the canvas dimensions in x- and y-direction
     */
    void setCanvasDimensions(std::tuple<std::uint32_t, std::uint32_t> canvas_dimensions);
    /**
     * @brief Allows to set the canvas dimensions using separate x and y values
     * @param x The canvas dimension in x-direction
     * @param y The canvas dimension in y-direction
     */
    void setCanvasDimensions(std::uint32_t x, std::uint32_t y);
    /**
     * @brief Gives access to the canvas dimensions
     * @return A tuple holding the canvas dimensions in x- and y-direction
     */
    std::tuple<std::uint32_t, std::uint32_t> getCanvasDimensions() const;

    /**
     * @brief Allows to add a "Print" command to the end of the script so that picture files are created
     * @param add_print_command If true, a print command is appended so that picture files are created
     */
    void setAddPrintCommand(bool add_print_command);
    /**
     * @brief Allows to retrieve the current value of the add_print_command_ variable
     * @return true if a print command will be appended to the script, false otherwise
     */
    bool getAddPrintCommand() const;

protected:
    /************************************************************************/

    /**
     * @brief Loads the data of another object into this one
     * @param cp A pointer to another object of this type, camouflaged as a GBasePluggableOM, whose data is copied
     */
    void load_(const oa::GBasePluggableOM *cp) override;

    /** @brief Allow access to this classes compare_ function */
    friend void Gem::Common::compare_base_t<GNAdpationsLogger>(
        GNAdpationsLogger const &,
        GNAdpationsLogger const &,
        Gem::Common::GToken &
    );

    /**
     * @brief Searches for compliance with expectations with respect to another object
     * @param cp A constant reference to another GBasePluggableOM object to compare against
     * @param e The expected outcome of the comparison (e.g. equality or inequality)
     * @param limit The maximum allowed deviation for comparisons of floating point types
     */
    void compare_(
        const oa::GBasePluggableOM &cp,
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
    /**
     * @brief Creates a deep clone of this object
     * @return A pointer to a freshly allocated deep copy of this object (ownership passes to the caller)
     */
    oa::GBasePluggableOM *clone_() const override;

    /**
     * @brief Allows to emit information in different stages of the information cycle
     *
     * The first (unnamed) argument is the information mode (init, processing or finalization).
     * @param goa A constant pointer to the optimization algorithm whose state is being monitored
     */
    void
    informationFunction_(infoMode, oa::GOptimizationAlgorithmBase const *const goa) override;

    std::string file_name_ =
        "NAdaptions.C"; ///< The name of the file to which solutions should be stored

    std::tuple<std::uint32_t, std::uint32_t> canvas_dimensions_ =
        std::tuple<std::uint32_t, std::uint32_t>(1200, 1600); ///< The dimensions of the canvas

    Gem::Common::GPlotDesigner gpd_{
        "Number of adaptions per iteration",
        1,
        2
    }; ///< A wrapper for the plots

    std::shared_ptr<Gem::Common::GHistogram2D>
        n_adaptions_hist2_d_oa_; ///< Holds the actual histogram
    std::shared_ptr<Gem::Common::GGraph2D>
        n_adaptions_graph2_d_oa_; ///< Used if we only monitor the best solution in each iteration
    std::shared_ptr<Gem::Common::GGraph2D>
        fitness_graph2_d_oa_; ///< Lets us monitor the current fitness of the population

    bool monitor_best_only_ =
        false; ///< Indicates whether only the best individuals should be monitored
    bool add_print_command_ =
        false; ///< Asks the GPlotDesigner to add a print command to result files

    std::size_t max_iteration_ = 0; ///< Holds the largest iteration recorded for the algorithm
    std::size_t n_iterations_recorded_ =
        0; ///< Holds the number of iterations that were recorded (not necessarily == max_iteration_

    std::vector<std::tuple<double, double>>
        n_adaptions_store_; ///< Holds all information about the number of adaptions
};

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/
/**
 * This class allows to log chosen properties of adaptors. Such properties
 * are limited to numeric entities, that may be converted to double
 *
 * @tparam num_type The arithmetic type of the logged adaptor property (convertible to double)
 */
template <typename num_type>
class GAdaptorPropertyLoggerT // NOLINT(cppcoreguidelines-special-member-functions)
  : public oa::GBasePluggableOM {
    ///////////////////////////////////////////////////////////////////////
    friend class boost::serialization::access;

    /***************************************************************************/
    /**
     * Single declaration of this class'es local data members, driving
     * serialize(), load_() and compare_() from one place. All members are
     * handled unconditionally: the two GGraph/GHistogram smart pointers are
     * deep-cloned on load (make_cloneable_member), every other member (including
     * the gpd_ GPlotDesigner value, which load_() assigns plainly here) uses
     * make_member. No manual tail is needed.
     */
    template <typename Self>
    static auto localMembers_(Self &self) {
        return std::make_tuple(
            Gem::Common::make_member("file_name_", self.file_name_),
            Gem::Common::make_member("adaptor_name_", self.adaptor_name_),
            Gem::Common::make_member("property_", self.property_),
            Gem::Common::make_member("canvas_dimensions_", self.canvas_dimensions_),
            Gem::Common::make_member("gpd_", self.gpd_),
            Gem::Common::make_cloneable_member("adaptor_property_hist2_d_oa_", self.adaptor_property_hist2_d_oa_),
            Gem::Common::make_cloneable_member("fitness_graph2_d_oa_", self.fitness_graph2_d_oa_),
            Gem::Common::make_member("monitor_best_only_", self.monitor_best_only_),
            Gem::Common::make_member("add_print_command_", self.add_print_command_),
            Gem::Common::make_member("max_iteration_", self.max_iteration_),
            Gem::Common::make_member("n_iterations_recorded_", self.n_iterations_recorded_),
            Gem::Common::make_member("adaptor_property_store_", self.adaptor_property_store_)
        );
    }

    template <typename Archive>
    void serialize(Archive &ar, const unsigned int) {
        using boost::serialization::make_nvp;

        ar &make_nvp(
            "GBasePluggableOM",
            boost::serialization::base_object<oa::GBasePluggableOM>(*this)
        );

        // All members are derived from the single localMembers() declaration.
        Gem::Common::serialize_members(ar, localMembers_(*this));
    }
    ///////////////////////////////////////////////////////////////////////

    static_assert(std::is_arithmetic_v<num_type>, "num_type should be an arithmetic type");

public:
    /***************************************************************************/
    /**
	  * The default constructor. Note that some parmeters may be initialized in
	  * the class body.
	  */
    GAdaptorPropertyLoggerT() = default;

    /***************************************************************************/
    /**
	  * @brief Initialization with a file name, adaptor name and property name
	  *
	  * @param file_name The name of the file the logged property should be written to
	  * @param adaptor_name The name of the adaptor whose property should be logged
	  * @param property The name of the property to be logged (e.g. "sigma")
	  */
    GAdaptorPropertyLoggerT(std::string file_name, std::string adaptor_name, std::string property)
      : file_name_(std::move(file_name))
      , adaptor_name_(std::move(adaptor_name))
      , property_(std::move(property))
      , canvas_dimensions_(std::tuple<std::uint32_t, std::uint32_t>(1200, 1600))
      , gpd_("Adaptor properties", 1, 2) { /* nothing */
    }

    /***************************************************************************/
    /**
	  * @brief The copy constructor
	  *
	  * @param cp A constant reference to another GAdaptorPropertyLoggerT object whose data is deep-copied
	  */
    GAdaptorPropertyLoggerT(const GAdaptorPropertyLoggerT<num_type> &cp)
      : oa::GBasePluggableOM(cp)
      , file_name_(cp.file_name_)
      , adaptor_name_(cp.adaptor_name_)
      , property_(cp.property_)
      , canvas_dimensions_(cp.canvas_dimensions_)
      , gpd_(cp.gpd_)
      , monitor_best_only_(cp.monitor_best_only_)
      , add_print_command_(cp.add_print_command_)
      , max_iteration_(cp.max_iteration_)
      , n_iterations_recorded_(cp.n_iterations_recorded_)
      , adaptor_property_store_(cp.adaptor_property_store_) {
        // Copy the smart pointers over
        Gem::Common::copyCloneableSmartPointer(
            cp.adaptor_property_hist2_d_oa_,
            adaptor_property_hist2_d_oa_
        );
        Gem::Common::copyCloneableSmartPointer(cp.fitness_graph2_d_oa_, fitness_graph2_d_oa_);
    }

    /***************************************************************************/
    /**
	  * The destructor
	  */
    ~GAdaptorPropertyLoggerT() override = default;

    /***************************************************************************/
    /**
	  * @brief Sets the file name
	  *
	  * @param file_name The name of the file the logged property should be written to
	  */
    void setFileName(const std::string &file_name) {
        file_name_ = file_name;
    }

    /***************************************************************************/
    /**
	  * @brief Retrieves the current file name
	  *
	  * @return The name of the file the logged property is written to
	  */
    std::string getFileName() const {
        return file_name_;
    }

    /***************************************************************************/
    /**
	  * @brief Sets the name of the adaptor
	  *
	  * @param adaptor_name The name of the adaptor whose property should be logged
	  */
    void setAdaptorName(std::string adaptor_name) {
        adaptor_name_ = adaptor_name;
    }

    /***************************************************************************/
    /**
	  * @brief Retrieves the name of the adaptor
	  *
	  * @return The name of the adaptor whose property is logged
	  */
    std::string getAdaptorName() const {
        return adaptor_name_;
    }

    /***************************************************************************/
    /**
	  * @brief Sets the name of the property
	  *
	  * @param property The name of the property to be logged (e.g. "sigma")
	  */
    void setPropertyName(std::string property) {
        property_ = property;
    }

    /***************************************************************************/
    /**
	  * @brief Retrieves the name of the property
	  *
	  * @return The name of the property being logged
	  */
    std::string getPropertyName() const {
        return property_;
    }

    /***************************************************************************/
    /**
	  * @brief Allows to specify whether only the best individuals should be monitored.
	  *
	  * @param monitor_best_only If true, only the best individual(s) of the population are monitored
	  */
    void setMonitorBestOnly(bool monitor_best_only = true) {
        monitor_best_only_ = monitor_best_only;
    }

    /***************************************************************************/
    /**
	  * @brief Allows to check whether only the best individuals should be monitored.
	  *
	  * @return true if only the best individual(s) are monitored, false otherwise
	  */
    bool getMonitorBestOnly() const {
        return monitor_best_only_;
    }

    /***************************************************************************/
    /**
	  * @brief Allows to set the canvas dimensions
	  *
	  * @param canvas_dimensions A tuple holding the canvas dimensions in x- and y-direction
	  */
    void setCanvasDimensions(std::tuple<std::uint32_t, std::uint32_t> canvas_dimensions) {
        canvas_dimensions_ = canvas_dimensions;
    }

    /***************************************************************************/
    /**
	  * @brief Allows to set the canvas dimensions using separate x and y values
	  *
	  * @param x The canvas dimension in x-direction
	  * @param y The canvas dimension in y-direction
	  */
    void setCanvasDimensions(std::uint32_t x, std::uint32_t y) {
        canvas_dimensions_ = std::tuple<std::uint32_t, std::uint32_t>(x, y);
    }

    /***************************************************************************/
    /**
	  * @brief Gives access to the canvas dimensions
	  *
	  * @return A tuple holding the canvas dimensions in x- and y-direction
	  */
    std::tuple<std::uint32_t, std::uint32_t> getCanvasDimensions() const {
        return canvas_dimensions_;
    }

    /******************************************************************************/
    /**
	  * @brief Allows to add a "Print" command to the end of the script so that picture files are created
	  *
	  * @param add_print_command If true, a print command is appended so that picture files are created
	  */
    void setAddPrintCommand(bool add_print_command) {
        add_print_command_ = add_print_command;
    }

    /******************************************************************************/
    /**
	  * @brief Allows to retrieve the current value of the add_print_command_ variable
	  *
	  * @return true if a print command will be appended to the script, false otherwise
	  */
    bool getAddPrintCommand() const {
        return add_print_command_;
    }

protected:
    /************************************************************************/
    /**
	  * @brief Loads the data of another object into this one
	  *
	  * @param cp A pointer to another GAdaptorPropertyLoggerT<num_type> object, camouflaged as a GBasePluggableOM, whose data is copied
	  */
    void load_(const oa::GBasePluggableOM *cp) override {
        // Check that we are dealing with a GAdaptorPropertyLoggerT<num_type> reference independent of this object and convert the pointer
        const GAdaptorPropertyLoggerT<num_type> *p_load =
            Gem::Common::g_convert_and_compare<oa::GBasePluggableOM, GAdaptorPropertyLoggerT<num_type>>(
                cp,
                this
            );

        // Load the parent classes' data ...
        oa::GBasePluggableOM::load_(cp);

        // ... and then all local data, derived from the single localMembers() declaration.
        Gem::Common::g_load_members(localMembers_(*this), localMembers_(*p_load));
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
     * @param cp A constant reference to another GBasePluggableOM object to compare against
     * @param e The expected outcome of the comparison (e.g. equality or inequality)
     * @param limit The maximum allowed deviation for comparisons of floating point types (unused here)
     */
    void compare_(
        const oa::GBasePluggableOM &cp,
        const Gem::Common::expectation &e,
        [[maybe_unused]] const double & limit
    ) const override {
        using namespace Gem::Common;

        // Check that we are dealing with a GAdaptorPropertyLoggerT<num_type> reference independent of this object and convert the pointer
        const GAdaptorPropertyLoggerT<num_type> *p_load =
            Gem::Common::g_convert_and_compare<oa::GBasePluggableOM, GAdaptorPropertyLoggerT<num_type>>(
                cp,
                this
            );

        GToken token("GAdaptorPropertyLoggerT", e);

        // Compare our parent data ...
        Gem::Common::compare_base_t<oa::GBasePluggableOM>(*this, *p_load, token);

        // ... and then all local data, derived from the single localMembers() declaration.
        Gem::Common::g_compare_members(localMembers_(*this), localMembers_(*p_load), token);

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
	  * @brief Emits a name for this class / object
	  *
	  * @return The class name of this object
	  */
    std::string name_() const override {
        return std::string("GAdaptorPropertyLoggerT");
    }

    /************************************************************************/
    /**
	  * @brief Creates a deep clone of this object
	  *
	  * @return A pointer to a freshly allocated deep copy of this object (ownership passes to the caller)
	  */
    oa::GBasePluggableOM *clone_() const override {
        return new GAdaptorPropertyLoggerT<num_type>(*this);
    }

    /***************************************************************************/
    /**
     * @brief Allows to emit information in different stages of the information cycle
     * (initialization, during each cycle and during finalization)
     *
     * @param im The information mode (initialization, processing or finalization) for this call
     * @param goa A constant pointer to the optimization algorithm whose state is being monitored
     */
    void informationFunction_(infoMode im, oa::GOptimizationAlgorithmBase const *const goa) override {
        using namespace Gem::Common;

        switch(im) {
        case Gem::Geneva::infoMode::INFOINIT: {
            // If the file pointed to by file_name_ already exists, make a back-up
            if(std::filesystem::exists(file_name_)) {
                std::string new_file_name =
                    file_name_ + ".bak_" +
                    Gem::Common::getMSSince1970(); // NOLINT(cppcoreguidelines-init-variables)

                glogger << "In GAdaptorPropertyLoggerT::informationFunction_(): Error!" << '\n'
                        << "Attempt to output information to file " << file_name_ << '\n'
                        << "which already exists. We will rename the old file to" << '\n'
                        << new_file_name << '\n'
                        << GWARNING;

                std::filesystem::rename(file_name_, new_file_name);
            }

            // Make sure the progress plotter has the desired size
            gpd_.setCanvasDimensions(canvas_dimensions_);

            // Set up a graph to monitor the best fitness found
            fitness_graph2_d_oa_ = std::make_shared<Gem::Common::GGraph2D>();
            fitness_graph2_d_oa_->setXAxisLabel("Iteration");
            fitness_graph2_d_oa_->setYAxisLabel("Fitness");
            fitness_graph2_d_oa_->setPlotMode(Gem::Common::graphPlotMode::CURVE);
        } break;

        case Gem::Geneva::infoMode::INFOPROCESSING: {
            std::uint32_t iteration = goa->getIteration();

            // Record the current fitness
            std::shared_ptr<gen::GOptimizableEntity> p =
                goa->Interface::GOptimizerIT<oa::GOptimizationAlgorithmBase>::template getBestGlobalIndividual<gen::GOptimizableEntity>();
            (*fitness_graph2_d_oa_) &
                std::tuple<double, double>(static_cast<double>(iteration), p->raw_fitness(0));

            // Update the largest known iteration and the number of recorded iterations
            max_iteration_ = iteration;
            n_iterations_recorded_++;

            // Do the actual logging. The per-group adaption state (sigma, …) is OA-owned
            // scratch and lives on the GIndividualSlot, not on the individual. The live evolving sigma
            // is therefore read from each population slot's scratch via readAdaptionSigmas(). Only the
            // "sigma" property is exposed by the flat genome.
            //
            // NOTE: the "best only" path reads the globally best individual from the OA's archive, which
            // holds pure individual clones detached from any slot, so the live sigma is not available
            // there; it falls back to the configured SEED sigma (a freshly seeded scratch). The "all
            // individuals" path (the data-oriented monitor demo, e.g. example 13) reads the live evolved
            // sigma straight from the population slots.
            // The adaptor settings live on the OA-owned config now (the genome is structure-only), so read
            // it from the algorithm. A non-adapting algorithm returns null and no sigma is logged.
            std::shared_ptr<const oa::GAdaptionConfigBase> cfg_ptr = goa->getAdaptionConfig();

            if(monitor_best_only_) {
                if(property_ == "sigma" && cfg_ptr) {
                    // The best individual is an off-slot archive clone with no live scratch, so report the
                    // configured SEED sigma read from a freshly seeded scratch.
                    gen::GAuxiliaryStore seed_scratch;
                    cfg_ptr->installInto(seed_scratch);
                    for(double sigma : oa::readAdaptionSigmas(seed_scratch, *cfg_ptr, adaptor_name_)) {
                        adaptor_property_store_.emplace_back(static_cast<double>(iteration), sigma);
                    }
                }
            }
            else { // Monitor all individuals
                // Loop over all individuals of the algorithm, reading the live adaption state off each
                // slot's OA-owned scratch.
                for(std::size_t pos = 0; pos < goa->size(); pos++) {
                    const auto &slot = goa->at(pos);

                    if(property_ == "sigma" && cfg_ptr) {
                        for(double sigma : oa::readAdaptionSigmas(slot->scratch(), *cfg_ptr, adaptor_name_)) {
                            adaptor_property_store_.emplace_back(static_cast<double>(iteration), sigma);
                        }
                    }
                }
            }
        } break;

        case Gem::Geneva::infoMode::INFOEND: {
            // Within adaptor_property_store_, find the largest number of adaptions performed
            double max_property = 0.;
            for(const auto &property_entry : adaptor_property_store_) {
                if(std::get<1>(property_entry) > max_property) {
                    max_property = std::get<1>(property_entry);
                }
            }

            // Create the histogram object
            adaptor_property_hist2_d_oa_ = std::make_shared<GHistogram2D>(
                n_iterations_recorded_,
                100,
                0.,
                double(max_iteration_),
                0.,
                max_property
            );

            adaptor_property_hist2_d_oa_->setXAxisLabel("Iteration");
            adaptor_property_hist2_d_oa_->setYAxisLabel(
                std::string("Adaptor-Name: ") + adaptor_name_ + std::string(", Property: ") +
                property_
            );
            adaptor_property_hist2_d_oa_->setDrawingArguments("BOX");

            // Fill the object with data
            for(const auto &property_entry : adaptor_property_store_) {
                (*adaptor_property_hist2_d_oa_) & property_entry;
            }

            // Add the histogram to the plot designer
            gpd_.registerPlotter(adaptor_property_hist2_d_oa_);

            // Add the fitness monitor
            gpd_.registerPlotter(fitness_graph2_d_oa_);

            // Inform the plot designer whether it should print png files
            gpd_.setAddPrintCommand(add_print_command_);

            // Write out the result. Note that we add
            gpd_.writeToFile(file_name_);

            // Remove all plotters (they will survive inside of gpd)
            gpd_.resetPlotters();
            adaptor_property_hist2_d_oa_.reset();
        } break;

        default: {
            throw geneva_exception(
                g_error_streamer(DO_LOG, Gem::Common::timeAndPlace())
                << "In GAdaptorPropertyLoggerT: Received invalid infoMode " << im << '\n'
            );
        }
        };
    }

    /************************************************************************/

    std::string file_name_ =
        "NAdaptions.C"; ///< The name of the file to which solutions should be stored

    std::string adaptor_name_ =
        "GDoubleGaussAdaptor"; ///< The  name of the adaptor for which properties should be logged
    std::string property_ = "sigma"; ///< The name of the property to be logged

    std::tuple<std::uint32_t, std::uint32_t> canvas_dimensions_ =
        std::tuple<std::uint32_t, std::uint32_t>(1200, 1600); ///< The dimensions of the canvas

    Gem::Common::GPlotDesigner gpd_{"Adaptor properties", 1, 2}; ///< A wrapper for the plots

    std::shared_ptr<Gem::Common::GHistogram2D>
        adaptor_property_hist2_d_oa_; ///< Holds the actual histogram
    std::shared_ptr<Gem::Common::GGraph2D>
        fitness_graph2_d_oa_; ///< Lets us monitor the current fitness of the population

    bool monitor_best_only_ =
        false; ///< Indicates whether only the best individuals should be monitored
    bool add_print_command_ =
        false; ///< Asks the GPlotDesigner to add a print command to result files

    std::size_t max_iteration_ = 0; ///< Holds the largest iteration recorded for the algorithm
    std::size_t n_iterations_recorded_ =
        0; ///< Holds the number of iterations that were recorded (not necessarily == max_iteration_

    std::vector<std::tuple<double, double>>
        adaptor_property_store_; ///< Holds all information about the number of adaptions
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

    /***************************************************************************/
    /**
     * Single declaration of this class'es local data members, driving
     * serialize(), load_() and compare_() from one place. All members are
     * handled unconditionally: the eight histogram smart pointers are
     * deep-cloned on load (make_cloneable_member); every other member (including
     * the two gpd_pth* GPlotDesigner values, which load_() assigns plainly) uses
     * make_member. No manual tail is needed.
     *
     * NOTE: deriving load_() from this declaration also fixes a latent bug --
     * the previous hand-written load_() forgot to load n_bins_y_ (it was
     * serialized and compared but never copied on load).
     */
    template <typename Self>
    static auto localMembers_(Self &self) {
        return std::make_tuple(
            Gem::Common::make_member("file_name_pth_", self.file_name_pth_),
            Gem::Common::make_member("canvas_dimensions_pth_", self.canvas_dimensions_pth_),
            Gem::Common::make_member("gpd_pth_", self.gpd_pth_),
            Gem::Common::make_member("file_name_pth2_", self.file_name_pth2_),
            Gem::Common::make_member("canvas_dimensions_pth2_", self.canvas_dimensions_pth2_),
            Gem::Common::make_member("gpd_pth2_", self.gpd_pth2_),
            Gem::Common::make_member("file_name_txt_", self.file_name_txt_),
            Gem::Common::make_cloneable_member("pre_processing_times_hist_", self.pre_processing_times_hist_),
            Gem::Common::make_cloneable_member("processing_times_hist_", self.processing_times_hist_),
            Gem::Common::make_cloneable_member("post_processing_times_hist_", self.post_processing_times_hist_),
            Gem::Common::make_cloneable_member("all_processing_times_hist_", self.all_processing_times_hist_),
            Gem::Common::make_cloneable_member("pre_processing_times_hist2_d_", self.pre_processing_times_hist2_d_),
            Gem::Common::make_cloneable_member("processing_times_hist2_d_", self.processing_times_hist2_d_),
            Gem::Common::make_cloneable_member("post_processing_times_hist2_d_", self.post_processing_times_hist2_d_),
            Gem::Common::make_cloneable_member("all_processing_times_hist2_d_", self.all_processing_times_hist2_d_),
            Gem::Common::make_member("n_bins_x_", self.n_bins_x_),
            Gem::Common::make_member("n_bins_y_", self.n_bins_y_)
        );
    }

    template <typename Archive>
    void serialize(Archive &ar, const unsigned int) {
        using boost::serialization::make_nvp;

        ar &make_nvp(
            "GBasePluggableOM",
            boost::serialization::base_object<oa::GBasePluggableOM>(*this)
        );

        // All members are derived from the single localMembers() declaration.
        Gem::Common::serialize_members(ar, localMembers_(*this));
    }
    ///////////////////////////////////////////////////////////////////////

public:
    /***************************************************************************/

    /** @brief The default constructor */
    GProcessingTimesLogger();
    /**
     * @brief Initialization with file names and histogram bin counts
     * @param file_name_pth The file name for the 1D processing-times histograms (ROOT format)
     * @param file_name_pth2 The file name for the 2D processing-times-versus-iteration histograms (ROOT format)
     * @param file_name_txt The file name for the plain-text processing-times output
     * @param n_bins_x The number of histogram bins in x-direction
     * @param n_bins_y The number of histogram bins in y-direction
     */
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

    /**
     * @brief Sets the file name for the processing times histogram
     * @param file_name The file name for the 1D processing-times histograms
     */
    void setFileName_pth(const std::string &file_name);
    /**
     * @brief Retrieves the current file name for the processing times histogram
     * @return The file name for the 1D processing-times histograms
     */
    std::string getFileName_pth() const;
    /**
     * @brief Sets the file name for the processing times histograms (2D)
     * @param file_name The file name for the 2D processing-times histograms
     */
    void setFileName_pth2(const std::string &file_name);
    /**
     * @brief Retrieves the current file name for the processing times histograms (2D)
     * @return The file name for the 2D processing-times histograms
     */
    std::string getFileName_pth2() const;

    /**
     * @brief Sets the file name for the text output
     * @param file_name The file name for the plain-text processing-times output
     */
    void setFileName_txt(const std::string &file_name);
    /**
     * @brief Retrieves the current file name for the text output
     * @return The file name for the plain-text processing-times output
     */
    std::string getFileName_txt() const;

    /**
     * @brief Allows to set the canvas dimensions for the processing times histograms
     * @param canvas_dimensions A tuple holding the canvas dimensions in x- and y-direction
     */
    void setCanvasDimensions_pth(std::tuple<std::uint32_t, std::uint32_t> canvas_dimensions);
    /**
     * @brief Allows to set the canvas dimensions using separate x and y values for the processing times histograms
     * @param x The canvas dimension in x-direction
     * @param y The canvas dimension in y-direction
     */
    void setCanvasDimensions_pth(std::uint32_t x, std::uint32_t y);

    /**
     * @brief Gives access to the canvas dimensions of the processing times histograms
     * @return A tuple holding the canvas dimensions in x- and y-direction
     */
    std::tuple<std::uint32_t, std::uint32_t> getCanvasDimensions_pth() const;
    /**
     * @brief Allows to set the canvas dimensions for the processing times histograms (2D)
     * @param canvas_dimensions A tuple holding the canvas dimensions in x- and y-direction
     */
    void setCanvasDimensions_pth2(std::tuple<std::uint32_t, std::uint32_t> canvas_dimensions);

    /**
     * @brief Allows to set the canvas dimensions using separate x and y values for the processing times histograms (2D)
     * @param x The canvas dimension in x-direction
     * @param y The canvas dimension in y-direction
     */
    void setCanvasDimensions_pth2(std::uint32_t x, std::uint32_t y);
    /**
     * @brief Gives access to the canvas dimensions of the processing times histograms (2D)
     * @return A tuple holding the canvas dimensions in x- and y-direction
     */
    std::tuple<std::uint32_t, std::uint32_t> getCanvasDimensions_pth2() const;

    /**
     * @brief Sets the number of bins for the processing times histograms in x-direction
     * @param n_bins_x The number of histogram bins in x-direction
     */
    void setNBinsX(std::size_t n_bins_x);
    /**
     * @brief Retrieves the current number of bins for the processing times histograms in x-direction
     * @return The number of histogram bins in x-direction
     */
    std::size_t getNBinsX() const;

    /**
     * @brief Sets the number of bins for the processing times histograms in y-direction
     * @param n_bins_y The number of histogram bins in y-direction
     */
    void setNBinsY(std::size_t n_bins_y);
    /**
     * @brief Retrieves the current number of bins for the processing times histograms in y-direction
     * @return The number of histogram bins in y-direction
     */
    std::size_t getNBinsY() const;

protected:
    /************************************************************************/

    /**
     * @brief Loads the data of another object into this one
     * @param cp A pointer to another object of this type, camouflaged as a GBasePluggableOM, whose data is copied
     */
    void load_(const oa::GBasePluggableOM *cp) override;

    /** @brief Allow access to this classes compare_ function */
    friend void Gem::Common::compare_base_t<GProcessingTimesLogger>(
        GProcessingTimesLogger const &,
        GProcessingTimesLogger const &,
        Gem::Common::GToken &
    );

    /**
     * @brief Searches for compliance with expectations with respect to another object of the same type
     * @param cp A constant reference to another GBasePluggableOM object to compare against
     * @param e The expected outcome of the comparison (e.g. equality or inequality)
     * @param limit The maximum allowed deviation for comparisons of floating point types
     */
    void compare_(
        const oa::GBasePluggableOM &cp,
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
    /**
     * @brief Emits a name for this class / object
     * @return The human-readable class name of this object
     */
    std::string name_() const override;
    /**
     * @brief Creates a deep clone of this object
     * @return A pointer to a freshly allocated deep copy of this object (ownership passes to the caller)
     */
    oa::GBasePluggableOM *clone_() const override;

    /**
     * @brief Allows to emit information in different stages of the information cycle
     *
     * The first (unnamed) argument is the information mode (init, processing or finalization).
     * @param goa A constant pointer to the optimization algorithm whose state is being monitored
     */
    void
    informationFunction_(infoMode, oa::GOptimizationAlgorithmBase const *const goa) override;

    /************************************************************************/

    std::string file_name_pth_ =
        "processingTimingsHist.C"; ///< The name of the file to which timings should be written in ROOT format
    std::tuple<std::uint32_t, std::uint32_t> canvas_dimensions_pth_ =
        std::tuple<std::uint32_t, std::uint32_t>(1600, 1200); ///< The dimensions of the canvas
    Gem::Common::GPlotDesigner gpd_pth_{
        "Timings for the processing steps of individuals",
        2,
        2
    }; ///< A wrapper for the plots

    std::string file_name_pth2_ =
        "processingTimingsVsIteration.C"; ///< The name of the file to which timings should be written in ROOT format
    std::tuple<std::uint32_t, std::uint32_t> canvas_dimensions_pth2_ =
        std::tuple<std::uint32_t, std::uint32_t>(1600, 1200); ///< The dimensions of the canvas
    Gem::Common::GPlotDesigner gpd_pth2_{
        "Timings for the processing steps of individuals vs. iteration",
        2,
        2
    }; ///< A wrapper for the plots

    std::string file_name_txt_ =
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
        pre_processing_times_hist2_d_; ///< The amount of time needed for pre-processing
    std::shared_ptr<Gem::Common::GHistogram2D>
        processing_times_hist2_d_; ///< The amount of time needed for processing
    std::shared_ptr<Gem::Common::GHistogram2D>
        post_processing_times_hist2_d_; ///< The amount of time needed for post-processing
    std::shared_ptr<Gem::Common::GHistogram2D>
        all_processing_times_hist2_d_; ///< The amount of time needed for the entire processing step

    std::size_t n_bins_x_ =
        Gem::Common::DEFAULTNBINSGPD; ///< The number of bins in the histograms in x-direction
    std::size_t n_bins_y_ =
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
