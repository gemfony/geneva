/**
 * @file GInfoFunction.hpp
 */

/* Copyright (C) Dr. Ruediger Berlich and Karlsruhe Institute of Technology
 * (University of the State of Baden-Wuerttemberg and National Laboratory
 * of the Helmholtz Association)
 *
 * Contact: info [at] gemfony (dot) com
 *
 * This file is part of the Geneva library, Gemfony scientific's optimization
 * library.
 *
 * Geneva is free software: you can redistribute it and/or modify
 * it under the terms of version 3 of the GNU Affero General Public License
 * as published by the Free Software Foundation.
 *
 * Geneva is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with the Geneva library.  If not, see <http://www.gnu.org/licenses/>.
 *
 * For further information on Gemfony scientific and Geneva, visit
 * http://www.gemfony.com .
 */

// Standard header files go here
#include <iostream>
#include <cmath>
#include <sstream>

// Boost header files go here
#include <boost/filesystem.hpp>
#include <boost/lexical_cast.hpp>

// GenEvA header files go here
#include "GRandom.hpp"
#include "GBasePopulation.hpp"
#include "GBoostThreadPopulation.hpp"
#include "GBrokerPopulation.hpp"
#include "GIndividualBroker.hpp"
#include "GAsioTCPConsumer.hpp"
#include "GAsioTCPClient.hpp"
#include "GAsioHelperFunctions.hpp"

// The individual that should be optimized
#include "GStartIndividual.hpp"

namespace Gem {
  namespace GenEvA {
    /************************************************************************************************/
    /**
     * An information object that will also emit result information in every n-th generation,
     * if requested.
     */
    class optimizationMonitor{
    public:
      /*********************************************************************************************/
      /**
       * The standard constructor. All collected data will to be written to file
       *
       * @param nGenInfo The number of generations after which a result file should be emitted (0 if none is desired)
       * @param nInfoIndividuals The amount of individuals for which information should be emitted
       * @param summary The stream to which information should be written
       */
      optimizationMonitor(const boost::uint16_t nGenInfo, 
			  const std::size_t nInfoIndividuals, 
			  std::ostream& summary)
	:nGenInfo_(nGenInfo),
	 nInfoIndividuals_(nInfoIndividuals),
	 summary_(summary)
      { /* nothing */  }
  
      /*********************************************************************************************/
      /**
       * The function that does the actual collection of data. It can be called in
       * three modes:
       *
       * INFOINIT: is called once before the optimization run.
       * INFOPROCESSING: is called in regular intervals during the optimization, as determined by the user
       * INFOEND: is called once after the optimization run
       * 
       * @param im The current mode in which the function is called
       * @param gbp A pointer to a GBasePopulation object for which information should be collected
       */
      void informationFunction(const infoMode& im, GBasePopulation * const gbp){
	// First act on the request to emit result files
	if(nGenInfo_) {
	  switch(im) {
	    //---------------------------------------------------------------------------
	  case Gem::GenEvA::INFOINIT:
	    {
	      // Output the header to the summary stream
	      summary_ << "{" << std::endl
		       << "  gROOT->Reset();" << std::endl
		       << "  gStyle->SetOptTitle(0);" << std::endl
		       << "  TCanvas *cc = new TCanvas(\"cc\",\"cc\",0,0,800,1200);" << std::endl
		       << "  cc->Divide(1," << nInfoIndividuals_ << ");" << std::endl 
		       << std::endl
		       << "  std::vector<long> generation;" << std::endl;

	      for(std::size_t p=0; p<nInfoIndividuals_; p++) {
		summary_ << "  std::vector<double> evaluation" << p << ";" << std::endl
			 << std::endl;
	      }
	    }
	    break;
	
	    //---------------------------------------------------------------------------
	  case Gem::GenEvA::INFOPROCESSING:
	    {
	      bool isDirty = false;
	      double currentEvaluation = 0.;

	      // Retrieve the current generation
	      boost::uint32_t generation = gbp->getGeneration();
	      
	      if(generation%nGenInfo_ == 0) {
		summary_ << "  generation.push_back(" << generation << ");" << std::endl;

		for(std::size_t p=0; p<nInfoIndividuals_; p++) {
		  // Get access to the inidividual
		  boost::shared_ptr<GStartIndividual> gdii_ptr = gbp->individual_cast<GStartIndividual>(p);
	    
		  // Retrieve the fitness of this individual
		  currentEvaluation = gdii_ptr->getCurrentFitness(isDirty);

		  // Let the audience know about the best result
		  if(p==0) {
		    std::cout << generation << ": " << currentEvaluation << std::endl;
		  }

		  // Write information to the output stream		  
		  summary_ << "  evaluation" << p << ".push_back(" <<  currentEvaluation << ");" << (isDirty?" // dirty flag is set":"") << std::endl;
		}
		summary_ << std::endl; // Improves readability when following the output with "tail -f"
	      }
	    }
	    break;
	
	    //---------------------------------------------------------------------------
	  case Gem::GenEvA::INFOEND:
	    {
	      // Output final print logic to the stream
	      summary_ << "  // Transfer the vectors into arrays" << std::endl
		       << "  double generation_arr[generation.size()];" << std::endl;
	      for(std::size_t p=0; p<nInfoIndividuals_; p++) {
		summary_ << "  double evaluation" << p << "_arr[evaluation" << p << ".size()];" << std::endl
			 << std::endl
			 << "  for(std::size_t i=0; i<generation.size(); i++) {" << std::endl;
		
		if(p==0) summary_ << "     generation_arr[i] = (double)generation[i];" << std::endl;
		
		summary_ << "     evaluation" << p << "_arr[i] = evaluation" << p << "[i];" << std::endl
			 << "  }" << std::endl
			 << std::endl
			 << "  // Create a TGraph object" << std::endl
			 << "  TGraph *evGraph" << p << " = new TGraph(evaluation" << p << ".size(), generation_arr, evaluation" << p << "_arr);" << std::endl
			 << std::endl;
	      }

	      summary_ << "  // Do the actual drawing" << std::endl;

	      for(std::size_t p=0; p<nInfoIndividuals_; p++) {
		summary_ << "  cc->cd(" << p+1 << ");" << std::endl
			 << "  evGraph" << p << "->Draw(\"AP\");" << std::endl;
	      }

	      summary_ << "  cc->cd();" << std::endl		
		       << "}" << std::endl;
	    }
	    break;

	    //---------------------------------------------------------------------------
	  default:
	    break;
	  };
	}     
      }

      /*********************************************************************************************/
  
    private:
      optimizationMonitor(); ///< Intentionally left undefined
  
      boost::uint16_t nGenInfo_; ///< The amount of generations after which results should be generated
      std::size_t nInfoIndividuals_; ///< The number of individuals for which information should be gathered
      std::ostream& summary_; ///< The stream to which information is written
    };

    /************************************************************************************************/

  } /* namespace GenEvA */
} /* namespace Gem */
