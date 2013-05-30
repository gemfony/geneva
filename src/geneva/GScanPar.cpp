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
 * Returns a set of boolean data items
 */
template <>
std::vector<bool> fillWithData<bool>(
      std::size_t nSteps
      , bool      lower
      , bool      upper
      , bool      randomFill
) {
   std::vector<bool> result;

   if(randomFill) { // Fill with the expected number of random entries
      Gem::Hap::GRandom gr;
      for(std::size_t i=0; i<nSteps; i++) {
         result.push_back(gr.uniform_bool());
      }
   } else { // Just the two allowed values
      result.push_back(false);
      result.push_back(true);
   }

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
      , bool           randomFill
) {
   std::vector<boost::int32_t> result;

   if(randomFill) { // Fill with the expected number of random entries
      Gem::Hap::GRandom gr;
      for(std::size_t i=0; i<nSteps; i++) {
         result.push_back(gr.uniform_int<boost::int32_t>(lower, upper+1)); // uniform_int excludes the upper boundary
      }
   } else {
      for(boost::int32_t i=lower; i<=upper; i++) {
         result.push_back(i);
      }
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
      , bool      randomFill
) {
   std::vector<float> result;

   if(randomFill) {
      for(std::size_t i=0; i<nSteps; i++) {
         Gem::Hap::GRandom gr;
         result.push_back(gr.uniform_real<float>(lower, upper));
      }
   } else {
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
      , bool      randomFill
) {
   std::vector<double> result;

   if(randomFill) {
      Gem::Hap::GRandom gr;
      for(std::size_t i=0; i<nSteps; i++) {
         result.push_back(gr.uniform_real<double>(lower, upper));
      }
   } else {
      // We require at least 2 steps, unless we are are in random mode
      if(nSteps<2) {
         glogger
         << "In std::vector<float> fillWithData<float>(): Error!" << std::endl
         << "Number of requested steps is too low: " << nSteps << std::endl
         << GEXCEPTION;
      }

      for(std::size_t i=0; i<nSteps; i++) {
         result.push_back(lower + (upper-lower)*double(i)/double(nSteps-1));
      }
   }

   return result;
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
    std::size_t p
    , std::size_t nS
    , bool l
    , bool u
    , bool s
)
   : baseScanParT<bool>(p,nS,l,u,s,"b")
{ /* nothing */ }

/******************************************************************************/
/**
 * Construction from the specification string
 */
bScanPar::bScanPar(
      const std::string& desc
      , bool s
)
   : baseScanParT<bool>(desc, s)
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
    std::size_t p
    , std::size_t nS
    , boost::int32_t l
    , boost::int32_t u
    , bool s
)
   : baseScanParT<boost::int32_t>(p,nS,l,u,s,"i")
{ /* nothing */ }

/******************************************************************************/
/**
 * Construction from the specification string
 */
int32ScanPar::int32ScanPar(
      const std::string& desc
      , bool s
)
   : baseScanParT<boost::int32_t>(desc,s)
{
   // Check that we are indeed dealing with a boolean description
   if(this->typeDescription != "i") {
      glogger
      << "int32ScanPar::int32ScanPar(): Error!" << std::endl
      << "Not a description of a boost::int32_t parameter:" << std::endl
      << "\"" << desc << "\"" << std::endl
      << GTERMINATION;
   }
}

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
    std::size_t p
    , std::size_t nS
    , double l
    , double u
    , bool s
)
   : baseScanParT<double>(p,nS,l,u,s,"d")
{ /* nothing */ }

/******************************************************************************/
/**
 * Construction from the specification string
 */
dScanPar::dScanPar(
      const std::string& desc
      , bool s
)
   : baseScanParT<double>(desc,s)
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
    std::size_t p
    , std::size_t nS
    , float l
    , float u
    , bool s
)
   : baseScanParT<float>(p,nS,l,u,s,"f")
{ /* nothing */ }

/******************************************************************************/
/**
 * Construction from the specification string
 */
fScanPar::fScanPar(
      const std::string& desc
      , bool s
)
   : baseScanParT<float>(desc,s)
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
