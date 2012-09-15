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

BOOST_CLASS_EXPORT_IMPLEMENT(Gem::Common::GRgb);
BOOST_CLASS_EXPORT_IMPLEMENT(Gem::Common::GColumn);
BOOST_CLASS_EXPORT_IMPLEMENT(Gem::Common::GCanvas<8>);
BOOST_CLASS_EXPORT_IMPLEMENT(Gem::Common::GCanvas<16>);
BOOST_CLASS_EXPORT_IMPLEMENT(Gem::Common::GCanvas<32>);

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
 * Needed for sorting the structs
 */
float triangle_circle_struct::getAlphaValue() const {
   return a;
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

} /* namespace Common */
} /* namespace Gem */
