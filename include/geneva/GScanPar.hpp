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
 * Basic parameter functionality
 */
template <typename T>
class baseScanParT {
   ///////////////////////////////////////////////////////////////////////
   friend class boost::serialization::access;

   template<typename Archive>
   void serialize(Archive & ar, const unsigned int) {
      using boost::serialization::make_nvp;

      ar & BOOST_SERIALIZATION_NVP(pos)
         & BOOST_SERIALIZATION_NVP(nSteps)
         & BOOST_SERIALIZATION_NVP(lower)
         & BOOST_SERIALIZATION_NVP(upper)
         & BOOST_SERIALIZATION_NVP(typeDescription)
         & BOOST_SERIALIZATION_NVP(currentStep);
   }

   ///////////////////////////////////////////////////////////////////////

public:
   /** The standard constructor */
   baseScanParT(
         std::size_t p
         , std::size_t nS
         , T l
         , T u
   )
      : pos(p)
      , nSteps(nS)
      , lower(l)
      , upper(u)
      , currentStep(0)
   { /* nothing */ }

   /** A constructor that splits an input string into tokens */
   baseScanParT(const std::string& desc) {
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
      typeDescription = tokens.at(0);
      pos             = boost::lexical_cast<std::size_t>(tokens.at(1));
      lower           = boost::lexical_cast<T>(tokens.at(2));
      upper           = boost::lexical_cast<T>(tokens.at(3));
      nSteps          = boost::lexical_cast<std::size_t>(tokens.at(4));
   }

   /** The destructor */
   virtual ~baseScanParT()
   { /* nothing */ }

   /** Returns all allowed values for this type */
   std::vector<T> getValues(bool random=false) const {
      T item;
      std::vector<T> result;

      while(true) {
         try {
            item = this->next(random);
         } catch(g_end_of_par& g) {
            break; // Terminate the loop
         }

         result.push_back(item);
      }

      return result;
   }

   /** Retrieves the next value or throws, if there are no more values */
   T next(bool random=false) const {
      if(currentStep++ < (random?nSteps:this->getMaxSteps())) {
         return getNextValue(currentStep-1, random);
      } else {
         currentStep=0; // Rewind the counter
         throw g_end_of_par();
      }

      // Make the compiler happy
      return T(0);
   }

protected:
   /** @brief Retrieves the next value in the chain */
   virtual T getNextValue(std::size_t, bool) const = 0;
   /** @brief Retrieves the maximum number of steps for a given type if not in random mode */
   virtual std::size_t getMaxSteps() const = 0;

   std::size_t pos; ///< The position of the parameter
   std::size_t nSteps; ///< Holds the number of parameters to scan for this value

   T lower; ///< Holds the lower boundary of this type
   T upper; ///< Holds the upper boundary of this type

   mutable Gem::Hap::GRandom gr; ///< Random number generator

   std::string typeDescription; ///< Holds an identifier for the type described by this class

   /** @brief The default constructor -- only needed for de-serialization, hence protected */
   baseScanParT() :currentStep(0) { /* nothing */ }

private:
   mutable std::size_t currentStep;
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
      std::size_t p
      , std::size_t nS
      , bool l
      , bool u
  );
  /** @brief Construction from the specification string */
  bScanPar(const std::string&);
  /** @brief The destructor */
  virtual ~bScanPar();

protected:
  /** @brief Retrieves the next value in the chain */
  virtual bool getNextValue(std::size_t, bool) const;
  /** @brief Retrieves the maximum number of steps for a given type if not in random mode */
  virtual std::size_t getMaxSteps() const;

private:
  /** @brief The default constructor -- only needed for de-serialization, hence private */
  bScanPar();
};

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/
/**
 * This class holds integer parameters
 */
template <typename int_type>
class iScanParT
   :public baseScanParT<int_type>
{
   ///////////////////////////////////////////////////////////////////////
   friend class boost::serialization::access;

   template<typename Archive>
   void serialize(Archive & ar, const unsigned int) {
      using boost::serialization::make_nvp;

      ar & BOOST_SERIALIZATION_BASE_OBJECT_NVP(baseScanParT<int_type>);
   }

   ///////////////////////////////////////////////////////////////////////

  // Make sure this class only accepts unsigned integer types
  BOOST_MPL_ASSERT((boost::is_integral<int_type>));

public:
  /** @brief Construction from local variables */
  iScanParT(
      std::size_t p
      , std::size_t nS
      , int_type l
      , int_type u
  )
      : baseScanParT<int_type>(p, nS, l, u)
  { /* nothing */ }

  /** @brief Construction from the specification string */
  iScanParT(const std::string& desc)
      : baseScanParT<int_type>(desc)
  { /* nothing */ }

  /** @brief The destructor */
  virtual ~iScanParT()
  { /* nothing */ }

protected:
  /** @brief Retrieves the next value in the chain */
  virtual int_type getNextValue(
        std::size_t cv
        , bool random
  ) const {
     if(random) {
        return this->gr.template uniform_int<int_type>(this->lower, this->upper+1); // uniform_int excludes the upper boundary
     } else {
        return this->lower + int_type(cv);
     }
  }

  /** @brief Retrieves the maximum number of steps for a given type if not in random mode */
  virtual std::size_t getMaxSteps() const {
     return (this->upper - this->lower + 1);
  }

  /** @brief The default constructor; only needed for (de-)serialization, hence protected */
  iScanParT() : baseScanParT<int_type>() { /* nothing */ }
};

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/
/**
 * A derivative of iScanParT for boost::int32_t values
 */
class int32ScanPar
   :public iScanParT<boost::int32_t>
{
   ///////////////////////////////////////////////////////////////////////
   friend class boost::serialization::access;

   template<typename Archive>
   void serialize(Archive & ar, const unsigned int) {
      using boost::serialization::make_nvp;

      ar & BOOST_SERIALIZATION_BASE_OBJECT_NVP(iScanParT<boost::int32_t>);
   }

   ///////////////////////////////////////////////////////////////////////

public:
   /** @brief The standard destructor */
   int32ScanPar(
       std::size_t
       , std::size_t
       , boost::int32_t
       , boost::int32_t
   );

   /** @brief Construction from the specification string */
   int32ScanPar(const std::string&);

private:
   /** @brief The default constructor -- only needed for de-serialization, hence private */
   int32ScanPar();
};

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/
/**
 * This class holds integer parameters
 */
template <typename fp_type>
class fpScanParT
   :public baseScanParT<fp_type>
{
   ///////////////////////////////////////////////////////////////////////
   friend class boost::serialization::access;

   template<typename Archive>
   void serialize(Archive & ar, const unsigned int) {
      using boost::serialization::make_nvp;

      ar & BOOST_SERIALIZATION_BASE_OBJECT_NVP(baseScanParT<fp_type>);
   }

   ///////////////////////////////////////////////////////////////////////

  // Make sure this class only accepts unsigned integer types
  BOOST_MPL_ASSERT((boost::is_floating_point<fp_type>));

public:
  /** @brief Construction from local variables */
  fpScanParT(
      std::size_t p
      , std::size_t nS
      , fp_type l
      , fp_type u
  )
      : baseScanParT<fp_type>(p, nS, l, u)
  { /* nothing */ }

  /** @brief Construction from the specification string */
  fpScanParT(const std::string& desc)
      : baseScanParT<fp_type>(desc)
  { /* nothing */ }

  /** @brief The destructor */
  virtual ~fpScanParT()
  { /* nothing */ }

protected:
  /** @brief Retrieves the next value in the chain */
  virtual fp_type getNextValue(
        std::size_t cv
        , bool random
  ) const {
     if(random) {
        return this->gr.template uniform_real<fp_type>(this->lower, this->upper);
     } else {
        return this->lower + (this->upper-this->lower)*fp_type(cv)/fp_type(this->getMaxSteps() - 1);
     }
  }

  /** @brief Retrieves the maximum number of steps for a given type if not in random mode */
  virtual std::size_t getMaxSteps() const {
     return this->nSteps;
  }

  /** @brief The default constructor; only needed for (de-)serialization, hence protected */
  fpScanParT() : baseScanParT<fp_type>() { /* nothing */ }
};

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/
/**
 * A derivative of fpScanParT for double values
 */
class dScanPar
   :public fpScanParT<double>
{
   ///////////////////////////////////////////////////////////////////////
   friend class boost::serialization::access;

   template<typename Archive>
   void serialize(Archive & ar, const unsigned int) {
      using boost::serialization::make_nvp;

      ar & BOOST_SERIALIZATION_BASE_OBJECT_NVP(fpScanParT<double>);
   }

   ///////////////////////////////////////////////////////////////////////

public:
   /** @brief The standard destructor */
   dScanPar(
       std::size_t
       , std::size_t
       , double
       , double
   );

   /** @brief Construction from the specification string */
   dScanPar(const std::string&);

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
   :public fpScanParT<float>
{
   ///////////////////////////////////////////////////////////////////////
   friend class boost::serialization::access;

   template<typename Archive>
   void serialize(Archive & ar, const unsigned int) {
      using boost::serialization::make_nvp;

      ar & BOOST_SERIALIZATION_BASE_OBJECT_NVP(fpScanParT<float>);
   }

   ///////////////////////////////////////////////////////////////////////

public:
   /** @brief The standard destructor */
   fScanPar(
       std::size_t
       , std::size_t
       , float
       , float
   );

   /** @brief Construction from the specification string */
   fScanPar(const std::string&);

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
