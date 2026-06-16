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

    template <typename Archive>
    void serialize(Archive &ar, const unsigned int) {
        using boost::serialization::make_nvp;

        ar &BOOST_SERIALIZATION_NVP(drawing_arguments_) & BOOST_SERIALIZATION_NVP(x_axis_label_) &
            BOOST_SERIALIZATION_NVP(y_axis_label_) & BOOST_SERIALIZATION_NVP(z_axis_label_) &
            BOOST_SERIALIZATION_NVP(plot_label_) & BOOST_SERIALIZATION_NVP(ds_marker_) &
            BOOST_SERIALIZATION_NVP(secondary_plotter_) & BOOST_SERIALIZATION_NVP(id_);
    }
    ///////////////////////////////////////////////////////////////////////

public:
    /** @brief Copy constructor */
    GBasePlotter(GBasePlotter const &);
    /** @brief Assignment operator */
    GBasePlotter &operator=(GBasePlotter const &);

    /*********************************************************************/
    // Defaulted constructors, destructor and assignment operators

    GBasePlotter() = default;
    GBasePlotter(GBasePlotter &&) = default;
    ~GBasePlotter() override = default;

    GBasePlotter &operator=(GBasePlotter &&) = default;

    /*********************************************************************/

    /** @brief Allows to set the drawing arguments for this plot */
    void setDrawingArguments(std::string);

    /** @brief Sets the label for the x-axis */
    void setXAxisLabel(std::string);
    /** @brief Retrieve the x-axis label */
    std::string xAxisLabel() const;
    /** @brief Sets the label for the y-axis */
    void setYAxisLabel(std::string);
    /** @brief Retrieve the y-axis label */
    std::string yAxisLabel() const;
    /** @brief Sets the label for the z-axis */
    void setZAxisLabel(std::string);
    /** @brief Retrieve the z-axis label */
    std::string zAxisLabel() const;

    /** @brief Allows to assign a label to the entire plot */
    void setPlotLabel(std::string);
    /** @brief Allows to retrieve the plot label */
    std::string plotLabel() const;

    /** @brief Allows to assign a marker to data structures */
    void setDataStructureMarker(std::string);
    /** @brief Allows to retrieve the data structure marker */
    std::string dsMarker() const;

    /** @brief Allows to add secondary plots to be added to the same sub-canvas */
    void registerSecondaryPlotter(std::shared_ptr<GBasePlotter>);

    /** @brief Allows to retrieve the id of this object */
    std::size_t id() const;
    /** @brief Sets the id of the object */
    void setId(const std::size_t &);

    /** @brief Retrieves a unique name for this plotter */
    virtual std::string getPlotterName() const = 0;

    /***************************************************************************/

    /** @brief Retrieve header settings for this plot (and any sub-plots) */
    std::string headerData(const std::string &) const;

    /** @brief Retrieves body / data settings for this plot (and any sub-plots) */
    std::string bodyData(const std::string &) const;

    /** @brief Retrieves footer / drawing settings for this plot (and any sub-plots) */
    std::string footerData(const std::string &) const;

protected:
    /***************************************************************************/
    /** @brief Loads the data of another object */
    void load_(const GBasePlotter *) override;

    /** @brief Allow access to this classes compare_ function */
    friend void compare_base_t<GBasePlotter>(GBasePlotter const &, GBasePlotter const &, GToken &);

    /** @brief Searches for compliance with expectations with respect to another object of the same type */
    void compare_(
        const GBasePlotter & // the other object
        ,
        const expectation & // the expectation for this object, e.g. equality
        ,
        const double & // the limit for allowed deviations of floating point types
    ) const override;

    /***************************************************************************/
    // Functions to be specified in derived classes

    /** @brief Retrieve specific header settings for this plot */
    virtual std::string headerData_(bool, std::size_t, const std::string &) const = 0;

    /** @brief Retrieves the actual data sets */
    virtual std::string bodyData_(bool, std::size_t, const std::string &) const = 0;

    /** @brief retrieves specific draw commands for this plot */
    virtual std::string footerData_(bool, std::size_t, const std::string &) const = 0;

    /** @brief Retrieve the current drawing arguments */
    virtual std::string drawingArguments(bool) const = 0;

    /** @brief Check that a given plotter is compatible with us */
    virtual bool isCompatible(std::shared_ptr<GBasePlotter>) const;

    /** @brief calculate a suffix from id and parent ids */
    std::string suffix(bool, std::size_t) const;

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
    /** @brief Returns the name of this class */
    std::string name_() const override;
    /** @brief Creates a deep clone of this object */
    GBasePlotter *clone_() const override = 0;

    /***************************************************************************/
    /** @brief A list of plotters that should emit their data into the same canvas */
    std::vector<std::shared_ptr<GBasePlotter>> secondary_plotter_;

    std::size_t id_ = 0; ///< The id of this object
};


} /* namespace Gem::Common */
