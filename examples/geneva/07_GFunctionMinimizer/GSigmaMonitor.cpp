/**
 * @file GSigmaMonitor.cpp
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

#include "GSigmaMonitor.hpp"

namespace Gem {
namespace Geneva {

/******************************************************************************/
/**
 * The default constructor
 */
GSigmaMonitor::GSigmaMonitor(const std::string fileName)
	: GBaseEA::GEAOptimizationMonitor()
	, fileName_(fileName)
	, gpd_("Progress information", 1, 2)
	, progressPlotter_(new Gem::Common::GGraph2D())
	, sigmaPlotter_(new Gem::Common::GGraph2D())
{ /* nothing */ }

/******************************************************************************/
/**
 * The copy constructor
 *
 * @param cp A copy of another GSigmaMonitor object
 */
GSigmaMonitor::GSigmaMonitor(const GSigmaMonitor& cp)
	: GBaseEA::GEAOptimizationMonitor(cp)
	, fileName_(cp.fileName_)
	, gpd_("Progress information", 1, 2) // We do not want to copy progress information of another object
	, progressPlotter_(new Gem::Common::GGraph2D())
	, sigmaPlotter_(new Gem::Common::GGraph2D())
{ /* nothing */ }

/******************************************************************************/
/**
 * The destructor
 */
GSigmaMonitor::~GSigmaMonitor()
{ /* nothing */ }

/******************************************************************************/
/**
 * A function that is called once before the optimization starts
 *
 * @param cp A constant pointer to the evolutionary algorithm that is calling us
 */
void GSigmaMonitor::firstInformation(GOptimizationAlgorithmT<GParameterSet> * const goa) {
	// Initialize the plots we want to record
	progressPlotter_->setPlotMode(Gem::Common::CURVE);
	progressPlotter_->setPlotLabel("Fitness as a function of the iteration");
	progressPlotter_->setXAxisLabel("Iteration");
	progressPlotter_->setYAxisLabel("Best Result (lower is better)");

	sigmaPlotter_->setPlotMode(Gem::Common::CURVE);
	sigmaPlotter_->setPlotLabel("Development of sigma (aka \\\"step width\\\")");
	sigmaPlotter_->setXAxisLabel("Iteration");
	sigmaPlotter_->setYAxisLabel("Sigma");

	gpd_.setCanvasDimensions(P_XDIM, P_YDIM);
	gpd_.registerPlotter(progressPlotter_);
	gpd_.registerPlotter(sigmaPlotter_);

	// We call the parent classes firstInformation function,
	// as we do not want to change its actions
	GBaseEA::GEAOptimizationMonitor::firstInformation(goa);
}

/******************************************************************************/
/**
 * A function that is called during each optimization cycle. The function first collects
 * the requested data, then calls the parent class'es eaCycleInformation() function, as
 * we do not want to change its actions.
 *
 * @param cp A constant pointer to the evolutionary algorithm that is calling us
 */
void GSigmaMonitor::cycleInformation(GOptimizationAlgorithmT<GParameterSet> * const goa) {
	// Convert the base pointer to the target type
	GBaseEA * const ea = static_cast<GBaseEA * const>(goa);

	// Extract the requested data. First retrieve the best individual.
	// It can always be found in the first position with evolutionary algorithms
	std::shared_ptr<GFMinIndividual> p = ea->clone_at<GFMinIndividual>(0);

	// Retrieve the best "raw" fitness and average sigma value and add it to our local storage
	progressPlotter_->add(std::tuple<double,double>((double)ea->getIteration(), p->fitness()));
	sigmaPlotter_->add(std::tuple<double,double>((double)ea->getIteration(), p->getAverageSigma()));
	//---------------------------------------------------------
	// Call our parent class'es function
	GBaseEA::GEAOptimizationMonitor::cycleInformation(goa);
}

/******************************************************************************/
/**
 * A function that is called once at the end of the optimization cycle
 *
 * @param cp A constant pointer to the evolutionary algorithm that is calling us
 */
void GSigmaMonitor::lastInformation(GOptimizationAlgorithmT<GParameterSet> * const goa) {
	// Write out the result
	gpd_.writeToFile(fileName_);

	// We just call the parent classes eaLastInformation function,
	// as we do not want to change its actions
	GBaseEA::GEAOptimizationMonitor::lastInformation(goa);
}

/******************************************************************************/
/**
 * Loads the data of another object
 *
 * @param cp A copy of another GSigmaMonitor object, camouflaged as a GObject
 */
void GSigmaMonitor::load_(const GObject* cp) {
	// Check that we are dealing with a GSigmaMonitor reference independent of this object and convert the pointer
	const GSigmaMonitor *p_load = Gem::Common::g_convert_and_compare<GObject, GSigmaMonitor>(cp, this);

	// Trigger loading of our parent class'es data
	GBaseEA::GEAOptimizationMonitor::load_(cp);

	// Load local data
	fileName_ = p_load->fileName_;
}

/******************************************************************************/
/**
 * Creates a deep clone of this object
 *
 * @return A deep clone of this object
 */
GObject* GSigmaMonitor::clone_() const {
	return new GSigmaMonitor(*this);
}

/******************************************************************************/
/**
 * The default constructor -- intentionally private
 */
GSigmaMonitor::GSigmaMonitor()
	: GBaseEA::GEAOptimizationMonitor()
	, fileName_("empty")
	, gpd_("Progress information", 1, 2)
	, progressPlotter_(new Gem::Common::GGraph2D())
	, sigmaPlotter_(new Gem::Common::GGraph2D())
{ /* nothing */ }

/******************************************************************************/

} /* namespace Geneva */
} /* namespace Gem */
