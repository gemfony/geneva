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

#include <cstdint>
#include <span>
#include <variant>
#include <vector>

#include "dietrich/plotting/GDecorators.hpp"
#include "dietrich/plotting/GPlotSpec.hpp"

namespace Gem::Dietrich {

// The plotting library builds on common's facilities (logging, serialization helpers,
// exception types, make_member, EmitStream, ...); make them visible here without
// per-name qualification. This affects lookup only within Gem::Dietrich.
using namespace Gem::Common;

/******************************************************************************/
/**
 * A read-only, type-tagged view of one exportable plotter column: either a float64
 * (double) or an int32 value vector. The plot backends consume a plotter's data
 * through a list of these (see GBasePlotter::dataColumns()), so an integer-valued
 * plotter (the GHistogram1I integer histogram) exports its samples with their TRUE
 * int32 dtype -- a numpy .npz then carries a real int32 array, not a widened float64.
 * The pointer aliases the plotter's column and is valid for its lifetime.
 */
using GPlotColumn = std::variant<const std::vector<double> *, const std::vector<std::int32_t> *>;

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

    /**
     * @brief Serializes this plotter's state to or from a Boost archive
     * @tparam Archive The Boost.Serialization archive type
     * @param ar The archive being read from or written to
     * @param version The (unused) class version supplied by Boost.Serialization
     */
    template <typename Archive>
    void serialize(Archive &ar, [[maybe_unused]] const unsigned int version) {
        using boost::serialization::make_nvp;
        // The member list is derived from the single localMembers() declaration
        // so serialize()/load_()/compare_() stay in sync (no silently-dropped member).
        Gem::Common::serialize_members(ar, localMembers_(*this));
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
    [[nodiscard]] std::string xAxisLabel() const;
    /**
     * @brief Sets the label for the y-axis
     * @param y_axis_label The text to use as the y-axis label
     */
    void setYAxisLabel(std::string y_axis_label);
    /**
     * @brief Retrieve the y-axis label
     * @return The current y-axis label
     */
    [[nodiscard]] std::string yAxisLabel() const;
    /**
     * @brief Sets the label for the z-axis
     * @param z_axis_label The text to use as the z-axis label
     */
    void setZAxisLabel(std::string z_axis_label);
    /**
     * @brief Retrieve the z-axis label
     * @return The current z-axis label
     */
    [[nodiscard]] std::string zAxisLabel() const;

    /**
     * @brief Allows to assign a label to the entire plot
     * @param plot_label The label for the entire plot
     */
    void setPlotLabel(std::string p_l);
    /**
     * @brief Allows to retrieve the plot label
     * @return The current plot label
     */
    [[nodiscard]] std::string plotLabel() const;

    /**
     * @brief Allows to assign a marker to data structures
     * @param ds_marker A marker that makes the origin of data structures clear in the output file
     */
    void setDataStructureMarker(std::string ds_marker);
    /**
     * @brief Allows to retrieve the data structure marker
     * @return The current data-structure marker
     */
    [[nodiscard]] std::string dsMarker() const;

    /**
     * @brief Allows to add secondary plots to be added to the same sub-canvas
     * @param secondary_plotter A plotter whose data should be emitted into the same canvas as this one
     */
    void registerSecondaryPlotter(std::shared_ptr<GBasePlotter> sp);

    /**
     * @brief Read-only access to the secondary plotters sharing this plotter's pad
     * @return A const reference to the list of registered secondary plotters
     */
    [[nodiscard]] const std::vector<std::shared_ptr<GBasePlotter>> &secondaryPlotters() const;

    /**
     * @brief Allows to retrieve the id of this object
     * @return The id currently assigned to this plotter
     */
    [[nodiscard]] std::size_t id() const;
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

    /**
     * @brief Reports this plotter's choice (kind, role, labels, columns, bins) as a
     * first-class GPlotSpec value.
     *
     * This is a pure const reporter derived from the plotter's existing state; it
     * adds no stored member and does not affect serialize()/load_()/compare_(). The
     * base fills the common fields (name, labels, drawing args) and a default kind;
     * each concrete plotter overrides it to set its kind, its column names and (for
     * histograms) the bin counts.
     *
     * @return A GPlotSpec describing this plotter
     */
    [[nodiscard]] virtual GPlotSpec plotSpec() const;

    /**
     * @brief Reports this plotter's stored columnar data as a (storage-order) list of
     * read-only double columns, decoupled from the concrete plotter type.
     *
     * Together with plotSpec() this lets the render / data backends consume a plotter
     * generically -- they switch on plotSpec().kind and read the columns through this
     * accessor, instead of dynamic_cast'ing back to each concrete plotter. The returned
     * columns alias this plotter's data and stay valid for its lifetime; the order
     * matches plotSpec().columns. Each column is type-tagged (float64 or int32), so an
     * integer plotter exports its samples with their true dtype. The base returns an
     * empty list (a plotter that holds no exportable sample columns -- a function
     * plotter -- exports nothing this way).
     *
     * @return Type-tagged views of this plotter's per-axis value vectors, in storage order
     */
    [[nodiscard]] virtual std::vector<GPlotColumn> dataColumns() const;

    /**
     * @brief Appends one data row (one value per axis, in storage/column order) to this
     * plotter, generically -- the inverse of dataColumns() for feeding data without
     * knowing the concrete plotter type. This is the data path used by GDataLog to fill
     * a plotter built from a GPlotSpec.
     *
     * The base throws (a plotter that holds no double sample columns -- a function
     * plotter -- cannot accept rows this way); the columnar collectors override it. The
     * row size must equal the plotter's column count.
     *
     * @param row One value per axis, in column order (size must match the column count)
     */
    virtual void appendRow(std::span<const double> row);

    /**
     * @brief Sorts this plotter's data rows by their first column, generically -- the
     * data path used by GDataLog to reproduce a monitor's GGraph2D::sortX() without
     * knowing the concrete plotter type. The base is a no-op (a plotter with no sortable
     * columns -- a function plotter -- has nothing to sort); the columnar collectors
     * override it.
     */
    virtual void sortByFirstColumn();

    /***************************************************************************/

    /**
     * @brief Retrieve header settings for this plot (and any sub-plots)
     * @param indent The indentation string prepended to each emitted line
     * @return The combined header section for this plotter and its secondary plotters
     */
    [[nodiscard]] std::string headerData(const std::string &indent) const;

    /**
     * @brief Retrieves body / data settings for this plot (and any sub-plots)
     * @param indent The indentation string prepended to each emitted line
     * @return The combined body / data section for this plotter and its secondary plotters
     */
    [[nodiscard]] std::string bodyData(const std::string &indent) const;

    /**
     * @brief Retrieves footer / drawing settings for this plot (and any sub-plots)
     * @param indent The indentation string prepended to each emitted line
     * @return The combined footer / drawing section for this plotter and its secondary plotters
     */
    [[nodiscard]] std::string footerData(const std::string &indent) const;

protected:
    /***************************************************************************/
    /**
     * @brief Loads the data of another object
     * @param cp A pointer to the GBasePlotter whose data should be loaded into this object
     */
    void load_(const GBasePlotter *cp) override;

    /** @brief Allow access to this classes compare_ function */
    friend void Gem::Common::compare_base_t<GBasePlotter>(GBasePlotter const &, GBasePlotter const &, GToken &);

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
     * @param own_id This plotter's own emit index, threaded in by the caller (replaces the former mutated id_)
     * @param indent The indentation string prepended to each emitted line
     * @return The plotter-specific header section
     */
    virtual std::string headerData_(bool is_secondary, std::size_t parent_id, std::size_t own_id, const std::string &indent) const = 0;

    /**
     * @brief Retrieves the actual data sets
     * @param is_secondary true if this plotter is a secondary plotter sharing a parent's canvas
     * @param parent_id The id of the parent plotter (only meaningful when is_secondary is true)
     * @param own_id This plotter's own emit index, threaded in by the caller (replaces the former mutated id_)
     * @param indent The indentation string prepended to each emitted line
     * @return The plotter-specific data / body section
     */
    virtual std::string bodyData_(bool is_secondary, std::size_t parent_id, std::size_t own_id, const std::string &indent) const = 0;

    /**
     * @brief retrieves specific draw commands for this plot
     * @param is_secondary true if this plotter is a secondary plotter sharing a parent's canvas
     * @param parent_id The id of the parent plotter (only meaningful when is_secondary is true)
     * @param own_id This plotter's own emit index, threaded in by the caller (replaces the former mutated id_)
     * @param indent The indentation string prepended to each emitted line
     * @return The plotter-specific footer / draw section
     */
    virtual std::string footerData_(bool is_secondary, std::size_t parent_id, std::size_t own_id, const std::string &indent) const = 0;

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
     * @param own_id This plotter's own emit index, threaded in by the caller (replaces the former mutated id_)
     * @return A suffix string built from this plotter's own id (and the parent id for secondary plotters)
     */
    std::string suffix(bool is_secondary, std::size_t p_id, std::size_t own_id) const;

    /** @brief Applies modifications to this object. This is needed for testing purposes.
     *  Mutates this base class'es serialized members (the labels, drawing arguments, data-structure
     *  marker and id); derived plotters call it and then mutate their own state. As a test-only
     *  hook it is never invoked on the rendered objects, so it does not affect emitted output. */
    bool modify_GUnitTests_() override {
        drawing_arguments_ += "_m";
        x_axis_label_ += "_m";
        y_axis_label_ += "_m";
        z_axis_label_ += "_m";
        plot_label_ += "_m";
        ds_marker_ += "_m";
        ++id_;
        return true;
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


} /* namespace Gem::Dietrich */
