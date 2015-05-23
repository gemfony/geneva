/**
 * @file GDelayIndividual.hpp
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

// Standard header files go here
#include <iostream>
#include <cmath>
#include <sstream>
#include <vector>

// Boost header files go here
#include <boost/serialization/nvp.hpp>
#include <boost/date_time.hpp>
#include <boost/date_time/posix_time/time_serialize.hpp>
#include <boost/date_time/gregorian/greg_serialize.hpp>
#include <boost/thread.hpp>
#include <boost/tokenizer.hpp>
#include <boost/lexical_cast.hpp>

#ifndef GDELAYINDIVIDUAL_HPP_
#define GDELAYINDIVIDUAL_HPP_

// Geneva header files go here
#include "common/GFactoryT.hpp"
#include "common/GHelperFunctionsT.hpp"
#include "geneva/GDoubleCollection.hpp"
#include "geneva/GDoubleObject.hpp"
#include "geneva/GDoubleObjectCollection.hpp"
#include "geneva/GParameterSet.hpp"
#include "geneva/GDoubleGaussAdaptor.hpp"

namespace Gem {
namespace Tests {

/******************************************************************************/
/**
 * This individual waits for a predefined amount time before returning the result of the evaluation
 * (which is always the same). Its purpose is to measure the overhead of the parallelization, compared
 * to the serial execution.
 */
class GDelayIndividual: public Gem::Geneva::GParameterSet
{
   /////////////////////////////////////////////////////////////////////////////
   friend class boost::serialization::access;

   template<class Archive>
   void serialize(Archive & ar, const unsigned int version)
   {
      using boost::serialization::make_nvp;

      ar
      & BOOST_SERIALIZATION_BASE_OBJECT_NVP(Gem::Geneva::GParameterSet)
      & BOOST_SERIALIZATION_NVP(sleepTime_);
   }
   /////////////////////////////////////////////////////////////////////////////

public:
   /** The default constructor */
   GDelayIndividual();
   /** @brief A standard copy constructor */
   GDelayIndividual(const GDelayIndividual&);
   /** @brief The standard destructor */
   virtual ~GDelayIndividual();

   /** @brief A standard assignment operator */
   const GDelayIndividual& operator=(const GDelayIndividual&);

   /** @brief Checks for equality with another GDelayIndividual object */
   bool operator==(const GDelayIndividual&) const;
   /** @brief Checks for inequality with another GDelayIndividual object */
   bool operator!=(const GDelayIndividual&) const;

   /** @brief Searches for compliance with expectations with respect to another object of the same type */
   virtual G_API_GENEVA void compare(
      const GObject& // the other object
      , const Gem::Common::expectation& // the expectation for this object, e.g. equality
      , const double& // the limit for allowed deviations of floating point types
   ) const final;

   /** @brief Sets the sleep-time to a user-defined value */
   void setSleepTime(const boost::posix_time::time_duration&);
   /** @brief Retrieval of the current value of the sleepTime_ variable */
   boost::posix_time::time_duration getSleepTime() const;

protected:
   /** @brief Loads the data of another GDelayIndividual, camouflaged as a GObject */
   virtual void load_(const GObject*) final;
   /** @brief Creates a deep clone of this object */
   virtual Gem::Geneva::GObject* clone_() const final;

   /** @brief The actual adaption operations */
   virtual std::size_t customAdaptions() final;
   /** @brief The actual fitness calculation takes place here */
   virtual double fitnessCalculation() final;

private:
   boost::posix_time::time_duration sleepTime_; ///< The amount of time the evaluation function should sleep before continuing
};

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/
/**
 * A factory for GFMinIndividual objects
 */
class GDelayIndividualFactory
   : public Gem::Common::GFactoryT<Gem::Geneva::GParameterSet>
{
public:
   /** @brief The standard constructor */
   GDelayIndividualFactory(const std::string&);
   /** @brief The destructor */
   virtual ~GDelayIndividualFactory();

   /** @brief Allows to retrieve the name of the result file */
   std::string getResultFileName() const;
   /** @brief Allows to retrieve the name of the file holding the short measurement results */
   std::string getShortResultFileName() const;
   /** @brief Allows to retrieve the number of delays requested by the user */
   std::size_t getNDelays() const;
   /** @brief Allows to retrieve the number of measurements to be made for each delay */
   boost::uint32_t getNMeasurements() const;
   /** @brief Retrieves the amount of seconds main() should wait between two measurements */
   boost::uint32_t getInterMeasurementDelay() const;
   /** @brief Retrieves the sleep times */
   std::vector<boost::tuple<unsigned int, unsigned int> > getSleepTimes() const;

protected:
   /** @brief Creates individuals of this type */
   virtual std::shared_ptr<Gem::Geneva::GParameterSet> getObject_(
      Gem::Common::GParserBuilder&
      , const std::size_t&
   ) final;
   /** @brief Allows to describe local configuration options in derived classes */
   virtual void describeLocalOptions_(Gem::Common::GParserBuilder&) final;
   /** @brief Allows to act on the configuration options received from the configuration file */
   virtual void postProcess_(std::shared_ptr<Gem::Geneva::GParameterSet>&) final;

private:
   /** @brief The default constructor. Intentionally private and undefined */
   GDelayIndividualFactory() = delete;

   /** @brief Converts a tuple to a time format */
   boost::posix_time::time_duration tupleToTime(const boost::tuple<unsigned int, unsigned int>&);

   std::size_t nVariables_;
   std::string delays_;
   std::vector<boost::tuple<unsigned int, unsigned int> > sleepTimes_;
   std::string resultFile_;
   std::string shortResultFile_;
   boost::uint32_t nMeasurements_; ///< The number of measurements for each delay
   boost::uint32_t interMeasurementDelay_; ///< The delay between two measurements
};

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/

} /* namespace Tests */
} /* namespace Gem */

BOOST_CLASS_EXPORT_KEY(Gem::Tests::GDelayIndividual)

#endif /* GDELAYINDIVIDUAL_HPP_ */
