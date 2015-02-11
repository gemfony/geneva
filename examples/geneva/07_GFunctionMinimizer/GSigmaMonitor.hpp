/**
 * @file GSigmaMonitor.hpp
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
#include <string>
#include <fstream>

// Boost header files go here
#include <boost/shared_ptr.hpp>

#ifndef GOPTIMIZATIONMONITOR_HPP_
#define GOPTIMIZATIONMONITOR_HPP_

// Geneva header files go here
#include "common/GPlotDesigner.hpp" // Ease recording of essential information
#include "geneva/GBaseEA.hpp" // Get access to the GEAOptimizationMonitor class
#include "GFMinIndividual.hpp" // The individual to be optimized

namespace Gem {
namespace Geneva {

const std::size_t P_XDIM=1200;
const std::size_t P_YDIM=1400;

/******************************************************************************/
/**
 * This class implements an optimization monitor for Evolutionary Algorithms. Its main purpose is
 * to find out information about the development of sigma over the course of the optimization for
 * the best individuals. This monitor is thus targeted at a specific type of individual. Note that
 * the class uses ROOT scripts for the output of its results.
 */
class G_API GSigmaMonitor
   :public GBaseEA::GEAOptimizationMonitor
{
   ///////////////////////////////////////////////////////////////////////
   friend class boost::serialization::access;

   template<typename Archive>
   void serialize(Archive & ar, const unsigned int){
      using boost::serialization::make_nvp;

      ar
      & make_nvp("GBaseEA_GEAOptimizationMonitor", boost::serialization::base_object<GBaseEA::GEAOptimizationMonitor>(*this))
      & BOOST_SERIALIZATION_NVP(fileName_);
   }
   ///////////////////////////////////////////////////////////////////////

public:
   /** @brief The default constructor */
   GSigmaMonitor(const std::string fileName);
   /** @brief The copy constructor */
   GSigmaMonitor(const GSigmaMonitor& cp);
   /** @brief The destructor */
   virtual ~GSigmaMonitor();

protected:
   /** @brief A function that is called once before the optimization starts */
   virtual void firstInformation(GOptimizationAlgorithmT<GParameterSet> * const goa);
   /** @brief A function that is called during each optimization cycle */
   virtual void cycleInformation(GOptimizationAlgorithmT<GParameterSet> * const goa);
   /** @brief A function that is called once at the end of the optimization cycle */
   virtual void lastInformation(GOptimizationAlgorithmT<GParameterSet> * const goa);

   /** @brief Loads the data of another object */
   virtual void load_(const GObject* cp);
   /** @brief Creates a deep clone of this object */
   virtual GObject* clone_() const;

private:
   /** @brief The default constructor -- intentionally private */
   GSigmaMonitor();

   std::string fileName_; ///< The name of the output file

   Gem::Common::GPlotDesigner gpd_; ///< Ease recording of essential information
   boost::shared_ptr<Gem::Common::GGraph2D> progressPlotter_; ///< Records progress information
   boost::shared_ptr<Gem::Common::GGraph2D> sigmaPlotter_; ///< Records progress information about the current sigma
};

/******************************************************************************/

} /* namespace Geneva */
} /* namespace Gem */

#endif /* GOPTIMIZATIONMONITOR_HPP_ */
