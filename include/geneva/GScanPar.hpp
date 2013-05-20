/**
 * @file GScanPar.hpp
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

// For Microsoft-compatible compilers
#if defined(_MSC_VER)  &&  (_MSC_VER >= 1020)
#pragma once
#endif

// Geneva headers go here
#include "hap/GRandomT.hpp"
#include "geneva/GObject.hpp"
#include "geneva/GStdSimpleVectorInterfaceT.hpp"

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
      , bool /* randomFill */
) {
   glogger
   << "In generic function template <typename T> std::vector<T> fillWithData(): Error!" << std::endl
   << "This function should never be called directly. Use one of the specializations." << std::endl
   << GEXCEPTION;

   // Make the compiler happy
   return std::vector<T>();
}

/** @bool Returns a set of boolean data items */
template <>
std::vector<bool> fillWithData<bool>(
      std::size_t /* nSteps */
      , bool      /* lower */
      , bool      /* upper */
      , bool      /* randomFill */
);

/** @brief Returns a set of boost::int32_t data items */
template <>
std::vector<boost::int32_t> fillWithData<boost::int32_t>(
      std::size_t      /* nSteps */
      , boost::int32_t /* lower */
      , boost::int32_t /* upper */
      , bool           /* randomFill */
);

/** @brief Returns a set of float data items */
template <>
std::vector<float> fillWithData<float>(
      std::size_t /* nSteps */
      , float     /* lower */
      , float     /* upper */
      , bool      /* randomFill */
);

/** @brief Returns a set of double data items */
template <>
std::vector<double> fillWithData<double>(
      std::size_t /* nSteps */
      , double    /* lower */
      , double    /* upper */
      , bool      /* randomFill */
);

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/
/**
 * An interface class for parameter scan objects
 */
class scanParI {
public:
   virtual ~scanParI(){ /* nothing */ }

   virtual std::size_t getPos() const = 0;
   virtual bool goToNextItem() = 0;
   virtual bool isAtTerminalPosition() const = 0;
   virtual bool isAtFirstPosition() const = 0;
   virtual void resetPosition() = 0;
   virtual std::string getTypeDescriptor() const = 0;
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
   , public scanParI
{
   ///////////////////////////////////////////////////////////////////////
   friend class boost::serialization::access;

   template<typename Archive>
   void serialize(Archive & ar, const unsigned int) {
      using boost::serialization::make_nvp;

      ar
      & BOOST_SERIALIZATION_BASE_OBJECT_NVP(GStdSimpleVectorInterfaceT<T>)
      & BOOST_SERIALIZATION_NVP(pos)
      & BOOST_SERIALIZATION_NVP(currentItemPos)
      & BOOST_SERIALIZATION_NVP(typeDescription);
   }

   ///////////////////////////////////////////////////////////////////////

public:
   /***************************************************************************/
   /**
    * The standard constructor
    */
   baseScanParT(
         std::size_t p
         , std::size_t nSteps
         , T lower
         , T upper
         , bool randomScan
         , std::string t // typeDescription
   )
      : GStdSimpleVectorInterfaceT<T>()
      , pos(p)
      , currentItemPos(0)
      , typeDescription(t)
   { // Fill the object with data
      this->data = fillWithData<T>(nSteps, lower, upper, randomScan);
   }

   /***************************************************************************/
   /**
    * A constructor that splits an input string into tokens
    */
   baseScanParT(
         const std::string& desc
         , bool randomScan
   )
      : GStdSimpleVectorInterfaceT<T>()
      , pos(0)
      , currentItemPos(0)
      , typeDescription("")
   {
      std::vector<std::string> tokens;
      boost::split(tokens, desc, boost::is_any_of(" "));

      // Check that we have the correct number of tokens
      if(tokens.size() != 5) {
         glogger
         << "In baseScanParT<>::baseScanParT(): Error!" << std::endl
         << "Invalid number of tokens" << std::endl
         << GTERMINATION;
      }

      std::vector<std::string>::iterator it;
      for(it=tokens.begin(); it!=tokens.end(); ++it) {
         boost::trim(*it);
      }

      // Assign tokens to our local variables
      typeDescription    = tokens.at(0);
      pos                = boost::lexical_cast<std::size_t>(tokens.at(1));
      T lower            = boost::lexical_cast<T>(tokens.at(2));
      T upper            = boost::lexical_cast<T>(tokens.at(3));
      std::size_t nSteps = boost::lexical_cast<std::size_t>(tokens.at(4));

      // Fill the object with data
      this->data = fillWithData<T>(nSteps, lower, upper, randomScan);
   }

   /***************************************************************************/
   /**
    * The copy constructor
    */
   baseScanParT(const baseScanParT<T>& cp)
      : GStdSimpleVectorInterfaceT<T>(cp)
      , pos(cp.pos)
      , currentItemPos(cp.currentItemPos)
      , typeDescription(cp.typeDescription)
   { /* nothing */ }

   /***************************************************************************/
   /**
    * The destructor
    */
   virtual ~baseScanParT()
   { /* nothing */ }

   /***************************************************************************/
   /**
    * Retrieve the position of this object
    */
   virtual std::size_t getPos() const {
      return pos;
   }

   /***************************************************************************/
   /**
    * Retrieves the current item position
    */
   std::size_t getCurrentItemPos() const {
      return currentItemPos;
   }

   /***************************************************************************/
   /**
    * Retrieve the current item
    */
   T getCurrentItem() const {
      return this->at(currentItemPos);
   }

   /***************************************************************************/
   /**
    * Switch to the next position in the vector or rewind
    *
    * @return A boolean indicating whether a warp has taken place
    */
   virtual bool goToNextItem() {
      if(++currentItemPos >= this->size()) {
         currentItemPos = 0;
         return true;
      }
      return false;
   }

   /***************************************************************************/
   /**
    * Checks whether currentItemPos points to the last item in the array
    */
   virtual bool isAtTerminalPosition() const {
      if(currentItemPos >= (this->size()-1)) return true;
      else return false;
   }

   /***************************************************************************/
   /**
    * Checks whether currentItemPos points to the first item in the array
    */
   virtual bool isAtFirstPosition() const {
      if(0 == currentItemPos) return true;
      else return false;
   }

   /***************************************************************************/
   /**
    * Resets the current position
    */
   virtual void resetPosition() {
      currentItemPos = 0;
   }

   /***************************************************************************/
   /**
    * Retrieve the type descriptor
    */
   virtual std::string getTypeDescriptor() const {
      return typeDescription;
   }

protected:
   /***************************************************************************/
   // Data

   std::size_t pos; ///< The position of the parameter
   std::size_t currentItemPos; ///< The current position in the data vector
   std::string typeDescription; ///< Holds an identifier for the type described by this class

   /***************************************************************************/
   /** @brief The default constructor -- only needed for de-serialization, hence protected */
   baseScanParT() { /* nothing */ }

   /** @brief Needs to be re-implemented for derivatives of GStdSimpleVectorInterfaceT<> */
   virtual void dummyFunction(){};
};

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

      ar & BOOST_SERIALIZATION_BASE_OBJECT_NVP(baseScanParT<bool>);
   }

   ///////////////////////////////////////////////////////////////////////

public:
  /** @brief Construction from local variables */
  bScanPar(
      std::size_t
      , std::size_t
      , bool
      , bool
      , bool
  );
  /** @brief Construction from the specification string */
  bScanPar(const std::string&, bool);
  /** @brief Copy constructor */
  bScanPar(const bScanPar&);
  /** @brief The destructor */
  virtual ~bScanPar();

  /** @brief Cloning of this object */
  boost::shared_ptr<bScanPar> clone() const;

private:
  /** @brief The default constructor -- only needed for de-serialization, hence private */
  bScanPar();
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

      ar & BOOST_SERIALIZATION_BASE_OBJECT_NVP(baseScanParT<boost::int32_t>);
   }

   ///////////////////////////////////////////////////////////////////////

public:
   /** @brief The standard destructor */
   int32ScanPar(
       std::size_t
       , std::size_t
       , boost::int32_t
       , boost::int32_t
       , bool
   );
   /** @brief Construction from the specification string */
   int32ScanPar(const std::string&,bool);
   /** @brief Copy constructor */
   int32ScanPar(const int32ScanPar&);
   /** @brief The destructor */
   virtual ~int32ScanPar();

   /** @brief Cloning of this object */
   boost::shared_ptr<int32ScanPar> clone() const;

private:
   /** @brief The default constructor -- only needed for de-serialization, hence private */
   int32ScanPar();
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

      ar & BOOST_SERIALIZATION_BASE_OBJECT_NVP(baseScanParT<double>);
   }

   ///////////////////////////////////////////////////////////////////////

public:
   /** @brief The standard destructor */
   dScanPar(
       std::size_t
       , std::size_t
       , double
       , double
       , bool
   );
   /** @brief Construction from the specification string */
   dScanPar(
         const std::string&
         , bool
   );
   /** @brief The copy constructor */
   dScanPar(const dScanPar&);
   /** @brief The destructor */
   virtual ~dScanPar();

   /** @brief Cloning of this object */
   boost::shared_ptr<dScanPar> clone() const;

private:
   /** @brief The default constructor -- only needed for de-serialization, hence private */
   dScanPar();
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

      ar & BOOST_SERIALIZATION_BASE_OBJECT_NVP(baseScanParT<float>);
   }

   ///////////////////////////////////////////////////////////////////////

public:
   /** @brief The standard destructor */
   fScanPar(
       std::size_t
       , std::size_t
       , float
       , float
       , bool
   );
   /** @brief Construction from the specification string */
   fScanPar(
         const std::string&
         , bool
   );
   /** @brief The copy constructor */
   fScanPar(const fScanPar&);
   /** @brief The destructor */
   virtual ~fScanPar();

   /** @brief Cloning of this object */
   boost::shared_ptr<fScanPar> clone() const;

private:
   /** @brief The default constructor -- only needed for de-serialization, hence private */
   fScanPar();
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

