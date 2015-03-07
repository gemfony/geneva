/**
 * @file GScanPar.hpp
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

// Global checks, defines and includes needed for all of Geneva
#include "common/GGlobalDefines.hpp"

// Standard headers go here
#include <vector>

// Boost headers go here
#include <boost/any.hpp>
#include <boost/cstdint.hpp>
#include <boost/mpl/assert.hpp>
#include <boost/type_traits.hpp>
#include <boost/lexical_cast.hpp>

#ifndef GSCANPAR_HPP_
#define GSCANPAR_HPP_

// Geneva headers go here
#include "hap/GRandomT.hpp"
#include "common/GSerializeTupleT.hpp"
#include "geneva/GObject.hpp"
#include "geneva/GStdSimpleVectorInterfaceT.hpp"
#include "geneva/GParameterPropertyParser.hpp"

namespace Gem {
namespace Geneva {

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/
/** Indicates that all possible parameter values have been explored */
class g_end_of_par : public std::exception { /* nothing */ };

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/
/**
 * This function fills a given std::vector<T> with items. It needs to be re-implemented
 * in concrete specializations. This generic function is just a trap.
 */
template <typename T>
std::vector<T> fillWithData(
   std::size_t /*nSteps*/
   , T /* lower */
   , T /* upper */
) {
   glogger
   << "In generic function template <typename T> std::vector<T> fillWithData(): Error!" << std::endl
   << "This function should never be called directly. Use one of the specializations." << std::endl
   << GEXCEPTION;

   // Make the compiler happy
   return std::vector<T>();
}

template <> G_API_GENEVA std::vector<bool> fillWithData<bool>(
   std::size_t nSteps
   , bool      lower
   , bool      upper
);

template <> G_API_GENEVA std::vector<boost::int32_t> fillWithData<boost::int32_t>(
   std::size_t      nSteps // will only be used for random entries
   , boost::int32_t lower
   , boost::int32_t upper // inclusive
);

template <> G_API_GENEVA std::vector<float> fillWithData<float>(
   std::size_t nSteps
   , float     lower
   , float     upper
);

template <> G_API_GENEVA std::vector<double> fillWithData<double>(
   std::size_t nSteps
   , double    lower
   , double    upper
);

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/
/**
 * An interface class for parameter scan objects
 */
class scanParInterface {
public:
   virtual G_API_GENEVA ~scanParInterface(){ /* nothing */ }

   virtual G_API_GENEVA NAMEANDIDTYPE getVarAddress() const = 0;
   virtual G_API_GENEVA bool goToNextItem() = 0;
   virtual G_API_GENEVA bool isAtTerminalPosition() const = 0;
   virtual G_API_GENEVA bool isAtFirstPosition() const = 0;
   virtual G_API_GENEVA void resetPosition() = 0;
   virtual G_API_GENEVA std::string getTypeDescriptor() const = 0;
};

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/
/**
 * Basic parameter functionality
 */
template <typename T>
class baseScanParT
   : public GStdSimpleVectorInterfaceT<T>
   , public scanParInterface
{
   ///////////////////////////////////////////////////////////////////////
   friend class boost::serialization::access;

   template<typename Archive>
   void serialize(Archive & ar, const unsigned int) {
      using boost::serialization::make_nvp;

      ar
      & BOOST_SERIALIZATION_BASE_OBJECT_NVP(GStdSimpleVectorInterfaceT<T>)
      & BOOST_SERIALIZATION_NVP(var_)
      & BOOST_SERIALIZATION_NVP(step_)
      & BOOST_SERIALIZATION_NVP(nSteps_)
      & BOOST_SERIALIZATION_NVP(lower_)
      & BOOST_SERIALIZATION_NVP(upper_)
      & BOOST_SERIALIZATION_NVP(randomScan_)
      & BOOST_SERIALIZATION_NVP(typeDescription_);
   }

   ///////////////////////////////////////////////////////////////////////

public:
   /***************************************************************************/
   /**
    * The standard constructor
    */
   baseScanParT(
      parPropSpec<T> pps
      , bool randomScan
      , std::string t // typeDescription_
   )
      : GStdSimpleVectorInterfaceT<T>()
      , var_(pps.var)
      , step_(0)
      , nSteps_(pps.nSteps)
      , lower_(pps.lowerBoundary)
      , upper_(pps.upperBoundary)
      , randomScan_(randomScan)
      , typeDescription_(t)
   {
      if(!randomScan_) {
         // Fill the object with data
         this->data = fillWithData<T>(nSteps_, lower_, upper_);
      }
   }

   /***************************************************************************/
   /**
    * The copy constructor
    */
   baseScanParT(const baseScanParT<T>& cp)
      : GStdSimpleVectorInterfaceT<T>(cp)
      , var_(cp.var_)
      , step_(cp.step_)
      , nSteps_(cp.nSteps_)
      , lower_(cp.lower_)
      , upper_(cp.upper_)
      , randomScan_(cp.randomScan_)
      , typeDescription_(cp.typeDescription_)
   { /* nothing */ }

   /***************************************************************************/
   /**
    * The destructor
    */
   virtual ~baseScanParT()
   { /* nothing */ }

   /***************************************************************************/
   /**
    * Retrieve the address of this object
    */
   virtual NAMEANDIDTYPE getVarAddress() const OVERRIDE {
      return var_;
   }

   /***************************************************************************/
   /**
    * Retrieves the current item position
    */
   std::size_t getCurrentItemPos() const {
      return step_;
   }

   /***************************************************************************/
   /**
    * Retrieve the current item
    */
   T getCurrentItem() const {
      if(randomScan_) {
         return getRandomItem();
      } else {
         return this->at(step_);
      }
   }

   /***************************************************************************/
   /**
    * Switch to the next position in the vector or rewind
    *
    * @return A boolean indicating whether a warp has taken place
    */
   virtual bool goToNextItem() BASE {
      if(++step_ >= nSteps_) {
         step_ = 0;
         return true;
      }
      return false;
   }

   /***************************************************************************/
   /**
    * Checks whether step_ points to the last item in the array
    */
   virtual bool isAtTerminalPosition() const BASE {
      if(step_ >= nSteps_) return true;
      else return false;
   }

   /***************************************************************************/
   /**
    * Checks whether step_ points to the first item in the array
    */
   virtual bool isAtFirstPosition() const BASE {
      if(0 == step_) return true;
      else return false;
   }

   /***************************************************************************/
   /**
    * Resets the current position
    */
   virtual void resetPosition() BASE {
      step_ = 0;
   }

   /***************************************************************************/
   /**
    * Retrieve the type descriptor
    */
   virtual std::string getTypeDescriptor() const BASE {
      return typeDescription_;
   }

protected:
   /***************************************************************************/
   // Data

   NAMEANDIDTYPE var_; ///< Name and/or position of the variable
   std::size_t step_; ///< The current position in the data vector
   std::size_t nSteps_; ///< The number of stepts to be taken in a scan
   T lower_; ///< The lower boundary of an item
   T upper_; ///< The upper boundary of an item
   bool randomScan_; ///< Indicates whether we are dealing with a random scan or not
   std::string typeDescription_; ///< Holds an identifier for the type described by this class

   mutable Gem::Hap::GRandom gr_; ///< Simple access to a random number generator

   /***************************************************************************/
   /** @brief The default constructor -- only needed for de-serialization, hence protected */
   baseScanParT()
   : var_(NAMEANDIDTYPE(0, "empty", 0))
   , step_(0)
   , nSteps_(2)
   , lower_(T(0))
   , upper_(T(1))
   , randomScan_(true)
   , typeDescription_("")
   { /* nothing */ }

   /***************************************************************************/
   /** @brief Needs to be re-implemented for derivatives of GStdSimpleVectorInterfaceT<> */
   virtual void dummyFunction(){};

   /***************************************************************************/
   /**
    * Retrieves a random item. To be re-implemented for each supported type
    */
   T getRandomItem() const {
      // A trap. This function needs to be re-implemented for each supported type
      glogger
      << "In baseScanParT::getRandomItem(): Error!" << std::endl
      << "Function called for unsupported type" << std::endl
      << GEXCEPTION;

      // Make the compiler happy
      return T(0);
   }
};

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/
/**
 * Retrieval of a random value for type bool
 */
template <>
inline bool baseScanParT<bool>::getRandomItem() const {
   return gr_.uniform_bool();
}

/******************************************************************************/
/**
 * Retrieval of a random value for type boost::int32_t
 */
template <>
inline boost::int32_t baseScanParT<boost::int32_t>::getRandomItem() const {
   return gr_.uniform_int<boost::int32_t>(lower_, upper_+1);
}

/******************************************************************************/
/**
 * Retrieval of a random value for type float
 */
template <>
inline float baseScanParT<float>::getRandomItem() const {
   return gr_.uniform_real<float>(lower_, upper_);
}

/******************************************************************************/
/**
 * Retrieval of a random value for type double
 */
template <>
inline double baseScanParT<double>::getRandomItem() const {
   return gr_.uniform_real<double>(lower_, upper_);
}

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/
/**
 * This class holds boolean parameters
 */
class bScanPar
   :public baseScanParT<bool>
{
   ///////////////////////////////////////////////////////////////////////
   friend class boost::serialization::access;

   template<typename Archive>
   void serialize(Archive & ar, const unsigned int) {
      using boost::serialization::make_nvp;

      ar
      & BOOST_SERIALIZATION_BASE_OBJECT_NVP(baseScanParT<bool>);
   }

   ///////////////////////////////////////////////////////////////////////

public:
  /** @brief Construction from local variables */
  G_API_GENEVA bScanPar(
     parPropSpec<bool>
     , bool
  );
  /** @brief Copy constructor */
  G_API_GENEVA bScanPar(const bScanPar&);
  /** @brief The destructor */
  virtual G_API_GENEVA ~bScanPar();

  /** @brief Cloning of this object */
  G_API_GENEVA boost::shared_ptr<bScanPar> clone() const;

private:
  /** @brief The default constructor -- only needed for de-serialization, hence private */
  G_API_GENEVA bScanPar();
};

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/
/**
 * A derivative of baseScanParT for boost::int32_t values
 */
class int32ScanPar
   :public baseScanParT<boost::int32_t>
{
   ///////////////////////////////////////////////////////////////////////
   friend class boost::serialization::access;

   template<typename Archive>
   void serialize(Archive & ar, const unsigned int) {
      using boost::serialization::make_nvp;

      ar
      & BOOST_SERIALIZATION_BASE_OBJECT_NVP(baseScanParT<boost::int32_t>);
   }

   ///////////////////////////////////////////////////////////////////////

public:
   /** @brief The standard destructor */
   G_API_GENEVA int32ScanPar(
      parPropSpec<boost::int32_t>
      , bool
   );
   /** @brief Copy constructor */
   G_API_GENEVA int32ScanPar(const int32ScanPar&);
   /** @brief The destructor */
   virtual G_API_GENEVA ~int32ScanPar();

   /** @brief Cloning of this object */
   G_API_GENEVA boost::shared_ptr<int32ScanPar> clone() const;

private:
   /** @brief The default constructor -- only needed for de-serialization, hence private */
   G_API_GENEVA int32ScanPar();
};

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/
/**
 * A derivative of fpScanParT for double values
 */
class dScanPar
   :public baseScanParT<double>
{
   ///////////////////////////////////////////////////////////////////////
   friend class boost::serialization::access;

   template<typename Archive>
   void serialize(Archive & ar, const unsigned int) {
      using boost::serialization::make_nvp;

      ar
      & BOOST_SERIALIZATION_BASE_OBJECT_NVP(baseScanParT<double>);
   }

   ///////////////////////////////////////////////////////////////////////

public:
   /** @brief The standard destructor */
   G_API_GENEVA dScanPar(
      parPropSpec<double>
      , bool
   );
   /** @brief The copy constructor */
   G_API_GENEVA dScanPar(const dScanPar&);
   /** @brief The destructor */
   virtual G_API_GENEVA ~dScanPar();

   /** @brief Cloning of this object */
   G_API_GENEVA boost::shared_ptr<dScanPar> clone() const;

private:
   /** @brief The default constructor -- only needed for de-serialization, hence private */
   G_API_GENEVA dScanPar();
};


/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/
/**
 * A derivative of fpScanParT for float values
 */
class fScanPar
   :public baseScanParT<float>
{
   ///////////////////////////////////////////////////////////////////////
   friend class boost::serialization::access;

   template<typename Archive>
   void serialize(Archive & ar, const unsigned int) {
      using boost::serialization::make_nvp;

      ar
      & BOOST_SERIALIZATION_BASE_OBJECT_NVP(baseScanParT<float>);
   }

   ///////////////////////////////////////////////////////////////////////

public:
   /** @brief The standard destructor */
   G_API_GENEVA fScanPar(
      parPropSpec<float>
      , bool
   );
   /** @brief The copy constructor */
   G_API_GENEVA fScanPar(const fScanPar&);
   /** @brief The destructor */
   virtual G_API_GENEVA ~fScanPar();

   /** @brief Cloning of this object */
   G_API_GENEVA boost::shared_ptr<fScanPar> clone() const;

private:
   /** @brief The default constructor -- only needed for de-serialization, hence private */
   G_API_GENEVA fScanPar();
};

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/

} /* namespace Geneva */
} /* namespace Gem */

BOOST_CLASS_EXPORT_KEY(Gem::Geneva::bScanPar)
BOOST_CLASS_EXPORT_KEY(Gem::Geneva::int32ScanPar)
BOOST_CLASS_EXPORT_KEY(Gem::Geneva::dScanPar)
BOOST_CLASS_EXPORT_KEY(Gem::Geneva::fScanPar)

#endif /* GSCANPAR_HPP_ */

