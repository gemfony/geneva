/**
 * @file GPluggableOptimizationMonitorsT.hpp
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

// Standard header files go here
#include <string>
#include <fstream>

// Boost header files go here
#include <boost/shared_ptr.hpp>

#ifndef GPLUGGABLEOPTIMIZATIONMONITORST_HPP_
#define GPLUGGABLEOPTIMIZATIONMONITORST_HPP_

// For Microsoft-compatible compilers
#if defined(_MSC_VER)  &&  (_MSC_VER >= 1020)
#pragma once
#endif

// Geneva headers go here
#include "common/GPlotDesigner.hpp"
#include "geneva/GOptimizationAlgorithmT.hpp"

namespace Gem {
namespace Geneva {

/******************************************************************************/
/*
 * This is a collection of simple pluggable modules suitable for emitting certain specialized
 * information from within optimization algorithms. They can be plugged into GOptimizationMonitorT<>
 * derivatives. The one requirement is that they implement a function
 */

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/
/**
 * The base class. It ensures that the pluggable optimization monitors cannot
 * be copied.
 */
template <typename ind_type>
class GBasePluggableOMT
{
public:
   /***************************************************************************/
   /**
    * The default constructpr
    */
   GBasePluggableOMT()
   { /* nothing */ }

   /***************************************************************************/
   /**
    * The copy constructor
    */
   GBasePluggableOMT(const GBasePluggableOMT<ind_type>& cp)
   { /* nothing */ }

   /***************************************************************************/
   /**
    * The Destructor
    */
   virtual ~GBasePluggableOMT()
   { /* nothing */ }

   /***************************************************************************/
   /**
    * Overload this function in dervied classes, specifying actions for
    * initialization, the optimization cycles and finalization.
    */
   virtual void informationFunction(
         const infoMode& im
         , GOptimizationAlgorithmT<ind_type> * const goa
   ) = 0;
};

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/
/**
 * This class accepts a number of other pluggable monitors and aggregates
 * their work
 */
template <typename ind_type>
class GCollectiveMonitorT
: public GBasePluggableOMT<ind_type>
{
public:
   /***************************************************************************/
   /**
    * The default constructor
    */
   GCollectiveMonitorT()
   { /* nothing */ }

   /***************************************************************************/
   /**
    * The copy constructor
    */
   GCollectiveMonitorT(const GCollectiveMonitorT<ind_type>& cp)
   : GBasePluggableOMT<ind_type>(cp)
   { /* nothing */ }

   /***************************************************************************/
   /**
    * The destructor
    */
   virtual ~GCollectiveMonitorT()
   { /* nothing */ }

   /***************************************************************************/
   /**
    * Aggregates the work of all registered pluggable monitors
    */
   virtual void informationFunction(
      const infoMode& im
      , GOptimizationAlgorithmT<ind_type> * const goa
   ) {
      typename std::vector<boost::shared_ptr<GBasePluggableOMT<ind_type> > >::iterator it;
      for(it=pluggable_monitors_.begin(); it!=pluggable_monitors_.end(); ++it) {
         (*it)->GBasePluggableOMT<ind_type>::informationFunction(im,goa);
      }
   }

   /***************************************************************************/
   /**
    * Allows to register a new pluggable monitor
    */
   void registerPluggableOM(boost::shared_ptr<GBasePluggableOMT<ind_type> > om_ptr) {
      if(om_ptr) {
         pluggable_monitors_.push_back(om_ptr);
      } else {
         glogger
         << "In GCollectiveMonitorT<>::registerPluggableOM(): Error!" << std::endl
         << "Got empty pointer to pluggable optimization monitor." << std::endl
         << GEXCEPTION;
      }
   }

   /***************************************************************************/
   /**
    * Allows to clear all registered monitors
    */
   void reset() {
      pluggable_monitors_.clear();
   }

private:
   std::vector<boost::shared_ptr<GBasePluggableOMT<ind_type> > > pluggable_monitors_; ///< The collection of monitors
};

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/
/**
 * This class allows to monitor a given set of variables inside of all individuals of a population,
 * creating a graphical output using ROOT.
 */
template <typename ind_type>
class GProgressPlotterT
: public GBasePluggableOMT<ind_type>
{
public:
   /***************************************************************************/
   /**
    * The default constructor
    */
   GProgressPlotterT()
   : profVarVec_()
   , gpd_oa_("Progress information", 1, 1)
   , fileName_("parameterScan.C")
   , canvasDimensions_(boost::tuple<boost::uint32_t,boost::uint32_t>(1024,768))
   { /* nothing */ }

   /***************************************************************************/
   /**
    * The copy constructor
    */
   GProgressPlotterT(const GProgressPlotterT<ind_type>& cp)
   : GBasePluggableOMT<ind_type>(cp)
   , profVarVec_(cp.profVarVec_)
   , gpd_oa_("Progress information", 1, 1) // Not copied
   , fileName_(cp.fileName_)
   , canvasDimensions_(cp.canvasDimensions_)
   { /* nothing */ }

   /***************************************************************************/
   /**
    * The destuctor
    */
   virtual ~GProgressPlotterT()
   { /* nothing */ }

   /**************************************************************************/
   /**
    * Adds a variable type and position to be profiled. We only allow floats,
    * doubles and integers.
    */
   void addProfileVar(std::string descr, std::size_t pos) {
      if(descr != "d" && descr != "f" && descr != "i") {
         glogger
         << "In GBasePluggableOMT<>::addProfileVar("<< descr << ", " << pos << "): Error!" << std::endl
         << "Got invalid type description" << std::endl
         << GEXCEPTION;
      }

      if(profVarVec_.size() >= 2) {
         glogger
         << "In GBasePluggableOMT<>::addProfileVar("<< descr << ", " << pos << "): Error!" << std::endl
         << "Trying to add a profile variable while already " << profVarVec_.size() << " variables are present" << std::endl
         << GEXCEPTION;
      }

      profVarVec_.push_back(boost::tuple<std::string, std::size_t>(descr, pos));
   }

   /***************************************************************************/
   /**
    * Clears all variables to be profiled
    */
   void clearProfileVars() {
      profVarVec_.clear();
   }

   /***************************************************************************/
   /**
    * Allows to check whether parameters should be profiled
    */
   bool parameterProfileCreationRequested() const {
      return !profVarVec_.empty();
   }

   /***************************************************************************/
   /**
    * Retrieves the number of variables that will be profiled
    */
   std::size_t nProfileVars() const {
      return profVarVec_.size();
   }

   /***************************************************************************/
   /**
    * Allows to set the canvas dimensions
    */
   void setCanvasDimensions(boost::tuple<boost::uint32_t,boost::uint32_t> canvasDimensions) {
      canvasDimensions_ = canvasDimensions;
   }

   /***************************************************************************/
   /**
    * Allows to set the canvas dimensions using individual x and y values
    */
   void setCanvasDimensions(boost::uint32_t x, boost::uint32_t y) {
      canvasDimensions_ = boost::tuple<boost::uint32_t,boost::uint32_t>(x,y);
   }

   /***************************************************************************/
   /**
    * Gives access to the canvas dimensions
    */
   boost::tuple<boost::uint32_t,boost::uint32_t> getCanvasDimensions() const {
      return canvasDimensions_;
   }

   /***************************************************************************/
   /**
    * Allows to set the filename
    */
   void setFileName(std::string fileName) {
      fileName_ = fileName;
   }

   /***************************************************************************/
   /**
    * Retrieves the current filename to which information will be emitted
    */
   std::string getFileName() const {
      return fileName_;
   }

   /***************************************************************************/
   /**
    * Allows to set the canvas label
    */
   void setCanvasLabel(const std::string& canvasLabel) {
      gpd_oa_.setCanvasLabel(canvasLabel);
   }

   /***************************************************************************/
   /**
    * Allows to retrieve the canvas label
    */
   std::string getCanvasLabel() const {
      return gpd_oa_.getCanvasLabel();
   }

   /***************************************************************************/
   /**
    * Allows to emit information in different stages of the information cycle
    * (initialization, during each cycle and during finalization)
    */
   virtual void informationFunction(
      const infoMode& im
      , GOptimizationAlgorithmT<ind_type> * const goa
   ) {
      switch(im) {
      case Gem::Geneva::INFOINIT:
      {
         switch(this->nProfileVars()) {
            case 1:
            {
               progressPlotter2D_oa_ = boost::shared_ptr<Gem::Common::GGraph2D>(new Gem::Common::GGraph2D());

               progressPlotter2D_oa_->setPlotMode(Gem::Common::SCATTER);
               progressPlotter2D_oa_->setPlotLabel("Fitness as a function of a parameter value");
               progressPlotter2D_oa_->setXAxisLabel("Parameter Value");
               progressPlotter2D_oa_->setYAxisLabel("Fitness");

               gpd_oa_.registerPlotter(progressPlotter2D_oa_);
            }
               break;
            case 2:
            {
               progressPlotter3D_oa_ = boost::shared_ptr<Gem::Common::GGraph3D>(new Gem::Common::GGraph3D());

               progressPlotter3D_oa_->setPlotLabel("Fitness as a function of parameter values");
               progressPlotter3D_oa_->setXAxisLabel("Parameter Value 1");
               progressPlotter3D_oa_->setYAxisLabel("Parameter Value 2");
               progressPlotter3D_oa_->setZAxisLabel("Fitness");

               gpd_oa_.registerPlotter(progressPlotter3D_oa_);
            }
               break;
         }

         gpd_oa_.setCanvasDimensions(canvasDimensions_);
      }
         break;

      case Gem::Geneva::INFOPROCESSING:
      {
         bool isDirty;
         typename GOptimizationAlgorithmT<ind_type>::iterator it;

         for(it=goa->begin(); it!=goa->end(); ++it) {
            switch(this->nProfileVars()) {
               case 1:
               {
                  double val0    = (*it)->GIndividual::getVarVal<double>(profVarVec_[0]);
                  double fitness = (*it)->getCachedFitness(isDirty);

                  progressPlotter2D_oa_->add(boost::tuple<double,double>(val0, fitness));
               }
               break;

               case 2:
               {
                  double val0 = (*it)->GIndividual::getVarVal<double>(profVarVec_[0]);
                  double val1 = (*it)->GIndividual::getVarVal<double>(profVarVec_[1]);
                  double fitness = (*it)->getCachedFitness(isDirty);

                  progressPlotter3D_oa_->add(boost::tuple<double,double,double>(val0, val1, fitness));
               }
               break;
            }
         }
      }
         break;

      case Gem::Geneva::INFOEND:
      {
         // Write out the result. Note that we add
         gpd_oa_.writeToFile(fileName_);

         // Remove all plotters
         gpd_oa_.resetPlotters();
         progressPlotter2D_oa_.reset();
         progressPlotter3D_oa_.reset();
      }
         break;

      default:
      {
         glogger
         << "In GProgressPlotterT<>::informationFunction(): Received invalid infoMode " << im << std::endl
         << GEXCEPTION;
      }
      break;
      };
   }

private:
   std::vector<boost::tuple<std::string, std::size_t> > profVarVec_; ///< Holds the types and positions of variables to be profiled

   Gem::Common::GPlotDesigner gpd_oa_; ///< A wrapper for the plots

   // These are temporaries
   boost::shared_ptr<Gem::Common::GGraph2D> progressPlotter2D_oa_;
   boost::shared_ptr<Gem::Common::GGraph3D> progressPlotter3D_oa_;

   std::string fileName_; ///< The name of the file the output should be written to. Note that the class will add the name of the algorithm it acts on
   boost::tuple<boost::uint32_t,boost::uint32_t> canvasDimensions_; ///< The dimensions of the canvas
};

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/

} /* namespace Geneva */
} /* namespace Gem */

#endif /* GPLUGGABLEOPTIMIZATIONMONITORST_HPP_ */
