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

// Global checks, defines and includes needed for all of Geneva
#include "common/GGlobalDefines.hpp"

// Standard header files go here
#include <string>
#include <fstream>

// Boost header files go here
#include <boost/shared_ptr.hpp>
#include <boost/mpl/assert.hpp>
#include <boost/type_traits.hpp>
#include <boost/filesystem.hpp>
#include <boost/filesystem/fstream.hpp>
#include <boost/filesystem/operations.hpp>
#include <boost/filesystem/convenience.hpp>
#include <boost/date_time.hpp>

#ifndef GPLUGGABLEOPTIMIZATIONMONITORST_HPP_
#define GPLUGGABLEOPTIMIZATIONMONITORST_HPP_

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
   G_API_GENEVA GBasePluggableOMT()
      : useRawEvaluation_(false)
   { /* nothing */ }

   /***************************************************************************/
   /**
    * The copy constructor
    */
   G_API_GENEVA GBasePluggableOMT(const GBasePluggableOMT<ind_type>& cp)
      : useRawEvaluation_(cp.useRawEvaluation_)
   { /* nothing */ }

   /***************************************************************************/
   /**
    * The Destructor
    */
   virtual G_API_GENEVA ~GBasePluggableOMT()
   { /* nothing */ }

   /***************************************************************************/
   /**
    * Overload this function in derived classes, specifying actions for
    * initialization, the optimization cycles and finalization.
    */
   virtual G_API_GENEVA void informationFunction(
      const infoMode& im
      , GOptimizationAlgorithmT<ind_type> * const goa
   ) BASE = 0;

   /***************************************************************************/
   /**
    * Allows to set the useRawEvaluation_ variable
    */
   G_API_GENEVA void setUseRawEvaluation(bool useRaw) {
      useRawEvaluation_ = useRaw;
   }

   /***************************************************************************/
   /**
    * Allows to retrieve the value of the useRawEvaluation_ variable
    */
   G_API_GENEVA bool getUseRawEvaluation() const {
      return useRawEvaluation_;
   }

protected:
   /***************************************************************************/
   bool useRawEvaluation_; ///< Specifies whether the true (unmodified) evaluation should be used
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
   G_API_GENEVA GCollectiveMonitorT()
   { /* nothing */ }

   /***************************************************************************/
   /**
    * The copy constructor
    */
   G_API_GENEVA GCollectiveMonitorT(const GCollectiveMonitorT<ind_type>& cp)
   : GBasePluggableOMT<ind_type>(cp)
   { /* nothing */ }

   /***************************************************************************/
   /**
    * The destructor
    */
   virtual G_API_GENEVA ~GCollectiveMonitorT()
   { /* nothing */ }

   /***************************************************************************/
   /**
    * Aggregates the work of all registered pluggable monitors
    */
   virtual G_API_GENEVA void informationFunction(
      const infoMode& im
      , GOptimizationAlgorithmT<ind_type> * const goa
   ) OVERRIDE {
      typename std::vector<boost::shared_ptr<GBasePluggableOMT<ind_type> > >::iterator it;
      for(it=pluggable_monitors_.begin(); it!=pluggable_monitors_.end(); ++it) {
         (*it)->informationFunction(im,goa);
      }
   }

   /***************************************************************************/
   /**
    * Allows to register a new pluggable monitor
    */
   G_API_GENEVA void registerPluggableOM(boost::shared_ptr<GBasePluggableOMT<ind_type> > om_ptr) {
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
    * Checks if adaptors have been registered in the collective monitor
    */
   G_API_GENEVA bool hasOptimizationMonitors() const {
      return !pluggable_monitors_.empty();
   }

   /***************************************************************************/
   /**
    * Allows to clear all registered monitors
    */
   G_API_GENEVA void reset() {
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
   G_API_GENEVA GProgressPlotterT()
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
   G_API_GENEVA GProgressPlotterT(bool monitorBestOnly, bool monitorValidOnly)
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
   G_API_GENEVA GProgressPlotterT(const GProgressPlotterT<ind_type, fp_type>& cp)
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
   virtual G_API_GENEVA ~GProgressPlotterT()
   { /* nothing */ }

   /**************************************************************************/
   /**
    * Sets the specifications of the variables to be profiled. Note that
    * boolean and integer variables specified in the argument will simply
    * be ignored.
    */
   G_API_GENEVA void setProfileSpec(std::string parStr) {
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
   G_API_GENEVA void setMonitorBestOnly(bool monitorBestOnly = true) {
      monitorBestOnly_ = monitorBestOnly;
   }

   /***************************************************************************/
   /**
    * Allows to check whether only the best individuals should be monitored.
    */
   G_API_GENEVA bool getMonitorBestOnly() const {
      return monitorBestOnly_;
   }

   /***************************************************************************/
   /**
    * Allows to specify whether only valid individuals should be monitored.
    */
   G_API_GENEVA void setMonitorValidOnly(bool monitorValidOnly = true) {
      monitorValidOnly_ = monitorValidOnly;
   }

   /***************************************************************************/
   /**
    * Allows to check whether only valid individuals should be monitored.
    */
   G_API_GENEVA bool getMonitorValidOnly() const {
      return monitorValidOnly_;
   }

   /***************************************************************************/
   /**
    * Allows to spefify whether scan boundaries should be observed
    */
   G_API_GENEVA void setObserveBoundaries(bool observeBoundaries) {
      observeBoundaries_ = observeBoundaries;
   }

   /***************************************************************************/
   /**
    * Allows to check whether boundaries should be observed
    */
   G_API_GENEVA bool getObserveBoundaries() const {
      return observeBoundaries_;
   }

   /***************************************************************************/
   /**
    * Allows to check whether parameters should be profiled
    */
   G_API_GENEVA bool parameterProfileCreationRequested() const {
      return !fp_profVarVec_.empty();
   }

   /***************************************************************************/
   /**
    * Retrieves the number of variables that will be profiled
    */
   G_API_GENEVA std::size_t nProfileVars() const {
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
   G_API_GENEVA void setCanvasDimensions(boost::uint32_t x, boost::uint32_t y) {
      canvasDimensions_ = boost::tuple<boost::uint32_t,boost::uint32_t>(x,y);
   }

   /***************************************************************************/
   /**
    * Gives access to the canvas dimensions
    */
   G_API_GENEVA boost::tuple<boost::uint32_t,boost::uint32_t> getCanvasDimensions() const {
      return canvasDimensions_;
   }

   /******************************************************************************/
   /**
    * Allows to add a "Print" command to the end of the script so that picture files are created
    */
   G_API_GENEVA void setAddPrintCommand(bool addPrintCommand) {
      addPrintCommand_ = addPrintCommand;
   }

   /******************************************************************************/
   /**
    * Allows to retrieve the current value of the addPrintCommand_ variable
    */
   G_API_GENEVA bool getAddPrintCommand() const {
      return addPrintCommand_;
   }

   /***************************************************************************/
   /**
    * Allows to set the filename
    */
   G_API_GENEVA void setFileName(std::string fileName) {
      fileName_ = fileName;
   }

   /***************************************************************************/
   /**
    * Retrieves the current filename to which information will be emitted
    */
   G_API_GENEVA std::string getFileName() const {
      return fileName_;
   }

   /***************************************************************************/
   /**
    * Allows to set the canvas label
    */
   G_API_GENEVA void setCanvasLabel(const std::string& canvasLabel) {
      gpd_oa_.setCanvasLabel(canvasLabel);
   }

   /***************************************************************************/
   /**
    * Allows to retrieve the canvas label
    */
   G_API_GENEVA std::string getCanvasLabel() const {
      return gpd_oa_.getCanvasLabel();
   }

   /***************************************************************************/
   /**
    * Determines a suitable label for a given parPropSpec value
    */
   G_API_GENEVA std::string getLabel(const parPropSpec<fp_type>& s) const {
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
   virtual G_API_GENEVA void informationFunction(
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
         double primaryFitness;

         if(monitorBestOnly_) { // Monitor the best individuals only
            boost::shared_ptr<GParameterSet> p = goa->GOptimizableI::template getBestIndividual<GParameterSet>();
            if(GBasePluggableOMT<ind_type>::useRawEvaluation_) {
               primaryFitness = p->fitness(0, PREVENTREEVALUATION, USERAWFITNESS);
            } else {
               primaryFitness = p->transformedFitness();
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
                           progressPlotter2D_oa_->add(boost::tuple<double,double>(double(val0), primaryFitness));
                        }
                     } else {
                        progressPlotter2D_oa_->add(boost::tuple<double,double>(double(val0), primaryFitness));
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
                           progressPlotter3D_oa_->add(boost::tuple<double,double,double>(double(val0), double(val1), primaryFitness));
                        }
                     } else {
                        progressPlotter3D_oa_->add(boost::tuple<double,double,double>(double(val0), double(val1), primaryFitness));
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
                           progressPlotter4D_oa_->add(boost::tuple<double,double,double,double>(double(val0), double(val1), double(val2), primaryFitness));
                        }
                     } else {
                        progressPlotter4D_oa_->add(boost::tuple<double,double,double,double>(double(val0), double(val1), double(val2), primaryFitness));
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

               if(GBasePluggableOMT<ind_type>::useRawEvaluation_) {
                  primaryFitness = (*it)->fitness(0, PREVENTREEVALUATION, USERAWFITNESS);
               } else {
                  primaryFitness = (*it)->fitness(0, PREVENTREEVALUATION, USETRANSFORMEDFITNESS);
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
                              progressPlotter2D_oa_->add(boost::tuple<double,double>(double(val0), primaryFitness));
                           }
                        } else {
                           progressPlotter2D_oa_->add(boost::tuple<double,double>(double(val0), primaryFitness));
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
                              progressPlotter3D_oa_->add(boost::tuple<double,double,double>(double(val0), double(val1), primaryFitness));
                           }
                        } else {
                           progressPlotter3D_oa_->add(boost::tuple<double,double,double>(double(val0), double(val1), primaryFitness));
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
                              progressPlotter4D_oa_->add(boost::tuple<double,double,double,double>(double(val0), double(val1), double(val2), primaryFitness));
                           }
                        } else {
                           progressPlotter4D_oa_->add(boost::tuple<double,double,double,double>(double(val0), double(val1), double(val2), primaryFitness));
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
 * This class allows to log all candidate solutions found to a file, including the parameetr
 * values. NOTE that the file may become very large! Results are output in the following format:
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
   G_API_GENEVA GAllSolutionFileLoggerT()
      : fileName_("CompleteSolutionLog.txt")
      , boundariesActive_(false)
      , withNameAndType_(false)
      , withCommas_(false)
      , useRawFitness_(true)
      , showValidity_(true)
   { /* nothing */ }

   /***************************************************************************/
   /**
    * Initialization with a file name
    */
   G_API_GENEVA GAllSolutionFileLoggerT(const std::string& fileName)
      : fileName_(fileName)
      , boundariesActive_(false)
      , withNameAndType_(false)
      , withCommas_(false)
      , useRawFitness_(true)
      , showValidity_(true)
   { /* nothing */ }

   /***************************************************************************/
   /**
    * Initialization with a file name and boundaries
    */
   G_API_GENEVA GAllSolutionFileLoggerT(
      const std::string& fileName
      , const std::vector<double>& boundaries
   )
      : fileName_(fileName)
      , boundaries_(boundaries)
      , boundariesActive_(true)
      , withNameAndType_(false)
      , withCommas_(false)
      , useRawFitness_(true)
      , showValidity_(true)
   { /* nothing */ }

   /***************************************************************************/
   /**
    * The copy constructor
    */
   G_API_GENEVA GAllSolutionFileLoggerT(const GAllSolutionFileLoggerT<ind_type>& cp)
      : fileName_(cp.fileName_)
      , boundaries_(cp.boundaries_)
      , boundariesActive_(cp.boundariesActive_)
      , withNameAndType_(cp.withNameAndType_)
      , withCommas_(cp.withCommas_)
      , useRawFitness_(cp.useRawFitness_)
      , showValidity_(cp.showValidity_)
   { /* nothing */ }

   /***************************************************************************/
   /**
    * The destructor
    */
   virtual G_API_GENEVA ~GAllSolutionFileLoggerT()
   { /* nothing */ }

   /***************************************************************************/
   /**
    * Sets the file name
    */
   G_API_GENEVA void setFileName(std::string fileName) {
      fileName_ = fileName;
   }

   /***************************************************************************/
   /**
    * Retrieves the current file name
    */
   G_API_GENEVA std::string getFileName() const {
      return fileName_;
   }

   /***************************************************************************/
   /**
    * Sets the boundaries
    */
   G_API_GENEVA void setBoundaries(std::vector<double> boundaries) {
      boundaries_ = boundaries;
      boundariesActive_ = true;
   }

   /***************************************************************************/
   /**
    * Allows to retrieve the boundaries
    */
   G_API_GENEVA std::vector<double> getBoundaries() const {
      return boundaries_;
   }

   /***************************************************************************/
   /**
    * Allows to check whether boundaries are active
    */
   G_API_GENEVA bool boundariesActive() const {
      return boundariesActive_;
   }

   /***************************************************************************/
   /**
    * Allows to inactivate boundaries
    */
   G_API_GENEVA void setBoundariesInactive() {
      boundariesActive_ = false;
   }

   /***************************************************************************/
   /**
    * Allows to specify whether explanations should be printed for parameter-
    * and fitness values.
    */
   G_API_GENEVA void setPrintWithNameAndType(bool withNameAndType) {
      withNameAndType_ = withNameAndType;
   }

   /***************************************************************************/
   /**
    * Allows to check whether explanations should be printed for parameter-
    * and fitness values
    */
   G_API_GENEVA bool getPrintWithNameAndType() const {
      return withNameAndType_;
   }

   /***************************************************************************/
   /**
    * Allows to specify whether commas should be printed in-between values
    */
   G_API_GENEVA void setPrintWithCommas(bool withCommas) {
      withCommas_ = withCommas;
   }

   /***************************************************************************/
   /**
    * Allows to check whether commas should be printed in-between values
    */
   G_API_GENEVA bool getPrintWithCommas() const {
      return withCommas_;
   }

   /***************************************************************************/
   /**
    * Allows to specify whether the true (instead of the transformed) fitness should be shown
    */
   G_API_GENEVA void setUseTrueFitness(bool useRawFitness) {
      useRawFitness_ = useRawFitness;
   }

   /***************************************************************************/
   /**
    * Allows to retrieve whether the true (instead of the transformed) fitness should be shown
    */
   G_API_GENEVA bool getUseTrueFitness() const {
      return useRawFitness_;
   }

   /***************************************************************************/
   /**
    * Allows to specify whether the validity of a solution should be shown
    */
   G_API_GENEVA void setShowValidity(bool showValidity) {
      showValidity_ = showValidity;
   }

   /***************************************************************************/
   /**
    * Allows to check whether the validity of a solution will be shown
    */
   G_API_GENEVA bool getShowValidity() const {
      return showValidity_;
   }

   /***************************************************************************/
   /**
    * Allows to emit information in different stages of the information cycle
    * (initialization, during each cycle and during finalization)
    */
   virtual G_API_GENEVA void informationFunction(
      const infoMode& im
      , GOptimizationAlgorithmT<ind_type> * const goa
   ) OVERRIDE {
      switch(im) {
      case Gem::Geneva::INFOINIT:
      {
         // If the file pointed to by fileName_ already exists, make a back-up
         if(bf::exists(fileName_)) {
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
         boost::filesystem::ofstream data(fileName_, std::ofstream::app);

         // Loop over all individuals of the algorithm.
         for(std::size_t pos=0; pos<goa->size(); pos++) {
            boost::shared_ptr<GParameterSet> ind = goa->template individual_cast<GParameterSet>(pos);

            // Note that isGoodEnough may throw if loop acts on a "dirty" individual
            if(!boundariesActive_ || ind->isGoodEnough(boundaries_)) {
               // Append the data to the external file
               if(0 == pos && goa->inFirstIteration()) { // Only output name and type in the very first line (if at all)
                  data << ind->toCSV(withNameAndType_, withCommas_, useRawFitness_, showValidity_);
               } else {
                  data << ind->toCSV(false, withCommas_, useRawFitness_, showValidity_);
               }
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
   bool useRawFitness_; ///< Indicates whether true- or transformed fitness should be output
   bool showValidity_; ///< Indicates whether the validity of a solution should be shown
};

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/
/**
 * This class prints out all evaluations of each iteration. The format is
 * eval0_0, eval0_1, ... ,eval0_n, ..., evalm_0, evalm_1, ... ,evalm_n
 */
template <typename ind_type>
class GIterationResultsFileLoggerT : public GBasePluggableOMT<ind_type>
{
   // Make sure this class can only be instantiated if ind_type is a derivative of GParameterSet
   BOOST_MPL_ASSERT((boost::is_base_of<GParameterSet, ind_type>));

public:
   /***************************************************************************/
   /**
    * The default constructor
    */
   G_API_GENEVA GIterationResultsFileLoggerT()
      : fileName_("CompleteSolutionLog.txt")
      , withCommas_(false)
      , useRawFitness_(true)
   { /* nothing */ }

   /***************************************************************************/
   /**
    * Initialization with a file name
    */
   G_API_GENEVA GIterationResultsFileLoggerT(const std::string& fileName)
      : fileName_(fileName)
      , withCommas_(false)
      , useRawFitness_(true)
   { /* nothing */ }

   /***************************************************************************/
   /**
    * The copy constructor
    */
   G_API_GENEVA GIterationResultsFileLoggerT(const GIterationResultsFileLoggerT<ind_type>& cp)
      : fileName_(cp.fileName_)
      , withCommas_(cp.withCommas_)
      , useRawFitness_(cp.useRawFitness_)
   { /* nothing */ }

   /***************************************************************************/
   /**
    * The destructor
    */
   virtual G_API_GENEVA ~GIterationResultsFileLoggerT()
   { /* nothing */ }

   /***************************************************************************/
   /**
    * Sets the file name
    */
   G_API_GENEVA void setFileName(std::string fileName) {
      fileName_ = fileName;
   }

   /***************************************************************************/
   /**
    * Retrieves the current file name
    */
   G_API_GENEVA std::string getFileName() const {
      return fileName_;
   }

   /***************************************************************************/
   /**
    * Allows to specify whether commas should be printed in-between values
    */
   G_API_GENEVA void setPrintWithCommas(bool withCommas) {
      withCommas_ = withCommas;
   }

   /***************************************************************************/
   /**
    * Allows to check whether commas should be printed in-between values
    */
   G_API_GENEVA bool getPrintWithCommas() const {
      return withCommas_;
   }

   /***************************************************************************/
   /**
    * Allows to specify whether the true (instead of the transformed) fitness should be shown
    */
   G_API_GENEVA void setUseTrueFitness(bool useRawFitness) {
      useRawFitness_ = useRawFitness;
   }

   /***************************************************************************/
   /**
    * Allows to retrieve whether the true (instead of the transformed) fitness should be shown
    */
   G_API_GENEVA bool getUseTrueFitness() const {
      return useRawFitness_;
   }

   /***************************************************************************/
   /**
    * Allows to emit information in different stages of the information cycle
    * (initialization, during each cycle and during finalization)
    */
   virtual G_API_GENEVA void informationFunction(
      const infoMode& im
      , GOptimizationAlgorithmT<ind_type> * const goa
   ) OVERRIDE {
      switch(im) {
      case Gem::Geneva::INFOINIT:
      {
         // If the file pointed to by fileName_ already exists, make a back-up
         if(bf::exists(fileName_)) {
            const boost::posix_time::ptime currentTime = boost::posix_time::second_clock::local_time();
            std::string newFileName = fileName_ + ".bak_" + boost::lexical_cast<std::string>(currentTime);

            glogger
            << "In GIterationResultsFileLoggerT<T>::informationFunction(): Warning!" << std::endl
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
         boost::filesystem::ofstream data(fileName_.c_str(), std::ofstream::app);
         std::vector<double> fitnessVec;

         // Loop over all individuals of the algorithm.
         std::size_t nIndividuals = goa->size();
         for(std::size_t pos=0; pos<nIndividuals; pos++) {
            boost::shared_ptr<GParameterSet> ind = goa->template individual_cast<GParameterSet>(pos);
            fitnessVec = goa->at(pos)->fitnessVec(useRawFitness_);

            std::size_t nFitnessCriteria = goa->getNumberOfFitnessCriteria();
            for(std::size_t i=0; i<nFitnessCriteria; i++) {
               data << fitnessVec.at(i) << ((withCommas_ && (nFitnessCriteria*nIndividuals > (i+1)*(pos+1)))?", ":" ");
            }
         }
         data << std::endl;

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
   bool withCommas_; ///< When set to true, commas will be printed in-between values
   bool useRawFitness_; ///< Indicates whether true- or transformed fitness should be output
};

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/
/**
 * This class allows to log the number of adaptions made inside of adaptors
 * to a file. This is mostly needed for debugging and profiling purposes. The
 * number of adaptions made is a good measure for the adaption probability.
 */
template <typename ind_type>
class GNAdpationsLoggerT : public GBasePluggableOMT<ind_type>
{
   // Make sure this class can only be instantiated if ind_type is a derivative of GParameterSet
   BOOST_MPL_ASSERT((boost::is_base_of<GParameterSet, ind_type>));

public:
   /***************************************************************************/
   /**
    * The default constructor
    */
   G_API_GENEVA GNAdpationsLoggerT()
      : fileName_("NAdaptions.C")
      , canvasDimensions_(boost::tuple<boost::uint32_t,boost::uint32_t>(1200,1600))
      , gpd_oa_("Number of adaptions per iteration", 1, 2)
      , monitorBestOnly_(false)
      , addPrintCommand_(false)
      , maxIteration_(0)
      , nIterationsRecorded_(0)
      , nAdaptionsStore_()
   { /* nothing */ }

   /***************************************************************************/
   /**
    * Initialization with a file name
    */
   explicit G_API_GENEVA GNAdpationsLoggerT(const std::string& fileName)
      : fileName_(fileName)
      , canvasDimensions_(boost::tuple<boost::uint32_t,boost::uint32_t>(1200,1600))
      , gpd_oa_("Number of adaptions per iteration", 1, 2)
      , monitorBestOnly_(false)
      , addPrintCommand_(false)
      , maxIteration_(0)
      , nIterationsRecorded_(0)
      , nAdaptionsStore_()
   { /* nothing */ }

   /***************************************************************************/
   /**
    * The copy constructor
    */
   G_API_GENEVA GNAdpationsLoggerT(const GNAdpationsLoggerT<ind_type>& cp)
      : fileName_(cp.fileName_)
      , canvasDimensions_(cp.canvasDimensions_)
      , gpd_oa_("Number of adaptions per iteration", 1, 2) // Not copied
      , monitorBestOnly_(cp.monitorBestOnly_)
      , addPrintCommand_(cp.addPrintCommand_)
      , maxIteration_(cp.maxIteration_)
      , nIterationsRecorded_(cp.nIterationsRecorded_)
      , nAdaptionsStore_(cp.nAdaptionsStore_)
   { /* nothing */ }

   /***************************************************************************/
   /**
    * The destructor
    */
   virtual G_API_GENEVA ~GNAdpationsLoggerT()
   { /* nothing */ }

   /***************************************************************************/
   /**
    * Sets the file name
    */
   G_API_GENEVA void setFileName(std::string fileName) {
      fileName_ = fileName;
   }

   /***************************************************************************/
   /**
    * Retrieves the current file name
    */
   G_API_GENEVA std::string getFileName() const {
      return fileName_;
   }

   /***************************************************************************/
   /**
    * Allows to specify whether only the best individuals should be monitored.
    */
   G_API_GENEVA void setMonitorBestOnly(bool monitorBestOnly = true) {
      monitorBestOnly_ = monitorBestOnly;
   }

   /***************************************************************************/
   /**
    * Allows to check whether only the best individuals should be monitored.
    */
   G_API_GENEVA bool getMonitorBestOnly() const {
      return monitorBestOnly_;
   }

   /***************************************************************************/
   /**
    * Allows to set the canvas dimensions
    */
   G_API_GENEVA void setCanvasDimensions(boost::tuple<boost::uint32_t,boost::uint32_t> canvasDimensions) {
      canvasDimensions_ = canvasDimensions;
   }

   /***************************************************************************/
   /**
    * Allows to set the canvas dimensions using separate x and y values
    */
   G_API_GENEVA void setCanvasDimensions(boost::uint32_t x, boost::uint32_t y) {
      canvasDimensions_ = boost::tuple<boost::uint32_t,boost::uint32_t>(x,y);
   }

   /***************************************************************************/
   /**
    * Gives access to the canvas dimensions
    */
   G_API_GENEVA boost::tuple<boost::uint32_t,boost::uint32_t> getCanvasDimensions() const {
      return canvasDimensions_;
   }

   /******************************************************************************/
   /**
    * Allows to add a "Print" command to the end of the script so that picture files are created
    */
   G_API_GENEVA void setAddPrintCommand(bool addPrintCommand) {
      addPrintCommand_ = addPrintCommand;
   }

   /******************************************************************************/
   /**
    * Allows to retrieve the current value of the addPrintCommand_ variable
    */
   G_API_GENEVA bool getAddPrintCommand() const {
      return addPrintCommand_;
   }

   /***************************************************************************/
   /**
    * Allows to emit information in different stages of the information cycle
    * (initialization, during each cycle and during finalization)
    */
   virtual G_API_GENEVA void informationFunction(
      const infoMode& im
      , GOptimizationAlgorithmT<ind_type> * const goa
   ) OVERRIDE {
      using namespace Gem::Common;

      switch(im) {
      case Gem::Geneva::INFOINIT:
      {
         // If the file pointed to by fileName_ already exists, make a back-up
         if(bf::exists(fileName_)) {
            const boost::posix_time::ptime currentTime = boost::posix_time::second_clock::local_time();
            std::string newFileName = fileName_ + ".bak_" + boost::lexical_cast<std::string>(currentTime);

            glogger
            << "In GNAdpationsLoggerT<T>::informationFunction(): Error!" << std::endl
            << "Attempt to output information to file " << fileName_ << std::endl
            << "which already exists. We will rename the old file to" << std::endl
            << newFileName << std::endl
            << GWARNING;

            bf::rename(fileName_, newFileName);
         }

         // Make sure the progress plotter has the desired size
         gpd_oa_.setCanvasDimensions(canvasDimensions_);

         // Set up a graph to monitor the best fitness found
         fitnessGraph2D_oa_ = boost::shared_ptr<Gem::Common::GGraph2D>(new Gem::Common::GGraph2D());
         fitnessGraph2D_oa_->setXAxisLabel("Iteration");
         fitnessGraph2D_oa_->setYAxisLabel("Fitness");
         fitnessGraph2D_oa_->setPlotMode(Gem::Common::CURVE);
      }
      break;

      case Gem::Geneva::INFOPROCESSING:
      {
         boost::uint32_t iteration = goa->getIteration();

         // Record the current fitness
         boost::shared_ptr<GParameterSet> p = goa->GOptimizableI::template getBestIndividual<GParameterSet>();
         (*fitnessGraph2D_oa_) & boost::tuple<double,double>(double(iteration), double(p->fitness()));

         // Update the largest known iteration and the number of recorded iterations
         maxIteration_ = iteration;
         nIterationsRecorded_++;

         // Do the actual logging
         if(monitorBestOnly_) {
            boost::shared_ptr<GParameterSet> best = goa->GOptimizableI::template getBestIndividual<GParameterSet>();
            nAdaptionsStore_.push_back(boost::tuple<double,double>(double(iteration), double(best->getNAdaptions())));
         } else { // Monitor all individuals
            // Loop over all individuals of the algorithm.
            for(std::size_t pos=0; pos<goa->size(); pos++) {
               boost::shared_ptr<GParameterSet> ind = goa->template individual_cast<GParameterSet>(pos);
               nAdaptionsStore_.push_back(boost::tuple<double,double>(double(iteration), double(ind->getNAdaptions())));
            }
         }
      }
      break;

      case Gem::Geneva::INFOEND:
      {
         std::vector<boost::tuple<double, double> >::iterator it;

         if(monitorBestOnly_) {
            // Create the graph object
            nAdaptionsGraph2D_oa_ = boost::shared_ptr<Gem::Common::GGraph2D>(new Gem::Common::GGraph2D());
            nAdaptionsGraph2D_oa_->setXAxisLabel("Iteration");
            nAdaptionsGraph2D_oa_->setYAxisLabel("Number of parameter adaptions");
            nAdaptionsGraph2D_oa_->setPlotMode(Gem::Common::CURVE);

            // Fill the object with data
            for(it=nAdaptionsStore_.begin(); it!=nAdaptionsStore_.end(); ++it) {
               (*nAdaptionsGraph2D_oa_) & *it;
            }

            // Add the histogram to the plot designer
            gpd_oa_.registerPlotter(nAdaptionsGraph2D_oa_);

         } else { // All individuals are monitored
            // Within nAdaptionsStore_, find the largest number of adaptions performed
            std::size_t maxNAdaptions = 0;
            for(it=nAdaptionsStore_.begin(); it!=nAdaptionsStore_.end(); ++it) {
               if(boost::get<1>(*it) > maxNAdaptions) {
                  maxNAdaptions = boost::numeric_cast<std::size_t>(boost::get<1>(*it));
               }
            }

            // Create the histogram object
            nAdaptionsHist2D_oa_ = boost::shared_ptr<GHistogram2D>(
                  new GHistogram2D(
                        nIterationsRecorded_
                        , maxNAdaptions+1
                        , 0., double(maxIteration_)
                        , 0., double(maxNAdaptions)
                  )
            );

            nAdaptionsHist2D_oa_->setXAxisLabel("Iteration");
            nAdaptionsHist2D_oa_->setYAxisLabel("Number of parameter adaptions");
            nAdaptionsHist2D_oa_->setDrawingArguments("BOX");

            // Fill the object with data
            for(it=nAdaptionsStore_.begin(); it!=nAdaptionsStore_.end(); ++it) {
               (*nAdaptionsHist2D_oa_) & *it;
            }

            // Add the histogram to the plot designer
            gpd_oa_.registerPlotter(nAdaptionsHist2D_oa_);
         }

         // Add the fitness monitor
         gpd_oa_.registerPlotter(fitnessGraph2D_oa_);

         // Inform the plot designer whether it should print png files
         gpd_oa_.setAddPrintCommand(addPrintCommand_);

         // Write out the result. Note that we add
         gpd_oa_.writeToFile(fileName_);

         // Remove all plotters
         gpd_oa_.resetPlotters();
         nAdaptionsHist2D_oa_.reset();
         nAdaptionsGraph2D_oa_.reset();
      }
      break;
      };
   }

private:
   /***************************************************************************/

   std::string fileName_; ///< The name of the file to which solutions should be stored

   boost::tuple<boost::uint32_t,boost::uint32_t> canvasDimensions_; ///< The dimensions of the canvas

   Gem::Common::GPlotDesigner gpd_oa_; ///< A wrapper for the plots

   boost::shared_ptr<Gem::Common::GHistogram2D> nAdaptionsHist2D_oa_;  ///< Holds the actual histogram
   boost::shared_ptr<Gem::Common::GGraph2D>     nAdaptionsGraph2D_oa_; ///< Used if we only monitor the best solution in each iteration
   boost::shared_ptr<Gem::Common::GGraph2D>     fitnessGraph2D_oa_;    ///< Lets us monitor the current fitness of the population

   bool monitorBestOnly_; ///< Indicates whether only the best individuals should be monitored
   bool addPrintCommand_; ///< Asks the GPlotDesigner to add a print command to result files

   std::size_t maxIteration_; ///< Holds the largest iteration recorded for the algorithm
   std::size_t nIterationsRecorded_; ///< Holds the number of iterations that were recorded (not necessarily == maxIteration_

   std::vector<boost::tuple<double, double> > nAdaptionsStore_; ///< Holds all information about the number of adaptions
};

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/
/**
 * This class allows to log chosen properties of adaptors. Such properties
 * are limited to numeric entities, that may be converted to double
 */
template <typename ind_type, typename num_type>
class GAdaptorPropertyLoggerT : public GBasePluggableOMT<ind_type>
{
   // Make sure this class can only be instantiated if ind_type is a derivative of GParameterSet
   BOOST_MPL_ASSERT((boost::is_base_of<GParameterSet, ind_type>));
   // Make sure this class can only be instantiated if num_type is numeric
   BOOST_MPL_ASSERT((boost::is_arithmetic<num_type>));

public:
   /***************************************************************************/
   /**
    * The default constructor
    */
   G_API_GENEVA GAdaptorPropertyLoggerT()
      : fileName_("NAdaptions.C")
      , adaptorName_("GDoubleGaussAdaptor")
      , property_("sigma")
      , canvasDimensions_(boost::tuple<boost::uint32_t,boost::uint32_t>(1200,1600))
      , gpd_oa_("Adaptor properties", 1, 2)
      , monitorBestOnly_(false)
      , addPrintCommand_(false)
      , maxIteration_(0)
      , nIterationsRecorded_(0)
      , adaptorPropertyStore_()
   { /* nothing */ }

   /***************************************************************************/
   /**
    * Initialization with a file name
    */
   G_API_GENEVA GAdaptorPropertyLoggerT(
      const std::string& fileName
      , const std::string& adaptorName
      , const std::string& property
   )
      : fileName_(fileName)
      , adaptorName_(adaptorName)
      , property_(property)
      , canvasDimensions_(boost::tuple<boost::uint32_t,boost::uint32_t>(1200,1600))
      , gpd_oa_("Adaptor properties", 1, 2)
      , monitorBestOnly_(false)
      , addPrintCommand_(false)
      , maxIteration_(0)
      , nIterationsRecorded_(0)
      , adaptorPropertyStore_()
   { /* nothing */ }

   /***************************************************************************/
   /**
    * The copy constructor
    */
   G_API_GENEVA GAdaptorPropertyLoggerT(const GAdaptorPropertyLoggerT<ind_type, num_type>& cp)
      : fileName_(cp.fileName_)
      , adaptorName_(cp.adaptorName_)
      , property_(cp.property_)
      , canvasDimensions_(cp.canvasDimensions_)
      , gpd_oa_("Number of adaptions per iteration", 1, 2) // Not copied
      , monitorBestOnly_(cp.monitorBestOnly_)
      , addPrintCommand_(cp.addPrintCommand_)
      , maxIteration_(cp.maxIteration_)
      , nIterationsRecorded_(cp.nIterationsRecorded_)
      , adaptorPropertyStore_(cp.adaptorPropertyStore_)
   { /* nothing */ }

   /***************************************************************************/
   /**
    * The destructor
    */
   virtual G_API_GENEVA ~GAdaptorPropertyLoggerT()
   { /* nothing */ }

   /***************************************************************************/
   /**
    * Sets the file name
    */
   G_API_GENEVA void setFileName(std::string fileName) {
      fileName_ = fileName;
   }

   /***************************************************************************/
   /**
    * Retrieves the current file name
    */
   G_API_GENEVA std::string getFileName() const {
      return fileName_;
   }

   /***************************************************************************/
   /**
    * Sets the name of the adaptor
    */
   G_API_GENEVA void setAdaptorName(std::string adaptorName) {
      adaptorName_ = adaptorName;
   }

   /***************************************************************************/
   /**
    * Retrieves the name of the adaptor
    */
   G_API_GENEVA std::string getAdaptorName() const {
      return adaptorName_;
   }

   /***************************************************************************/
   /**
    * Sets the name of the property
    */
   G_API_GENEVA void setPropertyName(std::string property) {
      property_ = property;
   }

   /***************************************************************************/
   /**
    * Retrieves the name of the property
    */
   G_API_GENEVA std::string getPropertyName() const {
      return property_;
   }

   /***************************************************************************/
   /**
    * Allows to specify whether only the best individuals should be monitored.
    */
   G_API_GENEVA void setMonitorBestOnly(bool monitorBestOnly = true) {
      monitorBestOnly_ = monitorBestOnly;
   }

   /***************************************************************************/
   /**
    * Allows to check whether only the best individuals should be monitored.
    */
   G_API_GENEVA bool getMonitorBestOnly() const {
      return monitorBestOnly_;
   }

   /***************************************************************************/
   /**
    * Allows to set the canvas dimensions
    */
   G_API_GENEVA void setCanvasDimensions(boost::tuple<boost::uint32_t,boost::uint32_t> canvasDimensions) {
      canvasDimensions_ = canvasDimensions;
   }

   /***************************************************************************/
   /**
    * Allows to set the canvas dimensions using separate x and y values
    */
   G_API_GENEVA void setCanvasDimensions(boost::uint32_t x, boost::uint32_t y) {
      canvasDimensions_ = boost::tuple<boost::uint32_t,boost::uint32_t>(x,y);
   }

   /***************************************************************************/
   /**
    * Gives access to the canvas dimensions
    */
   G_API_GENEVA boost::tuple<boost::uint32_t,boost::uint32_t> getCanvasDimensions() const {
      return canvasDimensions_;
   }

   /******************************************************************************/
   /**
    * Allows to add a "Print" command to the end of the script so that picture files are created
    */
   G_API_GENEVA void setAddPrintCommand(bool addPrintCommand) {
      addPrintCommand_ = addPrintCommand;
   }

   /******************************************************************************/
   /**
    * Allows to retrieve the current value of the addPrintCommand_ variable
    */
   G_API_GENEVA bool getAddPrintCommand() const {
      return addPrintCommand_;
   }

   /***************************************************************************/
   /**
    * Allows to emit information in different stages of the information cycle
    * (initialization, during each cycle and during finalization)
    */
   virtual G_API_GENEVA void informationFunction(
      const infoMode& im
      , GOptimizationAlgorithmT<ind_type> * const goa
   ) OVERRIDE {
      using namespace Gem::Common;

      switch(im) {
      case Gem::Geneva::INFOINIT:
      {
         // If the file pointed to by fileName_ already exists, make a back-up
         if(bf::exists(fileName_)) {
            const boost::posix_time::ptime currentTime = boost::posix_time::second_clock::local_time();
            std::string newFileName = fileName_ + ".bak_" + boost::lexical_cast<std::string>(currentTime);

            glogger
            << "In GAdaptorPropertyLoggerT<S,T>::informationFunction(): Error!" << std::endl
            << "Attempt to output information to file " << fileName_ << std::endl
            << "which already exists. We will rename the old file to" << std::endl
            << newFileName << std::endl
            << GWARNING;

            bf::rename(fileName_, newFileName);
         }

         // Make sure the progress plotter has the desired size
         gpd_oa_.setCanvasDimensions(canvasDimensions_);

         // Set up a graph to monitor the best fitness found
         fitnessGraph2D_oa_ = boost::shared_ptr<Gem::Common::GGraph2D>(new Gem::Common::GGraph2D());
         fitnessGraph2D_oa_->setXAxisLabel("Iteration");
         fitnessGraph2D_oa_->setYAxisLabel("Fitness");
         fitnessGraph2D_oa_->setPlotMode(Gem::Common::CURVE);
      }
      break;

      case Gem::Geneva::INFOPROCESSING:
      {
         boost::uint32_t iteration = goa->getIteration();

         // Record the current fitness
         boost::shared_ptr<GParameterSet> p = goa->GOptimizableI::template getBestIndividual<GParameterSet>();
         (*fitnessGraph2D_oa_) & boost::tuple<double,double>(double(iteration), double(p->fitness()));

         // Update the largest known iteration and the number of recorded iterations
         maxIteration_ = iteration;
         nIterationsRecorded_++;

         // Will hold the adaptor properties
         std::vector<boost::any> data;

         // Do the actual logging
         if(monitorBestOnly_) {
            boost::shared_ptr<GParameterSet> best = goa->GOptimizableI::template getBestIndividual<GParameterSet>();

            // Retrieve the adaptor data (e.g. the sigma of a GDoubleGaussAdaptor
            best->queryAdaptor(adaptorName_, property_, data);

            // Attach the data to adaptorPropertyStore_
            std::vector<boost::any>::iterator prop_it;
            for(prop_it=data.begin(); prop_it!=data.end(); ++prop_it) {
               adaptorPropertyStore_.push_back(boost::tuple<double,double>(double(iteration), double(boost::any_cast<num_type>(*prop_it))));
            }
         } else { // Monitor all individuals
            // Loop over all individuals of the algorithm.
            for(std::size_t pos=0; pos<goa->size(); pos++) {
               boost::shared_ptr<GParameterSet> ind = goa->template individual_cast<GParameterSet>(pos);

               // Retrieve the adaptor data (e.g. the sigma of a GDoubleGaussAdaptor
               ind->queryAdaptor(adaptorName_, property_, data);

               // Attach the data to adaptorPropertyStore_
               std::vector<boost::any>::iterator prop_it;
               for(prop_it=data.begin(); prop_it!=data.end(); ++prop_it) {
                  adaptorPropertyStore_.push_back(boost::tuple<double,double>(double(iteration), double(boost::any_cast<num_type>(*prop_it))));
               }
            }
         }
      }
      break;

      case Gem::Geneva::INFOEND:
      {
         std::vector<boost::tuple<double, double> >::iterator it;

         // Within adaptorPropertyStore_, find the largest number of adaptions performed
         double maxProperty = 0.;
         for(it=adaptorPropertyStore_.begin(); it!=adaptorPropertyStore_.end(); ++it) {
            if(boost::get<1>(*it) > maxProperty) {
               maxProperty = boost::get<1>(*it);
            }
         }

         // Create the histogram object
         adaptorPropertyHist2D_oa_ = boost::shared_ptr<GHistogram2D>(
               new GHistogram2D(
                     nIterationsRecorded_
                     , 100
                     , 0., double(maxIteration_)
                     , 0., maxProperty
               )
         );

         adaptorPropertyHist2D_oa_->setXAxisLabel("Iteration");
         adaptorPropertyHist2D_oa_->setYAxisLabel(std::string("Adaptor-Name: ") + adaptorName_ + std::string(", Property: ") + property_);
         adaptorPropertyHist2D_oa_->setDrawingArguments("BOX");

         // Fill the object with data
         for(it=adaptorPropertyStore_.begin(); it!=adaptorPropertyStore_.end(); ++it) {
            (*adaptorPropertyHist2D_oa_) & *it;
         }

         // Add the histogram to the plot designer
         gpd_oa_.registerPlotter(adaptorPropertyHist2D_oa_);

         // Add the fitness monitor
         gpd_oa_.registerPlotter(fitnessGraph2D_oa_);

         // Inform the plot designer whether it should print png files
         gpd_oa_.setAddPrintCommand(addPrintCommand_);

         // Write out the result. Note that we add
         gpd_oa_.writeToFile(fileName_);

         // Remove all plotters (they will survive inside of gpd)
         gpd_oa_.resetPlotters();
         adaptorPropertyHist2D_oa_.reset();
      }
      break;
      };
   }

private:
   /***************************************************************************/

   std::string fileName_; ///< The name of the file to which solutions should be stored

   std::string adaptorName_; ///< The  name of the adaptor for which properties should be logged
   std::string property_; ///< The name of the property to be logged

   boost::tuple<boost::uint32_t,boost::uint32_t> canvasDimensions_; ///< The dimensions of the canvas

   Gem::Common::GPlotDesigner gpd_oa_; ///< A wrapper for the plots

   boost::shared_ptr<Gem::Common::GHistogram2D> adaptorPropertyHist2D_oa_;  ///< Holds the actual histogram
   boost::shared_ptr<Gem::Common::GGraph2D>     fitnessGraph2D_oa_;    ///< Lets us monitor the current fitness of the population

   bool monitorBestOnly_; ///< Indicates whether only the best individuals should be monitored
   bool addPrintCommand_; ///< Asks the GPlotDesigner to add a print command to result files

   std::size_t maxIteration_; ///< Holds the largest iteration recorded for the algorithm
   std::size_t nIterationsRecorded_; ///< Holds the number of iterations that were recorded (not necessarily == maxIteration_

   std::vector<boost::tuple<double, double> > adaptorPropertyStore_; ///< Holds all information about the number of adaptions
};

/******************************************************************************/
////////////////////////////////////////////////////////////////////////////////
/******************************************************************************/

} /* namespace Geneva */
} /* namespace Gem */

#endif /* GPLUGGABLEOPTIMIZATIONMONITORST_HPP_ */
