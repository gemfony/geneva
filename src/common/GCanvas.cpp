/**
 * @file GCanvas.cpp
 */

/*
 * Copyright (C) Gemfony scientific UG (haftungsbeschraenkt) and Karlsruhe
 * Institute of Technology (University of the State of Baden-Wuerttemberg
 * and National Laboratory of the Helmholtz Association).
 *
 * See the AUTHORS file in the top-level directory for a list of authors.
 *
 * Contact: contact [at] gemfony (dot) com
 *
 * This file is part of the Geneva library collection
 *
 * Geneva is free software: you can redistribute it and/or modify
 * it under the terms of version 3 of the GNU Affero General Public License
 * as published by the Free Software Foundation.
 *
 * Geneva is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with the Geneva library.  If not, see <http://www.gnu.org/licenses/>.
 *
 * For further information on Gemfony scientific and Geneva, visit
 * http://www.gemfony.com .
 */

#include "common/GCanvas.hpp"

BOOST_CLASS_EXPORT_IMPLEMENT(Gem::Common::GRgb)
BOOST_CLASS_EXPORT_IMPLEMENT(Gem::Common::GColumn)
BOOST_CLASS_EXPORT_IMPLEMENT(Gem::Common::GCanvas8)
BOOST_CLASS_EXPORT_IMPLEMENT(Gem::Common::GCanvas16)
BOOST_CLASS_EXPORT_IMPLEMENT(Gem::Common::GCanvas24)
BOOST_CLASS_EXPORT_IMPLEMENT(Gem::Common::GCanvas32)

namespace Gem {
namespace Common {

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/
/**
 * The default constructor
 */
coord2D::coord2D()
   : x(0.f)
   , y(0.f)
{ /* nothing */ }

/******************************************************************************/
/**
 * Construction with positions
 */
coord2D::coord2D(const float& x_pos, const float& y_pos)
   : x(x_pos)
   , y(y_pos)
{ /* nothing */ }

/******************************************************************************/
/**
 * Copy construction
 */
coord2D::coord2D(const coord2D& cp)
   : x(cp.x)
   , y(cp.y)
{ /* nothing */ }

/******************************************************************************/
/**
 * An assignment operator
 */
const coord2D& coord2D::operator=(const coord2D& cp) {
   x = cp.x;
   y = cp.y;

   return *this;
}

/******************************************************************************/
/**
 * Convenience function for calculating the difference between two coordinate vectors
 */
coord2D operator-(const coord2D& a, const coord2D& b) {
   return coord2D(a.x-b.x, a.y-b.y);
}

/******************************************************************************/
/**
 * Convenience function for calculating the dot product of two coordinate vectors
 */
float operator*(const coord2D& a, const coord2D& b) {
   return a.x*b.x + a.y*b.y;
}

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/
/**
 * A simple assignment operator
 */
const triangle_circle_struct& triangle_circle_struct::operator=(
      const triangle_circle_struct& cp
) {
   middle = cp.middle;
   radius = cp.radius;
   angle1 = cp.angle1;
   angle2 = cp.angle2;
   angle3 = cp.angle3;
   r      = cp.r;
   g      = cp.g;
   b      = cp.b;
   a      = cp.a;

   return *this;
}

/******************************************************************************/
/**
 * Needed for sorting the structs
 */
float triangle_circle_struct::getAlphaValue() const {
   return a;
}

/******************************************************************************/
/**
 * Simplify debugging output
 */
std::ostream& operator<<(std::ostream& out, const t_circle& tc) {
   out << std::setprecision(5)
   << "middle.x = " << tc.middle.x << std::endl
   << "middle.y = " << tc.middle.y << std::endl
   << "radius   = " << tc.radius   << std::endl
   << "angle1   = " << tc.angle1   << std::endl
   << "angle2   = " << tc.angle2   << std::endl
   << "angle3   = " << tc.angle3   << std::endl
   << "red      = " << tc.r        << std::endl
   << "green    = " << tc.g        << std::endl
   << "blue     = " << tc.b        << std::endl
   << "alpha    = " << tc.a        << std::endl;

   return out;
}

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/
/**
 * The default constructor. Our colors are initialized with black
 */
GRgb::GRgb() : r(0.f), g(0.f), b(0.f)
{ /* nothing */ }

/******************************************************************************/
/**
 * Initialization with colors
 */
GRgb::GRgb(float red, float green, float blue): r(red), g(green), b(blue)
{ /* nothing */ }

/******************************************************************************/
/**
 * Initialization with colors held in a boost::tuple
 */
GRgb::GRgb(const boost::tuple<float,float,float>& color)
: r(boost::get<0>(color))
, g(boost::get<1>(color))
, b(boost::get<2>(color))
{ /* nothing */ }

/******************************************************************************/
/**
 * Copy Construction
 */
GRgb::GRgb(const GRgb& cp): r(cp.r), g(cp.g), b(cp.b)
{ /* nothing */ }

/******************************************************************************/
/**
 * The destructor
 */
GRgb::~GRgb()
{ /* nothing */ }

/******************************************************************************/
/**
 * Assignment operator
 */
const GRgb& GRgb::operator=(const GRgb& cp) {
   r=cp.r;
   g=cp.g;
   b=cp.b;

   return *this;
}

/******************************************************************************/
/**
 * Explicit reset of colors
 */
void GRgb::setColor(float red, float green, float blue) {
   r=red;
   g=green;
   b=blue;
}

/******************************************************************************/
/**
 * Explicit reset of colors, using a boost::tuple
 */
void GRgb::setColor(const boost::tuple<float, float, float>& color) {
   r=boost::get<0>(color);
   g=boost::get<1>(color);
   b=boost::get<2>(color);
}

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/
/**
 * The default constructor -- intentionally private
 */
GColumn::GColumn()
{ /* nothing */ }

/******************************************************************************/
/**
 * Initialization with dimensions and colors
 */
GColumn::GColumn(
      const std::size_t& sz
      , const boost::tuple<float,float,float>& color
)
   : columnData_(sz)
{
   for(std::size_t i=0; i<sz; i++) {
      columnData_[i].setColor(color);
   }
}

/******************************************************************************/
/**
 * Copy construction
 */
GColumn::GColumn(const GColumn& cp) : columnData_(cp.columnData_)
{ /* nothing */ }

/******************************************************************************/
/**
 * The destructor
 */
GColumn::~GColumn() {
   columnData_.clear();
}

/******************************************************************************/
/**
 * Assignment operator
 *
 * @param cp A copy of another GColumn object
 */
const GColumn& GColumn::operator=(const GColumn& cp) {
   columnData_ = cp.columnData_;
   return *this;
}

/******************************************************************************/
/**
 * Information about the size of this object
 */
std::size_t GColumn::size() const {
   return columnData_.size();
}

/******************************************************************************/
/**
 * Unchecked access
 */
GRgb& GColumn::operator[](const std::size_t& pos) {
   return columnData_[pos];
}

/******************************************************************************/
/**
 * Checked access
 */
GRgb& GColumn::at(const std::size_t& pos) {
   return columnData_.at(pos);
}

/******************************************************************************/
/**
 * Unchecked access
 */
const GRgb& GColumn::operator[](const std::size_t& pos) const {
   return columnData_[pos];
}

/******************************************************************************/
/**
 * Checked access
 */
const GRgb& GColumn::at(const std::size_t& pos) const {
   return columnData_.at(pos);
}

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/
/**
 * The default constructor
 */
GCanvas8::GCanvas8()
   : GCanvas<8>()
{ /* nothing */ }

/******************************************************************************/
/**
 * Initialization with dimensions and colors
 */
GCanvas8::GCanvas8(
      const boost::tuple<std::size_t, std::size_t>& dim
      , const boost::tuple<float,float,float>& color
) : GCanvas<8>(dim,color)
{ /* nothing */ }

/******************************************************************************/
/**
 * Initialization from data held in a string -- uses the PPM-P3 format
 */
GCanvas8::GCanvas8(const std::string& ppmData)
   : GCanvas<8>(ppmData)
{ /* nothing */ }

/******************************************************************************/
/**
 * Copy construction
 */
GCanvas8::GCanvas8(const GCanvas8& cp)
  : GCanvas<8>(cp)
{ /* nothing */ }

/******************************************************************************/
/**
 * The destructor
 */
GCanvas8::~GCanvas8()
{ /* nothing */ }

/******************************************************************************/
/**
 * The assignment operator
 */
const GCanvas8& GCanvas8::operator=(const GCanvas8& cp) {
   GCanvas<8>::operator=(cp);
   return *this;
}

/******************************************************************************/
/**
 * Find out the deviation between this and another canvas
 */
/*
float GCanvas8::diff(const GCanvas<8>& cp) const {
   return GCanvas<8>::diff(cp);
}
*/

/******************************************************************************/
/**
  * Convenience function for the calculation of the difference between two canvasses
  */
float operator-(const GCanvas8& x, const GCanvas8& y) {
   return x.diff(y);
}

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/
/**
 * The default constructor
 */
GCanvas16::GCanvas16()
   : GCanvas<16>()
{ /* nothing */ }

/******************************************************************************/
/**
 * Initialization with dimensions and colors
 */
GCanvas16::GCanvas16(
      const boost::tuple<std::size_t, std::size_t>& dim
      , const boost::tuple<float,float,float>& color
) : GCanvas<16>(dim,color)
{ /* nothing */ }

/******************************************************************************/
/**
 * Initialization from data held in a string -- uses the PPM-P3 format
 */
GCanvas16::GCanvas16(const std::string& ppmData)
   : GCanvas<16>(ppmData)
{ /* nothing */ }

/******************************************************************************/
/**
 * Copy construction
 */
GCanvas16::GCanvas16(const GCanvas16& cp)
  : GCanvas<16>(cp)
{ /* nothing */ }

/******************************************************************************/
/**
 * The destructor
 */
GCanvas16::~GCanvas16()
{ /* nothing */ }

/******************************************************************************/
/**
 * The assignment operator
 */
const GCanvas16& GCanvas16::operator=(const GCanvas16& cp) {
   GCanvas<16>::operator=(cp);
   return *this;
}

/******************************************************************************/
/**
 * Find out the deviation between this and another canvas
 */
/*
float GCanvas16::diff(const GCanvas16& cp) const {
   return GCanvas<16>::diff(cp);
}
*/
/******************************************************************************/
/**
  * Convenience function for the calculation of the difference between two canvasses
  */
float operator-(const GCanvas16& x, const GCanvas16& y) {
   return x.diff(y);
}

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/
/**
 * The default constructor
 */
GCanvas24::GCanvas24()
   : GCanvas<24>()
{ /* nothing */ }

/******************************************************************************/
/**
 * Initialization with dimensions and colors
 */
GCanvas24::GCanvas24(
      const boost::tuple<std::size_t, std::size_t>& dim
      , const boost::tuple<float,float,float>& color
) : GCanvas<24>(dim,color)
{ /* nothing */ }

/******************************************************************************/
/**
 * Initialization from data held in a string -- uses the PPM-P3 format
 */
GCanvas24::GCanvas24(const std::string& ppmData)
   : GCanvas<24>(ppmData)
{ /* nothing */ }

/******************************************************************************/
/**
 * Copy construction
 */
GCanvas24::GCanvas24(const GCanvas24& cp)
  : GCanvas<24>(cp)
{ /* nothing */ }

/******************************************************************************/
/**
 * The destructor
 */
GCanvas24::~GCanvas24()
{ /* nothing */ }

/******************************************************************************/
/**
 * The assignment operator
 */
const GCanvas24& GCanvas24::operator=(const GCanvas24& cp) {
   GCanvas<24>::operator=(cp);
   return *this;
}

/******************************************************************************/
/**
 * Find out the deviation between this and another canvas
 */
/*
float GCanvas24::diff(const GCanvas24& cp) const {
   return GCanvas<24>::diff(cp);
}
*/
/******************************************************************************/
/**
  * Convenience function for the calculation of the difference between two canvasses
  */
float operator-(const GCanvas24& x, const GCanvas24& y) {
   return x.diff(y);
}

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/
/**
 * The default constructor
 */
GCanvas32::GCanvas32()
   : GCanvas<32>()
{ /* nothing */ }

/******************************************************************************/
/**
 * Initialization with dimensions and colors
 */
GCanvas32::GCanvas32(
      const boost::tuple<std::size_t, std::size_t>& dim
      , const boost::tuple<float,float,float>& color
) : GCanvas<32>(dim,color)
{ /* nothing */ }

/******************************************************************************/
/**
 * Initialization from data held in a string -- uses the PPM-P3 format
 */
GCanvas32::GCanvas32(const std::string& ppmData)
   : GCanvas<32>(ppmData)
{ /* nothing */ }

/******************************************************************************/
/**
 * Copy construction
 */
GCanvas32::GCanvas32(const GCanvas32& cp)
  : GCanvas<32>(cp)
{ /* nothing */ }

/******************************************************************************/
/**
 * The destructor
 */
GCanvas32::~GCanvas32()
{ /* nothing */ }

/******************************************************************************/
/**
 * The assignment operator
 */
const GCanvas32& GCanvas32::operator=(const GCanvas32& cp) {
   GCanvas<32>::operator=(cp);
   return *this;
}

/******************************************************************************/
/**
 * Find out the deviation between this and another canvas
 */
/*
float GCanvas32::diff(const GCanvas32& cp) const {
   return GCanvas<32>::diff(cp);
}
*/
/******************************************************************************/
/**
  * Convenience function for the calculation of the difference between two canvasses
  */
float operator-(const GCanvas32& x, const GCanvas32& y) {
   return x.diff(y);
}

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/

} /* namespace Common */
} /* namespace Gem */
