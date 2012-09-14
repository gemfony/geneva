/**
 * @file GMathHelperFunctionsT.hpp
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

#ifndef GMATHHELPERFUNCTIONST_HPP_
#define GMATHHELPERFUNCTIONST_HPP_

// For Microsoft-compatible compilers
#if defined(_MSC_VER)  &&  (_MSC_VER >= 1020)
#pragma once
#endif

namespace Gem {
namespace Common {

/******************************************************************************/
/**
 * Calculation of pow for small positive integers using template metaprogramming
 */
template<std::size_t B, std::size_t E>
struct PowSmallPosInt
{
    enum {
       result = B*PowSmallPosInt<B,E-1>::result
    };
};

template<std::size_t B>
struct PowSmallPosInt<B,2>
{
    enum {
       result = B*B
    };
};

template<std::size_t B>
struct PowSmallPosInt<B,1>
{
    enum {
       result = B
    };
};

template<std::size_t B>
struct PowSmallPosInt<B,0>
{
    enum {
       result = 1
    };
};

/******************************************************************************/

} /* namespace Common */
} /* namespace Gem */

#endif /* GMATHHELPERFUNCTIONST_HPP_ */
