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

#include "common/plotting/GDecorators.hpp"

namespace Gem::Common {

/******************************************************************************/
/**
 * An abstract base class that defines functions for plots. Concrete plotters
 * derive from this class. They can be added to a master canvas, which takes care
 * to plot them into sub-pads.
 */
class GBasePlotter : public GCommonInterfaceT<GBasePlotter> {
    ///////////////////////////////////////////////////////////////////////
    friend class boost::serialization::access;

    /**
     * @brief Single declaration of this class'es local data members
     * @return A tuple of named member references driving serialize(), load_() and compare_()
     */
    template <typename Self>
    static auto localMembers_(Self &self) {
        return std::make_tuple(
            Gem::Common::make_member("drawing_arguments_", self.drawing_arguments_),
            Gem::Common::make_member("x_axis_label_", self.x_axis_label_),
            Gem::Common::make_member("y_axis_label_", self.y_axis_label_),
            Gem::Common::make_member("z_axis_label_", self.z_axis_label_),
            Gem::Common::make_member("plot_label_", self.plot_label_),
            Gem::Common::make_member("ds_marker_", self.ds_marker_),
            Gem::Common::make_cloneable_container_member("secondary_plotter_", self.secondary_plotter_),
            Gem::Common::make_member("id_", self.id_)
        );
    }
    auto localMembers() { return localMembers_(*this); }       // NOLINT -- intentionally hides the base localMembers()
    auto localMembers() const { return localMembers_(*this); } // NOLINT -- intentionally hides the base localMembers()

    /**
     * @brief Serializes this plotter's state to or from a Boost archive
     * @tparam Archive The Boost.Serialization archive type
     * @param ar The archive being read from or written to
     * @param unsigned int The (unused) class version supplied by Boost.Serialization
     */
    template <typename Archive>
    void serialize(Archive &ar, const unsigned int) {
        using boost::serialization::make_nvp;
        // The member list is derived from the single localMembers() declaration
        // so serialize()/load_()/compare_() stay in sync (no silently-dropped member).
        Gem::Common::serialize_members(ar, this->localMembers());
    }
    ///////////////////////////////////////////////////////////////////////

public:
    /**
     * @brief Copy constructor
     * @param cp The plotter to copy from
     */
    GBasePlotter(GBasePlotter const &cp);
    /**
     * @brief Assignment operator
     * @param cp The plotter to copy from
     * @return A reference to this object
     */
    GBasePlotter &operator=(GBasePlotter const &cp);

    /*********************************************************************/
    // Defaulted constructors, destructor and assignment operators

    /** @brief The default constructor */
    GBasePlotter() = default;
    /** @brief The move constructor */
    GBasePlotter(GBasePlotter &&) = default;
    /** @brief The (defaulted) destructor */
    ~GBasePlotter() override = default;

    /** @brief The move-assignment operator @return A reference to this object */
    GBasePlotter &operator=(GBasePlotter &&) = default;

    /*********************************************************************/

    /**
     * @brief Allows to set the drawing arguments for this plot
     * @param drawing_arguments The drawing arguments (passed to the plotting backend) for this plot
     */
    void setDrawingArguments(std::string drawing_arguments);

    /**
     * @brief Sets the label for the x-axis
     * @param x_axis_label The text to use as the x-axis label
     */
    void setXAxisLabel(std::string x_axis_label);
    /**
     * @brief Retrieve the x-axis label
     * @return The current x-axis label
     */
    std::string xAxisLabel() const;
    /**
     * @brief Sets the label for the y-axis
     * @param y_axis_label The text to use as the y-axis label
     */
    void setYAxisLabel(std::string y_axis_label);
    /**
     * @brief Retrieve the y-axis label
     * @return The current y-axis label
     */
    std::string yAxisLabel() const;
    /**
     * @brief Sets the label for the z-axis
     * @param z_axis_label The text to use as the z-axis label
     */
    void setZAxisLabel(std::string z_axis_label);
    /**
     * @brief Retrieve the z-axis label
     * @return The current z-axis label
     */
    std::string zAxisLabel() const;

    /**
     * @brief Allows to assign a label to the entire plot
     * @param plot_label The label for the entire plot
     */
    void setPlotLabel(std::string plot_label);
    /**
     * @brief Allows to retrieve the plot label
     * @return The current plot label
     */
    std::string plotLabel() const;

    /**
     * @brief Allows to assign a marker to data structures
     * @param ds_marker A marker that makes the origin of data structures clear in the output file
     */
    void setDataStructureMarker(std::string ds_marker);
    /**
     * @brief Allows to retrieve the data structure marker
     * @return The current data-structure marker
     */
    std::string dsMarker() const;

    /**
     * @brief Allows to add secondary plots to be added to the same sub-canvas
     * @param secondary_plotter A plotter whose data should be emitted into the same canvas as this one
     */
    void registerSecondaryPlotter(std::shared_ptr<GBasePlotter> secondary_plotter);

    /**
     * @brief Allows to retrieve the id of this object
     * @return The id currently assigned to this plotter
     */
    std::size_t id() const;
    /**
     * @brief Sets the id of the object
     * @param id The id to be assigned to this plotter
     */
    void setId(const std::size_t &id);

    /**
     * @brief Retrieves a unique name for this plotter
     * @return The plotter type's unique name (implemented in derived classes)
     */
    virtual std::string getPlotterName() const = 0;

    /***************************************************************************/

    /**
     * @brief Retrieve header settings for this plot (and any sub-plots)
     * @param indent The indentation string prepended to each emitted line
     * @return The combined header section for this plotter and its secondary plotters
     */
    std::string headerData(const std::string &indent) const;

    /**
     * @brief Retrieves body / data settings for this plot (and any sub-plots)
     * @param indent The indentation string prepended to each emitted line
     * @return The combined body / data section for this plotter and its secondary plotters
     */
    std::string bodyData(const std::string &indent) const;

    /**
     * @brief Retrieves footer / drawing settings for this plot (and any sub-plots)
     * @param indent The indentation string prepended to each emitted line
     * @return The combined footer / drawing section for this plotter and its secondary plotters
     */
    std::string footerData(const std::string &indent) const;

protected:
    /***************************************************************************/
    /**
     * @brief Loads the data of another object
     * @param cp A pointer to the GBasePlotter whose data should be loaded into this object
     */
    void load_(const GBasePlotter *cp) override;

    /** @brief Allow access to this classes compare_ function */
    friend void compare_base_t<GBasePlotter>(GBasePlotter const &, GBasePlotter const &, GToken &);

    /**
     * @brief Searches for compliance with expectations with respect to another object of the same type
     * @param cp The other GBasePlotter to compare against
     * @param e The expectation for this comparison, e.g. equality
     * @param limit The limit for allowed deviations of floating-point types
     */
    void compare_(
        const GBasePlotter &cp,
        const expectation &e,
        const double &limit
    ) const override;

    /***************************************************************************/
    // Functions to be specified in derived classes

    /**
     * @brief Retrieve specific header settings for this plot
     * @param is_secondary true if this plotter is a secondary plotter sharing a parent's canvas
     * @param parent_id The id of the parent plotter (only meaningful when is_secondary is true)
     * @param indent The indentation string prepended to each emitted line
     * @return The plotter-specific header section
     */
    virtual std::string headerData_(bool is_secondary, std::size_t parent_id, const std::string &indent) const = 0;

    /**
     * @brief Retrieves the actual data sets
     * @param is_secondary true if this plotter is a secondary plotter sharing a parent's canvas
     * @param parent_id The id of the parent plotter (only meaningful when is_secondary is true)
     * @param indent The indentation string prepended to each emitted line
     * @return The plotter-specific data / body section
     */
    virtual std::string bodyData_(bool is_secondary, std::size_t parent_id, const std::string &indent) const = 0;

    /**
     * @brief retrieves specific draw commands for this plot
     * @param is_secondary true if this plotter is a secondary plotter sharing a parent's canvas
     * @param parent_id The id of the parent plotter (only meaningful when is_secondary is true)
     * @param indent The indentation string prepended to each emitted line
     * @return The plotter-specific footer / draw section
     */
    virtual std::string footerData_(bool is_secondary, std::size_t parent_id, const std::string &indent) const = 0;

    /**
     * @brief Retrieve the current drawing arguments
     * @param is_secondary true if this plotter is a secondary plotter sharing a parent's canvas
     * @return The drawing arguments to be passed to the plotting backend
     */
    virtual std::string drawingArguments(bool is_secondary) const = 0;

    /**
     * @brief Check that a given plotter is compatible with us
     * @param other The other plotter whose compatibility with this one is checked
     * @return true if the other plotter is compatible (by default, has the same plotter name)
     */
    virtual bool isCompatible(std::shared_ptr<GBasePlotter> other) const;

    /**
     * @brief calculate a suffix from id and parent ids
     * @param is_secondary true if a parent id should be folded into the suffix
     * @param parent_id The id of the parent plotter (used only when is_secondary is true)
     * @return A suffix string built from this plotter's id (and the parent id for secondary plotters)
     */
    std::string suffix(bool is_secondary, std::size_t parent_id) const;

    /** @brief Applies modifications to this object. This is needed for testing purposes */
    bool modify_GUnitTests_() override {
        return false;
    }
    /** @brief Performs self tests that are expected to succeed. This is needed for testing purposes */
    void specificTestsNoFailureExpected_GUnitTests_() override { /* nothing */ };
    /** @brief Performs self tests that are expected to fail. This is needed for testing purposes */
    void specificTestsFailuresExpected_GUnitTests_() override { /* nothing */ };

    /***************************************************************************/

    std::string drawing_arguments_ = std::string(""); ///< Holds the drawing arguments for this plot

    std::string x_axis_label_ = std::string("x"); ///< A label for the x-axis
    std::string y_axis_label_ = std::string("y"); ///< A label for the y-axis
    std::string z_axis_label_ = std::string("z"); ///< A label for the z-axis (if available)

    std::string plot_label_ = std::string(""); ///< A label to be assigned to the entire plot
    std::string ds_marker_ = std::string(
        ""
    ); ///< A marker to make the origin of data structures clear in the output file

    std::vector<line> lines_; ///< Lines to be drawn into the drawing area

private:
    /***************************************************************************/
    /**
     * @brief Returns the name of this class
     * @return The string "GBasePlotter"
     */
    std::string name_() const override;
    /**
     * @brief Creates a deep clone of this object
     * @return A newly allocated deep copy of this plotter (implemented in derived classes)
     */
    GBasePlotter *clone_() const override = 0;

    /***************************************************************************/
    /** @brief A list of plotters that should emit their data into the same canvas */
    std::vector<std::shared_ptr<GBasePlotter>> secondary_plotter_;

    std::size_t id_ = 0; ///< The id of this object
};


} /* namespace Gem::Common */
