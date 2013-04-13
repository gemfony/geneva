/**
 * @file GScanPar.cpp
 */

/*
 * Copyright (C) Gemfony scientific UG (haftungsbeschraenkt)
 *
 * See the AUTHORS file in the top-level directory for a list of authors.
 *
 * Contact: contact [at] gemfony (dot) com
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
 * http://www.gemfony.com .
 */

#include "geneva/GScanPar.hpp"

BOOST_CLASS_EXPORT_IMPLEMENT(Gem::Geneva::bScanPar)
BOOST_CLASS_EXPORT_IMPLEMENT(Gem::Geneva::int32ScanPar)
BOOST_CLASS_EXPORT_IMPLEMENT(Gem::Geneva::dScanPar)
BOOST_CLASS_EXPORT_IMPLEMENT(Gem::Geneva::fScanPar)

namespace Gem {
namespace Geneva {

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/
/**
 * The default constructor. Only needed for de-serialization.
 */
bScanPar::bScanPar()
   : baseScanParT<bool>()
{ /* nothing */ }

/******************************************************************************/
/**
 * Construction from local variables
 */
bScanPar::bScanPar(
    std::size_t p
    , std::size_t nS
    , bool l
    , bool u
)
   : baseScanParT(p,nS, l, u)
{ /* nothing */ }

/******************************************************************************/
/**
 * Construction from the specification string
 */
bScanPar::bScanPar(const std::string& desc)
   : baseScanParT<bool>(desc)
{
   if(this->typeDescription != "b") {
      glogger
      << "In bScanPar::bScanPar(const std::string& desc): Error!" << std::endl
      << "Got invalid parameter description:" << std::endl
      << desc
      << GTERMINATION;
   }
}

/******************************************************************************/
/**
 * The destructor
 */
bScanPar::~bScanPar()
{ /* nothing */ }

/******************************************************************************/
/**
 * Retrieves the next value in the chain
 */
bool bScanPar::getNextValue(
      std::size_t cv
      , bool random
) const {
   if(random) {
      return gr.uniform_bool();
   } else {
      if(cv%2 == 0) return false;
      else return true;
   }
}

/******************************************************************************/
/**
 * Retrieves the maximum number of steps for a given type if not in random mode
 */
std::size_t bScanPar::getMaxSteps() const {
   return 2; // There only are two boolean values
}

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/
/**
 * The default constructor. Only needed for de-serialization.
 */
int32ScanPar::int32ScanPar()
   : iScanParT<boost::int32_t>()
{ /* nothing */ }

/******************************************************************************/
/**
 * The standard destructor
 */
int32ScanPar::int32ScanPar(
    std::size_t p
    , std::size_t nS
    , boost::int32_t l
    , boost::int32_t u
)
   : iScanParT<boost::int32_t>(p,nS,l,u)
{ /* nothing */ }

/******************************************************************************/
/**
 * Construction from the specification string
 */
int32ScanPar::int32ScanPar(const std::string& desc)
   : iScanParT<boost::int32_t>(desc)
{
   // Check that we are indeed dealing with a boolean description
   if(this->typeDescription != "i32") {
      glogger
      << "int32ScanPar::int32ScanPar(): Error!" << std::endl
      << "Not a description of a boost::int32_t parameter:" << std::endl
      << "\"" << desc << "\"" << std::endl
      << GTERMINATION;
   }
}

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/
/**
 * The default constructor. Only needed for de-serialization.
 */
dScanPar::dScanPar()
   : fpScanParT<double>()
{ /* nothing */ }

/******************************************************************************/
/**
 * The standard destructor
 */
dScanPar::dScanPar(
    std::size_t p
    , std::size_t nS
    , double l
    , double u
)
   : fpScanParT<double>(p,nS,l,u)
{ /* nothing */ }

/******************************************************************************/
/**
 * Construction from the specification string
 */
dScanPar::dScanPar(const std::string& desc)
   : fpScanParT<double>(desc)
{
   // Check that we are indeed dealing with a boolean description
   if(this->typeDescription != "d") {
      glogger
      << "dScanPar::dScanPar(): Error!" << std::endl
      << "Not a description of a double parameter:" << std::endl
      << "\"" << desc << "\"" << std::endl
      << GTERMINATION;
   }
}

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/
/**
 * The default constructor. Only needed for de-serialization.
 */
fScanPar::fScanPar()
   : fpScanParT<float>()
{ /* nothing */ }

/******************************************************************************/
/**
 * The standard destructor
 */
fScanPar::fScanPar(
    std::size_t p
    , std::size_t nS
    , float l
    , float u
)
   : fpScanParT<float>(p,nS,l,u)
{ /* nothing */ }

/******************************************************************************/
/**
 * Construction from the specification string
 */
fScanPar::fScanPar(const std::string& desc)
   : fpScanParT<float>(desc)
{
   // Check that we are indeed dealing with a boolean description
   if(this->typeDescription != "f") {
      glogger
      << "fScanPar::fScanPar(): Error!" << std::endl
      << "Not a description of a float parameter:" << std::endl
      << "\"" << desc << "\"" << std::endl
      << GTERMINATION;
   }
}

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/

} /* namespace Geneva */
} /* namespace Gem */
