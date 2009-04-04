/**
 * @file GParameterT.cpp
 */

/* Copyright (C) 2009 Dr. Ruediger Berlich
 *
 * This file is part of Geneva, Gemfony scientific's optimization library.
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
 */

#include "GParameterT.hpp"

namespace Gem
{
namespace GenEvA
{

/***********************************************************************************************/
/**
 * Checks similarity of this object, if typeof(T) == typeof(double)
 *
 * @param cp A constant reference to another GParameterT<double> object
 * @param limit The acceptable difference between two double values
 */
template<> bool GParameterT<double>::isSimilarTo(const GParameterT<double>& cp, const double& limit) const {
	// Check similarity of the parent class
	if(!GParameterBaseWithAdaptorsT<double>::isSimilarTo(cp)) return false;

	// Check the local data
	if(fabs(val_  - cp.val_) > fabs(limit)) return false;

	return true;
}

/***********************************************************************************************/
/**
 * A default constructor for typeof(T)==typeof(bool), needed due to call to GRandom::boolRandom()
 * in the initialization code.
 */
template <> GParameterT<bool>::GParameterT()
	:GParameterBaseWithAdaptorsT<bool>(),
	  val_(gr.boolRandom())
{ /* nothing */ }

/***********************************************************************************************/
/**
  * A default constructor for typeof(T)==typeof(bool), needed due to call to GRandom::boolRandom()
  * in the initialization code.
  */
template <> GParameterT<char>::GParameterT()
   :GParameterBaseWithAdaptorsT<char>(),
     val_(gr.charRandom())
{ /* nothing */ }

/***********************************************************************************************/
/**
 * A default constructor for typeof(T)==typeof(boost::int32_t), needed due to call to GRandom::discreteRandom()
 * in the initialization code.
 */
template <> GParameterT<boost::int32_t>::GParameterT()
	:GParameterBaseWithAdaptorsT<boost::int32_t>(),
	 val_(gr.discreteRandom<boost::int32_t>())
{ /* nothing */ }

/***********************************************************************************************/

} /* namespace GenEvA */
} /* namespace Gem */
