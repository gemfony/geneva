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
#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <iterator>
#include <sstream>
#include <string>
#include <tuple>
#include <utility>
#include <vector>

// Boost header files go here
#include <boost/archive/binary_iarchive.hpp>
#include <boost/archive/binary_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>
#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/xml_iarchive.hpp>
#include <boost/archive/xml_oarchive.hpp>
#include <boost/serialization/base_object.hpp>
#include <boost/serialization/export.hpp>
#include <boost/serialization/map.hpp>
#include <boost/serialization/nvp.hpp>
#include <boost/serialization/shared_ptr.hpp>
#include <boost/serialization/split_member.hpp>
#include <boost/serialization/tracking.hpp>
#include <boost/serialization/utility.hpp>
#include <boost/serialization/vector.hpp>

// Geneva header files go here
#include "common/GCommonHelperFunctions.hpp"
#include "common/GCommonHelperFunctionsT.hpp"
#include "common/GCommonMathHelperFunctionsT.hpp"
#include "common/GErrorStreamer.hpp"
#include "common/GExceptions.hpp"
#include "common/GLogger.hpp"
#include "common/GTupleIO.hpp"

namespace Gem {
namespace Geneva {

/******************************************************************************/
/**
 * A simple two-dimensional coordinate
 */
struct coord2D {
    /** @brief Construction with positions */
    coord2D(float, float);

    // Defaulted constructors, "rule of five"
    coord2D() = default;
    coord2D(coord2D const &) = default;
    coord2D(coord2D &&) = default;
    ~coord2D() = default;

    coord2D &operator=(coord2D const &) = default;
    coord2D &operator=(coord2D &&) = default;

    float x = 0.f;
    float y = 0.f;
};

/** @brief Convenience function for calculating the difference between two coordinate vectors */
coord2D operator-(coord2D const &, coord2D const &);

/** @brief Convenience function for calculating the dot product of two coordinate vectors */
float operator*(coord2D const &, coord2D const &);

/******************************************************************************/
/**
 * A struct holding the coordinates, colors and opacity of a single triangle, which is
 * defined via a surrounding circle
 */
using t_circle = struct triangle_circle_struct {
    //--------------------------------------------
    // Deleted or defaulted constructors and assignment
    // operators. Rule of five ...
    triangle_circle_struct() = default;
    triangle_circle_struct(triangle_circle_struct const &) = default;
    triangle_circle_struct(triangle_circle_struct &&) = default;
    ~triangle_circle_struct() = default;

    triangle_circle_struct &operator=(triangle_circle_struct const &) = default;
    triangle_circle_struct &operator=(triangle_circle_struct &&) = default;

    //--------------------------------------------

    /** @brief Needed for sorting */
    float getAlphaValue() const;

    /** @brief Translate to a string */
    std::string toString() const;

    //--------------------------------------------
    // Data

    coord2D middle{};
    float radius = 0.f;
    float angle1 = 0.f;
    float angle2 = 0.f;
    float angle3 = 0.f;
    float r = 0.f;
    float g = 0.f;
    float b = 0.f;
    float a = 0.f;
};

/** @brief Simplify debugging output */
std::ostream &operator<<(std::ostream &, t_circle const &);
/** @brief Simplify comparison of two t_circle structs */
bool operator==(t_circle const &, t_circle const &);
/** @brief Simplify comparison of two t_circle structs */
bool operator!=(t_circle const &, t_circle const &);

/******************************************************************************/
/**
 * A struct holding triangle definitions in standard coordinates
 */
using t_cart = struct t_spec_c {
    //--------------------------------------------
    // Deleted or defaulted constructors and assignment
    // operators. Rule of five ...
    t_spec_c() = default;
    t_spec_c(t_spec_c const &) = default;
    t_spec_c(t_spec_c &&) = default;
    ~t_spec_c() = default;

    t_spec_c &operator=(t_spec_c const &) = default;
    t_spec_c &operator=(t_spec_c &&) = default;

    //--------------------------------------------

    coord2D tr_one{};
    coord2D tr_two{};
    coord2D tr_three{};
    float r = 0.f;
    float g = 0.f;
    float b = 0.f;
    float a = 0.f;
};

/******************************************************************************/
/**
 * A simple class holding the rgb values of a pixel
 */
struct GRgb {
private:
    ///////////////////////////////////////////////////////////////////////
    friend class boost::serialization::access;

    template <typename Archive>
    void serialize(Archive &ar, const unsigned int) {
        using boost::serialization::make_nvp;

        ar &BOOST_SERIALIZATION_NVP(r) & BOOST_SERIALIZATION_NVP(g) & BOOST_SERIALIZATION_NVP(b);
    }
    ///////////////////////////////////////////////////////////////////////

public:
    /** @brief Initialization with colors */
    GRgb(float, float, float);
    /** @brief Initialization with colors held in a std::tuple */
    explicit GRgb(std::tuple<float, float, float> const &);

    //----------------------------------------------------
    // Defaulted constructors and destructors
    // Rule of five ...

    GRgb() = default;
    GRgb(GRgb const &) = default;
    GRgb(GRgb &&) = default;
    ~GRgb() = default;

    GRgb &operator=(GRgb const &) = default;
    GRgb &operator=(GRgb &&) = default;

    //--------------------------------------------

    /** @brief Explicit reset of colors */
    void setColor(float, float, float);
    /** @brief Explicit reset of colors, using a std::tuple */
    void setColor(std::tuple<float, float, float> const &);

    float r = 0.f; ///< red
    float g = 0.f; ///< green
    float b = 0.f; ///< blue
};

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/
/**
 * A column in a canvas
 */
class GColumn {
    ///////////////////////////////////////////////////////////////////////
    friend class boost::serialization::access;

    template <typename Archive>
    void serialize(Archive &ar, const unsigned int) {
        using boost::serialization::make_nvp;

        ar &BOOST_SERIALIZATION_NVP(column_data_mnt_);
    }
    ///////////////////////////////////////////////////////////////////////

public:
    /** @brief Initialization with dimensions and colors */
    GColumn(std::size_t, std::tuple<float, float, float> const &);

    //----------------------------------------------------
    // Defaulted constructors and destructors
    // Rule of five ...

    GColumn() = default;
    GColumn(GColumn const &) = default;
    GColumn(GColumn &&) = default;
    ~GColumn() = default;

    GColumn &operator=(GColumn const &) = default;
    GColumn &operator=(GColumn &&) = default;

    //--------------------------------------------

    /** @brief Information about the size of this object */
    [[nodiscard]] std::size_t size() const;

    /** @brief Unchecked access */
    GRgb &operator[](std::size_t);
    /** @brief Checked access */
    GRgb &at(std::size_t);
    /** @brief Unchecked access */
    const GRgb &operator[](std::size_t) const;
    /** @brief Checked access */
    const GRgb &at(std::size_t) const;

    /** @brief Initializes the object to a specific size */
    void init(std::size_t, std::tuple<float, float, float> const &);

private:
    std::vector<GRgb> column_data_mnt_; ///< Holds this column's pixels
};

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/
/**
 * A collection of pixels in a two-dimensional array
 */
template <std::size_t COLORDEPTH = 8>
class GCanvas {
    ///////////////////////////////////////////////////////////////////////
    friend class boost::serialization::access;

    template <typename Archive>
    void serialize(Archive &ar, const unsigned int) {
        using boost::serialization::make_nvp;

        ar &BOOST_SERIALIZATION_NVP(canvasData_) & BOOST_SERIALIZATION_NVP(xDim_) &
            BOOST_SERIALIZATION_NVP(yDim_);
    }
    ///////////////////////////////////////////////////////////////////////

public:
    /***************************************************************************/
    /**
	  * Initialization with dimensions and background colors. The default
	  * background color is black.
	  */
    GCanvas(
        std::tuple<std::size_t, std::size_t> const &dim,
        std::tuple<float, float, float> const &color
    ) {
        this->reset(dim, color);
    }

    /***************************************************************************/
    /**
	  * Initialization from data held in a string -- uses the PPM-P3 format
	  *
	  * @param ppmString A string holding a picture description in PPM-P3 format
	  */
    explicit GCanvas(std::string const &ppmString) {
        this->loadFromPPM(ppmString);
    }

    //----------------------------------------------------
    // Defaulted constructors and destructors
    // Rule of five ...

    GCanvas() = default;
    GCanvas(GCanvas<COLORDEPTH> const &) = default;
    GCanvas(GCanvas<COLORDEPTH> &&) = default;
    virtual ~GCanvas() = default;

    GCanvas<COLORDEPTH> &operator=(GCanvas<COLORDEPTH> const &) = default;
    GCanvas<COLORDEPTH> &operator=(GCanvas<COLORDEPTH> &&) = default;

    //--------------------------------------------

    /***************************************************************************/
    /**
	  * Get information about the canvas dimensions
	  */
    [[nodiscard]] auto dimensions() const {
        return std::tuple<std::size_t, std::size_t>{xDim_, yDim_};
    }

    /***************************************************************************/
    /**
	  * Retrieves the size in x-direction
	  *
	  * @return The value of the xDim_ parameter
	  */
    [[nodiscard]] std::size_t getXDim() const {
        return xDim_;
    }

    /***************************************************************************/
    /**
	  * Retrieve the size in y-direction
	  *
	  * @return The value of the yDim_ parameter
	  */
    [[nodiscard]] std::size_t getYDim() const {
        return yDim_;
    }

    /***************************************************************************/
    /**
	  * Retrieve the total number of pixels
	  *
	  * @return The total number of pixels in the canvas
	  */
    [[nodiscard]] std::size_t getNPixels() const {
        return xDim_ * yDim_;
    }

    /***************************************************************************/
    /**
	  * Retrieve our color depth.
	  *
	  * @return The chosen color depth
	  */
    [[nodiscard]] std::size_t getColorDepth() const {
        return COLORDEPTH;
    }

    /***************************************************************************/
    /**
	  * Retrieve the number of colors
	  *
	  * @return The number of representable colors
	  */
    [[nodiscard]] std::size_t getNColors() const {
        return NCOLORS;
    }

    /***************************************************************************/
    /**
	  * Retrieve the maximum color value
	  *
	  * @return The maximum allowed color value
	  */
    [[nodiscard]] std::size_t getMaxColor() const {
        return MAXCOLOR;
    }

    /***************************************************************************/
    /**
	  * Unchecked access
	  */
    GColumn &operator[](std::size_t pos) {
        return canvasData_[pos];
    }

    /***************************************************************************/
    /**
	  * Checked access
	  */
    GColumn &at(std::size_t pos) {
        return canvasData_.at(pos);
    }

    /***************************************************************************/
    /**
	  * Unchecked access
	  */
    const GColumn &operator[](std::size_t pos) const {
        return canvasData_[pos];
    }

    /***************************************************************************/
    /**
	  * Checked access
	  */
    const GColumn &at(std::size_t pos) const {
        return canvasData_.at(pos);
    }

    /***************************************************************************/
    /**
	  * Find out the deviation between this and another canvas
	  */
    [[nodiscard]] float diff(GCanvas<COLORDEPTH> const &cp) const {
        if(cp.dimensions() != this->dimensions()) {
            throw geneva_exception(
                g_error_streamer(DO_LOG, time_and_place)
                << "In GCanvas::diff(): Error!" << std::endl
                << "Dimensions differ: (" << std::get<0>(cp.dimensions()) << ", "
                << std::get<1>(cp.dimensions()) << ") / (" << std::get<0>(this->dimensions())
                << ", " << std::get<1>(this->dimensions()) << ")" << std::endl
            );
        }

        float result = 0.f;
        for(std::size_t i_x = 0; i_x < xDim_; i_x++) {
            for(std::size_t i_y = 0; i_y < yDim_; i_y++) {
                float dr = canvasData_[i_x][i_y].r - cp[i_x][i_y].r;
                float dg = canvasData_[i_x][i_y].g - cp[i_x][i_y].g;
                float db = canvasData_[i_x][i_y].b - cp[i_x][i_y].b;
                result += std::sqrt(dr * dr + dg * dg + db * db);
            }
        }

        return result;
    }

    /***************************************************************************/
    /**
	  * Converts the canvas to an image in PPM-P3 format
	  */
    [[nodiscard]] std::string toPPM() const {
        std::ostringstream result;

        result << "P3\n" << xDim_ << " " << yDim_ << '\n' << MAXCOLOR << '\n';

        for(std::size_t i_y = 0; i_y < yDim_; i_y++) {
            for(std::size_t i_x = 0; i_x < xDim_; i_x++) {
                result << static_cast<std::size_t>(
                              canvasData_[i_x][i_y].r * static_cast<float>(MAXCOLOR)
                          )
                       << " "
                       << static_cast<std::size_t>(
                              canvasData_[i_x][i_y].g * static_cast<float>(MAXCOLOR)
                          )
                       << " "
                       << static_cast<std::size_t>(
                              canvasData_[i_x][i_y].b * static_cast<float>(MAXCOLOR)
                          )
                       << " ";
            }
            result << '\n';
        }

        return result.str();
    }

    /***************************************************************************/
    /**
	  * Loads the data held in a string in PPM-P3 format
	  *
	  * @param ppmString A string holding an image in PPM-P3 format
	  */
    void loadFromPPM(std::string const &ppmString) {
        // Some status flags
        bool header_found = false;
        bool dimensions_found = false;
        bool color_depth_found = false;

        // Allows to read the string line by line
        std::istringstream input(ppmString);

        // Read the setup information. Skip empty lines and comments along the way.
        std::vector<std::size_t> v;
        std::string s; // Will hold newly read lines
        while(std::getline(input, s)) {
            v.clear();                 // Clear the vector
            std::istringstream iss(s); // Allows to retrieve sub-items from the line

            // Remove parts beginning with a # (i.e. comments)
            std::size_t pos = 0;
            if((pos = s.find('#')) != std::string::npos) {
                s.erase(pos); // Erase till the end of the string
            }

            // Remove leading or trailing white spaces
            { auto b=s.find_first_not_of(" \t\r\n"), e=s.find_last_not_of(" \t\r\n");
              s = (b==std::string::npos) ? std::string{} : s.substr(b, e-b+1); }

            // Skip empty lines
            if(s.empty()) {
                continue;
            }

            // The file should start with a header, which should read "P3". Complain if this isn't the case
            if(not header_found) {
                if(s != "P3") {
                    throw geneva_exception(
                        g_error_streamer(DO_LOG, time_and_place)
                        << "Error: Header should be \"P3\", but got " << s << std::endl
                    );
                }

                header_found = true;

                // Skip to the next line
                s.clear();
                continue;
            }

            // The next meaningful line of the input file should contain the picture dimensions
            if(not dimensions_found) {
                std::copy(
                    std::istream_iterator<std::size_t>(iss) // Begin reading from iss
                    ,
                    std::istream_iterator<std::size_t>() // End of stream
                    ,
                    std::back_inserter(v)
                );

                if(v.size() != 2) { // We should have received exactly two numbers
                    throw geneva_exception(
                        g_error_streamer(DO_LOG, time_and_place)
                        << "Error: Got invalid number of dimensions: " << v.size() << std::endl
                    );
                }

                if(v[0] == 0 || v[1] == 0) {
                    throw geneva_exception(
                        g_error_streamer(DO_LOG, time_and_place)
                        << "Error: Got invalid dimensions: " << v[0] << " / " << v[1] << std::endl
                    );
                }

                xDim_ = v[0];
                yDim_ = v[1];

                // Re-initialize the canvas with black
                this->reset(std::tuple<std::size_t, std::size_t>(xDim_, yDim_), 0.f, 0.f, 0.f);

                dimensions_found = true;

                // Skip to the next line
                s.clear();
                continue;
            }

            // Next should be the color depth
            if(not color_depth_found) {
                std::copy(
                    std::istream_iterator<std::size_t>(iss) // Begin reading from iss
                    ,
                    std::istream_iterator<std::size_t>() // End of stream
                    ,
                    std::back_inserter(v)
                );

                if(v.size() != 1) { // We should have received exactly one number
                    throw geneva_exception(
                        g_error_streamer(DO_LOG, time_and_place)
                        << "Error: Did not find specification of the number of colors" << std::endl
                        << "or an invalid number of specifications: " << v.size() << std::endl
                    );
                }

                // We only accept a single color depth for now. Except for this check, we
                // do nothing in this block
                if(v[0] != MAXCOLOR) {
                    throw geneva_exception(
                        g_error_streamer(DO_LOG, time_and_place)
                        << "Error: Got invalid color depth " << v[0] << std::endl
                    );
                }

                color_depth_found = true; // NOLINT
                s.clear();

                // We are ready to read the real data and terminate the loop
                break;
            }
        }

        if(not(header_found && dimensions_found && color_depth_found)) {
            throw geneva_exception(
                g_error_streamer(DO_LOG, time_and_place)
                << "Error: PPM string ended before all header fields were found." << std::endl
                << "  P3 header:   " << (header_found ? "found" : "MISSING") << std::endl
                << "  Dimensions:  " << (dimensions_found ? "found" : "MISSING") << std::endl
                << "  Color depth: " << (color_depth_found ? "found" : "MISSING") << std::endl
            );
        }

        // Read the per-pixel information
        v.clear();
        s.clear();
        while(std::getline(input, s)) {
            // Remove parts beginning with a # (i.e. comments)
            std::size_t pos = 0;
            if((pos = s.find('#')) != std::string::npos) {
                s.erase(pos); // Erase till the end of the string
            }

            // Remove leading or trailing white spaces
            { auto b=s.find_first_not_of(" \t\r\n"), e=s.find_last_not_of(" \t\r\n");
              s = (b==std::string::npos) ? std::string{} : s.substr(b, e-b+1); }

            // Skip empty lines
            if(s.empty()) {
                continue;
            }

            std::istringstream iss(s);

            // We are now getting to the color content. These are rgb integer triples
            std::copy(
                std::istream_iterator<std::size_t>(iss),
                std::istream_iterator<std::size_t>(),
                std::back_inserter(v)
            );

            // Return the string to pristine condition
            s.clear();
        }

        // v should now contain all per-pixel information. Check the size - as
        // we are reading triplets, the size of the vector is known.
        if(v.size() != 3 * xDim_ * yDim_) {
            throw geneva_exception(
                g_error_streamer(DO_LOG, time_and_place)
                << "Error: got invalid number of entries in line." << std::endl
                << "Expected " << 3 * xDim_ * yDim_ << ", but got " << v.size() << std::endl
                << "Note: xDim_ = " << xDim_ << ", yDim_ = " << yDim_ << std::endl
            );
        }

        // Add all pixel data to the canvas
        std::size_t offset = 0;
        for(std::size_t line_counter = 0; line_counter < yDim_; line_counter++) {
            for(std::size_t pixel_counter = 0; pixel_counter < xDim_; pixel_counter++) {
                offset = 3 * (line_counter * xDim_ + pixel_counter);

                canvasData_[pixel_counter][line_counter].r =
                    static_cast<float>(v[offset]) / static_cast<float>(MAXCOLOR);
                canvasData_[pixel_counter][line_counter].g =
                    static_cast<float>(v[offset + 1]) / static_cast<float>(MAXCOLOR);
                canvasData_[pixel_counter][line_counter].b =
                    static_cast<float>(v[offset + 2]) / static_cast<float>(MAXCOLOR);
            }
        }
    }

    /***************************************************************************/
    /**
	  * Loads the data held in a file in PPM-P3 format
	  *
	  * @param p The name of a file holding an image in PPM-P3 format
	  */
    void loadFromFile(std::filesystem::path const &p) {
        // Read in the entire file
        std::string imageData = Gem::Common::loadTextDataFromFile(p);

        // Hand the string over to loadFromPPM() -- it will do the rest
#ifdef DEBUG
        if(imageData.empty()) {
            throw geneva_exception(
                g_error_streamer(DO_LOG, time_and_place)
                << "GCanvas::loadFromFile(): Error!" << std::endl
                << "File data was empty" << std::endl
            );
        }
#endif

        this->loadFromPPM(imageData);
    }

    /***************************************************************************/
    /**
	  * Saves the canvas to a file
	  */
    void toFile(std::filesystem::path const &p) {
        std::ofstream result(p);

        if(not result) {
            throw geneva_exception(
                g_error_streamer(DO_LOG, time_and_place)
                << "In GCanvas<>::toFile(): Error!" << std::endl
                << "Could not open output file " << p.string() << std::endl
            );
        }

        result << this->toPPM();
        result.close();
    }

    /***************************************************************************/
    /**
	  * Removes all data from the canvas
	  */
    void clear() {
        canvasData_.clear();

        xDim_ = 0;
        yDim_ = 0;
    }

    /***************************************************************************/
    /**
	  * Resets the canvas to a given color and dimension
	  *
	  * @param
	  */
    void reset(
        std::tuple<std::size_t, std::size_t> const &dimension,
        float red,
        float green,
        float blue
    ) {
        this->clear();

        xDim_ = std::get<0>(dimension);
        yDim_ = std::get<1>(dimension);

        canvasData_.assign(
            xDim_,
            GColumn(yDim_, std::tuple<float, float, float>{red, green, blue})
        );
    }

    /***************************************************************************/
    /**
	  * Resets the canvas to a given color and dimension
	  */
    void reset(
        std::tuple<std::size_t, std::size_t> const &dimension,
        std::tuple<float, float, float> const &color
    ) {
        this->reset(dimension, std::get<0>(color), std::get<1>(color), std::get<2>(color));
    }

    /***************************************************************************/
    /**
	  * Adds a triangle to the canvas, using Gemfony's "circular" definition
	  */
    void addTriangle(t_circle const &t) {
        t_cart t_c;

#ifdef DEBUG
        // Check that angles are in consecutive order
        if(t.angle1 < 0.f || t.angle2 <= t.angle1 || t.angle3 <= t.angle2 || t.angle3 >= 1.f) {
            throw geneva_exception(
                g_error_streamer(DO_LOG, time_and_place)
                << "In GCanvas<>::addTriangle(): Error!" << std::endl
                << "Angles are not in consecutive oder: " << std::endl
                << t << std::endl
            );
        }
#endif /* DEBUG */

        // std::numbers is not available in all CUDA nvcc versions; use a local constexpr
        constexpr float two_pi_f = 6.28318530717958647692f;

        t_c.tr_one.x = t.middle.x + t.radius * std::cos(t.angle1 * two_pi_f);
        t_c.tr_one.y = t.middle.y + t.radius * std::sin(t.angle1 * two_pi_f);

        t_c.tr_two.x = t.middle.x + t.radius * std::cos(t.angle2 * two_pi_f);
        t_c.tr_two.y = t.middle.y + t.radius * std::sin(t.angle2 * two_pi_f);

        t_c.tr_three.x = t.middle.x + t.radius * std::cos(t.angle3 * two_pi_f);
        t_c.tr_three.y = t.middle.y + t.radius * std::sin(t.angle3 * two_pi_f);

        t_c.r = t.r;
        t_c.g = t.g;
        t_c.b = t.b;
        t_c.a = t.a;

        this->addTriangle(t_c);
    }

    /***************************************************************************/
    /**
	  * Adds a complete set of triangles to the canvas, using Gemfony's
	  * "circular" definition
	  */
    void addTriangles(std::vector<t_circle> const &ts) {
        for(auto const &t : ts) {
            this->addTriangle(t);
        }
    }

    /***************************************************************************/
    /**
	  * Adds a triangle to the canvas, using a struct holding cartesic coordinates
	  */
    void addTriangle(t_cart const &t) {
        float xDim_inv = 1.f / static_cast<float>(xDim_);
        float yDim_inv = 1.f / static_cast<float>(yDim_);
        float dot1p, dot2p, u, v;
        coord2D diffp1, pos_f;

        // These depend only on the triangle vertices — compute once
        coord2D diff31 = t.tr_three - t.tr_one;
        coord2D diff21 = t.tr_two - t.tr_one;
        float dot11 = diff31 * diff31;
        float dot12 = diff31 * diff21;
        float dot22 = diff21 * diff21;
        float denom_inv = 1.f / std::max(dot11 * dot22 - dot12 * dot12, 0.0000001f);

        for(std::size_t i_x = 0; i_x < xDim_; i_x++) {
            // Calculate the pixel x-position
            pos_f.x = float(i_x + 1) * xDim_inv;

            if(pos_f.x < t.tr_one.x && pos_f.x < t.tr_two.x && pos_f.x < t.tr_three.x) {
                continue;
            }

            if(pos_f.x > t.tr_one.x && pos_f.x > t.tr_two.x && pos_f.x > t.tr_three.x) {
                continue;
            }

            for(std::size_t i_y = 0; i_y < yDim_; i_y++) {
                // Calculate the pixel y-position
                pos_f.y = float(i_y + 1) * yDim_inv;

                if(pos_f.y < t.tr_one.y && pos_f.y < t.tr_two.y && pos_f.y < t.tr_three.y) {
                    continue;
                }

                if(pos_f.y > t.tr_one.y && pos_f.y > t.tr_two.y && pos_f.y > t.tr_three.y) {
                    continue;
                }

                diffp1 = pos_f - t.tr_one;
                dot1p = diff31 * diffp1;
                dot2p = diff21 * diffp1;

                u = (dot22 * dot1p - dot12 * dot2p) * denom_inv;
                v = (dot11 * dot2p - dot12 * dot1p) * denom_inv;

                if((u >= 0.f) && (v >= 0.f) && (u + v < 1.f)) {
                    canvasData_[i_x][i_y].r =
                        canvasData_[i_x][i_y].r + t.a * (t.r - canvasData_[i_x][i_y].r);
                    canvasData_[i_x][i_y].g =
                        canvasData_[i_x][i_y].g + t.a * (t.g - canvasData_[i_x][i_y].g);
                    canvasData_[i_x][i_y].b =
                        canvasData_[i_x][i_y].b + t.a * (t.b - canvasData_[i_x][i_y].b);
                }
            }
        }
    }

    /***************************************************************************/
    /**
	  * Calculates the average colors over all pixels
	  */
    [[nodiscard]] auto getAverageColors() const {
        float averageRed = 0.f;
        float averageGreen = 0.f;
        float averageBlue = 0.f;

        for(std::size_t i_x = 0; i_x < xDim_; i_x++) {
            for(std::size_t i_y = 0; i_y < yDim_; i_y++) {
                averageRed += canvasData_[i_x][i_y].r;
                averageGreen += canvasData_[i_x][i_y].g;
                averageBlue += canvasData_[i_x][i_y].b;
            }
        }

        averageRed /= static_cast<float>(xDim_ * yDim_);
        averageGreen /= static_cast<float>(xDim_ * yDim_);
        averageBlue /= static_cast<float>(xDim_ * yDim_);

        return std::tuple<float, float, float>{averageRed, averageGreen, averageBlue};
    }

    /***************************************************************************/

protected:
    std::size_t xDim_ = 0, yDim_ = 0; ///< The dimensions of this canvas
    std::vector<GColumn> canvasData_;  ///< Holds this canvas' columns

    static constexpr std::size_t NCOLORS = Gem::Common::PowSmallPosInt<2, COLORDEPTH>();
    static constexpr std::size_t MAXCOLOR = NCOLORS - 1;
};

/******************************************************************************/
/**
  * Convenience function for the calculation of the difference between two canvasses
  */
template <std::size_t COLORDEPTH>
float operator-(GCanvas<COLORDEPTH> const &x, GCanvas<COLORDEPTH> const &y) {
    return x.diff(y);
}

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/
/**
 * Specialization of GCanvas for a color depth of 8 bits
 */
class GCanvas8 : public GCanvas<8> {
    ///////////////////////////////////////////////////////////////////////
    friend class boost::serialization::access;

    template <typename Archive>
    void serialize(Archive &ar, const unsigned int) {
        using boost::serialization::make_nvp;

        ar &BOOST_SERIALIZATION_BASE_OBJECT_NVP(GCanvas<8>);
    }
    ///////////////////////////////////////////////////////////////////////

public:
    /** @brief Initialization with dimensions and colors */
    
    GCanvas8(std::tuple<std::size_t, std::size_t> const &, std::tuple<float, float, float> const &);
    /** @brief Initialization from data held in a string -- uses the PPM-P3 format */
    explicit GCanvas8(std::string const &);
    /** @brief Copy construction */

    //----------------------------------------------------
    // Defaulted constructors and destructors
    // Rule of five ...

    GCanvas8() = default;
    GCanvas8(GCanvas8 const &) = default;
    GCanvas8(GCanvas8 &&) = default;
    ~GCanvas8() override = default;

    GCanvas8 &operator=(GCanvas8 const &) = default;
    GCanvas8 &operator=(GCanvas8 &&) = default;

    //-----------------------------------------------------
};

/** @brief Convenience function for the calculation of the difference between two canvasses */
float operator-(GCanvas8 const &, GCanvas8 const &);

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/
/**
 * Specialization of GCanvas for a color depth of 16 bits
 */
class GCanvas16 : public GCanvas<16> {
    ///////////////////////////////////////////////////////////////////////
    friend class boost::serialization::access;

    template <typename Archive>
    void serialize(Archive &ar, const unsigned int) {
        using boost::serialization::make_nvp;

        ar &BOOST_SERIALIZATION_BASE_OBJECT_NVP(GCanvas<16>);
    }
    ///////////////////////////////////////////////////////////////////////

public:
    /** @brief Initialization with dimensions and colors */
    GCanvas16(
        std::tuple<std::size_t, std::size_t> const &,
        std::tuple<float, float, float> const &
    );

    /** @brief Initialization from data held in a string -- uses the PPM-P3 format */
    explicit GCanvas16(std::string const &);

    //----------------------------------------------------
    // Defaulted constructors and destructors
    // Rule of five ...

    GCanvas16() = default;
    GCanvas16(GCanvas16 const &) = default;
    GCanvas16(GCanvas16 &&) = default;

    ~GCanvas16() override = default;

    GCanvas16 &operator=(GCanvas16 const &) = default;
    GCanvas16 &operator=(GCanvas16 &&) = default;

    //-----------------------------------------------------
};

/** @brief Convenience function for the calculation of the difference between two canvasses */
float operator-(GCanvas16 const &, GCanvas16 const &);

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/
/**
 * Specialization of GCanvas for a color depth of 24 bits
 */
class GCanvas24 : public GCanvas<24> {
    ///////////////////////////////////////////////////////////////////////
    friend class boost::serialization::access;

    template <typename Archive>
    void serialize(Archive &ar, const unsigned int) {
        using boost::serialization::make_nvp;

        ar &BOOST_SERIALIZATION_BASE_OBJECT_NVP(GCanvas<24>);
    }
    ///////////////////////////////////////////////////////////////////////

public:
    /** @brief Initialization with dimensions and colors */
    GCanvas24(
        std::tuple<std::size_t, std::size_t> const &,
        std::tuple<float, float, float> const &
    );
    /** @brief Initialization from data held in a string -- uses the PPM-P3 format */
    explicit GCanvas24(std::string const &);

    //----------------------------------------------------
    // Defaulted constructors and destructors
    // Rule of five ...

    GCanvas24() = default;
    GCanvas24(GCanvas24 const &) = default;
    GCanvas24(GCanvas24 &&) = default;

    ~GCanvas24() override = default;

    GCanvas24 &operator=(GCanvas24 const &) = default;
    GCanvas24 &operator=(GCanvas24 &&) = default;

    //-----------------------------------------------------
};

/** @brief Convenience function for the calculation of the difference between two canvasses */
float operator-(GCanvas24 const &, GCanvas24 const &);

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/

} /* namespace Geneva */
} /* namespace Gem */

BOOST_CLASS_EXPORT_KEY(Gem::Geneva::GCanvas8)  // NOLINT
BOOST_CLASS_EXPORT_KEY(Gem::Geneva::GCanvas16) // NOLINT
BOOST_CLASS_EXPORT_KEY(Gem::Geneva::GCanvas24) // NOLINT
