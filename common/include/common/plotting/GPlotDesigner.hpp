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

#include <memory>

#include "common/plotting/GDataLog.hpp"
#include "common/plotting/GPlotEmitter.hpp"
#include "common/plotting/GPlots.hpp"

namespace Gem::Common {

/******************************************************************************/
/**
 * A class that outputs a ROOT input file (compare http://root.cern.ch), based
 * on the data providers stored in it.
 */
class GPlotDesigner : public GCommonInterfaceT<GPlotDesigner> {
    ///////////////////////////////////////////////////////////////////////
    friend class boost::serialization::access;
    // The pluggable backend emitters read the designer's plotters, canvas
    // dimensions / divisions and label to compose their backend document.
    friend class GRootEmitter;
    friend class GnuplotEmitter;
    friend class MatplotlibEmitter;
    friend class GDataEmitter;

    /**
     * @brief Single declaration of this class'es local data members
     * @return A tuple of named member references driving serialize(), load_() and compare_()
     */
    template <typename Self>
    static auto localMembers_(Self &self) {
        return std::make_tuple(
            Gem::Common::make_cloneable_container_member("plotters_cnt_", self.plotters_cnt_),
            Gem::Common::make_member("c_x_div_", self.c_x_div_),
            Gem::Common::make_member("c_y_div_", self.c_y_div_),
            Gem::Common::make_member("c_x_dim_", self.c_x_dim_),
            Gem::Common::make_member("c_y_dim_", self.c_y_dim_),
            Gem::Common::make_member("canvas_label_", self.canvas_label_),
            Gem::Common::make_member("add_print_command_", self.add_print_command_),
            Gem::Common::make_member("n_indention_spaces_", self.n_indention_spaces_)
        );
    }

    /**
     * @brief Serializes this designer's state to or from a Boost archive
     * @tparam Archive The Boost.Serialization archive type
     * @param ar The archive being read from or written to
     * @param version The (unused) class version supplied by Boost.Serialization
     */
    template <typename Archive>
    void serialize(Archive &ar, [[maybe_unused]] const unsigned int version) {
        using boost::serialization::make_nvp;
        // The member list is derived from the single localMembers() declaration
        // so serialize()/load_()/compare_() stay in sync. plotters_cnt_ (the plot
        // list) is included so a checkpointed monitor's accumulated plots survive
        // a resume -- it was previously dropped from the wire while load_()/compare_()
        // carried it.
        Gem::Common::serialize_members(ar, localMembers_(*this));
    }
    ///////////////////////////////////////////////////////////////////////

public:
    /**
     * @brief The standard constructor
     * @param canvas_label The label of the canvas
     * @param c_x_div The number of plot divisions (pads) in the x-direction
     * @param c_y_div The number of plot divisions (pads) in the y-direction
     */
    GPlotDesigner(const std::string &canvas_label, const std::size_t &c_x_div, const std::size_t &c_y_div);

    /**
     * @brief Copy constructor
     * @param cp The designer to copy from
     */
    GPlotDesigner(GPlotDesigner const &cp);
    /**
     * @brief Assignment operator
     * @param cp The designer to copy from
     * @return A reference to this object
     */
    GPlotDesigner &operator=(GPlotDesigner const &cp);

    /*********************************************************************/
    // Defaulted constructors, destructor and assignment operators

    // Defaulted default constructor in private section

    /** @brief The move constructor */
    GPlotDesigner(GPlotDesigner &&) = default;
    /** @brief The (defaulted) destructor */
    ~GPlotDesigner() override = default;

    /** @brief The move-assignment operator @return A reference to this object */
    GPlotDesigner &operator=(GPlotDesigner &&) = default;

    /*********************************************************************/

    /**
     * @brief Emits the overall plot as a ROOT input script
     * @param plot_name The name used in the emitted script (and a warning context); defaults to "empty"
     * @return The complete ROOT input script as a string
     */
    [[nodiscard]] std::string
    plot(const std::filesystem::path & plot_name = std::filesystem::path("empty")) const;
    /**
     * @brief Writes the plot to a file
     * @param file_name The path of the file the emitted plot script is written to
     */
    void writeToFile(const std::filesystem::path & file_name);

    /**
     * @brief Selects the backend the designer emits through
     *
     * Installs the standard emitter for the requested backend (ROOT, the default,
     * or GNUPLOT). plot() then delegates to the selected emitter.
     *
     * @param backend The plotting backend to use
     */
    void setPlotBackend(plotBackend backend);

    /**
     * @brief Installs a custom plot emitter
     *
     * Overrides the backend selected via setPlotBackend() with a caller-supplied
     * emitter. plot() delegates to it.
     *
     * @param emitter The emitter to install (must not be empty)
     */
    void setEmitter(std::shared_ptr<IPlotEmitter> emitter);

    /**
     * @brief Selects the DATA backend in the requested export format
     *
     * A convenience for `setEmitter(std::make_shared<GDataEmitter>(format))`: installs a
     * GDataEmitter that exports the raw series data (CSV text or a numpy .npz archive)
     * rather than a rendered plot. plot() then delegates to it.
     *
     * @param format The on-disk format to export (CSV, the default, or NPZ)
     */
    void setDataFormat(dataFormat format);

    /**
     * @brief Allows to add a new plotter object
     * @param plotter_ptr The plotter to register with this designer
     */
    void registerPlotter(std::shared_ptr<GBasePlotter> plotter_ptr);

    /**
     * @brief Set the dimensions of the output canvas
     * @param c_x_dim The canvas width in pixels
     * @param c_y_dim The canvas height in pixels
     */
    void setCanvasDimensions(const std::uint32_t & c_x_dim, const std::uint32_t & c_y_dim);
    /**
     * @brief Set the dimensions of the output canvas
     * @param c_dim A tuple holding the canvas width and height (in pixels)
     */
    void setCanvasDimensions(const std::tuple<std::uint32_t, std::uint32_t> & c_dim);
    /**
     * @brief Allows to retrieve the canvas dimensions
     * @return A tuple holding the canvas width and height (in pixels)
     */
    std::tuple<std::uint32_t, std::uint32_t> getCanvasDimensions() const;

    /**
     * @brief Allows to set the canvas label
     * @param canvas_label The label to be assigned to the entire canvas
     */
    void setCanvasLabel(const std::string & canvas_label);
    /**
     * @brief Allows to retrieve the canvas label
     * @return The current canvas label
     */
    std::string getCanvasLabel() const;

    /**
     * @brief Allows to add a "Print" command to the end of the script so that picture files are created
     * @param add_print_command Whether a print command (for png creation) should be appended to the script
     */
    void setAddPrintCommand(bool add_print_command);
    /**
     * @brief Allows to retrieve the current value of the add_print_command_ variable
     * @return Whether a print command is appended to the emitted script
     */
    bool getAddPrintCommand() const;

    /** @brief Resets the plotters */
    void resetPlotters();

    /**
     * @brief Allows to set the number of spaces used for indention
     * @param n_indention_spaces The number of spaces used for indenting emitted lines
     */
    void setNIndentionSpaces(const std::size_t & n_indention_spaces);

    /**
     * @brief Allows to retrieve the number spaces used for indention
     * @return The number of spaces currently used for indention
     */
    std::size_t getNIndentionSpaces() const;

    /**
     * @brief Returns the current number of indention spaces as a string
     * @return A string consisting of the configured number of indention spaces
     */
    std::string indent() const;

protected:
    /**
     * @brief A header for static data in a ROOT file
     * @param indent The indentation string prepended to each emitted line
     * @return The static-data header section of the ROOT script
     */
    std::string staticHeader(const std::string & indent) const;

    /**
     * @brief Loads the data of another object
     * @param cp A pointer to the GPlotDesigner whose data should be loaded into this object
     */
    void load_(const GPlotDesigner *cp) override;

    /** @brief Allow access to this classes compare_ function */
    friend void
    compare_base_t<GPlotDesigner>(GPlotDesigner const &, GPlotDesigner const &, GToken &);

    /**
     * @brief Searches for compliance with expectations with respect to another object of the same type
     * @param cp The other GPlotDesigner to compare against
     * @param e The expectation for this comparison, e.g. equality
     * @param limit The limit for allowed deviations of floating-point types
     */
    void compare_(
        const GPlotDesigner &cp,
        const expectation &e,
        const double &limit
    ) const override;

    /** @brief Applies modifications to this object. This is needed for testing purposes */
    bool modify_GUnitTests_() override {
        return false;
    }
    /** @brief Performs self tests that are expected to succeed. This is needed for testing purposes */
    void specificTestsNoFailureExpected_GUnitTests_() override { /* nothing */ };
    /** @brief Performs self tests that are expected to fail. This is needed for testing purposes */
    void specificTestsFailuresExpected_GUnitTests_() override { /* nothing */ };

private:
    /** @brief The default constructor -- only needed for (de-)serialization */
    GPlotDesigner() = default;

    /**
     * @brief Returns the name of this class
     * @return The string "GPlotDesigner"
     */
    std::string name_() const override;
    /**
     * @brief Creates a deep clone of this object
     * @return A newly allocated deep copy of this designer
     */
    GPlotDesigner *clone_() const override;

    std::vector<std::shared_ptr<GBasePlotter>>
        plotters_cnt_; ///< A list of plots to be added to the diagram

    std::size_t c_x_div_ = 1, c_y_div_ = 1; ///< The number of divisions in x- and y-direction
    std::uint32_t c_x_dim_ = DEFCXDIM,
                  c_y_dim_ = DEFCYDIM; ///< Holds the number of pixels of the canvas

    std::string canvas_label_ =
        std::string("empty"); ///< A label to be assigned to the entire canvas

    bool add_print_command_ =
        false; ///< Indicates whether a print command for the creation of a png file should be added

    std::size_t n_indention_spaces_ = (DEFNINDENTIONSPACES);

    // --- Backend selection (transient; not part of the serialized / compared state) ---

    /** @brief The emitter plot() delegates to (lazily defaulted to a GRootEmitter) */
    std::shared_ptr<IPlotEmitter> emitter_;
    /** @brief The plot name threaded from plot() to the emitter for the print command */
    mutable std::filesystem::path pending_plot_name_ = std::filesystem::path("empty");
};

/******************************************************************************/

} /* namespace Gem::Common */

/******************************************************************************/
// Declare abstract or export class names for Boost.Serialization
namespace boost::serialization {

/** @brief Marks the 2D GDecorator as abstract for Boost.Serialization. @tparam coordinate_type The decorator's coordinate type */
template <typename coordinate_type>
struct is_abstract<Gem::Common::GDecorator<Gem::Common::dimensions::Dim2, coordinate_type>>
  : public std::true_type { /* nothing */
};
/** @brief Marks the const 2D GDecorator as abstract for Boost.Serialization. @tparam coordinate_type The decorator's coordinate type */
template <typename coordinate_type>
struct is_abstract<const Gem::Common::GDecorator<Gem::Common::dimensions::Dim2, coordinate_type>>
  : public std::true_type { /* nothing */
};

/** @brief Marks the 3D GDecorator as abstract for Boost.Serialization. @tparam coordinate_type The decorator's coordinate type */
template <typename coordinate_type>
struct is_abstract<Gem::Common::GDecorator<Gem::Common::dimensions::Dim3, coordinate_type>>
  : public std::true_type { /* nothing */
};
/** @brief Marks the const 3D GDecorator as abstract for Boost.Serialization. @tparam coordinate_type The decorator's coordinate type */
template <typename coordinate_type>
struct is_abstract<const Gem::Common::GDecorator<Gem::Common::dimensions::Dim3, coordinate_type>>
  : public std::true_type { /* nothing */
};

/** @brief Marks the 2D GDecoratorContainer as abstract for Boost.Serialization. @tparam coordinate_type The container's coordinate type */
template <typename coordinate_type>
struct is_abstract<Gem::Common::GDecoratorContainer<Gem::Common::dimensions::Dim2, coordinate_type>>
  : public std::true_type { /* nothing */
};
/** @brief Marks the const 2D GDecoratorContainer as abstract for Boost.Serialization. @tparam coordinate_type The container's coordinate type */
template <typename coordinate_type>
struct is_abstract<
    const Gem::Common::GDecoratorContainer<Gem::Common::dimensions::Dim2, coordinate_type>>
  : public std::true_type { /* nothing */
};

/** @brief Marks the 3D GDecoratorContainer as abstract for Boost.Serialization. @tparam coordinate_type The container's coordinate type */
template <typename coordinate_type>
struct is_abstract<Gem::Common::GDecoratorContainer<Gem::Common::dimensions::Dim3, coordinate_type>>
  : public std::true_type { /* nothing */
};
/** @brief Marks the const 3D GDecoratorContainer as abstract for Boost.Serialization. @tparam coordinate_type The container's coordinate type */
template <typename coordinate_type>
struct is_abstract<
    const Gem::Common::GDecoratorContainer<Gem::Common::dimensions::Dim3, coordinate_type>>
  : public std::true_type { /* nothing */
};

/** @brief Marks GDataCollector1T as abstract for Boost.Serialization. @tparam x_type The x-coordinate data type */
template <typename x_type>
struct is_abstract<Gem::Common::GDataCollector1T<x_type>> : public std::true_type { /* nothing */
};
/** @brief Marks const GDataCollector1T as abstract for Boost.Serialization. @tparam x_type The x-coordinate data type */
template <typename x_type>
struct is_abstract<const Gem::Common::GDataCollector1T<x_type>>
  : public std::true_type { /* nothing */
};

/** @brief Marks GDataCollector2T as abstract for Boost.Serialization. @tparam x_type The x data type @tparam y_type The y data type */
template <typename x_type, typename y_type>
struct is_abstract<Gem::Common::GDataCollector2T<x_type, y_type>>
  : public std::true_type { /* nothing */
};
/** @brief Marks const GDataCollector2T as abstract for Boost.Serialization. @tparam x_type The x data type @tparam y_type The y data type */
template <typename x_type, typename y_type>
struct is_abstract<const Gem::Common::GDataCollector2T<x_type, y_type>>
  : public std::true_type { /* nothing */
};

/** @brief Marks GDataCollector2ET as abstract for Boost.Serialization. @tparam x_type The x data type @tparam y_type The y data type */
template <typename x_type, typename y_type>
struct is_abstract<Gem::Common::GDataCollector2ET<x_type, y_type>>
  : public std::true_type { /* nothing */
};
/** @brief Marks const GDataCollector2ET as abstract for Boost.Serialization. @tparam x_type The x data type @tparam y_type The y data type */
template <typename x_type, typename y_type>
struct is_abstract<const Gem::Common::GDataCollector2ET<x_type, y_type>>
  : public std::true_type { /* nothing */
};

/** @brief Marks GDataCollector3T as abstract for Boost.Serialization. @tparam x_type The x data type @tparam y_type The y data type @tparam z_type The z data type */
template <typename x_type, typename y_type, typename z_type>
struct is_abstract<Gem::Common::GDataCollector3T<x_type, y_type, z_type>>
  : public std::true_type { /* nothing */
};
/** @brief Marks const GDataCollector3T as abstract for Boost.Serialization. @tparam x_type The x data type @tparam y_type The y data type @tparam z_type The z data type */
template <typename x_type, typename y_type, typename z_type>
struct is_abstract<const Gem::Common::GDataCollector3T<x_type, y_type, z_type>>
  : public std::true_type { /* nothing */
};

/** @brief Marks GDataCollector4T as abstract for Boost.Serialization. @tparam x_type The x data type @tparam y_type The y data type @tparam z_type The z data type @tparam w_type The w data type */
template <typename x_type, typename y_type, typename z_type, typename w_type>
struct is_abstract<Gem::Common::GDataCollector4T<x_type, y_type, z_type, w_type>>
  : public std::true_type { /* nothing */
};
/** @brief Marks const GDataCollector4T as abstract for Boost.Serialization. @tparam x_type The x data type @tparam y_type The y data type @tparam z_type The z data type @tparam w_type The w data type */
template <typename x_type, typename y_type, typename z_type, typename w_type>
struct is_abstract<const Gem::Common::GDataCollector4T<x_type, y_type, z_type, w_type>>
  : public std::true_type { /* nothing */
};

} /* namespace boost::serialization */

BOOST_SERIALIZATION_ASSUME_ABSTRACT(GBasePlotter)                          // NOLINT
BOOST_CLASS_EXPORT_KEY(Gem::Common::GMarker<short>)                        // NOLINT
BOOST_CLASS_EXPORT_KEY(Gem::Common::GMarker<std::int32_t>)                 // NOLINT
BOOST_CLASS_EXPORT_KEY(Gem::Common::GMarker<std::uint32_t>)                // NOLINT
BOOST_CLASS_EXPORT_KEY(Gem::Common::GMarker<float>)                        // NOLINT
BOOST_CLASS_EXPORT_KEY(Gem::Common::GMarker<double>)                       // NOLINT
BOOST_CLASS_EXPORT_KEY(Gem::Common::GDecoratorContainer_2D<short>)         // NOLINT
BOOST_CLASS_EXPORT_KEY(Gem::Common::GDecoratorContainer_2D<std::int32_t>)  // NOLINT
BOOST_CLASS_EXPORT_KEY(Gem::Common::GDecoratorContainer_2D<std::uint32_t>) // NOLINT
BOOST_CLASS_EXPORT_KEY(Gem::Common::GDecoratorContainer_2D<float>)         // NOLINT
BOOST_CLASS_EXPORT_KEY(Gem::Common::GDecoratorContainer_2D<double>)        // NOLINT
BOOST_CLASS_EXPORT_KEY(Gem::Common::GDecoratorContainer_3D<short>)         // NOLINT
BOOST_CLASS_EXPORT_KEY(Gem::Common::GDecoratorContainer_3D<std::int32_t>)  // NOLINT
BOOST_CLASS_EXPORT_KEY(Gem::Common::GDecoratorContainer_3D<std::uint32_t>) // NOLINT
BOOST_CLASS_EXPORT_KEY(Gem::Common::GDecoratorContainer_3D<float>)         // NOLINT
BOOST_CLASS_EXPORT_KEY(Gem::Common::GDecoratorContainer_3D<double>)        // NOLINT
BOOST_CLASS_EXPORT_KEY(Gem::Common::GHistogram1D)                          // NOLINT
BOOST_CLASS_EXPORT_KEY(Gem::Common::GHistogram1I)                          // NOLINT
BOOST_CLASS_EXPORT_KEY(Gem::Common::GHistogram2D)                          // NOLINT
BOOST_CLASS_EXPORT_KEY(Gem::Common::GGraph2D)                              // NOLINT
BOOST_CLASS_EXPORT_KEY(Gem::Common::GGraph2ED)                             // NOLINT
BOOST_CLASS_EXPORT_KEY(Gem::Common::GGraph3D)                              // NOLINT
BOOST_CLASS_EXPORT_KEY(Gem::Common::GGraph4D)                              // NOLINT
BOOST_CLASS_EXPORT_KEY(Gem::Common::GFunctionPlotter1D)                    // NOLINT
BOOST_CLASS_EXPORT_KEY(Gem::Common::GFunctionPlotter2D)                    // NOLINT
BOOST_CLASS_EXPORT_KEY(Gem::Common::GPlotDesigner)                         // NOLINT
