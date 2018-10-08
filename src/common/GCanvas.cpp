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

#include "common/GCanvas.hpp"

BOOST_CLASS_EXPORT_IMPLEMENT(Gem::Common::GRgb) // NOLINT
BOOST_CLASS_EXPORT_IMPLEMENT(Gem::Common::GColumn) // NOLINT
BOOST_CLASS_EXPORT_IMPLEMENT(Gem::Common::GCanvas8) // NOLINT
BOOST_CLASS_EXPORT_IMPLEMENT(Gem::Common::GCanvas16) // NOLINT
BOOST_CLASS_EXPORT_IMPLEMENT(Gem::Common::GCanvas24) // NOLINT

namespace Gem {
namespace Common {

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/
/**
 * Construction with positions
 */
coord2D::coord2D(float x_pos, float y_pos)
	: x(x_pos)
	, y(y_pos)
{ /* nothing */ }

/******************************************************************************/
/**
 * Convenience function for calculating the difference between two coordinate vectors
 */
coord2D operator-(coord2D const & a, coord2D const & b) {
	return {a.x - b.x, a.y - b.y};
}

/******************************************************************************/
/**
 * Convenience function for calculating the dot product of two coordinate vectors
 */
float operator*(coord2D const & a, coord2D const& b) {
	return a.x * b.x + a.y * b.y;
}

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/
/**
 * Needed for sorting the structs
 */
float triangle_circle_struct::getAlphaValue() const {
	return this->a;
}

/******************************************************************************/
/**
 * Translate to a string
 */
std::string triangle_circle_struct::toString() const {
	return Gem::Common::to_string(*this);
}

/******************************************************************************/
/**
 * Simplify debugging output
 */
std::ostream &operator<<(std::ostream &out, t_circle const &tc) {
	out << std::setprecision(5)
	<< "middle.x = " << tc.middle.x << std::endl
	<< "middle.y = " << tc.middle.y << std::endl
	<< "radius   = " << tc.radius << std::endl
	<< "angle1   = " << tc.angle1 << std::endl
	<< "angle2   = " << tc.angle2 << std::endl
	<< "angle3   = " << tc.angle3 << std::endl
	<< "red      = " << tc.r << std::endl
	<< "green    = " << tc.g << std::endl
	<< "blue     = " << tc.b << std::endl
	<< "alpha    = " << tc.a << std::endl;

	return out;
}

/******************************************************************************/
/**
 * Simplify comparison of two t_circle structs
 */
bool operator==(t_circle const &a, t_circle const &b) {
	if (a.middle.x != b.middle.x) return false;
	if (a.middle.y != b.middle.y) return false;
	if (a.radius != b.radius) return false;
	if (a.angle1 != b.angle1) return false;
	if (a.angle2 != b.angle2) return false;
	if (a.angle3 != b.angle3) return false;
	if (a.r != b.r) return false;
	if (a.g != b.g) return false;
	if (a.b != b.b) return false;
	return a.a == b.a;
}

/******************************************************************************/
/**
 * Simplify comparison of two t_circle structs
 */
bool operator!=(t_circle const &a, t_circle const &b) {
	return !operator==(a, b);
}

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/
/**
 * Initialization with colors
 */
GRgb::GRgb(float red, float green, float blue)
	: r(red)
	, g(green)
	, b(blue)
{ /* nothing */ }

/******************************************************************************/
/**
 * Initialization with colors held in a std::tuple
 */
GRgb::GRgb(std::tuple<float, float, float> const& color)
	: r(std::get<0>(color))
	, g(std::get<1>(color))
	, b(std::get<2>(color))
{ /* nothing */ }

/******************************************************************************/
/**
 * Explicit reset of colors
 */
void GRgb::setColor(
	float red
	, float green
	, float blue
) {
	r = red;
	g = green;
	b = blue;
}

/******************************************************************************/
/**
 * Explicit reset of colors, using a std::tuple
 */
void GRgb::setColor(std::tuple<float, float, float> color) {
	r = std::get<0>(color);
	g = std::get<1>(color);
	b = std::get<2>(color);
}

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/
/**
 * Initialization with dimensions and colors
 */
GColumn::GColumn(
	std::size_t sz
	, std::tuple<float, float, float> const& color
)
	: m_column_data_vec(sz)
{
	for(auto & column_data: m_column_data_vec) {
		column_data.setColor(color);
	}
}

/******************************************************************************/
/**
 * Information about the size of this object
 */
std::size_t GColumn::size() const {
	return m_column_data_vec.size();
}

/******************************************************************************/
/**
 * Unchecked access
 */
GRgb &GColumn::operator[](std::size_t pos) {
	return m_column_data_vec[pos];
}

/******************************************************************************/
/**
 * Checked access
 */
GRgb &GColumn::at(std::size_t pos) {
	return m_column_data_vec.at(pos);
}

/******************************************************************************/
/**
 * Unchecked access
 */
const GRgb &GColumn::operator[](std::size_t pos) const {
	return m_column_data_vec[pos];
}

/******************************************************************************/
/**
 * Checked access
 */
const GRgb &GColumn::at(std::size_t pos) const {
	return m_column_data_vec.at(pos);
}

/******************************************************************************/
/**
 * Initializes the object to a specific size
 */
void GColumn::init(
	std::size_t sz
	, std::tuple<float, float, float> const& color
) {
	m_column_data_vec.clear();
	m_column_data_vec.resize(sz);

	for(auto &column_data: m_column_data_vec) {
		column_data.setColor(color);
	}
}

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/
/**
 * Initialization with dimensions and colors
 */
GCanvas8::GCanvas8(
	std::tuple<std::size_t, std::size_t> const& dim
	, std::tuple<float, float, float> const& color
)
	: GCanvas<8>(dim, color)
{ /* nothing */ }

/******************************************************************************/
/**
 * Initialization from data held in a string -- uses the PPM-P3 format
 */
GCanvas8::GCanvas8(std::string const &ppmData)
	: GCanvas<8>(ppmData)
{ /* nothing */ }

/******************************************************************************/
/**
  * Convenience function for the calculation of the difference between two canvasses
  */
float operator-(GCanvas8 const &x, GCanvas8 const &y) {
	return x.diff(y);
}

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/
/**
 * Initialization with dimensions and colors
 */
GCanvas16::GCanvas16(
	std::tuple<std::size_t, std::size_t> const& dim
	, std::tuple<float, float, float> const& color
)
	: GCanvas<16>(dim, color)
{ /* nothing */ }

/******************************************************************************/
/**
 * Initialization from data held in a string -- uses the PPM-P3 format
 */
GCanvas16::GCanvas16(std::string const & ppmData)
	: GCanvas<16>(ppmData)
{ /* nothing */ }

/******************************************************************************/
/**
  * Convenience function for the calculation of the difference between two canvasses
  */
float operator-(GCanvas16 const &x, GCanvas16 const &y) {
	return x.diff(y);
}

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/
/**
 * Initialization with dimensions and colors
 */
GCanvas24::GCanvas24(
	std::tuple<std::size_t, std::size_t> const& dim
	, std::tuple<float, float, float> const& color
)
	: GCanvas<24>(dim, color)
{ /* nothing */ }

/******************************************************************************/
/**
 * Initialization from data held in a string -- uses the PPM-P3 format
 */
GCanvas24::GCanvas24(std::string const & ppmData)
	: GCanvas<24>(ppmData)
{ /* nothing */ }

/******************************************************************************/
/**
  * Convenience function for the calculation of the difference between two canvasses
  */
float operator-(GCanvas24 const & x, GCanvas24 const & y) {
	return x.diff(y);
}

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/

} /* namespace Common */
} /* namespace Gem */
