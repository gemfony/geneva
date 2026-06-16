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

    template <typename Archive>
    void serialize(Archive &ar, const unsigned int) {
        using boost::serialization::make_nvp;

        ar &BOOST_SERIALIZATION_NVP(c_x_div_) & BOOST_SERIALIZATION_NVP(c_y_div_) &
            BOOST_SERIALIZATION_NVP(c_x_dim_) & BOOST_SERIALIZATION_NVP(c_y_dim_) &
            BOOST_SERIALIZATION_NVP(canvas_label_) & BOOST_SERIALIZATION_NVP(add_print_command_) &
            BOOST_SERIALIZATION_NVP(n_indention_spaces_);
    }
    ///////////////////////////////////////////////////////////////////////

public:
    /** @brief The standard constructor */
    GPlotDesigner(const std::string &, const std::size_t &, const std::size_t &);

    /** @brief Copy constructor */
    GPlotDesigner(GPlotDesigner const &);
    /** @brief Assignment operator */
    GPlotDesigner &operator=(GPlotDesigner const &);

    /*********************************************************************/
    // Defaulted constructors, destructor and assignment operators

    // Defaulted default constructor in private section

    GPlotDesigner(GPlotDesigner &&) = default;
    ~GPlotDesigner() override = default;

    GPlotDesigner &operator=(GPlotDesigner &&) = default;

    /*********************************************************************/

    /* @brief Emits the overall plot */
    std::string
    plot(const std::filesystem::path & = std::filesystem::path("empty")) const;
    /** @brief Writes the plot to a file */
    void writeToFile(const std::filesystem::path &);

    /** @brief Allows to add a new plotter object */
    void registerPlotter(std::shared_ptr<GBasePlotter>);

    /** @brief Set the dimensions of the output canvas */
    void setCanvasDimensions(const std::uint32_t &, const std::uint32_t &);
    /** @brief Set the dimensions of the output canvas */
    void setCanvasDimensions(const std::tuple<std::uint32_t, std::uint32_t> &);
    /** @brief Allows to retrieve the canvas dimensions */
    std::tuple<std::uint32_t, std::uint32_t> getCanvasDimensions() const;

    /** @brief Allows to set the canvas label */
    void setCanvasLabel(const std::string &);
    /** @brief Allows to retrieve the canvas label */
    std::string getCanvasLabel() const;

    /** @brief Allows to add a "Print" command to the end of the script so that picture files are created */
    void setAddPrintCommand(bool);
    /** @brief Allows to retrieve the current value of the add_print_command_ variable */
    bool getAddPrintCommand() const;

    /** @brief Resets the plotters */
    void resetPlotters();

    /** @brief Allows to set the number of spaces used for indention */
    void setNIndentionSpaces(const std::size_t &);

    /** @brief Allows to retrieve the number spaces used for indention */
    std::size_t getNIndentionSpaces() const;

    /** @brief Returns the current number of indention spaces as a string */
    std::string indent() const;

protected:
    /** @brief A header for static data in a ROOT file */
    std::string staticHeader(const std::string &) const;

    /** @brief Loads the data of another object */
    void load_(const GPlotDesigner *) override;

    /** @brief Allow access to this classes compare_ function */
    friend void
    compare_base_t<GPlotDesigner>(GPlotDesigner const &, GPlotDesigner const &, GToken &);

    /** @brief Searches for compliance with expectations with respect to another object of the same type */
    void compare_(
        const GPlotDesigner & // the other object
        ,
        const expectation & // the expectation for this object, e.g. equality
        ,
        const double & // the limit for allowed deviations of floating point types
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

    /** @brief Returns the name of this class */
    std::string name_() const override;
    /** @brief Creates a deep clone of this object */
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
};

/******************************************************************************/

} /* namespace Gem::Common */

/******************************************************************************/
// Declare abstract or export class names for Boost.Serialization
namespace boost::serialization {

template <typename coordinate_type>
struct is_abstract<Gem::Common::GDecorator<Gem::Common::dimensions::Dim2, coordinate_type>>
  : public std::true_type { /* nothing */
};
template <typename coordinate_type>
struct is_abstract<const Gem::Common::GDecorator<Gem::Common::dimensions::Dim2, coordinate_type>>
  : public std::true_type { /* nothing */
};

template <typename coordinate_type>
struct is_abstract<Gem::Common::GDecorator<Gem::Common::dimensions::Dim3, coordinate_type>>
  : public std::true_type { /* nothing */
};
template <typename coordinate_type>
struct is_abstract<const Gem::Common::GDecorator<Gem::Common::dimensions::Dim3, coordinate_type>>
  : public std::true_type { /* nothing */
};

template <typename coordinate_type>
struct is_abstract<Gem::Common::GDecoratorContainer<Gem::Common::dimensions::Dim2, coordinate_type>>
  : public std::true_type { /* nothing */
};
template <typename coordinate_type>
struct is_abstract<
    const Gem::Common::GDecoratorContainer<Gem::Common::dimensions::Dim2, coordinate_type>>
  : public std::true_type { /* nothing */
};

template <typename coordinate_type>
struct is_abstract<Gem::Common::GDecoratorContainer<Gem::Common::dimensions::Dim3, coordinate_type>>
  : public std::true_type { /* nothing */
};
template <typename coordinate_type>
struct is_abstract<
    const Gem::Common::GDecoratorContainer<Gem::Common::dimensions::Dim3, coordinate_type>>
  : public std::true_type { /* nothing */
};

template <typename x_type>
struct is_abstract<Gem::Common::GDataCollector1T<x_type>> : public std::true_type { /* nothing */
};
template <typename x_type>
struct is_abstract<const Gem::Common::GDataCollector1T<x_type>>
  : public std::true_type { /* nothing */
};

template <typename x_type, typename y_type>
struct is_abstract<Gem::Common::GDataCollector2T<x_type, y_type>>
  : public std::true_type { /* nothing */
};
template <typename x_type, typename y_type>
struct is_abstract<const Gem::Common::GDataCollector2T<x_type, y_type>>
  : public std::true_type { /* nothing */
};

template <typename x_type, typename y_type>
struct is_abstract<Gem::Common::GDataCollector2ET<x_type, y_type>>
  : public std::true_type { /* nothing */
};
template <typename x_type, typename y_type>
struct is_abstract<const Gem::Common::GDataCollector2ET<x_type, y_type>>
  : public std::true_type { /* nothing */
};

template <typename x_type, typename y_type, typename z_type>
struct is_abstract<Gem::Common::GDataCollector3T<x_type, y_type, z_type>>
  : public std::true_type { /* nothing */
};
template <typename x_type, typename y_type, typename z_type>
struct is_abstract<const Gem::Common::GDataCollector3T<x_type, y_type, z_type>>
  : public std::true_type { /* nothing */
};

template <typename x_type, typename y_type, typename z_type, typename w_type>
struct is_abstract<Gem::Common::GDataCollector4T<x_type, y_type, z_type, w_type>>
  : public std::true_type { /* nothing */
};
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
