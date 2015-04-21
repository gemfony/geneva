/**
 * @file GLineFit.cpp
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

#include "GLineFit.hpp"

namespace Gem {
namespace Geneva {

/******************************************************************************/
/**
 * This function takes a vector of x-y coordinates and calculates the straight
 * line that best fits the measurements. It does so using the least-square method.
 *
 * @param points_ The measurements (x/y-coordinates)
 * @return A tuple of doubles, so that f(x)=a+b*x
 */
boost::tuple<double, double> gLineFit(const std::vector<boost::tuple<double, double> >& points_) {
   // Copy the data, as we want to modify (sort) it
   std::vector<boost::tuple<double, double> > points = points_;

   // Create a factory for GLineFitIndividual objects
   boost::shared_ptr<GLineFitIndividualFactory> glfif_ptr(new GLineFitIndividualFactory(points, "./config/GLineFitIndividual.json"));

   // Create the optimizer and set its options
   boost::shared_ptr<GMultiThreadedEA> ea_ptr(new GMultiThreadedEA());

   ea_ptr->setPopulationSizes(203,3);
   ea_ptr->setMaxIteration(5000);
   ea_ptr->setMaxTime(boost::posix_time::seconds(60));
   ea_ptr->setReportIteration(0);
   ea_ptr->setSortingScheme(MUNU1PRETAIN_SINGLEEVAL);
   ea_ptr->setMaxStallIteration(100);

   // Retrieve a first individual with random values
   boost::shared_ptr<GParameterSet> p = glfif_ptr->get();

   // Add it to the algorithm
   ea_ptr->push_back(p);

   // Make sure the points vector are sorted according to their x-values
   std::sort(
      points.begin()
      , points.end()
      , [](const boost::tuple<double,double>& a, const boost::tuple<double,double>& b) -> bool {
         return boost::get<0>(a) < boost::get<1>(b);
      }
   );

   // Calculate a line from the first and last points in the points-vector
   double deltaX = boost::get<0>(points.back()) - boost::get<0>(points.front());
   double deltaY = boost::get<1>(points.back()) - boost::get<1>(points.front());

   // Check if deltaX is 0, if so, complain
   if(0. == deltaX)  {
      glogger
      << "In gLinearFit(): First and last point of array seem to be identical, so deltaX is 0.!" << std::endl
      << GEXCEPTION;
   }

   // Calculate a and b
   double b = deltaY/deltaX;
   double a = boost::get<1>(points.front()) - b*boost::get<0>(points.front());

   // Set the first individuals values to a and b
   std::vector<double> parVec;
   p->streamline(parVec);
   parVec[0] = a;
   parVec[1] = b;
   p->assignValueVector(parVec);

   // Start the optimization and extract the best individual
   boost::shared_ptr<GLineFitIndividual> p_best = ea_ptr->GOptimizableI::optimize<GLineFitIndividual>();

   return p_best->getLine();
}

/******************************************************************************/

} /* namespace Geneva */
} /* namespace Gem */


