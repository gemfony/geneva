/**
 * @file GPluggableOptimizationMonitorsT.hpp
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

// Standard header files go here
#include <string>
#include <fstream>

// Boost header files go here
#include <boost/shared_ptr.hpp>
#include <boost/mpl/assert.hpp>
#include <boost/type_traits.hpp>
#include <boost/filesystem.hpp>
#include "boost/filesystem/path.hpp"
#include "boost/filesystem/operations.hpp"
#include "boost/filesystem/convenience.hpp"
#include <boost/date_time.hpp>

#ifndef GPLUGGABLEOPTIMIZATIONMONITORST_HPP_
#define GPLUGGABLEOPTIMIZATIONMONITORST_HPP_

// For Microsoft-compatible compilers
#if defined(_MSC_VER)  &&  (_MSC_VER >= 1020)
#pragma once
#endif

// Geneva headers go here
#include "common/GPlotDesigner.hpp"
#include "common/GLogger.hpp"
#include "geneva/GOptimizationAlgorithmT.hpp"
#include "geneva/GParameterPropertyParser.hpp"

namespace Gem {
namespace Geneva {

/******************************************************************************/
/*
 * This is a collection of simple pluggable modules suitable for emitting certain specialized
 * information from within optimization algorithms. They can be plugged into GOptimizationMonitorT<>
 * derivatives. The one requirement is that they implement a function "informationFunction"
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
      : useTrueEvaluation_(false)
   { /* nothing */ }

   /***************************************************************************/
   /**
    * The copy constructor
    */
   GBasePluggableOMT(const GBasePluggableOMT<ind_type>& cp)
      : useTrueEvaluation_(cp.useTrueEvaluation_)
   { /* nothing */ }

   /***************************************************************************/
   /**
    * The Destructor
    */
   virtual ~GBasePluggableOMT()
   { /* nothing */ }

   /***************************************************************************/
   /**
    * Overload this function in derived classes, specifying actions for
    * initialization, the optimization cycles and finalization.
    */
   virtual void informationFunction(
         const infoMode& im
         , GOptimizationAlgorithmT<ind_type> * const goa
   ) = 0;

   /***************************************************************************/
   /**
    * Allows to set the useTrueEvaluation_ variable
    */
   void setUseTrueEvaluation(bool useTrue) {
      useTrueEvaluation_ = useTrue;
   }

   /***************************************************************************/
   /**
    * Allows to retrieve the value of the useTrueEvaluation_ variable
    */
   bool getUseTrueEvaluation() const {
      return useTrueEvaluation_;
   }

protected:
   /***************************************************************************/
   bool useTrueEvaluation_; ///< Specifies whether the true (unmodified) evaluation should be used
};

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/
/**
 * This class accepts a number of other pluggable monitors and executes them
 * in sequnce.
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
   ) OVERRIDE {
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
 * This class allows to monitor a given set of variables inside of all or of the
 * best individuals of a population, creating a graphical output using ROOT. It
 * supports floating point types only. double and float values may not be mixed.
 */
template <typename ind_type, typename fp_type>
class GProgressPlotterT : public GBasePluggableOMT<ind_type>
{
   // Make sure this class can only be instantiated if fp_type really is a floating point type
   BOOST_MPL_ASSERT((boost::is_floating_point<fp_type>));

public:
   /***************************************************************************/
   /**
    * The default constructor
    */
   GProgressPlotterT()
      : gpd_oa_("Progress information", 1, 1)
      , fileName_("progressScan.C")
      , canvasDimensions_(boost::tuple<boost::uint32_t,boost::uint32_t>(1024,768))
      , monitorBestOnly_(false)
      , monitorValidOnly_(false)
      , observeBoundaries_(false)
      , addPrintCommand_(false)
   { /* nothing */ }

   /***************************************************************************/
   /**
    * Construction with the information whether only the best individuals
    * should be monitored and whether only valid items should be recorded.
    */
   GProgressPlotterT(bool monitorBestOnly, bool monitorValidOnly)
      : gpd_oa_("Progress information", 1, 1)
      , fileName_("progressScan.C")
      , canvasDimensions_(boost::tuple<boost::uint32_t,boost::uint32_t>(1024,768))
      , monitorBestOnly_(monitorBestOnly)
      , monitorValidOnly_(monitorValidOnly)
      , observeBoundaries_(false)
      , addPrintCommand_(false)
   { /* nothing */ }

   /***************************************************************************/
   /**
    * The copy constructor
    */
   GProgressPlotterT(const GProgressPlotterT<ind_type, fp_type>& cp)
      : GBasePluggableOMT<ind_type>(cp)
      , gpd_oa_("Progress information", 1, 1) // Not copied
      , fileName_(cp.fileName_)
      , canvasDimensions_(cp.canvasDimensions_)
      , monitorBestOnly_(cp.monitorBestOnly_)
      , monitorValidOnly_(cp.monitorValidOnly_)
      , observeBoundaries_(cp.observeBoundaries_)
      , addPrintCommand_(cp.addPrintCommand_)
   { /* nothing */ }

   /***************************************************************************/
   /**
    * The destuctor
    */
   virtual ~GProgressPlotterT()
   { /* nothing */ }

   /**************************************************************************/
   /**
    * Sets the specifications of the variables to be profiled. Note that
    * boolean and integer variables specified in the argument will simply
    * be ignored.
    */
   void setProfileSpec(std::string parStr) {
      // Check that the parameter string isn't empty
      if(parStr.empty()) {
         glogger
         << "In GPluggableOptimizationMonitorsT<>::setProfileSpec(std::string): Error!" << std::endl
         << "Parameter string " << parStr << " is empty" << std::endl
         << GEXCEPTION;
      }

      //---------------------------------------------------------------------------
      // Clear the parameter vectors
      fp_profVarVec_.clear();

      // Parse the parameter string
      GParameterPropertyParser ppp(parStr);

      //---------------------------------------------------------------------------
      // Retrieve the parameters

      boost::tuple<
         typename std::vector<parPropSpec<fp_type> >::const_iterator
         , typename std::vector<parPropSpec<fp_type> >::const_iterator
      > t_d = ppp.getIterators<fp_type>();

      typename std::vector<parPropSpec<fp_type> >::const_iterator fp_cit = boost::get<0>(t_d);
      typename std::vector<parPropSpec<fp_type> >::const_iterator d_end = boost::get<1>(t_d);
      for(; fp_cit!=d_end; ++fp_cit) { // Note: fp_cit is already set to the begin of the double parameter arrays
         fp_profVarVec_.push_back(*fp_cit);
      }

      //---------------------------------------------------------------------------
   }

   /***************************************************************************/
   /**
    * Allows to specify whether only the best individuals should be monitored.
    */
   void setMonitorBestOnly(bool monitorBestOnly = true) {
      monitorBestOnly_ = monitorBestOnly;
   }

   /***************************************************************************/
   /**
    * Allows to check whether only the best individuals should be monitored.
    */
   bool getMonitorBestOnly() const {
      return monitorBestOnly_;
   }

   /***************************************************************************/
   /**
    * Allows to specify whether only valid individuals should be monitored.
    */
   void setMonitorValidOnly(bool monitorValidOnly = true) {
      monitorValidOnly_ = monitorValidOnly;
   }

   /***************************************************************************/
   /**
    * Allows to check whether only valid individuals should be monitored.
    */
   bool getMonitorValidOnly() const {
      return monitorValidOnly_;
   }

   /***************************************************************************/
   /**
    * Allows to spefify whether scan boundaries should be observed
    */
   void setObserveBoundaries(bool observeBoundaries) {
      observeBoundaries_ = observeBoundaries;
   }

   /***************************************************************************/
   /**
    * Allows to check whether boundaries should be observed
    */
   bool getObserveBoundaries() const {
      return observeBoundaries_;
   }

   /***************************************************************************/
   /**
    * Allows to check whether parameters should be profiled
    */
   bool parameterProfileCreationRequested() const {
      return !fp_profVarVec_.empty();
   }

   /***************************************************************************/
   /**
    * Retrieves the number of variables that will be profiled
    */
   std::size_t nProfileVars() const {
      return fp_profVarVec_.size();
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
    * Allows to set the canvas dimensions using separate x and y values
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

   /******************************************************************************/
   /**
    * Allows to add a "Print" command to the end of the script so that picture files are created
    */
   void setAddPrintCommand(bool addPrintCommand) {
      addPrintCommand_ = addPrintCommand_;
   }

   /******************************************************************************/
   /**
    * Allows to retrieve the current value of the addPrintCommand_ variable
    */
   bool getAddPrintCommand() const {
      return addPrintCommand_;
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
    * Determines a suitable label for a given parPropSpec value
    */
   std::string getLabel(const parPropSpec<fp_type>& s) const {
      std::string result;

      std::size_t var_mode = boost::get<0>(s.var);
      std::string var_name = boost::get<1>(s.var);
      std::size_t var_pos  = boost::get<2>(s.var);

      switch(var_mode) {
         //--------------------------------------------------------------------
         case 0: // parameters are identified by id
         {
            result = std::string("variable id ") + boost::lexical_cast<std::string>(var_pos);
         }
         break;

         //--------------------------------------------------------------------
         case 1:
         {
            result = var_name + "[" + boost::lexical_cast<std::string>(var_pos) + "]";
         }
         break;

         //--------------------------------------------------------------------
         case 2:
         {
            result = var_name;
         }
         break;

         //--------------------------------------------------------------------
         default:
         {
            glogger
            << "In GProgressPlotterT<ind_type, fp_type>::getLabel(): Error" << std::endl
            << "Invalid mode " << var_mode << " requested" << std::endl
            << GEXCEPTION;
         }
         break;

         //--------------------------------------------------------------------
      };

      return result;
   }

   /***************************************************************************/
   /**
    * Allows to emit information in different stages of the information cycle
    * (initialization, during each cycle and during finalization)
    */
   virtual void informationFunction(
      const infoMode& im
      , GOptimizationAlgorithmT<ind_type> * const goa
   ) OVERRIDE {
      switch(im) {
      case Gem::Geneva::INFOINIT:
      {
         switch(this->nProfileVars()) {
            case 1:
            {
               progressPlotter2D_oa_ = boost::shared_ptr<Gem::Common::GGraph2D>(new Gem::Common::GGraph2D());

               progressPlotter2D_oa_->setPlotMode(Gem::Common::CURVE);
               progressPlotter2D_oa_->setPlotLabel("Fitness as a function of a parameter value");
               progressPlotter2D_oa_->setXAxisLabel(this->getLabel(fp_profVarVec_[0]));
               progressPlotter2D_oa_->setYAxisLabel("Fitness");

               gpd_oa_.registerPlotter(progressPlotter2D_oa_);
            }
               break;
            case 2:
            {
               progressPlotter3D_oa_ = boost::shared_ptr<Gem::Common::GGraph3D>(new Gem::Common::GGraph3D());

               progressPlotter3D_oa_->setPlotLabel("Fitness as a function of parameter values");
               progressPlotter3D_oa_->setXAxisLabel(this->getLabel(fp_profVarVec_[0]));
               progressPlotter3D_oa_->setYAxisLabel(this->getLabel(fp_profVarVec_[1]));
               progressPlotter3D_oa_->setZAxisLabel("Fitness");

               gpd_oa_.registerPlotter(progressPlotter3D_oa_);
            }
               break;

            case 3:
            {
               progressPlotter4D_oa_ = boost::shared_ptr<Gem::Common::GGraph4D>(new Gem::Common::GGraph4D());

               progressPlotter4D_oa_->setPlotLabel("Fitness (color-coded) as a function of parameter values");
               progressPlotter4D_oa_->setXAxisLabel(this->getLabel(fp_profVarVec_[0]));
               progressPlotter4D_oa_->setYAxisLabel(this->getLabel(fp_profVarVec_[1]));
               progressPlotter4D_oa_->setZAxisLabel(this->getLabel(fp_profVarVec_[2]));

               gpd_oa_.registerPlotter(progressPlotter4D_oa_);
            }
               break;

            default:
            {
               glogger
               << "NOTE: In GProgressPlotterT<ind_type, fp_type>::informationFunction(INFOINIT):" << std::endl
               << "Number of profiling dimensions " << this->nProfileVars() << " can not be displayed." << std::endl
               << "No graphical output will be created." << std::endl
               << GLOGGING;
            }
            break;
         }

         gpd_oa_.setCanvasDimensions(canvasDimensions_);
      }
         break;

      case Gem::Geneva::INFOPROCESSING:
      {
         bool isDirty = true;
         double fitness;

         if(monitorBestOnly_) { // Monitor the best individuals only
            boost::shared_ptr<GParameterSet> p = goa->GOptimizableI::template getBestIndividual<GParameterSet>();
            if(GBasePluggableOMT<ind_type>::useTrueEvaluation_) {
               fitness = p->trueFitness();
            } else {
               fitness = p->fitness();
            }

            if(!monitorValidOnly_ || p->isValid()) {
               switch(this->nProfileVars()) {
                  case 1:
                  {
                     fp_type val0    = p->GOptimizableEntity::getVarVal<fp_type>(fp_profVarVec_[0].var);

                     if(observeBoundaries_) {
                        if(
                           val0 >= fp_profVarVec_[0].lowerBoundary && val0 <= fp_profVarVec_[0].upperBoundary
                        ) {
                           progressPlotter2D_oa_->add(boost::tuple<double,double>(double(val0), fitness));
                        }
                     } else {
                        progressPlotter2D_oa_->add(boost::tuple<double,double>(double(val0), fitness));
                     }
                  }
                  break;

                  case 2:
                  {
                     fp_type val0 = p->GOptimizableEntity::getVarVal<fp_type>(fp_profVarVec_[0].var);
                     fp_type val1 = p->GOptimizableEntity::getVarVal<fp_type>(fp_profVarVec_[1].var);

                     if(observeBoundaries_) {
                        if(
                           val0 >= fp_profVarVec_[0].lowerBoundary && val0 <= fp_profVarVec_[0].upperBoundary
                           && val1 >= fp_profVarVec_[1].lowerBoundary && val1 <= fp_profVarVec_[1].upperBoundary
                        ) {
                           progressPlotter3D_oa_->add(boost::tuple<double,double,double>(double(val0), double(val1), fitness));
                        }
                     } else {
                        progressPlotter3D_oa_->add(boost::tuple<double,double,double>(double(val0), double(val1), fitness));
                     }
                  }
                  break;

                  case 3:
                  {
                     fp_type val0 = p->GOptimizableEntity::getVarVal<fp_type>(fp_profVarVec_[0].var);
                     fp_type val1 = p->GOptimizableEntity::getVarVal<fp_type>(fp_profVarVec_[1].var);
                     fp_type val2 = p->GOptimizableEntity::getVarVal<fp_type>(fp_profVarVec_[2].var);

                     if(observeBoundaries_) {
                        if(
                           val0 >= fp_profVarVec_[0].lowerBoundary && val0 <= fp_profVarVec_[0].upperBoundary
                           && val1 >= fp_profVarVec_[1].lowerBoundary && val1 <= fp_profVarVec_[1].upperBoundary
                           && val2 >= fp_profVarVec_[2].lowerBoundary && val2 <= fp_profVarVec_[2].upperBoundary
                        ) {
                           progressPlotter4D_oa_->add(boost::tuple<double,double,double,double>(double(val0), double(val1), double(val2), fitness));
                        }
                     } else {
                        progressPlotter4D_oa_->add(boost::tuple<double,double,double,double>(double(val0), double(val1), double(val2), fitness));
                     }
                  }
                  break;

                  default: // Do nothing by default. The number of profiling dimensions is too large
                  break;
               }
            }
         } else { // Monitor all individuals
            typename GOptimizationAlgorithmT<ind_type>::iterator it;
            for(it=goa->begin(); it!=goa->end(); ++it) {

               if(GBasePluggableOMT<ind_type>::useTrueEvaluation_) {
                  fitness = (*it)->trueFitness();
               } else {
                  fitness = (*it)->fitness();
               }

               if(!monitorValidOnly_ || (*it)->isValid()) {
                  switch(this->nProfileVars()) {
                     case 1:
                     {
                        fp_type val0    = (*it)->GOptimizableEntity::template getVarVal<fp_type>(fp_profVarVec_[0].var);

                        if(observeBoundaries_) {
                           if(
                              val0 >= fp_profVarVec_[0].lowerBoundary && val0 <= fp_profVarVec_[0].upperBoundary
                           ) {
                              progressPlotter2D_oa_->add(boost::tuple<double,double>(double(val0), fitness));
                           }
                        } else {
                           progressPlotter2D_oa_->add(boost::tuple<double,double>(double(val0), fitness));
                        }
                     }
                     break;

                     case 2:
                     {
                        fp_type val0 = (*it)->GOptimizableEntity::template getVarVal<fp_type>(fp_profVarVec_[0].var);
                        fp_type val1 = (*it)->GOptimizableEntity::template getVarVal<fp_type>(fp_profVarVec_[1].var);

                        if(observeBoundaries_) {
                           if(
                              val0 >= fp_profVarVec_[0].lowerBoundary && val0 <= fp_profVarVec_[0].upperBoundary
                              && val1 >= fp_profVarVec_[1].lowerBoundary && val1 <= fp_profVarVec_[1].upperBoundary
                           ) {
                              progressPlotter3D_oa_->add(boost::tuple<double,double,double>(double(val0), double(val1), fitness));
                           }
                        } else {
                           progressPlotter3D_oa_->add(boost::tuple<double,double,double>(double(val0), double(val1), fitness));
                        }
                     }
                     break;

                     case 3:
                     {
                        fp_type val0 = (*it)->GOptimizableEntity::template getVarVal<fp_type>(fp_profVarVec_[0].var);
                        fp_type val1 = (*it)->GOptimizableEntity::template getVarVal<fp_type>(fp_profVarVec_[1].var);
                        fp_type val2 = (*it)->GOptimizableEntity::template getVarVal<fp_type>(fp_profVarVec_[2].var);

                        if(observeBoundaries_) {
                           if(
                              val0 >= fp_profVarVec_[0].lowerBoundary && val0 <= fp_profVarVec_[0].upperBoundary
                              && val1 >= fp_profVarVec_[1].lowerBoundary && val1 <= fp_profVarVec_[1].upperBoundary
                              && val2 >= fp_profVarVec_[2].lowerBoundary && val2 <= fp_profVarVec_[2].upperBoundary
                           ) {
                              progressPlotter4D_oa_->add(boost::tuple<double,double,double,double>(double(val0), double(val1), double(val2), fitness));
                           }
                        } else {
                           progressPlotter4D_oa_->add(boost::tuple<double,double,double,double>(double(val0), double(val1), double(val2), fitness));
                        }
                     }
                     break;

                     default: // Do nothing by default. The number of profiling dimensions is too large
                     break;
                  }
               }
            }
         }
      }
         break;

      case Gem::Geneva::INFOEND:
      {
         // Make sure 1-D data is sorted
         if(1 == this->nProfileVars()) {
            progressPlotter2D_oa_->sortX();
         }

         // Inform the plot designer whether it should print png files
         gpd_oa_.setAddPrintCommand(addPrintCommand_);

         // Write out the result. Note that we add
         gpd_oa_.writeToFile(fileName_);

         // Remove all plotters
         gpd_oa_.resetPlotters();
         progressPlotter2D_oa_.reset();
         progressPlotter3D_oa_.reset();
         progressPlotter4D_oa_.reset();
      }
         break;

      default:
      {
         glogger
         << "In GProgressPlotterT<ind_type, fp_type>::informationFunction(): Received invalid infoMode " << im << std::endl
         << GEXCEPTION;
      }
      break;
      };
   }

private:
   std::vector<parPropSpec<fp_type> > fp_profVarVec_; ///< Holds information about variables to be profiled

   Gem::Common::GPlotDesigner gpd_oa_; ///< A wrapper for the plots

   // These are temporaries
   boost::shared_ptr<Gem::Common::GGraph2D> progressPlotter2D_oa_;
   boost::shared_ptr<Gem::Common::GGraph3D> progressPlotter3D_oa_;
   boost::shared_ptr<Gem::Common::GGraph4D> progressPlotter4D_oa_;

   std::string fileName_; ///< The name of the file the output should be written to. Note that the class will add the name of the algorithm it acts on
   boost::tuple<boost::uint32_t,boost::uint32_t> canvasDimensions_; ///< The dimensions of the canvas

   bool monitorBestOnly_;  ///< Indicates whether only the best individuals should be monitored
   bool monitorValidOnly_; ///< Indicates whether only valid individuals should be plotted
   bool observeBoundaries_; ///< When set to true, the plotter will ignore values outside of a scan boundary

   bool addPrintCommand_; ///< Asks the GPlotDesigner to add a print command to result files
};

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/
/**
 * This class allows to log all candidate solutions found to a file. NOTE that
 * the file may become very large! Results are output in the following format:
 * param1 param2 ... param_m eval1 eval2 ... eval_n . By default, no commas and
 * explanations are printed. If withNameAndType is set to true, the values are
 * prepended by a line with variable names and types. If withCommas is set to true,
 * commas will be printed in-between values. It is possible to filter the results by
 * asking the class to only log solutions better than a given set of values. What
 * is considered better depends on whether evaluation criteria are maximized or minimized
 * and is determined from the individual. Note that this class can only be instantiated
 * if ind_type is either a derivative of GParamterSet or is an object of the
 * GParameterSet class itself.
 */
template <typename ind_type>
class GAllSolutionFileLoggerT : public GBasePluggableOMT<ind_type>
{
   // Make sure this class can only be instantiated if ind_type is a derivative of GParameterSet
   BOOST_MPL_ASSERT((boost::is_base_of<GParameterSet, ind_type>));

public:
   /***************************************************************************/
   /**
    * The default constructor
    */
   GAllSolutionFileLoggerT()
      : fileName_("CompleteSolutionLog.txt")
      , boundariesActive_(false)
      , withNameAndType_(false)
      , withCommas_(false)
   { /* nothing */ }

   /***************************************************************************/
   /**
    * Initialization with a file name
    */
   GAllSolutionFileLoggerT(const std::string& fileName)
      : fileName_(fileName)
      , boundariesActive_(false)
      , withNameAndType_(false)
      , withCommas_(false)
   { /* nothing */ }

   /***************************************************************************/
   /**
    * Initialization with a file name and boundaries
    */
   GAllSolutionFileLoggerT(
      const std::string& fileName
      , const std::vector<double>& boundaries
   )
      : fileName_(fileName)
      , boundaries_(boundaries)
      , boundariesActive_(true)
      , withNameAndType_(false)
      , withCommas_(false)
   { /* nothing */ }

   /***************************************************************************/
   /**
    * The copy constructor
    */
   GAllSolutionFileLoggerT(const GAllSolutionFileLoggerT<ind_type>& cp)
      : fileName_(cp.fileName_)
      , boundaries_(cp.boundaries_)
      , boundariesActive_(cp.boundariesActive_)
      , withNameAndType_(cp.withNameAndType_)
      , withCommas_(cp.withCommas_)
   { /* nothing */ }

   /***************************************************************************/
   /**
    * The destructor
    */
   virtual ~GAllSolutionFileLoggerT()
   { /* nothing */ }

   /***************************************************************************/
   /**
    * Sets the file name
    */
   void setFileName(std::string fileName) {
      fileName_ = fileName;
   }

   /***************************************************************************/
   /**
    * Retrieves the current file name
    */
   std::string getFileName() const {
      return fileName_;
   }

   /***************************************************************************/
   /**
    * Sets the boundaries
    */
   void setBoundaries(std::vector<double> boundaries) {
      boundaries_ = boundaries;
      boundariesActive_ = true;
   }

   /***************************************************************************/
   /**
    * Allows to retrieve the boundaries
    */
   std::vector<double> getBoundaries() const {
      return boundaries_;
   }

   /***************************************************************************/
   /**
    * Allows to check whether boundaries are active
    */
   bool boundariesActive() const {
      return boundariesActive_;
   }

   /***************************************************************************/
   /**
    * Allows to inactivate boundaries
    */
   void setBoundariesInactive() {
      boundariesActive_ = false;
   }

   /***************************************************************************/
   /**
    * Allows to specify whether explanations should be printed for parameter-
    * and fitness values.
    */
   void setPrintWithNameAndType(bool withNameAndType) {
      withNameAndType_ = withNameAndType;
   }

   /***************************************************************************/
   /**
    * Allows to check whether explanations should be printed for parameter-
    * and fitness values
    */
   bool getPrintWithNameAndType() const {
      return withNameAndType_;
   }

   /***************************************************************************/
   /**
    * Allows to specify whether commas should be printed in-between values
    */
   void setPrintWithCommas(bool withCommas) {
      withCommas_ = withCommas;
   }

   /***************************************************************************/
   /**
    * Allows to check whether commas should be printed in-between values
    */
   bool getPrintWithCommas() const {
      return withCommas_;
   }


   /***************************************************************************/
   /**
    * Allows to emit information in different stages of the information cycle
    * (initialization, during each cycle and during finalization)
    */
   virtual void informationFunction(
      const infoMode& im
      , GOptimizationAlgorithmT<ind_type> * const goa
   ) OVERRIDE {
      switch(im) {
      case Gem::Geneva::INFOINIT:
      {
         // If the file pointed to by fileName_ already exists, make a back-up
         if(bf::exists(fileName_)) {
            /*
            std::ostringstream timeStringStream;
            const boost::posix_time::ptime currentTime = boost::posix_time::second_clock::local_time();
            const boost::posix_time::time_facet *f = new boost::posix_time::time_facet("%H-%M-%S");
            timeStringStream.imbue(std::locale(timeStringStream.getloc(),f));

            std::string newFileName = fileName_ + ".bak" + timeStringStream.str();

            std::cout << "New file name is " << timeStringStream.str() << std::endl;
             */

            const boost::posix_time::ptime currentTime = boost::posix_time::second_clock::local_time();
            std::string newFileName = fileName_ + ".bak_" + boost::lexical_cast<std::string>(currentTime);

            glogger
            << "In GAllSolutionFileLoggerT<T>::informationFunction(): Error!" << std::endl
            << "Attempt to output information to file " << fileName_ << std::endl
            << "which already exists. We will rename the old file to" << std::endl
            << newFileName << std::endl
            << GWARNING;

            bf::rename(fileName_, newFileName);
         }
      }
      break;

      case Gem::Geneva::INFOPROCESSING:
      {
         // Open the external file
         std::ofstream data(fileName_.c_str(), std::ofstream::app);

         // Loop over all individuals of the algorithm.
         for(std::size_t pos=0; pos<goa->size(); pos++) {
            boost::shared_ptr<GParameterSet> ind = goa->GOptimizationAlgorithmT<ind_type>::template individual_cast<GParameterSet>(pos);

            // Note that isGoodEnough may throw if loop acts on a "dirty" individual
            if(!boundariesActive_ || ind->isGoodEnough(boundaries_)) {
               // Append the data to the external file
               data << ind->toCSV(withNameAndType_, withCommas_);
            }
         }

         // Close the external file
         data.close();
      }
      break;

      case Gem::Geneva::INFOEND:
      // nothing
      break;
      };
   }

private:
   /***************************************************************************/

   std::string fileName_; ///< The name of the file to which solutions should be stored
   std::vector<double> boundaries_; ///< Value boundaries used to filter logged solutions
   bool boundariesActive_; ///< Set to true if boundaries have been set
   bool withNameAndType_; ///< When set to true, explanations for values are printed
   bool withCommas_; ///< When set to true, commas will be printed in-between values
};

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/

} /* namespace Geneva */
} /* namespace Gem */

#endif /* GPLUGGABLEOPTIMIZATIONMONITORST_HPP_ */
