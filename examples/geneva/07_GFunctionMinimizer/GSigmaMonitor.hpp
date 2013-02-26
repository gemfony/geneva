/**
 * @file GSigmaMonitor.hpp
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

#ifndef GOPTIMIZATIONMONITOR_HPP_
#define GOPTIMIZATIONMONITOR_HPP_

// For Microsoft-compatible compilers
#if defined(_MSC_VER)  &&  (_MSC_VER >= 1020)
#pragma once
#endif

// Geneva header files go here
#include "geneva/GBaseEA.hpp" // Get access to the GEAOptimizationMonitor class
#include "GFMinIndividual.hpp" // The individual to be optimized

namespace Gem {
namespace Geneva {

/******************************************************************************/
/**
 * This class implements an optimization monitor for Evolutionary Algorithms. Its main purpose is
 * to find out information about the development of sigma over the course of the optimization for
 * the best individuals. This monitor is thus targeted at a specific type of individual. Note that
 * the class uses ROOT scripts for the output of its results.
 */
class GSigmaMonitor
	:public GBaseEA::GEAOptimizationMonitor
{
public:
	/***************************************************************************/
	/**
	 * The default constructor
	 */
	GSigmaMonitor(const std::string fileName)
		: fileName_(fileName)
	{ /* nothing */ }

	/***************************************************************************/
	/**
	 * The copy constructor
	 *
	 * @param cp A copy of another GSigmaMonitor object
	 */
	GSigmaMonitor(const GSigmaMonitor& cp)
		: GBaseEA::GEAOptimizationMonitor(cp)
		, bestSigma_(cp.bestSigma_)
		, fileName_(cp.fileName_)
	{ /* nothing */ }

	/***************************************************************************/
	/**
	 * The destructor
	 */
	virtual ~GSigmaMonitor()
	{ /* nothing */ }

protected:
	/***************************************************************************/
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
    	boost::shared_ptr<GFMinIndividual> p = ea->clone_at<GFMinIndividual>(0);

    	// Retrieve the average sigma value and add it to our local storage
    	bestSigma_.push_back(p->getAverageSigma());

    	//---------------------------------------------------------
    	// Call our parent class'es function
    	GBaseEA::GEAOptimizationMonitor::cycleInformation(goa);
    }

	/***************************************************************************/
    /**
     * A function that is called once at the end of the optimization cycle
     *
     * @param cp A constant pointer to the evolutionary algorithm that is calling us
     */
    virtual void lastInformation(GOptimizationAlgorithmT<GParameterSet> * const goa) {
    	// Write out the result
    	this->writeResult();

    	// We just call the parent classes eaLastInformation function,
    	// as we do not want to change its actions
    	GBaseEA::GEAOptimizationMonitor::lastInformation(goa);
    }

	/***************************************************************************/
	/**
	 * Writes out a ROOT script with the results to a file
	 *
	 * @param fileName The name of the file to which the results should be written
	 */
	void writeResult() const {
		// Open the requested file. No error-checks - this is a demo
		std::ofstream result(fileName_.c_str());

		// Write out the data
		result
		<< "{" << std::endl
		<< "  gROOT->Reset();" << std::endl
		<< "  gStyle->SetOptTitle(0);" << std::endl
		<< "  TCanvas *cc = new TCanvas(\"cc\",\"cc\",0,0,1024,768);" << std::endl
		<< std::endl
		<< "  std::vector<long> iteration;" << std::endl
		<< "  std::vector<double> sigma;" << std::endl
		<< std::endl
		<< "  // Fill with results" << std::endl;

		std::size_t iteration = 0;
		std::vector<double>::const_iterator sigma_it;
		for(sigma_it=bestSigma_.begin(); sigma_it!=bestSigma_.end(); ++sigma_it) {
			result
			<< "  iteration.push_back(" << iteration++ << ");" << std::endl
			<< "  sigma.push_back(" << *sigma_it << ");" << std::endl;
		}

		result
		<< std::endl
		<< "  // Transfer the results into a TGraph object" << std::endl
		<< "  double iteration_arr[iteration.size()];" << std::endl
		<< "  double sigma_arr[sigma.size()];" << std::endl
		<< std::endl
		<< "  for(std::size_t i=0; i<iteration.size(); i++) {" << std::endl
		<< "     iteration_arr[i] = (double)iteration[i];"
		<< "     sigma_arr[i] = sigma[i];" << std::endl
		<< "  }" << std::endl
		<< std::endl
		<< "  // Create a TGraph object" << std::endl
		<< "  TGraph *sGraph = new TGraph(sigma.size(), iteration_arr, sigma_arr);" << std::endl
		<< std::endl
		<< "  // Set the axis titles" << std::endl
		<< "  sGraph->GetXaxis()->SetTitle(\"Iteration\");" << std::endl
		<< "  sGraph->GetYaxis()->SetTitleOffset(1.1);" << std::endl
		<< "  sGraph->GetYaxis()->SetTitle(\"Average Sigma\");" << std::endl
		<< std::endl
		<< "  // Set the line color to red"
		<< "  sGraph->SetLineColor(2);" << std::endl
		<< std::endl
		<< "  // Set the y-axis to a logarithmic scale" << std::endl
		<< "  cc->SetLogy();"
		<< std::endl
		<< "  // Do the actual drawing" << std::endl
		<< "  sGraph->Draw(\"ALP\");" << std::endl
		<< "}" << std::endl;

		// Clean up
		result.close();
	}

	/***************************************************************************/
    /**
     * Loads the data of another object
     *
     * @param cp A copy of another GSigmaMonitor object, camouflaged as a GObject
     */
    virtual void load_(const GObject* cp) {
    	// Check that we are indeed dealing with an object of the same type and that we are not
    	// accidently trying to compare this object with itself.
    	const GSigmaMonitor *p_load = gobject_conversion<GSigmaMonitor>(cp);

    	// Trigger loading of our parent class'es data
    	GBaseEA::GEAOptimizationMonitor::load_(cp);

    	// Load local data
    	fileName_ = p_load->fileName_;
    	bestSigma_ = p_load->bestSigma_;
    }

	/***************************************************************************/
    /**
     * Creates a deep clone of this object
     *
     * @return A deep clone of this object
     */
	virtual GObject* clone_() const {
		return new GSigmaMonitor(*this);
	}

private:
	/***************************************************************************/

	GSigmaMonitor(); ///< Default constructor; Intentionally private and undefined

	std::vector<double> bestSigma_; ///< Holds the best sigma value of each iteration
	std::string fileName_;
};

/******************************************************************************/

} /* namespace Geneva */
} /* namespace Gem */

#endif /* GOPTIMIZATIONMONITOR_HPP_ */
