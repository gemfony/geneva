/**
 * @file GOptOptMonitor.hpp
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

#ifndef GOPTOPTMONITOR_HPP_
#define GOPTOPTMONITOR_HPP_

// For Microsoft-compatible compilers
#if defined(_MSC_VER)  &&  (_MSC_VER >= 1020)
#pragma once
#endif

// Geneva header files go here
#include "common/GPlotDesigner.hpp" // Ease recording of essential information
#include "geneva/GBaseEA.hpp" // Get access to the GEAOptimizationMonitor class
#include "GMetaOptimizerIndividual.hpp" // The individual to be optimized

namespace Gem {
namespace Geneva {

const std::size_t P_XDIM=1200;
const std::size_t P_YDIM=1400;

/************************************************************************************************/
/**
 * This class implements an optimization monitor for Evolutionary Algorithms. Its main purpose is
 * to find out information about the development of sigma over the course of the optimization for
 * the best individuals. This monitor is thus targeted at a specific type of individual. Note that
 * the class uses ROOT scripts for the output of its results. It will also record the quality of
 * the best solutions found.
 */
class GOptOptMonitor
	:public GBaseEA::GEAOptimizationMonitor
{
public:
	/********************************************************************************************/
	/**
	 * The default constructor
	 */
	GOptOptMonitor(const std::string fileName)
      : fileName_(fileName)
      , gpd_("Progress information", 2, 4)
      , progressPlotter_(new Gem::Common::GGraph2D())
      , nParentPlotter_(new Gem::Common::GGraph2D())
      , nChildrenPlotter_(new Gem::Common::GGraph2D())
      , adProbPlotter_(new Gem::Common::GGraph2D())
      , minSigmaPlotter_(new Gem::Common::GGraph2D())
      , maxSigmaPlotter_(new Gem::Common::GGraph2D())
      , sigmaRangePlotter_(new Gem::Common::GGraph2D())
      , sigmaSigmaPlotter_(new Gem::Common::GGraph2D())
	{ /* nothing */ }

	/********************************************************************************************/
	/**
	 * The copy constructor
	 *
	 * @param cp A copy of another GOptOptMonitor object
	 */
	GOptOptMonitor(const GOptOptMonitor& cp)
		: GBaseEA::GEAOptimizationMonitor(cp)
		, fileName_(cp.fileName_)
	   , gpd_("Progress information", 2, 4) // We do not want to copy progress information of another object
      , progressPlotter_(new Gem::Common::GGraph2D())
      , nParentPlotter_(new Gem::Common::GGraph2D())
      , nChildrenPlotter_(new Gem::Common::GGraph2D())
      , adProbPlotter_(new Gem::Common::GGraph2D())
      , minSigmaPlotter_(new Gem::Common::GGraph2D())
      , maxSigmaPlotter_(new Gem::Common::GGraph2D())
      , sigmaRangePlotter_(new Gem::Common::GGraph2D())
      , sigmaSigmaPlotter_(new Gem::Common::GGraph2D())
	{ /* nothing */ }

	/********************************************************************************************/
	/**
	 * The destructor
	 */
	virtual ~GOptOptMonitor()
	{ /* nothing */ }

protected:
	/********************************************************************************************/
    /**
     * A function that is called once before the optimization starts
     *
     * @param cp A constant pointer to the evolutionary algorithm that is calling us
     */
    virtual void firstInformation(GOptimizationAlgorithmT<GParameterSet> * const goa) {
      // Initialize the plots we want to record
      progressPlotter_->setPlotMode(Gem::Common::CURVE);
      progressPlotter_->setPlotLabel("Number of solver calls");
      progressPlotter_->setXAxisLabel("Iteration");
      progressPlotter_->setYAxisLabel("Best Result (lower is better)");

      nParentPlotter_->setPlotMode(Gem::Common::CURVE);
      nParentPlotter_->setPlotLabel("Number of parents as a function of the iteration");
      nParentPlotter_->setXAxisLabel("Iteration");
      nParentPlotter_->setYAxisLabel("Number of parents");

      nChildrenPlotter_->setPlotMode(Gem::Common::CURVE);
      nChildrenPlotter_->setPlotLabel("Number of children as a function of the iteration");
      nChildrenPlotter_->setXAxisLabel("Iteration");
      nChildrenPlotter_->setYAxisLabel("Number of children");

      adProbPlotter_->setPlotMode(Gem::Common::CURVE);
      adProbPlotter_->setPlotLabel("Adaption probability as a function of the iteration");
      adProbPlotter_->setXAxisLabel("Iteration");
      adProbPlotter_->setYAxisLabel("Adaption probability");

      minSigmaPlotter_->setPlotMode(Gem::Common::CURVE);
      minSigmaPlotter_->setPlotLabel("Lower sigma boundary as a function of the iteration");
      minSigmaPlotter_->setXAxisLabel("Iteration");
      minSigmaPlotter_->setYAxisLabel("Lower sigma boundary");

      maxSigmaPlotter_->setPlotMode(Gem::Common::CURVE);
      maxSigmaPlotter_->setPlotLabel("Upper sigma boundary as a function of the iteration");
      maxSigmaPlotter_->setXAxisLabel("Iteration");
      maxSigmaPlotter_->setYAxisLabel("Upper sigma boundary");

      sigmaRangePlotter_->setPlotMode(Gem::Common::CURVE);
      sigmaRangePlotter_->setPlotLabel("Development of the sigma range as a function of the iteration");
      sigmaRangePlotter_->setXAxisLabel("Iteration");
      sigmaRangePlotter_->setYAxisLabel("Sigma range");

      sigmaSigmaPlotter_->setPlotMode(Gem::Common::CURVE);
      sigmaSigmaPlotter_->setPlotLabel("Development of the adaption strength as a function of the iteration");
      sigmaSigmaPlotter_->setXAxisLabel("Iteration");
      sigmaSigmaPlotter_->setYAxisLabel("Sigma-Sigma");

      gpd_.registerPlotter(progressPlotter_);
      gpd_.registerPlotter(nParentPlotter_);
      gpd_.registerPlotter(nChildrenPlotter_);
      gpd_.registerPlotter(adProbPlotter_);
      gpd_.registerPlotter(minSigmaPlotter_);
      gpd_.registerPlotter(maxSigmaPlotter_);
      gpd_.registerPlotter(sigmaRangePlotter_);
      gpd_.registerPlotter(sigmaSigmaPlotter_);

      gpd_.setCanvasDimensions(P_XDIM, P_YDIM);

    	// We call the parent classes eaFirstInformation function,
    	// as we do not want to change its actions
    	GBaseEA::GEAOptimizationMonitor::firstInformation(goa);
    }

	/********************************************************************************************/
    /**
     * A function that is called during each optimization cycle. The function first collects
     * the requested data, then calls the parent class'es eaCycleInformation() function, as
     * we do not want to change its actions.
     *
     * @param cp A constant pointer to the evolutionary algorithm that is calling us
     */
    virtual void cycleInformation(GOptimizationAlgorithmT<GParameterSet> * const goa) {
      // Convert the base pointer to the target type
      GBaseEA * const ea = static_cast<GBaseEA * const>(goa);

    	// Extract the requested data. First retrieve the best individual.
    	// It can always be found in the first position with evolutionary algorithms
    	boost::shared_ptr<GMetaOptimizerIndividual> p = ea->clone_at<GMetaOptimizerIndividual>(0);

    	// Retrieve the best fitness and average sigma value and add it to our local storage
    	(*progressPlotter_) & boost::tuple<double,double>((double)ea->getIteration(), p->fitness());
    	(*nParentPlotter_) & boost::tuple<double,double>((double)ea->getIteration(), (double) p->getNParents());
    	(*nChildrenPlotter_) & boost::tuple<double,double>((double)ea->getIteration(), (double) p->getNChildren());
    	(*adProbPlotter_) & boost::tuple<double,double>((double)ea->getIteration(), p->getAdProb());

    	double minSigma = p->getMinSigma();
    	double sigmaRange = p->getSigmaRange();
    	double maxSigma = minSigma+sigmaRange;

    	(*minSigmaPlotter_) & boost::tuple<double,double>((double)ea->getIteration(), minSigma);
      (*maxSigmaPlotter_) & boost::tuple<double,double>((double)ea->getIteration(), maxSigma);
      (*sigmaRangePlotter_) & boost::tuple<double,double>((double)ea->getIteration(), sigmaRange);
      (*sigmaSigmaPlotter_) & boost::tuple<double,double>((double)ea->getIteration(), p->getSigmaSigma());

    	//---------------------------------------------------------
    	// Call our parent class'es function
    	GBaseEA::GEAOptimizationMonitor::cycleInformation(goa);
    }

	/********************************************************************************************/
    /**
     * A function that is called once at the end of the optimization cycle
     *
     * @param cp A constant pointer to the evolutionary algorithm that is calling us
     */
    virtual void lastInformation(GOptimizationAlgorithmT<GParameterSet> * const goa) {
    	// Write out the result
    	gpd_.writeToFile(fileName_);

    	// We just call the parent classes lastInformation function,
    	// as we do not want to change its actions
    	GBaseEA::GEAOptimizationMonitor::lastInformation(goa);
    }

	/********************************************************************************************/
    /**
     * Loads the data of another object
     *
     * @param cp A copy of another GOptOptMonitor object, camouflaged as a GObject
     */
    virtual void load_(const GObject* cp) {
    	// Check that we are indeed dealing with an object of the same type and that we are not
    	// accidently trying to compare this object with itself.
    	const GOptOptMonitor *p_load = gobject_conversion<GOptOptMonitor>(cp);

    	// Trigger loading of our parent class'es data
    	GBaseEA::GEAOptimizationMonitor::load_(cp);

    	// Load local data
    	fileName_ = p_load->fileName_;
    }

	/********************************************************************************************/
    /**
     * Creates a deep clone of this object
     *
     * @return A deep clone of this object
     */
	virtual GObject* clone_() const {
		return new GOptOptMonitor(*this);
	}

private:
	/********************************************************************************************/

	GOptOptMonitor(); ///< Default constructor; Intentionally private and undefined

	std::string fileName_; ///< The name of the output file
	Gem::Common::GPlotDesigner gpd_; ///< Ease recording of essential information
	boost::shared_ptr<Gem::Common::GGraph2D> progressPlotter_; ///< Records progress information
	boost::shared_ptr<Gem::Common::GGraph2D> nParentPlotter_; ///< Records the number of parents in the individual
	boost::shared_ptr<Gem::Common::GGraph2D> nChildrenPlotter_; ///< Records the number of children in the individual
	boost::shared_ptr<Gem::Common::GGraph2D> adProbPlotter_; ///< Records the adaption probability for the individual
	boost::shared_ptr<Gem::Common::GGraph2D> minSigmaPlotter_; ///< Records the development of the lower sigma boundary
	boost::shared_ptr<Gem::Common::GGraph2D> maxSigmaPlotter_; ///< Records the development of the upper sigma boundary
	boost::shared_ptr<Gem::Common::GGraph2D> sigmaRangePlotter_; ///< Records the development of the sigma range
	boost::shared_ptr<Gem::Common::GGraph2D> sigmaSigmaPlotter_; ///< Records the development of the adaption strength
};

/************************************************************************************************/

} /* namespace Geneva */
} /* namespace Gem */

#endif /* GOPTOPTMONITOR_HPP_ */
