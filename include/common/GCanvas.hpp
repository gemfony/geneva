/**
 * @file
 */

/*
 * Copyright (C) Gemfony scientific UG (haftungsbeschraenkt)
 *
 * See the AUTHORS file in the top-level directory for a list of authors.
 *
 * Contact: contact [at] gemfony (dot) eu
 *
 * This file is part of the Geneva library collection.
 *
 * Geneva was developed with kind support from Karlsruhe Institute of
 * Technology (KIT) and Steinbuch Centre for Computing (SCC). Further
 * information about KIT and SCC can be found at http://www.kit.edu/english
 * and http://scc.kit.edu .
 *
 * Geneva is free software: you can redistribute and/or modify it under
 * the terms of version 3 of the GNU Affero General Public License
 * as published by the Free Software Foundation.
 *
 * Geneva is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with the Geneva library. If not, see <http://www.gnu.org/licenses/>.
 *
 * For further information on Gemfony scientific and Geneva, visit
 * http://www.gemfony.eu .
 */

#pragma once

// Global checks, defines and includes needed for all of Geneva
#include "common/GGlobalDefines.hpp"

// Standard header files go here
#include <iostream>
#include <sstream>
#include <fstream>
#include <string>
#include <cstdlib>
#include <vector>
#include <iterator>
#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <vector>
#include <list>
#include <algorithm> // for std::sort
#include <utility> // For std::pair
#include <tuple>

// Boost header files go here
#include <boost/filesystem.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/algorithm/string/trim.hpp>
#include <boost/cast.hpp>
#include <boost/filesystem.hpp>
#include <boost/filesystem/fstream.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/math/constants/constants.hpp>
#include <boost/optional.hpp>

#include <boost/archive/xml_oarchive.hpp>
#include <boost/archive/xml_iarchive.hpp>
#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>
#include <boost/archive/binary_oarchive.hpp>
#include <boost/archive/binary_iarchive.hpp>
#include <boost/serialization/nvp.hpp>
#include <boost/serialization/map.hpp>
#include <boost/serialization/variant.hpp>
#include <boost/serialization/vector.hpp>
#include <boost/serialization/shared_ptr.hpp>
#include <boost/serialization/base_object.hpp>
#include <boost/serialization/utility.hpp>
#include <boost/serialization/tracking.hpp>
#include <boost/serialization/split_member.hpp>
#include <boost/serialization/export.hpp>

// Geneva header files go here
#include "common/GLogger.hpp"
#include "common/GExceptions.hpp"
#include "common/GErrorStreamer.hpp"
#include "common/GCommonHelperFunctions.hpp"
#include "common/GCommonHelperFunctionsT.hpp"
#include "common/GCommonMathHelperFunctions.hpp"
#include "common/GCommonMathHelperFunctionsT.hpp"
#include "common/GTupleIO.hpp"

// aliases for ease of use
namespace bf = boost::filesystem;

namespace Gem {
namespace Common {

/******************************************************************************/
/**
 * A simple two-dimensional coordinate
 */
struct coord2D {
	 /** @brief Construction with positions */
	 coord2D(float, float);

	 // Defaulted constructors, "rule of five"
	 coord2D() = default;
	 coord2D(coord2D const&) = default;
	 coord2D(coord2D &&) = default;
	 ~coord2D() = default;

	 coord2D& operator=(coord2D const&) = default;
	 coord2D& operator=(coord2D &&) = default;

	 float x = 0.f;
	 float y = 0.f;
};

/** @brief Convenience function for calculating the difference between two coordinate vectors */
coord2D operator-(coord2D const&, coord2D const&);

/** @brief Convenience function for calculating the dot product of two coordinate vectors */
float operator*(coord2D const&, coord2D const&);

/******************************************************************************/
/**
 * A struct holding the coordinates, colors and opacity of a single triangle, which is
 * defined via a surrounding circle
 */
using t_circle =
struct triangle_circle_struct {
	 //--------------------------------------------
	 // Deleted or defaulted constructors and assignment
	 // operators. Rule of five ...
	 triangle_circle_struct() = default;
	 triangle_circle_struct(triangle_circle_struct const&) = default;
	 triangle_circle_struct(triangle_circle_struct &&) = default;
	 ~triangle_circle_struct() = default;

	 triangle_circle_struct& operator=(triangle_circle_struct const&) = default;
	 triangle_circle_struct& operator=(triangle_circle_struct &&) = default;

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
G_API_COMMON std::ostream &
operator<<(std::ostream &, t_circle const&);
/** @brief Simplify comparison of two t_circle structs */
G_API_COMMON bool
operator==(t_circle const&, t_circle const&);
/** @brief Simplify comparison of two t_circle structs */
G_API_COMMON bool
operator!=(t_circle const&, t_circle const&);

/******************************************************************************/
/**
 * A struct holding triangle definitions in standard coordinates
 */
using t_cart =
struct t_spec_c {
	//--------------------------------------------
	// Deleted or defaulted constructors and assignment
	// operators. Rule of five ...
	t_spec_c() = default;
	t_spec_c(t_spec_c const&) = default;
	t_spec_c(t_spec_c &&) = default;
	~t_spec_c() = default;

	t_spec_c& operator=(t_spec_c const&);
	t_spec_c& operator=(t_spec_c &&);

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

	 template<typename Archive>
	 void serialize(Archive &ar, const unsigned int) {
		 using boost::serialization::make_nvp;

		 ar
		 & BOOST_SERIALIZATION_NVP(r)
		 & BOOST_SERIALIZATION_NVP(g)
		 & BOOST_SERIALIZATION_NVP(b);
	 }
	 ///////////////////////////////////////////////////////////////////////

public:
	 /** @brief Initialization with colors */
	 G_API_COMMON GRgb(float, float, float);
	 /** @brief Initialization with colors held in a std::tuple */
	 explicit G_API_COMMON GRgb(
	 	std::tuple<float, float, float> const&
	 );

	 //----------------------------------------------------
	 // Defaulted constructors and destructors
	 // Rule of five ...

	 G_API_COMMON GRgb() = default;
	 G_API_COMMON GRgb(GRgb const&) = default;
	 G_API_COMMON GRgb(GRgb &&) = default;
	 virtual G_API_COMMON ~GRgb() BASE = default;

	 G_API_COMMON GRgb& operator=(GRgb const&) = default;
	 G_API_COMMON GRgb& operator=(GRgb &&) = default;

	//--------------------------------------------

	/** @brief Explicit reset of colors */
	 G_API_COMMON void setColor(float, float, float);
	 /** @brief Explicit reset of colors, using a std::tuple */
	 G_API_COMMON void setColor(
	 	std::tuple<float, float, float>
	 );

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

	 template<typename Archive>
	 void serialize(Archive &ar, const unsigned int) {
		 using boost::serialization::make_nvp;

		 ar &BOOST_SERIALIZATION_NVP(m_column_data_mnt);
	 }
	 ///////////////////////////////////////////////////////////////////////

public:
	 /** @brief Initialization with dimensions and colors */
	 G_API_COMMON GColumn(
	 	 std::size_t
		 , std::tuple<float, float, float> const&
	 );

	//----------------------------------------------------
	// Defaulted constructors and destructors
	// Rule of five ...

	 G_API_COMMON GColumn() = default;
	 G_API_COMMON GColumn(GColumn const &) = default;
	 G_API_COMMON GColumn(GColumn &&) = default;
	 virtual G_API_COMMON ~GColumn() BASE = default;

	 G_API_COMMON  GColumn &operator=(GColumn const &) = default;
	 G_API_COMMON  GColumn &operator=(GColumn &&) = default;

	//--------------------------------------------

	/** @brief Information about the size of this object */
	 G_API_COMMON std::size_t size() const;

	 /** @brief Unchecked access */
	 G_API_COMMON GRgb &operator[](std::size_t);
	 /** @brief Checked access */
	 G_API_COMMON GRgb &at(std::size_t);
	 /** @brief Unchecked access */
	 G_API_COMMON const GRgb &operator[](std::size_t) const;
	 /** @brief Checked access */
	 G_API_COMMON const GRgb &at(std::size_t) const;

	 /** @brief Initializes the object to a specific size */
	 G_API_COMMON void init(
		 std::size_t
		 , std::tuple<float, float, float> const&
	 );

private:
	 std::vector<GRgb> m_column_data_mnt;  ///< Holds this column's pixels
};

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/
/**
 * A collection of pixels in a two-dimensional array
 */
template<std::size_t COLORDEPTH = 8>
class GCanvas {
	 ///////////////////////////////////////////////////////////////////////
	 friend class boost::serialization::access;

	 template<typename Archive>
	 void serialize(Archive &ar, const unsigned int) {
		 using boost::serialization::make_nvp;

		 ar
		 & BOOST_SERIALIZATION_NVP(m_canvasData)
		 & BOOST_SERIALIZATION_NVP(m_xDim)
		 & BOOST_SERIALIZATION_NVP(m_yDim);
	 }
	 ///////////////////////////////////////////////////////////////////////

public:
	 /***************************************************************************/
	 /**
	  * Initialization with dimensions and background colors. The default
	  * background color is black.
	  */
	 GCanvas(
		 std::tuple<std::size_t, std::size_t> const& dim
		 , std::tuple<float, float, float> const& color
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
	 virtual ~GCanvas() BASE = default;

	 GCanvas<COLORDEPTH> & operator=(GCanvas<COLORDEPTH> const &) = default;
	 GCanvas<COLORDEPTH> & operator=(GCanvas<COLORDEPTH> &&) = default;

	//--------------------------------------------


	/***************************************************************************/
	 /**
	  * Get information about the canvas dimensions
	  */
	 auto
	 dimensions() const {
		 return std::tuple<std::size_t, std::size_t>{m_xDim, m_yDim};
	 }

	 /***************************************************************************/
	 /**
	  * Retrieves the size in x-direction
	  *
	  * @return The value of the xDim_ parameter
	  */
	 std::size_t
	 getXDim() const {
		 return m_xDim;
	 }

	 /***************************************************************************/
	 /**
	  * Retrieve the size in y-direction
	  *
	  * @return The value of the yDim_ parameter
	  */
	 std::size_t
	 getYDim() const {
		 return m_yDim;
	 }

	 /***************************************************************************/
	 /**
	  * Retrieve the total number of pixels
	  *
	  * @return The total number of pixels in the canvas
	  */
	 std::size_t
	 getNPixels() const {
		 return m_xDim * m_yDim;
	 }

	 /***************************************************************************/
	 /**
	  * Retrieve our color depth.
	  *
	  * @return The chosen color depth
	  */
	 std::size_t
	 getColorDepth() const {
		 return COLORDEPTH;
	 }

	 /***************************************************************************/
	 /**
	  * Retrieve the number of colors
	  *
	  * @return The number of representable colors
	  */
	 std::size_t
	 getNColors() const {
		 return NCOLORS;
	 }

	 /***************************************************************************/
	 /**
	  * Retrieve the maximum color value
	  *
	  * @return The maximum allowed color value
	  */
	 std::size_t
	 getMaxColor() const {
		 return MAXCOLOR;
	 }

	 /***************************************************************************/
	 /**
	  * Unchecked access
	  */
	 GColumn &operator[](std::size_t pos) {
		 return m_canvasData[pos];
	 }

	 /***************************************************************************/
	 /**
	  * Checked access
	  */
	 GColumn &
	 at(std::size_t pos) {
		 return m_canvasData.at(pos);
	 }

	 /***************************************************************************/
	 /**
	  * Unchecked access
	  */
	 const GColumn &
	 operator[](std::size_t pos) const {
		 return m_canvasData[pos];
	 }

	 /***************************************************************************/
	 /**
	  * Checked access
	  */
	 const GColumn &
	 at(std::size_t pos) const {
		 return m_canvasData.at(pos);
	 }

	 /***************************************************************************/
	 /**
	  * Find out the deviation between this and another canvas
	  */
	 float
	 diff(GCanvas<COLORDEPTH> const &cp) const {
		 using namespace Gem::Common;

		 if (cp.dimensions() != this->dimensions()) {
			 throw gemfony_exception(
				 g_error_streamer(DO_LOG, time_and_place)
					 << "In GCanvas::diff(): Error!" << std::endl
					 << "Dimensions differ: (" << std::get<0>(cp.dimensions()) << ", " << std::get<1>(cp.dimensions()) << ") / ("
					 << std::get<0>(this->dimensions()) << ", " << std::get<1>(this->dimensions()) << ")" << std::endl
			 );
		 }

		 float result = 0.f;
		 for (std::size_t i_x = 0; i_x < m_xDim; i_x++) {
			 for (std::size_t i_y = 0; i_y < m_yDim; i_y++) {
				 result += gsqrt(
					 gpow((m_canvasData[i_x][i_y]).r - (cp[i_x][i_y]).r, 2.f)
					 + gpow((m_canvasData[i_x][i_y]).g - (cp[i_x][i_y]).g, 2.f)
					 + gpow((m_canvasData[i_x][i_y]).b - (cp[i_x][i_y]).b, 2.f)
				 );
			 }
		 }

		 return result;
	 }

	 /***************************************************************************/
	 /**
	  * Converts the canvas to an image in PPM-P3 format
	  */
	 std::string
	 toPPM() const {
		 std::ostringstream result;

		 result
			 << "P3" << std::endl
			 << m_xDim << " " << m_yDim << std::endl
			 << MAXCOLOR << std::endl;

		 for (std::size_t i_y = 0; i_y < m_yDim; i_y++) {
			 for (std::size_t i_x = 0; i_x < m_xDim; i_x++) {
				 result
					 << (std::size_t) (m_canvasData[i_x][i_y].r * float(MAXCOLOR)) << " "
					 << (std::size_t) (m_canvasData[i_x][i_y].g * float(MAXCOLOR)) << " "
					 << (std::size_t) (m_canvasData[i_x][i_y].b * float(MAXCOLOR)) << " ";
			 }
			 result << std::endl;
		 }

		 return result.str();
	 }

	 /***************************************************************************/
	 /**
	  * Loads the data held in a string in PPM-P3 format
	  *
	  * @param ppmString A string holding an image in PPM-P3 format
	  */
	 void
	 loadFromPPM(std::string const &ppmString) {
		 using namespace std;

		 // Some status flags
		 bool header_found = false;
		 bool dimensions_found = false;
		 bool color_depth_found = false;

		 // Allows to read the string line by line
		 std::istringstream input(ppmString);

		 // Read the setup information. Skip empty lines and comments along the way.
		 std::vector<std::size_t> v;
		 std::string s; // Will hold newly read lines
		 while (std::getline(input, s)) {
			 v.clear(); // Clear the vector
			 std::istringstream iss(s); // Allows to retrieve sub-items from the line

			 // Remove parts beginning with a # (i.e. comments)
			 std::size_t pos = 0;
			 if ((pos = s.find('#')) != string::npos) {
				 s.erase(pos); // Erase till the end of the string
			 }

			 // Remove leading or trailing white spaces
			 boost::trim(s);

			 // Skip empty lines
			 if (s.empty()) {
				 s.clear();
				 continue;
			 }

			 // The file should start with a header, which should read "P3". Complain if this isn't the case
			 if (not header_found) {
				 if (s != "P3") {
					 throw gemfony_exception(
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
			 if (not dimensions_found) {
				 std::copy(istream_iterator<std::size_t>(iss), istream_iterator<std::size_t>(), back_inserter(v));

				 if (v.size() != 2) { // We should have received exactly two numbers
					 throw gemfony_exception(
						 g_error_streamer(DO_LOG, time_and_place)
							 << "Error: Got invalid number of dimensions: " << v.size() << std::endl
					 );
				 }

				 if (v[0] <= 0 || v[1] <= 0) {
					 throw gemfony_exception(
						 g_error_streamer(DO_LOG, time_and_place)
							 << "Error: Got invalid dimensions: " << v[0] << " / " << v[1] << std::endl
					 );
				 }

				 m_xDim = v[0];
				 m_yDim = v[1];

				 // Re-initialize the canvas with black
				 this->reset(
					 std::tuple<std::size_t, std::size_t>(m_xDim, m_yDim), 0.f, 0.f, 0.f
				 );

				 dimensions_found = true;

				 // Skip to the next line
				 s.clear();
				 continue;
			 }

			 // Next should be the color depth
			 if (not color_depth_found) {
				 copy(istream_iterator<std::size_t>(iss), istream_iterator<std::size_t>(), back_inserter(v));

				 if (v.size() != 1) { // We should have received exactly one number
					 throw gemfony_exception(
						 g_error_streamer(DO_LOG, time_and_place)
							 << "Error: Did not find specification of the number of colors" << std::endl
							 << "or an invalid number of specifications: " << v.size() << std::endl
					 );
				 }

				 // We only accept a single color depth for now. Except for this check, we
				 // do nothing in this block
				 if (v[0] != MAXCOLOR) {
					 throw gemfony_exception(
						 g_error_streamer(DO_LOG, time_and_place)
							 << "Error: Got invalid color depth " << v[0] << std::endl
					 );
				 }

				 color_depth_found = true; // NOLINT
				 s.clear();
			 }

			 // We are ready to read the real data and terminate the loop
			 break;
		 }

		 // Read the per-pixel information
		 v.clear();
		 s.clear();
		 while (std::getline(input, s)) {
			 // Remove parts beginning with a # (i.e. comments)
			 std::size_t pos = 0;
			 if ((pos = s.find('#')) != string::npos) {
				 s.erase(pos); // Erase till the end of the string
			 }

			 // Remove leading or trailing white spaces
			 boost::trim(s);

			 // Skip empty lines
			 if (s.empty()) {
				 s.clear();
				 continue;
			 }

			 istringstream iss(s);

			 // We are now getting to the color content. These are rgb integer triples
			 copy(istream_iterator<std::size_t>(iss), istream_iterator<std::size_t>(), back_inserter(v));

			 // Return the string to pristine condition
			 s.clear();
		 }

		 // v should now contain all per-pixel information. Check the size - as
		 // we are reading triplets, the size of the vector is known.
		 if (v.size() != 3 * m_xDim * m_yDim) {
			 throw gemfony_exception(
				 g_error_streamer(DO_LOG, time_and_place)
					 << "Error: got invalid number of entries in line." << std::endl
					 << "Expected " << 3 * m_xDim * m_yDim << ", but got " << v.size() << std::endl
					 << "Note: xDim_ = " << m_xDim << ", yDim_ = " << m_yDim << std::endl
			 );
		 }

		 // Add all pixel data to the canvas
		 std::size_t offset = 0;
		 for (std::size_t line_counter = 0; line_counter < m_yDim; line_counter++) {
			 for (std::size_t pixel_counter = 0; pixel_counter < m_xDim; pixel_counter++) {
				 offset = 3 * (line_counter * m_xDim + pixel_counter);

				 (m_canvasData[pixel_counter][line_counter]).r = float(v[offset + std::size_t(0)]) / float(MAXCOLOR);
				 (m_canvasData[pixel_counter][line_counter]).g = float(v[offset + std::size_t(1)]) / float(MAXCOLOR);
				 (m_canvasData[pixel_counter][line_counter]).b = float(v[offset + std::size_t(2)]) / float(MAXCOLOR);
			 }
		 }
	 }

	 /***************************************************************************/
	 /**
	  * Loads the data held in a file in PPM-P3 format
	  *
	  * @param p The name of a file holding an image in PPM-P3 format
	  */
	 void
	 loadFromFile(bf::path const &p) {
		 // Read in the entire file
		 std::string imageData = Gem::Common::loadTextDataFromFile(p);

		 // Hand the string over to loadFromPPM() -- it will do the rest
#ifdef DEBUG
		 if(imageData.empty()) {
			 throw gemfony_exception(
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
	 void
	 toFile(bf::path const & p) {
		 bf::ofstream result(p);

		 if (not result) {
			 throw gemfony_exception(
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
	 void
	 clear() {
		 m_canvasData.clear();

		 m_xDim = std::size_t(0);
		 m_yDim = std::size_t(0);
	 }

	 /***************************************************************************/
	 /**
	  * Resets the canvas to a given color and dimension
	  *
	  * @param
	  */
	 void
	 reset(
		 std::tuple<std::size_t, std::size_t> const& dimension
		 , float red
		 , float green
		 , float blue
	 ) {
		 this->clear();

		 m_xDim = std::get<0>(dimension);
		 m_yDim = std::get<1>(dimension);

		 for (std::size_t i = 0; i < m_xDim; i++) {
			 m_canvasData.push_back(GColumn(m_yDim, std::tuple<float, float, float>(red, green, blue)));
		 }
	 }

	 /***************************************************************************/
	 /**
	  * Resets the canvas to a given color and dimension
	  */
	 void
	 reset(
		 std::tuple<std::size_t, std::size_t> const& dimension
		 , std::tuple<float, float, float> const& color
	 ) {
		 this->reset(
			 dimension
			 , std::get<0>(color)
			 , std::get<1>(color)
			 , std::get<2>(color)
		 );
	 }

	 /***************************************************************************/
	 /**
	  * Adds a triangle to the canvas, using Gemfony's "circular" definition
	  */
	 void
	 addTriangle(t_circle const & t) {
		 t_cart t_c;

#ifdef DEBUG
		 // Check that angles are in consecutive order
		 if(t.angle1 < 0.f || t.angle2 <= t.angle1 || t.angle3 <= t.angle2 || t.angle3 >= 1.f) {
			 throw gemfony_exception(
				 g_error_streamer(DO_LOG, time_and_place)
					 << "In GCanvas<>::addTriangel(): Error!" << std::endl
					 << "Angles are not in consecutive oder: " << std::endl
					 << t << std::endl
			 );
		 }
#endif /* DEBUG */

		 // and store them in the structs holding the cartesic coordinates
		 t_c.tr_one.x = t.middle.x + t.radius * gcos(t.angle1 * 2.0f * boost::math::constants::pi<float>());
		 t_c.tr_one.y = t.middle.y + t.radius * gsin(t.angle1 * 2.0f * boost::math::constants::pi<float>());

		 t_c.tr_two.x = t.middle.x + t.radius * gcos(t.angle2 * 2.0f * boost::math::constants::pi<float>());
		 t_c.tr_two.y = t.middle.y + t.radius * gsin(t.angle2 * 2.0f * boost::math::constants::pi<float>());

		 t_c.tr_three.x = t.middle.x + t.radius * gcos(t.angle3 * 2.0f * boost::math::constants::pi<float>());
		 t_c.tr_three.y = t.middle.y + t.radius * gsin(t.angle3 * 2.0f * boost::math::constants::pi<float>());

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
	 void
	 addTriangles(std::vector<t_circle> const & ts) {
	 	 for(auto const & t: ts) {
			 this->addTriangle(t);
	 	 }
	 }

	 /***************************************************************************/
	 /**
	  * Adds a triangle to the canvas, using a struct holding cartesic coordinates
	  */
	 void
	 addTriangle(t_cart const & t) {
		 using namespace Gem::Common;

		 float xDim_inv = 1.f / float(m_xDim);
		 float yDim_inv = 1.f / float(m_yDim);
		 float dot11, dot12, dot22, dot1p, dot2p, denom_inv, u, v;
		 coord2D diff31, diff21, diffp1, pos_f;

		 for (std::size_t i_x = 0; i_x < m_xDim; i_x++) {
			 // Calculate the pixel x-position
			 pos_f.x = float(i_x + 1) * xDim_inv;

			 if (
				 pos_f.x < t.tr_one.x
				 && pos_f.x < t.tr_two.x
				 && pos_f.x < t.tr_three.x
				 ) {
				 continue;
			 }

			 if (
				 pos_f.x > t.tr_one.x
				 && pos_f.x > t.tr_two.x
				 && pos_f.x > t.tr_three.x
				 ) {
				 continue;
			 }

			 for (std::size_t i_y = 0; i_y < m_yDim; i_y++) {
				 // Calculate the pixel y-position
				 pos_f.y = float(i_y + 1) * yDim_inv;

				 if (
					 pos_f.y < t.tr_one.y
					 && pos_f.y < t.tr_two.y
					 && pos_f.y < t.tr_three.y
					 ) {
					 continue;
				 }

				 if (
					 pos_f.y > t.tr_one.y
					 && pos_f.y > t.tr_two.y
					 && pos_f.y > t.tr_three.y
					 ) {
					 continue;
				 }

				 diff31 = t.tr_three - t.tr_one;
				 diff21 = t.tr_two - t.tr_one;
				 diffp1 = pos_f - t.tr_one;

				 dot11 = diff31 * diff31;
				 dot12 = diff31 * diff21;
				 dot22 = diff21 * diff21;
				 dot1p = diff31 * diffp1;
				 dot2p = diff21 * diffp1;

				 denom_inv = 1.f / gmax(dot11 * dot22 - dot12 * dot12, 0.0000001f);

				 u = (dot22 * dot1p - dot12 * dot2p) * denom_inv;
				 v = (dot11 * dot2p - dot12 * dot1p) * denom_inv;

				 if ((u >= 0.f) && (v >= 0.f) && (u + v < 1.f)) {
					 m_canvasData[i_x][i_y].r = gmix(m_canvasData[i_x][i_y].r, t.r, t.a);
					 m_canvasData[i_x][i_y].g = gmix(m_canvasData[i_x][i_y].g, t.g, t.a);
					 m_canvasData[i_x][i_y].b = gmix(m_canvasData[i_x][i_y].b, t.b, t.a);
				 }
			 }
		 }
	 }

	 /***************************************************************************/
	 /**
	  * Calculates the average colors over all pixels
	  */
	 auto
	 getAverageColors() const {
		 float averageRed = 0.f;
		 float averageGreen = 0.f;
		 float averageBlue = 0.f;

		 for (std::size_t i_x = 0; i_x < m_xDim; i_x++) {
			 for (std::size_t i_y = 0; i_y < m_yDim; i_y++) {
				 averageRed += m_canvasData[i_x][i_y].r;
				 averageGreen += m_canvasData[i_x][i_y].g;
				 averageBlue += m_canvasData[i_x][i_y].b;
			 }
		 }

		 averageRed /= (float) (m_xDim * m_yDim);
		 averageGreen /= (float) (m_xDim * m_yDim);
		 averageBlue /= (float) (m_xDim * m_yDim);

		 return std::tuple<float, float, float>{averageRed, averageGreen, averageBlue};
	 }

	 /***************************************************************************/
	 // Converts the three angles

	 /***************************************************************************/

protected:
	 std::size_t m_xDim = 0, m_yDim = 0; ///< The dimensions of this canvas
	 std::vector<GColumn> m_canvasData; ///< Holds this canvas' columns

	 std::size_t NCOLORS = Gem::Common::PowSmallPosInt<2, COLORDEPTH>::result;
	 std::size_t MAXCOLOR = NCOLORS - 1;
};

/******************************************************************************/
/**
  * Convenience function for the calculation of the difference between two canvasses
  */
template<std::size_t COLORDEPTH>
G_API_COMMON
float
operator-(
	GCanvas<COLORDEPTH> const & x
	, GCanvas<COLORDEPTH> const & y
) {
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

	 template<typename Archive>
	 void serialize(Archive &ar, const unsigned int) {
		 using boost::serialization::make_nvp;

		 ar &BOOST_SERIALIZATION_BASE_OBJECT_NVP(GCanvas<8>);
	 }
	 ///////////////////////////////////////////////////////////////////////

public:
	 /** @brief Initialization with dimensions and colors */
	 G_API_COMMON GCanvas8(
		 std::tuple<std::size_t, std::size_t> const&
		 , std::tuple<float, float, float> const&
	 );
	 /** @brief Initialization from data held in a string -- uses the PPM-P3 format */
	 explicit G_API_COMMON GCanvas8(std::string const &);
	 /** @brief Copy construction */

	//----------------------------------------------------
	// Defaulted constructors and destructors
	// Rule of five ...

	GCanvas8() = default;
	GCanvas8(GCanvas8 const &) = default;
	GCanvas8(GCanvas8 &&) = default;
	virtual ~GCanvas8() override = default;

	GCanvas8 & operator=(GCanvas8 const &) = default;
	GCanvas8 & operator=(GCanvas8 &&) = default;

	//-----------------------------------------------------

};

/** @brief Convenience function for the calculation of the difference between two canvasses */
G_API_COMMON float operator-(GCanvas8 const &, GCanvas8 const &);

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/
/**
 * Specialization of GCanvas for a color depth of 16 bits
 */
class GCanvas16 : public GCanvas<16>
{
	///////////////////////////////////////////////////////////////////////
	friend class boost::serialization::access;

	template<typename Archive>
	void serialize(Archive &ar, const unsigned int) {
		using boost::serialization::make_nvp;

		ar
		& BOOST_SERIALIZATION_BASE_OBJECT_NVP(GCanvas<16>);
	}
	///////////////////////////////////////////////////////////////////////

public:
	/** @brief Initialization with dimensions and colors */
	G_API_COMMON GCanvas16(
		std::tuple<std::size_t, std::size_t> const &
		, std::tuple<float, float, float> const &
	);

	/** @brief Initialization from data held in a string -- uses the PPM-P3 format */
	explicit G_API_COMMON GCanvas16(std::string const &);

	//----------------------------------------------------
	// Defaulted constructors and destructors
	// Rule of five ...

	GCanvas16() = default;
	GCanvas16(GCanvas16 const &) = default;
	GCanvas16(GCanvas16 &&) = default;

	virtual ~GCanvas16() override = default;

	GCanvas16 &operator=(GCanvas16 const &) = default;
	GCanvas16 &operator=(GCanvas16 &&) = default;

	//-----------------------------------------------------
};

/** @brief Convenience function for the calculation of the difference between two canvasses */
G_API_COMMON float operator-(
	GCanvas16 const &
	, GCanvas16 const &
);

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/
/**
 * Specialization of GCanvas for a color depth of 24 bits
 */
class GCanvas24 : public GCanvas<24> {
	 ///////////////////////////////////////////////////////////////////////
	 friend class boost::serialization::access;

	 template<typename Archive>
	 void serialize(Archive &ar, const unsigned int) {
		 using boost::serialization::make_nvp;

		 ar &BOOST_SERIALIZATION_BASE_OBJECT_NVP(GCanvas<24>);
	 }
	 ///////////////////////////////////////////////////////////////////////

public:
	 /** @brief Initialization with dimensions and colors */
	 G_API_COMMON GCanvas24(
		 std::tuple<std::size_t, std::size_t> const&
		 , std::tuple<float, float, float> const&
	 );
	 /** @brief Initialization from data held in a string -- uses the PPM-P3 format */
	 explicit G_API_COMMON GCanvas24(std::string const &);

	//----------------------------------------------------
	// Defaulted constructors and destructors
	// Rule of five ...

	GCanvas24() = default;
	GCanvas24(GCanvas24 const &) = default;
	GCanvas24(GCanvas24 &&) = default;

	virtual ~GCanvas24() override = default;

	GCanvas24 &operator=(GCanvas24 const &) = default;
	GCanvas24 &operator=(GCanvas24 &&) = default;

	//-----------------------------------------------------
};

/** @brief Convenience function for the calculation of the difference between two canvasses */
G_API_COMMON float operator-(
	GCanvas24 const &
	, GCanvas24 const &
);

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/

} /* namespace Common */
} /* namespace Gem */

BOOST_CLASS_EXPORT_KEY(Gem::Common::GRgb)
BOOST_CLASS_EXPORT_KEY(Gem::Common::GColumn)
BOOST_CLASS_EXPORT_KEY(Gem::Common::GCanvas8)
BOOST_CLASS_EXPORT_KEY(Gem::Common::GCanvas16)
BOOST_CLASS_EXPORT_KEY(Gem::Common::GCanvas24)
