/**
 * @file GScanPar.cpp
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
 * Returns a set of boolean data items
 */
template <>
std::vector<bool> fillWithData<bool>(
      std::size_t nSteps
      , bool      lower
      , bool      upper
) {
   std::vector<bool> result;
   result.push_back(false);
   result.push_back(true);
   return result;
}

/******************************************************************************/
/**
 * Returns a set of boost::int32_t data items
 */
template <>
std::vector<boost::int32_t> fillWithData<boost::int32_t>(
      std::size_t      nSteps // will only be used for random entries
      , boost::int32_t lower
      , boost::int32_t upper // inclusive
) {
   std::vector<boost::int32_t> result;
   for(boost::int32_t i=lower; i<=upper; i++) {
      result.push_back(i);
   }
   return result;
}

/******************************************************************************/
/**
 * Returns a set of float data items
 */
template <>
std::vector<float> fillWithData<float>(
      std::size_t nSteps
      , float     lower
      , float     upper
) {
   std::vector<float> result;

   // We require at least 2 steps, unless we are are in random mode
   if(nSteps<2) {
      glogger
      << "In std::vector<float> fillWithData<float>(): Error!" << std::endl
      << "Number of reqsted steps is too low: " << nSteps << std::endl
      << GEXCEPTION;
   }

   for(std::size_t i=0; i<nSteps; i++) {
      result.push_back(lower + (upper-lower)*float(i)/float(nSteps-1));
   }

   return result;
}

/******************************************************************************/
/**
 * Returns a set of double data items
 */
template <>
std::vector<double> fillWithData<double>(
      std::size_t nSteps
      , double    lower
      , double    upper
) {
   std::vector<double> result;

   // We require at least 2 steps, unless we are are in random mode
   if(nSteps<2) {
      glogger
      << "In std::vector<float> fillWithData<double>(): Error!" << std::endl
      << "Number of requested steps is too low: " << nSteps << std::endl
      << GEXCEPTION;
   }

   for(std::size_t i=0; i<nSteps; i++) {
      result.push_back(lower + (upper-lower)*double(i)/double(nSteps-1));
   }

   return result;
}

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/
/**
 * Retrieval of a random value for type bool
 */
template <> bool baseScanParT<bool>::getRandomItem() const {
   return gr_.uniform_bool();
}

/******************************************************************************/
/**
 * Retrieval of a random value for type boost::int32_t
 */
template <> boost::int32_t baseScanParT<boost::int32_t>::getRandomItem() const {
   return gr_.uniform_int<boost::int32_t>(lower_, upper_+1);
}

/******************************************************************************/
/**
 * Retrieval of a random value for type float
 */
template <> float baseScanParT<float>::getRandomItem() const {
   return gr_.uniform_real<float>(lower_, upper_);
}

/******************************************************************************/
/**
 * Retrieval of a random value for type double
 */
template <> double baseScanParT<double>::getRandomItem() const {
   return gr_.uniform_real<double>(lower_, upper_);
}

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
   parPropSpec<bool> pps
   , bool randomScan
)
   : baseScanParT<bool>(pps, randomScan,"b")
{ /* nothing */ }

/******************************************************************************/
/**
 * The copy constructor
 */
bScanPar::bScanPar(const bScanPar& cp)
   :baseScanParT<bool>(cp)
{ /* nothing */ }

/******************************************************************************/
/**
 * The destructor
 */
bScanPar::~bScanPar()
{ /* nothing */ }

/******************************************************************************/
/**
 * Cloning of this object
 */
boost::shared_ptr<bScanPar> bScanPar::clone() const {
   return boost::shared_ptr<bScanPar>(new bScanPar(*this));
}

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/
/**
 * The default constructor. Only needed for de-serialization.
 */
int32ScanPar::int32ScanPar()
   : baseScanParT<boost::int32_t>()
{ /* nothing */ }

/******************************************************************************/
/**
 * The standard destructor
 */
int32ScanPar::int32ScanPar(
   parPropSpec<boost::int32_t> pps
   , bool randomScan
)
   : baseScanParT<boost::int32_t>(pps, randomScan, "i")
{ /* nothing */ }

/******************************************************************************/
/**
 * The copy constructor
 */
int32ScanPar::int32ScanPar(const int32ScanPar& cp)
   : baseScanParT<boost::int32_t>(cp)
{ /* nothing */ }

/******************************************************************************/
/**
 * The destructor
 */
int32ScanPar::~int32ScanPar()
{ /* nothing */ }

/******************************************************************************/
/**
 * Cloning
 */
boost::shared_ptr<int32ScanPar> int32ScanPar::clone() const {
   return boost::shared_ptr<int32ScanPar>(new int32ScanPar(*this));
}

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/
/**
 * The default constructor. Only needed for de-serialization.
 */
dScanPar::dScanPar()
   : baseScanParT<double>()
{ /* nothing */ }

/******************************************************************************/
/**
 * The standard destructor
 */
dScanPar::dScanPar(
   parPropSpec<double> pps
   , bool randomScan
)
   : baseScanParT<double>(pps, randomScan, "d")
{ /* nothing */ }

/******************************************************************************/
/**
 * The copy constructor
 */
dScanPar::dScanPar(const dScanPar& cp)
   : baseScanParT<double>(cp)
{ /* nothing */ }

/******************************************************************************/
/**
 * The destructor
 */
dScanPar::~dScanPar()
{ /* nothing */ }

/******************************************************************************/
/**
 * Cloning
 */
boost::shared_ptr<dScanPar> dScanPar::clone() const {
   return boost::shared_ptr<dScanPar>(new dScanPar(*this));
}

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/
/**
 * The default constructor. Only needed for de-serialization.
 */
fScanPar::fScanPar()
   : baseScanParT<float>()
{ /* nothing */ }

/******************************************************************************/
/**
 * The standard destructor
 */
fScanPar::fScanPar(
      parPropSpec<float> pps
      , bool randomScan
)
   : baseScanParT<float>(pps, randomScan, "f")
{ /* nothing */ }

/******************************************************************************/
/**
 * The copy constructor
 */
fScanPar::fScanPar(const fScanPar& cp)
   : baseScanParT<float>(cp)
{ /* nothing */ }

/******************************************************************************/
/**
 * The destructor
 */
fScanPar::~fScanPar()
{ /* nothing */ }

/******************************************************************************/
/**
 * Cloning
 */
boost::shared_ptr<fScanPar> fScanPar::clone() const {
   return boost::shared_ptr<fScanPar>(new fScanPar(*this));
}

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/

} /* namespace Geneva */
} /* namespace Gem */
