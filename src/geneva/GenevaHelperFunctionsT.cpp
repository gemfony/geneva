/**
 * @file GenevaHelperFunctionsT.cpp
 */

/*
 * Copyright (C) Authors of the Geneva library collection and Karlsruhe
 * Institute of Technology (University of the State of Baden-Wuerttemberg
 * and National Laboratory of the Helmholtz Association).
 *
 * See the AUTHORS file in the top-level directory for a list of authors.
 *
 * Contact: info [at] gemfony (dot) com
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

#include "geneva/GenevaHelperFunctionsT.hpp"
#include "geneva/GDoubleGaussAdaptor.hpp"
#include "geneva/GInt32FlipAdaptor.hpp"
#include "geneva/GBooleanAdaptor.hpp"

namespace Gem
{
namespace Geneva
{

/**************************************************************************************************/
/**
 * A factory function that returns the default adaptor for the base type "double"
 *
 * @return The default adaptor for the base type "double"
 */
template <>
boost::shared_ptr<GAdaptorT<double> > getDefaultAdaptor<double>() {
	return boost::shared_ptr<GDoubleGaussAdaptor>(new GDoubleGaussAdaptor());
}

/**************************************************************************************************/
/**
 * A factory function that returns the default adaptor for the base type "boost::int32_t"
 *
 * @return The default adaptor for the base type "boost::int32_t"
 */
template <>
boost::shared_ptr<GAdaptorT<boost::int32_t> > getDefaultAdaptor<boost::int32_t>() {
	return boost::shared_ptr<GInt32FlipAdaptor>(new GInt32FlipAdaptor());
}

/**************************************************************************************************/
/**
 * A factory function that returns the default adaptor for the base type "bool"
 *
 * @return The default adaptor for the base type "bool"
 */
template <>
boost::shared_ptr<GAdaptorT<bool> > getDefaultAdaptor<bool>() {
	return boost::shared_ptr<GBooleanAdaptor>(new GBooleanAdaptor());
}

/**************************************************************************************************/

} /* namespace Geneva */
} /* namespace Gem */
